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
	port
	pq
	pgcommon
)

target_compile_definitions(pg_isolation_regress PRIVATE
	-DHOST_TUPLE="${CMAKE_HOST_SYSTEM}"
	-DSHELLPROG="$ENV{SHELL}"
)

bison_target(SPEC_PARSER specparse.y ${CMAKE_CURRENT_SOURCE_DIR}/specparce.c)
flex_target(SPEC_SCANNER specscanner.l  ${CMAKE_CURRENT_SOURCE_DIR}/specscanner.c)
add_flex_bison_dependency(SPEC_SCANNER SPEC_PARSER)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/specparce.c PROPERTIES OBJECT_DEPENDS ${FLEX_SPEC_SCANNER_OUTPUTS})

add_executable(isolationtester
	isolationtester.c
	${BISON_SPEC_PARSER_OUTPUTS}
)

target_link_libraries(isolationtester
	port
	pq
	pgcommon
)

if(MSVC)
	#Need rewrite
	set(pre_pg_regress_check
		${PGBINDIR}/pg_isolation_regress${CMAKE_EXECUTABLE_SUFFIX}
		--inputdir="${CMAKE_SOURCE_DIR}/src/test/isolation"
		--temp-instance="tmp_check"
		--encoding=UTF8
		--no-locale
	)
else(MSVC)
	set(pre_pg_regress_check
		${CMAKE_BINARY_DIR}/src/test/isolation/${CMAKE_INSTALL_CONFIG_NAME}/pg_isolation_regress${CMAKE_EXECUTABLE_SUFFIX}
		--inputdir="${CMAKE_SOURCE_DIR}/src/test/isolation"
		--temp-instance="tmp_check"
		--encoding=UTF8
		--no-locale
	)
endif()

set(pg_regress_check
	${env_cmd}
	${pre_pg_regress_check}
	--bindir=$ENV{DESTDIR}${PGBINDIR}
)

set(pg_regress_check_tmp
	${tmp_env_cmd}
	${pre_pg_regress_check}
	--bindir=${tmp_check_folder}${PGBINDIR}
)

if(MAX_CONNECTIONS)
	set(MAXCONNOPT "${MAXCONNOPT} --max-connections=${MAX_CONNECTIONS}")
endif()

if(TEMP_CONFIG)
	set(TEMP_CONF "${TEMP_CONF} --temp-config=${TEMP_CONFIG}")
endif()

add_custom_target(isolation_check
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/tmp_install
	COMMAND make install DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/tmp_install
	COMMAND make isolation_installcheck_tmp DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(isolation_installcheck
	COMMAND ${pg_regress_check} ${REGRESS_OPTS} --schedule=${CMAKE_CURRENT_SOURCE_DIR}/isolation_schedule --dlpath=$ENV{DESTDIR}${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${EXTRA_TESTS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(isolation_installcheck_tmp
	COMMAND ${pg_regress_check_tmp} ${REGRESS_OPTS} --schedule=${CMAKE_CURRENT_SOURCE_DIR}/isolation_schedule --dlpath=${CMAKE_CURRENT_BINARY_DIR}/tmp_install${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${EXTRA_TESTS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
