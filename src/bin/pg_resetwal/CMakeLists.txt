include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_resetwal
	pg_resetwal.c
)

target_link_libraries(pg_resetwal
	pgcommon
	pgport
)

CMAKE_SET_TARGET_FOLDER(pg_resetwal bin)

install(TARGETS pg_resetwal
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(pg_resetwal "cs;de;es;fr;it;ja;ko;pl;pt_BR;ru;sv;zh_CN")
endif()
