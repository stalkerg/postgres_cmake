# We must will add generate target

if(PROVE)
	TAP_CHECK("ssl_tap" "${TAP_FLAGS}" "t/*.pl")
endif()
