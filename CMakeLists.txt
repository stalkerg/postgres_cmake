cmake_minimum_required(VERSION 2.8.4)

project(Postgres C)

include(CheckTypeSize)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckCSourceCompiles)
include(CheckCSourceRuns)
include(TestBigEndian)
include(CheckStructHasMember)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	message(STATUS "${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "You must use another folder for build PostgreSQL")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(POSTGRES_MAJOR_VERSION 11)
set(POSTGRES_MINOR_VERSION 0)
set(POSTGRES_PATCH_VERSION 0)
set(POSTGRES_VERSION
  ${POSTGRES_MAJOR_VERSION}.${POSTGRES_MINOR_VERSION}.${POSTGRES_PATCH_VERSION})

set(PG_VERSION "11devel")
set(PG_VERSION_NUM 100000)
set(PACKAGE_BUGREPORT "pgsql-bugs@postgresql.org")

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
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -multiply_defined suppress -undefined dynamic_lookup")
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -multiply_defined suppress -undefined dynamic_lookup")
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD|NetBSD|OpenBSD")
	set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-Wl,-x,-soname,")
endif()

set(PLUGIN_TYPE MODULE)

if(MSVC)
	# Try hide warnings
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS ")
	option(USE_FP_STRICT "use strict mode for float and double in MSVC" ON)
	if (USE_FP_STRICT)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fp:strict")
	endif()
	# We can't use MODULE because CMake get export symbols only for SHARED.
	set(PLUGIN_TYPE SHARED)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "AIX" AND "${CMAKE_C_FLAGS}" MATCHES "aix64")
	SET(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> <LINK_FLAGS> -X64 cr <TARGET> <OBJECTS>")
	SET(CMAKE_C_ARCHIVE_APPEND "<CMAKE_AR> <LINK_FLAGS> -X64 r <TARGET> <OBJECTS>")
endif()

test_big_endian(WORDS_BIGENDIAN)
set(FIND_LIBRARY_USE_LIB64_PATHS ON)

find_package(Perl REQUIRED)
option(WITH_PERL "libperl is optional" OFF)
if(WITH_PERL)
	find_package(PerlLibs)
endif()
if(PERLLIBS_FOUND)
	EXECUTE_PROCESS(COMMAND ${PERL_EXECUTABLE} -MExtUtils::Embed -e ccopts OUTPUT_VARIABLE PERL_CFLAGS)
	EXECUTE_PROCESS(COMMAND ${PERL_EXECUTABLE} -MExtUtils::Embed -e ldopts OUTPUT_VARIABLE PERL_LDFLAGS)
	STRING(REGEX REPLACE "[\r\n]" " " PERL_CFLAGS ${PERL_CFLAGS})
	STRING(REGEX REPLACE " +$" "" PERL_CFLAGS ${PERL_CFLAGS})
	STRING(STRIP "${PERL_CFLAGS}" PERL_CFLAGS)
	STRING(REGEX REPLACE "[\r\n]" " " PERL_LDFLAGS ${PERL_LDFLAGS})
	STRING(REGEX REPLACE " +$" "" PERL_LDFLAGS ${PERL_LDFLAGS})
	STRING(REGEX REPLACE "-lperl" "" PERL_LDFLAGS ${PERL_LDFLAGS})
	if(MINGW AND NOT MSYS)
		STRING(REGEX REPLACE "\\\\" "/" PERL_LDFLAGS ${PERL_LDFLAGS})
	endif()
	STRING(STRIP "${PERL_LDFLAGS}" PERL_LDFLAGS)
endif()

option(WITH_TAP "try enable TAP tests" OFF)
# It's really pain to use the TAP tests without env cmake command
if (CMAKE_VERSION VERSION_GREATER "3.2.0")
	find_program(PROVE prove)
endif()
if(NOT WITH_TAP)
	set(PROVE "")
endif()

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(Threads)

option(WITH_OPENSSL "OPENSSL is optional" OFF)
if(WITH_OPENSSL)
	find_package(OpenSSL)
endif()
find_package(ZLIB)

option(WITH_PYTHON "Python is optional" OFF)
if(WITH_PYTHON)
	find_package(PythonInterp)
	find_package(PythonLibs)
endif()
find_package(SELinux)

option(WITH_LIBXML "LIBXML is optional" OFF)
if(WITH_LIBXML)
	find_package(LibXml2)
	find_package(LibXslt)
endif()

option(WITH_TCL "TCL is optional" OFF)
if(WITH_TCL)
	find_package(TCL)
endif()
find_package(LibSocket)

option(WITH_ICU "ICU is optional" OFF)
if(WITH_ICU)
	find_package(ICU 4.0 COMPONENTS i18n REQUIRED)
endif()
if(ICU_FOUND)
	set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${ICU_INCLUDE_DIRS}")
	check_include_files(unicode/ucol.h HAVE_UCOL_H)
	if(NOT HAVE_UCOL_H)
		message(FATAL_ERROR "unicode/ucol.h not found")
	endif()
	set(USE_ICU 1)
endif()
option(USE_PAM "build with PAM support" OFF)
if(USE_PAM)
	find_library(PAM_LIB pam)
	if(NOT PAM_LIB)
		message(ERROR "library 'pam' is required for PAM")
	endif()
endif()

option(USE_LDAP "build with LDAP support" OFF)
if(USE_LDAP)
	find_package(LDAP REQUIRED)
endif()

if(WIN32 OR MINGW)
	set(NEED_REPL_SNPRINTF ON)
else()
	set(NEED_REPL_SNPRINTF OFF)
endif()

option(ENABLE_NLS "Define if you want National Language Support" OFF)
if(ENABLE_NLS)
	include(MakeNLS)
	find_package(Gettext REQUIRED)
	find_program(GETTEXT_XGETTEXT_EXECUTABLE xgettext)
	if(NOT GETTEXT_XGETTEXT_EXECUTABLE)
		message(FATAL_ERROR "xgettext not found")
	endif(NOT GETTEXT_XGETTEXT_EXECUTABLE)
	if(NOT NLS_LANGUAGES)
		set(NLS_LANGUAGES "ko;cs;pt_BR;zh_CN;ru;fr;de;es;it;tr;ja;pl;zh_TW")
	endif()

	if(NOT NEED_REPL_SNPRINTF)
		check_c_source_runs("
			#include <stdio.h>
			#include <string.h>

			int main()
			{
			  char buf[100];

			  /* can it swap arguments? */
			  snprintf(buf, 100, \"%2\$d %1\$d\", 3, 4);
			  if (strcmp(buf, \"4 3\") != 0)
				return 1;
			  return 0;
			}
		" SNPRINTF_ARG_CONTROL)
		if(NOT SNPRINTF_ARG_CONTROL)
			set(NEED_REPL_SNPRINTF ON)
		endif()
	endif()
endif()

option(ENABLE_GSS "Define to build with GSSAPI support." OFF)
if(ENABLE_GSS)
	check_include_files(gssapi/gssapi.h HAVE_GSSAPI_GSSAPI_H)
	check_include_files(gssapi.h HAVE_GSSAPI_H)
	set(PG_KRB_SRVTAB "FILE:${SYSCONFDIR}/krb5.keytab")
	if(WIN32)
		SET(GSS_LIBS "-lgssapi32")
	else()
		set(CMAKE_REQUIRED_LIBRARIES_OLD ${CMAKE_REQUIRED_LIBRARIES})
		set(CMAKE_REQUIRED_LIBRARIES "-lgssapi_krb5")
		check_function_exists(gss_init_sec_context HAVE_GSS_INIT)
		if(NOT HAVE_GSS_INIT)
			set(CMAKE_REQUIRED_LIBRARIES "-lgss")
			check_function_exists(gss_init_sec_context HAVE_GSS_INIT)
			if(NOT HAVE_GSS_INIT)
				set(CMAKE_REQUIRED_LIBRARIES "-lgssapi -lkrb5 -lcrypto")
				check_function_exists(gss_init_sec_context HAVE_GSS_INIT)
			endif(NOT HAVE_GSS_INIT)
		endif(NOT HAVE_GSS_INIT)
		if(HAVE_GSS_INIT)
			set(GSS_LIBS ${CMAKE_REQUIRED_LIBRARIES})
		else()
			message(FATAL_ERROR "Not found gss_init_sec_context function for GSSAPI")
		endif()
		set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES_OLD})
	endif()
else()
	set(GSS_LIBS "")
endif()

option(USE_SYSTEMD "Define to build with systemd support" OFF)
option(USE_BSD_AUTH "Define to build with BSD Authentication support" OFF)
option(USE_ASSERT_CHECKING "Define to build with assertion checks." OFF)
option(USE_BONJOUR "Define to build with Bonjour support." OFF)
if(USE_BONJOUR)
	check_include_files(dns_sd.h HAVE_DNS_SD_H)
	if(NOT HAVE_DNS_SD_H)
		message(FATAL_ERROR "header file <dns_sd.h> is required for Bonjour")
	endif()
endif()

option(STRONG_RANDOM "Strong random number source" ON)
option(STRONG_RANDOM_SOURCE "which random number source to use - openssl, win32, dev" OFF)
if(STRONG_RANDOM)
	if(NOT STRONG_RANDOM_SOURCE)
		if(WITH_OPENSSL)
			set(STRONG_RANDOM_SOURCE "openssl")
		elseif(WIN32)
			set(STRONG_RANDOM_SOURCE "win32")
		elseif(EXISTS "/dev/urandom")
			set(STRONG_RANDOM_SOURCE "dev")
		endif()
	endif()
	if(STRONG_RANDOM_SOURCE STREQUAL "openssl")
		set(USE_OPENSSL_RANDOM 1)
	elseif(STRONG_RANDOM_SOURCE STREQUAL "win32")
		set(USE_WIN32_RANDOM 1)
	elseif(STRONG_RANDOM_SOURCE STREQUAL "dev")
		set(USE_DEV_URANDOM 1)
	else()
		message(ERROR "no source of strong random numbers was found
PostgreSQL can use OpenSSL or /dev/urandom as a source of random numbers,
for authentication protocols. You can use -DSTRONG_RANDOM=OFF to use of a built-in
pseudo random number generator, but that may be insecure.")
	endif()

	set(HAVE_STRONG_RANDOM 1)
else()
	message(WARNING "Not using a strong random number source may be insecure.")
endif()

if(NOT MSVC)
	find_library(DL_LIBRARIES NAMES dl)
	find_library(M_LIB m)
	if(NOT M_LIB)
		set(M_LIB "")
	endif()
endif()

if(NOT DL_LIBRARIES)
	set(DL_LIBRARIES "")
endif()

if(LIBXML2_FOUND)
	set(HAVE_LIBXML2 1)
	set(USE_LIBXML 1)
endif()

if(LIBXSLT_FOUND)
	set(HAVE_LIBXSLT 1)
	set(USE_LIBXSLT 1)
endif()

if(ZLIB_FOUND)
	set(HAVE_LIBZ 1)
endif()

if(NOT CMAKE_C_FLAGS)
	set(CMAKE_C_FLAGS "-O2")
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 3.3 OR CMAKE_C_COMPILER_VERSION VERSION_EQUAL 3.3)
		# Disable strict-aliasing rules; needed for gcc 3.3+
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
	endif()

	if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 3.4 OR CMAKE_C_COMPILER_VERSION VERSION_EQUAL 3.4)
		# Disable FP optimizations that cause various errors on gcc 4.5+ or maybe 4.6+
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fwrapv")
	endif()

	if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 4.5 OR CMAKE_C_COMPILER_VERSION VERSION_EQUAL 4.5)
		# Disable FP optimizations that cause various errors on gcc 4.5+ or maybe 4.6+
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fexcess-precision=standard")
	endif()

	# Optimization flags for specific files that benefit from vectorization
	set(CFLAGS_VECTOR "${CFLAGS_VECTOR} -funroll-loops")
	if(CMAKE_C_COMPILER_VERSION VERSION_GREATER 4.5 OR CMAKE_C_COMPILER_VERSION VERSION_EQUAL 4.5)
		set(CFLAGS_VECTOR "${CFLAGS_VECTOR} -ftree-vectorize")
	endif()

	option(PROFILE_PID_DIR "Enable to allow profiling output to be saved separately for each process." OFF)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
	# Disable strict-aliasing rules; needed for gcc 3.3+
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
	# Disable FP optimizations that cause various errors on gcc 4.5+ or maybe 4.6+
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fwrapv")

	# Optimization flags for specific files that benefit from vectorization
	set(CFLAGS_VECTOR "${CFLAGS_VECTOR} -funroll-loops")
	set(CFLAGS_VECTOR "${CFLAGS_VECTOR} -ftree-vectorize")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "Intel")
	# Intel's compiler has a bug/misoptimization in checking for
	# division by NAN (NaN == 0), -mp1 fixes it, so add it to the CFLAGS.
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mp1")
	# Make sure strict aliasing is off (though this is said to be the default)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-strict-aliasing")
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

# Test math functions
check_symbol_exists(rint "math.h" HAVE_RINT)

if(WIN32 AND NOT MINGW)
	set(LIB_M "")
else()
	set(LIB_M m)
endif()
set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} ${LIB_M}")
check_c_source_runs("
	#include <math.h>
	int main(void){
#ifdef _MSC_VER
		return isinf(INFINITY) ? 0 : 1;
#else
		return isinf(1.0/0.0) ? 0 : 1;
#endif
	}
" HAVE_ISINF)

# Check common include files

check_include_files(ieeefp.h HAVE_IEEEFP_H)
if(NOT HAVE_IEEEFP_H)
	set(HAVE_IEEEFP_H 0)
else()
	set(HAVE_IEEEFP_H 1)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};ieeefp.h")
endif()

check_include_files(fp_class.h HAVE_FP_CLASS_H)
if (HAVE_FP_CLASS_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};fp_class.h")
endif()
set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};float.h;math.h")

check_include_files(sys/un.h HAVE_SYS_UN_H)
check_include_files(ucred.h HAVE_UCRED_H)
check_include_files(sys/ucred.h HAVE_SYS_UCRED_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_files(sys/sockio.h HAVE_SYS_SOCKIO_H)
check_include_files(stdint.h HAVE_STDINT_H)

check_include_files(sys/select.h HAVE_SYS_SELECT_H)
check_include_files(sys/poll.h HAVE_SYS_POLL_H)
check_include_files(sys/epoll.h HAVE_SYS_EPOLL_H)
check_include_files(sys/pstat.h HAVE_SYS_PSTAT_H)
check_include_files(sys/tas.h HAVE_SYS_TAS_H)
check_include_files(dld.h HAVE_DLD_H)
check_include_files(langinfo.h HAVE_LANGINFO_H)
check_include_files(poll.h HAVE_POLL_H)

check_include_files(wchar.h HAVE_WCHAR_H)
check_include_files(wctype.h HAVE_WCTYPE_H)
check_include_files(winldap.h HAVE_WINLDAP_H)
check_include_files(pwd.h HAVE_PWD_H)
check_include_files(crtdefs.h HAVE_CRTDEFS_H)

check_include_files(nbtool_config.h HAVE_NBTOOL_CONFIG_H)

check_include_files(mbarrier.h HAVE_MBARRIER_H)
check_include_files(atomic.h HAVE_ATOMIC_H)

check_include_files(netinet/tcp.h HAVE_NETINET_TCP_H)
check_include_files(net/if.h HAVE_NET_IF_H)

check_include_files(pam/pam_appl.h HAVE_PAM_PAM_APPL_H)
check_include_files(security/pam_appl.h HAVE_SECURITY_PAM_APPL_H)
if(USE_PAM AND NOT (HAVE_PAM_PAM_APPL_H OR HAVE_SECURITY_PAM_APPL_H))
	message(FATAL_ERROR "header file <security/pam_appl.h> or <pam/pam_appl.h> is required for PAM.")
endif()

check_include_files(syslog.h HAVE_SYSLOG)
check_include_files(termios.h HAVE_TERMIOS_H)

check_include_files("sys/ipc.h" HAVE_SYS_IPC_H)
check_include_files("sys/sem.h" HAVE_SYS_SEM_H)
check_include_files("sys/shm.h" HAVE_SYS_SHM_H)
check_include_files("sys/ioctl.h" HAVE_SYS_IOCTL_H)

check_include_files(ifaddrs.h HAVE_IFADDRS_H)
check_include_files(crypt.h HAVE_CRYPT_H)
check_include_files(unistd.h HAVE_UNISTD_H)

check_include_files("locale.h" HAVE_LOCALE_H)
check_include_files("xlocale.h" HAVE_XLOCALE_H)

check_include_files(bsd_auth.h HAVE_BSD_AUTH_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(getopt.h HAVE_GETOPT_H)
check_include_files(sys/mman.h HAVE_SYS_MMAN_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(sys/resource.h HAVE_SYS_RESOURCE_H)
check_include_files(utime.h HAVE_UTIME_H)

# Check functions

check_function_exists(fpclass HAVE_FPCLASS) # Need for isinf implementation
check_function_exists(fp_class HAVE_FP_CLASS)
check_function_exists(fp_class_d HAVE_FP_CLASS_D)
check_function_exists(class HAVE_CLASS)

check_function_exists(getpeereid HAVE_GETPEEREID)
check_function_exists(getpeerucred HAVE_GETPEERUCRED)
check_function_exists(memmove HAVE_MEMMOVE)
if(MSVC)
	check_function_exists(_mbstowcs_l HAVE_MBSTOWCS_L)
else()
	check_function_exists(mbstowcs_l HAVE_MBSTOWCS_L)
endif()

check_function_exists(towlower HAVE_TOWLOWER)
check_function_exists(wcstombs HAVE_WCSTOMBS)

check_function_exists(mkdtemp HAVE_MKDTEMP)
check_function_exists(mkstemp HAVE_MKSTEMP)
check_function_exists(poll HAVE_POLL)

check_function_exists(getrlimit HAVE_GETRLIMIT)
check_function_exists(readlink HAVE_READLINK)
check_function_exists(cbrt HAVE_CBRT)
check_function_exists(pthread_is_threaded_np HAVE_PTHREAD_IS_THREADED_NP)
check_function_exists(random HAVE_RANDOM)
check_function_exists(sync_file_range HAVE_SYNC_FILE_RANGE)

#So strange
check_function_exists(pstat HAVE_PSTAT)

if(HAVE_IFADDRS_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};ifaddrs.h")
endif()
check_function_exists(getifaddrs HAVE_GETIFADDRS)

if(MSVC)
	set(HAVE_MINIDUMP_TYPE 1)
endif()

if(NOT CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};fcntl.h")
	check_function_exists(posix_fadvise HAVE_DECL_POSIX_FADVISE)
	if(HAVE_DECL_POSIX_FADVISE)
		set(HAVE_POSIX_FADVISE 1)
	endif()
endif()

find_library(CRYPT_LIB crypt)
if(CRYPT_LIB)
	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} ${CRYPT_LIB}")
endif()

if(HAVE_CRYPT_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};crypt.h")
endif()
check_function_exists(crypt HAVE_CRYPT)

if(HAVE_UNISTD_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};unistd.h")
endif()
check_function_exists(fdatasync HAVE_DECL_FDATASYNC)
check_function_exists(sys_siglist HAVE_DECL_FDATASYNC)
check_function_exists(setproctitle HAVE_SETPROCTITLE)
check_function_exists(setsid HAVE_SETSID)
if(HAVE_UNISTD_H)
	check_symbol_exists(sys_siglist "signal.h;unistd.h" HAVE_DECL_SYS_SIGLIST)
	check_symbol_exists(opterr "unistd.h" HAVE_INT_OPTERR)
	check_symbol_exists(optreset "unistd.h" HAVE_INT_OPTRESET)
else()
	check_symbol_exists(sys_siglist "signal.h" HAVE_DECL_SYS_SIGLIST)
endif()

set(CMAKE_MACOSX_RPATH 1)
#set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(MSVC)
	check_function_exists(_fseeki64 HAVE_FSEEKO)
else()
	check_function_exists(fseeko HAVE_FSEEKO)
endif()

if(NOT MSVC)
	check_include_files(dlfcn.h HAVE_DLFCN_H)
	if(HAVE_DLFCN_H)
		set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};dlfcn.h")
	endif()
	set(CMAKE_REQUIRED_LIBRARIES ${DL_LIBRARIES})
	check_function_exists(dlopen HAVE_DLOPEN)
endif()

set(CMAKE_REQUIRED_LIBRARIES "")

if(HAVE_STRINGS_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};strings.h")
endif()
if(HAVE_STRING_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};string.h")
endif()
check_function_exists(fls HAVE_FLS)

if(HAVE_GETOPT_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};getopt.h")
endif()

check_function_exists(strtoll HAVE_STRTOLL)
check_function_exists(strtoq HAVE_STRTOQ)
check_function_exists(strtoull HAVE_STRTOULL)
check_function_exists(strtouq HAVE_STRTOUQ)

check_type_size("struct option" HAVE_STRUCT_OPTION)
check_function_exists(getopt HAVE_GETOPT)
check_function_exists(getopt_long HAVE_GETOPT_LONG)
check_function_exists(gethostbyname_r HAVE_GETHOSTBYNAME_R)
check_function_exists(getpwuid_r HAVE_GETPWUID_R)
check_function_exists(strerror_r HAVE_STRERROR_R)
check_function_exists(strerror HAVE_STRERROR)
#I am not sure about this code.
check_c_source_runs("
#include <string.h>
int main(void){
#ifndef _AIX
int strerror_r(int, char *, size_t);
#else
/* Older AIX has 'int' for the third argument so we don't test the args. */
int strerror_r();
#endif
return 0;
}
" STRERROR_R_INT)

check_c_source_runs("
int main(void){
int a = 0; int *p = &a; int r;
 __asm__ __volatile__ (\" lwarx %0,0,%1,1\n\" : \"=&r\"(r) : \"r\"(p));
return 0;
}
" HAVE_PPC_LWARX_MUTEX_HINT)

check_c_source_runs("
#include <machine/vmparam.h>
#include <sys/exec.h>
int main(void){
PS_STRINGS->ps_nargvstr = 1;
PS_STRINGS->ps_argvstr = \"foo\";
return 0;
}
" HAVE_PS_STRINGS)

if(HAVE_SYS_MMAN_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};sys/mman.h")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES} -lrt")
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
check_function_exists(shm_open HAVE_SHM_OPEN)


if(HAVE_SYS_TIME_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};sys/time.h")
endif()
if(HAVE_SYS_RESOURCE_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};sys/resource.h")
endif()
check_function_exists(getrusage HAVE_GETRUSAGE)
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)

if(HAVE_UTIME_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};utime.h")
endif()
check_function_exists(utime HAVE_UTIME)
check_function_exists(utimes HAVE_UTIMES)

set(OLD_INCLUDES "${CMAKE_EXTRA_INCLUDE_FILES}")
set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};netinet/in.h;arpa/inet.h")
check_function_exists(inet_aton HAVE_INET_ATON)
set(CMAKE_EXTRA_INCLUDE_FILES "${OLD_INCLUDES}")

if(OPENSSL_FOUND)
	set(USE_OPENSSL 1)
	set(CMAKE_REQUIRED_INCLUDES "${CMAKE_REQUIRED_INCLUDES};${OPENSSL_INCLUDE_DIR}")
	set(CMAKE_REQUIRED_LIBRARIES ${OPENSSL_LIBRARIES})
	check_function_exists(SSL_get_current_compression HAVE_SSL_GET_CURRENT_COMPRESSION)
endif(OPENSSL_FOUND)

if(USE_SYSTEMD)
	check_include_files(systemd/sd-daemon.h HAVE_SYSTEMD_SD_DAEMON_H)
	if(NOT HAVE_SYSTEMD_SD_DAEMON_H)
		message(FATAL_ERROR "header file <systemd/sd-daemon.h> is required for systemd support")
	endif()
endif()


if(USE_BSD_AUTH AND NOT HAVE_BSD_AUTH_H)
	message(FATAL_ERROR "header file <bsd_auth.h> is required for BSD Authentication support")
endif()

# Check ReadLine includes
option(WITH_READLINE "do not use GNU Readline nor BSD Libedit for editing" ON)
if(WITH_READLINE AND NOT MSVC)
	find_package(Readline)
	if(READLINE_FOUND)
		#Sometimes for redline need curses
		message(STATUS "Found Readline: ${READLINE_LIBRARY}")
		find_package(Curses)
		if(NOT CURSES_FOUND)
			set(CURSES_NEED_NCURSES TRUE)
		endif()
		find_package(Curses)
		check_include_files("stdio.h;readline.h" HAVE_READLINE_H)
		check_include_files("stdio.h;history.h" HAVE_HISTORY_H)
		check_include_files("stdio.h;readline/history.h" HAVE_READLINE_HISTORY_H)
		check_include_files("stdio.h;readline/readline.h" HAVE_READLINE_READLINE_H)
		check_include_files("stdio.h;editline/history.h" HAVE_EDITLINE_HISTORY_H)
		check_include_files("stdio.h;editline/readline.h" HAVE_EDITLINE_READLINE_H)

		set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${READLINE_LIBRARY})
		if(CURSES_FOUND)
			set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} ${CURSES_LIBRARIES})
			set(READLINE_LIBRARY ${READLINE_LIBRARY} ${CURSES_LIBRARIES})
		endif()
		check_function_exists(rl_completion_append_character HAVE_RL_COMPLETION_APPEND_CHARACTER)
		check_function_exists(rl_completion_matches HAVE_RL_COMPLETION_MATCHES)
		check_function_exists(rl_filename_completion_function HAVE_RL_FILENAME_COMPLETION_FUNCTION)
		check_function_exists(rl_reset_screen_size HAVE_RL_RESET_SCREEN_SIZE)
		check_function_exists(append_history HAVE_APPEND_HISTORY)
		check_function_exists(history_truncate_file HAVE_HISTORY_TRUNCATE_FILE)
	endif(READLINE_FOUND)
endif()

if(READLINE_FOUND)
	set(HAVE_LIBREADLINE TRUE)
else()
	set(READLINE_LIBRARY "")
	set(HAVE_LIBREADLINE FALSE)
endif()

# Check if _GNU_SOURCE is available.
check_symbol_exists(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)
check_symbol_exists(F_FULLFSYNC "fcntl.h" HAVE_DECL_F_FULLFSYNC)

include(ReplacePython)
include(CheckCpuID)
include(CheckSSE42)

if(MSVC)
	if(HAVE__GET_CPUID OR HAVE__CPUID)
		set(USE_SSE42_CRC32C_WITH_RUNTIME_CHECK 1)
	endif()
else()
	if(HAVE_SSE42 AND HAVE_SSE42_INTRINSICS)
		set(USE_SSE42_CRC32C 1)
	elseif(HAVE_SSE42_INTRINSICS AND (HAVE__GET_CPUID OR HAVE__CPUID))
		set(USE_SSE42_CRC32C_WITH_RUNTIME_CHECK 1)
	else()
		set(USE_SLICING_BY_8_CRC32C 1)
	endif()
endif()

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
check_type_size(uint8 HAVE_UINT8)
check_type_size("void *" VOID_POINTER_SIZE)
math(EXPR VOID_POINTER_SIZE_BIT "${VOID_POINTER_SIZE}*8")
check_type_size("long int" LONG_INT_SIZE)
check_type_size("long long int" HAVE_LONG_LONG_INT)
check_type_size("long" SIZEOF_LONG)
check_type_size("size_t" SIZEOF_SIZE_T)
check_type_size(__int128 PG_INT128_TYPE)
if(PG_INT128_TYPE AND NOT WIN32)
	set(HAVE_INT128 1)
	set(PG_INT128_TYPE __int128)
endif()


if(HAVE_LOCALE_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};locale.h")
endif()
check_type_size("locale_t" HAVE_LOCALE_T)
if(NOT HAVE_LOCALE_T AND HAVE_XLOCALE_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};xlocale.h")
	check_type_size("locale_t" HAVE_LOCALE_T)
	if(HAVE_LOCALE_T)
		set(LOCALE_T_IN_XLOCALE 1)
	else()
		set(LOCALE_T_IN_XLOCALE 0)
	endif()
else()
	set(LOCALE_T_IN_XLOCALE 0)
endif()

check_type_size("wcstombs_l" HAVE_WCSTOMBS_L)
#check_function_exists(wcstombs_l HAVE_WCSTOMBS_L)
if(NOT HAVE_WCSTOMBS_L AND HAVE_XLOCALE_H)
	set(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_EXTRA_INCLUDE_FILES};xlocale.h")
	check_type_size("wcstombs_l" HAVE_WCSTOMBS_L)
	if(HAVE_WCSTOMBS_L)
		set(WCSTOMBS_L_IN_XLOCALE 1)
	else()
		set(WCSTOMBS_L_IN_XLOCALE 0)
	endif()
else()
	set(WCSTOMBS_L_IN_XLOCALE 0)
endif()

if(LONG_INT_SIZE EQUAL 8)
	set(PG_INT64_TYPE "long int")
	set(HAVE_LONG_INT_64 ${LONG_INT_SIZE})
else(LONG_INT_SIZE EQUAL 8)
	if(HAVE_LONG_LONG_INT EQUAL 8)
		set(PG_INT64_TYPE "long long int")
		set(HAVE_LONG_LONG_INT_64 1)
	else()
		message(FATAL_ERROR "Cannot find a working 64-bit integer type.")
	endif()
endif(LONG_INT_SIZE EQUAL 8)

message(STATUS "PG_INT64_TYPE: ${PG_INT64_TYPE} HAVE_LONG_INT_64: ${HAVE_LONG_INT_64}")
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
	if(NOT NEED_REPL_SNPRINTF)
		include(CheckSnprintfLongLongIntModifier)
		if(NOT LONG_LONG_INT_MODIFIER)
			set(LONG_LONG_INT_MODIFIER "ll")
			set(NEED_REPL_SNPRINTF ON)
		endif(NOT LONG_LONG_INT_MODIFIER)
	else(NOT NEED_REPL_SNPRINTF)
		set(LONG_LONG_INT_MODIFIER "ll")
	endif(NOT NEED_REPL_SNPRINTF)
else(HAVE_LONG_LONG_INT_64)
	set(LONG_LONG_INT_MODIFIER "l")
endif(HAVE_LONG_LONG_INT_64)

if(HAVE_LONG_LONG_INT_64)
	message(STATUS "HAVE_LONG_LONG_INT_64 ${HAVE_LONG_LONG_INT_64}")
endif()
if(HAVE_LONG_LONG_INT_64)
	include(CheckLLConstants)
endif()

#TODO: some strange here
option(USE_FLOAT4_BYVAL "float4 values are passed by value" ON)
if(USE_FLOAT4_BYVAL)
	set(FLOAT4PASSBYVAL 1)
else(USE_FLOAT4_BYVAL)
	set(FLOAT4PASSBYVAL 0)
endif(USE_FLOAT4_BYVAL)

if(VOID_POINTER_SIZE EQUAL 8)
	option(USE_FLOAT8_BYVAL "float8 values are passed by value" ON)
else()
	option(USE_FLOAT8_BYVAL "float8 values are passed by value" OFF)
endif()
if(USE_FLOAT8_BYVAL AND NOT (VOID_POINTER_SIZE EQUAL 8))
	message(FATAL_ERROR "USE_FLOAT8_BYVAL is not supported on 32-bit platforms.")
elseif(USE_FLOAT8_BYVAL AND VOID_POINTER_SIZE EQUAL 8)
	set(FLOAT8PASSBYVAL 1)
	set(USE_FLOAT8_BYVAL 1)
else()
	set(FLOAT8PASSBYVAL 0)
	set(USE_FLOAT8_BYVAL 0)
endif(USE_FLOAT8_BYVAL AND NOT (VOID_POINTER_SIZE EQUAL 8))

if (_GNU_SOURCE)
	add_definitions(-D_GNU_SOURCE)
endif()
if(Threads_FOUND)
	set(ENABLE_THREAD_SAFETY 1)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
	set(PTHREAD_CFLAGS "-D_REENTRANT -D_THREAD_SAFE -D_POSIX_PTHREAD_SEMANTICS")
	if(THREADS_HAVE_PTHREAD_ARG)
		set(PTHREAD_CFLAGS "${PTHREAD_CFLAGS} -pthread")
	endif()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${PTHREAD_CFLAGS}")
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

option(HAVE_ATOMICS "Define to ON if you want to use atomics if available." ON)

if(HAVE_ATOMICS)
	check_c_source_compiles("
		int main(void){
			char lock = 0;
			__sync_lock_test_and_set(&lock, 1);
			__sync_lock_release(&lock);
			return 0;
		}
	" HAVE_GCC__SYNC_CHAR_TAS)

	check_c_source_compiles("
		int main(void){
			int lock = 0;
			__sync_lock_test_and_set(&lock, 1);
			__sync_lock_release(&lock);
			return 0;
		}
	" HAVE_GCC__SYNC_INT32_TAS)

	check_c_source_compiles("
		int main(void){
			int val = 0;
			__sync_val_compare_and_swap(&val, 0, 37);
			return 0;
		}
	" HAVE_GCC__SYNC_INT32_CAS)

	check_c_source_compiles("
		int main(void){
			${PG_INT64_TYPE} lock = 0;
			__sync_val_compare_and_swap(&lock, 0, (${PG_INT64_TYPE}) 37);
			return 0;
		}
	" HAVE_GCC__SYNC_INT64_CAS)

	check_c_source_compiles("
		int main(void){
			int val = 0;
			int expect = 0;
			__atomic_compare_exchange_n(&val, &expect, 37, 0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
			return 0;
		}
	" HAVE_GCC__ATOMIC_INT32_CAS)

	check_c_source_compiles("
		int main(void){
			${PG_INT64_TYPE} val = 0;
			${PG_INT64_TYPE} expect = 0;
			__atomic_compare_exchange_n(&val, &expect, 37, 0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
			return 0;
		}
	" HAVE_GCC__ATOMIC_INT64_CAS)
endif(HAVE_ATOMICS)

check_c_source_runs("
	int main(void){
		static unsigned long int x = __builtin_bswap32(0xaabbccdd);
		return 0;
	}
" HAVE__BUILTIN_BSWAP32)

check_c_source_runs("
	int main(void){
		static unsigned long int x = __builtin_bswap64(0xaabbccddeeff0011);
		return 0;
	}
" HAVE__BUILTIN_BSWAP64)

check_c_source_runs("
	int main(void){
		static int x; static int y[__builtin_constant_p(x) ? x : 1];
		return 0;
	}
" HAVE__BUILTIN_CONSTANT_P)

check_c_source_runs("
	int main(void){
		int x; static int y[__builtin_types_compatible_p(__typeof__(x), int)];
		return 0;
	}
" HAVE__BUILTIN_TYPES_COMPATIBLE_P)

check_c_source_runs("
	int main(void){
		__builtin_unreachable();
		return 0;
	}
" HAVE__BUILTIN_UNREACHABLE)

check_c_source_runs("
	int main(void){
		{ _Static_assert(1, \"foo\"); }
		return 0;
	}
" HAVE__STATIC_ASSERT)

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

check_struct_has_member("struct tm" tm_zone "sys/types.h;time.h" HAVE_TM_ZONE LANGUAGE C)
check_struct_has_member("struct tm" tm_gmtoff "sys/types.h;time.h" HAVE_STRUCT_TM_TM_ZONE LANGUAGE C)
set(CMAKE_EXTRA_INCLUDE_FILES "time.h")
check_type_size("*tzname" HAVE_TZNAME)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
	set(CMAKE_REQUIRED_FLAGS "-Werror")
endif()
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


if(WIN32 OR MINGW)
	set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;winsock2.h;ws2tcpip.h")
else()
	set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;sys/socket.h;netdb.h")
endif()

check_type_size("struct addrinfo" HAVE_STRUCT_ADDRINFO)
check_type_size("struct cmsgcred" HAVE_STRUCT_CMSGCRED)
if(HAVE_STRUCT_ADDRINFO)
	CHECK_STRUCT_HAS_MEMBER("struct addrinfo" sa_len "${CMAKE_EXTRA_INCLUDE_FILES}" HAVE_STRUCT_SOCKADDR_SA_LEN LANGUAGE C)
	if(NOT (WIN32 OR MINGW))
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

check_symbol_exists(fdatasync "unistd.h" HAVE_FDATASYNC)

if(WIN32 OR MINGW)
	set(USE_WIN32_SEMAPHORES 1)
	set(SEMA_IMPLEMENTATION "${PROJECT_SOURCE_DIR}/src/backend/port/win32_sema.c")
else(WIN32 OR MINGW)
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
endif(WIN32 OR MINGW)

#Realy bad name for win32
set(USE_SYSV_SHARED_MEMORY 1)
if(WIN32 OR MINGW)
	set(SHMEM_IMPLEMENTATION "${PROJECT_SOURCE_DIR}/src/backend/port/win32_shmem.c")
else(WIN32 OR MINGW)
	set(SHMEM_IMPLEMENTATION "${PROJECT_SOURCE_DIR}/src/backend/port/sysv_shmem.c")
endif(WIN32 OR MINGW)

option(HAVE_SPINLOCKS "Define to ON if you have spinlocks." ON)


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

if(WIN32 OR MINGW)
	set(CMAKE_EXTRA_INCLUDE_FILES
		${CMAKE_EXTRA_INCLUDE_FILES}
		windows.h
		string.h
		dbghelp.h
	)
	set(CMAKE_REQUIRED_DEFINITIONS "WIN32_LEAN_AND_MEAN")
	check_type_size(MINIDUMP_TYPE NAVE_MINIDUMP_TYPE)
endif(WIN32 OR MINGW)

set(WIN32_STACK_RLIMIT 4194304)
if(WIN32)
	add_definitions(-DWIN32_STACK_RLIMIT=${WIN32_STACK_RLIMIT})
endif()

if(NEED_REPL_SNPRINTF)
	option(USE_REPL_SNPRINTF "Use replacement snprintf() functions." ON)
endif()

find_program(SED_EXECUTABLE sed)
if(NOT SED_EXECUTABLE)
	message(FATAL_ERROR "sed programm not found")
endif(NOT SED_EXECUTABLE)

include(GenDef)
#Not work correctly for postgres see bug: 
#https://gitlab.kitware.com/cmake/cmake/issues/16161
#set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(RegressCheck)

# Need add sco and unixware?
if(WIN32 OR MINGW)
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
elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
	set(pgos_include_SRCS ${PROJECT_SOURCE_DIR}/src/include/port/solaris.h)
elseif(CMAKE_SYSTEM_NAME STREQUAL "AIX")
	set(pgos_include_SRCS ${PROJECT_SOURCE_DIR}/src/include/port/aix.h)
elseif(CYGWIN)
	set(pgos_include_SRCS ${PROJECT_SOURCE_DIR}/src/include/port/cygwin.h)
else(WIN32 OR MINGW)
	message(WARNING "${CMAKE_SYSTEM_NAME}")
endif(WIN32 OR MINGW)

# Try make fake host tuple for regress and isolation tests
if(MINGW AND VOID_POINTER_SIZE EQUAL 8)
	set(HOST_TUPLE "x86_64-w64-mingw32")
elseif(MINGW AND VOID_POINTER_SIZE EQUAL 4)
	set(HOST_TUPLE "i686-pc-mingw32")
elseif(CYGWIN)
	set(HOST_TUPLE "i686-pc-cygwin")
elseif(MSVC AND VOID_POINTER_SIZE EQUAL 4)
	set(HOST_TUPLE "i686-pc-win32vc")
else()
	set(HOST_TUPLE ${CMAKE_HOST_SYSTEM})
endif()

set(WITH_UUID "OFF" CACHE STRING "type of uuid lib [bsd, e2fs, ossp]")
if(WITH_UUID)
	find_package(LibUUID)

	if(WITH_UUID STREQUAL "bsd")
		set(HAVE_UUID_BSD 1)
		set(UUID_EXTRA_OBJS
			${PROJECT_SOURCE_DIR}/contrib/pgcrypto/md5.c
			${PROJECT_SOURCE_DIR}/contrib/pgcrypto/sha1.c
		)
	elseif(WITH_UUID STREQUAL "e2fs")
		set(HAVE_UUID_E2FS 1)
		set(UUID_EXTRA_OBJS
			${PROJECT_SOURCE_DIR}/contrib/pgcrypto/md5.c
			${PROJECT_SOURCE_DIR}/contrib/pgcrypto/sha1.c
		)
	elseif(WITH_UUID STREQUAL "ossp")
		set(HAVE_UUID_OSSP 1)
		set(UUID_EXTRA_OBJS "")
	else()
		message(WARNING "Not correct type of uuid lib:${WITH_UUID}")
	endif()
endif()

#file(REMOVE ${PROJECT_SOURCE_DIR}/src/include/pg_config_os.h)
file(GENERATE OUTPUT ${PROJECT_SOURCE_DIR}/src/include/pg_config_os.h
	INPUT ${pgos_include_SRCS})

if(MINGW OR MSVC)
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

configure_file(
	"${PROJECT_SOURCE_DIR}/src/interfaces/ecpg/include/ecpg_config_cmake.in"
	"${PROJECT_SOURCE_DIR}/src/interfaces/ecpg/include/ecpg_config.h"
)

configure_file(
	"${PROJECT_SOURCE_DIR}/PGXS.cmake.in"
	"${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PGXS.cmake"
	@ONLY
)

install(FILES ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/PGXS.cmake
		DESTINATION ${LIBDIR}/cmake)


option(CMAKE_USE_FOLDERS "Enable folder grouping of projects in IDEs." ON)
mark_as_advanced(CMAKE_USE_FOLDERS)

macro(CMAKE_SET_TARGET_FOLDER tgt folder)
	if(CMAKE_USE_FOLDERS)
		set_property(GLOBAL PROPERTY USE_FOLDERS ON)
		if(MSVC AND TARGET ${tgt})
			set_property(TARGET "${tgt}" PROPERTY FOLDER "${folder}")
		endif()
	else()
		set_property(GLOBAL PROPERTY USE_FOLDERS OFF)
	endif()
endmacro()

# Add sub-directories
add_subdirectory(src)
add_subdirectory(contrib)

message(STATUS "------------------")
message(STATUS "    Conclusion")
message(STATUS "Host tuple: ${HOST_TUPLE}")
message(STATUS "OpenSSL: ${OPENSSL_FOUND} version: ${OPENSSL_VERSION} libs: ${OPENSSL_LIBRARIES} include: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "Python: ${PYTHONINTERP_FOUND}")
message(STATUS "PythonLibs: ${PYTHONLIBS_FOUND}")
message(STATUS "PerlLibs: ${PERLLIBS_FOUND}")
message(STATUS "LibXML2: ${LIBXML2_FOUND}")
message(STATUS "LibXslt: ${LIBXSLT_FOUND}")
message(STATUS "TCL: ${TCL_FOUND}")
message(STATUS "PAM: ${PAM_LIB}")
message(STATUS "PROVE (TAP tests): ${PROVE}")
message(STATUS "Strong random source: ${STRONG_RANDOM_SOURCE}")
message(STATUS "------------------")
