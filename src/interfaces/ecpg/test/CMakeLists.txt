include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/port"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
	"${PROJECT_SOURCE_DIR}/src/test/regress"
	"${PROJECT_SOURCE_DIR}/src/interfaces/ecpg/include"
)

add_executable(pg_ecpg_regress
	${PROJECT_SOURCE_DIR}/src/test/regress/pg_regress.c
	pg_regress_ecpg.c
)

target_link_libraries(pg_ecpg_regress
	pq
	pgcommon
	pgport
)

target_compile_definitions(pg_ecpg_regress PRIVATE
	-DHOST_TUPLE="${HOST_TUPLE}"
	-DSHELLPROG="$ENV{SHELL}"
)

set(connect_tests
	test1
	test2
	test3
	test4
	test5
)

set(compat_informix_tests
	test_informix
	test_informix2
	dec_test
	rfmtdate
	rfmtlong
	rnull
	sqlda
	describe
	charfuncs
)

set(pgtypeslib_tests
	dt_test
	dt_test2
	num_test
	num_test2
	nan_test
)

set(preproc_tests
	array_of_struct
	autoprep
	comment
	cursor
	define
	init
	strings
	outofscope
	type
	variable
	whenever
	pointer_to_struct
)

set(sql_tests
	array
	binary
	code100
	copystdout
	define
	desc
	sqlda
	describe
	dyntest
	dynalloc
	dynalloc2
	execute
	fetch
	func
	indicators
	oldexec
	parser
	quote
	show
	insupd
	twophase
)

set(thread_tests
	thread_implicit
	thread
	prep
	descriptor
	alloc
)

set(ecpg_preproc_bin ${CMAKE_BINARY_DIR}/src/interfaces/ecpg/preproc/ecpg${CMAKE_EXECUTABLE_SUFFIX})

macro(ECPG_CHECK TARGET_LIST FOLDER PREPROC_OPTS)
	foreach(test ${TARGET_LIST})
		if(${test} STREQUAL "rnull")
			set(PREPROC_OPTS2 "-r;no_indicator")
		elseif(${test} STREQUAL "array_of_struct")
			set(PREPROC_OPTS2 "-c")
		elseif(${test} STREQUAL "pointer_to_struct")
			set(PREPROC_OPTS2 "-c")
		elseif(${test} STREQUAL "autoprep")
			set(PREPROC_OPTS2 "-r;prepare")
		elseif(${test} STREQUAL "strings")
			set(PREPROC_OPTS2 "-i")
		elseif(${test} STREQUAL "oldexec")
			set(PREPROC_OPTS2 "-r;questionmarks")
		else()
			set(PREPROC_OPTS2 "")
		endif()
		add_custom_command(
			DEPENDS ecpg_preproc
			OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/${FOLDER}/${test}.c
			COMMAND ${ecpg_preproc_bin} --regression ${PREPROC_OPTS} ${PREPROC_OPTS2} -I${PROJECT_SOURCE_DIR}/src/interfaces/ecpg/include -I${PROJECT_SOURCE_DIR}/src/interfaces/ecpg/test/${FOLDER} ${FOLDER}/${test}.pgc
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
		add_executable(ecpg_regress_${FOLDER}_${test}
			${CMAKE_CURRENT_SOURCE_DIR}/${FOLDER}/${test}.c
		)
		target_link_libraries(ecpg_regress_${FOLDER}_${test}
			ecpg
			ecpg_compat
			pgtypes
			pq
		)
		set_target_properties(ecpg_regress_${FOLDER}_${test}
			PROPERTIES
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${FOLDER}"
			OUTPUT_NAME ${test}
		)
	endforeach()
endmacro(ECPG_CHECK TARGET_LIST FOLDER PREPROC_OPTS)

ecpg_check("${compat_informix_tests}" "compat_informix" "-C;INFORMIX")
ecpg_check("${connect_tests}" "connect" "")
ecpg_check("${pgtypeslib_tests}" "pgtypeslib" "")
ecpg_check("${preproc_tests}" "preproc" "")
ecpg_check("${sql_tests}" "sql" "")
ecpg_check("${thread_tests}" "thread" "")


if(CMAKE_GENERATOR STREQUAL "Ninja")
	add_custom_target(ecpg_check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
		COMMAND DESTDIR=${tmp_check_folder} ${check_make_command} install
		COMMAND DESTDIR=${tmp_check_folder} ${check_make_command} ecpg_installcheck_tmp
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
else()
	add_custom_target(ecpg_check
		COMMAND ${CMAKE_COMMAND} -E remove_directory ${tmp_check_folder}
		COMMAND ${check_make_command} install DESTDIR=${tmp_check_folder}
		COMMAND ${check_make_command} ecpg_installcheck_tmp DESTDIR=${tmp_check_folder}
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
	)
endif()

set(REGRESS_OPTS "${REGRESS_OPTS};--dbname=ecpg1_regression,ecpg2_regression;--create-role=regress_ecpg_user1,regress_ecpg_user2")

add_custom_target(ecpg_installcheck
	COMMAND ${pg_ecpg_regress_check} ${REGRESS_OPTS} --schedule=${CMAKE_CURRENT_SOURCE_DIR}/ecpg_schedule --dlpath=$ENV{DESTDIR}${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${EXTRA_TESTS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(ecpg_installcheck_tmp
	COMMAND ${pg_ecpg_regress_check_tmp} ${REGRESS_OPTS} --schedule=${CMAKE_CURRENT_SOURCE_DIR}/ecpg_schedule --dlpath=${tmp_check_folder}${LIBDIR} ${MAXCONNOPT} ${TEMP_CONF} ${EXTRA_TESTS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
