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



test_big_endian(WORDS_BIGENDIAN)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Readline)
find_package(Threads)

check_function_exists(strlcpy HAVE_DECL_STRLCPY)
check_function_exists(strlcat HAVE_DECL_STRLCAT)
check_function_exists(unsetenv HAVE_UNSETENV)
check_function_exists(getpeereid HAVE_GETPEEREID)
check_function_exists(getpeerucred HAVE_GETPEERUCRED)


check_include_files(sys/un.h HAVE_SYS_UN_H)
check_include_files(ucred.h HAVE_UCRED_H)
check_include_files(sys/ucred.h HAVE_SYS_UCRED_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(history.h HAVE_HISTORY_H)
check_include_files(readline.h HAVE_READLINE_H)
check_include_files(readline/history.h HAVE_READLINE_HISTORY_H)
check_include_files(readline/readline.h HAVE_READLINE_READLINE_H)
check_include_files(editline/history.h HAVE_EDITLINE_HISTORY_H)
check_include_files(editline/readline.h HAVE_EDITLINE_READLINE_H)

# Check if _GNU_SOURCE is available.
check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)

include(FuncAcceptArgtypes)
include(CheckTypeAlignment)
check_type_alignment(double ALIGNOF_DOUBLE)
check_type_alignment(int ALIGNOF_INT)
check_type_alignment(long ALIGNOF_LONG)
check_type_alignment("long long int" ALIGNOF_LONG_LONG_INT)
check_type_alignment(short ALIGNOF_SHORT)

check_type_size(int64 HAVE_INT64)
check_type_size(uint64 HAVE_UINT64)
check_type_size(int8 HAVE_INT8)

check_type_size("long int" HAVE_LONG_INT_64)
if(HAVE_LONG_INT_64 EQUAL 8)
	set(PG_INT64_TYPE "long int")
else(HAVE_LONG_INT_64 EQUAL 8)
	check_type_size("long long int" HAVE_LONG_LONG_INT_64)
	if(HAVE_LONG_LONG_INT_64)
		set(HAVE_LONG_LONG_INT 1)
	endif(HAVE_LONG_LONG_INT_64)
	if(HAVE_LONG_LONG_INT_64 EQUAL 8)
		set(PG_INT64_TYPE "long long int")
	else(HAVE_LONG_LONG_INT_64 EQUAL 8)
		message(FATAL_ERROR "Cannot find a working 64-bit integer type.")
	endif(HAVE_LONG_LONG_INT_64 EQUAL 8)
endif(HAVE_LONG_INT_64 EQUAL 8)

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
	if(NOT LONG_LONG_INT_MODIFIER)
		include(CheckSnprintfLongLongIntModifier)
		if(NOT LONG_LONG_INT_MODIFIER)
			set(LONG_LONG_INT_MODIFIER "ll")
		endif(NOT LONG_LONG_INT_MODIFIER)
	endif(NOT LONG_LONG_INT_MODIFIER)
else(HAVE_LONG_LONG_INT_64)
	set(LONG_LONG_INT_MODIFIER "l")
endif(HAVE_LONG_LONG_INT_64)

if (_GNU_SOURCE)
    add_definitions(-D_GNU_SOURCE)
endif()

set(PTHREAD_CFLAGS "-pthread -D_REENTRANT -D_THREAD_SAFE -D_POSIX_PTHREAD_SEMANTICS")
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

if(NOT BLOCKSIZE)
	set(BLOCKSIZE 8)
endif(NOT BLOCKSIZE)

if(BLOCKSIZE EQUAL 1)
	set(BLCKSZ 1024)
elseif(BLOCKSIZE EQUAL 2)
	set(BLCKSZ 2048)
elseif(BLOCKSIZE EQUAL 4)
	set(BLCKSZ 4096)
elseif(BLOCKSIZE EQUAL 8)
	set(BLCKSZ 8192)
elseif(BLOCKSIZE EQUAL 16)
	set(BLCKSZ 16384)
elseif(BLOCKSIZE EQUAL 32)
	set(BLCKSZ 32768)
else(BLOCKSIZE EQUAL 1)
	message(FATAL_ERROR "Invalid block size. Allowed values are 1,2,4,8,16,32.")
endif(BLOCKSIZE EQUAL 1)

message(STATUS "BLCKSZ - ${BLCKSZ}")

if(NOT PGPORT)
	set(PGPORT 5432)
endif(NOT PGPORT)

include(CheckFlexibleArray)

if(NOT FLOAT4PASSBYVAL)
	set(FLOAT4PASSBYVAL 1)
endif(NOT FLOAT4PASSBYVAL)

if(NOT FLOAT8PASSBYVAL)
	set(FLOAT8PASSBYVAL 1)
endif(NOT FLOAT8PASSBYVAL)

CHECK_C_SOURCE_COMPILES("
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

CHECK_C_SOURCE_COMPILES("
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

set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;sys/socket.h;netdb.h")
check_type_size("struct addrinfo" HAVE_STRUCT_ADDRINFO)
if(HAVE_STRUCT_ADDRINFO)
	CHECK_STRUCT_HAS_MEMBER("struct addrinfo" sa_len "sys/types.h;sys/socket.h;netdb.h" HAVE_STRUCT_SOCKADDR_SA_LEN LANGUAGE C)
	if(WIN32)
		#TODO
	else(WIN32)
		set(HAVE_GETADDRINFO 1)
	endif(WIN32)
endif(HAVE_STRUCT_ADDRINFO)

set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;sys/socket.h")
check_type_size("struct sockaddr_storage" HAVE_STRUCT_SOCKADDR_STORAGE)
if(HAVE_STRUCT_SOCKADDR_STORAGE)
	CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage" ss_family "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_STORAGE_SS_FAMILY LANGUAGE C)
	CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage" __ss_family "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_STORAGE___SS_FAMILY LANGUAGE C)
	CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage" ss_len "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_STORAGE_SS_LEN LANGUAGE C)
	CHECK_STRUCT_HAS_MEMBER("struct sockaddr_storage" __ss_len "sys/types.h;sys/socket.h" HAVE_STRUCT_SOCKADDR_STORAGE___SS_LEN LANGUAGE C)
endif(HAVE_STRUCT_SOCKADDR_STORAGE)

# If `struct sockaddr_un' exists, define HAVE_UNIX_SOCKETS.
if(HAVE_SYS_UN_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;sys/un.h")
else(HAVE_SYS_UN_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h")
endif(HAVE_SYS_UN_H)
check_type_size("struct sockaddr_un" HAVE_UNIX_SOCKETS)

configure_file(
	"${PROJECT_SOURCE_DIR}/src/include/pg_config_cmake.in"
	"${PROJECT_SOURCE_DIR}/src/include/pg_config.h"
)

# Add sub-directories
add_subdirectory(src)
#add_subdirectory(contrib)