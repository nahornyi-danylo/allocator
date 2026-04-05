#define _DEFAULT_SOURCE
#include <sys/mman.h>
#include <stdlib.h>
#include "kernel.h"
#include "config.h"

#define DEFAULT_ARENA_CACHE_SIZE 1024

static void *cache[DEFAULT_ARENA_CACHE_SIZE];
static size_t nCached = 0;

block *kernel_get_mem(size_t size) {
    void *mem;
    block *b;
    size_t realSize;

    if (size <= MAX_SINGLE_ARENA_ALLOCATION_SIZE) {
        if (nCached) {
            mem = cache[nCached - 1];
            nCached--;
        }
        else {
            mem = mmap(NULL, DEFAULT_ARENA_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
            if (mem == MAP_FAILED) {
                perror("mmap failed");
                return NULL;
            }
        }
        realSize = DEFAULT_ARENA_SIZE;
    }
    else {
        mem = mmap(NULL, TO_PAGE_ALIGNED(size + ALIGNED_BLOCK_SIZE), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mem == MAP_FAILED) {
            perror("big mmap failed");
            return NULL;
        }
        realSize = TO_PAGE_ALIGNED(size + ALIGNED_BLOCK_SIZE); 
    }

    b = mem;
    b->offset = ALIGNED_BLOCK_SIZE;
    block_set_size(b, realSize - ALIGNED_BLOCK_SIZE);
    block_set_prev_size(b, 0);
    block_set_in_use_bit(b, 0);
    block_set_is_first_bit(b, 1);
    block_set_is_last_bit(b, 1);

    return b;
}

void kernel_free_mem(void *p, size_t size) {
    if (size > DEFAULT_ARENA_SIZE) {
        int res = munmap(p, size);
        if (res < 0) {
            perror("big munmap failed");
        }
        return;
    }

    size = DEFAULT_ARENA_SIZE;
    if (nCached < DEFAULT_ARENA_CACHE_SIZE) {
        cache[nCached++] = p;
        int res = madvise(p, size, MADV_FREE);
        if (res < 0) {
            perror("madvise in cache failed");
        }
    }
    else {
        int res = munmap(p, size);
        if (res < 0) {
            perror("munmap failed");
        }
    }
}

void kernel_discard_page_range(void *p, size_t size) {
    int res = madvise(p, size, MADV_FREE);
    if (res < 0) {
        perror("internal madvise failed");
    }
}

bool kernel_is_partial_free_available() {
    return true;
}

void kernel_partial_free(void *p, size_t size) {
    int res = munmap(p, size);
    if (res < 0) {
        perror("partial munmap failed");
    }
}
