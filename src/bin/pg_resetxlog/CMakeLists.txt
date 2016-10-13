include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_resetxlog
	pg_resetxlog.c
)

target_link_libraries(pg_resetxlog
	pgcommon
	pgport
)

CMAKE_SET_TARGET_FOLDER(pg_resetxlog bin)

install(TARGETS pg_resetxlog
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(pg_resetxlog "cs;de;es;fr;it;ja;ko;pl;pt_BR;ru;sv;zh_CN")
endif()
