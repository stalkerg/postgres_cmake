include_directories("${PROJECT_SOURCE_DIR}/src/include")

set(extension_name btree_gist)

add_library(${extension_name} ${PLUGIN_TYPE}
	${extension_name}.c
	btree_utils_num.c
	btree_utils_var.c
	btree_int2.c
	btree_int4.c
	btree_int8.c
	btree_float4.c
	btree_float8.c
	btree_cash.c
	btree_oid.c
	btree_ts.c
	btree_time.c
	btree_date.c
	btree_interval.c
	btree_macaddr.c
	btree_macaddr8.c
	btree_inet.c
	btree_text.c
	btree_bytea.c
	btree_bit.c
	btree_numeric.c
	btree_uuid.c
	btree_enum.c
)
target_link_libraries(${extension_name} ${contrib_libs} ${LIB_M})
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
gen_def(${extension_name})
if(NOT PGXS)
	CMAKE_SET_TARGET_FOLDER(${extension_name} contrib)
endif()
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	${extension_name}.control
	${extension_name}--1.2.sql
	${extension_name}--1.4--1.5.sql
	${extension_name}--1.3--1.4.sql
	${extension_name}--1.2--1.3.sql
	${extension_name}--1.1--1.2.sql
	${extension_name}--1.0--1.1.sql
	${extension_name}--unpackaged--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)


set(regres_files
	init
	int2
	int4
	int8
	float4
	float8
	cash
	oid
	timestamp
	timestamptz
	time
	timetz
	date
	interval
	macaddr
	inet
	cidr
	text
	varchar
	char
	bytea
	bit
	varbit
	numeric
	not_equal
)
CONTRIB_REGRESS_CHECK(${extension_name} "" "${regres_files}")
