Requirements
============
For Unix system you can build postgres with CMake 3.0 and higher.
For Windows build you must use CMake 3.7.1 and higher.
If you don't have CMake on your system (Solaris, AIX or old Linux),
you can build it from source without any problems.


Basic commands
==============
Full documentations you can find here:
https://cmake.org/cmake/help/latest/
and small but usefull tutorial here:
https://cmake.org/cmake-tutorial/

Very basic usage in Unix systems:
cmake ..

where .. is path to postgres source dir. To use Ninja instead
of GNU Make you must write:

cmake .. -G "Ninja"

To explore other generators use:
cmake .. -G help

Once you have generated Makefile or Ninja you should run
make or ninja commands to build postgres. If you want to show all compiler
option you must set env variable VERBOSE. Example:
VERBOSE=1 make

Choose configure options
========================
All options can be set via -DNAME_OPTION=(ON|OFF|NUMBER|STRING) param in command line.
Example:
cmake .. -DCMAKE_INSTALL_PREFIX="/usr/local" -DWITH_LIBXML=ON

here we set install prefix and enable build with libXML.

Set CFLAGS or LDFLAGS
=====================
CFLAGS or LDFLAGS you can set only on the generation stage as
environment variable. Example:
CFLAGS="-s -O2" cmake ..

you can't change CFLAGS if project has been generated.
This example won't work:
CFLAGS="-s -O3" make

You must remove cache and generate project again:
rm ./CMakeCache.txt
CFLAGS="-s -O3" cmake ..

Targets
=======
In CMake you can have only one main target other targets cannot be installed.
Also we have only one enter point to the project.
To build postgres input:
make

or
make all

All test targets implemented as optional targets:
make check
make isolation_check
make contrib_check
make ecpg_check
make modules_check

also you can use installcheck insted check in target name to run tests for installed postgres.
Example:
make installcheck
make isolation_installcheck

Some tests we can run only if corresponding option is enabled:
make plperl_check
make pltcl_check
make plpython_check

And experemental TAP tests:
make commit_ts_tap_check
make bloom_tap_check
make initdb_tap_check
make pg_basebackup_tap_check
make pg_config_tap_check
make pg_controldata_tap_check
make pg_ctl_tap_check
make pg_dump_tap_check
make pg_rewind_tap_check
make pgbench_tap_check
make scripts_tap_check
make test_pg_dump_tap_check
make recovery_tap_check
make ssl_tap_check

Postgres CMake build options
============================
**-DWITH_PERL** default OFF, optional

**-DWITH_OPENSSL** default OFF, optional

**-DOPENSSL_ROOT_DIR** Set this variable to the root installation of OpenSSL

**-DWITH_PYTHON** default OFF, optional

**-DWITH_LIBXML** default OFF, optional

**-DWITH_TCL** default OFF, optional

**-DWITH_ICU** default OFF, optional

**-DENABLE_NLS** default OFF

**-DENABLE_GSS** default OFF

**-DUSE_PAM** default OFF

**-DUSE_ASSERT_CHECKING** default OFF

**-DUSE_BONJOUR** default OFF

**-DUSE_SYSTEMD** default OFF

**-DUSE_BSD_AUTH** default OFF

**-DUSE_LDAP** default OFF

**-DPROFILE_PID_DIR** default OFF, only for GCC, enable to allow profiling output to be saved separately for each process

**-DUSE_FLOAT4_BYVAL** default ON

**-DUSE_FLOAT8_BYVAL** default ON for 64bit system

**-DHAVE_ATOMICS** default ON

**-DHAVE_SPINLOCKS** default ON

**-DUSE_INTEGER_DATETIMES** default ON, 64-bit integer timestamp and interval support

**-DHAVE_SYMLINK** only for WIN32, default ON

**-DPG_KRB_SRVNAM** default 'postgres', the name of the default PostgreSQL service principal in Kerberos (GSSAPI)

**-DCMAKE_USE_FOLDERS** default ON, folder grouping of projects in IDEs, actual for MSVC

**-DUSE_REPL_SNPRINTF** default not use but if ON you can switch OFF

**-DWITH_TAP** default OFF, experemental support TAP tests

**-DSTRONG_RANDOM** default ON, strong random number source

**-DSTRONG_RANDOM_SOURCE** default OFF (autodetect), which random number source to use - openssl, win32, dev

Only **MSVC** options:

**-DUSE_FP_STRICT** default ON, if we use /fp:stirct compiler option we much closer to C99 standart in float part

CMake options for modules
=========================
To find some libraries we use standard CMake modules and you can read documentation
on the CMake site.

For example setup for python2.7:
-DPython_ADDITIONAL_VERSIONS=2.7 -DPYTHON_EXECUTABLE=/usr/bin/python2.7

For details you can read:
Python lib options: https://cmake.org/cmake/help/latest/module/FindPythonLibs.html
Python interpreter options: https://cmake.org/cmake/help/latest/module/FindPythonInterp.html
Perl libs: https://cmake.org/cmake/help/latest/module/FindPerlLibs.html
OpenSSL: https://cmake.org/cmake/help/latest/module/FindOpenSSL.html
Curses: https://cmake.org/cmake/help/latest/module/FindCurses.htm

Uncommon CMake Examples
=======================

Solaris 10, Sun-Fire-V210:
CFLAGS="-m64 -mcpu=v9" cmake .. -DFIND_LIBRARY_USE_LIB64_PATHS=ON -DCURSES_LIBRARY=/usr/ccs/lib/sparcv9/libcurses.so -DCURSES_FORM_LIBRARY=/usr/ccs/lib/sparcv9/libform.so -DCMAKE_INSTALL_PREFIX="/home/stalkerg/postgresql_bin"

AIX 7.1, Power8, gcc 4.8:
CFLAGS="-std=c11 -mcpu=power8 -mtune=power8 -mveclibabi=mass -maix64" LDFLAGS="-Wl,-bbigtoc"  cmake ..


CMake build and test howto
==========================
git clone https://github.com/stalkerg/postgres_cmake.git
cd ./postgres_cmake
#build dir is optional but best way for CMake
mkdir build
cd build
CFLAGS="-s -O2" cmake .. -DCMAKE_INSTALL_PREFIX="/usr/local" -DWITH_PERL=ON -DWITH_LIBXML=ON
#strip binary + O2 optimisation, for CLang you must use -strip-all insted -s
#also you can use -G Ninja for Ninja make system
make -j2
make check
make isolation_check
make contrib_check

#Optional:
make ecpg_check
make modules_check
make plperl_check
make pltcl_check

#For example setup for python2.7:
#-DPython_ADDITIONAL_VERSIONS=2.7 -DPYTHON_EXECUTABLE=/usr/bin/python2.7
make plpython_check

#TAP tests (experimental):
make commit_ts_tap_check
make bloom_tap_check
make initdb_tap_check
make pg_basebackup_tap_check
make pg_config_tap_check
make pg_controldata_tap_check
make pg_ctl_tap_check
make pg_dump_tap_check
make pg_rewind_tap_check
make pgbench_tap_check
make scripts_tap_check
make test_pg_dump_tap_check
make recovery_tap_check
make ssl_tap_check

make install
make installcheck

Minimal build for Windows 7/10 with VisualStudio 2015/2017
==========================================================

First, you should install next programms:
CMake - https://cmake.org/ (64 bit if you have 64 bit windows)
Perl - https://www.activestate.com/activeperl/downloads (not forget add perl to %PATH% in options, 64 bit for build 64 bit Postgres with Perl)
Bison and Flex - https://sourceforge.net/projects/winflexbison/  (after unzip to folder please add this folder to %PATH%)
Sed - http://gnuwin32.sourceforge.net/packages/sed.htm
Diff - http://gnuwin32.sourceforge.net/packages/diffutils.htm

After installing Sed and Diff please add "C:/Program Files (x86)/GnuWin32/bin/" to %PATH% .
How change %PATH% on windows? Look here https://www.computerhope.com/issues/ch000549.htm .
Now you can clone our repo create "build" dir inside and open CMake GUI.
In CMake GUI you should choose current repo folder and new "build" as for build and click "Configure".
After this process, you should change CMAKE_INSTALL_PREFIX variable to something like "C:\\postgres_bin" (and create this folder). 
Now you can click on "Generate" for generation MSVC solution and after you can open this solution in your MSVC. 
In MSVC important find ALL_BUILD "project" and build this target. After complete, you should build INSTALL. 
Also, after all, that you can run build "insatllcheck".
