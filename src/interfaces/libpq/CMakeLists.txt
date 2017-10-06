include_directories(BEFORE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${PROJECT_SOURCE_DIR}/src/port"
)

if(USE_OPENSSL)
	include_directories(BEFORE ${OPENSSL_INCLUDE_DIR})
	set(pq_ssl_SRCS
		fe-secure-openssl.c
		${PROJECT_SOURCE_DIR}/src/common/sha2_openssl.c
	)
else()
	set(pq_ssl_SRCS ${PROJECT_SOURCE_DIR}/src/common/sha2.c)
endif(USE_OPENSSL)

set(pq_SRCS
	fe-auth.c
	fe-connect.c
	fe-exec.c
	fe-misc.c
	fe-print.c
	fe-lobj.c
	fe-protocol2.c
	fe-protocol3.c
	fe-auth-scram.c
	pqexpbuffer.c
	fe-secure.c
	libpq-events.c

	${PORT_DIR}/chklocale.c
	${PORT_DIR}/inet_net_ntop.c
	${PORT_DIR}/noblock.c
	${PORT_DIR}/pgstrcasecmp.c
	${PORT_DIR}/pqsignal.c
	${PORT_DIR}/thread.c

	${PROJECT_SOURCE_DIR}/src/common/ip.c
	${PROJECT_SOURCE_DIR}/src/common/md5.c
	${PROJECT_SOURCE_DIR}/src/common/base64.c
	${PROJECT_SOURCE_DIR}/src/common/scram-common.c
	${PROJECT_SOURCE_DIR}/src/common/saslprep.c
	${PROJECT_SOURCE_DIR}/src/common/unicode_norm.c
	${MB_UTILS_BACKEND_DIR}/encnames.c
	${MB_UTILS_BACKEND_DIR}/wchar.c

	${fallback_SRCS}
	${pq_ssl_SRCS}
)

if(USE_REPL_SNPRINTF)
	set(pq_SRCS ${pq_SRCS} ${PORT_DIR}/snprintf.c)
endif()

if(HAVE_STRONG_RANDOM)
	set(pq_SRCS ${pq_SRCS} ${PORT_DIR}/pg_strong_random.c)
else()
	set(pq_SRCS ${pq_SRCS} ${PORT_DIR}/erand48.c)
endif()

set(RES_FILES "")
if(WIN32)
	set(pq_SRCS ${pq_SRCS}
		win32.c
		${PORT_DIR}/win32error.c
		${PORT_DIR}/win32setlocale.c
		${PORT_DIR}/pgsleep.c
		${PORT_DIR}/open.c
		${PORT_DIR}/system.c
		${PORT_DIR}/inet_aton.c
		${PORT_DIR}/thread.c
		${PORT_DIR}/getaddrinfo.c
	)
	if (Threads_FOUND)
		set(pq_SRCS ${pq_SRCS} pthread-win32.c)
	endif()
	set(RES_FILES "libpq.rc")
	set(CMAKE_RC_COMPILER_INIT windres)
	ENABLE_LANGUAGE(RC)
	SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif()

add_library(pq SHARED ${pq_SRCS} ${RES_FILES})
target_link_libraries(pq
	${CMAKE_THREAD_LIBS_INIT}
	${LIBSOCKET_LIBRARIES}
	${GSS_LIBS}
)
if(USE_OPENSSL)
	target_link_libraries(pq ${OPENSSL_LIBRARIES})
endif()
if (MSVC)
	gen_def(pq)
endif()
if(CRYPT_LIB)
	target_link_libraries(pq ${CRYPT_LIB})
endif()
if(WIN32)
	target_link_libraries(pq
		#port
		Secur32
		ws2_32
	)
endif()

if(LDAP_FOUND)
	target_link_libraries(pq ${LDAP_LIBRARIES})
endif()

if(USE_OPENSSL)
	target_include_directories(pq PUBLIC ${OPENSSL_INCLUDE_DIR})
endif()


target_compile_definitions(pq PRIVATE -DFRONTEND -DUNSAFE_STAT_OK)
install(TARGETS pq
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(libpq "cs;de;es;fr;it;ja;ko;pl;pt_BR;ru;tr;zh_CN;zh_TW;he;sv")
endif()

#TODO 
#set_target_properties(pq PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports.list")
