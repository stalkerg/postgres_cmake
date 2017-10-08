include_directories(
	"${PROJECT_SOURCE_DIR}/src/include"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
)

set(extension_name postgres_fdw)

add_library(${extension_name} ${PLUGIN_TYPE}
	${extension_name}.c
	option.c
	deparse.c
	connection.c
	shippable.c
)
target_link_libraries(${extension_name} ${contrib_libs} pq)
if(MSVC)
	gen_def(${extension_name})
endif()
if(NOT PGXS)
	CMAKE_SET_TARGET_FOLDER(${extension_name} contrib)
endif()
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	${extension_name}.control
	${extension_name}--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)

set(regress_files postgres_fdw)
CONTRIB_REGRESS_CHECK(${extension_name} "" "${regress_files}")
