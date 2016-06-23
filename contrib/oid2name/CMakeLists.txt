include_directories(
	"${PROJECT_SOURCE_DIR}/src/include"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
)

set(extension_name oid2name)

add_library(${extension_name} SHARED ${extension_name}.c)
target_link_libraries(${extension_name} ${contrib_libs})
if(MSVC)
	target_link_libraries(${extension_name} pq)
endif()
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
