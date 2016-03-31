macro(REGRESS_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)
	add_custom_target(${TARGET_NAME}_installcheck_tmp
		COMMAND ${pg_regress_check_tmp} ${REGRESS_OPTS} --dlpath=${tmp_check_folder}${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${REGRESS_FILES}
		DEPENDS tablespace-setup
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	add_custom_target(${TARGET_NAME}_check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
		COMMAND make install DESTDIR=${tmp_check_folder}
		COMMAND make ${TARGET_NAME}_installcheck_tmp DESTDIR=${tmp_check_folder}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
endmacro(REGRESS_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)
