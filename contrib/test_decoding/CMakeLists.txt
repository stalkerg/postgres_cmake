option(PGXS "Separate build" OFF)
if(PGXS)
	cmake_minimum_required(VERSION 2.8)
	find_package(PostgreSQL)
	include(${PostgreSQL_LIBRARY_DIRS}/cmake/PGXS.cmake)
else()
	include_directories("${PROJECT_SOURCE_DIR}/src/include")
endif()

set(extension_name test_decoding)

add_library(${extension_name} ${PLUGIN_TYPE}
	test_decoding.c
)
target_link_libraries(${extension_name} ${contrib_libs})
set_target_properties(${extension_name} PROPERTIES PREFIX "")
if (MSVC)
	gen_def(${extension_name})
endif()
if(NOT PGXS)
	add_dependencies(${extension_name} postgres)
	CMAKE_SET_TARGET_FOLDER(${extension_name} contrib)
endif()
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

set(regerss_checks
	ddl
	xact
	rewrite
	toast
	permissions
	decoding_in_xact
	decoding_into_rel
	binary
	prepared
	replorigin
	time
	messages
	slot
)

set(regress_options
	--temp-config ${CMAKE_SOURCE_DIR}/contrib/test_decoding/logical.conf
)

set(isolation_checks
	mxact
	delayed_startup
	ondisk_startup
	concurrent_ddl_dml
)

# Don't add tests to installcheck
REGRESS_CHECK(${extension_name} "${regress_options}" "${regerss_checks}")
ISOLATION_CHECK(${extension_name} "${regress_options}" "${isolation_checks}")
set(contrib_check_targets
	${contrib_check_targets}
	${extension_name}_installcheck_tmp
	${extension_name}_isolation_installcheck_tmp
PARENT_SCOPE)
