set(CMAKE_REQUIRED_INCLUDES "stdio.h")
foreach(pgac_modifier "ll" "q" "I64")
	unset(LONG_LONG_INT_MODIFIER CACHE)
	CHECK_C_SOURCE_COMPILES("
		typedef long long int ac_int64;

		ac_int64 a = 20000001;
		ac_int64 b = 40000005;

		int does_int64_snprintf_work() {
			ac_int64 c;
			char buf[100];
			if (sizeof(ac_int64) != 8)
				return 0;	/* doesn't look like the right size */
			
			c = a * b;
			snprintf(buf, 100, \"%${pgac_modifier}d\", c);
			if (strcmp(buf, \"800000140000005\") != 0)
				return 0;	/* either multiply or snprintf is busted */
			return 1;
		}
		main() {
			exit(! does_int64_snprintf_work());
		}
	" LONG_LONG_INT_MODIFIER)
	if(LONG_LONG_INT_MODIFIER)
		set(LONG_LONG_INT_MODIFIER ${pgac_modifier})
		break()
	endif(LONG_LONG_INT_MODIFIER)
endforeach(pgac_modifier)
message(STATUS "LONG_LONG_INT_MODIFIER ${LONG_LONG_INT_MODIFIER}")