include_directories("${PROJECT_SOURCE_DIR}/src/include")


set(modules_list 
	autoinc
	insert_username
	moddatetime
	refint
	timetravel
)

foreach(loop_var IN ITEMS ${modules_list})
	add_library(${loop_var} ${PLUGIN_TYPE} ${loop_var}.c)
	target_compile_definitions(${loop_var} PRIVATE -DREFINT_VERBOSE)
	target_link_libraries(${loop_var} ${contrib_libs})
	set_target_properties(${loop_var} PROPERTIES PREFIX "")
	add_dependencies(${loop_var} postgres)
	if(MSVC)
		gen_def(${loop_var})
	endif()
	if(NOT PGXS)
		CMAKE_SET_TARGET_FOLDER(${loop_var} contrib/spi)
	endif()
	install(TARGETS ${loop_var}
			RUNTIME DESTINATION ${PGBINDIR}
			LIBRARY DESTINATION ${LIBDIR})
	install(FILES ${loop_var}.control ${loop_var}--1.0.sql ${loop_var}--unpackaged--1.0.sql
		DESTINATION ${PGSHAREDIR}/extension)
endforeach(loop_var)
