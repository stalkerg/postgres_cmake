include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(extension_name ltree)
add_definitions(-DLOWER_NODE)

add_library(${extension_name} SHARED
	ltree_io.c
	ltree_op.c
	lquery_op.c
	_ltree_op.c
	crc32.c
	ltxtquery_io.c
	ltxtquery_op.c
	ltree_gist.c
	_ltree_gist.c
)
target_link_libraries(${extension_name} ${contrib_libs})
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	${extension_name}.control
	${extension_name}--1.0.sql
	${extension_name}--unpackaged--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)
