add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/spiexceptions.h"
	MAIN_DEPENDENCY generate-spiexceptions.pl
	COMMAND ${PERL_EXECUTABLE} "generate-spiexceptions.pl" "${PROJECT_SOURCE_DIR}/src/backend/utils/errcodes.txt" > "${CMAKE_CURRENT_SOURCE_DIR}/spiexceptions.h"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(plname "plpython${PYTHON_VERSION_MAJOR}")

set(plpython_SRCS
	"${CMAKE_CURRENT_SOURCE_DIR}/spiexceptions.h"
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
add_library(${plname} ${PLUGIN_TYPE} ${plpython_SRCS})
target_link_libraries(${plname} postgres ${PYTHON_LIBRARIES})
if (MSVC)
	gen_def(${plname})
	target_link_libraries(${plname} pgport)
endif()
if(MINGW)
	set_target_properties(${plname} PROPERTIES LINK_FLAGS -Wl,--out-implib=libmin${plname}.dll.a)
endif()
set_target_properties(${plname} PROPERTIES PREFIX "")
add_dependencies(${plname} postgres)

install(TARGETS ${plname}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	plpython2u.control
	plpython2u--1.0.sql
	plpython2u--unpackaged--1.0.sql
	plpython3u.control
	plpython3u--1.0.sql
	plpython3u--unpackaged--1.0.sql
	plpythonu.control
	plpythonu--1.0.sql
	plpythonu--unpackaged--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)

set(PL_TESTDB pl_regression)
set(REGRESS_OPTS --dbname=${PL_TESTDB})
if (PYTHON_VERSION_MAJOR EQUAL 2)
	set(REGRESS_OPTS ${REGRESS_OPTS} --load-extension=plpythonu)
endif()

set(plregres_files
	plpython_schema
	plpython_populate
	plpython_test
	plpython_do
	plpython_global
	plpython_import
	plpython_spi
	plpython_newline
	plpython_void
	plpython_params
	plpython_setof
	plpython_record
	plpython_trigger
	plpython_types
	plpython_error
	plpython_unicode
	plpython_quote
	plpython_composite
	plpython_subtransaction
	plpython_drop
)

if(PYTHON_VERSION_MAJOR EQUAL 3)
	replace_python_files("${plregres_files}")
	REGRESS_CHECK(plpython "${REGRESS_OPTS}" "${regress_files3}")
else()
	REGRESS_CHECK(plpython "${REGRESS_OPTS}" "${plregres_files}")
endif()

if(ENABLE_NLS)
	MAKE_MO(plpython "cs;de;es;fr;it;ja;ko;pl;pt_BR;ru;zh_CN;sv")
endif()
