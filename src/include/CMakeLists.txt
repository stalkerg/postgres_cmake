set(INCLUDEDIR_INTERNAL ${PKGINCLUDEDIR}/internal)

# These headers are needed by the public headers of the interfaces.
install(FILES
	postgres_ext.h
	pg_config.h
	pg_config_ext.h
	pg_config_os.h
	pg_config_manual.h
	DESTINATION ${INCLUDEDIR})
install(FILES
	libpq/libpq-fs.h
	DESTINATION ${INCLUDEDIR}/libpq)

# These headers are needed by the not-so-public headers of the interfaces.
install(FILES
	c.h
	port.h
	postgres_fe.h
	DESTINATION ${INCLUDEDIR_INTERNAL})
install(FILES
	libpq/pqcomm.h
	DESTINATION ${INCLUDEDIR_INTERNAL}/libpq)

# These headers are needed for server-side development
set(SUBDIRS
	access
	bootstrap
	catalog
	commands
	common
	datatype
	executor
	foreign
	lib
	libpq
	mb
	nodes
	optimizer
	parser
	postmaster
	regex
	replication
	rewrite
	storage
	tcop
	snowball
	tsearch
	utils
	port
	portability
)

install(FILES
	c.h
	dynloader.h
	fmgr.h
	funcapi.h
	getaddrinfo.h
	getopt_long.h
	miscadmin.h
	pg_config_ext.h
	pg_config.h
	pg_config_manual.h
	pg_config_os.h
	pg_getopt.h
	pgstat.h
	pgtar.h
	pgtime.h
	pg_trace.h
	port.h
	postgres_ext.h
	postgres_fe.h
	postgres.h
	rusagestub.h
	windowapi.h
	DESTINATION ${INCLUDEDIRSERVER})
install(DIRECTORY ${SUBDIRS} DESTINATION ${INCLUDEDIRSERVER})
