include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_archivecleanup pg_archivecleanup.c)

target_link_libraries(pg_archivecleanup
	port
#	pq
	pgcommon
	${M_LIB}
)

install(TARGETS pg_archivecleanup
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
