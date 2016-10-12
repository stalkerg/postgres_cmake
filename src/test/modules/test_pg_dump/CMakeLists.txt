option(PGXS "Separate build" OFF)
if(PGXS)
	cmake_minimum_required(VERSION 2.8)
	find_package(PostgreSQL)
	include(${PostgreSQL_LIBRARY_DIRS}/cmake/PGXS.cmake)
else()
	include_directories("${PROJECT_SOURCE_DIR}/src/include")
endif()

set(extension_name test_pg_dump)

install(FILES
	test_pg_dump--1.0.sql
	test_pg_dump.control
	DESTINATION ${PGSHAREDIR}/extension)


MODULES_REGRESS_CHECK(${extension_name} "" "${extension_name}")
if(PROVE)
	MODULES_TAP_CHECK(${extension_name} "${TAP_FLAGS}" "t/*.pl")
endif()
