include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_waldump
	${PROJECT_SOURCE_DIR}/src/backend/access/transam/xlogreader.c
	pg_waldump.c
	compat.c
	rmgrdesc.c
	${rmgrdesc_SRC}
)

target_link_libraries(pg_waldump
	pgcommon
	pgport
	pq
)

target_compile_definitions(pg_waldump PRIVATE -DFRONTEND)

CMAKE_SET_TARGET_FOLDER(pg_waldump bin)

if(ENABLE_NLS)
	MAKE_MO(pg_waldump "es;fr;sv")
endif()

install(TARGETS pg_waldump
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
