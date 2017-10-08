include_directories(
	"${PROJECT_SOURCE_DIR}/src/include"
)
if(ZLIB_FOUND)
	include_directories(BEFORE ${ZLIB_INCLUDE_DIRS})
endif()

set(extension_name pgcrypto)

set(pgcrypto_src
	pgcrypto.c
	px.c
	px-hmac.c
	px-crypt.c
	crypt-gensalt.c
	crypt-blowfish.c
	crypt-des.c
	crypt-md5.c
	mbuf.c
	pgp.c
	pgp-armor.c
	pgp-cfb.c
	pgp-compress.c
	pgp-decrypt.c
	pgp-encrypt.c
	pgp-info.c
	pgp-mpi.c
	pgp-pubdec.c
	pgp-pubenc.c
	pgp-pubkey.c
	pgp-s2k.c
	pgp-pgsql.c
)

set(regress_files
	init
	md5
	sha1
	hmac-md5
	hmac-sha1
	blowfish
	rijndael
	crypt-des
	crypt-md5
	crypt-blowfish
	crypt-xdes
	pgp-armor
	pgp-decrypt
	pgp-encrypt
	pgp-pubkey-decrypt
	pgp-pubkey-encrypt
	pgp-info
)

if(OPENSSL_FOUND)
	include_directories(BEFORE ${OPENSSL_INCLUDE_DIR})
	set(pgcrypto_src ${pgcrypto_src} openssl.c pgp-mpi-openssl.c)
	set(regress_files ${regress_files}
		sha2
		des
		3des
		cast5
	)
else()
	add_executable(gen-rtab rijndael.c)
	target_link_libraries(gen-rtab pgport)
	target_compile_definitions(gen-rtab PRIVATE -DPRINT_TABS)

	add_custom_command(
		DEPENDS gen-rtab
		OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/rijndael.tbl
		COMMAND gen-rtab > rijndael.tbl
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	set(pgcrypto_src ${pgcrypto_src}
		md5.c
		sha1.c
		internal.c
		internal-sha2.c
		blf.c
		rijndael.tbl
		rijndael.c
		pgp-mpi-internal.c
		imath.c
	)
	set(regress_files ${regress_files} sha2)
endif()

if(ZLIB_FOUND)
	set(regress_files ${regress_files} pgp-compression)
endif()

add_library(${extension_name} ${PLUGIN_TYPE} ${pgcrypto_src})
target_link_libraries(${extension_name}
	${contrib_libs}
)
if(OPENSSL_FOUND)
	target_link_libraries(${extension_name} ${OPENSSL_LIBRARIES})
endif()
if(CRYPT_LIB)
	target_link_libraries(${extension_name} ${CRYPT_LIB})
endif()
if(ZLIB_FOUND)
	target_link_libraries(${extension_name} ${ZLIB_LIBRARIES})
endif()
if(WIN32)
	target_link_libraries(${extension_name} ws2_32)
endif()
if(MSVC)
	gen_def(${extension_name})
endif()
if(NOT PGXS)
	CMAKE_SET_TARGET_FOLDER(${extension_name} contrib)
endif()
set_target_properties(${extension_name} PROPERTIES PREFIX "")
add_dependencies(${extension_name} postgres)
install(TARGETS ${extension_name}
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})
install(FILES
	${extension_name}.control
	${extension_name}--1.3.sql
	${extension_name}--1.0--1.1.sql
	${extension_name}--1.1--1.2.sql
	${extension_name}--1.2--1.3.sql
	${extension_name}--unpackaged--1.0.sql
	DESTINATION ${PGSHAREDIR}/extension)


CONTRIB_REGRESS_CHECK(${extension_name} "" "${regress_files}")
