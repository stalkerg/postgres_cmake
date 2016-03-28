include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(extension_name pageinspect)

add_library(${extension_name} SHARED
	rawpage.c
	heapfuncs.c
	btreefuncs.c
	fsmfuncs.c
	brinfuncs.c
	ginfuncs.c
)
target_link_libraries(${extension_name} ${contrib_libs})
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	${extension_name}.control
	${extension_name}--1.4.sql
	${extension_name}--1.3--1.4.sql
	${extension_name}--1.2--1.3.sql
	${extension_name}--1.1--1.2.sql
	${extension_name}--1.0--1.1.sql
	${extension_name}--unpackaged--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)
