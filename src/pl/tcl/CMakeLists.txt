set(plname "pltcl")

include_directories(${TCL_INCLUDE_PATH})
add_library(${plname} SHARED pltcl.c)
target_link_libraries(${plname} postgres ${TCL_LIBRARY})
if (MSVC)
	target_link_libraries(${plname} port)
endif()
set_target_properties(${plname} PROPERTIES PREFIX "")
add_dependencies(${plname} postgres)

install(TARGETS ${plname}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
		${plname}.control
		${plname}--1.0.sql
		${plname}--unpackaged--1.0.sql
		${plname}u.control
		${plname}u--1.0.sql
		${plname}u--unpackaged--1.0.sql
		DESTINATION ${PGSHAREDIR}/extension)
