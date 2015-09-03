macro(CHECK_TYPE_ALIGNMENT TYPE NAME)
	if(NOT ${NAME})
		message(STATUS "Check alignment of ${TYPE}")
		
		set(INCLUDE_HEADERS "#include <stddef.h>
			#include <stdio.h>
			#include <stdlib.h>")

		if(HAVE_STDINT_H)
			set(INCLUDE_HEADERS "${INCLUDE_HEADERS}\n#include <stdint.h>\n")
		endif(HAVE_STDINT_H)

		file(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/check_${NAME}_alignment.c"
			"${INCLUDE_HEADERS}
			int main(){
				char diff;
				struct foo {char a; ${TYPE} b;};
				struct foo *p = (struct foo *) malloc(sizeof(struct foo));
				diff = ((char *)&p->b) - ((char *)&p->a);
				return diff;}
		")

		try_run(${NAME} COMPILE_RESULT "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/"
			"${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/check_${NAME}_alignment.c")

		message(STATUS "Check alignment of ${TYPE} - ${${NAME}}")

	endif(NOT ${NAME})
endmacro(CHECK_TYPE_ALIGNMENT TYPE NAME ALIGNOF_TYPE)