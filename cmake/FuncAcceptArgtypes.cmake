if(HAVE_SYS_TYPES_H)
	set(INCLUDE_SYS_TYPES_H "#include <sys/types.h>")
endif(HAVE_SYS_TYPES_H)

if(HAVE_SYS_SOCKET_H)
	set(INCLUDE_SYS_SOCKET_H "#include <sys/socket.h>")
endif(HAVE_SYS_SOCKET_H)


message(STATUS "Looking for accept function args")
set(CMAKE_REQUIRED_QUIET 1)
foreach(ac_cv_func_accept_return "int" "unsigned int PASCAL" "SOCKET WSAAPI")
	foreach(ac_cv_func_accept_arg1 "unsigned int" "int" "SOCKET")
		foreach(ac_cv_func_accept_arg2 "struct sockaddr *" "const struct sockaddr *" "void *")
			foreach(ac_cv_func_accept_arg3 "int" "size_t" "socklen_t" "unsigned int" "void")
				unset(AC_FUNC_ACCEPT CACHE)
				CHECK_C_SOURCE_COMPILES("
					${INCLUDE_SYS_TYPES_H}
					${INCLUDE_SYS_SOCKET_H}
					extern ${ac_cv_func_accept_return} accept (${ac_cv_func_accept_arg1}, ${ac_cv_func_accept_arg2}, ${ac_cv_func_accept_arg3} *);
					int main(void){return 0;}
				" AC_FUNC_ACCEPT)
				if(AC_FUNC_ACCEPT)
					set(ACCEPT_TYPE_RETURN ${ac_cv_func_accept_return})
					set(ACCEPT_TYPE_ARG1 ${ac_cv_func_accept_arg1})
					set(ACCEPT_TYPE_ARG2 ${ac_cv_func_accept_arg2})
					set(ACCEPT_TYPE_ARG3 ${ac_cv_func_accept_arg3})
					break()
				endif(AC_FUNC_ACCEPT)
			endforeach(ac_cv_func_accept_arg3)
			if(AC_FUNC_ACCEPT)
				break()
			endif(AC_FUNC_ACCEPT)
		endforeach(ac_cv_func_accept_arg2)
		if(AC_FUNC_ACCEPT)
			break()
		endif(AC_FUNC_ACCEPT)
	endforeach(ac_cv_func_accept_arg1)
	if(AC_FUNC_ACCEPT)
		break()
	endif(AC_FUNC_ACCEPT)
endforeach(ac_cv_func_accept_return)
unset(CMAKE_REQUIRED_QUIET)

if(NOT AC_FUNC_ACCEPT)
	message(ERROR "could not determine argument types")
endif(NOT AC_FUNC_ACCEPT)
if(ACCEPT_TYPE_ARG3 STREQUAL "void")
	set(ACCEPT_TYPE_ARG3 "int")
endif()

message(STATUS "Looking for accept function args - found ${ACCEPT_TYPE_RETURN}, ${ACCEPT_TYPE_ARG1}, ${ACCEPT_TYPE_ARG2}, ${ACCEPT_TYPE_ARG3} *")
