include_directories(BEFORE
 	"${PROJECT_SOURCE_DIR}/src/include/libpq"
 	"${PROJECT_SOURCE_DIR}/src/port"
 	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
 	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_regress
	pg_regress.c
	pg_regress_main.c
)

target_link_libraries(pg_regress
	port
	pq
	pgcommon
)

target_compile_definitions(pg_regress PRIVATE
	-DHOST_TUPLE="${CMAKE_HOST_SYSTEM}"
	-DSHELLPROG="$ENV{SHELL}")


add_library(regress SHARED regress.c)
target_link_libraries(regress
	pq
)
if (MSVC)
	target_link_libraries(regress postgres pgcommon port)
	gen_def(regress)
endif()
set_target_properties(regress PROPERTIES PREFIX "")
add_dependencies(regress postgres)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(env_cmd
		${CMAKE_COMMAND} -E env
		"DYLD_LIBRARY_PATH=$ENV{DESTDIR}${LIBDIR}:$DYLD_LIBRARY_PATH"
	)
	set(tmp_env_cmd
		${CMAKE_COMMAND} -E env
		"DYLD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}/tmp_install${LIBDIR}:$DYLD_LIBRARY_PATH"
	)
elif(CMAKE_SYSTEM_NAME STREQUAL "AIX")
	set(env_cmd
		${CMAKE_COMMAND} -E env
		"LIBPATH=$ENV{DESTDIR}${LIBDIR}:$LIBPATH"
	)
	set(tmp_env_cmd
		${CMAKE_COMMAND} -E env
		"LIBPATH=${CMAKE_CURRENT_BINARY_DIR}/tmp_install${LIBDIR}:$LIBPATH"
	)
else()
	set(env_cmd
		${CMAKE_COMMAND} -E env
		"LD_LIBRARY_PATH=$ENV{DESTDIR}${LIBDIR}:$LD_LIBRARY_PATH"
	)
	set(tmp_env_cmd
		${CMAKE_COMMAND} -E env
		"LD_LIBRARY_PATH=${CMAKE_CURRENT_BINARY_DIR}/tmp_install${LIBDIR}:$LD_LIBRARY_PATH"
	)
endif()


set(pre_pg_regress_check
	${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_INSTALL_CONFIG_NAME}/pg_regress${CMAKE_EXECUTABLE_SUFFIX}
	--inputdir="${CMAKE_CURRENT_SOURCE_DIR}"
	--temp-instance="tmp_check"
	--encoding=UTF8
	--no-locale
)

set(pg_regress_check
	${env_cmd}
	${pre_pg_regress_check}
	--bindir=$ENV{DESTDIR}${PGBINDIR}
)

set(pg_regress_check_tmp
	${tmp_env_cmd}
	${pre_pg_regress_check}
	--bindir=${CMAKE_CURRENT_BINARY_DIR}/tmp_install${PGBINDIR}
)

if(MAX_CONNECTIONS)
	set(MAXCONNOPT "${MAXCONNOPT} --max-connections=${MAX_CONNECTIONS}")
endif()

if(TEMP_CONFIG)
	set(TEMP_CONF "${TEMP_CONF} --temp-config=${TEMP_CONFIG}")
endif()

add_custom_target(tablespace-setup
	COMMAND ${CMAKE_COMMAND} -E remove_directory ./testtablespace
	COMMAND ${CMAKE_COMMAND} -E make_directory ./testtablespace
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(check
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/tmp_install
	COMMAND make install DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/tmp_install
	COMMAND make installcheck_tmp DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/tmp_install
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(installcheck
	COMMAND ${pg_regress_check} ${REGRESS_OPTS} --schedule=${CMAKE_CURRENT_SOURCE_DIR}/parallel_schedule --dlpath=$ENV{DESTDIR}${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${EXTRA_TESTS}
	DEPENDS tablespace-setup
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(installcheck_tmp
	COMMAND ${pg_regress_check_tmp} ${REGRESS_OPTS} --schedule=${CMAKE_CURRENT_SOURCE_DIR}/parallel_schedule --dlpath=${CMAKE_CURRENT_BINARY_DIR}/tmp_install${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${EXTRA_TESTS}
	DEPENDS tablespace-setup
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS pg_regress
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

install(TARGETS regress
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})