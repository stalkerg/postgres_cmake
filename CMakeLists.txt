cmake_minimum_required(VERSION 2.8)

include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckIncludeFiles)

project(Postgres C)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(POSTGRES_MAJOR_VERSION 9)
set(POSTGRES_MINOR_VERSION 6)
set(POSTGRES_PATCH_VERSION 0)
set(POSTGRES_VERSION
  ${POSTGRES_MAJOR_VERSION}.${POSTGRES_MINOR_VERSION}.${POSTGRES_PATCH_VERSION})

 # Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE PATH
  "Installation directory for header files")

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Readline)
find_package(Threads)

check_function_exists(strlcpy HAVE_STRLCPY)
check_function_exists(strlcat HAVE_STRLCAT)
check_function_exists(getpeereid HAVE_GETPEERID)

check_include_files(sys/un.h HAVE_SYS_UN_H)
check_include_files(ucred.h HAVE_UCRED_H)
check_include_files(sys/ucred.h HAVE_SYS_UCRED_H)

# Check if _GNU_SOURCE is available.
check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)

if (_GNU_SOURCE)
    add_definitions(-D_GNU_SOURCE)
endif()

set(PTHREAD_CFLAGS "-pthread -D_REENTRANT -D_THREAD_SAFE -D_POSIX_PTHREAD_SEMANTICS")
set(PORT_DIR "${PROJECT_SOURCE_DIR}/src/port")
set(PQ_BACKEND_DIR "${PROJECT_SOURCE_DIR}/src/backend/libpq")
set(MB_UTILS_BACKEND_DIR "${PROJECT_SOURCE_DIR}/src/backend/utils/mb")


set(fallback_SRCS "")

if(NOT HAVE_STRLCPY)
  set(fallback_SRCS ${fallback_SRCS} "${PORT_DIR}/strlcpy.c")
endif(NOT HAVE_STRLCPY)

if(NOT HAVE_STRLCAT)
  set(fallback_SRCS ${fallback_SRCS} "${PORT_DIR}/strlcat.c")
endif(NOT HAVE_STRLCAT)

if(NOT HAVE_GETPEERID)
  set(fallback_SRCS ${fallback_SRCS} "${PORT_DIR}/getpeereid.c")
endif(NOT HAVE_GETPEERID)

# Add sub-directories
add_subdirectory(src)
#add_subdirectory(contrib)