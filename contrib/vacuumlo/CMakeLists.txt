include_directories(
	"${PROJECT_SOURCE_DIR}/src/include"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
)

set(extension_name vacuumlo)

add_executable(${extension_name} ${extension_name}.c)
target_link_libraries(${extension_name} pgport pgcommon pq)
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
if(NOT PGXS)
	CMAKE_SET_TARGET_FOLDER(${extension_name} contrib)
endif()
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
