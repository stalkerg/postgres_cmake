if(WIN32)
	set(contrib_libs
		port
		pgcommon
	)
endif()

if(MSVC)
	set(contrib_libs
		${contrib_libs}
		postgres
	)
endif()

set(contrib_check_targets "")

add_subdirectory(adminpack)
add_subdirectory(auth_delay)
add_subdirectory(auto_explain)
add_subdirectory(btree_gin)
add_subdirectory(btree_gist)
add_subdirectory(chkpass)
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
add_subdirectory(tsearch2)
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

#add_custom_target(contrib_check
#	DEPENDS ${contrib_check_targets}
#)

add_custom_target(contrib_check
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
	COMMAND make install DESTDIR=${tmp_check_folder}
	COMMAND make ${contrib_check_targets} DESTDIR=${tmp_check_folder}
	DEPENDS tablespace-setup
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
