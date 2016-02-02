include_directories(BEFORE
	"${CMAKE_CURRENT_SOURCE_DIR}"
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
	set(RES_FILES "libpq.rc")
	set(CMAKE_RC_COMPILER_INIT windres)
	ENABLE_LANGUAGE(RC)
	SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif()

add_library(pq SHARED ${pq_SRCS} ${RES_FILES})
target_link_libraries(pq
	${OPENSSL_LIBRARIES}
)
if(WIN32)
	target_link_libraries(pq
		#port
		Secur32
		ws2_32
	)
endif()

if(MSVC)
	SET_TARGET_PROPERTIES(pq PROPERTIES LINK_FLAGS "/DEF:\"${CMAKE_CURRENT_BINARY_DIR}/pq.def\" ")
	add_custom_command(
		TARGET pq
		PRE_LINK
		COMMAND ${PERL_EXECUTABLE} ${PROJECT_SOURCE_DIR}/src/tools/msvc/gendef.pl ${CMAKE_CURRENT_BINARY_DIR}/$<$<BOOL:${CMAKE_BUILD_TYPE}>:${CMAKE_FILES_DIRECTORY}>pq.dir/${CMAKE_CFG_INTDIR} ${CMAKE_CURRENT_BINARY_DIR}/pq.def ${CMAKE_VS_PLATFORM_NAME}
		#Need because pgwin32_putenv build as __cdel
		#COMMAND ${CMAKE_COMMAND} -E echo_append "  pgwin32_putenv" >>  ${CMAKE_CURRENT_BINARY_DIR}/pq.def
		COMMENT " Gen defs "
    )
endif()

if(USE_OPENSSL)
	target_include_directories(pq PUBLIC ${OPENSSL_INCLUDE_DIR})
endif()
target_compile_definitions(pq PRIVATE -DFRONTEND -DUNSAFE_STAT_OK)
set_target_properties(pq PROPERTIES COMPILE_FLAGS ${PTHREAD_CFLAGS})
install(TARGETS pq
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

#TODO 
#set_target_properties(pq PROPERTIES LINK_FLAGS "-Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/exports.list")