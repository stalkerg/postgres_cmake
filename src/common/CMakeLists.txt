set(pgcommon_srv_SRCS
	exec.c
	pg_lzcompress.c
	pgfnames.c
	psprintf.c
	relpath.c
	rmtree.c
	string.c
	username.c
	wait_error.c
)

set(pgcommon_SRCS
	${pgcommon_srv_SRCS}
	fe_memutils.c
	restricted_token.c
)


add_library(pgcommon_srv STATIC ${pgcommon_srv_SRCS})
add_library(pgcommon STATIC ${pgcommon_SRCS})

target_compile_definitions(pgcommon PRIVATE -DFRONTEND)