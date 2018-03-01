CHECK_C_SOURCE_COMPILES (
  "int test (void *restrict x); int main (void) {return 0;}"
  HAVE_RESTRICT)
