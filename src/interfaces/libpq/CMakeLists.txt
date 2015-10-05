include_directories(
	"${PROJECT_SOURCE_DIR}/src/port"
)

if(USE_OPENSSL)
	set(pq_ssl_SRCS fe-secure-openssl.c)
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
	pqexpbuffer.c
	fe-secure.c
	libpq-events.c

	${PORT_DIR}/chklocale.c
	${PORT_DIR}/inet_net_ntop.c
	${PORT_DIR}/noblock.c
	${PORT_DIR}/pgstrcasecmp.c
	${PORT_DIR}/pqsignal.c
	${PORT_DIR}/thread.c

	${PQ_BACKEND_DIR}/ip.c
	${PQ_BACKEND_DIR}/md5.c
	${MB_UTILS_BACKEND_DIR}/encnames.c
	${MB_UTILS_BACKEND_DIR}/wchar.c

	${fallback_SRCS}
	${pq_ssl_SRCS}
)


add_library(pq SHARED ${pq_SRCS})
target_link_libraries(pq
	${OPENSSL_LIBRARIES}
)

target_compile_definitions(pq PRIVATE -DFRONTEND -DUNSAFE_STAT_OK)
set_target_properties(pq PROPERTIES COMPILE_FLAGS ${PTHREAD_CFLAGS})
install(TARGETS pq
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

#TODO 
#set_target_properties(pq PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports.list")