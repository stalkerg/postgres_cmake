include_directories(BEFORE
 	"${PROJECT_SOURCE_DIR}/src/include/libpq"
 	"${PROJECT_SOURCE_DIR}/src/port"
 	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
 	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_regress
	pg_regress.c
	pg_regress_main.c
)

target_link_libraries(pg_regress
	port
	pq
	pgcommon
)

target_compile_definitions(pg_regress PRIVATE
	-DHOST_TUPLE="${CMAKE_HOST_SYSTEM}"
	-DSHELLPROG="$ENV{SHELL}")


set(pg_regress_check pg_regress --inputdir="${CMAKE_CURRENT_SOURCE_DIR}" --temp-instance="./tmp_check" --bindir= --encoding=UTF8 --no-locale)

if(MAX_CONNECTIONS)
	set(MAXCONNOPT "${MAXCONNOPT} --max-connections=${MAX_CONNECTIONS}")
endif()

if(TEMP_CONFIG)
	set(TEMP_CONF "${TEMP_CONF} --temp-config=${TEMP_CONFIG}")
endif()

message(STATUS "${pg_regress_check} ${REGRESS_OPTS} --schedule=${CMAKE_CURRENT_SOURCE_DIR}/parallel_schedule ${MAXCONNOPT} ${TEMP_CONF} ${EXTRA_TESTS}")

add_custom_target(check
	COMMAND ${pg_regress_check} ${REGRESS_OPTS} --schedule=${CMAKE_CURRENT_SOURCE_DIR}/parallel_schedule ${MAXCONNOPT} ${TEMP_CONF} ${EXTRA_TESTS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS pg_regress
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})