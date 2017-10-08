BISON_TARGET(PLPGSQL_PARSER pl_gram.y ${CMAKE_CURRENT_SOURCE_DIR}/pl_gram.c COMPILE_FLAGS "-d")

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/plerrcodes.h"
	MAIN_DEPENDENCY generate-plerrcodes.pl
	COMMAND ${PERL_EXECUTABLE} "generate-plerrcodes.pl" "${PROJECT_SOURCE_DIR}/src/backend/utils/errcodes.txt" > "${CMAKE_CURRENT_SOURCE_DIR}/plerrcodes.h"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(plpgsql_SRCS
	pl_handler.c
	pl_comp.c
	plerrcodes.h
	pl_exec.c
	pl_funcs.c
	pl_scanner.c
	${BISON_PLPGSQL_PARSER_OUTPUT_SOURCE}
)

add_library(plpgsql ${PLUGIN_TYPE} ${plpgsql_SRCS})
target_link_libraries(plpgsql postgres)
if (MSVC)
	gen_def(plpgsql)
	target_link_libraries(plpgsql pgport pgcommon)
endif()
set_target_properties(plpgsql PROPERTIES PREFIX "")
add_dependencies(plpgsql postgres)

install(TARGETS plpgsql
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES plpgsql.control plpgsql--1.0.sql plpgsql--unpackaged--1.0.sql
		DESTINATION ${PGSHAREDIR}/extension)

if(ENABLE_NLS)
	MAKE_MO(plpgsql "cs;de;es;fr;it;ja;ko;pl;pt_BR;ro;ru;zh_CN;zh_TW;sv")
endif()
