include_directories(BEFORE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${PROJECT_SOURCE_DIR}/src/port"
	"${PROJECT_SOURCE_DIR}/src/interfaces/ecpg/include"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
)
set(ecpg_compat_SRC
	informix.c
)
if(USE_REPL_SNPRINTF)
	set(ecpg_compat_SRC
		${ecpg_compat_SRC}
		${PROJECT_SOURCE_DIR}/src/port/snprintf.c
	)
endif()

add_library(ecpg_compat SHARED ${ecpg_compat_SRC})
if (MSVC)
	gen_def(ecpg_compat)
endif()
target_link_libraries(ecpg_compat ${LIB_M} pgtypes pq ecpg)
target_compile_definitions(ecpg_compat PRIVATE -DFRONTEND)
set_target_properties(ecpg_compat PROPERTIES VERSION "3.8")

install(TARGETS ecpg_compat
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
