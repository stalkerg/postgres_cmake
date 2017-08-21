include_directories(
	"${PROJECT_SOURCE_DIR}/src/include"
	"${PROJECT_SOURCE_DIR}/src/pl/plpython"
	"${PROJECT_SOURCE_DIR}/contrib/ltree"
	${PYTHON_INCLUDE_DIRS}
)

set(extension_name ltree_plpython)

add_library(${extension_name}${PYTHON_VERSION_MAJOR} ${PLUGIN_TYPE} ${extension_name}.c)
target_link_libraries(${extension_name}${PYTHON_VERSION_MAJOR} ${contrib_libs} ${PYTHON_LIBRARIES})
set_target_properties(${extension_name}${PYTHON_VERSION_MAJOR} PROPERTIES PREFIX "")
add_dependencies(${extension_name}${PYTHON_VERSION_MAJOR} postgres)
target_compile_definitions(${extension_name}${PYTHON_VERSION_MAJOR} PRIVATE -DPLPYTHON_LIBNAME="plpython${PYTHON_VERSION_MAJOR}")
if (MSVC)
	gen_def(${extension_name}${PYTHON_VERSION_MAJOR})
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

set(regress_files ltree_plpython)


if(PYTHON_VERSION_MAJOR EQUAL 3)
	replace_python_files("${regress_files}")
	set(regress_options
		"--load-extension=ltree"
	)
	CONTRIB_REGRESS_CHECK(${extension_name} "${regress_options}" "${regress_files3}")
else()
	set(regress_options
		"--load-extension=hstore"
		"--load-extension=plpythonu"
		"--load-extension=ltree"
		"--load-extension=ltree_plpythonu"
	)
	CONTRIB_REGRESS_CHECK(${extension_name} "${regress_options}" "${regress_files}")
endif()
