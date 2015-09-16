include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

set(psqlscan_flags, "-Cfe -p -p")

flex_target(psqlscan
	psqlscan.l 
	"${CMAKE_CURRENT_SOURCE_DIR}/psqlscan.c"
	${psqlscan_flags}
)

set_source_files_properties(mainloop.c
	PROPERTIES OBJECT_DEPENDS ${FLEX_psqlscan_OUTPUTS})

set(psql_SRCS
	command.c
	common.c
	help.c
	help.h
	input.c
	stringutils.c
	mainloop.c
	copy.c
	copy.h
	startup.c
	prompt.c
	variables.c
	large_obj.c
	print.c
	describe.c
	tab-complete.c
	mbprint.c
	dumputils.c
	keywords.c
	kwlookup.c
	sql_help.c
)

add_executable(psql
	${psql_SRCS}
)

target_link_libraries(psql
	pgcommon
	port
	pq
	${READLINE_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	m
	${OPENSSL_LIBRARIES}
)
