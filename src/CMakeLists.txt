include_directories("${PROJECT_SOURCE_DIR}/src/include")

add_subdirectory(port)
add_subdirectory(common)
add_subdirectory(interfaces/libpq)
add_subdirectory(bin/psql)
add_subdirectory(bin/pg_ctl)
add_subdirectory(bin/initdb)

add_subdirectory(backend)