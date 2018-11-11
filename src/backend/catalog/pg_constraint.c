/*-------------------------------------------------------------------------
 *
 * pg_constraint.c
 *	  routines to support manipulation of the pg_constraint relation
 *
 * Portions Copyright (c) 1996-2018, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/catalog/pg_constraint.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/genam.h"
#include "access/heapam.h"
#include "access/htup_details.h"
#include "access/sysattr.h"
#include "access/tupconvert.h"
#include "access/xact.h"
#include "catalog/dependency.h"
#include "catalog/indexing.h"
#include "catalog/objectaccess.h"
#include "catalog/partition.h"
#include "catalog/pg_constraint.h"
#include "catalog/pg_operator.h"
#include "catalog/pg_type.h"
#include "commands/defrem.h"
#include "commands/tablecmds.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/fmgroids.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "utils/syscache.h"
#include "utils/tqual.h"


static void clone_fk_constraints(Relation pg_constraint, Relation parentRel,
					 Relation partRel, List *clone, List **cloned);


/*
 * CreateConstraintEntry
 *	Create a constraint table entry.
 *
 * Subsidiary records (such as triggers or indexes to implement the
 * constraint) are *not* created here.  But we do make dependency links
 * from the constraint to the things it depends on.
 *
 * The new constraint's OID is returned.
 */
Oid
CreateConstraintEntry(const char *constraintName,
					  Oid constraintNamespace,
					  char constraintType,
					  bool isDeferrable,
					  bool isDeferred,
					  bool isValidated,
					  Oid parentConstrId,
					  Oid relId,
					  const int16 *constraintKey,
					  int constraintNKeys,
					  int constraintNTotalKeys,
					  Oid domainId,
					  Oid indexRelId,
					  Oid foreignRelId,
					  const int16 *foreignKey,
					  const Oid *pfEqOp,
					  const Oid *ppEqOp,
					  const Oid *ffEqOp,
					  int foreignNKeys,
					  char foreignUpdateType,
					  char foreignDeleteType,
					  char foreignMatchType,
					  const Oid *exclOp,
					  Node *conExpr,
					  const char *conBin,
					  bool conIsLocal,
					  int conInhCount,
					  bool conNoInherit,
					  bool is_internal)
{
	Relation	conDesc;
	Oid			conOid;
	HeapTuple	tup;
	bool		nulls[Natts_pg_constraint];
	Datum		values[Natts_pg_constraint];
	ArrayType  *conkeyArray;
	ArrayType  *confkeyArray;
	ArrayType  *conpfeqopArray;
	ArrayType  *conppeqopArray;
	ArrayType  *conffeqopArray;
	ArrayType  *conexclopArray;
	NameData	cname;
	int			i;
	ObjectAddress conobject;

	conDesc = heap_open(ConstraintRelationId, RowExclusiveLock);

	Assert(constraintName);
	namestrcpy(&cname, constraintName);

	/*
	 * Convert C arrays into Postgres arrays.
	 */
	if (constraintNKeys > 0)
	{
		Datum	   *conkey;

		conkey = (Datum *) palloc(constraintNKeys * sizeof(Datum));
		for (i = 0; i < constraintNKeys; i++)
			conkey[i] = Int16GetDatum(constraintKey[i]);
		conkeyArray = construct_array(conkey, constraintNKeys,
									  INT2OID, 2, true, 's');
	}
	else
		conkeyArray = NULL;

	if (foreignNKeys > 0)
	{
		Datum	   *fkdatums;

		fkdatums = (Datum *) palloc(foreignNKeys * sizeof(Datum));
		for (i = 0; i < foreignNKeys; i++)
			fkdatums[i] = Int16GetDatum(foreignKey[i]);
		confkeyArray = construct_array(fkdatums, foreignNKeys,
									   INT2OID, 2, true, 's');
		for (i = 0; i < foreignNKeys; i++)
			fkdatums[i] = ObjectIdGetDatum(pfEqOp[i]);
		conpfeqopArray = construct_array(fkdatums, foreignNKeys,
										 OIDOID, sizeof(Oid), true, 'i');
		for (i = 0; i < foreignNKeys; i++)
			fkdatums[i] = ObjectIdGetDatum(ppEqOp[i]);
		conppeqopArray = construct_array(fkdatums, foreignNKeys,
										 OIDOID, sizeof(Oid), true, 'i');
		for (i = 0; i < foreignNKeys; i++)
			fkdatums[i] = ObjectIdGetDatum(ffEqOp[i]);
		conffeqopArray = construct_array(fkdatums, foreignNKeys,
										 OIDOID, sizeof(Oid), true, 'i');
	}
	else
	{
		confkeyArray = NULL;
		conpfeqopArray = NULL;
		conppeqopArray = NULL;
		conffeqopArray = NULL;
	}

	if (exclOp != NULL)
	{
		Datum	   *opdatums;

		opdatums = (Datum *) palloc(constraintNKeys * sizeof(Datum));
		for (i = 0; i < constraintNKeys; i++)
			opdatums[i] = ObjectIdGetDatum(exclOp[i]);
		conexclopArray = construct_array(opdatums, constraintNKeys,
										 OIDOID, sizeof(Oid), true, 'i');
	}
	else
		conexclopArray = NULL;

	/* initialize nulls and values */
	for (i = 0; i < Natts_pg_constraint; i++)
	{
		nulls[i] = false;
		values[i] = (Datum) NULL;
	}

	values[Anum_pg_constraint_conname - 1] = NameGetDatum(&cname);
	values[Anum_pg_constraint_connamespace - 1] = ObjectIdGetDatum(constraintNamespace);
	values[Anum_pg_constraint_contype - 1] = CharGetDatum(constraintType);
	values[Anum_pg_constraint_condeferrable - 1] = BoolGetDatum(isDeferrable);
	values[Anum_pg_constraint_condeferred - 1] = BoolGetDatum(isDeferred);
	values[Anum_pg_constraint_convalidated - 1] = BoolGetDatum(isValidated);
	values[Anum_pg_constraint_conrelid - 1] = ObjectIdGetDatum(relId);
	values[Anum_pg_constraint_contypid - 1] = ObjectIdGetDatum(domainId);
	values[Anum_pg_constraint_conindid - 1] = ObjectIdGetDatum(indexRelId);
	values[Anum_pg_constraint_conparentid - 1] = ObjectIdGetDatum(parentConstrId);
	values[Anum_pg_constraint_confrelid - 1] = ObjectIdGetDatum(foreignRelId);
	values[Anum_pg_constraint_confupdtype - 1] = CharGetDatum(foreignUpdateType);
	values[Anum_pg_constraint_confdeltype - 1] = CharGetDatum(foreignDeleteType);
	values[Anum_pg_constraint_confmatchtype - 1] = CharGetDatum(foreignMatchType);
	values[Anum_pg_constraint_conislocal - 1] = BoolGetDatum(conIsLocal);
	values[Anum_pg_constraint_coninhcount - 1] = Int32GetDatum(conInhCount);
	values[Anum_pg_constraint_connoinherit - 1] = BoolGetDatum(conNoInherit);

	if (conkeyArray)
		values[Anum_pg_constraint_conkey - 1] = PointerGetDatum(conkeyArray);
	else
		nulls[Anum_pg_constraint_conkey - 1] = true;

	if (confkeyArray)
		values[Anum_pg_constraint_confkey - 1] = PointerGetDatum(confkeyArray);
	else
		nulls[Anum_pg_constraint_confkey - 1] = true;

	if (conpfeqopArray)
		values[Anum_pg_constraint_conpfeqop - 1] = PointerGetDatum(conpfeqopArray);
	else
		nulls[Anum_pg_constraint_conpfeqop - 1] = true;

	if (conppeqopArray)
		values[Anum_pg_constraint_conppeqop - 1] = PointerGetDatum(conppeqopArray);
	else
		nulls[Anum_pg_constraint_conppeqop - 1] = true;

	if (conffeqopArray)
		values[Anum_pg_constraint_conffeqop - 1] = PointerGetDatum(conffeqopArray);
	else
		nulls[Anum_pg_constraint_conffeqop - 1] = true;

	if (conexclopArray)
		values[Anum_pg_constraint_conexclop - 1] = PointerGetDatum(conexclopArray);
	else
		nulls[Anum_pg_constraint_conexclop - 1] = true;

	if (conBin)
		values[Anum_pg_constraint_conbin - 1] = CStringGetTextDatum(conBin);
	else
		nulls[Anum_pg_constraint_conbin - 1] = true;

	tup = heap_form_tuple(RelationGetDescr(conDesc), values, nulls);

	conOid = CatalogTupleInsert(conDesc, tup);

	conobject.classId = ConstraintRelationId;
	conobject.objectId = conOid;
	conobject.objectSubId = 0;

	heap_close(conDesc, RowExclusiveLock);

	if (OidIsValid(relId))
	{
		/*
		 * Register auto dependency from constraint to owning relation, or to
		 * specific column(s) if any are mentioned.
		 */
		ObjectAddress relobject;

		relobject.classId = RelationRelationId;
		relobject.objectId = relId;
		if (constraintNTotalKeys > 0)
		{
			for (i = 0; i < constraintNTotalKeys; i++)
			{
				relobject.objectSubId = constraintKey[i];

				recordDependencyOn(&conobject, &relobject, DEPENDENCY_AUTO);
			}
		}
		else
		{
			relobject.objectSubId = 0;

			recordDependencyOn(&conobject, &relobject, DEPENDENCY_AUTO);
		}
	}

	if (OidIsValid(domainId))
	{
		/*
		 * Register auto dependency from constraint to owning domain
		 */
		ObjectAddress domobject;

		domobject.classId = TypeRelationId;
		domobject.objectId = domainId;
		domobject.objectSubId = 0;

		recordDependencyOn(&conobject, &domobject, DEPENDENCY_AUTO);
	}

	if (OidIsValid(foreignRelId))
	{
		/*
		 * Register normal dependency from constraint to foreign relation, or
		 * to specific column(s) if any are mentioned.
		 */
		ObjectAddress relobject;

		relobject.classId = RelationRelationId;
		relobject.objectId = foreignRelId;
		if (foreignNKeys > 0)
		{
			for (i = 0; i < foreignNKeys; i++)
			{
				relobject.objectSubId = foreignKey[i];

				recordDependencyOn(&conobject, &relobject, DEPENDENCY_NORMAL);
			}
		}
		else
		{
			relobject.objectSubId = 0;

			recordDependencyOn(&conobject, &relobject, DEPENDENCY_NORMAL);
		}
	}

	if (OidIsValid(indexRelId) && constraintType == CONSTRAINT_FOREIGN)
	{
		/*
		 * Register normal dependency on the unique index that supports a
		 * foreign-key constraint.  (Note: for indexes associated with unique
		 * or primary-key constraints, the dependency runs the other way, and
		 * is not made here.)
		 */
		ObjectAddress relobject;

		relobject.classId = RelationRelationId;
		relobject.objectId = indexRelId;
		relobject.objectSubId = 0;

		recordDependencyOn(&conobject, &relobject, DEPENDENCY_NORMAL);
	}

	if (foreignNKeys > 0)
	{
		/*
		 * Register normal dependencies on the equality operators that support
		 * a foreign-key constraint.  If the PK and FK types are the same then
		 * all three operators for a column are the same; otherwise they are
		 * different.
		 */
		ObjectAddress oprobject;

		oprobject.classId = OperatorRelationId;
		oprobject.objectSubId = 0;

		for (i = 0; i < foreignNKeys; i++)
		{
			oprobject.objectId = pfEqOp[i];
			recordDependencyOn(&conobject, &oprobject, DEPENDENCY_NORMAL);
			if (ppEqOp[i] != pfEqOp[i])
			{
				oprobject.objectId = ppEqOp[i];
				recordDependencyOn(&conobject, &oprobject, DEPENDENCY_NORMAL);
			}
			if (ffEqOp[i] != pfEqOp[i])
			{
				oprobject.objectId = ffEqOp[i];
				recordDependencyOn(&conobject, &oprobject, DEPENDENCY_NORMAL);
			}
		}
	}

	/*
	 * We don't bother to register dependencies on the exclusion operators of
	 * an exclusion constraint.  We assume they are members of the opclass
	 * supporting the index, so there's an indirect dependency via that. (This
	 * would be pretty dicey for cross-type operators, but exclusion operators
	 * can never be cross-type.)
	 */

	if (conExpr != NULL)
	{
		/*
		 * Register dependencies from constraint to objects mentioned in CHECK
		 * expression.
		 */
		recordDependencyOnSingleRelExpr(&conobject, conExpr, relId,
										DEPENDENCY_NORMAL,
										DEPENDENCY_NORMAL, false);
	}

	/* Post creation hook for new constraint */
	InvokeObjectPostCreateHookArg(ConstraintRelationId, conOid, 0,
								  is_internal);

	return conOid;
}

/*
 * CloneForeignKeyConstraints
 *		Clone foreign keys from a partitioned table to a newly acquired
 *		partition.
 *
 * relationId is a partition of parentId, so we can be certain that it has the
 * same columns with the same datatypes.  The columns may be in different
 * order, though.
 *
 * The *cloned list is appended ClonedConstraint elements describing what was
 * created.
 */
void
CloneForeignKeyConstraints(Oid parentId, Oid relationId, List **cloned)
{
	Relation	pg_constraint;
	Relation	parentRel;
	Relation	rel;
	ScanKeyData key;
	SysScanDesc scan;
	HeapTuple	tuple;
	List	   *clone = NIL;

	parentRel = heap_open(parentId, NoLock);	/* already got lock */
	/* see ATAddForeignKeyConstraint about lock level */
	rel = heap_open(relationId, AccessExclusiveLock);
	pg_constraint = heap_open(ConstraintRelationId, RowShareLock);

	/* Obtain the list of constraints to clone or attach */
	ScanKeyInit(&key,
				Anum_pg_constraint_conrelid, BTEqualStrategyNumber,
				F_OIDEQ, ObjectIdGetDatum(parentId));
	scan = systable_beginscan(pg_constraint, ConstraintRelidTypidNameIndexId, true,
							  NULL, 1, &key);
	while ((tuple = systable_getnext(scan)) != NULL)
		clone = lappend_oid(clone, HeapTupleGetOid(tuple));
	systable_endscan(scan);

	/* Do the actual work, recursing to partitions as needed */
	clone_fk_constraints(pg_constraint, parentRel, rel, clone, cloned);

	/* We're done.  Clean up */
	heap_close(parentRel, NoLock);
	heap_close(rel, NoLock);	/* keep lock till commit */
	heap_close(pg_constraint, RowShareLock);
}

/*
 * clone_fk_constraints
 *		Recursive subroutine for CloneForeignKeyConstraints
 *
 * Clone the given list of FK constraints when a partition is attached.
 *
 * When cloning foreign keys to a partition, it may happen that equivalent
 * constraints already exist in the partition for some of them.  We can skip
 * creating a clone in that case, and instead just attach the existing
 * constraint to the one in the parent.
 *
 * This function recurses to partitions, if the new partition is partitioned;
 * of course, only do this for FKs that were actually cloned.
 */
static void
clone_fk_constraints(Relation pg_constraint, Relation parentRel,
					 Relation partRel, List *clone, List **cloned)
{
	TupleDesc	tupdesc;
	AttrNumber *attmap;
	List	   *partFKs;
	List	   *subclone = NIL;
	ListCell   *cell;

	tupdesc = RelationGetDescr(pg_constraint);

	/*
	 * The constraint key may differ, if the columns in the partition are
	 * different.  This map is used to convert them.
	 */
	attmap = convert_tuples_by_name_map(RelationGetDescr(partRel),
										RelationGetDescr(parentRel),
										gettext_noop("could not convert row type"));

	partFKs = copyObject(RelationGetFKeyList(partRel));

	foreach(cell, clone)
	{
		Oid			parentConstrOid = lfirst_oid(cell);
		Form_pg_constraint constrForm;
		HeapTuple	tuple;
		AttrNumber	conkey[INDEX_MAX_KEYS];
		AttrNumber	mapped_conkey[INDEX_MAX_KEYS];
		AttrNumber	confkey[INDEX_MAX_KEYS];
		Oid			conpfeqop[INDEX_MAX_KEYS];
		Oid			conppeqop[INDEX_MAX_KEYS];
		Oid			conffeqop[INDEX_MAX_KEYS];
		Constraint *fkconstraint;
		bool		attach_it;
		Oid			constrOid;
		ObjectAddress parentAddr,
					childAddr;
		int			nelem;
		ListCell   *cell;
		int			i;
		ArrayType  *arr;
		Datum		datum;
		bool		isnull;

		tuple = SearchSysCache1(CONSTROID, parentConstrOid);
		if (!tuple)
			elog(ERROR, "cache lookup failed for constraint %u",
				 parentConstrOid);
		constrForm = (Form_pg_constraint) GETSTRUCT(tuple);

		/* only foreign keys */
		if (constrForm->contype != CONSTRAINT_FOREIGN)
		{
			ReleaseSysCache(tuple);
			continue;
		}

		ObjectAddressSet(parentAddr, ConstraintRelationId, parentConstrOid);

		datum = fastgetattr(tuple, Anum_pg_constraint_conkey,
							tupdesc, &isnull);
		if (isnull)
			elog(ERROR, "null conkey");
		arr = DatumGetArrayTypeP(datum);
		nelem = ARR_DIMS(arr)[0];
		if (ARR_NDIM(arr) != 1 ||
			nelem < 1 ||
			nelem > INDEX_MAX_KEYS ||
			ARR_HASNULL(arr) ||
			ARR_ELEMTYPE(arr) != INT2OID)
			elog(ERROR, "conkey is not a 1-D smallint array");
		memcpy(conkey, ARR_DATA_PTR(arr), nelem * sizeof(AttrNumber));

		for (i = 0; i < nelem; i++)
			mapped_conkey[i] = attmap[conkey[i] - 1];

		datum = fastgetattr(tuple, Anum_pg_constraint_confkey,
							tupdesc, &isnull);
		if (isnull)
			elog(ERROR, "null confkey");
		arr = DatumGetArrayTypeP(datum);
		nelem = ARR_DIMS(arr)[0];
		if (ARR_NDIM(arr) != 1 ||
			nelem < 1 ||
			nelem > INDEX_MAX_KEYS ||
			ARR_HASNULL(arr) ||
			ARR_ELEMTYPE(arr) != INT2OID)
			elog(ERROR, "confkey is not a 1-D smallint array");
		memcpy(confkey, ARR_DATA_PTR(arr), nelem * sizeof(AttrNumber));

		datum = fastgetattr(tuple, Anum_pg_constraint_conpfeqop,
							tupdesc, &isnull);
		if (isnull)
			elog(ERROR, "null conpfeqop");
		arr = DatumGetArrayTypeP(datum);
		nelem = ARR_DIMS(arr)[0];
		if (ARR_NDIM(arr) != 1 ||
			nelem < 1 ||
			nelem > INDEX_MAX_KEYS ||
			ARR_HASNULL(arr) ||
			ARR_ELEMTYPE(arr) != OIDOID)
			elog(ERROR, "conpfeqop is not a 1-D OID array");
		memcpy(conpfeqop, ARR_DATA_PTR(arr), nelem * sizeof(Oid));

		datum = fastgetattr(tuple, Anum_pg_constraint_conpfeqop,
							tupdesc, &isnull);
		if (isnull)
			elog(ERROR, "null conpfeqop");
		arr = DatumGetArrayTypeP(datum);
		nelem = ARR_DIMS(arr)[0];
		if (ARR_NDIM(arr) != 1 ||
			nelem < 1 ||
			nelem > INDEX_MAX_KEYS ||
			ARR_HASNULL(arr) ||
			ARR_ELEMTYPE(arr) != OIDOID)
			elog(ERROR, "conpfeqop is not a 1-D OID array");
		memcpy(conpfeqop, ARR_DATA_PTR(arr), nelem * sizeof(Oid));

		datum = fastgetattr(tuple, Anum_pg_constraint_conppeqop,
							tupdesc, &isnull);
		if (isnull)
			elog(ERROR, "null conppeqop");
		arr = DatumGetArrayTypeP(datum);
		nelem = ARR_DIMS(arr)[0];
		if (ARR_NDIM(arr) != 1 ||
			nelem < 1 ||
			nelem > INDEX_MAX_KEYS ||
			ARR_HASNULL(arr) ||
			ARR_ELEMTYPE(arr) != OIDOID)
			elog(ERROR, "conppeqop is not a 1-D OID array");
		memcpy(conppeqop, ARR_DATA_PTR(arr), nelem * sizeof(Oid));

		datum = fastgetattr(tuple, Anum_pg_constraint_conffeqop,
							tupdesc, &isnull);
		if (isnull)
			elog(ERROR, "null conffeqop");
		arr = DatumGetArrayTypeP(datum);
		nelem = ARR_DIMS(arr)[0];
		if (ARR_NDIM(arr) != 1 ||
			nelem < 1 ||
			nelem > INDEX_MAX_KEYS ||
			ARR_HASNULL(arr) ||
			ARR_ELEMTYPE(arr) != OIDOID)
			elog(ERROR, "conffeqop is not a 1-D OID array");
		memcpy(conffeqop, ARR_DATA_PTR(arr), nelem * sizeof(Oid));

		/*
		 * Before creating a new constraint, see whether any existing FKs are
		 * fit for the purpose.  If one is, attach the parent constraint to it,
		 * and don't clone anything.  This way we avoid the expensive
		 * verification step and don't end up with a duplicate FK.  This also
		 * means we don't consider this constraint when recursing to
		 * partitions.
		 */
		attach_it = false;
		foreach(cell, partFKs)
		{
			ForeignKeyCacheInfo *fk = lfirst_node(ForeignKeyCacheInfo, cell);
			Form_pg_constraint partConstr;
			HeapTuple	partcontup;

			attach_it = true;

			/*
			 * Do some quick & easy initial checks.  If any of these fail, we
			 * cannot use this constraint, but keep looking.
			 */
			if (fk->confrelid != constrForm->confrelid || fk->nkeys != nelem)
			{
				attach_it = false;
				continue;
			}
			for (i = 0; i < nelem; i++)
			{
				if (fk->conkey[i] != mapped_conkey[i] ||
					fk->confkey[i] != confkey[i] ||
					fk->conpfeqop[i] != conpfeqop[i])
				{
					attach_it = false;
					break;
				}
			}
			if (!attach_it)
				continue;

			/*
			 * Looks good so far; do some more extensive checks.  Presumably
			 * the check for 'convalidated' could be dropped, since we don't
			 * really care about that, but let's be careful for now.
			 */
			partcontup = SearchSysCache1(CONSTROID,
										 ObjectIdGetDatum(fk->conoid));
			if (!partcontup)
				elog(ERROR, "cache lookup failed for constraint %u",
					 fk->conoid);
			partConstr = (Form_pg_constraint) GETSTRUCT(partcontup);
			if (OidIsValid(partConstr->conparentid) ||
				!partConstr->convalidated ||
				partConstr->condeferrable != constrForm->condeferrable ||
				partConstr->condeferred != constrForm->condeferred ||
				partConstr->confupdtype != constrForm->confupdtype ||
				partConstr->confdeltype != constrForm->confdeltype ||
				partConstr->confmatchtype != constrForm->confmatchtype)
			{
				ReleaseSysCache(partcontup);
				attach_it = false;
				continue;
			}

			ReleaseSysCache(partcontup);

			/* looks good!  Attach this constraint */
			ConstraintSetParentConstraint(fk->conoid,
										  HeapTupleGetOid(tuple));
			CommandCounterIncrement();
			attach_it = true;
			break;
		}

		/*
		 * If we attached to an existing constraint, there is no need to
		 * create a new one.  In fact, there's no need to recurse for this
		 * constraint to partitions, either.
		 */
		if (attach_it)
		{
			ReleaseSysCache(tuple);
			continue;
		}

		constrOid =
			CreateConstraintEntry(NameStr(constrForm->conname),
								  constrForm->connamespace,
								  CONSTRAINT_FOREIGN,
								  constrForm->condeferrable,
								  constrForm->condeferred,
								  constrForm->convalidated,
								  HeapTupleGetOid(tuple),
								  RelationGetRelid(partRel),
								  mapped_conkey,
								  nelem,
								  nelem,
								  InvalidOid,	/* not a domain constraint */
								  constrForm->conindid, /* same index */
								  constrForm->confrelid,	/* same foreign rel */
								  confkey,
								  conpfeqop,
								  conppeqop,
								  conffeqop,
								  nelem,
								  constrForm->confupdtype,
								  constrForm->confdeltype,
								  constrForm->confmatchtype,
								  NULL,
								  NULL,
								  NULL,
								  false,
								  1, false, true);
		subclone = lappend_oid(subclone, constrOid);

		ObjectAddressSet(childAddr, ConstraintRelationId, constrOid);
		recordDependencyOn(&childAddr, &parentAddr, DEPENDENCY_INTERNAL_AUTO);

		fkconstraint = makeNode(Constraint);
		/* for now this is all we need */
		fkconstraint->conname = pstrdup(NameStr(constrForm->conname));
		fkconstraint->fk_upd_action = constrForm->confupdtype;
		fkconstraint->fk_del_action = constrForm->confdeltype;
		fkconstraint->deferrable = constrForm->condeferrable;
		fkconstraint->initdeferred = constrForm->condeferred;

		createForeignKeyTriggers(partRel, constrForm->confrelid, fkconstraint,
								 constrOid, constrForm->conindid, false);

		if (cloned)
		{
			ClonedConstraint *newc;

			/*
			 * Feed back caller about the constraints we created, so that they
			 * can set up constraint verification.
			 */
			newc = palloc(sizeof(ClonedConstraint));
			newc->relid = RelationGetRelid(partRel);
			newc->refrelid = constrForm->confrelid;
			newc->conindid = constrForm->conindid;
			newc->conid = constrOid;
			newc->constraint = fkconstraint;

			*cloned = lappend(*cloned, newc);
		}

		ReleaseSysCache(tuple);
	}

	pfree(attmap);
	list_free_deep(partFKs);

	/*
	 * If the partition is partitioned, recurse to handle any constraints that
	 * were cloned.
	 */
	if (partRel->rd_rel->relkind == RELKIND_PARTITIONED_TABLE &&
		subclone != NIL)
	{
		PartitionDesc partdesc = RelationGetPartitionDesc(partRel);
		int			i;

		for (i = 0; i < partdesc->nparts; i++)
		{
			Relation	childRel;

			childRel = heap_open(partdesc->oids[i], AccessExclusiveLock);
			clone_fk_constraints(pg_constraint,
								 partRel,
								 childRel,
								 subclone,
								 cloned);
			heap_close(childRel, NoLock);	/* keep lock till commit */
		}
	}
}

/*
 * Test whether given name is currently used as a constraint name
 * for the given object (relation or domain).
 *
 * This is used to decide whether to accept a user-specified constraint name.
 * It is deliberately not the same test as ChooseConstraintName uses to decide
 * whether an auto-generated name is OK: here, we will allow it unless there
 * is an identical constraint name in use *on the same object*.
 *
 * NB: Caller should hold exclusive lock on the given object, else
 * this test can be fooled by concurrent additions.
 */
bool
ConstraintNameIsUsed(ConstraintCategory conCat, Oid objId,
					 const char *conname)
{
	bool		found;
	Relation	conDesc;
	SysScanDesc conscan;
	ScanKeyData skey[3];

	conDesc = heap_open(ConstraintRelationId, AccessShareLock);

	ScanKeyInit(&skey[0],
				Anum_pg_constraint_conrelid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum((conCat == CONSTRAINT_RELATION)
								 ? objId : InvalidOid));
	ScanKeyInit(&skey[1],
				Anum_pg_constraint_contypid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum((conCat == CONSTRAINT_DOMAIN)
								 ? objId : InvalidOid));
	ScanKeyInit(&skey[2],
				Anum_pg_constraint_conname,
				BTEqualStrategyNumber, F_NAMEEQ,
				CStringGetDatum(conname));

	conscan = systable_beginscan(conDesc, ConstraintRelidTypidNameIndexId,
								 true, NULL, 3, skey);

	/* There can be at most one matching row */
	found = (HeapTupleIsValid(systable_getnext(conscan)));

	systable_endscan(conscan);
	heap_close(conDesc, AccessShareLock);

	return found;
}

/*
 * Does any constraint of the given name exist in the given namespace?
 *
 * This is used for code that wants to match ChooseConstraintName's rule
 * that we should avoid autogenerating duplicate constraint names within a
 * namespace.
 */
bool
ConstraintNameExists(const char *conname, Oid namespaceid)
{
	bool		found;
	Relation	conDesc;
	SysScanDesc conscan;
	ScanKeyData skey[2];

	conDesc = heap_open(ConstraintRelationId, AccessShareLock);

	ScanKeyInit(&skey[0],
				Anum_pg_constraint_conname,
				BTEqualStrategyNumber, F_NAMEEQ,
				CStringGetDatum(conname));

	ScanKeyInit(&skey[1],
				Anum_pg_constraint_connamespace,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(namespaceid));

	conscan = systable_beginscan(conDesc, ConstraintNameNspIndexId, true,
								 NULL, 2, skey);

	found = (HeapTupleIsValid(systable_getnext(conscan)));

	systable_endscan(conscan);
	heap_close(conDesc, AccessShareLock);

	return found;
}

/*
 * Select a nonconflicting name for a new constraint.
 *
 * The objective here is to choose a name that is unique within the
 * specified namespace.  Postgres does not require this, but the SQL
 * spec does, and some apps depend on it.  Therefore we avoid choosing
 * default names that so conflict.
 *
 * name1, name2, and label are used the same way as for makeObjectName(),
 * except that the label can't be NULL; digits will be appended to the label
 * if needed to create a name that is unique within the specified namespace.
 *
 * 'others' can be a list of string names already chosen within the current
 * command (but not yet reflected into the catalogs); we will not choose
 * a duplicate of one of these either.
 *
 * Note: it is theoretically possible to get a collision anyway, if someone
 * else chooses the same name concurrently.  This is fairly unlikely to be
 * a problem in practice, especially if one is holding an exclusive lock on
 * the relation identified by name1.
 *
 * Returns a palloc'd string.
 */
char *
ChooseConstraintName(const char *name1, const char *name2,
					 const char *label, Oid namespaceid,
					 List *others)
{
	int			pass = 0;
	char	   *conname = NULL;
	char		modlabel[NAMEDATALEN];
	Relation	conDesc;
	SysScanDesc conscan;
	ScanKeyData skey[2];
	bool		found;
	ListCell   *l;

	conDesc = heap_open(ConstraintRelationId, AccessShareLock);

	/* try the unmodified label first */
	StrNCpy(modlabel, label, sizeof(modlabel));

	for (;;)
	{
		conname = makeObjectName(name1, name2, modlabel);

		found = false;

		foreach(l, others)
		{
			if (strcmp((char *) lfirst(l), conname) == 0)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			ScanKeyInit(&skey[0],
						Anum_pg_constraint_conname,
						BTEqualStrategyNumber, F_NAMEEQ,
						CStringGetDatum(conname));

			ScanKeyInit(&skey[1],
						Anum_pg_constraint_connamespace,
						BTEqualStrategyNumber, F_OIDEQ,
						ObjectIdGetDatum(namespaceid));

			conscan = systable_beginscan(conDesc, ConstraintNameNspIndexId, true,
										 NULL, 2, skey);

			found = (HeapTupleIsValid(systable_getnext(conscan)));

			systable_endscan(conscan);
		}

		if (!found)
			break;

		/* found a conflict, so try a new name component */
		pfree(conname);
		snprintf(modlabel, sizeof(modlabel), "%s%d", label, ++pass);
	}

	heap_close(conDesc, AccessShareLock);

	return conname;
}

/*
 * Delete a single constraint record.
 */
void
RemoveConstraintById(Oid conId)
{
	Relation	conDesc;
	HeapTuple	tup;
	Form_pg_constraint con;

	conDesc = heap_open(ConstraintRelationId, RowExclusiveLock);

	tup = SearchSysCache1(CONSTROID, ObjectIdGetDatum(conId));
	if (!HeapTupleIsValid(tup)) /* should not happen */
		elog(ERROR, "cache lookup failed for constraint %u", conId);
	con = (Form_pg_constraint) GETSTRUCT(tup);

	/*
	 * Special processing depending on what the constraint is for.
	 */
	if (OidIsValid(con->conrelid))
	{
		Relation	rel;

		/*
		 * If the constraint is for a relation, open and exclusive-lock the
		 * relation it's for.
		 */
		rel = heap_open(con->conrelid, AccessExclusiveLock);

		/*
		 * We need to update the relcheck count if it is a check constraint
		 * being dropped.  This update will force backends to rebuild relcache
		 * entries when we commit.
		 */
		if (con->contype == CONSTRAINT_CHECK)
		{
			Relation	pgrel;
			HeapTuple	relTup;
			Form_pg_class classForm;

			pgrel = heap_open(RelationRelationId, RowExclusiveLock);
			relTup = SearchSysCacheCopy1(RELOID,
										 ObjectIdGetDatum(con->conrelid));
			if (!HeapTupleIsValid(relTup))
				elog(ERROR, "cache lookup failed for relation %u",
					 con->conrelid);
			classForm = (Form_pg_class) GETSTRUCT(relTup);

			if (classForm->relchecks == 0)	/* should not happen */
				elog(ERROR, "relation \"%s\" has relchecks = 0",
					 RelationGetRelationName(rel));
			classForm->relchecks--;

			CatalogTupleUpdate(pgrel, &relTup->t_self, relTup);

			heap_freetuple(relTup);

			heap_close(pgrel, RowExclusiveLock);
		}

		/* Keep lock on constraint's rel until end of xact */
		heap_close(rel, NoLock);
	}
	else if (OidIsValid(con->contypid))
	{
		/*
		 * XXX for now, do nothing special when dropping a domain constraint
		 *
		 * Probably there should be some form of locking on the domain type,
		 * but we have no such concept at the moment.
		 */
	}
	else
		elog(ERROR, "constraint %u is not of a known type", conId);

	/* Fry the constraint itself */
	CatalogTupleDelete(conDesc, &tup->t_self);

	/* Clean up */
	ReleaseSysCache(tup);
	heap_close(conDesc, RowExclusiveLock);
}

/*
 * RenameConstraintById
 *		Rename a constraint.
 *
 * Note: this isn't intended to be a user-exposed function; it doesn't check
 * permissions etc.  Currently this is only invoked when renaming an index
 * that is associated with a constraint, but it's made a little more general
 * than that with the expectation of someday having ALTER TABLE RENAME
 * CONSTRAINT.
 */
void
RenameConstraintById(Oid conId, const char *newname)
{
	Relation	conDesc;
	HeapTuple	tuple;
	Form_pg_constraint con;

	conDesc = heap_open(ConstraintRelationId, RowExclusiveLock);

	tuple = SearchSysCacheCopy1(CONSTROID, ObjectIdGetDatum(conId));
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "cache lookup failed for constraint %u", conId);
	con = (Form_pg_constraint) GETSTRUCT(tuple);

	/*
	 * For user-friendliness, check whether the name is already in use.
	 */
	if (OidIsValid(con->conrelid) &&
		ConstraintNameIsUsed(CONSTRAINT_RELATION,
							 con->conrelid,
							 newname))
		ereport(ERROR,
				(errcode(ERRCODE_DUPLICATE_OBJECT),
				 errmsg("constraint \"%s\" for relation \"%s\" already exists",
						newname, get_rel_name(con->conrelid))));
	if (OidIsValid(con->contypid) &&
		ConstraintNameIsUsed(CONSTRAINT_DOMAIN,
							 con->contypid,
							 newname))
		ereport(ERROR,
				(errcode(ERRCODE_DUPLICATE_OBJECT),
				 errmsg("constraint \"%s\" for domain %s already exists",
						newname, format_type_be(con->contypid))));

	/* OK, do the rename --- tuple is a copy, so OK to scribble on it */
	namestrcpy(&(con->conname), newname);

	CatalogTupleUpdate(conDesc, &tuple->t_self, tuple);

	InvokeObjectPostAlterHook(ConstraintRelationId, conId, 0);

	heap_freetuple(tuple);
	heap_close(conDesc, RowExclusiveLock);
}

/*
 * AlterConstraintNamespaces
 *		Find any constraints belonging to the specified object,
 *		and move them to the specified new namespace.
 *
 * isType indicates whether the owning object is a type or a relation.
 */
void
AlterConstraintNamespaces(Oid ownerId, Oid oldNspId,
						  Oid newNspId, bool isType, ObjectAddresses *objsMoved)
{
	Relation	conRel;
	ScanKeyData key[2];
	SysScanDesc scan;
	HeapTuple	tup;

	conRel = heap_open(ConstraintRelationId, RowExclusiveLock);

	ScanKeyInit(&key[0],
				Anum_pg_constraint_conrelid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(isType ? InvalidOid : ownerId));
	ScanKeyInit(&key[1],
				Anum_pg_constraint_contypid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(isType ? ownerId : InvalidOid));

	scan = systable_beginscan(conRel, ConstraintRelidTypidNameIndexId, true,
							  NULL, 2, key);

	while (HeapTupleIsValid((tup = systable_getnext(scan))))
	{
		Form_pg_constraint conform = (Form_pg_constraint) GETSTRUCT(tup);
		ObjectAddress thisobj;

		thisobj.classId = ConstraintRelationId;
		thisobj.objectId = HeapTupleGetOid(tup);
		thisobj.objectSubId = 0;

		if (object_address_present(&thisobj, objsMoved))
			continue;

		/* Don't update if the object is already part of the namespace */
		if (conform->connamespace == oldNspId && oldNspId != newNspId)
		{
			tup = heap_copytuple(tup);
			conform = (Form_pg_constraint) GETSTRUCT(tup);

			conform->connamespace = newNspId;

			CatalogTupleUpdate(conRel, &tup->t_self, tup);

			/*
			 * Note: currently, the constraint will not have its own
			 * dependency on the namespace, so we don't need to do
			 * changeDependencyFor().
			 */
		}

		InvokeObjectPostAlterHook(ConstraintRelationId, thisobj.objectId, 0);

		add_exact_object_address(&thisobj, objsMoved);
	}

	systable_endscan(scan);

	heap_close(conRel, RowExclusiveLock);
}

/*
 * ConstraintSetParentConstraint
 *		Set a partition's constraint as child of its parent table's
 *
 * This updates the constraint's pg_constraint row to show it as inherited, and
 * add a dependency to the parent so that it cannot be removed on its own.
 */
void
ConstraintSetParentConstraint(Oid childConstrId, Oid parentConstrId)
{
	Relation	constrRel;
	Form_pg_constraint constrForm;
	HeapTuple	tuple,
				newtup;
	ObjectAddress depender;
	ObjectAddress referenced;

	constrRel = heap_open(ConstraintRelationId, RowExclusiveLock);
	tuple = SearchSysCache1(CONSTROID, ObjectIdGetDatum(childConstrId));
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "cache lookup failed for constraint %u", childConstrId);
	newtup = heap_copytuple(tuple);
	constrForm = (Form_pg_constraint) GETSTRUCT(newtup);
	if (OidIsValid(parentConstrId))
	{
		constrForm->conislocal = false;
		constrForm->coninhcount++;
		constrForm->conparentid = parentConstrId;

		CatalogTupleUpdate(constrRel, &tuple->t_self, newtup);

		ObjectAddressSet(referenced, ConstraintRelationId, parentConstrId);
		ObjectAddressSet(depender, ConstraintRelationId, childConstrId);

		recordDependencyOn(&depender, &referenced, DEPENDENCY_INTERNAL_AUTO);
	}
	else
	{
		constrForm->coninhcount--;
		if (constrForm->coninhcount <= 0)
			constrForm->conislocal = true;
		constrForm->conparentid = InvalidOid;

		deleteDependencyRecordsForClass(ConstraintRelationId, childConstrId,
										ConstraintRelationId,
										DEPENDENCY_INTERNAL_AUTO);
		CatalogTupleUpdate(constrRel, &tuple->t_self, newtup);
	}

	ReleaseSysCache(tuple);
	heap_close(constrRel, RowExclusiveLock);
}


/*
 * get_relation_constraint_oid
 *		Find a constraint on the specified relation with the specified name.
 *		Returns constraint's OID.
 */
Oid
get_relation_constraint_oid(Oid relid, const char *conname, bool missing_ok)
{
	Relation	pg_constraint;
	HeapTuple	tuple;
	SysScanDesc scan;
	ScanKeyData skey[3];
	Oid			conOid = InvalidOid;

	pg_constraint = heap_open(ConstraintRelationId, AccessShareLock);

	ScanKeyInit(&skey[0],
				Anum_pg_constraint_conrelid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(relid));
	ScanKeyInit(&skey[1],
				Anum_pg_constraint_contypid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(InvalidOid));
	ScanKeyInit(&skey[2],
				Anum_pg_constraint_conname,
				BTEqualStrategyNumber, F_NAMEEQ,
				CStringGetDatum(conname));

	scan = systable_beginscan(pg_constraint, ConstraintRelidTypidNameIndexId, true,
							  NULL, 3, skey);

	/* There can be at most one matching row */
	if (HeapTupleIsValid(tuple = systable_getnext(scan)))
		conOid = HeapTupleGetOid(tuple);

	systable_endscan(scan);

	/* If no such constraint exists, complain */
	if (!OidIsValid(conOid) && !missing_ok)
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_OBJECT),
				 errmsg("constraint \"%s\" for table \"%s\" does not exist",
						conname, get_rel_name(relid))));

	heap_close(pg_constraint, AccessShareLock);

	return conOid;
}

/*
 * get_relation_constraint_attnos
 *		Find a constraint on the specified relation with the specified name
 *		and return the constrained columns.
 *
 * Returns a Bitmapset of the column attnos of the constrained columns, with
 * attnos being offset by FirstLowInvalidHeapAttributeNumber so that system
 * columns can be represented.
 *
 * *constraintOid is set to the OID of the constraint, or InvalidOid on
 * failure.
 */
Bitmapset *
get_relation_constraint_attnos(Oid relid, const char *conname,
							   bool missing_ok, Oid *constraintOid)
{
	Bitmapset  *conattnos = NULL;
	Relation	pg_constraint;
	HeapTuple	tuple;
	SysScanDesc scan;
	ScanKeyData skey[3];

	/* Set *constraintOid, to avoid complaints about uninitialized vars */
	*constraintOid = InvalidOid;

	pg_constraint = heap_open(ConstraintRelationId, AccessShareLock);

	ScanKeyInit(&skey[0],
				Anum_pg_constraint_conrelid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(relid));
	ScanKeyInit(&skey[1],
				Anum_pg_constraint_contypid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(InvalidOid));
	ScanKeyInit(&skey[2],
				Anum_pg_constraint_conname,
				BTEqualStrategyNumber, F_NAMEEQ,
				CStringGetDatum(conname));

	scan = systable_beginscan(pg_constraint, ConstraintRelidTypidNameIndexId, true,
							  NULL, 3, skey);

	/* There can be at most one matching row */
	if (HeapTupleIsValid(tuple = systable_getnext(scan)))
	{
		Datum		adatum;
		bool		isNull;

		*constraintOid = HeapTupleGetOid(tuple);

		/* Extract the conkey array, ie, attnums of constrained columns */
		adatum = heap_getattr(tuple, Anum_pg_constraint_conkey,
							  RelationGetDescr(pg_constraint), &isNull);
		if (!isNull)
		{
			ArrayType  *arr;
			int			numcols;
			int16	   *attnums;
			int			i;

			arr = DatumGetArrayTypeP(adatum);	/* ensure not toasted */
			numcols = ARR_DIMS(arr)[0];
			if (ARR_NDIM(arr) != 1 ||
				numcols < 0 ||
				ARR_HASNULL(arr) ||
				ARR_ELEMTYPE(arr) != INT2OID)
				elog(ERROR, "conkey is not a 1-D smallint array");
			attnums = (int16 *) ARR_DATA_PTR(arr);

			/* Construct the result value */
			for (i = 0; i < numcols; i++)
			{
				conattnos = bms_add_member(conattnos,
										   attnums[i] - FirstLowInvalidHeapAttributeNumber);
			}
		}
	}

	systable_endscan(scan);

	/* If no such constraint exists, complain */
	if (!OidIsValid(*constraintOid) && !missing_ok)
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_OBJECT),
				 errmsg("constraint \"%s\" for table \"%s\" does not exist",
						conname, get_rel_name(relid))));

	heap_close(pg_constraint, AccessShareLock);

	return conattnos;
}

/*
 * Return the OID of the constraint associated with the given index in the
 * given relation; or InvalidOid if no such index is catalogued.
 */
Oid
get_relation_idx_constraint_oid(Oid relationId, Oid indexId)
{
	Relation	pg_constraint;
	SysScanDesc scan;
	ScanKeyData key;
	HeapTuple	tuple;
	Oid			constraintId = InvalidOid;

	pg_constraint = heap_open(ConstraintRelationId, AccessShareLock);

	ScanKeyInit(&key,
				Anum_pg_constraint_conrelid,
				BTEqualStrategyNumber,
				F_OIDEQ,
				ObjectIdGetDatum(relationId));
	scan = systable_beginscan(pg_constraint, ConstraintRelidTypidNameIndexId,
							  true, NULL, 1, &key);
	while ((tuple = systable_getnext(scan)) != NULL)
	{
		Form_pg_constraint constrForm;

		constrForm = (Form_pg_constraint) GETSTRUCT(tuple);
		if (constrForm->conindid == indexId)
		{
			constraintId = HeapTupleGetOid(tuple);
			break;
		}
	}
	systable_endscan(scan);

	heap_close(pg_constraint, AccessShareLock);
	return constraintId;
}

/*
 * get_domain_constraint_oid
 *		Find a constraint on the specified domain with the specified name.
 *		Returns constraint's OID.
 */
Oid
get_domain_constraint_oid(Oid typid, const char *conname, bool missing_ok)
{
	Relation	pg_constraint;
	HeapTuple	tuple;
	SysScanDesc scan;
	ScanKeyData skey[3];
	Oid			conOid = InvalidOid;

	pg_constraint = heap_open(ConstraintRelationId, AccessShareLock);

	ScanKeyInit(&skey[0],
				Anum_pg_constraint_conrelid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(InvalidOid));
	ScanKeyInit(&skey[1],
				Anum_pg_constraint_contypid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(typid));
	ScanKeyInit(&skey[2],
				Anum_pg_constraint_conname,
				BTEqualStrategyNumber, F_NAMEEQ,
				CStringGetDatum(conname));

	scan = systable_beginscan(pg_constraint, ConstraintRelidTypidNameIndexId, true,
							  NULL, 3, skey);

	/* There can be at most one matching row */
	if (HeapTupleIsValid(tuple = systable_getnext(scan)))
		conOid = HeapTupleGetOid(tuple);

	systable_endscan(scan);

	/* If no such constraint exists, complain */
	if (!OidIsValid(conOid) && !missing_ok)
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_OBJECT),
				 errmsg("constraint \"%s\" for domain %s does not exist",
						conname, format_type_be(typid))));

	heap_close(pg_constraint, AccessShareLock);

	return conOid;
}

/*
 * get_primary_key_attnos
 *		Identify the columns in a relation's primary key, if any.
 *
 * Returns a Bitmapset of the column attnos of the primary key's columns,
 * with attnos being offset by FirstLowInvalidHeapAttributeNumber so that
 * system columns can be represented.
 *
 * If there is no primary key, return NULL.  We also return NULL if the pkey
 * constraint is deferrable and deferrableOk is false.
 *
 * *constraintOid is set to the OID of the pkey constraint, or InvalidOid
 * on failure.
 */
Bitmapset *
get_primary_key_attnos(Oid relid, bool deferrableOk, Oid *constraintOid)
{
	Bitmapset  *pkattnos = NULL;
	Relation	pg_constraint;
	HeapTuple	tuple;
	SysScanDesc scan;
	ScanKeyData skey[1];

	/* Set *constraintOid, to avoid complaints about uninitialized vars */
	*constraintOid = InvalidOid;

	/* Scan pg_constraint for constraints of the target rel */
	pg_constraint = heap_open(ConstraintRelationId, AccessShareLock);

	ScanKeyInit(&skey[0],
				Anum_pg_constraint_conrelid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(relid));

	scan = systable_beginscan(pg_constraint, ConstraintRelidTypidNameIndexId, true,
							  NULL, 1, skey);

	while (HeapTupleIsValid(tuple = systable_getnext(scan)))
	{
		Form_pg_constraint con = (Form_pg_constraint) GETSTRUCT(tuple);
		Datum		adatum;
		bool		isNull;
		ArrayType  *arr;
		int16	   *attnums;
		int			numkeys;
		int			i;

		/* Skip constraints that are not PRIMARY KEYs */
		if (con->contype != CONSTRAINT_PRIMARY)
			continue;

		/*
		 * If the primary key is deferrable, but we've been instructed to
		 * ignore deferrable constraints, then we might as well give up
		 * searching, since there can only be a single primary key on a table.
		 */
		if (con->condeferrable && !deferrableOk)
			break;

		/* Extract the conkey array, ie, attnums of PK's columns */
		adatum = heap_getattr(tuple, Anum_pg_constraint_conkey,
							  RelationGetDescr(pg_constraint), &isNull);
		if (isNull)
			elog(ERROR, "null conkey for constraint %u",
				 HeapTupleGetOid(tuple));
		arr = DatumGetArrayTypeP(adatum);	/* ensure not toasted */
		numkeys = ARR_DIMS(arr)[0];
		if (ARR_NDIM(arr) != 1 ||
			numkeys < 0 ||
			ARR_HASNULL(arr) ||
			ARR_ELEMTYPE(arr) != INT2OID)
			elog(ERROR, "conkey is not a 1-D smallint array");
		attnums = (int16 *) ARR_DATA_PTR(arr);

		/* Construct the result value */
		for (i = 0; i < numkeys; i++)
		{
			pkattnos = bms_add_member(pkattnos,
									  attnums[i] - FirstLowInvalidHeapAttributeNumber);
		}
		*constraintOid = HeapTupleGetOid(tuple);

		/* No need to search further */
		break;
	}

	systable_endscan(scan);

	heap_close(pg_constraint, AccessShareLock);

	return pkattnos;
}

/*
 * Determine whether a relation can be proven functionally dependent on
 * a set of grouping columns.  If so, return true and add the pg_constraint
 * OIDs of the constraints needed for the proof to the *constraintDeps list.
 *
 * grouping_columns is a list of grouping expressions, in which columns of
 * the rel of interest are Vars with the indicated varno/varlevelsup.
 *
 * Currently we only check to see if the rel has a primary key that is a
 * subset of the grouping_columns.  We could also use plain unique constraints
 * if all their columns are known not null, but there's a problem: we need
 * to be able to represent the not-null-ness as part of the constraints added
 * to *constraintDeps.  FIXME whenever not-null constraints get represented
 * in pg_constraint.
 */
bool
check_functional_grouping(Oid relid,
						  Index varno, Index varlevelsup,
						  List *grouping_columns,
						  List **constraintDeps)
{
	Bitmapset  *pkattnos;
	Bitmapset  *groupbyattnos;
	Oid			constraintOid;
	ListCell   *gl;

	/* If the rel has no PK, then we can't prove functional dependency */
	pkattnos = get_primary_key_attnos(relid, false, &constraintOid);
	if (pkattnos == NULL)
		return false;

	/* Identify all the rel's columns that appear in grouping_columns */
	groupbyattnos = NULL;
	foreach(gl, grouping_columns)
	{
		Var		   *gvar = (Var *) lfirst(gl);

		if (IsA(gvar, Var) &&
			gvar->varno == varno &&
			gvar->varlevelsup == varlevelsup)
			groupbyattnos = bms_add_member(groupbyattnos,
										   gvar->varattno - FirstLowInvalidHeapAttributeNumber);
	}

	if (bms_is_subset(pkattnos, groupbyattnos))
	{
		/* The PK is a subset of grouping_columns, so we win */
		*constraintDeps = lappend_oid(*constraintDeps, constraintOid);
		return true;
	}

	return false;
}
