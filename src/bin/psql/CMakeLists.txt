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

flex_target(SCANSLASH_SCAN
	psqlscanslash.l
	${CMAKE_CURRENT_SOURCE_DIR}/psqlscanslash.c
)

set(psql_SRCS
	common.c
	variables.c
	command.c
	copy.c
	prompt.c
	mainloop.c
	crosstabview.c
	input.c
	help.c
	large_obj.c
	tab-complete.c
	stringutils.c
	describe.c
	startup.c
	../pg_dump/dumputils.c
	${FLEX_SCANSLASH_SCAN_OUTPUTS}
)

add_executable(psql
	${psql_SRCS}
)

target_link_libraries(psql
	pgfeutils
	pgcommon
	pgport
	pq
	${READLINE_LIBRARY}
	${CMAKE_THREAD_LIBS_INIT}
)

if(OPENSSL_FOUND)
	target_link_libraries(psql ${OPENSSL_LIBRARIES})
endif()
if(NOT MSVC)
	target_link_libraries(psql m)
endif()

add_dependencies(psql postgres)

CMAKE_SET_TARGET_FOLDER(psql bin)

install(TARGETS psql
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(pslq "cs;de;es;fr;it;ja;pl;pt_BR;ru;zh_CN;zh_TW;he;ko;sv")
endif()

