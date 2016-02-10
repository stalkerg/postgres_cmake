include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
	"${PROJECT_SOURCE_DIR}/src/bin/psql"
)

add_library(common_dump STATIC
	common.c
	${PROJECT_SOURCE_DIR}/src/bin/pg_dump/dumputils.c
	${PROJECT_SOURCE_DIR}/src/bin/pg_dump/keywords.c
	${PROJECT_SOURCE_DIR}/src/backend/parser/kwlookup.c
)

add_library(common_print STATIC
	common.c
	${PROJECT_SOURCE_DIR}/src/bin/psql/print.c
	${PROJECT_SOURCE_DIR}/src/bin/psql/mbprint.c
)

set(common_libs
	pgcommon
	port
	pq
	${LIB_M}
)

add_executable(createlang
	createlang.c
)

target_link_libraries(createlang
	common_print
	${common_libs}
)

add_executable(droplang
	droplang.c
)

target_link_libraries(droplang
	common_print
	${common_libs}
)

add_executable(pg_isready
	pg_isready.c
	common.c
)

target_link_libraries(pg_isready
	${common_libs}
)

set(dump_modules_list 
	createdb
	createuser
	dropdb
	dropuser
	clusterdb
	vacuumdb
	reindexdb
)

foreach(loop_var IN ITEMS ${dump_modules_list})
	add_executable(${loop_var}
		${loop_var}.c
		${dump_parser_src}
	)

	target_link_libraries(${loop_var}
		common_dump
		${common_libs}
	)
endforeach()


install(TARGETS ${dump_modules_list} createlang droplang pg_isready
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
