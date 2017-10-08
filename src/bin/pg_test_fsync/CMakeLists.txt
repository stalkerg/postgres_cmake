include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_test_fsync
	pg_test_fsync.c
	
)

target_link_libraries(pg_test_fsync
	pgcommon
	pgport
)

CMAKE_SET_TARGET_FOLDER(pg_test_fsync bin)

install(TARGETS pg_test_fsync
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(pg_rewind "es;fr;pl;ru;sv")
endif()
