if(USE_SSE42_CRC32C)
	SET(PG_CRC32C_OBJS pg_crc32c_sse42.c)
elseif(USE_SSE42_CRC32C_WITH_RUNTIME_CHECK)
	SET(PG_CRC32C_OBJS
		pg_crc32c_sse42.c
		pg_crc32c_sb8.c
		pg_crc32c_choose.c
	)
elseif(USE_SLICING_BY_8_CRC32C)
	SET(PG_CRC32C_OBJS pg_crc32c_sb8.c)
endif()

set_source_files_properties(pg_crc32c_sse42.c PROPERTIES COMPILE_FLAGS ${CFLAGS_SSE42})

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
	pg_putenv_proxy.c
	${fallback_SRCS}
	${PG_CRC32C_OBJS}
)

if(NOT HAVE_FLS)
	set(port_SRCS
		${port_SRCS}
		fls.c
	)
endif(NOT HAVE_FLS)

if(NOT HAVE_ISINF)
	set(port_SRCS
		${port_SRCS}
		isinf.c
	)
endif()

if(NOT HAVE_MKDTEMP)
	set(port_SRCS
		${port_SRCS}
		mkdtemp.c
	)
endif()

if(NOT HAVE_GETOPT)
	set(port_SRCS
		${port_SRCS}
		getopt.c
	)
endif()

if(NOT (HAVE_STRUCT_OPTION AND HAVE_GETOPT_LONG))
	set(port_SRCS
		${port_SRCS}
		getopt_long.c
	)
endif()

if(NOT HAVE_GETTIMEOFDAY)
	set(port_SRCS
		${port_SRCS}
		gettimeofday.c
	)
endif()

if(MSVC)
	set(port_SRCS
		${port_SRCS}
		random.c
		srandom.c
		dirmod.c
		kill.c
		open.c
		dirent.c
		getrusage.c
		system.c
		win32security.c
		win32setlocale.c
		win32env.c
		win32error.c
	)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	# Solaris' getopt() doesn't do what we want for long options, so always use
	# our version on that platform.
	set(port_SRCS
		${port_SRCS}
		getopt.c
	)
endif()

if(NOT HAVE_CRYPT)
	set(port_SRCS
		${port_SRCS}
		crypt.c
	)
endif()

add_library(port_srv STATIC ${port_SRCS})
add_library(port STATIC ${port_SRCS})

add_dependencies(port_srv gen_errorcodes)
add_dependencies(port gen_errorcodes)

# because port used in shared libraries
set_property(TARGET port PROPERTY POSITION_INDEPENDENT_CODE TRUE)


target_compile_definitions(port PRIVATE -DFRONTEND)
set_source_files_properties(thread.c PROPERTIES COMPILE_FLAGS ${PTHREAD_CFLAGS})
install(TARGETS port
		ARCHIVE DESTINATION ${LIBDIR})
