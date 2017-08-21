include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_upgrade
	check.c
	controldata.c
	dump.c
	exec.c
	file.c
	function.c
	info.c
	option.c
	parallel.c
	pg_upgrade.c
	relfilenode.c
	server.c
	tablespace.c
	util.c
	version.c 
)

target_link_libraries(pg_upgrade
	pgfeutils
	pgcommon
	pgport
	pq
)

CMAKE_SET_TARGET_FOLDER(pg_upgrade bin)

install(TARGETS pg_upgrade
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(pg_rewind "fr;ru")
endif()

