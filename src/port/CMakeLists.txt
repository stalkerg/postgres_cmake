#include_directories(
#	"${PROJECT_SOURCE_DIR}/src/include/libpq"
#	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
#	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
#)

if(NOT MSVC)
	IF(ENABLE_SSE42)
		SET(PG_CRC32C_OBJS pg_crc32c_sse42.c)
	ELSE(ENABLE_SSE42)
		SET(PG_CRC32C_OBJS pg_crc32c_sb8.c)
	ENDIF(ENABLE_SSE42)
endif()

add_custom_command(
	MAIN_DEPENDENCY ${gen_errorcodes_depend}
	OUTPUT ${gen_errorcodes_output}
	COMMAND ${PERL_EXECUTABLE} generate-errcodes.pl errcodes.txt > ${gen_errorcodes_output}
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/backend/utils
)

set(port_SRCS
	chklocale.c
	erand48.c
	inet_net_ntop.c
	noblock.c
	path.c
	pgcheckdir.c
	pgmkdirp.c
	pgsleep.c
	pgstrcasecmp.c
	pqsignal.c
	qsort.c
	qsort_arg.c
	quotes.c
	sprompt.c
	tar.c
	thread.c
	${fallback_SRCS}
	${PG_CRC32C_OBJS}
	${gen_errorcodes_output}
)

if(NOT HAVE_FLS)
	set(port_SRCS
		${port_SRCS}
		fls.c
	)
endif(NOT HAVE_FLS)

if(MSVC)
	set(port_SRCS
		${port_SRCS}
		gettimeofday.c
		random.c
		srandom.c
		dirmod.c
		getopt.c
		getopt_long.c
		kill.c
		open.c
		dirent.c
		getrusage.c
		win32security.c
		win32setlocale.c
		win32env.c
		pg_crc32c_sse42.c
		pg_crc32c_sb8.c
		pg_crc32c_choose.c
	)
endif()

add_library(port_srv STATIC ${port_SRCS})
add_library(port STATIC ${port_SRCS})

target_compile_definitions(port PRIVATE -DFRONTEND)
set_source_files_properties(thread.c PROPERTIES COMPILE_FLAGS ${PTHREAD_CFLAGS})