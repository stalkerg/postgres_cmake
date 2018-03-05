include_directories("${PROJECT_SOURCE_DIR}/src/include")
include_directories(${LIBUUID_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/contrib/pgcrypto")

set(extension_name uuid-ossp)

add_library(${extension_name} ${PLUGIN_TYPE} ${extension_name}.c ${UUID_EXTRA_OBJS})
target_link_libraries(${extension_name} ${contrib_libs} ${LIBUUID_LIBRARIES})
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
if(MSVC)
	gen_def(${extension_name})
endif()
if(NOT PGXS)
	CMAKE_SET_TARGET_FOLDER(${extension_name} contrib)
endif()
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	${extension_name}.control
	${extension_name}--1.1.sql
	${extension_name}--1.0--1.1.sql
	${extension_name}--unpackaged--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)

set(regress_files uuid_ossp)
CONTRIB_REGRESS_CHECK(${extension_name} "" "${regress_files}")
