set(C99_C_FLAG_CANDIDATES
   " "
   "-std=c99"
   "-std=gnu99"
   "-c99"
   "-AC99"
   "-xc99=all"
   "-qlanglvl=extc99"
)

message(STATUS "Check flexible array support")
if(NOT MSVC)
	foreach(FLAG ${C99_C_FLAG_CANDIDATES})
		set(CMAKE_REQUIRED_FLAGS "${FLAG}")
		unset(FLEXIBLE_ARRAY_MEMBER CACHE)
		CHECK_C_SOURCE_COMPILES("
			int main(void){
				int x = 10;
				int y[x];
				return 0;
			}
		" FLEXIBLE_ARRAY_MEMBER)
		if(FLEXIBLE_ARRAY_MEMBER)
			set(C99_C_FLAGS_INTERNAL "${FLAG}")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
			break()
		endif(FLEXIBLE_ARRAY_MEMBER)
	endforeach(FLAG ${C99_C_FLAG_CANDIDATES})
endif()

if(FLEXIBLE_ARRAY_MEMBER)
	message(STATUS "Check flexible array support - yes with ${C99_C_FLAGS_INTERNAL}")
	set(FLEXIBLE_ARRAY_MEMBER "")
else(FLEXIBLE_ARRAY_MEMBER)
	message(STATUS "Check flexible array support - no")
	set(FLEXIBLE_ARRAY_MEMBER 1)
endif(FLEXIBLE_ARRAY_MEMBER)