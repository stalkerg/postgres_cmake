#TODO compile timezones

include_directories(
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)


set(tzdata 
	${CMAKE_CURRENT_SOURCE_DIR}/data/africa
	${CMAKE_CURRENT_SOURCE_DIR}/data/antarctica
	${CMAKE_CURRENT_SOURCE_DIR}/data/asia
	${CMAKE_CURRENT_SOURCE_DIR}/data/australasia
	${CMAKE_CURRENT_SOURCE_DIR}/data/europe
	${CMAKE_CURRENT_SOURCE_DIR}/data/northamerica
	${CMAKE_CURRENT_SOURCE_DIR}/data/southamerica
	${CMAKE_CURRENT_SOURCE_DIR}/data/pacificnew
	${CMAKE_CURRENT_SOURCE_DIR}/data/etcetera
	${CMAKE_CURRENT_SOURCE_DIR}/data/factory
	${CMAKE_CURRENT_SOURCE_DIR}/data/backward
	${CMAKE_CURRENT_SOURCE_DIR}/data/systemv
)

add_executable(zic
	zic.c
	ialloc.c
	scheck.c
	localtime.c
)
target_link_libraries(zic
	port
)

install(FILES 
	tznames/Africa.txt
	tznames/America.txt
	tznames/Antarctica.txt
	tznames/Asia.txt
	tznames/Atlantic.txt
	tznames/Australia.txt
	tznames/Etc.txt
	tznames/Europe.txt
	tznames/Indian.txt
	tznames/Pacific.txt
	tznames/Default
	tznames/Australia
	tznames/India
DESTINATION ${PGSHAREDIR}/timezonesets)

function(JOIN VALUES GLUE OUTPUT)
  string (REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
  string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

JOIN("${tzdata}" " " tzdata_string)

install(
	CODE "execute_process(COMMAND ${CMAKE_CURRENT_BINARY_DIR}/\${CMAKE_INSTALL_CONFIG_NAME}/zic${CMAKE_EXECUTABLE_SUFFIX} -d \"${PGSHAREDIR}/timezone\" -p \"US/Eastern\" ${tzdata_string})"
)
