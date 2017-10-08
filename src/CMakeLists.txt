include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(gen_errorcodes_depend
	${PROJECT_SOURCE_DIR}/src/backend/utils/errcodes.txt
	${PROJECT_SOURCE_DIR}/src/backend/utils/generate-errcodes.pl
)
set(gen_errorcodes_output
	${PROJECT_SOURCE_DIR}/src/include/utils/errcodes.h
)

add_custom_command(
	MAIN_DEPENDENCY ${gen_errorcodes_depend}
	DEPENDS ${gen_errorcodes_depend}
	OUTPUT ${gen_errorcodes_output}
	COMMAND ${PERL_EXECUTABLE} generate-errcodes.pl errcodes.txt > ${gen_errorcodes_output}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/backend/utils
)

add_custom_target(gen_errorcodes DEPENDS ${gen_errorcodes_output})

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${gen_errorcodes_output})

set(PARSER_SCAN_FLEX_FLAGS, "-Cfe -p -p")
bison_target(PARSER_GRAM
	${PROJECT_SOURCE_DIR}/src/backend/parser/gram.y
	${PROJECT_SOURCE_DIR}/src/backend/parser/gram.c
)
flex_target(PARSER_SCAN
	${PROJECT_SOURCE_DIR}/src/backend/parser/scan.l
	${PROJECT_SOURCE_DIR}/src/backend/parser/scan.c
	${PARSER_SCAN_FLEX_FLAGS}
)

set_source_files_properties(${BISON_PARSER_GRAM_OUTPUTS}
	PROPERTIES OBJECT_DEPENDS ${FLEX_PARSER_SCAN_OUTPUTS})

add_custom_command(
	MAIN_DEPENDENCY ${BISON_PARSER_GRAM_OUTPUTS}
	OUTPUT ${PROJECT_SOURCE_DIR}/src/include/parser/gram.h
	COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/src/backend/parser/gram.h ${PROJECT_SOURCE_DIR}/src/include/parser/gram.h
)

add_custom_target(generate_parser_gram_h DEPENDS ${PROJECT_SOURCE_DIR}/src/include/parser/gram.h)

add_subdirectory(include)
add_subdirectory(port)
add_subdirectory(common)
add_subdirectory(fe_utils)
add_subdirectory(interfaces/libpq)
add_subdirectory(interfaces/ecpg/pgtypeslib)
add_subdirectory(interfaces/ecpg/ecpglib)
add_subdirectory(interfaces/ecpg/compatlib)
add_subdirectory(interfaces/ecpg/include)
add_subdirectory(interfaces/ecpg/preproc)
add_subdirectory(interfaces/ecpg/test)

add_subdirectory(backend)
add_subdirectory(timezone)
add_subdirectory(test/regress)
add_subdirectory(test/isolation)
add_subdirectory(test/modules)
add_subdirectory(test/recovery)
add_subdirectory(test/ssl)

add_subdirectory(pl/plpgsql/src)
if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
	add_subdirectory(pl/plpython)
endif()
if(PERLLIBS_FOUND)
	add_subdirectory(pl/plperl)
endif()
if(TCL_FOUND)
	add_subdirectory(pl/tcl)
endif()

add_subdirectory(bin/initdb)
add_subdirectory(bin/pg_archivecleanup)
add_subdirectory(bin/pg_basebackup)
add_subdirectory(bin/pg_config)
add_subdirectory(bin/pg_controldata)
add_subdirectory(bin/pg_ctl)
add_subdirectory(bin/pg_dump)
add_subdirectory(bin/pg_resetwal)
add_subdirectory(bin/pg_rewind)
add_subdirectory(bin/pg_test_fsync)
add_subdirectory(bin/pg_test_timing)
add_subdirectory(bin/pg_upgrade)
add_subdirectory(bin/pg_waldump)
add_subdirectory(bin/pgbench)
add_subdirectory(bin/psql)
add_subdirectory(bin/scripts)
