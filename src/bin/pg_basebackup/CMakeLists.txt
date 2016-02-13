include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_library(common_basebackup STATIC
	receivelog.c
	streamutil.c
)

target_link_libraries(common_basebackup
	port
)

set(basebackup_list 
	pg_basebackup
	pg_receivexlog
	pg_recvlogical
)

foreach(loop_var IN ITEMS ${basebackup_list})
	add_executable(${loop_var} ${loop_var}.c)

	target_link_libraries(${loop_var}
		port
		pq
		pgcommon
		${M_LIB}
		common_basebackup
	)
endforeach()

install(TARGETS ${basebackup_list}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
