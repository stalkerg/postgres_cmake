include_directories(
		"${PROJECT_SOURCE_DIR}/src/include/libpq"
		"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
		"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

set(tzdata_files
		${CMAKE_CURRENT_SOURCE_DIR}/data/tzdata.zi
		)

if(MSVC)
	foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
		string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
		set_target_properties(zic PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_BINARY_DIR} )
		set_target_properties(zic PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_BINARY_DIR} )
		set_target_properties(zic PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_CURRENT_BINARY_DIR} )
	endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
endif(MSVC)

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

if(WITH_SYSTEM_TZDATA)
	add_compile_options(-DSYSTEMTZDIR="${WITH_SYSTEM_TZDATA}")
else()
	if(MINGW)
		string(SUBSTRING ${PGSHAREDIR} 1 1 CHAR_POS)
		if(CHAR_POS STREQUAL ":")
			string(SUBSTRING ${PGSHAREDIR} 2 -1 PGSHAREDIR_ZIC)
		else()
			set(PGSHAREDIR_ZIC ${PGSHAREDIR})
		endif()
	else()
		set(PGSHAREDIR_ZIC ${PGSHAREDIR})
	endif()

	add_executable(zic zic.c)
	target_link_libraries(zic pgport)

	set(ZIC ${CMAKE_CURRENT_BINARY_DIR}/zic${CMAKE_EXECUTABLE_SUFFIX})
	set(POSIXRULES "US/Eastern")

	install(CODE "execute_process(COMMAND ${ZIC} -d ${CMAKE_CURRENT_BINARY_DIR}/timezone -p ${POSIXRULES} ${tzdata_files})")
	install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/timezone DESTINATION ${PGSHAREDIR_ZIC})
endif()
