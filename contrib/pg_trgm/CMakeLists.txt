include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(extension_name pg_trgm)

add_library(${extension_name} SHARED
	trgm_op.c
	trgm_gist.c
	trgm_gin.c
	trgm_regexp.c
)
target_link_libraries(${extension_name} ${contrib_libs})
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	${extension_name}.control
	${extension_name}--1.2.sql
	${extension_name}--1.1--1.2.sql
	${extension_name}--1.0--1.1.sql
	${extension_name}--unpackaged--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)
