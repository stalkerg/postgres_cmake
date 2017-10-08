include_directories(
	"${PROJECT_SOURCE_DIR}/src/include"
	"${PROJECT_SOURCE_DIR}/src/pl/plperl"
	"${PROJECT_SOURCE_DIR}/contrib/hstore"
	${PERL_INCLUDE_PATH}
)

if(MINGW)
	link_directories("${PROJECT_BINARY_DIR}/contrib/hstore")
endif()
set(extension_name hstore_plperl)

add_library(${extension_name} ${PLUGIN_TYPE} ${extension_name}.c)
target_link_libraries(${extension_name} ${contrib_libs} ${PERL_LIBRARY} ${PERL_LDFLAGS})
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
if (MSVC)
	gen_def(${extension_name})
endif()
if(MINGW)
	target_link_libraries(${extension_name} minhstore)
	target_compile_definitions(${extension_name} PRIVATE -DPLPERL_HAVE_UID_GID)
endif()
if(NOT PGXS)
	CMAKE_SET_TARGET_FOLDER(${extension_name} contrib)
endif()
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
