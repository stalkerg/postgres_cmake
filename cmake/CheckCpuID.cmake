CHECK_C_SOURCE_COMPILES("
	#include <cpuid.h>
	int main() {
		unsigned int exx[4] = {0, 0, 0, 0};
		__get_cpuid(1, &exx[0], &exx[1], &exx[2], &exx[3]);
		return 0;
	}
" HAVE__GET_CPUID)

CHECK_C_SOURCE_COMPILES("
	#include <intrin.h>
	int main() {
		unsigned int exx[4] = {0, 0, 0, 0};
		__cpuid(exx[0], 1);
		return 0;
	}
" HAVE__CPUID)
