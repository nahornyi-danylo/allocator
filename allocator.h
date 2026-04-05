#include <stddef.h>

void *mem_alloc(size_t size);
void mem_free(void *p);
void *mem_realloc(void *p, size_t size);
void mem_show();
