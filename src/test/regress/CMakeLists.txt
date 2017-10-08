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
	pq
	pgcommon
	pgport
)

target_compile_definitions(pg_regress PRIVATE
	-DHOST_TUPLE="${HOST_TUPLE}"
	-DSHELLPROG="$ENV{SHELL}")


add_library(regress ${PLUGIN_TYPE} regress.c)
#target_link_libraries(regress
#	pq
#)
if (MSVC)
	gen_def(regress)
endif()

if(MSVC OR MINGW)
	target_link_libraries(regress postgres pgport pgcommon)
endif()

set_target_properties(regress PROPERTIES PREFIX "")
add_dependencies(regress postgres)

add_custom_target(tablespace-setup
	COMMAND ${CMAKE_COMMAND} -E remove_directory ./testtablespace
	COMMAND ${CMAKE_COMMAND} -E make_directory ./testtablespace
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

if(CMAKE_GENERATOR STREQUAL "Ninja")
	add_custom_target(check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/tmp_install
		COMMAND DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/tmp_install ${check_make_command} install
		COMMAND DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/ ${check_make_command} installcheck_tmp
		DEPENDS tablespace-setup
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
else()
	add_custom_target(check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/tmp_install
		COMMAND ${check_make_command} install DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/tmp_install
		COMMAND ${check_make_command} installcheck_tmp DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/
		DEPENDS tablespace-setup
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
endif()

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
