macro(MAKE_MO BIN_NAME LANGUAGES)
	set(gmo_files "")
	foreach(lang ${LANGUAGES})
		if (NOT ";${NLS_LANGUAGES};" MATCHES ";${lang};")
			continue()
		endif()
		set(_gmoFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo)
		add_custom_command(OUTPUT ${_gmoFile}
			COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${_gmoFile} po/${lang}.po
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
			DEPENDS po/${lang}.po
		)
		install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${lang}.gmo DESTINATION share/locale/${lang}/LC_MESSAGES/ RENAME ${BIN_NAME}-${POSTGRES_MAJOR_VERSION}.${POSTGRES_MINOR_VERSION}.mo)
		if(NOT TARGET pofiles)
			add_custom_target(pofiles)
		endif()
		add_custom_target(${BIN_NAME}-mofile-${lang} ALL DEPENDS ${_gmoFile})
		add_dependencies(pofiles ${BIN_NAME}-mofile-${lang})
		set(gmo_files "${gmo_files};${_gmoFile}")
	endforeach()
endmacro()
