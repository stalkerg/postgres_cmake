CMake build and test howto
==========================
```bash
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
```

CMake build options
===================
**-DWITH_PERL** default OFF, optional

**-DWITH_OPENSSL** default OFF, optional

**-DOPENSSL_ROOT_DIR** Set this variable to the root installation of OpenSSL

**-DWITH_PYTHON** default OFF, optional

**-DWITH_LIBXML** default OFF, optional

**-DWITH_TCL** default OFF, optional

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

**-DSTRONG_RANDOM** default ON, strong random number source

**-DSTRONG_RANDOM_SOURCE** default OFF (autodetect), which random number source to use - openssl, win32, dev

Only **MSVC** options:

**-DUSE_FP_STRICT** default ON, if we use /fp:stirct compiler option we much closer to C99 standart in float part

Not Common CMake Examples
==============

Solaris 10, Sun-Fire-V210:

CFLAGS="-m64 -mcpu=v9" cmake .. -DFIND_LIBRARY_USE_LIB64_PATHS=ON  -DWITH_OPENSSL=OFF -DCURSES_LIBRARY=/usr/ccs/lib/sparcv9/libcurses.so -DCURSES_FORM_LIBRARY=/usr/ccs/lib/sparcv9/libform.so -DCMAKE_INSTALL_PREFIX="/home/stalkerg/postgresql_bin"

AIX 7.1, Power8, gcc 4.8:

CFLAGS="-std=c11 -mcpu=power8 -mtune=power8 -mveclibabi=mass -maix64" LDFLAGS="-Wl,-bbigtoc"  cmake ..
