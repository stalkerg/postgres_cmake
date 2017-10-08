include_directories(BEFORE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${PROJECT_SOURCE_DIR}/src/port"
	"${PROJECT_SOURCE_DIR}/src/interfaces/ecpg/include"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
)
set(ecpg_SRC
	execute.c
	typename.c
	descriptor.c
	sqlda.c
	data.c
	error.c
	prepare.c
	memory.c
	connect.c
	misc.c
	${PROJECT_SOURCE_DIR}/src/port/path.c
	${PROJECT_SOURCE_DIR}/src/port/pgstrcasecmp.c
)

if(USE_REPL_SNPRINTF)
	set(ecpg_SRC
		${ecpg_SRC}
		${PROJECT_SOURCE_DIR}/src/port/snprintf.c
	)
endif()

if(NOT HAVE_RINT)
	set(ecpg_SRC
		${ecpg_SRC}
		${PROJECT_SOURCE_DIR}/src/port/rint.c
	)
endif()

if(NOT HAVE_DECL_STRLCPY)
	set(ecpg_SRC
		${ecpg_SRC}
		${PROJECT_SOURCE_DIR}/src/port/strlcpy.c
	)
endif()

if(NOT HAVE_ISINF)
	set(ecpg_SRC
		${ecpg_SRC}
		${PROJECT_SOURCE_DIR}/src/port/isinf.c
	)
endif()

if(WIN32)
	set(ecpg_SRC
		${ecpg_SRC}
		${PROJECT_SOURCE_DIR}/src/port/win32setlocale.c
	)
else()
	set(ecpg_SRC
		${ecpg_SRC}
		${PROJECT_SOURCE_DIR}/src/port/thread.c
	)
endif()

add_library(ecpg SHARED ${ecpg_SRC})
if (MSVC)
	gen_def(ecpg)
endif()
target_link_libraries(ecpg ${LIB_M} pgtypes pq)
target_compile_definitions(ecpg PRIVATE -DFRONTEND)
set_target_properties(ecpg PROPERTIES VERSION "6.8")

install(TARGETS ecpg
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(ecpg "cs;de;es;fr;it;ja;ko;pl;pt_BR;ru;tr;zh_CN")
endif()
