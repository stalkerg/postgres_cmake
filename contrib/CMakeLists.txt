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

add_subdirectory(spi)
add_subdirectory(adminpack)
add_subdirectory(auth_delay)
add_subdirectory(auto_explain)
add_subdirectory(btree_gin)
add_subdirectory(btree_gist)
add_subdirectory(chkpass)
add_subdirectory(citext)
