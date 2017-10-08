include_directories(BEFORE
 	"${PROJECT_SOURCE_DIR}/src/include/libpq"
 	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
 	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
	${ZLIB_INCLUDE_DIRS}
)

if(ZLIB_FOUND)
	include_directories(BEFORE ${ZLIB_INCLUDE_DIRS})
endif()

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
)

add_executable(pg_dump
	pg_dump.c
	common.c
	pg_dump_sort.c
)

target_link_libraries(pg_dump
	libdump
	pgfeutils
	pgcommon
	pgport
	pq
)

CMAKE_SET_TARGET_FOLDER(pg_dump bin)

if(ZLIB_FOUND)
	target_link_libraries(pg_dump ${ZLIB_LIBRARIES})
endif()

add_executable(pg_restore
	pg_restore.c
)

target_link_libraries(pg_restore
	libdump
	pgfeutils
	pgcommon
	pgport
	pq
)

CMAKE_SET_TARGET_FOLDER(pg_restore bin)

if(ZLIB_FOUND)
	target_link_libraries(pg_restore ${ZLIB_LIBRARIES})
endif()

add_executable(pg_dumpall
	pg_dumpall.c
	dumputils.c
)

target_link_libraries(pg_dumpall
	libdump
	pgfeutils
	pgcommon
	pgport
	pq
)

CMAKE_SET_TARGET_FOLDER(pg_dumpall bin)

if(ZLIB_FOUND)
	target_link_libraries(pg_dumpall ${ZLIB_LIBRARIES})
endif()

install(TARGETS pg_dump pg_restore pg_dumpall
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(pg_dump "cs;de;es;fr;it;ja;ko;pl;pt_BR;ru;zh_CN;sv")
endif()

if(PROVE)
	TAP_CHECK("pg_dump_tap" "${TAP_FLAGS}" "t/*.pl")
endif()

