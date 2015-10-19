include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(gen_errorcodes_depend
	${PROJECT_SOURCE_DIR}/src/backend/utils/errcodes.txt
	${PROJECT_SOURCE_DIR}/src/backend/utils/generate-errcodes.pl
)
set(gen_errorcodes_output
	${PROJECT_SOURCE_DIR}/src/include/utils/errcodes.h
)

add_subdirectory(port)
add_subdirectory(common)
add_subdirectory(interfaces/libpq)
add_subdirectory(bin/psql)
add_subdirectory(bin/pg_ctl)
add_subdirectory(bin/initdb)

add_subdirectory(backend)
add_subdirectory(timezone)