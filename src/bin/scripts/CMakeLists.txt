include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
	"${PROJECT_SOURCE_DIR}/src/bin/psql"
)

add_library(common_dump STATIC
	common.c
	${PROJECT_SOURCE_DIR}/src/bin/pg_dump/dumputils.c
)

add_library(common_print STATIC
	common.c
)

set(common_libs
	pgfeutils
	pgcommon
	pgport
	pq
	${LIB_M}
)

add_executable(pg_isready
	pg_isready.c
	common.c
)

target_link_libraries(pg_isready
	${common_libs}
)

CMAKE_SET_TARGET_FOLDER(pg_isready bin)

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

	CMAKE_SET_TARGET_FOLDER(${loop_var} bin)
endforeach()


install(TARGETS ${dump_modules_list} pg_isready
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(pgscripts "cs;de;es;fr;it;ja;ko;pl;pt_BR;ru;sv;zh_CN;he")
endif()

if(PROVE)
	TAP_CHECK("scripts_tap" "${TAP_FLAGS}" "t/*.pl")
endif()
