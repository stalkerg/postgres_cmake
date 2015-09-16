include_directories(
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

#set(psqlscan_flags, "-Cfe -p -p")

#flex_target(psqlscan
#	psqlscan.l 
#	"${CMAKE_CURRENT_SOURCE_DIR}/psqlscan.c"
#	${psqlscan_flags}
#)

#set_source_files_properties(mainloop.c
#      PROPERTIES OBJECT_DEPENDS ${FLEX_psqlscan_OUTPUTS})

set(access_SRCS
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

set(postgres_include_catalog "${PROJECT_SOURCE_DIR}/src/include/catalog")
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

set(utils_SRCS
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
)

set(postgres_SRCS
	${access_SRCS}
	${bootstrap_SRCS}
	${commands_SRCS}
	${catalog_SRCS}
	${utils_SRCS}
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
)

#Realy need?
add_custom_target(duplicate_oids
	COMMAND ${PERL_EXECUTABLE} ./duplicate_oids
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/include/catalog
)
