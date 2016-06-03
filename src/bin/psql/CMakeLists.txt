include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_custom_command(
	MAIN_DEPENDENCY create_help.pl
	OUTPUT ${PROJECT_SOURCE_DIR}/src/bin/psql/sql_help.c ${PROJECT_SOURCE_DIR}/src/bin/psql/sql_help.h
	COMMAND ${PERL_EXECUTABLE} create_help.pl ../../../doc/src/sgml/ref sql_help
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/bin/psql
)

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
	describe.c
	tab-complete.c
	../pg_dump/dumputils.c
	sql_help.c
	psqlscanslash.c
	crosstabview.c
)

add_executable(psql
	${psql_SRCS}
)

target_link_libraries(psql
	pgfeutils
	pgcommon
	port
	pq
	${READLINE_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
	${OPENSSL_LIBRARIES}
)

if(NOT MSVC)
	target_link_libraries(psql m)
endif()

add_dependencies(psql postgres)

install(TARGETS psql
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
