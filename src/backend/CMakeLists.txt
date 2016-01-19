include_directories(
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

set(postgres_include_catalog "${PROJECT_SOURCE_DIR}/src/include/catalog")

set(gen_fmgrtab_depend
	utils/Gen_fmgrtab.pl
	${CMAKE_CURRENT_SOURCE_DIR}/catalog/Catalog.pm
	${postgres_include_catalog}/pg_proc.h
)
set(gen_fmgrtab_output
	${PROJECT_SOURCE_DIR}/src/include/utils/fmgroids.h
	${CMAKE_CURRENT_SOURCE_DIR}/utils/fmgrtab.c
)

add_custom_command(
	MAIN_DEPENDENCY ${gen_fmgrtab_depend}
	OUTPUT ${gen_fmgrtab_output}
	COMMAND ${PERL_EXECUTABLE} -I${CMAKE_CURRENT_SOURCE_DIR}/catalog Gen_fmgrtab.pl ${postgres_include_catalog}/pg_proc.h
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/utils/fmgroids.h ${PROJECT_SOURCE_DIR}/src/include/utils/fmgroids.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

set(gen_probe_depend
	${CMAKE_CURRENT_SOURCE_DIR}/utils/Gen_dummy_probes.sed
	${CMAKE_CURRENT_SOURCE_DIR}/utils/probes.d
)

set(gen_probe_output
	${PROJECT_SOURCE_DIR}/src/include/utils/probes.h
)

add_custom_command(
	MAIN_DEPENDENCY ${gen_probe_depend}
	OUTPUT ${gen_probe_output}
	COMMAND sed -f Gen_dummy_probes.sed probes.d >  ${gen_probe_output}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

if(WIN32)
	#TODO: maybe need test CYGWIN also
	set(dynloader_SRCS port/dynloader/win32.c)
	set(dynloader_H port/dynloader/win32.h)
elseif(APPLE)
	set(dynloader_SRCS port/dynloader/darwin.c)
	set(dynloader_H port/dynloader/darwin.h)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(dynloader_SRCS port/dynloader/linux.c)
	set(dynloader_H port/dynloader/linux.h)
elseif(CMAKE_SYSTEM_NAME STREQUAL "HP-UX")
	set(dynloader_SRCS port/dynloader/hpux.c)
	set(dynloader_H port/dynloader/hpux.h)
#TODO: FreeBSD, OpenBSD, NetBSD is equal loader
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	set(dynloader_SRCS port/dynloader/freebsd.c)
	set(dynloader_H port/dynloader/freebsd.h)
elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
	set(dynloader_SRCS port/dynloader/openbsd.c)
	set(dynloader_H port/dynloader/openbsd.h)
elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
	set(dynloader_SRCS port/dynloader/netbsd.c)
	set(dynloader_H port/dynloader/netbsd.h)
else(WIN32)
	message(WARNING "${CMAKE_SYSTEM_NAME}")
endif(WIN32)

add_custom_command(
	MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${dynloader_H}"
	OUTPUT "${PROJECT_SOURCE_DIR}/src/include/dynloader.h"
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${dynloader_H} ${PROJECT_SOURCE_DIR}/src/include/dynloader.h
)



set(access_SRCS
	${gen_fmgrtab_output}
	${gen_probe_output}
	${PROJECT_SOURCE_DIR}/src/include/dynloader.h

	access/brin/brin.c
	access/brin/brin_pageops.c
	access/brin/brin_revmap.c
	access/brin/brin_tuple.c
	access/brin/brin_xlog.c
	access/brin/brin_minmax.c
	access/brin/brin_inclusion.c

	access/common/heaptuple.c
	access/common/indextuple.c
	access/common/printtup.c
	access/common/reloptions.c
	access/common/scankey.c
	access/common/tupconvert.c
	access/common/tupdesc.c

	access/gin/ginutil.c
	access/gin/gininsert.c
	access/gin/ginxlog.c
	access/gin/ginentrypage.c
	access/gin/gindatapage.c
	access/gin/ginbtree.c
	access/gin/ginscan.c
	access/gin/ginget.c
	access/gin/ginvacuum.c
	access/gin/ginarrayproc.c
	access/gin/ginbulk.c
	access/gin/ginfast.c
	access/gin/ginpostinglist.c
	access/gin/ginlogic.c

	access/gist/gist.c
	access/gist/gistutil.c
	access/gist/gistxlog.c
	access/gist/gistvacuum.c
	access/gist/gistget.c
	access/gist/gistscan.c
	access/gist/gistproc.c
	access/gist/gistsplit.c
	access/gist/gistbuild.c
	access/gist/gistbuildbuffers.c

	access/hash/hash.c
	access/hash/hashfunc.c
	access/hash/hashinsert.c
	access/hash/hashovfl.c
	access/hash/hashpage.c
	access/hash/hashscan.c
	access/hash/hashsearch.c
	access/hash/hashsort.c
	access/hash/hashutil.c

	access/heap/heapam.c
	access/heap/hio.c
	access/heap/pruneheap.c
	access/heap/rewriteheap.c
	access/heap/syncscan.c
	access/heap/tuptoaster.c
	access/heap/visibilitymap.c

	access/index/genam.c
	access/index/indexam.c

	access/nbtree/nbtcompare.c
	access/nbtree/nbtinsert.c
	access/nbtree/nbtpage.c
	access/nbtree/nbtree.c
	access/nbtree/nbtsearch.c
	access/nbtree/nbtutils.c
	access/nbtree/nbtsort.c
	access/nbtree/nbtxlog.c

	access/rmgrdesc/brindesc.c
	access/rmgrdesc/clogdesc.c
	access/rmgrdesc/committsdesc.c
	access/rmgrdesc/dbasedesc.c
	access/rmgrdesc/gindesc.c
	access/rmgrdesc/gistdesc.c
	access/rmgrdesc/hashdesc.c
	access/rmgrdesc/heapdesc.c
	access/rmgrdesc/mxactdesc.c
	access/rmgrdesc/nbtdesc.c
	access/rmgrdesc/relmapdesc.c
	access/rmgrdesc/replorigindesc.c
	access/rmgrdesc/seqdesc.c
	access/rmgrdesc/smgrdesc.c
	access/rmgrdesc/spgdesc.c
	access/rmgrdesc/standbydesc.c
	access/rmgrdesc/tblspcdesc.c
	access/rmgrdesc/xactdesc.c
	access/rmgrdesc/xlogdesc.c

	access/spgist/spgutils.c
	access/spgist/spginsert.c
	access/spgist/spgscan.c
	access/spgist/spgvacuum.c
	access/spgist/spgdoinsert.c
	access/spgist/spgxlog.c
	access/spgist/spgtextproc.c
	access/spgist/spgquadtreeproc.c
	access/spgist/spgkdtreeproc.c

	access/tablesample/bernoulli.c
	access/tablesample/system.c
	access/tablesample/tablesample.c

	access/transam/clog.c
	access/transam/commit_ts.c
	access/transam/multixact.c
	access/transam/parallel.c
	access/transam/rmgr.c
	access/transam/slru.c
	access/transam/subtrans.c
	access/transam/timeline.c
	access/transam/transam.c
	access/transam/twophase.c
	access/transam/twophase_rmgr.c
	access/transam/varsup.c
	access/transam/xact.c
	access/transam/xlog.c
	access/transam/xlogarchive.c
	access/transam/xlogfuncs.c
	access/transam/xloginsert.c
	access/transam/xlogreader.c
	access/transam/xlogutils.c
)

bison_target(BOOTSTRAP_PARSER bootstrap/bootparse.y ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap/bootparse.c)
flex_target(BOOTSTRAP_SCANNER bootstrap/bootscanner.l  ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap/bootscanner.c)
add_flex_bison_dependency(BOOTSTRAP_SCANNER BOOTSTRAP_PARSER)

set_source_files_properties(bootstrap/bootparse.c PROPERTIES OBJECT_DEPENDS ${FLEX_BOOTSTRAP_SCANNER_OUTPUTS})

set(bootstrap_SRCS
	bootstrap/bootstrap.c
	${BISON_BOOTSTRAP_PARSER_OUTPUTS}
)


set(POSTGRES_BKI_SRCS
	${postgres_include_catalog}/pg_proc.h
	${postgres_include_catalog}/pg_type.h
	${postgres_include_catalog}/pg_attribute.h
	${postgres_include_catalog}/pg_class.h
	${postgres_include_catalog}/pg_attrdef.h
	${postgres_include_catalog}/pg_constraint.h
	${postgres_include_catalog}/pg_inherits.h
	${postgres_include_catalog}/pg_index.h
	${postgres_include_catalog}/pg_operator.h
	${postgres_include_catalog}/pg_opfamily.h
	${postgres_include_catalog}/pg_opclass.h
	${postgres_include_catalog}/pg_am.h
	${postgres_include_catalog}/pg_amop.h
	${postgres_include_catalog}/pg_amproc.h
	${postgres_include_catalog}/pg_language.h
	${postgres_include_catalog}/pg_largeobject_metadata.h
	${postgres_include_catalog}/pg_largeobject.h
	${postgres_include_catalog}/pg_aggregate.h
	${postgres_include_catalog}/pg_statistic.h
	${postgres_include_catalog}/pg_rewrite.h
	${postgres_include_catalog}/pg_trigger.h
	${postgres_include_catalog}/pg_event_trigger.h
	${postgres_include_catalog}/pg_description.h
	${postgres_include_catalog}/pg_cast.h
	${postgres_include_catalog}/pg_enum.h
	${postgres_include_catalog}/pg_namespace.h
	${postgres_include_catalog}/pg_conversion.h
	${postgres_include_catalog}/pg_depend.h
	${postgres_include_catalog}/pg_database.h
	${postgres_include_catalog}/pg_db_role_setting.h
	${postgres_include_catalog}/pg_tablespace.h
	${postgres_include_catalog}/pg_pltemplate.h
	${postgres_include_catalog}/pg_authid.h
	${postgres_include_catalog}/pg_auth_members.h
	${postgres_include_catalog}/pg_shdepend.h
	${postgres_include_catalog}/pg_shdescription.h
	${postgres_include_catalog}/pg_ts_config.h
	${postgres_include_catalog}/pg_ts_config_map.h
	${postgres_include_catalog}/pg_ts_dict.h
	${postgres_include_catalog}/pg_ts_parser.h
	${postgres_include_catalog}/pg_ts_template.h
	${postgres_include_catalog}/pg_extension.h
	${postgres_include_catalog}/pg_foreign_data_wrapper.h
	${postgres_include_catalog}/pg_foreign_server.h
	${postgres_include_catalog}/pg_user_mapping.h
	${postgres_include_catalog}/pg_foreign_table.h
	${postgres_include_catalog}/pg_policy.h
	${postgres_include_catalog}/pg_replication_origin.h
	${postgres_include_catalog}/pg_default_acl.h
	${postgres_include_catalog}/pg_seclabel.h
	${postgres_include_catalog}/pg_shseclabel.h
	${postgres_include_catalog}/pg_collation.h
	${postgres_include_catalog}/pg_range.h
	${postgres_include_catalog}/pg_transform.h
	${postgres_include_catalog}/toasting.h
	${postgres_include_catalog}/indexing.h
)

set(catalog_SRCS
	catalog/catalog.c
	catalog/dependency.c
	catalog/heap.c
	catalog/index.c
	catalog/indexing.c
	catalog/namespace.c
	catalog/aclchk.c
	catalog/objectaccess.c
	catalog/objectaddress.c
	catalog/pg_aggregate.c
	catalog/pg_collation.c
	catalog/pg_constraint.c
	catalog/pg_conversion.c
	catalog/pg_depend.c
	catalog/pg_enum.c
	catalog/pg_inherits.c
	catalog/pg_largeobject.c
	catalog/pg_namespace.c
	catalog/pg_operator.c
	catalog/pg_proc.c
	catalog/pg_range.c
	catalog/pg_db_role_setting.c
	catalog/pg_shdepend.c
	catalog/pg_type.c
	catalog/storage.c
	catalog/toasting.c
)

set(catalog_include
	"-I${PROJECT_SOURCE_DIR}/src/backend/catalog"
	"-I${PROJECT_SOURCE_DIR}/src/include/catalog"
)

add_custom_command(
	DEPENDS ${POSTGRES_BKI_SRCS}
	OUTPUT ${PROJECT_SOURCE_DIR}/src/include/catalog/schemapg.h
	COMMAND ${PERL_EXECUTABLE} genbki.pl ${catalog_include} --set-version=${POSTGRES_MAJOR_VERSION}.${POSTGRES_MINOR_VERSION} ${POSTGRES_BKI_SRCS}
	COMMAND ${CMAKE_COMMAND} -E copy schemapg.h ${PROJECT_SOURCE_DIR}/src/include/catalog/schemapg.h
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/catalog
)

set_source_files_properties(utils/cache/relcache.c
	PROPERTIES OBJECT_DEPENDS ${PROJECT_SOURCE_DIR}/src/include/catalog/schemapg.h)

set(commands_SRCS
	commands/aggregatecmds.c
	commands/alter.c
	commands/analyze.c
	commands/async.c
	commands/cluster.c
	commands/comment.c
	commands/collationcmds.c
	commands/constraint.c
	commands/conversioncmds.c
	commands/copy.c
	commands/createas.c
	commands/dbcommands.c
	commands/define.c
	commands/discard.c
	commands/dropcmds.c
	commands/event_trigger.c
	commands/explain.c
	commands/extension.c
	commands/foreigncmds.c
	commands/functioncmds.c
	commands/indexcmds.c
	commands/lockcmds.c
	commands/matview.c
	commands/operatorcmds.c
	commands/opclasscmds.c
	commands/policy.c
	commands/portalcmds.c
	commands/prepare.c
	commands/proclang.c
	commands/schemacmds.c
	commands/seclabel.c
	commands/sequence.c
	commands/tablecmds.c
	commands/tablespace.c
	commands/trigger.c
	commands/tsearchcmds.c
	commands/typecmds.c
	commands/user.c
	commands/vacuum.c
	commands/vacuumlazy.c
	commands/variable.c
	commands/view.c
)

set(executor_SRCS
	executor/execAmi.c
	executor/execCurrent.c
	executor/execGrouping.c
	executor/execIndexing.c
	executor/execJunk.c
	executor/execMain.c
	executor/execProcnode.c
	executor/execQual.c
	executor/execScan.c
	executor/execTuples.c
	executor/execUtils.c
	executor/functions.c
	executor/instrument.c
	executor/nodeAppend.c
	executor/nodeAgg.c
	executor/nodeBitmapAnd.c
	executor/nodeBitmapOr.c
	executor/nodeBitmapHeapscan.c
	executor/nodeBitmapIndexscan.c
	executor/nodeCustom.c
	executor/nodeHash.c
	executor/nodeHashjoin.c
	executor/nodeIndexscan.c
	executor/nodeIndexonlyscan.c
	executor/nodeLimit.c
	executor/nodeLockRows.c
	executor/nodeMaterial.c
	executor/nodeMergeAppend.c
	executor/nodeMergejoin.c
	executor/nodeModifyTable.c
	executor/nodeNestloop.c
	executor/nodeFunctionscan.c
	executor/nodeRecursiveunion.c
	executor/nodeResult.c
	executor/nodeSamplescan.c
	executor/nodeSeqscan.c
	executor/nodeSetOp.c
	executor/nodeSort.c
	executor/nodeUnique.c
	executor/nodeValuesscan.c
	executor/nodeCtescan.c
	executor/nodeWorktablescan.c
	executor/nodeGroup.c
	executor/nodeSubplan.c
	executor/nodeSubqueryscan.c
	executor/nodeTidscan.c
	executor/nodeForeignscan.c
	executor/nodeWindowAgg.c
	executor/tstoreReceiver.c
	executor/spi.c
)

set(lib_SRCS
	lib/binaryheap.c
	lib/bipartite_match.c
	lib/hyperloglog.c
	lib/ilist.c
	lib/pairingheap.c
	lib/rbtree.c
	lib/stringinfo.c
)

if(OPENSSL_FOUND)
	set(libpq_ssl_SRCS libpq/be-secure-openssl.c)
endif(OPENSSL_FOUND)

set(libpq_SRCS
	libpq/be-fsstubs.c
	libpq/be-secure.c
	libpq/auth.c
	libpq/crypt.c
	libpq/hba.c
	libpq/ip.c
	libpq/md5.c
	libpq/pqcomm.c
	libpq/pqformat.c
	libpq/pqmq.c
	libpq/pqsignal.c
	${libpq_ssl_SRCS}
)

set(utils_SRCS
	utils/error/assert.c
	utils/error/elog.c
	utils/cache/attoptcache.c
	utils/cache/catcache.c
	utils/cache/evtcache.c
	utils/cache/inval.c
	utils/cache/plancache.c
	utils/cache/relcache.c
	utils/cache/relmapper.c
	utils/cache/relfilenodemap.c
	utils/cache/spccache.c
	utils/cache/syscache.c
	utils/cache/lsyscache.c
	utils/cache/typcache.c
	utils/cache/ts_cache.c
	utils/mmgr/aset.c
	utils/mmgr/mcxt.c
	utils/mmgr/portalmem.c
	utils/adt/acl.c
	utils/adt/arrayfuncs.c
	utils/adt/array_expanded.c
	utils/adt/array_selfuncs.c
	utils/adt/array_typanalyze.c
	utils/adt/array_userfuncs.c
	utils/adt/arrayutils.c
	utils/adt/ascii.c
	utils/adt/bool.c
	utils/adt/cash.c
	utils/adt/char.c
	utils/adt/date.c
	utils/adt/datetime.c
	utils/adt/datum.c
	utils/adt/dbsize.c
	utils/adt/domains.c
	utils/adt/encode.c
	utils/adt/enum.c
	utils/adt/expandeddatum.c
	utils/adt/float.c
	utils/adt/format_type.c
	utils/adt/formatting.c
	utils/adt/genfile.c
	utils/adt/geo_ops.c
	utils/adt/geo_selfuncs.c
	utils/adt/inet_cidr_ntop.c
	utils/adt/inet_net_pton.c
	utils/adt/int.c
	utils/adt/int8.c
	utils/adt/json.c
	utils/adt/jsonb.c
	utils/adt/jsonb_gin.c
	utils/adt/jsonb_op.c
	utils/adt/jsonb_util.c
	utils/adt/jsonfuncs.c
	utils/adt/like.c
	#utils/adt/like_match.c
	utils/adt/lockfuncs.c
	utils/adt/mac.c
	utils/adt/misc.c
	utils/adt/nabstime.c
	utils/adt/name.c
	utils/adt/network.c
	utils/adt/network_gist.c
	utils/adt/network_selfuncs.c
	utils/adt/numeric.c
	utils/adt/numutils.c
	utils/adt/oid.c
	utils/adt/oracle_compat.c
	utils/adt/orderedsetaggs.c
	utils/adt/pg_locale.c
	utils/adt/pg_lsn.c
	utils/adt/pg_upgrade_support.c
	utils/adt/pgstatfuncs.c
	utils/adt/pseudotypes.c
	utils/adt/quote.c
	utils/adt/rangetypes.c
	utils/adt/rangetypes_gist.c
	utils/adt/rangetypes_selfuncs.c
	utils/adt/rangetypes_spgist.c
	utils/adt/rangetypes_typanalyze.c
	utils/adt/regexp.c
	utils/adt/regproc.c
	utils/adt/ri_triggers.c
	utils/adt/rowtypes.c
	utils/adt/ruleutils.c
	utils/adt/selfuncs.c
	utils/adt/tid.c
	utils/adt/timestamp.c
	utils/adt/trigfuncs.c
	utils/adt/tsginidx.c
	utils/adt/tsgistidx.c
	utils/adt/tsquery.c
	utils/adt/tsquery_cleanup.c
	utils/adt/tsquery_gist.c
	utils/adt/tsquery_op.c
	utils/adt/tsquery_rewrite.c
	utils/adt/tsquery_util.c
	utils/adt/tsrank.c
	utils/adt/tsvector.c
	utils/adt/tsvector_op.c
	utils/adt/tsvector_parser.c
	utils/adt/txid.c
	utils/adt/uuid.c
	utils/adt/varbit.c
	utils/adt/varchar.c
	utils/adt/varlena.c
	#utils/adt/levenshtein.c
	utils/adt/version.c
	utils/adt/windowfuncs.c
	utils/adt/xid.c
	utils/adt/xml.c
	utils/fmgr/dfmgr.c
	utils/fmgr/fmgr.c
	utils/fmgr/funcapi.c
	utils/hash/dynahash.c
	utils/hash/hashfn.c
	utils/hash/pg_crc.c
	utils/init/globals.c
	utils/init/miscinit.c
	utils/init/postinit.c
	utils/resowner/resowner.c
	utils/sort/logtape.c
	utils/sort/sortsupport.c
	utils/sort/tuplesort.c
	utils/sort/tuplestore.c
	utils/time/combocid.c
	utils/time/tqual.c
	utils/time/snapmgr.c
	utils/misc/guc.c
	utils/misc/help_config.c
	utils/misc/pg_rusage.c
	utils/misc/ps_status.c
	utils/misc/rls.c
	utils/misc/sampling.c
	utils/misc/superuser.c
	utils/misc/timeout.c
	utils/misc/tzparser.c
	utils/mb/encnames.c
	utils/mb/conv.c
	utils/mb/mbutils.c
	utils/mb/wchar.c
	utils/mb/wstrcmp.c
	utils/mb/wstrncmp.c
)

set(nodes_SRCS
	nodes/nodeFuncs.c
	nodes/nodes.c
	nodes/list.c
	nodes/bitmapset.c
	nodes/tidbitmap.c
	nodes/copyfuncs.c
	nodes/equalfuncs.c
	nodes/makefuncs.c
	nodes/outfuncs.c
	nodes/readfuncs.c
	nodes/print.c
	nodes/read.c
	nodes/params.c
	nodes/value.c
)

set(optimizer_SRCS
	optimizer/geqo/geqo_copy.c
	optimizer/geqo/geqo_eval.c
	optimizer/geqo/geqo_main.c
	optimizer/geqo/geqo_misc.c
	optimizer/geqo/geqo_mutation.c
	optimizer/geqo/geqo_pool.c
	optimizer/geqo/geqo_random.c
	optimizer/geqo/geqo_recombination.c
	optimizer/geqo/geqo_selection.c
	optimizer/geqo/geqo_erx.c
	optimizer/geqo/geqo_pmx.c
	optimizer/geqo/geqo_cx.c
	optimizer/geqo/geqo_px.c
	optimizer/geqo/geqo_ox1.c
	optimizer/geqo/geqo_ox2.c
	optimizer/path/allpaths.c
	optimizer/path/clausesel.c
	optimizer/path/costsize.c
	optimizer/path/equivclass.c
	optimizer/path/indxpath.c
	optimizer/path/joinpath.c
	optimizer/path/joinrels.c
	optimizer/path/pathkeys.c
	optimizer/path/tidpath.c
	optimizer/plan/analyzejoins.c
	optimizer/plan/createplan.c
	optimizer/plan/initsplan.c
	optimizer/plan/planagg.c
	optimizer/plan/planmain.c
	optimizer/plan/planner.c
	optimizer/plan/setrefs.c
	optimizer/plan/subselect.c
	optimizer/prep/prepjointree.c
	optimizer/prep/prepqual.c
	optimizer/prep/prepsecurity.c
	optimizer/prep/preptlist.c
	optimizer/prep/prepunion.c
	optimizer/util/clauses.c
	optimizer/util/joininfo.c
	optimizer/util/orclauses.c
	optimizer/util/pathnode.c
	optimizer/util/placeholder.c
	optimizer/util/plancat.c
	optimizer/util/predtest.c
	optimizer/util/relnode.c
	optimizer/util/restrictinfo.c
	optimizer/util/tlist.c
	optimizer/util/var.c
)

set(rewrite_SRCS
	rewrite/rewriteRemove.c
	rewrite/rewriteDefine.c
	rewrite/rewriteHandler.c
	rewrite/rewriteManip.c
	rewrite/rewriteSupport.c
	rewrite/rowsecurity.c
)

set(PARSER_SCAN_FLEX_FLAGS, "-Cfe -p -p")
bison_target(PARSER_GRAM
	parser/gram.y
	${CMAKE_CURRENT_SOURCE_DIR}/parser/gram.c
)
flex_target(PARSER_SCAN
	parser/scan.l
	"${CMAKE_CURRENT_SOURCE_DIR}/parser/scan.c"
	${PARSER_SCAN_FLEX_FLAGS}
)

set_source_files_properties(${BISON_PARSER_GRAM_OUTPUTS}
	PROPERTIES OBJECT_DEPENDS ${FLEX_PARSER_SCAN_OUTPUTS})

add_custom_command(
	MAIN_DEPENDENCY ${BISON_PARSER_GRAM_OUTPUTS}
	OUTPUT ${PROJECT_SOURCE_DIR}/src/include/parser/gram.h
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/parser/gram.h ${PROJECT_SOURCE_DIR}/src/include/parser/gram.h
)

set(parser_SRCS
	parser/analyze.c
	#parser/gram.c
	parser/keywords.c
	parser/kwlookup.c
	parser/parser.c
	parser/parse_agg.c
	parser/parse_clause.c
	parser/parse_coerce.c
	parser/parse_collate.c
	parser/parse_cte.c
	parser/parse_expr.c
	parser/parse_func.c
	parser/parse_node.c
	parser/parse_oper.c
	parser/parse_param.c
	parser/parse_relation.c
	parser/parse_target.c
	parser/parse_type.c
	parser/parse_utilcmd.c
	parser/scansup.c
	${BISON_PARSER_GRAM_OUTPUTS}
)

set(tcop_SRCS
	tcop/dest.c
	tcop/fastpath.c
	tcop/postgres.c
	tcop/pquery.c
	tcop/utility.c
)

set(tsearch_SRCS
	tsearch/ts_locale.c
	tsearch/ts_parse.c
	tsearch/wparser.c
	tsearch/wparser_def.c
	tsearch/dict.c
	tsearch/dict_simple.c
	tsearch/dict_synonym.c
	tsearch/dict_thesaurus.c
	tsearch/dict_ispell.c
	tsearch/regis.c
	tsearch/spell.c
	tsearch/to_tsany.c
	tsearch/ts_selfuncs.c
	tsearch/ts_typanalyze.c
	tsearch/ts_utils.c
)

install(FILES 
			tsearch/ispell_sample.dict
			tsearch/ispell_sample.affix
			tsearch/hunspell_sample.affix
			tsearch/synonym_sample.syn
			tsearch/thesaurus_sample.ths
		DESTINATION ${PGSHAREDIR}/tsearch_data)

set(regex_SRCS
	regex/regcomp.c
	regex/regerror.c
	regex/regexec.c
	regex/regfree.c
	regex/regprefix.c
	regex/regexport.c
)

set(postmaster_SRCS
	postmaster/autovacuum.c
	postmaster/bgworker.c
	postmaster/bgwriter.c
	postmaster/checkpointer.c
	postmaster/fork_process.c
	postmaster/pgarch.c
	postmaster/pgstat.c
	postmaster/postmaster.c
	postmaster/startup.c
	postmaster/syslogger.c
	postmaster/walwriter.c
)

if(WIN32)
	set(backend_port_win32_SRC
		port/win32/timer.c
		port/win32/socket.c
		port/win32/signal.c
		port/win32/security.c
		port/win32/mingwcompat.c
	)
	if(NAVE_MINIDUMP_TYPE)
		set(backend_port_win32_SRC
			${backend_port_win32_SRC}
			port/win32/crashdump.c
		)
	endif(NAVE_MINIDUMP_TYPE)
endif(WIN32)

if(APPLE)
	set(backend_port_darwin_SRC
		port/darwin/system.c
	)
endif(APPLE)



set(backend_port_SRCS
	port/atomics.c
	port/tas/${TAS}
	${backend_port_win32_SRC}
	${backend_port_darwin_SRC}
	${dynloader_SRCS}
	${SEMA_IMPLEMENTATION}
	${SHMEM_IMPLEMENTATION}
	${LATCH_IMPLEMENTATION}
)


set(storage_SRCS
	storage/buffer/buf_table.c
	storage/buffer/buf_init.c
	storage/buffer/bufmgr.c
	storage/buffer/freelist.c
	storage/buffer/localbuf.c
	storage/file/fd.c
	storage/file/buffile.c
	storage/file/copydir.c
	storage/file/reinit.c
	storage/freespace/freespace.c
	storage/freespace/fsmpage.c
	storage/freespace/indexfsm.c
	storage/ipc/dsm_impl.c
	storage/ipc/dsm.c
	storage/ipc/ipc.c
	storage/ipc/ipci.c
	storage/ipc/pmsignal.c
	storage/ipc/procarray.c
	storage/ipc/procsignal.c
	storage/ipc/shmem.c
	storage/ipc/shmqueue.c
	storage/ipc/shm_mq.c
	storage/ipc/shm_toc.c
	storage/ipc/sinval.c
	storage/ipc/sinvaladt.c
	storage/ipc/standby.c
	storage/large_object/inv_api.c
	storage/lmgr/lmgr.c # Need some for check s_lock
	storage/lmgr/lock.c
	storage/lmgr/proc.c
	storage/lmgr/deadlock.c
	storage/lmgr/lwlock.c
	storage/lmgr/spin.c
	storage/lmgr/s_lock.c
	storage/lmgr/predicate.c
	storage/page/bufpage.c
	storage/page/checksum.c
	storage/page/itemptr.c
	storage/smgr/md.c
	storage/smgr/smgr.c
	storage/smgr/smgrtype.c
)

bison_target(REPL_GRAM
	replication/repl_gram.y
	${CMAKE_CURRENT_SOURCE_DIR}/replication/repl_gram.c
)
flex_target(REPL_SCAN
	replication/repl_scanner.l
	${CMAKE_CURRENT_SOURCE_DIR}/replication/repl_scanner.c
)
set_source_files_properties(${BISON_REPL_GRAM_OUTPUTS}
	PROPERTIES OBJECT_DEPENDS ${FLEX_REPL_SCAN_OUTPUTS})

set(replication_SRCS
	replication/walsender.c
	replication/walreceiverfuncs.c
	replication/walreceiver.c
	replication/basebackup.c
	replication/slot.c
	replication/slotfuncs.c
	replication/syncrep.c
	replication/logical/decode.c
	replication/logical/logical.c
	replication/logical/logicalfuncs.c
	replication/logical/reorderbuffer.c
	replication/logical/origin.c
	replication/logical/snapbuild.c
	${BISON_REPL_GRAM_OUTPUTS}
)

set(timezone_SRCS
	${PROJECT_SOURCE_DIR}/src/timezone/localtime.c
	${PROJECT_SOURCE_DIR}/src/timezone/strftime.c
	${PROJECT_SOURCE_DIR}/src/timezone/pgtz.c
)

add_custom_command(
	OUTPUT "${PROJECT_SOURCE_DIR}/src/backend/utils/sort/qsort_tuple.c"
	#MAIN_DEPENDENCY gen_qsort_tuple.pl
	COMMAND ${PERL_EXECUTABLE} "gen_qsort_tuple.pl" > "${PROJECT_SOURCE_DIR}/src/backend/utils/sort/qsort_tuple.c"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/utils/sort
)

set_source_files_properties(utils/sort/tuplesort.c
	PROPERTIES OBJECT_DEPENDS "${PROJECT_SOURCE_DIR}/src/backend/utils/sort/qsort_tuple.c")

add_custom_command(
	MAIN_DEPENDENCY ${gen_errorcodes_depend}
	OUTPUT ${gen_errorcodes_output}
	COMMAND ${PERL_EXECUTABLE} generate-errcodes.pl errcodes.txt > ${gen_errorcodes_output}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/backend/utils
)

set(postgres_SRCS
	${gen_errorcodes_output}
	${access_SRCS}
	${bootstrap_SRCS}
	${commands_SRCS}
	${catalog_SRCS}
	${executor_SRCS}
	foreign/foreign.c
	${utils_SRCS}
	${lib_SRCS}
	${libpq_SRCS}
	${nodes_SRCS}
	${optimizer_SRCS}
	${rewrite_SRCS}
	${parser_SRCS}
	${tcop_SRCS}
	${tsearch_SRCS}
	${regex_SRCS}
	${postmaster_SRCS}
	${backend_port_SRCS}
	${storage_SRCS}
	${replication_SRCS}
	${timezone_SRCS}
	main/main.c
)

include_directories(BEFORE
	${OPENSSL_INCLUDE_DIR}
)

add_executable(postgres
	${postgres_SRCS}
)

target_link_libraries(postgres
	pgcommon_srv
	port_srv
	pq
	${READLINE_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	m
	${OPENSSL_LIBRARIES}
	${DL_LIBRARIES}
)

if(UNIX AND NOT APPLE)
	target_link_libraries(postgres rt)
endif()

#Realy need?
add_custom_target(duplicate_oids
	COMMAND ${PERL_EXECUTABLE} ./duplicate_oids
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/catalog
)

add_library(pqwalreceiver SHARED replication/libpqwalreceiver/libpqwalreceiver.c)
target_link_libraries(pqwalreceiver pq)

install(TARGETS postgres pqwalreceiver
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

install(FILES 
			catalog/postgres.bki
			catalog/postgres.description
			catalog/postgres.shdescription
			catalog/information_schema.sql
			catalog/sql_features.txt
			catalog/system_views.sql
			libpq/pg_hba.conf.sample
			libpq/pg_ident.conf.sample
			utils/misc/postgresql.conf.sample
		DESTINATION ${PGSHAREDIR})

add_subdirectory(utils/mb/conversion_procs)
add_subdirectory(snowball)