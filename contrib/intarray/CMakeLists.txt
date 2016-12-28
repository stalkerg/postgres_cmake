include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(extension_name intarray)
set(extension_lib_name _int)

add_library(${extension_lib_name} ${PLUGIN_TYPE}
	_int_bool.c
	_int_gist.c
	_int_op.c
	_int_tool.c
	_intbig_gist.c
	_int_gin.c
	_int_selfuncs.c
)
target_link_libraries(${extension_lib_name} ${contrib_libs})
set_target_properties(${extension_lib_name} PROPERTIES PREFIX "")
add_dependencies(${extension_lib_name} postgres)
if (MSVC)
	gen_def(${extension_lib_name})
endif()
if(NOT PGXS)
	CMAKE_SET_TARGET_FOLDER(${extension_name} contrib)
endif()
install(TARGETS ${extension_lib_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	${extension_name}.control
	${extension_name}--1.2.sql
	${extension_name}--1.0--1.1.sql
	${extension_name}--1.1--1.2.sql
	${extension_name}--unpackaged--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)

set(regress_files ${extension_lib_name})
CONTRIB_REGRESS_CHECK(${extension_name} "" "${regress_files}")
