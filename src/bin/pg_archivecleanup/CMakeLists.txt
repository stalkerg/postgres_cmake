include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_archivecleanup pg_archivecleanup.c)

target_link_libraries(pg_archivecleanup
	pgcommon
	pgport
	${M_LIB}
)

CMAKE_SET_TARGET_FOLDER(pg_archivecleanup bin)

install(TARGETS pg_archivecleanup
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})


if(ENABLE_NLS)
	MAKE_MO(initdb "de;es;fr;pl;ru;sv;")
endif()
