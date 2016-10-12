[![Build Status](https://travis-ci.org/stalkerg/postgres_cmake.svg?branch=cmake)](https://travis-ci.org/stalkerg/postgres_cmake)

CMake build and test howto
==========================
```bash
git clone https://github.com/stalkerg/postgres_cmake.git
cd ./postgres_cmake
#build dir is optional but best way for CMake
mkdir build
cd build
CFLAGS="-s -O2" cmake .. -DCMAKE_INSTALL_PREFIX="/usr/local"
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
```

CMake build options
===================
**-DWITH_PERL** default ON, optional

**-DWITH_OPENSSL** default ON, optional

**-DOPENSSL_ROOT_DIR** Set this variable to the root installation of OpenSSL

**-DWITH_PYTHON** default ON, optional

**-DWITH_LIBXML** default ON, optional

**-DWITH_TCL** default ON, optional

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

**-DPG_KRB_SRVNAM** default "postgres", the name of the default PostgreSQL service principal in Kerberos (GSSAPI)

**-DCMAKE_USE_FOLDERS** default ON, folder grouping of projects in IDEs, actual for MSVC

**-DUSE_REPL_SNPRINTF** default not use but if ON you can switch OFF

**-DWITH_TAP** default OFF, experemental support TAP tests



PostgreSQL Database Management System
=====================================

This directory contains the source code distribution of the PostgreSQL
database management system.

PostgreSQL is an advanced object-relational database management system
that supports an extended subset of the SQL standard, including
transactions, foreign keys, subqueries, triggers, user-defined types
and functions.  This distribution also contains C language bindings.

PostgreSQL has many language interfaces, many of which are listed here:

	http://www.postgresql.org/download

See the file INSTALL for instructions on how to build and install
PostgreSQL.  That file also lists supported operating systems and
hardware platforms and contains information regarding any other
software packages that are required to build or run the PostgreSQL
system.  Copyright and license information can be found in the
file COPYRIGHT.  A comprehensive documentation set is included in this
distribution; it can be read as described in the installation
instructions.

The latest version of this software may be obtained at
http://www.postgresql.org/download/.  For more information look at our
web site located at http://www.postgresql.org/.
