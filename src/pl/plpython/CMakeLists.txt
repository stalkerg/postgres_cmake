add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/spiexceptions.h"
	MAIN_DEPENDENCY generate-spiexceptions.pl
	COMMAND ${PERL_EXECUTABLE} "generate-spiexceptions.pl" "${PROJECT_SOURCE_DIR}/src/backend/utils/errcodes.txt" > "${CMAKE_CURRENT_SOURCE_DIR}/spiexceptions.h"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(plname "plpython${PYTHON_VERSION_MAJOR}")

set(plpython_SRCS
	plpy_cursorobject.c
	plpy_elog.c
	plpy_exec.c
	plpy_main.c
	plpy_planobject.c
	plpy_plpymodule.c
	plpy_procedure.c
	plpy_resultobject.c
	plpy_spi.c
	plpy_subxactobject.c
	plpy_typeio.c
	plpy_util.c
)

include_directories(${PYTHON_INCLUDE_DIRS})
add_library(${plname} SHARED ${plpython_SRCS})
target_link_libraries(${plname} postgres ${PYTHON_LIBRARIES})
if (MSVC)
	target_link_libraries(${plname} port)
endif()
set_target_properties(${plname} PROPERTIES PREFIX "")
add_dependencies(${plname} postgres)

install(TARGETS ${plname}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES ${plname}u.control ${plname}u--1.0.sql ${plname}u--unpackaged--1.0.sql
		DESTINATION ${PGSHAREDIR}/extension)
