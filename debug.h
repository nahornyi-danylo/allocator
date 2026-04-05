#include <stdio.h>

#ifdef DEBUG_PRINT_EXTRA
#define debug_print_extra(...) printf(__VA_ARGS__)
#else
#define debug_print_extra(...) 
#endif
