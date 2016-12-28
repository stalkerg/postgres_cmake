set(tmp_check_folder ${CMAKE_BINARY_DIR}/src/test/regress/tmp_install)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(env_cmd
		${CMAKE_COMMAND} -E env
		"DYLD_LIBRARY_PATH=$ENV{DESTDIR}${LIBDIR}:$$DYLD_LIBRARY_PATH"
		"PATH=$ENV{DESTDIR}${PGBINDIR}:$$PATH"
	)
	set(tmp_env_cmd
		${CMAKE_COMMAND} -E env
		"DYLD_LIBRARY_PATH=${tmp_check_folder}${LIBDIR}:$$DYLD_LIBRARY_PATH"
		"PATH=${tmp_check_folder}${PGBINDIR}:$$PATH"
	)
elseif(CMAKE_SYSTEM_NAME STREQUAL "AIX")
	set(env_cmd
		${CMAKE_COMMAND} -E env
		"LIBPATH=$ENV{DESTDIR}${LIBDIR}:$LIBPATH"
		"PATH=$ENV{DESTDIR}${PGBINDIR}:$$PATH"
	)
	set(tmp_env_cmd
		${CMAKE_COMMAND} -E env
		"LIBPATH=${tmp_check_folder}${LIBDIR}:$LIBPATH"
		"PATH=${tmp_check_folder}${PGBINDIR}:$$PATH"
	)
elseif(MSVC)
	# We really need add PATH but only for ecpg
	set(env_cmd "")
else()
	if (CMAKE_VERSION VERSION_GREATER "3.2.0")
		set(env_cmd
			${CMAKE_COMMAND} -E env
			"LD_LIBRARY_PATH=$ENV{DESTDIR}${LIBDIR}:$$LD_LIBRARY_PATH"
			"PATH=$ENV{DESTDIR}${PGBINDIR}:$$PATH"
		)
		set(tmp_env_cmd
			${CMAKE_COMMAND} -E env
			"LD_LIBRARY_PATH=${tmp_check_folder}${LIBDIR}:$$LD_LIBRARY_PATH"
			"PATH=${tmp_check_folder}${PGBINDIR}:$$PATH"
		)
	else()
		set(env_cmd
			export "LD_LIBRARY_PATH=$ENV{DESTDIR}${LIBDIR}:$$LD_LIBRARY_PATH"
				   "PATH=$ENV{DESTDIR}${PGBINDIR}:$$PATH" &&
		)
		set(tmp_env_cmd
			export "LD_LIBRARY_PATH=${tmp_check_folder}${LIBDIR}:$$LD_LIBRARY_PATH"
				   "PATH=${tmp_check_folder}${PGBINDIR}:$$PATH" &&
		)
	endif()
endif()

if(MSVC OR MSYS OR MINGW OR CMAKE_GENERATOR STREQUAL Xcode)
	#Need rewrite
	set(pre_pg_regress_check
		${PGBINDIR}/pg_regress${CMAKE_EXECUTABLE_SUFFIX}
		--inputdir="${CMAKE_SOURCE_DIR}/src/test/regress"
		--temp-instance="tmp_check"
		--encoding=UTF8
		--no-locale
	)
	set(pre_pg_isolation_regress_check
		${PGBINDIR}/pg_isolation_regress${CMAKE_EXECUTABLE_SUFFIX}
		--inputdir="${CMAKE_SOURCE_DIR}/src/test/isolation"
		--temp-instance="tmp_check"
		--encoding=UTF8
		--no-locale
	)
else()
	set(pre_pg_regress_check
		${CMAKE_BINARY_DIR}/src/test/regress/${CMAKE_INSTALL_CONFIG_NAME}/pg_regress${CMAKE_EXECUTABLE_SUFFIX}
		--inputdir="${CMAKE_SOURCE_DIR}/src/test/regress"
		--temp-instance="tmp_check"
		--encoding=UTF8
		--no-locale
	)
	set(pre_pg_isolation_regress_check
		${CMAKE_BINARY_DIR}/src/test/isolation/${CMAKE_INSTALL_CONFIG_NAME}/pg_isolation_regress${CMAKE_EXECUTABLE_SUFFIX}
		--inputdir="${CMAKE_SOURCE_DIR}/src/test/isolation"
		--temp-instance="tmp_check"
		--encoding=UTF8
		--no-locale
	)
	set(pre_pg_ecpg_regress_check
		${CMAKE_BINARY_DIR}/src/interfaces/ecpg/test/${CMAKE_INSTALL_CONFIG_NAME}/pg_ecpg_regress${CMAKE_EXECUTABLE_SUFFIX}
		--inputdir="${CMAKE_BINARY_DIR}/src/interfaces/ecpg/test"
		--temp-instance="tmp_check"
		--encoding=UTF8
		--no-locale
	)
endif()

set(pg_regress_check
	${env_cmd}
	${pre_pg_regress_check}
	--bindir=$ENV{DESTDIR}${PGBINDIR}
)

set(pg_regress_check_tmp
	${tmp_env_cmd}
	${pre_pg_regress_check}
	--bindir=${tmp_check_folder}${PGBINDIR}
)

set(pg_isolation_regress_check
	${env_cmd}
	${pre_pg_isolation_regress_check}
	--bindir=$ENV{DESTDIR}${PGBINDIR}
)

set(pg_isolation_regress_check_tmp
	${tmp_env_cmd}
	${pre_pg_isolation_regress_check}
	--bindir=${tmp_check_folder}${PGBINDIR}
)

set(pg_ecpg_regress_check
	${env_cmd}
	${pre_pg_ecpg_regress_check}
	--bindir=$ENV{DESTDIR}${PGBINDIR}
)

set(pg_ecpg_regress_check_tmp
	${tmp_env_cmd}
	${pre_pg_ecpg_regress_check}
	--bindir=${tmp_check_folder}${PGBINDIR}
)

if(MAX_CONNECTIONS)
	set(MAXCONNOPT "${MAXCONNOPT} --max-connections=${MAX_CONNECTIONS}")
endif()

if(TEMP_CONFIG)
	set(TEMP_CONF "${TEMP_CONF} --temp-config=${TEMP_CONFIG}")
endif()


set(TAP_FLAGS "-I;${CMAKE_SOURCE_DIR}/src/test/perl/;--verbose")

if(CMAKE_GENERATOR STREQUAL "Ninja")
	set(check_make_command "ninja")
else()
	set(check_make_command ${CMAKE_MAKE_PROGRAM})
endif()

macro(REGRESS_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)
	add_custom_target(${TARGET_NAME}_installcheck_tmp
		COMMAND ${pg_regress_check_tmp} --inputdir="${CMAKE_CURRENT_SOURCE_DIR}" --dbname=${TARGET_NAME}_regress ${REGRESS_OPTS} --dlpath=${tmp_check_folder}${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${REGRESS_FILES}
		DEPENDS tablespace-setup
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
	
	add_custom_target(${TARGET_NAME}_installcheck
		COMMAND ${pg_regress_check} --inputdir="${CMAKE_CURRENT_SOURCE_DIR}" --dbname=${TARGET_NAME}_regress ${REGRESS_OPTS} --dlpath=$ENV{DESTDIR}${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${REGRESS_FILES}
		DEPENDS tablespace-setup
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	if(CMAKE_GENERATOR STREQUAL "Ninja")
		add_custom_target(${TARGET_NAME}_check
			COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
			COMMAND DESTDIR=${tmp_check_folder} ${check_make_command} install
			COMMAND DESTDIR=${tmp_check_folder} ${check_make_command} ${TARGET_NAME}_installcheck_tmp
			DEPENDS tablespace-setup
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		)
	else()
		add_custom_target(${TARGET_NAME}_check
			COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
			COMMAND ${check_make_command} install DESTDIR=${tmp_check_folder}
			COMMAND ${check_make_command} ${TARGET_NAME}_installcheck_tmp DESTDIR=${tmp_check_folder}
			DEPENDS tablespace-setup
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		)
	endif()
	
	CMAKE_SET_TARGET_FOLDER(${TARGET_NAME}_installcheck_tmp tests/tmp)
	CMAKE_SET_TARGET_FOLDER(${TARGET_NAME}_installcheck "tests/install")
	CMAKE_SET_TARGET_FOLDER(${TARGET_NAME}_check tests)
endmacro(REGRESS_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)

macro(ISOLATION_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)
	add_custom_target(${TARGET_NAME}_isolation_installcheck_tmp
		COMMAND ${pg_isolation_regress_check_tmp} --inputdir="${CMAKE_CURRENT_SOURCE_DIR}" --dbname=${TARGET_NAME}_regress ${REGRESS_OPTS} --dlpath=${tmp_check_folder}${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${REGRESS_FILES}
		DEPENDS pg_isolation_regress
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	add_custom_target(${TARGET_NAME}_isolation_installcheck
		COMMAND ${pg_isolation_regress_check} --inputdir="${CMAKE_CURRENT_SOURCE_DIR}" --dbname=${TARGET_NAME}_regress ${REGRESS_OPTS} --dlpath==$ENV{DESTDIR}${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${REGRESS_FILES}
		DEPENDS pg_isolation_regress
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	add_custom_target(${TARGET_NAME}_isolation_check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
		COMMAND ${check_make_command} install DESTDIR=${tmp_check_folder}
		COMMAND ${check_make_command} ${TARGET_NAME}_isolation_installcheck_tmp DESTDIR=${tmp_check_folder}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
	CMAKE_SET_TARGET_FOLDER(${TARGET_NAME}_isolation_installcheck_tmp tests/tmp)
	CMAKE_SET_TARGET_FOLDER(${TARGET_NAME}_check tests)
endmacro(ISOLATION_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)

macro(TAP_CHECK TARGET_NAME OPTS REGRESS_FILES)
	set(TAP_TMP_CMD
		${CMAKE_COMMAND} -E env
		TESTDIR="${CMAKE_CURRENT_SOURCE_DIR}" PGPORT="65432" PG_REGRESS="${CMAKE_BINARY_DIR}/src/test/regress/${CMAKE_INSTALL_CONFIG_NAME}/pg_regress${CMAKE_EXECUTABLE_SUFFIX}"
		${PROVE}
	)
	set(TAP_CMD
		${CMAKE_COMMAND} -E env
		PATH="$ENV{DESTDIR}${BINDIR}:$$PATH" TESTDIR="${CMAKE_CURRENT_SOURCE_DIR}" PGPORT="65432" PG_REGRESS="${CMAKE_BINARY_DIR}/src/test/regress/${CMAKE_INSTALL_CONFIG_NAME}/pg_regress${CMAKE_EXECUTABLE_SUFFIX}"
		${PROVE}
	)
	add_custom_target(${TARGET_NAME}_installcheck_tmp
		COMMAND ${tmp_env_cmd} ${TAP_TMP_CMD} ${OPTS} ${REGRESS_FILES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	add_custom_target(${TARGET_NAME}_installcheck
		COMMAND ${env_cmd} ${TAP_CMD} ${OPTS} ${REGRESS_FILES}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	if(CMAKE_GENERATOR STREQUAL "Ninja")
		add_custom_target(${TARGET_NAME}_check
			COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
			COMMAND DESTDIR=${tmp_check_folder} ${check_make_command} install
			COMMAND DESTDIR=${tmp_check_folder} ${check_make_command} ${TARGET_NAME}_installcheck_tmp
			DEPENDS tablespace-setup
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		)
	else()
		add_custom_target(${TARGET_NAME}_check
			COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
			COMMAND ${check_make_command} install DESTDIR=${tmp_check_folder}
			COMMAND ${check_make_command} ${TARGET_NAME}_installcheck_tmp DESTDIR=${tmp_check_folder}
			DEPENDS tablespace-setup
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		)
	endif()

	CMAKE_SET_TARGET_FOLDER(${TARGET_NAME}_installcheck_tmp tests/tmp)
	CMAKE_SET_TARGET_FOLDER(${TARGET_NAME}_check tests)
endmacro(TAP_CHECK TARGET_NAME OPTS REGRESS_FILES)

# Contrib macros
macro(CONTRIB_REGRESS_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)
	set(contrib_check_targets ${contrib_check_targets} ${TARGET_NAME}_installcheck_tmp PARENT_SCOPE)
	set(contrib_installcheck_targets ${contrib_installcheck_targets} ${TARGET_NAME}_installcheck PARENT_SCOPE)
	REGRESS_CHECK("${TARGET_NAME}" "${REGRESS_OPTS}" "${REGRESS_FILES}")
endmacro(CONTRIB_REGRESS_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)

macro(CONTRIB_ISOLATION_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)
	set(contrib_check_targets ${contrib_check_targets} ${TARGET_NAME}_isolation_installcheck_tmp PARENT_SCOPE)
	set(contrib_installcheck_targets ${contrib_installcheck_targets} ${TARGET_NAME}_isolation_installcheck PARENT_SCOPE)
	ISOLATION_CHECK("${TARGET_NAME}" "${REGRESS_OPTS}" "${REGRESS_FILES}")
endmacro(CONTRIB_ISOLATION_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)

macro(CONTRIB_TAP_CHECK TARGET_NAME OPTS REGRESS_FILES)
	set(contrib_check_targets ${contrib_check_targets} ${TARGET_NAME}_tap_installcheck_tmp PARENT_SCOPE)
	set(contrib_installcheck_targets ${contrib_installcheck_targets} ${TARGET_NAME}_tap_installcheck PARENT_SCOPE)
	TAP_CHECK("${TARGET_NAME}_tap" "${OPTS}" "${REGRESS_FILES}")
endmacro(CONTRIB_TAP_CHECK TARGET_NAME OPTS REGRESS_FILES)


# Modules macros
macro(MODULES_REGRESS_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)
	set(modules_check_targets ${modules_check_targets} ${TARGET_NAME}_installcheck_tmp PARENT_SCOPE)
	set(modules_installcheck_targets ${modules_installcheck_targets} ${TARGET_NAME}_installcheck PARENT_SCOPE)
	REGRESS_CHECK("${TARGET_NAME}" "${REGRESS_OPTS}" "${REGRESS_FILES}")
endmacro(MODULES_REGRESS_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)

macro(MODULES_ISOLATION_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)
	set(modules_check_targets ${modules_check_targets} ${TARGET_NAME}_isolation_installcheck_tmp PARENT_SCOPE)
	set(modules_installcheck_targets ${modules_installcheck_targets} ${TARGET_NAME}_isolation_installcheck PARENT_SCOPE)
	ISOLATION_CHECK("${TARGET_NAME}" "${REGRESS_OPTS}" "${REGRESS_FILES}")
endmacro(MODULES_ISOLATION_CHECK TARGET_NAME REGRESS_OPTS REGRESS_FILES)

macro(MODULES_TAP_CHECK TARGET_NAME OPTS REGRESS_FILES)
	set(modules_check_targets ${modules_check_targets} ${TARGET_NAME}_tap_installcheck_tmp PARENT_SCOPE)
	set(modules_installcheck_targets ${modules_installcheck_targets} ${TARGET_NAME}_tap_installcheck PARENT_SCOPE)
	TAP_CHECK("${TARGET_NAME}_tap" "${OPTS}" "${REGRESS_FILES}")
endmacro(MODULES_TAP_CHECK TARGET_NAME OPTS REGRESS_FILES)
