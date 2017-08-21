set(plname "pltcl")

include_directories(${TCL_INCLUDE_PATH})

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/pltclerrcodes.h"
	MAIN_DEPENDENCY generate-pltclerrcodes.pl
	COMMAND ${PERL_EXECUTABLE} "generate-pltclerrcodes.pl" "${PROJECT_SOURCE_DIR}/src/backend/utils/errcodes.txt" > "${CMAKE_CURRENT_SOURCE_DIR}/pltclerrcodes.h"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(gen_pltclerrcodes DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/pltclerrcodes.h")

add_library(${plname} ${PLUGIN_TYPE}
	pltcl.c
)

target_link_libraries(${plname} postgres ${TCL_LIBRARY})
if (MSVC)
	gen_def(${plname})
	target_link_libraries(${plname} pgport)
endif()
set_target_properties(${plname} PROPERTIES PREFIX "")
add_dependencies(${plname} postgres gen_pltclerrcodes)

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

set(PL_TESTDB pl_regression)
set(REGRESS_OPTS --dbname=${PL_TESTDB} --load-extension=pltcl)
set(plregres_files
	pltcl_setup
	pltcl_queries
	pltcl_unicode
	pltcl_subxact
	pltcl_start_proc
)
REGRESS_CHECK(pltcl "${REGRESS_OPTS}" "${plregres_files}")

if(ENABLE_NLS)
	MAKE_MO(pltcl "cs;de;es;fr;it;ja;ko;pl;pt_BR;ro;ru;tr;zh_CN;zh_TW;sv")
endif()
