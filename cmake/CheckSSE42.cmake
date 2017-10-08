foreach(FLAG ";-msse4.2")
	unset(HAVE_SSE42_INTRINSICS CACHE)
	set(CMAKE_REQUIRED_FLAGS ${FLAG})
	check_c_source_runs("
		#include <nmmintrin.h>
		int main(void) {
			unsigned int crc = 0;
			crc = _mm_crc32_u8(crc, 0);
			crc = _mm_crc32_u32(crc, 0);
			/* return computed value, to prevent the above being optimized away */
			return !(crc == 0);
		}
	" HAVE_SSE42_INTRINSICS)
	if (HAVE_SSE42_INTRINSICS)
		set(CFLAGS_SSE42 ${FLAG})
		break()
	endif()
endforeach()

message(STATUS "SSE4.2 flags: ${CFLAGS_SSE42}")
set(CMAKE_REQUIRED_FLAGS ${CFLAGS_SSE42})
check_c_source_compiles("
	#ifndef __SSE4_2__
	#error __SSE4_2__ not defined
	#endif
	int main(void){
		return 0;
	}
" HAVE_SSE42)
set(CMAKE_REQUIRED_FLAGS "")
