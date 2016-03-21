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
	controldata_utils.c
	config_info.c
)

set(pgcommon_SRCS
	${pgcommon_srv_SRCS}
	fe_memutils.c
	restricted_token.c
)

add_library(pgcommon_srv STATIC ${pgcommon_srv_SRCS})
add_library(pgcommon STATIC ${pgcommon_SRCS})

add_dependencies(pgcommon_srv gen_errorcodes)
add_dependencies(pgcommon gen_errorcodes)

target_compile_definitions(pgcommon PRIVATE -DFRONTEND)
# because port used in shared libraries
set_property(TARGET pgcommon PROPERTY POSITION_INDEPENDENT_CODE TRUE)
