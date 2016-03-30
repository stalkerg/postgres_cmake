include_directories(
	"${PROJECT_SOURCE_DIR}/src/include"
	${LIBXML2_INCLUDE_DIR}
	${LIBXSLT_INCLUDE_DIR}
)

set(extension_name xml2)

add_library(${extension_name} SHARED xpath.c xslt_proc.c)
target_link_libraries(${extension_name}
	${contrib_libs}
	${LIBXML2_LIBRARIES}
	${LIBXSLT_LIBRARIES}
)
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
