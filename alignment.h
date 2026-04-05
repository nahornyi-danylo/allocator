#include <stddef.h>

#define ALIGN (_Alignof(max_align_t))
#define TO_ALIGNED(b) (((b) + (ALIGN - 1)) & ~(ALIGN - 1))
