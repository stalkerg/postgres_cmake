include_directories(BEFORE
	"${PROJECT_SOURCE_DIR}/src/include/libpq"
	"${PROJECT_SOURCE_DIR}/src/interfaces/libpq"
	"${PROJECT_SOURCE_DIR}/src/bin/pg_dump"
)

add_executable(pg_config
	pg_config.c
)

target_link_libraries(pg_config
	pgcommon
	pgport
)

CMAKE_SET_TARGET_FOLDER(pg_config bin)

install(TARGETS pg_config
		RUNTIME DESTINATION ${PGBINDIR}
		LIBRARY DESTINATION ${LIBDIR})

if(ENABLE_NLS)
	MAKE_MO(pg_config "cs;de;es;fr;it;ja;ko;nb;pl;pt_BR;ro;ru;sv;ta;tr;zh_CN;zh_TW;he")
endif()

if(PROVE)
	TAP_CHECK("pg_config_tap" "${TAP_FLAGS}" "t/*.pl")
endif()
