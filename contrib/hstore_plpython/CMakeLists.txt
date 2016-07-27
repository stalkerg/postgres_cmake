include_directories(
	"${PROJECT_SOURCE_DIR}/src/include"
	"${PROJECT_SOURCE_DIR}/src/pl/plpython"
	"${PROJECT_SOURCE_DIR}/contrib/hstore"
	${PYTHON_INCLUDE_DIRS}
)

set(extension_name hstore_plpython)

add_library(${extension_name}${PYTHON_VERSION_MAJOR} MODULE ${extension_name}.c)
target_link_libraries(${extension_name}${PYTHON_VERSION_MAJOR} ${contrib_libs} ${PYTHON_LIBRARIES})
set_target_properties(${extension_name}${PYTHON_VERSION_MAJOR} PROPERTIES PREFIX "")
add_dependencies(${extension_name}${PYTHON_VERSION_MAJOR} postgres)
if (MSVC)
	gen_def(${extension_name})
endif()
if(NOT PGXS)
	CMAKE_SET_TARGET_FOLDER(${extension_name}${PYTHON_VERSION_MAJOR} contrib)
endif()
install(TARGETS ${extension_name}${PYTHON_VERSION_MAJOR}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	${extension_name}u.control
	${extension_name}u--1.0.sql
	${extension_name}2u.control
	${extension_name}2u--1.0.sql
	${extension_name}3u.control
	${extension_name}3u--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)

set(regress_files hstore_plpython)

if(PYTHON_VERSION_MAJOR EQUAL 3)
	replace_python_files("${regress_files}")
	set(regress_options
		"--load-extension=hstore"
	)
	CONTRIB_REGRESS_CHECK(${extension_name} "${regress_options}" "${regress_files3}")
else()
	set(regress_options
		"--load-extension=hstore"
		"--load-extension=plpythonu"
		"--load-extension=hstore_plpythonu"
	)
	CONTRIB_REGRESS_CHECK(${extension_name} "${regress_options}" "${regress_files}")
endif()
