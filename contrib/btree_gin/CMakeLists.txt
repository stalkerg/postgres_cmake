include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(extension_name btree_gin)

add_library(${extension_name} SHARED ${extension_name}.c)
target_link_libraries(${extension_name}
	pq
	port
)
target_link_libraries(${extension_name} postgres)
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES ${extension_name}.control ${extension_name}--1.0.sql ${extension_name}--unpackaged--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)

