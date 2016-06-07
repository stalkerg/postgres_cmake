find_library( LIBSOCKET_LIBRARY NAMES libsocket socket )
mark_as_advanced( LIBSOCKET_LIBRARY )

# handle the QUIETLY and REQUIRED arguments and set LIBSOCKET_FOUND to TRUE if
# all listed variables are TRUE
include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( LIBSOCKET DEFAULT_MSG LIBSOCKET_LIBRARY )

if( LIBSOCKET_FOUND )
	set( LIBSOCKET_LIBRARIES ${LIBSOCKET_LIBRARY} )
endif( LIBSOCKET_FOUND )
