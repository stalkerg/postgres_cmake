include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_rewind
	pg_rewind.c
	parsexlog.c
	datapagemap.c
	timeline.c
	fetch.c
	file_ops.c
	copy_fetch.c
	libpq_fetch.c
	filemap.c
	logging.c
	${PROJECT_SOURCE_DIR}/src/backend/access/transam/xlogreader.c
)

target_link_libraries(pg_rewind
	pgcommon
	pgport
	pq
)

target_compile_definitions(pg_rewind PRIVATE -DFRONTEND)

CMAKE_SET_TARGET_FOLDER(pg_rewind bin)

install(TARGETS pg_rewind
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(pg_rewind "de;es;fr;it;ko;pl;ru;zh_CN;ja;pt_BR;sv")
endif()

if(PROVE)
	TAP_CHECK("pg_rewind_tap" "${TAP_FLAGS}" "t/*.pl")
endif()
