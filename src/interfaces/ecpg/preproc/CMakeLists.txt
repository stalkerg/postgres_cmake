include_directories(BEFORE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${PROJECT_SOURCE_DIR}/src/port"
	"${PROJECT_SOURCE_DIR}/src/interfaces/ecpg/include"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
)

set(preproc_parser_DEP
	${PROJECT_SOURCE_DIR}/src/backend/parser/gram.y
	${CMAKE_CURRENT_SOURCE_DIR}/parse.pl
)

add_custom_command(
	DEPENDS ${preproc_parser_DEP}
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/preproc.y
	COMMAND ${PERL_EXECUTABLE} parse.pl . < ${PROJECT_SOURCE_DIR}/src/backend/parser/gram.y > preproc.y
	COMMAND ${PERL_EXECUTABLE} check_rules.pl . ${PROJECT_SOURCE_DIR}/src/backend/parser/gram.y
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

bison_target(PREPROC_PARSER ${CMAKE_CURRENT_SOURCE_DIR}/preproc.y ${CMAKE_CURRENT_SOURCE_DIR}/preproc.c)
flex_target(PREPROC_SCANNER pgc.l  ${CMAKE_CURRENT_SOURCE_DIR}/pgc.c)
add_flex_bison_dependency(PREPROC_SCANNER PREPROC_PARSER)


set(preproc_SRC
	preproc.c
	pgc.c
	type.c
	ecpg.c
	output.c
	parser.c
	keywords.c
	c_keywords.c
	ecpg_keywords.c
	${PROJECT_SOURCE_DIR}/src/interfaces/ecpg/ecpglib/typename.c
	descriptor.c
	variable.c
)


add_executable(ecpg_preproc
	${preproc_SRC}
)

target_link_libraries(ecpg_preproc
	#pq
	pgcommon
	pgport
)

target_compile_definitions(ecpg_preproc PRIVATE
	-DECPG_COMPILE
)

SET_TARGET_PROPERTIES(ecpg_preproc
	PROPERTIES OUTPUT_NAME ecpg
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
