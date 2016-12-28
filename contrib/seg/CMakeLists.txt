include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(extension_name seg)

bison_target(CUBE_PARSER segparse.y ${CMAKE_CURRENT_SOURCE_DIR}/segparse.c)
flex_target(CUBE_SCANNER segscan.l  ${CMAKE_CURRENT_SOURCE_DIR}/segscan.c)
add_flex_bison_dependency(CUBE_SCANNER CUBE_PARSER)
set_source_files_properties(${BISON_CUBE_PARSER_OUTPUTS} PROPERTIES OBJECT_DEPENDS ${FLEX_CUBE_SCANNER_OUTPUTS})

add_library(${extension_name} ${PLUGIN_TYPE}
	${extension_name}.c
	${BISON_CUBE_PARSER_OUTPUTS}
)
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
	${extension_name}--1.0--1.1.sql
	${extension_name}--unpackaged--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)

set(regress_files seg)
CONTRIB_REGRESS_CHECK(${extension_name} "" "${regress_files}")
