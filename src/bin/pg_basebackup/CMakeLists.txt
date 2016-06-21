include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
	${ZLIB_INCLUDE_DIRS}
)
if(ZLIB_FOUND)
	include_directories(BEFORE ${ZLIB_INCLUDE_DIRS})
endif()


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
	if(ZLIB_FOUND)
		target_link_libraries(${loop_var} ${ZLIB_LIBRARIES})
	endif()
endforeach()

install(TARGETS ${basebackup_list}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(pg_basebackup "de;es;fr;it;ko;pl;pt_BR;ru;zh_CN")
endif()
