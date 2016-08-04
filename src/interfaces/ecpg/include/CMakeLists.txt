set(INCLUDEDIR_INTERNAL ${PKGINCLUDEDIR}/internal)

install(FILES
	ecpgerrno.h
	ecpglib.h
	ecpgtype.h
	sqlca.h
	sql3types.h
	ecpg_informix.h
	pgtypes_error.h
	pgtypes_numeric.h
	pgtypes_timestamp.h
	pgtypes_date.h
	pgtypes_interval.h
	sqlda.h
	sqlda-compat.h
	sqlda-native.h
	ecpg_config.h
	DESTINATION ${INCLUDEDIR})

install(FILES
	datetime.h
	decimal.h
	sqltypes.h
	DESTINATION ${PKGINCLUDEDIR}/informix/esql)
