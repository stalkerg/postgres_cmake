
MODULES_REGRESS_CHECK("commit_ts" "--temp-config=${CMAKE_SOURCE_DIR}/src/test/modules/commit_ts/commit_ts.conf" "commit_timestamp")
if(PROVE)
	MODULES_TAP_CHECK("commit_ts" "${TAP_FLAGS}" "t/*.pl")
endif()
