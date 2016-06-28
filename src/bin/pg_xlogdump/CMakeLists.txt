include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_xlogdump
	${PROJECT_SOURCE_DIR}/src/backend/access/transam/xlogreader.c
	pg_xlogdump.c
	compat.c
	rmgrdesc.c
	${rmgrdesc_SRC}
)

target_link_libraries(pg_xlogdump
	pgcommon
	port
	pq
)

target_compile_definitions(pg_xlogdump PRIVATE -DFRONTEND)

CMAKE_SET_TARGET_FOLDER(pg_xlogdump bin)

install(TARGETS pg_xlogdump
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
