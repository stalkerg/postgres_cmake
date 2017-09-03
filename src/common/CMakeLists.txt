if(OPENSSL_FOUND)
	include_directories(BEFORE ${OPENSSL_INCLUDE_DIR})
endif()

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
	keywords.c
	md5.c
	ip.c
	base64.c
	saslprep.c
	scram-common.c
)

if(OPENSSL_FOUND)
	set(pgcommon_srv_SRCS ${pgcommon_srv_SRCS} sha2_openssl.c)
else()
	set(pgcommon_srv_SRCS ${pgcommon_srv_SRCS} sha2.c)
endif()

set(pgcommon_SRCS
	${pgcommon_srv_SRCS}
	fe_memutils.c
	file_utils.c
	restricted_token.c
	${PROJECT_SOURCE_DIR}/src/include/parser/gram.h
)

add_library(pgcommon_srv STATIC ${pgcommon_srv_SRCS})
add_library(pgcommon STATIC ${pgcommon_SRCS})

set_source_files_properties(
	${PROJECT_SOURCE_DIR}/src/include/parser/gram.h
	PROPERTIES GENERATED TRUE
)

add_dependencies(pgcommon_srv generate_parser_gram_h)
add_dependencies(pgcommon_srv gen_errorcodes)
add_dependencies(pgcommon gen_errorcodes)

target_compile_definitions(pgcommon PRIVATE -DFRONTEND)

target_compile_definitions(pgcommon PRIVATE -DVAL_CPPFLAGS="${CMAKE_CXX_FLAGS}")
target_compile_definitions(pgcommon PRIVATE -DVAL_CFLAGS="${CMAKE_C_FLAGS} ${COMPILE_DEFINITIONS}")
target_compile_definitions(pgcommon PRIVATE -DVAL_CC="${CMAKE_C_COMPILER}")
target_compile_definitions(pgcommon PRIVATE -DVAL_LDFLAGS="${CMAKE_SHARED_LINKER_FLAGS}")

# because port used in shared libraries
set_property(TARGET pgcommon PROPERTY POSITION_INDEPENDENT_CODE TRUE)

install(TARGETS pgcommon
		ARCHIVE DESTINATION ${LIBDIR})
