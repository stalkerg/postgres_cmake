include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(extension_name sepgsql)

set(sepgsql_sql_output
	${CMAKE_CURRENT_SOURCE_DIR}/sepgsql.sql
)

add_custom_command(
	MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/sepgsql.sql.in
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/sepgsql.sql
	COMMAND sed 's,MODULE_PATHNAME,$$libdir/${extension_name},g' sepgsql.sql.in > ${CMAKE_CURRENT_SOURCE_DIR}/sepgsql.sql
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(${extension_name} ${PLUGIN_TYPE}
	hooks.c
	selinux.c
	uavc.c
	label.c
	dml.c
	database.c
	schema.c
	relation.c
	proc.c
	${CMAKE_CURRENT_SOURCE_DIR}/sepgsql.sql
)
target_link_libraries(${extension_name} ${contrib_libs} ${SELINUX_LIBRARIES})
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
	${CMAKE_CURRENT_SOURCE_DIR}/sepgsql.sql
	DESTINATION ${PGSHAREDIR}/contrib)

#set(regress_files seg)
#CONTRIB_REGRESS_CHECK(${extension_name} "" "${regress_files}")
