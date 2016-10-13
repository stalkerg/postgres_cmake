include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/port"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
	"${PROJECT_SOURCE_DIR}/src/test/regress"
)

add_executable(pg_isolation_regress
	${PROJECT_SOURCE_DIR}/src/test/regress/pg_regress.c
	isolation_main.c
)

target_link_libraries(pg_isolation_regress
	pq
	pgcommon
	pgport
)

target_compile_definitions(pg_isolation_regress PRIVATE
	-DHOST_TUPLE="${HOST_TUPLE}"
	-DSHELLPROG="$ENV{SHELL}"
)

bison_target(SPEC_PARSER specparse.y ${CMAKE_CURRENT_SOURCE_DIR}/specparse.c)
flex_target(SPEC_SCANNER specscanner.l  ${CMAKE_CURRENT_SOURCE_DIR}/specscanner.c)
add_flex_bison_dependency(SPEC_SCANNER SPEC_PARSER)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/specparse.c PROPERTIES OBJECT_DEPENDS ${FLEX_SPEC_SCANNER_OUTPUTS})

add_executable(isolationtester
	isolationtester.c
	${BISON_SPEC_PARSER_OUTPUTS}
)

target_link_libraries(isolationtester
	pgcommon
	pgport
	pq
)

if(CMAKE_GENERATOR STREQUAL "Ninja")
	add_custom_target(isolation_check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/tmp_install
		COMMAND DESTDIR=${tmp_check_folder} ${check_make_command} install
		COMMAND DESTDIR=${tmp_check_folder} ${check_make_command} isolation_installcheck_tmp
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
else()
	add_custom_target(isolation_check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/tmp_install
		COMMAND ${check_make_command} install DESTDIR=${tmp_check_folder}
		COMMAND ${check_make_command} isolation_installcheck_tmp DESTDIR=${tmp_check_folder}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
endif()

add_custom_target(isolation_installcheck
	COMMAND ${pg_isolation_regress_check} ${REGRESS_OPTS} --schedule=${CMAKE_CURRENT_SOURCE_DIR}/isolation_schedule --dlpath=$ENV{DESTDIR}${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${EXTRA_TESTS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(isolation_installcheck_tmp
	COMMAND ${pg_isolation_regress_check_tmp} ${REGRESS_OPTS} --schedule=${CMAKE_CURRENT_SOURCE_DIR}/isolation_schedule --dlpath=${tmp_check_folder}${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${EXTRA_TESTS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS isolationtester
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

install(TARGETS pg_isolation_regress
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
