set(modules_libs postgres)

#TODO: need test under mingw or cygwin
if(WIN32)
	set(modules_libs
		pgport
		pgcommon
	)
endif()

if(MSVC OR MINGW)
	set(modules_libs
		pgport
		postgres
		pgcommon_srv
	)
endif()


set(modules_check_targets "")
set(modules_installcheck_targets "")

# Tests
add_subdirectory(brin)
add_subdirectory(commit_ts)
add_subdirectory(dummy_seclabel)
add_subdirectory(snapshot_too_old)
add_subdirectory(test_ddl_deparse)
add_subdirectory(test_extensions)
add_subdirectory(test_parser)
add_subdirectory(test_pg_dump)
add_subdirectory(test_rls_hooks)
add_subdirectory(test_shm_mq)

if(CMAKE_GENERATOR STREQUAL "Ninja")
	add_custom_target(modules_check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
		COMMAND DESTDIR=${tmp_check_folder} ${check_make_command} install
		COMMAND DESTDIR=${tmp_check_folder} ${check_make_command} ${modules_check_targets}
		DEPENDS tablespace-setup
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
else()
	add_custom_target(modules_check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
		COMMAND ${check_make_command} install DESTDIR=${tmp_check_folder}
		COMMAND ${check_make_command} ${modules_check_targets} DESTDIR=${tmp_check_folder}
		DEPENDS tablespace-setup
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
endif()

# Make chain for run tests in strict order
# TODO: we can't run tests separately now
set(first_element "")
list(GET modules_installcheck_targets 0 first_element)
add_custom_target(modules_installcheck
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
	DEPENDS tablespace-setup ${first_element}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

set(last_element OFF)
foreach(loop_var IN ITEMS ${modules_installcheck_targets})
	if(last_element)
		add_dependencies(${last_element} ${loop_var})
	endif()
	set(last_element ${loop_var})
endforeach()
