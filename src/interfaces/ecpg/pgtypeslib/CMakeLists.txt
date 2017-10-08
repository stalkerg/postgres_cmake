include_directories(BEFORE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${PROJECT_SOURCE_DIR}/src/port"
	"${PROJECT_SOURCE_DIR}/src/interfaces/ecpg/include"
)
set(pgtypes_SRC
	numeric.c
	datetime.c
	common.c
	dt_common.c
	timestamp.c
	interval.c
	${PROJECT_SOURCE_DIR}/src/port/pgstrcasecmp.c
)
if(USE_REPL_SNPRINTF)
	set(pgtypes_SRC
		${pgtypes_SRC}
		${PROJECT_SOURCE_DIR}/src/port/snprintf.c
	)
endif()

if(NOT HAVE_RINT)
	set(pgtypes_SRC
		${pgtypes_SRC}
		${PROJECT_SOURCE_DIR}/src/port/rint.c
	)
endif()

add_library(pgtypes SHARED ${pgtypes_SRC})
if (MSVC)
	gen_def(pgtypes)
endif()

target_link_libraries(pgtypes ${LIB_M})
target_compile_definitions(pgtypes PRIVATE -DFRONTEND)
set_target_properties(pgtypes PROPERTIES VERSION "3.7")


install(TARGETS pgtypes
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
