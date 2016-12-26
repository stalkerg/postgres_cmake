option(PGXS "Separate build" OFF)
if(PGXS)
	cmake_minimum_required(VERSION 2.8)
	find_package(PostgreSQL)
	include(${PostgreSQL_LIBRARY_DIRS}/cmake/PGXS.cmake)
else()
	include_directories("${PROJECT_SOURCE_DIR}/src/include")
endif()

set(extension_name test_extensions)

install(FILES
	test_ext1--1.0.sql
	test_ext1.control
	test_ext2--1.0.sql
	test_ext2.control
	test_ext3--1.0.sql
	test_ext3.control
	test_ext4--1.0.sql
	test_ext4.control
	test_ext5--1.0.sql
	test_ext5.control
	test_ext6--1.0.sql
	test_ext6.control
	test_ext7--1.0.sql
	test_ext7--1.0--2.0.sql
	test_ext7.control
	test_ext8--1.0.sql
	test_ext8.control
	test_ext_cyclic1--1.0.sql
	test_ext_cyclic1.control
	test_ext_cyclic2--1.0.sql
	test_ext_cyclic2.control
	DESTINATION ${PGSHAREDIR}/extension)

set(tests_list
	test_extensions
	test_extdepend
)

MODULES_REGRESS_CHECK(${extension_name} "" "${tests_list}")
