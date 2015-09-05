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

set(postgres_SRCS
	${access_SRCS}
	${bootstrap_SRCS}
	${commands_SRCS}
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
