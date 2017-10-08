include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(extension_name pg_visibility)

add_library(${extension_name} ${PLUGIN_TYPE} ${extension_name}.c)
target_link_libraries(${extension_name} ${contrib_libs})
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
	${extension_name}--1.1--1.2.sql
	${extension_name}--1.0--1.1.sql
	DESTINATION ${PGSHAREDIR}/extension)

set(regres_files
	${extension_name}
)
CONTRIB_REGRESS_CHECK(${extension_name} "" "${regres_files}")
