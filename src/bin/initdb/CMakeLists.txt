include_directories(BEFORE
 	"${PROJECT_SOURCE_DIR}/src/include/libpq"
 	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
 	"${PROJECT_SOURCE_DIR}/src/timezone"
)

add_executable(initdb
	initdb.c
	findtimezone.c
	"${PROJECT_SOURCE_DIR}/src/timezone/localtime.c"
	"${PROJECT_SOURCE_DIR}/src/backend/utils/mb/encnames.c"
)

target_link_libraries(initdb
	pgfeutils
	pgcommon
	pgport
	pq
	${GSS_LIBS}
)

if(UNIX AND NOT APPLE)
	target_link_libraries(initdb rt)
endif()

target_compile_definitions(initdb PRIVATE -DFRONTEND)
CMAKE_SET_TARGET_FOLDER(initdb bin)

install(TARGETS initdb
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(initdb "cs;de;es;fr;it;ja;ko;pl;pt_BR;ru;sv;zh_CN;he")
endif()

if(PROVE)
	TAP_CHECK("initdb_tap" "${TAP_FLAGS}" "t/*.pl")
endif()
