include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(extension_name intarray)

add_library(${extension_name} SHARED
	_int_bool.c
	_int_gist.c
	_int_op.c
	_int_tool.c
	_intbig_gist.c
	_int_gin.c
	_int_selfuncs.c
)
target_link_libraries(${extension_name} ${contrib_libs})
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	${extension_name}.control
	${extension_name}--1.1.sql
	${extension_name}--1.0--1.1.sql
	${extension_name}--unpackaged--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)
