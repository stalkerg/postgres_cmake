include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

bison_target(PGBENCH_PARSER exprparse.y ${PROJECT_SOURCE_DIR}/src/bin/pgbench/exprparse.c)
flex_target(PGBENCH_SCANNER exprscan.l  ${PROJECT_SOURCE_DIR}/src/bin/pgbench/exprscan.c)
add_flex_bison_dependency(PGBENCH_SCANNER PGBENCH_PARSER)

set_source_files_properties(exprparse.c PROPERTIES OBJECT_DEPENDS ${FLEX_PGBENCH_SCANNER_OUTPUTS})

add_executable(pgbench
	pgbench.c
	${BISON_PGBENCH_PARSER_OUTPUTS}
)

target_link_libraries(pgbench
	pgfeutils
	pgport
	pq
	pgcommon
	${M_LIB}
)

add_dependencies(pgbench postgres)

CMAKE_SET_TARGET_FOLDER(pgbench bin)

install(TARGETS pgbench
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(PROVE)
	TAP_CHECK("pgbench_tap" "${TAP_FLAGS}" "t/*.pl")
endif()
