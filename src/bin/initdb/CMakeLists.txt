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
	port
	pgcommon
)

if(UNIX AND NOT APPLE)
	target_link_libraries(initdb rt)
endif()

target_compile_definitions(initdb PRIVATE -DFRONTEND)

install(TARGETS initdb
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

