include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pgbench
	pgbench.c
	exprparse.c
)

target_link_libraries(pgbench
	port
	pq
	pgcommon
	${M_LIB}
)

flex_target(PGBENCH_SCANNER exprscan.l  ${PROJECT_SOURCE_DIR}/src/bin/pgbench/exprscan.c)
set_source_files_properties(exprparse.c PROPERTIES OBJECT_DEPENDS ${FLEX_PGBENCH_SCANNER_OUTPUTS})

install(TARGETS pgbench
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})