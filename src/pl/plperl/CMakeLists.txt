if(WIN32)
	add_definitions(-DPLPERL_HAVE_UID_GID)
endif()

execute_process(
	COMMAND ${PERL_EXECUTABLE} -e "use List::Util qw(first); print first { -r \"$_/ExtUtils/xsubpp\" } @INC"
	OUTPUT_VARIABLE XSUBPPDIR
)
execute_process(
	COMMAND ${PERL_EXECUTABLE} -V:usemultiplicity
	OUTPUT_VARIABLE PERL_MULTIPLICITY
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/SPI.c"
	MAIN_DEPENDENCY SPI.xs plperl_helpers.h
	COMMAND ${PERL_EXECUTABLE} ${XSUBPPDIR}/ExtUtils/xsubpp -typemap ${PERL_PRIVLIB}/ExtUtils/typemap SPI.xs > "${CMAKE_CURRENT_SOURCE_DIR}/SPI.c"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/Util.c"
	MAIN_DEPENDENCY Util.xs plperl_helpers.h
	COMMAND ${PERL_EXECUTABLE} ${XSUBPPDIR}/ExtUtils/xsubpp -typemap ${PERL_PRIVLIB}/ExtUtils/typemap Util.xs > "${CMAKE_CURRENT_SOURCE_DIR}/Util.c"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/perlchunks.h"
	MAIN_DEPENDENCY plc_perlboot.pl plc_trusted.pl
	COMMAND ${PERL_EXECUTABLE} text2macro.pl "--strip='^(\\#.*|\\s*)$$'" plc_perlboot.pl plc_trusted.pl > "${CMAKE_CURRENT_SOURCE_DIR}/perlchunks.h"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(
	OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/plperl_opmask.h"
	MAIN_DEPENDENCY plperl_opmask.pl
	COMMAND ${PERL_EXECUTABLE} plperl_opmask.pl "${CMAKE_CURRENT_SOURCE_DIR}/plperl_opmask.h"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(plname "plperl")

set(plperl_SRCS
	"${CMAKE_CURRENT_SOURCE_DIR}/perlchunks.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/plperl_opmask.h"
	plperl.c
	SPI.c
	Util.c
)

include_directories(${PERL_INCLUDE_PATH})
add_library(${plname} ${PLUGIN_TYPE} ${plperl_SRCS})
target_link_libraries(${plname} postgres ${PERL_LIBRARY} ${PERL_LDFLAGS})
if (MSVC)
	gen_def(${plname})
	target_link_libraries(${plname} pgport)
endif()
set_target_properties(${plname} PROPERTIES PREFIX "")
add_dependencies(${plname} postgres)

install(TARGETS ${plname}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
		plperl.control
		plperl--1.0.sql
		plperl--unpackaged--1.0.sql
		plperlu.control
		plperlu--1.0.sql
		plperlu--unpackaged--1.0.sql
		DESTINATION ${PGSHAREDIR}/extension)
install(FILES
	plperl.h
	ppport.h
	DESTINATION ${INCLUDEDIRSERVER})

set(PL_TESTDB pl_regression)
set(REGRESS_OPTS --dbname=${PL_TESTDB} --load-extension=plperl --load-extension=plperlu)
set(plregres_files
	plperl
	plperl_lc
	plperl_trigger
	plperl_shared
	plperl_elog
	plperl_util
	plperl_init
	plperlu
	plperl_array
)

if(PERL_MULTIPLICITY STREQUAL "usemultiplicity='define'")
	set(plregres_files ${plregres_files} plperl_plperlu)
endif()

REGRESS_CHECK(plperl "${REGRESS_OPTS}" "${plregres_files}")

if(ENABLE_NLS)
	MAKE_MO(plperl "cs;de;es;fr;it;ja;ko;pl;pt_BR;ro;ru;sv;tr;zh_CN;zh_TW")
endif()
