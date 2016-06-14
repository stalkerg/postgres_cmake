include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_controldata
	pg_controldata.c
)

target_link_libraries(pg_controldata
	pgcommon
	port
)

install(TARGETS pg_controldata
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(pg_controldata "cs;de;es;fr;it;ja;ko;pl;pt_BR;ru;sv;zh_CN")
endif()
