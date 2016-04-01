include_directories(
	"${PROJECT_SOURCE_DIR}/src/include"
	"${PROJECT_SOURCE_DIR}/src/pl/plperl"
	"${PROJECT_SOURCE_DIR}/contrib/hstore"
	${PERL_INCLUDE_PATH}
)

set(extension_name hstore_plperl)

add_library(${extension_name} SHARED ${extension_name}.c)
target_link_libraries(${extension_name} ${contrib_libs} ${PERL_LIBRARY})
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	${extension_name}.control
	${extension_name}--1.0.sql
	${extension_name}u.control
	${extension_name}u--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)

set(regress_files hstore_plperl)
CONTRIB_REGRESS_CHECK(${extension_name} "" "${regress_files}")
