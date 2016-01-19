include_directories(BEFORE
 	"${PROJECT_SOURCE_DIR}/src/include/libpq"
 	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
 	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_library(libdump STATIC
	pg_backup_archiver.c
	pg_backup_db.c
	pg_backup_custom.c
	pg_backup_null.c
	pg_backup_tar.c
	pg_backup_directory.c
	pg_backup_utils.c
	parallel.c
	compress_io.c
	dumputils.c
	keywords.c
	${PROJECT_SOURCE_DIR}/src/backend/parser/kwlookup.c
)

add_executable(pg_dump
	pg_dump.c
	common.c
	pg_dump_sort.c
)

target_link_libraries(pg_dump
	libdump
	port
	pq
	pgcommon
)

add_executable(pg_restore
	pg_restore.c
)

target_link_libraries(pg_restore
	libdump
	port
	pq
	pgcommon
)

add_executable(pg_dumpall
	pg_dumpall.c
	dumputils.c
)

target_link_libraries(pg_dumpall
	libdump
	port
	pq
	pgcommon
)

install(TARGETS pg_dump pg_restore pg_dumpall
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})