cmake_minimum_required(VERSION 2.8)

include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckCSourceCompiles)
include(TestBigEndian)
include(CheckStructHasMember)

project(Postgres C)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(POSTGRES_MAJOR_VERSION 9)
set(POSTGRES_MINOR_VERSION 6)
set(POSTGRES_PATCH_VERSION 0)
set(POSTGRES_VERSION
  ${POSTGRES_MAJOR_VERSION}.${POSTGRES_MINOR_VERSION}.${POSTGRES_PATCH_VERSION})

set(PG_VERSION "9.6devel")
set(PG_VERSION_NUM 90600)

 # Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH
  "Installation directory for header files")

if(NOT PGBINDIR)
	set(PGBINDIR "${CMAKE_INSTALL_PREFIX}/bin")
endif(NOT PGBINDIR)

if(NOT PGSHAREDIR)
	set(PGSHAREDIR "${CMAKE_INSTALL_PREFIX}/share/postgresql")
endif(NOT PGSHAREDIR)

if(NOT SYSCONFDIR)
	set(SYSCONFDIR "${CMAKE_INSTALL_PREFIX}/etc")
endif(NOT SYSCONFDIR)

if(NOT INCLUDEDIR)
	set(INCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")
endif(NOT INCLUDEDIR)

if(NOT PKGINCLUDEDIR)
	set(PKGINCLUDEDIR "${CMAKE_INSTALL_PREFIX}/include")
endif(NOT PKGINCLUDEDIR)

if(NOT INCLUDEDIRSERVER)
	set(INCLUDEDIRSERVER "${CMAKE_INSTALL_PREFIX}/include/server")
endif(NOT INCLUDEDIRSERVER)

if(MSVC)
	if(NOT LIBDIR)
		set(LIBDIR "${CMAKE_INSTALL_PREFIX}/bin")
	endif(NOT LIBDIR)

	if(NOT PKGLIBDIR)
		set(PKGLIBDIR "${CMAKE_INSTALL_PREFIX}/bin")
	endif(NOT PKGLIBDIR)
else()
	if(NOT LIBDIR)
		set(LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
	endif(NOT LIBDIR)

	if(NOT PKGLIBDIR)
		set(PKGLIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
	endif(NOT PKGLIBDIR)
endif()

if(NOT LOCALEDIR)
	set(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/share/locale")
endif(NOT LOCALEDIR)

if(NOT DOCDIR)
	set(DOCDIR "${CMAKE_INSTALL_PREFIX}/share/doc/")
endif(NOT DOCDIR)

if(NOT HTMLDIR)
	set(HTMLDIR "${CMAKE_INSTALL_PREFIX}/share/doc/")
endif(NOT HTMLDIR)

if(NOT MANDIR)
	set(MANDIR "${CMAKE_INSTALL_PREFIX}/share/man")
endif(NOT MANDIR)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(CMAKE_SHARED_LINKER_FLAGS "-undefined dynamic_lookup")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD|NetBSD|OpenBSD")
	set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-Wl,-x,-soname,")
endif()

if (${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-ignored-attributes")
endif()

if(MSVC)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D _CRT_SECURE_NO_WARNINGS")
endif()

test_big_endian(WORDS_BIGENDIAN)

find_package(Perl REQUIRED)
find_package(PerlLibs)
find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Threads)
find_package(OpenSSL)

if(NOT MSVC)
	find_library(DL_LIBRARIES NAMES dl)
	find_library(M_LIB m)
endif()

if(NOT DL_LIBRARIES)
	set(DL_LIBRARIES "")
endif()

check_symbol_exists(strlcpy "stdio.h;string.h" HAVE_DECL_STRLCPY)
if(NOT HAVE_DECL_STRLCPY)
	set(HAVE_DECL_STRLCPY 0)
endif()
check_symbol_exists(strlcat "stdio.h;string.h" HAVE_DECL_STRLCAT)
if(NOT HAVE_DECL_STRLCAT)
	set(HAVE_DECL_STRLCAT 0)
endif()
check_symbol_exists(snprintf "stdio.h;string.h" HAVE_DECL_SNPRINTF)
if(NOT HAVE_DECL_SNPRINTF)
	set(HAVE_DECL_SNPRINTF 0)
endif()
check_symbol_exists(vsnprintf "stdio.h;string.h" HAVE_DECL_VSNPRINTF)
if(NOT HAVE_DECL_VSNPRINTF)
	set(HAVE_DECL_VSNPRINTF 0)
endif()
check_symbol_exists(unsetenv "stdlib.h" HAVE_UNSETENV)
check_symbol_exists(srandom "stdlib.h" HAVE_SRANDOM)
check_symbol_exists(isinf "math.h" HAVE_ISINF)
check_symbol_exists(rint "math.h" HAVE_RINT)
check_function_exists(getpeereid HAVE_GETPEEREID)
check_function_exists(getpeerucred HAVE_GETPEERUCRED)
check_function_exists(memmove HAVE_MEMMOVE)
check_function_exists(mbstowcs_l HAVE_MBSTOWCS_L)
check_function_exists(towlower HAVE_TOWLOWER)
check_function_exists(wcstombs HAVE_WCSTOMBS)
check_function_exists(wcstombs_l HAVE_WCSTOMBS_L)

set(CMAKE_MACOSX_RPATH 1)
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(MSVC)
	check_function_exists(_fseeki64 HAVE_FSEEKO)
else()
	check_function_exists(fseeko HAVE_FSEEKO)
endif()

if(NOT MSVC)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};dlfcn.h")
	set(CMAKE_REQUIRED_LIBRARIES ${DL_LIBRARIES})
	check_function_exists(dlopen HAVE_DLOPEN)
endif()

set(CMAKE_REQUIRED_LIBRARIES "")
set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};string.h")
check_function_exists(fls HAVE_FLS)
set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};sys/mman.h")
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -lrt")
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
check_function_exists(shm_open HAVE_SHM_OPEN)
set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};sys/time.h;sys/resource.h")
check_function_exists(getrusage HAVE_GETRUSAGE)

if(OPENSSL_FOUND)
	set(USE_OPENSSL 1)
	set(CMAKE_REQUIRED_INCLUDES ${OPENSSL_INCLUDE_DIR})
	set(CMAKE_REQUIRED_LIBRARIES ${OPENSSL_LIBRARIES})
	check_function_exists(SSL_get_current_compression HAVE_SSL_GET_CURRENT_COMPRESSION)
endif(OPENSSL_FOUND)


check_include_files(sys/un.h HAVE_SYS_UN_H)
check_include_files(ucred.h HAVE_UCRED_H)
check_include_files(sys/ucred.h HAVE_SYS_UCRED_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(sys/resource.h HAVE_SYS_RESOURCE_H)
check_include_files(sys/select.h HAVE_SYS_SELECT_H)
check_include_files(sys/poll.h HAVE_SYS_POLL_H)
check_include_files(sys/pstat.h HAVE_SYS_PSTAT_H)
check_include_files(dld.h HAVE_DLD_H)
check_include_files(langinfo.h HAVE_LANGINFO_H)
check_include_files(ieeefp.h HAVE_IEEEFP_H)
check_include_files(wchar.h HAVE_WCHAR_H)
check_include_files(wctype.h HAVE_WCTYPE_H)
check_include_files(winldap.h HAVE_WINLDAP_H)
check_include_files(pwd.h HAVE_PWD_H)

#Check ReadLine includes
option(WITH_READLINE "do not use GNU Readline nor BSD Libedit for editing" ON)
if(WITH_READLINE AND NOT MSVC)
	find_package(Readline)
	if(READLINE_FOUND)
		#Sometimes for redline need curses
		find_package(Curses)
		check_include_files("stdio.h;readline.h" HAVE_READLINE_H)
		check_include_files("stdio.h;history.h" HAVE_HISTORY_H)
		check_include_files("stdio.h;readline/history.h" HAVE_READLINE_HISTORY_H)
		check_include_files("stdio.h;readline/readline.h" HAVE_READLINE_READLINE_H)
		check_include_files("stdio.h;editline/history.h" HAVE_EDITLINE_HISTORY_H)
		check_include_files("stdio.h;editline/readline.h" HAVE_EDITLINE_READLINE_H)

		set(CMAKE_REQUIRED_LIBRARIES ${READLINE_LIBRARY} ${CURSES_LIBRARIES})
		check_function_exists(rl_completion_matches HAVE_RL_COMPLETION_MATCHES)
		check_function_exists(rl_filename_completion_function HAVE_RL_FILENAME_COMPLETION_FUNCTION)
	endif(READLINE_FOUND)
endif()

check_include_files("sys/ipc.h" HAVE_SYS_IPC_H)
check_include_files("sys/sem.h" HAVE_SYS_SEM_H)
check_include_files("sys/shm.h" HAVE_SYS_SHM_H)

# Check if _GNU_SOURCE is available.
check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)

include(CheckCpuID)
if(MSVC AND HAVE__CPUID)
	set(USE_SSE42_CRC32C_WITH_RUNTIME_CHECK 1)
endif()

include(FuncAcceptArgtypes)
include(CheckTypeAlignment)
include(GenDef)
check_type_alignment(double ALIGNOF_DOUBLE)
check_type_alignment(int ALIGNOF_INT)
check_type_alignment(long ALIGNOF_LONG)
check_type_alignment("long long int" ALIGNOF_LONG_LONG_INT)
check_type_alignment(short ALIGNOF_SHORT)

check_type_size(int64 HAVE_INT64)
check_type_size(uint64 HAVE_UINT64)
check_type_size(int8 HAVE_INT8)
check_type_size("void *" VOID_POINTER_SIZE)
check_type_size("long int" LONG_INT_SIZE)
check_type_size("long" SIZEOF_LONG)
check_type_size("size_t" SIZEOF_SIZE_T)
check_type_size(__int128 PG_INT128_TYPE)
if(PG_INT128_TYPE)
	set(PG_INT128_TYPE __int128)
endif()

set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};locale.h")
check_type_size("locale_t" HAVE_LOCALE_T)

if(LONG_INT_SIZE EQUAL 8)
	set(PG_INT64_TYPE "long int")
	set(HAVE_LONG_INT_64 ${LONG_INT_SIZE})
else(LONG_INT_SIZE EQUAL 8)
	check_type_size("long long int" HAVE_LONG_LONG_INT_64)
	if(HAVE_LONG_LONG_INT_64)
		set(HAVE_LONG_LONG_INT 1)
	endif(HAVE_LONG_LONG_INT_64)
	if(HAVE_LONG_LONG_INT_64 EQUAL 8)
		set(PG_INT64_TYPE "long long int")
	else(HAVE_LONG_LONG_INT_64 EQUAL 8)
		message(FATAL_ERROR "Cannot find a working 64-bit integer type.")
	endif(HAVE_LONG_LONG_INT_64 EQUAL 8)
endif(LONG_INT_SIZE EQUAL 8)

# Compute maximum alignment of any basic type.
# We assume long's alignment is at least as strong as char, short, or int;
# but we must check long long (if it exists) and double.

if(NOT MAXIMUM_ALIGNOF)
	set(MAX_ALIGNOF ${ALIGNOF_LONG})
	if(MAX_ALIGNOF LESS ALIGNOF_DOUBLE)
		set(MAX_ALIGNOF ${ALIGNOF_DOUBLE})
	endif(MAX_ALIGNOF LESS ALIGNOF_DOUBLE)
	if(HAVE_LONG_LONG_INT_64 AND MAX_ALIGNOF LESS HAVE_LONG_LONG_INT_64)
		set(MAX_ALIGNOF ${HAVE_LONG_LONG_INT_64})
	endif(HAVE_LONG_LONG_INT_64 AND MAX_ALIGNOF LESS HAVE_LONG_LONG_INT_64)
	if(MAX_ALIGNOF)
		set(MAXIMUM_ALIGNOF ${MAX_ALIGNOF})
	endif(MAX_ALIGNOF)
endif(NOT MAXIMUM_ALIGNOF)
message(STATUS "MAXIMUM_ALIGNOF ${MAXIMUM_ALIGNOF}")

if(HAVE_LONG_LONG_INT_64)
	include(CheckSnprintfLongLongIntModifier)
	if(NOT LONG_LONG_INT_MODIFIER)
		set(LONG_LONG_INT_MODIFIER "ll")
	endif(NOT LONG_LONG_INT_MODIFIER)
else(HAVE_LONG_LONG_INT_64)
	set(LONG_LONG_INT_MODIFIER "l")
endif(HAVE_LONG_LONG_INT_64)

if(HAVE_LONG_LONG_INT_64)
	message(STATUS "HAVE_LONG_LONG_INT_64 ${HAVE_LONG_LONG_INT_64}")
endif()
if(HAVE_LONG_LONG_INT_64)
	include(CheckLLConstants)
endif()

option(FLOAT4PASSBYVAL "float4 values are passed by value" ON)
if(FLOAT4PASSBYVAL)
	set(FLOAT4PASSBYVAL 1)
endif(FLOAT4PASSBYVAL)
option(USE_FLOAT4_BYVAL "float4 values are passed by value" ON)


if(FLOAT8PASSBYVAL AND NOT (VOID_POINTER_SIZE EQUAL 8))
	message(FATAL_ERROR "FLOAT8PASSBYVAL is not supported on 32-bit platforms.")
elseif(NOT FLOAT8PASSBYVAL AND VOID_POINTER_SIZE EQUAL 8)
	set(FLOAT8PASSBYVAL 1)
	set(USE_FLOAT8_BYVAL 1)
else()
	set(FLOAT8PASSBYVAL 0)
	set(USE_FLOAT8_BYVAL 0)
endif(FLOAT8PASSBYVAL AND NOT (VOID_POINTER_SIZE EQUAL 8))

if (_GNU_SOURCE)
    add_definitions(-D_GNU_SOURCE)
endif()
if(Threads_FOUND)
	set(ENABLE_THREAD_SAFETY 1)
	set(PTHREAD_CFLAGS "-D_REENTRANT -D_THREAD_SAFE -D_POSIX_PTHREAD_SEMANTICS")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PTHREAD_CFLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_THREAD_LIBS_INIT}")
endif()
set(PORT_DIR "${PROJECT_SOURCE_DIR}/src/port")
set(PQ_BACKEND_DIR "${PROJECT_SOURCE_DIR}/src/backend/libpq")
set(MB_UTILS_BACKEND_DIR "${PROJECT_SOURCE_DIR}/src/backend/utils/mb")


set(fallback_SRCS "")

if(NOT HAVE_DECL_STRLCPY)
  set(fallback_SRCS ${fallback_SRCS} "${PORT_DIR}/strlcpy.c")
endif(NOT HAVE_DECL_STRLCPY)

if(NOT HAVE_DECL_STRLCAT)
  set(fallback_SRCS ${fallback_SRCS} "${PORT_DIR}/strlcat.c")
endif(NOT HAVE_DECL_STRLCAT)

if(NOT HAVE_GETPEEREID)
  set(fallback_SRCS ${fallback_SRCS} "${PORT_DIR}/getpeereid.c")
endif(NOT HAVE_GETPEEREID)

set(TABLE_BLOCKSIZE 8 CACHE STRING "set table block size in kB")

if(TABLE_BLOCKSIZE EQUAL 1)
	set(BLCKSZ 1024)
elseif(TABLE_BLOCKSIZE EQUAL 2)
	set(BLCKSZ 2048)
elseif(TABLE_BLOCKSIZE EQUAL 4)
	set(BLCKSZ 4096)
elseif(TABLE_BLOCKSIZE EQUAL 8)
	set(BLCKSZ 8192)
elseif(TABLE_BLOCKSIZE EQUAL 16)
	set(BLCKSZ 16384)
elseif(TABLE_BLOCKSIZE EQUAL 32)
	set(BLCKSZ 32768)
else(TABLE_BLOCKSIZE EQUAL 1)
	message(FATAL_ERROR "Invalid block size. Allowed values are 1,2,4,8,16,32.")
endif(TABLE_BLOCKSIZE EQUAL 1)

message(STATUS "BLCKSZ - ${BLCKSZ}")

set(SEGSIZE 1 CACHE STRING "set table segment size in GB")
math(EXPR RELSEG_SIZE "(1024 / ${TABLE_BLOCKSIZE}) * ${SEGSIZE} * 1024")

set(WAL_BLOCKSIZE 8 CACHE STRING "set WAL block size in kB")

if(WAL_BLOCKSIZE EQUAL 1)
	set(XLOG_BLCKSZ 1024)
elseif(WAL_BLOCKSIZE EQUAL 2)
	set(XLOG_BLCKSZ 2048)
elseif(WAL_BLOCKSIZE EQUAL 4)
	set(XLOG_BLCKSZ 4096)
elseif(WAL_BLOCKSIZE EQUAL 8)
	set(XLOG_BLCKSZ 8192)
elseif(WAL_BLOCKSIZE EQUAL 16)
	set(XLOG_BLCKSZ 16384)
elseif(WAL_BLOCKSIZE EQUAL 32)
	set(XLOG_BLCKSZ 32768)
elseif(WAL_BLOCKSIZE EQUAL 64)
	set(XLOG_BLCKSZ 65536)
else(WAL_BLOCKSIZE EQUAL 1)
	message(FATAL_ERROR "Invalid WAL block size. Allowed values are 1,2,4,8,16,32,64.")
endif(WAL_BLOCKSIZE EQUAL 1)

message(STATUS "XLOG_BLCKSZ - ${XLOG_BLCKSZ}")

set(WAL_SEGSIZE 16 CACHE STRING "set WAL segment size in MB")

if (";1;2;4;8;16;32;64;" MATCHES ";${WAL_SEGSIZE};")
	math(EXPR XLOG_SEG_SIZE "${WAL_SEGSIZE} * 1024 * 1024")
else()
	message(FATAL_ERROR "${WAL_SEGSIZE} Invalid WAL segment size. Allowed values are 1,2,4,8,16,32,64.")
endif()

message(STATUS "XLOG_SEG_SIZE - ${XLOG_SEG_SIZE}")

if(NOT PGPORT)
	set(PGPORT 5432)
endif(NOT PGPORT)

include(CheckFlexibleArray)

check_c_source_compiles("
	#include <sys/time.h>
	int main(void){
		struct timeval *tp;
		struct timezone *tzp;
		gettimeofday(tp,tzp);
		return 0;
	}
" GETTIMEOFDAY_2ARG)

if(NOT GETTIMEOFDAY_2ARG)
	set(GETTIMEOFDAY_1ARG 1)
endif(NOT GETTIMEOFDAY_2ARG)

check_c_source_compiles("
	#include <time.h>
	int main(void){
		int res;
	#ifndef __CYGWIN__
		res = timezone / 60;
	#else
		res = _timezone / 60;
	#endif
		return 0;
	}
" HAVE_INT_TIMEZONE)

check_c_source_compiles("
	#include <stdio.h>
	int main(void){
		printf(\"%s\", __func__);
		return 0;
	}
" HAVE_FUNCNAME__FUNC)

check_c_source_compiles("
	#include <stdio.h>
	int main(void){
		printf(\"%s\", __FUNCTION__);
		return 0;
	}
" HAVE_FUNCNAME__FUNCTION)

check_c_source_compiles("
	#include <stdio.h>
	int main(void){
		#define debug(...) fprintf(stderr, __VA_ARGS__)
		debug(\"%s\", \"blarg\");
		return 0;
	}
" HAVE__VA_ARGS)

if(WIN32)
	set(LIB_M "")
else()
	set(LIB_M m)
endif()

check_struct_has_member("struct tm" tm_zone "sys/types.h;time.h" HAVE_TM_ZONE LANGUAGE C)
check_struct_has_member("struct tm" tm_gmtoff "sys/types.h;time.h" HAVE_STRUCT_TM_TM_ZONE LANGUAGE C)
set(CMAKE_EXTRA_INCLUDE_FILES "time.h")
check_type_size("*tzname" HAVE_TZNAME)

check_c_source_compiles("
	extern int pgac_write(int ignore, const char *fmt,...) __attribute__((format(gnu_printf, 2, 3)));
	int main(void){return 0;}
" PG_PRINTF_ATTRIBUTE)

if(PG_PRINTF_ATTRIBUTE)
	set(PG_PRINTF_ATTRIBUTE gnu_printf)
else(PG_PRINTF_ATTRIBUTE)
	set(PG_PRINTF_ATTRIBUTE printf)
endif(PG_PRINTF_ATTRIBUTE)

if(NOT MEMSET_LOOP_LIMIT)
	set(MEMSET_LOOP_LIMIT 1024)
endif(NOT MEMSET_LOOP_LIMIT)

if(WIN32)
	set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;winsock2.h")
else()
	set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;sys/socket.h;netdb.h")
endif()
check_type_size("struct addrinfo" HAVE_STRUCT_ADDRINFO)
if(HAVE_STRUCT_ADDRINFO)
	CHECK_STRUCT_HAS_MEMBER("struct addrinfo" sa_len "${CMAKE_EXTRA_INCLUDE_FILES}" HAVE_STRUCT_SOCKADDR_SA_LEN LANGUAGE C)
	if(NOT WIN32)
		set(HAVE_GETADDRINFO 1)
	endif()
endif(HAVE_STRUCT_ADDRINFO)

check_type_size("struct sockaddr_storage" HAVE_STRUCT_SOCKADDR_STORAGE)
if(HAVE_STRUCT_SOCKADDR_STORAGE)
	CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage" ss_family "${CMAKE_EXTRA_INCLUDE_FILES}" HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY LANGUAGE C)
	CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage" __ss_family "${CMAKE_EXTRA_INCLUDE_FILES}" HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY LANGUAGE C)
	CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage" ss_len "${CMAKE_EXTRA_INCLUDE_FILES}" HAVE_STRUCT_SOCKADDR_STORAGE_SS_LEN LANGUAGE C)
	CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage" __ss_len "${CMAKE_EXTRA_INCLUDE_FILES}" HAVE_STRUCT_SOCKADDR_STORAGE___SS_LEN LANGUAGE C)
endif(HAVE_STRUCT_SOCKADDR_STORAGE)

# If `struct sockaddr_un' exists, define HAVE_UNIX_SOCKETS.
if(HAVE_SYS_UN_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;sys/un.h")
else(HAVE_SYS_UN_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h")
endif(HAVE_SYS_UN_H)
check_type_size("struct sockaddr_un" HAVE_UNIX_SOCKETS)

if(WIN32)
	set(HAVE_IPV6 1)
else()
	set(CMAKE_EXTRA_INCLUDE_FILES "netinet/in.h")
	check_type_size("struct sockaddr_in6" HAVE_IPV6)
endif()

set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;sys/ipc.h;sys/sem.h")
check_type_size("union semun" HAVE_UNION_SEMUN)

check_include_file("sys/stat.h" HAVE_SYS_STAT_H)

CHECK_SYMBOL_EXISTS(fdatasync "unistd.h" HAVE_FDATASYNC)

if(WIN32)
	set(USE_WIN32_SEMAPHORES 1)
	set(SEMA_IMPLEMENTATION "${PROJECT_SOURCE_DIR}/src/backend/port/win32_sema.c")
else(WIN32)
	if(USE_NAMED_POSIX_SEMAPHORES)
		set(USE_NAMED_POSIX_SEMAPHORES 1)
		set(SEMA_IMPLEMENTATION "${PROJECT_SOURCE_DIR}/src/backend/port/posix_sema.c")
	elseif(USE_UNNAMED_POSIX_SEMAPHORES)
		set(USE_UNNAMED_POSIX_SEMAPHORES 1)
		set(SEMA_IMPLEMENTATION "${PROJECT_SOURCE_DIR}/src/backend/port/posix_sema.c")
	else(USE_NAMED_POSIX_SEMAPHORES)
		set(USE_SYSV_SEMAPHORES 1)
		set(SEMA_IMPLEMENTATION "${PROJECT_SOURCE_DIR}/src/backend/port/sysv_sema.c")
	endif(USE_NAMED_POSIX_SEMAPHORES)
endif(WIN32)

#Realy bad name for win32
set(USE_SYSV_SHARED_MEMORY 1)
if(WIN32)
	set(SHMEM_IMPLEMENTATION "${PROJECT_SOURCE_DIR}/src/backend/port/win32_shmem.c")
else(WIN32)
	set(SHMEM_IMPLEMENTATION "${PROJECT_SOURCE_DIR}/src/backend/port/sysv_shmem.c")
endif(WIN32)

if(WIN32)
	set(LATCH_IMPLEMENTATION "${PROJECT_SOURCE_DIR}/src/backend/port/win32_latch.c")
else(WIN32)
	set(LATCH_IMPLEMENTATION "${PROJECT_SOURCE_DIR}/src/backend/port/unix_latch.c")
endif(WIN32)

option(HAVE_SPINLOCKS "Define to ON if you have spinlocks." ON)
option(HAVE_ATOMICS "Define to ON if you want to use atomics if available." ON)

option(USE_INTEGER_DATETIMES "Define to ON if you want 64-bit integer timestamp and interval support." ON)

if(WIN32)
	option(HAVE_SYMLINK "Define to ON if you have the `symlink' function." ON)
else(WIN32)
	set(HAVE_SYMLINK 1)
endif(WIN32)

option(PG_KRB_SRVNAM "Define to the name of the default PostgreSQL service principal in Kerberos (GSSAPI)." "postgres")

#TODO: Need test this
if(CMAKE_C_COMPILER_ID STREQUAL "SunPro")
	if(CMAKE_SYSTEM_PROCESSOR MATCHES "sparc")
		set(TAS sunstudio_sparc.s)
	else(CMAKE_SYSTEM_PROCESSOR MATCHES "sparc")
		set(TAS sunstudio_x86.s)
	endif(CMAKE_SYSTEM_PROCESSOR MATCHES "sparc")
elseif(CMAKE_C_COMPILER_ID STREQUAL "HP-UX")
	set(TAS hpux_hppa.s)
else(CMAKE_C_COMPILER_ID STREQUAL "SunPro")
	set(TAS dummy.s)
endif(CMAKE_C_COMPILER_ID STREQUAL "SunPro")

if(WIN32)
	set(CMAKE_REQUIRED_INCLUDES
		windows.h
		string.h
		dbghelp.h
	)
	set(CMAKE_REQUIRED_DEFINITIONS "WIN32_LEAN_AND_MEAN")
	check_type_size(MINIDUMP_TYPE NAVE_MINIDUMP_TYPE)
endif(WIN32)

set(WIN32_STACK_RLIMIT 4194304)
if(WIN32)
	add_definitions(-DWIN32_STACK_RLIMIT=${WIN32_STACK_RLIMIT})
endif()

# Need add sco and unixware?
if(WIN32)
	set(pgos_include_SRCS ${PROJECT_SOURCE_DIR}/src/include/port/win32.h)
elseif(APPLE)
	set(pgos_include_SRCS ${PROJECT_SOURCE_DIR}/src/include/port/darwin.h)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(pgos_include_SRCS ${PROJECT_SOURCE_DIR}/src/include/port/linux.h)
elseif(CMAKE_SYSTEM_NAME STREQUAL "HP-UX")
	set(pgos_include_SRCS ${PROJECT_SOURCE_DIR}/src/include/port/hpux.h)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	set(pgos_include_SRCS ${PROJECT_SOURCE_DIR}/src/include/port/freebsd.h)
elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
	set(pgos_include_SRCS ${PROJECT_SOURCE_DIR}/src/include/port/openbsd.h)
elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
	set(pgos_include_SRCS ${PROJECT_SOURCE_DIR}/src/include/port/newtbsd.h)
else(WIN32)
	message(WARNING "${CMAKE_SYSTEM_NAME}")
endif(WIN32)

if(WIN32)
	include_directories("${PROJECT_SOURCE_DIR}/src/include/port/win32")
endif()

if(MSVC)
	include_directories("${PROJECT_SOURCE_DIR}/src/include/port/win32_msvc")
endif()

configure_file(
	"${PROJECT_SOURCE_DIR}/src/include/pg_config_cmake.in"
	"${PROJECT_SOURCE_DIR}/src/include/pg_config.h"
)

configure_file(
	"${PROJECT_SOURCE_DIR}/src/include/pg_config_ext_cmake.in"
	"${PROJECT_SOURCE_DIR}/src/include/pg_config_ext.h"
)

configure_file(
	"${PROJECT_SOURCE_DIR}/src/include/pg_config_paths_cmake.in"
	"${PROJECT_SOURCE_DIR}/src/port/pg_config_paths.h"
)

# Add sub-directories
add_subdirectory(src)
add_subdirectory(contrib/spi)
#add_subdirectory(contrib)
