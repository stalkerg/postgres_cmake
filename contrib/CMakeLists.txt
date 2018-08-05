set(contrib_libs postgres)

#TODO: need test under mingw or cygwin
if(WIN32)
	set(contrib_libs
		pgport
		pgcommon
	)
endif()

if(MSVC OR MINGW)
	set(contrib_libs
		pgport
		postgres
		pgcommon_srv
	)
endif()

set(contrib_check_targets "")
set(contrib_installcheck_targets "")

# Tests
add_subdirectory(adminpack)
add_subdirectory(auth_delay)
add_subdirectory(auto_explain)
add_subdirectory(btree_gin)
add_subdirectory(btree_gist)
add_subdirectory(bloom)
add_subdirectory(citext)
add_subdirectory(cube)
add_subdirectory(dblink)
add_subdirectory(dict_int)
add_subdirectory(dict_xsyn)
add_subdirectory(earthdistance)
add_subdirectory(file_fdw)
add_subdirectory(fuzzystrmatch)
add_subdirectory(hstore)
if(PERLLIBS_FOUND)
	add_subdirectory(hstore_plperl)
endif()
if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
	add_subdirectory(hstore_plpython)
endif()
add_subdirectory(intarray)
add_subdirectory(isn)
add_subdirectory(lo)
add_subdirectory(ltree)
if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
	add_subdirectory(ltree_plpython)
endif()
add_subdirectory(oid2name)
add_subdirectory(pageinspect)
add_subdirectory(passwordcheck)
add_subdirectory(pg_buffercache)
add_subdirectory(pg_freespacemap)
add_subdirectory(pg_prewarm)
add_subdirectory(pg_standby)
add_subdirectory(pg_stat_statements)
add_subdirectory(pg_trgm)
add_subdirectory(pg_visibility)
add_subdirectory(pgcrypto)
add_subdirectory(pgrowlocks)
add_subdirectory(pgstattuple)
add_subdirectory(postgres_fdw)
add_subdirectory(seg)
if(SELINUX_LIBRARIES)
	add_subdirectory(sepgsql)
endif()
add_subdirectory(spi)
if(OPENSSL_FOUND)
	add_subdirectory(sslinfo)
endif()
add_subdirectory(tablefunc)
add_subdirectory(tcn)
add_subdirectory(tsm_system_rows)
add_subdirectory(tsm_system_time)
add_subdirectory(unaccent)
if(WITH_UUID)
	add_subdirectory(uuid-ossp)
endif()
add_subdirectory(vacuumlo)
if(LIBXML2_FOUND)
	add_subdirectory(xml2)
endif()

add_subdirectory(test_decoding)
add_subdirectory(amcheck)

if(CMAKE_GENERATOR STREQUAL "Ninja")
	add_custom_target(contrib_check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
		COMMAND DESTDIR=${tmp_check_folder} ${check_make_command} install
		COMMAND DESTDIR=${tmp_check_folder} -j1 ${check_make_command} ${contrib_check_targets}
		DEPENDS tablespace-setup
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
else()
	add_custom_target(contrib_check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
		COMMAND ${check_make_command} install DESTDIR=${tmp_check_folder}
		COMMAND ${check_make_command} ${contrib_check_targets} DESTDIR=${tmp_check_folder}
		DEPENDS tablespace-setup
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
endif()
# Make chain for run tests in strict order
# TODO: we can't run tests separately now
set(first_element "")
list(GET contrib_installcheck_targets 0 first_element)
add_custom_target(contrib_installcheck
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
	DEPENDS tablespace-setup ${first_element}
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

set(last_element OFF)
foreach(loop_var IN ITEMS ${contrib_installcheck_targets})
	if(last_element)
		add_dependencies(${last_element} ${loop_var})
	endif()
	set(last_element ${loop_var})
endforeach()
