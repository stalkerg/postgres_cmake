if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
	find_program(NM_EXECUTABLE nm)
endif()

macro(GEN_DEF NAME)
	if(MSVC)
		set_target_properties(${NAME} PROPERTIES ENABLE_EXPORTS 1)
		set_target_properties(${NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
		#SET_TARGET_PROPERTIES(${NAME} PROPERTIES LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_BINARY_DIR}/${NAME}.def\" ")
		#add_custom_command(
		#	TARGET ${NAME}
		#	PRE_LINK
		#	#COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.def
		#	#OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.def
		#	COMMAND ${PERL_EXECUTABLE} ${PROJECT_SOURCE_DIR}/src/tools/msvc/gendef.pl ${CMAKE_CURRENT_BINARY_DIR}/$<$<BOOL:${CMAKE_BUILD_TYPE}>:${CMAKE_FILES_DIRECTORY}>${NAME}.dir/${CMAKE_CFG_INTDIR} ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.def ${CMAKE_VS_PLATFORM_NAME}
		#	COMMENT " Gen defs "
		#)
		#set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.def)
	elseif(CMAKE_SYSTEM_NAME STREQUAL "AIX" AND CMAKE_C_COMPILER_ID MATCHES "xlc")
		# TODO for xls
		SET_TARGET_PROPERTIES(${NAME} PROPERTIES LINK_FLAGS "-Wl,-bE:${CMAKE_CURRENT_BINARY_DIR}/${NAME}.exp")
		if("${CMAKE_C_FLAGS}" MATCHES "aix64")
			set(FLAG_64 "-X64")
		endif()
		add_custom_command(
			TARGET ${NAME}
			PRE_LINK
			COMMAND ${NM_EXECUTABLE} ${FLAG_64} -BCg ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${NAME}.dir/*.o | egrep ' [TDB] ' | sed -e 's/.* //' | egrep -v '\\$$' | sed -e 's/^[.]//' | sort | uniq > ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.exp
			COMMENT " Gen exports "
		)
	endif()
endmacro(GEN_DEF NAME)
