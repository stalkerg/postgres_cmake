/*-------------------------------------------------------------------------
 *
 * execPartition.c
 *	  Support routines for partitioning.
 *
 * Portions Copyright (c) 1996-2018, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  src/backend/executor/execPartition.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "catalog/pg_inherits_fn.h"
#include "executor/execPartition.h"
#include "executor/executor.h"
#include "mb/pg_wchar.h"
#include "miscadmin.h"
#include "utils/lsyscache.h"
#include "utils/rls.h"
#include "utils/ruleutils.h"

static PartitionDispatch *RelationGetPartitionDispatchInfo(Relation rel,
								 int *num_parted, List **leaf_part_oids);
static void get_partition_dispatch_recurse(Relation rel, Relation parent,
							   List **pds, List **leaf_part_oids);
static void FormPartitionKeyDatum(PartitionDispatch pd,
					  TupleTableSlot *slot,
					  EState *estate,
					  Datum *values,
					  bool *isnull);
static char *ExecBuildSlotPartitionKeyDescription(Relation rel,
									 Datum *values,
									 bool *isnull,
									 int maxfieldlen);

/*
 * ExecSetupPartitionTupleRouting - sets up information needed during
 * tuple routing for partitioned tables, encapsulates it in
 * PartitionTupleRouting, and returns it.
 *
 * Note that all the relations in the partition tree are locked using the
 * RowExclusiveLock mode upon return from this function.
 *
 * While we allocate the arrays of pointers of ResultRelInfo and
 * TupleConversionMap for all partitions here, actual objects themselves are
 * lazily allocated for a given partition if a tuple is actually routed to it;
 * see ExecInitPartitionInfo.  However, if the function is invoked for update
 * tuple routing, caller would already have initialized ResultRelInfo's for
 * some of the partitions, which are reused and assigned to their respective
 * slot in the aforementioned array.
 */
PartitionTupleRouting *
ExecSetupPartitionTupleRouting(ModifyTableState *mtstate, Relation rel)
{
	TupleDesc	tupDesc = RelationGetDescr(rel);
	List	   *leaf_parts;
	ListCell   *cell;
	int			i;
	ResultRelInfo *update_rri = NULL;
	int			num_update_rri = 0,
				update_rri_index = 0;
	PartitionTupleRouting *proute;

	/*
	 * Get the information about the partition tree after locking all the
	 * partitions.
	 */
	(void) find_all_inheritors(RelationGetRelid(rel), RowExclusiveLock, NULL);
	proute = (PartitionTupleRouting *) palloc0(sizeof(PartitionTupleRouting));
	proute->partition_dispatch_info =
		RelationGetPartitionDispatchInfo(rel, &proute->num_dispatch,
										 &leaf_parts);
	proute->num_partitions = list_length(leaf_parts);
	proute->partitions = (ResultRelInfo **) palloc(proute->num_partitions *
												   sizeof(ResultRelInfo *));
	proute->parent_child_tupconv_maps =
		(TupleConversionMap **) palloc0(proute->num_partitions *
										sizeof(TupleConversionMap *));
	proute->partition_oids = (Oid *) palloc(proute->num_partitions *
											sizeof(Oid));

	/* Set up details specific to the type of tuple routing we are doing. */
	if (mtstate && mtstate->operation == CMD_UPDATE)
	{
		ModifyTable *node = (ModifyTable *) mtstate->ps.plan;

		update_rri = mtstate->resultRelInfo;
		num_update_rri = list_length(node->plans);
		proute->subplan_partition_offsets =
			palloc(num_update_rri * sizeof(int));
		proute->num_subplan_partition_offsets = num_update_rri;

		/*
		 * We need an additional tuple slot for storing transient tuples that
		 * are converted to the root table descriptor.
		 */
		proute->root_tuple_slot = MakeTupleTableSlot(NULL);
	}

	/*
	 * Initialize an empty slot that will be used to manipulate tuples of any
	 * given partition's rowtype.  It is attached to the caller-specified node
	 * (such as ModifyTableState) and released when the node finishes
	 * processing.
	 */
	proute->partition_tuple_slot = MakeTupleTableSlot(NULL);

	i = 0;
	foreach(cell, leaf_parts)
	{
		ResultRelInfo *leaf_part_rri = NULL;
		Oid			leaf_oid = lfirst_oid(cell);

		proute->partition_oids[i] = leaf_oid;

		/*
		 * If the leaf partition is already present in the per-subplan result
		 * rels, we re-use that rather than initialize a new result rel. The
		 * per-subplan resultrels and the resultrels of the leaf partitions
		 * are both in the same canonical order. So while going through the
		 * leaf partition oids, we need to keep track of the next per-subplan
		 * result rel to be looked for in the leaf partition resultrels.
		 */
		if (update_rri_index < num_update_rri &&
			RelationGetRelid(update_rri[update_rri_index].ri_RelationDesc) == leaf_oid)
		{
			Relation	partrel;
			TupleDesc	part_tupdesc;

			leaf_part_rri = &update_rri[update_rri_index];
			partrel = leaf_part_rri->ri_RelationDesc;

			/*
			 * This is required in order to convert the partition's tuple to
			 * be compatible with the root partitioned table's tuple
			 * descriptor.  When generating the per-subplan result rels, this
			 * was not set.
			 */
			leaf_part_rri->ri_PartitionRoot = rel;

			/* Remember the subplan offset for this ResultRelInfo */
			proute->subplan_partition_offsets[update_rri_index] = i;

			update_rri_index++;

			part_tupdesc = RelationGetDescr(partrel);

			/*
			 * Save a tuple conversion map to convert a tuple routed to this
			 * partition from the parent's type to the partition's.
			 */
			proute->parent_child_tupconv_maps[i] =
				convert_tuples_by_name(tupDesc, part_tupdesc,
									   gettext_noop("could not convert row type"));

			/*
			 * Verify result relation is a valid target for an INSERT.  An
			 * UPDATE of a partition-key becomes a DELETE+INSERT operation, so
			 * this check is required even when the operation is CMD_UPDATE.
			 */
			CheckValidResultRel(leaf_part_rri, CMD_INSERT);
		}

		proute->partitions[i] = leaf_part_rri;
		i++;
	}

	/*
	 * For UPDATE, we should have found all the per-subplan resultrels in the
	 * leaf partitions.  (If this is an INSERT, both values will be zero.)
	 */
	Assert(update_rri_index == num_update_rri);

	return proute;
}

/*
 * ExecFindPartition -- Find a leaf partition in the partition tree rooted
 * at parent, for the heap tuple contained in *slot
 *
 * estate must be non-NULL; we'll need it to compute any expressions in the
 * partition key(s)
 *
 * If no leaf partition is found, this routine errors out with the appropriate
 * error message, else it returns the leaf partition sequence number
 * as an index into the array of (ResultRelInfos of) all leaf partitions in
 * the partition tree.
 */
int
ExecFindPartition(ResultRelInfo *resultRelInfo, PartitionDispatch *pd,
				  TupleTableSlot *slot, EState *estate)
{
	int			result;
	Datum		values[PARTITION_MAX_KEYS];
	bool		isnull[PARTITION_MAX_KEYS];
	Relation	rel;
	PartitionDispatch parent;
	ExprContext *ecxt = GetPerTupleExprContext(estate);
	TupleTableSlot *ecxt_scantuple_old = ecxt->ecxt_scantuple;

	/*
	 * First check the root table's partition constraint, if any.  No point in
	 * routing the tuple if it doesn't belong in the root table itself.
	 */
	if (resultRelInfo->ri_PartitionCheck &&
		!ExecPartitionCheck(resultRelInfo, slot, estate))
		ExecPartitionCheckEmitError(resultRelInfo, slot, estate);

	/* start with the root partitioned table */
	parent = pd[0];
	while (true)
	{
		PartitionDesc partdesc;
		TupleTableSlot *myslot = parent->tupslot;
		TupleConversionMap *map = parent->tupmap;
		int			cur_index = -1;

		rel = parent->reldesc;
		partdesc = RelationGetPartitionDesc(rel);

		/*
		 * Convert the tuple to this parent's layout so that we can do certain
		 * things we do below.
		 */
		if (myslot != NULL && map != NULL)
		{
			HeapTuple	tuple = ExecFetchSlotTuple(slot);

			ExecClearTuple(myslot);
			tuple = do_convert_tuple(tuple, map);
			ExecStoreTuple(tuple, myslot, InvalidBuffer, true);
			slot = myslot;
		}

		/*
		 * Extract partition key from tuple. Expression evaluation machinery
		 * that FormPartitionKeyDatum() invokes expects ecxt_scantuple to
		 * point to the correct tuple slot.  The slot might have changed from
		 * what was used for the parent table if the table of the current
		 * partitioning level has different tuple descriptor from the parent.
		 * So update ecxt_scantuple accordingly.
		 */
		ecxt->ecxt_scantuple = slot;
		FormPartitionKeyDatum(parent, slot, estate, values, isnull);

		/*
		 * Nothing for get_partition_for_tuple() to do if there are no
		 * partitions to begin with.
		 */
		if (partdesc->nparts == 0)
		{
			result = -1;
			break;
		}

		cur_index = get_partition_for_tuple(rel, values, isnull);

		/*
		 * cur_index < 0 means we failed to find a partition of this parent.
		 * cur_index >= 0 means we either found the leaf partition, or the
		 * next parent to find a partition of.
		 */
		if (cur_index < 0)
		{
			result = -1;
			break;
		}
		else if (parent->indexes[cur_index] >= 0)
		{
			result = parent->indexes[cur_index];
			break;
		}
		else
			parent = pd[-parent->indexes[cur_index]];
	}

	/* A partition was not found. */
	if (result < 0)
	{
		char	   *val_desc;

		val_desc = ExecBuildSlotPartitionKeyDescription(rel,
														values, isnull, 64);
		Assert(OidIsValid(RelationGetRelid(rel)));
		ereport(ERROR,
				(errcode(ERRCODE_CHECK_VIOLATION),
				 errmsg("no partition of relation \"%s\" found for row",
						RelationGetRelationName(rel)),
				 val_desc ? errdetail("Partition key of the failing row contains %s.", val_desc) : 0));
	}

	ecxt->ecxt_scantuple = ecxt_scantuple_old;
	return result;
}

/*
 * ExecInitPartitionInfo
 *		Initialize ResultRelInfo and other information for a partition if not
 *		already done
 *
 * Returns the ResultRelInfo
 */
ResultRelInfo *
ExecInitPartitionInfo(ModifyTableState *mtstate,
					  ResultRelInfo *resultRelInfo,
					  PartitionTupleRouting *proute,
					  EState *estate, int partidx)
{
	Relation	rootrel = resultRelInfo->ri_RelationDesc,
				partrel;
	ResultRelInfo *leaf_part_rri;
	ModifyTable *node = mtstate ? (ModifyTable *) mtstate->ps.plan : NULL;
	MemoryContext oldContext;

	/*
	 * We locked all the partitions in ExecSetupPartitionTupleRouting
	 * including the leaf partitions.
	 */
	partrel = heap_open(proute->partition_oids[partidx], NoLock);

	/*
	 * Keep ResultRelInfo and other information for this partition in the
	 * per-query memory context so they'll survive throughout the query.
	 */
	oldContext = MemoryContextSwitchTo(estate->es_query_cxt);

	leaf_part_rri = (ResultRelInfo *) palloc0(sizeof(ResultRelInfo));
	InitResultRelInfo(leaf_part_rri,
					  partrel,
					  node ? node->nominalRelation : 1,
					  rootrel,
					  estate->es_instrument);

	/*
	 * Verify result relation is a valid target for an INSERT.  An UPDATE of a
	 * partition-key becomes a DELETE+INSERT operation, so this check is still
	 * required when the operation is CMD_UPDATE.
	 */
	CheckValidResultRel(leaf_part_rri, CMD_INSERT);

	/*
	 * Since we've just initialized this ResultRelInfo, it's not in any list
	 * attached to the estate as yet.  Add it, so that it can be found later.
	 *
	 * Note that the entries in this list appear in no predetermined order,
	 * because partition result rels are initialized as and when they're
	 * needed.
	 */
	estate->es_tuple_routing_result_relations =
		lappend(estate->es_tuple_routing_result_relations,
				leaf_part_rri);

	/*
	 * Open partition indices.  The user may have asked to check for conflicts
	 * within this leaf partition and do "nothing" instead of throwing an
	 * error.  Be prepared in that case by initializing the index information
	 * needed by ExecInsert() to perform speculative insertions.
	 */
	if (partrel->rd_rel->relhasindex &&
		leaf_part_rri->ri_IndexRelationDescs == NULL)
		ExecOpenIndices(leaf_part_rri,
						(mtstate != NULL &&
						 mtstate->mt_onconflict != ONCONFLICT_NONE));

	/*
	 * Build WITH CHECK OPTION constraints for the partition.  Note that we
	 * didn't build the withCheckOptionList for partitions within the planner,
	 * but simple translation of varattnos will suffice.  This only occurs for
	 * the INSERT case or in the case of UPDATE tuple routing where we didn't
	 * find a result rel to reuse in ExecSetupPartitionTupleRouting().
	 */
	if (node && node->withCheckOptionLists != NIL)
	{
		List	   *wcoList;
		List	   *wcoExprs = NIL;
		ListCell   *ll;
		int			firstVarno = mtstate->resultRelInfo[0].ri_RangeTableIndex;
		Relation	firstResultRel = mtstate->resultRelInfo[0].ri_RelationDesc;

		/*
		 * In the case of INSERT on a partitioned table, there is only one
		 * plan.  Likewise, there is only one WCO list, not one per partition.
		 * For UPDATE, there are as many WCO lists as there are plans.
		 */
		Assert((node->operation == CMD_INSERT &&
				list_length(node->withCheckOptionLists) == 1 &&
				list_length(node->plans) == 1) ||
			   (node->operation == CMD_UPDATE &&
				list_length(node->withCheckOptionLists) ==
				list_length(node->plans)));

		/*
		 * Use the WCO list of the first plan as a reference to calculate
		 * attno's for the WCO list of this partition.  In the INSERT case,
		 * that refers to the root partitioned table, whereas in the UPDATE
		 * tuple routing case, that refers to the first partition in the
		 * mtstate->resultRelInfo array.  In any case, both that relation and
		 * this partition should have the same columns, so we should be able
		 * to map attributes successfully.
		 */
		wcoList = linitial(node->withCheckOptionLists);

		/*
		 * Convert Vars in it to contain this partition's attribute numbers.
		 */
		wcoList = map_partition_varattnos(wcoList, firstVarno,
										  partrel, firstResultRel, NULL);
		foreach(ll, wcoList)
		{
			WithCheckOption *wco = castNode(WithCheckOption, lfirst(ll));
			ExprState  *wcoExpr = ExecInitQual(castNode(List, wco->qual),
											   mtstate->mt_plans[0]);

			wcoExprs = lappend(wcoExprs, wcoExpr);
		}

		leaf_part_rri->ri_WithCheckOptions = wcoList;
		leaf_part_rri->ri_WithCheckOptionExprs = wcoExprs;
	}

	/*
	 * Build the RETURNING projection for the partition.  Note that we didn't
	 * build the returningList for partitions within the planner, but simple
	 * translation of varattnos will suffice.  This only occurs for the INSERT
	 * case or in the case of UPDATE tuple routing where we didn't find a
	 * result rel to reuse in ExecSetupPartitionTupleRouting().
	 */
	if (node && node->returningLists != NIL)
	{
		TupleTableSlot *slot;
		ExprContext *econtext;
		List	   *returningList;
		int			firstVarno = mtstate->resultRelInfo[0].ri_RangeTableIndex;
		Relation	firstResultRel = mtstate->resultRelInfo[0].ri_RelationDesc;

		/* See the comment above for WCO lists. */
		Assert((node->operation == CMD_INSERT &&
				list_length(node->returningLists) == 1 &&
				list_length(node->plans) == 1) ||
			   (node->operation == CMD_UPDATE &&
				list_length(node->returningLists) ==
				list_length(node->plans)));

		/*
		 * Use the RETURNING list of the first plan as a reference to
		 * calculate attno's for the RETURNING list of this partition.  See
		 * the comment above for WCO lists for more details on why this is
		 * okay.
		 */
		returningList = linitial(node->returningLists);

		/*
		 * Convert Vars in it to contain this partition's attribute numbers.
		 */
		returningList = map_partition_varattnos(returningList, firstVarno,
												partrel, firstResultRel,
												NULL);

		/*
		 * Initialize the projection itself.
		 *
		 * Use the slot and the expression context that would have been set up
		 * in ExecInitModifyTable() for projection's output.
		 */
		Assert(mtstate->ps.ps_ResultTupleSlot != NULL);
		slot = mtstate->ps.ps_ResultTupleSlot;
		Assert(mtstate->ps.ps_ExprContext != NULL);
		econtext = mtstate->ps.ps_ExprContext;
		leaf_part_rri->ri_projectReturning =
			ExecBuildProjectionInfo(returningList, econtext, slot,
									&mtstate->ps, RelationGetDescr(partrel));
	}

	Assert(proute->partitions[partidx] == NULL);
	proute->partitions[partidx] = leaf_part_rri;

	/*
	 * Save a tuple conversion map to convert a tuple routed to this partition
	 * from the parent's type to the partition's.
	 */
	proute->parent_child_tupconv_maps[partidx] =
		convert_tuples_by_name(RelationGetDescr(rootrel),
							   RelationGetDescr(partrel),
							   gettext_noop("could not convert row type"));

	MemoryContextSwitchTo(oldContext);

	return leaf_part_rri;
}

/*
 * ExecSetupChildParentMapForLeaf -- Initialize the per-leaf-partition
 * child-to-root tuple conversion map array.
 *
 * This map is required for capturing transition tuples when the target table
 * is a partitioned table. For a tuple that is routed by an INSERT or UPDATE,
 * we need to convert it from the leaf partition to the target table
 * descriptor.
 */
void
ExecSetupChildParentMapForLeaf(PartitionTupleRouting *proute)
{
	Assert(proute != NULL);

	/*
	 * These array elements get filled up with maps on an on-demand basis.
	 * Initially just set all of them to NULL.
	 */
	proute->child_parent_tupconv_maps =
		(TupleConversionMap **) palloc0(sizeof(TupleConversionMap *) *
										proute->num_partitions);

	/* Same is the case for this array. All the values are set to false */
	proute->child_parent_map_not_required =
		(bool *) palloc0(sizeof(bool) * proute->num_partitions);
}

/*
 * TupConvMapForLeaf -- Get the tuple conversion map for a given leaf partition
 * index.
 */
TupleConversionMap *
TupConvMapForLeaf(PartitionTupleRouting *proute,
				  ResultRelInfo *rootRelInfo, int leaf_index)
{
	ResultRelInfo **resultRelInfos = proute->partitions;
	TupleConversionMap **map;
	TupleDesc	tupdesc;

	/* Don't call this if we're not supposed to be using this type of map. */
	Assert(proute->child_parent_tupconv_maps != NULL);

	/* If it's already known that we don't need a map, return NULL. */
	if (proute->child_parent_map_not_required[leaf_index])
		return NULL;

	/* If we've already got a map, return it. */
	map = &proute->child_parent_tupconv_maps[leaf_index];
	if (*map != NULL)
		return *map;

	/* No map yet; try to create one. */
	tupdesc = RelationGetDescr(resultRelInfos[leaf_index]->ri_RelationDesc);
	*map =
		convert_tuples_by_name(tupdesc,
							   RelationGetDescr(rootRelInfo->ri_RelationDesc),
							   gettext_noop("could not convert row type"));

	/* If it turns out no map is needed, remember for next time. */
	proute->child_parent_map_not_required[leaf_index] = (*map == NULL);

	return *map;
}

/*
 * ConvertPartitionTupleSlot -- convenience function for tuple conversion.
 * The tuple, if converted, is stored in new_slot, and *p_my_slot is
 * updated to point to it.  new_slot typically should be one of the
 * dedicated partition tuple slots. If map is NULL, *p_my_slot is not changed.
 *
 * Returns the converted tuple, unless map is NULL, in which case original
 * tuple is returned unmodified.
 */
HeapTuple
ConvertPartitionTupleSlot(TupleConversionMap *map,
						  HeapTuple tuple,
						  TupleTableSlot *new_slot,
						  TupleTableSlot **p_my_slot)
{
	if (!map)
		return tuple;

	tuple = do_convert_tuple(tuple, map);

	/*
	 * Change the partition tuple slot descriptor, as per converted tuple.
	 */
	*p_my_slot = new_slot;
	Assert(new_slot != NULL);
	ExecSetSlotDescriptor(new_slot, map->outdesc);
	ExecStoreTuple(tuple, new_slot, InvalidBuffer, true);

	return tuple;
}

/*
 * ExecCleanupTupleRouting -- Clean up objects allocated for partition tuple
 * routing.
 *
 * Close all the partitioned tables, leaf partitions, and their indices.
 */
void
ExecCleanupTupleRouting(PartitionTupleRouting *proute)
{
	int			i;
	int			subplan_index = 0;

	/*
	 * Remember, proute->partition_dispatch_info[0] corresponds to the root
	 * partitioned table, which we must not try to close, because it is the
	 * main target table of the query that will be closed by callers such as
	 * ExecEndPlan() or DoCopy(). Also, tupslot is NULL for the root
	 * partitioned table.
	 */
	for (i = 1; i < proute->num_dispatch; i++)
	{
		PartitionDispatch pd = proute->partition_dispatch_info[i];

		heap_close(pd->reldesc, NoLock);
		ExecDropSingleTupleTableSlot(pd->tupslot);
	}

	for (i = 0; i < proute->num_partitions; i++)
	{
		ResultRelInfo *resultRelInfo = proute->partitions[i];

		/* skip further processsing for uninitialized partitions */
		if (resultRelInfo == NULL)
			continue;

		/*
		 * If this result rel is one of the UPDATE subplan result rels, let
		 * ExecEndPlan() close it. For INSERT or COPY,
		 * proute->subplan_partition_offsets will always be NULL. Note that
		 * the subplan_partition_offsets array and the partitions array have
		 * the partitions in the same order. So, while we iterate over
		 * partitions array, we also iterate over the
		 * subplan_partition_offsets array in order to figure out which of the
		 * result rels are present in the UPDATE subplans.
		 */
		if (proute->subplan_partition_offsets &&
			subplan_index < proute->num_subplan_partition_offsets &&
			proute->subplan_partition_offsets[subplan_index] == i)
		{
			subplan_index++;
			continue;
		}

		ExecCloseIndices(resultRelInfo);
		heap_close(resultRelInfo->ri_RelationDesc, NoLock);
	}

	/* Release the standalone partition tuple descriptors, if any */
	if (proute->root_tuple_slot)
		ExecDropSingleTupleTableSlot(proute->root_tuple_slot);
	if (proute->partition_tuple_slot)
		ExecDropSingleTupleTableSlot(proute->partition_tuple_slot);
}

/*
 * RelationGetPartitionDispatchInfo
 *		Returns information necessary to route tuples down a partition tree
 *
 * The number of elements in the returned array (that is, the number of
 * PartitionDispatch objects for the partitioned tables in the partition tree)
 * is returned in *num_parted and a list of the OIDs of all the leaf
 * partitions of rel is returned in *leaf_part_oids.
 *
 * All the relations in the partition tree (including 'rel') must have been
 * locked (using at least the AccessShareLock) by the caller.
 */
static PartitionDispatch *
RelationGetPartitionDispatchInfo(Relation rel,
								 int *num_parted, List **leaf_part_oids)
{
	List	   *pdlist = NIL;
	PartitionDispatchData **pd;
	ListCell   *lc;
	int			i;

	Assert(rel->rd_rel->relkind == RELKIND_PARTITIONED_TABLE);

	*num_parted = 0;
	*leaf_part_oids = NIL;

	get_partition_dispatch_recurse(rel, NULL, &pdlist, leaf_part_oids);
	*num_parted = list_length(pdlist);
	pd = (PartitionDispatchData **) palloc(*num_parted *
										   sizeof(PartitionDispatchData *));
	i = 0;
	foreach(lc, pdlist)
	{
		pd[i++] = lfirst(lc);
	}

	return pd;
}

/*
 * get_partition_dispatch_recurse
 *		Recursively expand partition tree rooted at rel
 *
 * As the partition tree is expanded in a depth-first manner, we maintain two
 * global lists: of PartitionDispatch objects corresponding to partitioned
 * tables in *pds and of the leaf partition OIDs in *leaf_part_oids.
 *
 * Note that the order of OIDs of leaf partitions in leaf_part_oids matches
 * the order in which the planner's expand_partitioned_rtentry() processes
 * them.  It's not necessarily the case that the offsets match up exactly,
 * because constraint exclusion might prune away some partitions on the
 * planner side, whereas we'll always have the complete list; but unpruned
 * partitions will appear in the same order in the plan as they are returned
 * here.
 */
static void
get_partition_dispatch_recurse(Relation rel, Relation parent,
							   List **pds, List **leaf_part_oids)
{
	TupleDesc	tupdesc = RelationGetDescr(rel);
	PartitionDesc partdesc = RelationGetPartitionDesc(rel);
	PartitionKey partkey = RelationGetPartitionKey(rel);
	PartitionDispatch pd;
	int			i;

	check_stack_depth();

	/* Build a PartitionDispatch for this table and add it to *pds. */
	pd = (PartitionDispatch) palloc(sizeof(PartitionDispatchData));
	*pds = lappend(*pds, pd);
	pd->reldesc = rel;
	pd->key = partkey;
	pd->keystate = NIL;
	pd->partdesc = partdesc;
	if (parent != NULL)
	{
		/*
		 * For every partitioned table other than the root, we must store a
		 * tuple table slot initialized with its tuple descriptor and a tuple
		 * conversion map to convert a tuple from its parent's rowtype to its
		 * own. That is to make sure that we are looking at the correct row
		 * using the correct tuple descriptor when computing its partition key
		 * for tuple routing.
		 */
		pd->tupslot = MakeSingleTupleTableSlot(tupdesc);
		pd->tupmap = convert_tuples_by_name(RelationGetDescr(parent),
											tupdesc,
											gettext_noop("could not convert row type"));
	}
	else
	{
		/* Not required for the root partitioned table */
		pd->tupslot = NULL;
		pd->tupmap = NULL;
	}

	/*
	 * Go look at each partition of this table.  If it's a leaf partition,
	 * simply add its OID to *leaf_part_oids.  If it's a partitioned table,
	 * recursively call get_partition_dispatch_recurse(), so that its
	 * partitions are processed as well and a corresponding PartitionDispatch
	 * object gets added to *pds.
	 *
	 * About the values in pd->indexes: for a leaf partition, it contains the
	 * leaf partition's position in the global list *leaf_part_oids minus 1,
	 * whereas for a partitioned table partition, it contains the partition's
	 * position in the global list *pds multiplied by -1.  The latter is
	 * multiplied by -1 to distinguish partitioned tables from leaf partitions
	 * when going through the values in pd->indexes.  So, for example, when
	 * using it during tuple-routing, encountering a value >= 0 means we found
	 * a leaf partition.  It is immediately returned as the index in the array
	 * of ResultRelInfos of all the leaf partitions, using which we insert the
	 * tuple into that leaf partition.  A negative value means we found a
	 * partitioned table.  The value multiplied by -1 is returned as the index
	 * in the array of PartitionDispatch objects of all partitioned tables in
	 * the tree.  This value is used to continue the search in the next level
	 * of the partition tree.
	 */
	pd->indexes = (int *) palloc(partdesc->nparts * sizeof(int));
	for (i = 0; i < partdesc->nparts; i++)
	{
		Oid			partrelid = partdesc->oids[i];

		if (get_rel_relkind(partrelid) != RELKIND_PARTITIONED_TABLE)
		{
			*leaf_part_oids = lappend_oid(*leaf_part_oids, partrelid);
			pd->indexes[i] = list_length(*leaf_part_oids) - 1;
		}
		else
		{
			/*
			 * We assume all tables in the partition tree were already locked
			 * by the caller.
			 */
			Relation	partrel = heap_open(partrelid, NoLock);

			pd->indexes[i] = -list_length(*pds);
			get_partition_dispatch_recurse(partrel, rel, pds, leaf_part_oids);
		}
	}
}

/* ----------------
 *		FormPartitionKeyDatum
 *			Construct values[] and isnull[] arrays for the partition key
 *			of a tuple.
 *
 *	pd				Partition dispatch object of the partitioned table
 *	slot			Heap tuple from which to extract partition key
 *	estate			executor state for evaluating any partition key
 *					expressions (must be non-NULL)
 *	values			Array of partition key Datums (output area)
 *	isnull			Array of is-null indicators (output area)
 *
 * the ecxt_scantuple slot of estate's per-tuple expr context must point to
 * the heap tuple passed in.
 * ----------------
 */
static void
FormPartitionKeyDatum(PartitionDispatch pd,
					  TupleTableSlot *slot,
					  EState *estate,
					  Datum *values,
					  bool *isnull)
{
	ListCell   *partexpr_item;
	int			i;

	if (pd->key->partexprs != NIL && pd->keystate == NIL)
	{
		/* Check caller has set up context correctly */
		Assert(estate != NULL &&
			   GetPerTupleExprContext(estate)->ecxt_scantuple == slot);

		/* First time through, set up expression evaluation state */
		pd->keystate = ExecPrepareExprList(pd->key->partexprs, estate);
	}

	partexpr_item = list_head(pd->keystate);
	for (i = 0; i < pd->key->partnatts; i++)
	{
		AttrNumber	keycol = pd->key->partattrs[i];
		Datum		datum;
		bool		isNull;

		if (keycol != 0)
		{
			/* Plain column; get the value directly from the heap tuple */
			datum = slot_getattr(slot, keycol, &isNull);
		}
		else
		{
			/* Expression; need to evaluate it */
			if (partexpr_item == NULL)
				elog(ERROR, "wrong number of partition key expressions");
			datum = ExecEvalExprSwitchContext((ExprState *) lfirst(partexpr_item),
											  GetPerTupleExprContext(estate),
											  &isNull);
			partexpr_item = lnext(partexpr_item);
		}
		values[i] = datum;
		isnull[i] = isNull;
	}

	if (partexpr_item != NULL)
		elog(ERROR, "wrong number of partition key expressions");
}

/*
 * ExecBuildSlotPartitionKeyDescription
 *
 * This works very much like BuildIndexValueDescription() and is currently
 * used for building error messages when ExecFindPartition() fails to find
 * partition for a row.
 */
static char *
ExecBuildSlotPartitionKeyDescription(Relation rel,
									 Datum *values,
									 bool *isnull,
									 int maxfieldlen)
{
	StringInfoData buf;
	PartitionKey key = RelationGetPartitionKey(rel);
	int			partnatts = get_partition_natts(key);
	int			i;
	Oid			relid = RelationGetRelid(rel);
	AclResult	aclresult;

	if (check_enable_rls(relid, InvalidOid, true) == RLS_ENABLED)
		return NULL;

	/* If the user has table-level access, just go build the description. */
	aclresult = pg_class_aclcheck(relid, GetUserId(), ACL_SELECT);
	if (aclresult != ACLCHECK_OK)
	{
		/*
		 * Step through the columns of the partition key and make sure the
		 * user has SELECT rights on all of them.
		 */
		for (i = 0; i < partnatts; i++)
		{
			AttrNumber	attnum = get_partition_col_attnum(key, i);

			/*
			 * If this partition key column is an expression, we return no
			 * detail rather than try to figure out what column(s) the
			 * expression includes and if the user has SELECT rights on them.
			 */
			if (attnum == InvalidAttrNumber ||
				pg_attribute_aclcheck(relid, attnum, GetUserId(),
									  ACL_SELECT) != ACLCHECK_OK)
				return NULL;
		}
	}

	initStringInfo(&buf);
	appendStringInfo(&buf, "(%s) = (",
					 pg_get_partkeydef_columns(relid, true));

	for (i = 0; i < partnatts; i++)
	{
		char	   *val;
		int			vallen;

		if (isnull[i])
			val = "null";
		else
		{
			Oid			foutoid;
			bool		typisvarlena;

			getTypeOutputInfo(get_partition_col_typid(key, i),
							  &foutoid, &typisvarlena);
			val = OidOutputFunctionCall(foutoid, values[i]);
		}

		if (i > 0)
			appendStringInfoString(&buf, ", ");

		/* truncate if needed */
		vallen = strlen(val);
		if (vallen <= maxfieldlen)
			appendStringInfoString(&buf, val);
		else
		{
			vallen = pg_mbcliplen(val, vallen, maxfieldlen);
			appendBinaryStringInfo(&buf, val, vallen);
			appendStringInfoString(&buf, "...");
		}
	}

	appendStringInfoChar(&buf, ')');

	return buf.data;
}
