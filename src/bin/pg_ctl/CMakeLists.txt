include_directories(BEFORE
 	"${PROJECT_SOURCE_DIR}/src/include/libpq"
 	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
 	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_ctl
	pg_ctl.c
)

target_link_libraries(pg_ctl
	port
	pq
	pgcommon
)
