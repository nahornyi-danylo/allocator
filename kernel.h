#include "block.h"

block *kernel_get_mem(size_t size);
void kernel_free_mem(void *p, size_t size);
bool kernel_is_partial_free_available();
void kernel_partial_free(void *p, size_t size);

#ifdef PAGED
void kernel_discard_page_range(void *p, size_t size);
#endif
