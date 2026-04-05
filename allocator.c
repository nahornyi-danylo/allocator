#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "allocator.h"
#include "block.h"
#include "kernel.h"
#include "tree.h"
#include "config.h"

static rbnode *root = NULL;

void *mem_alloc(size_t size) {
    if (TO_ALIGNED(size) > MAX_SINGLE_ARENA_ALLOCATION_SIZE) {
        block *b = kernel_get_mem(size);
        if (!b) return NULL;
        block_set_in_use_bit(b, 1);
        return block_to_payload(b);
    }

    if (!root) {
        block *firstBlock = kernel_get_mem(MAX_SINGLE_ARENA_ALLOCATION_SIZE);
        if (!firstBlock) return NULL;
        tree_insert(&root, firstBlock);
    }

again: ;
    rbnode *node = root;
    block *b = NULL;

    while (node) {
        block *this = rbnode_to_block(node);
        size_t curSize = block_get_size(this);

        if (curSize >= TO_ALIGNED(size)) {
            b = this;
            node = node->left;
        }
        else {
            node = node->right;
        }
    }
    if (b) {
        // we always end up with the head of the list of the same size, after the tree search
        // if next node in the list exists, using it avoids copying the rbnode data
        treeNode *n = block_to_payload(b);
        if (n->lnode.next) {
            b = lnode_to_block(n->lnode.next);
        }

#ifdef DEBUG_PRINT_EXTRA
        printf("========\n");
        mem_show();
        printf("Size %lu found block with %lu\n", size, block_get_size(b));
#endif

        block_set_in_use_bit(b, 1);
        tree_delete(&root, b);

        if (block_get_size(b) >= TO_ALIGNED(size) + ALIGNED_BLOCK_SIZE + ALIGNED_TREENODE_SIZE) {
            block_split(b, TO_ALIGNED(size));
            tree_insert(&root, block_next(b));
        }

        return block_to_payload(b);
    }
    else {
        block *new = kernel_get_mem(MAX_SINGLE_ARENA_ALLOCATION_SIZE);
        if (!new) return NULL;

        tree_insert(&root, new);
        goto again;
    }
}

// a helper for finding free pages inside a block and discarding them
#ifdef PAGED
static void find_discard_free_pages(block *b) {
    size_t startRoundUpOffset = (b->offset + PAGE_SIZE - 1)/PAGE_SIZE * PAGE_SIZE;
    size_t endRoundDownOffset = (b->offset + block_get_size(b))/PAGE_SIZE * PAGE_SIZE;
    if (startRoundUpOffset < endRoundDownOffset) {
        uintptr_t arenaStart = (uintptr_t)block_to_payload(b) - b->offset;
        void *p = (void *)(arenaStart + startRoundUpOffset);
        kernel_discard_page_range(p, endRoundDownOffset - startRoundUpOffset);
    }
}
#endif

void mem_free(void *p) {
    if (!p) return;
    block *b = payload_to_block(p);
    block_set_in_use_bit(b, 0);

    if (block_get_size(b) > MAX_SINGLE_ARENA_ALLOCATION_SIZE) {
        kernel_free_mem(b, block_get_size(b) + ALIGNED_BLOCK_SIZE);
        return;
    }

    if (!block_get_is_last_bit(b)) {
        block *next = block_next(b);
        if (!block_get_in_use_bit(next)) {
            if (block_get_size(next) >= ALIGNED_TREENODE_SIZE) {
                tree_delete(&root, next);
            }
            block_merge(b);
        }
    }

    if (!block_get_is_first_bit(b)) {
        block *prev = block_prev(b);
        if (!block_get_in_use_bit(prev)) {
            if (block_get_size(prev) >= ALIGNED_TREENODE_SIZE) {
                tree_delete(&root, prev);
            }
            block_merge(prev);
            b = prev;
        }
    } 

    if (block_get_is_first_bit(b) && block_get_is_last_bit(b)) {
        kernel_free_mem(b, DEFAULT_ARENA_SIZE);
        return;
    }
    else {
      // blocks with size less then that of treeNode are basically left dangling
      // and unreachable until a free is called on a neighboring block
        if (block_get_size(b) >= ALIGNED_TREENODE_SIZE) {
            tree_insert(&root, b);
        }
    }
#ifdef PAGED
    find_discard_free_pages(b);
#endif
}

void *mem_realloc(void *p, size_t size) {
    if (!p) return mem_alloc(size);

    block *b = payload_to_block(p);
    size_t initialSize = block_get_size(b);

    if (initialSize > TO_ALIGNED(size)) {
        // big shrink
        if (initialSize > MAX_SINGLE_ARENA_ALLOCATION_SIZE) {
            if (kernel_is_partial_free_available()) {
                size_t new = TO_DEFAULT_ARENA_ALIGNED(ALIGNED_BLOCK_SIZE + TO_ALIGNED(size));
                size_t old = initialSize + ALIGNED_BLOCK_SIZE;
                if (new < old) {
                    kernel_partial_free((char *)b + new, old - new);
                    block_set_size(b, new - ALIGNED_BLOCK_SIZE);
                }
                return p;
            }
            size_t shrinkProportion = initialSize/TO_ALIGNED(size);
            if (shrinkProportion > 2) {
                void *ret = mem_alloc(size);
                if (!ret) return NULL;
                memcpy(ret, p, size);
                mem_free(p);
                return ret;
            }
            return p;
        }

        if (initialSize >= TO_ALIGNED(size) + ALIGNED_BLOCK_SIZE + ALIGNED_TREENODE_SIZE) {
            block_split(b, TO_ALIGNED(size));

            block *n = block_next(b);
            if (!block_get_is_last_bit(n)) {
                block *nn = block_next(n);
                if (!block_get_in_use_bit(nn)) {
                    if (block_get_size(nn) >= ALIGNED_TREENODE_SIZE) {
                        tree_delete(&root, nn);
                    }
                    block_merge(n);
                }
            }

            tree_insert(&root, n);
#ifdef PAGED
            find_discard_free_pages(n);
#endif
        }

        return p;
    }
    else if (initialSize == TO_ALIGNED(size)) return p;
    else {
        if (!block_get_is_last_bit(b)) {
            block *n = block_next(b);
            if (!block_get_in_use_bit(n) && block_get_size(n) + block_get_size(b) + ALIGNED_BLOCK_SIZE >= TO_ALIGNED(size)) {
                if (block_get_size(n) >= ALIGNED_TREENODE_SIZE) {
                    tree_delete(&root, n);
                }

                block_merge(b);

                if (block_get_size(b) >= TO_ALIGNED(size) + ALIGNED_BLOCK_SIZE + ALIGNED_TREENODE_SIZE) {
                    block_split(b, TO_ALIGNED(size));
                    tree_insert(&root, block_next(b));
                }

                return p;
            } 
        }
    }

    void *ret = mem_alloc(size);
    if (!ret) return NULL;
    memcpy(ret, p, initialSize);
    mem_free(p);
    return ret;
}

void mem_show_dfs(rbnode *from) {
    if (!from) return;
    block *b = rbnode_to_block(from);
    block_print(b);
    for (listNode *node = block_to_lnode(b)->next; node != NULL; node = node->next) {
        printf("<->\n");
        block *this = lnode_to_block(node);
        block_print(this);
    }
    printf("\n");

    mem_show_dfs(from->left);
    mem_show_dfs(from->right);
}

void mem_show() {
    mem_show_dfs(root);
}

