include_directories(
	"${PROJECT_SOURCE_DIR}/src/include"
	"${PROJECT_SOURCE_DIR}/src/pl/plpython"
	"${PROJECT_SOURCE_DIR}/contrib/ltree"
	${PYTHON_INCLUDE_DIRS}
)

set(extension_name ltree_plpython)

add_library(${extension_name}${PYTHON_VERSION_MAJOR} SHARED ${extension_name}.c)
target_link_libraries(${extension_name}${PYTHON_VERSION_MAJOR} ${contrib_libs} ${PYTHON_LIBRARIES})
set_target_properties(${extension_name}${PYTHON_VERSION_MAJOR} PROPERTIES PREFIX "")
add_dependencies(${extension_name}${PYTHON_VERSION_MAJOR} postgres)
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
set(regress_options
	"--load-extension=ltree"
	"--load-extension=plpythonu"
	"--load-extension=ltree_plpythonu"
)
CONTRIB_REGRESS_CHECK(${extension_name} "${regress_options}" "${regress_files}")
