#include <stdio.h>
#include "block.h"

// assuming the check for size and in use has been done elsewhere
// and the size is aligned
void block_split(block *b, size_t size) {
    size_t prevSize = block_get_size(b);
    size_t newSize = prevSize - size - ALIGNED_BLOCK_SIZE;

    block_set_size(b, size);

    block *newBlock = block_next(b);
    // since we assume that sizes are already properly aligned, setting the sizes directly will also zero
    // the bitfields
    newBlock->size = newSize;
    newBlock->prevSize = size;
#ifdef PAGED
    newBlock->offset = b->offset + size + ALIGNED_BLOCK_SIZE;
#endif
    if (block_get_is_last_bit(b)) {
        block_set_is_last_bit(b, 0);
        block_set_is_last_bit(newBlock, 1);

        return;
    }
    
    block *next = block_next(newBlock);
    block_set_prev_size(next, newSize);
}

// again assuming checks were done already, and merging with the right neighbor
void block_merge(block *b) {
    block *blockToMerge = block_next(b);
    size_t newSize = block_get_size(b) + block_get_size(blockToMerge) + ALIGNED_BLOCK_SIZE;
    block_set_size(b, newSize);
    
    if (block_get_is_last_bit(blockToMerge)) {
        block_set_is_last_bit(b, 1);
        return;
    }

    block *newNext = block_next(b);
    block_set_prev_size(newNext, newSize);
}

void block_print(block *b) {
    printf("Block %p {\n", b);
    printf("\tsize = %lu\n", block_get_size(b));
    printf("\tprevSize = %lu\n", block_get_prev_size(b));
    printf("\tfirst = %u\n", block_get_is_first_bit(b));
    printf("\tlast = %u\n", block_get_is_last_bit(b));
    printf("\tin use = %u\n", block_get_in_use_bit(b));
    printf("}\n");
}

