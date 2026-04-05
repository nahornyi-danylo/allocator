#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "config.h"
#include "alignment.h"

#ifndef BLOCK_INCLUDE_GUARD
#define BLOCK_INCLUDE_GUARD

typedef struct {
    // bit 0 -> in use flag
    size_t size;
    // bit 0 -> last in arena flag
    // bit 1 -> first in arena flag
    size_t prevSize;
#ifdef PAGED
    size_t offset;
#endif
} block;

#define ALIGNED_BLOCK_SIZE TO_ALIGNED(sizeof(block))
#define BLOCK_IN_USE (1u << 0)
#define BLOCK_LAST (1u << 0)
#define BLOCK_FIRST (1u << 1)

static inline size_t block_get_size(block *b) {
    return b->size & ~BLOCK_IN_USE; 
}

static inline size_t block_get_prev_size(block *b) {
    return b->prevSize & ~(BLOCK_LAST | BLOCK_FIRST);
}

static inline bool block_get_in_use_bit(block *b) {
    return b->size & BLOCK_IN_USE;
}

static inline bool block_get_is_first_bit(block *b) {
    return b->prevSize & BLOCK_FIRST;
}

static inline bool block_get_is_last_bit(block *b) {
    return b->prevSize & BLOCK_LAST;
}

static inline void block_set_size(block *b, size_t value) {
    b->size = (b->size & BLOCK_IN_USE) | (value & ~BLOCK_IN_USE);
}
static inline void block_set_prev_size(block *b, size_t value) {
    b->prevSize = (b->prevSize & (BLOCK_LAST | BLOCK_FIRST)) | (value & ~(BLOCK_LAST | BLOCK_FIRST));
}

static inline void block_set_in_use_bit(block *b, bool value) {
    if (value) {
        b->size |= BLOCK_IN_USE;
    }
    else {
        b->size &= ~BLOCK_IN_USE;
    }
}

static inline void block_set_is_first_bit(block *b, bool value) {
    if (value) {
        b->prevSize |= BLOCK_FIRST;
    }
    else {
        b->prevSize &= ~BLOCK_FIRST;
    }
}

static inline void block_set_is_last_bit(block *b, bool value) {
    if (value) {
        b->prevSize |= BLOCK_LAST;
    }
    else {
        b->prevSize &= ~BLOCK_LAST;
    }
}

static inline block *block_next(block *b) {
    return (block *)((char *)b + block_get_size(b) + ALIGNED_BLOCK_SIZE);
}

static inline block *block_prev(block *b) {
    return (block *)((char *)b - block_get_prev_size(b) - ALIGNED_BLOCK_SIZE);
}

static inline block *payload_to_block(void *p) {
    return (block *)((char *)p - ALIGNED_BLOCK_SIZE);
}

static inline void *block_to_payload(block *b) {
    return (char *)b + ALIGNED_BLOCK_SIZE;
}

#endif

void block_split(block *b, size_t size);
void block_merge(block *b);
void block_print(block *b);

