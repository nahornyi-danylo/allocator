#ifdef ALLOCATOR_TEST
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include "allocator.h"
#include "config.h"
#include "llist.h"
#include "tree.h"
#include "debug.h"

typedef struct {
    unsigned char *data;
    size_t size;
    size_t checksum;
    listNode node;
} allocation;

static struct {
    size_t allocationCount;
    size_t nAlloc;
    size_t nFree;
    size_t nRealloc;
} testingData;

allocation *head = NULL;

void random_fill(unsigned char *data, size_t n) {
    for (size_t i = 0; i < n; i++) {
        data[i] = (unsigned char)rand();
    }
}

size_t sum(unsigned char *data, size_t n) {
    size_t counter = 0;

    for (size_t i = 0; i < n; i++) {
        counter += data[i];
    }

    return counter;
}

bool is_aligned(uintptr_t p) {
    if (!(p & (ALIGN - 1))) return true;
    return false;
}

void process_allocation(void *p, size_t size) {
    allocation *a = malloc(sizeof(allocation));
    if (!a) {
        printf("Malloc failed!\n");
        exit(1);
    }
    a->size = size;
    a->data = p;
    random_fill(p, size);
    a->checksum = sum(a->data, a->size);

    if (!head) {
        head = a;
        a->node.next = NULL;
        a->node.prev = NULL;
    }
    else {
        llist_insert_after(&head->node, &a->node);
    }

    testingData.allocationCount++;
}

// list is assumed to have been kept correct
void process_realloc(size_t n, size_t size) {
    listNode *node = &head->node;
    for (size_t i = 0; i < n; i++) {
        node = node->next;
    }

    allocation *this = field_parent_ptr(node, allocation, node);
    if (this->size < size) {
        void *tmp = mem_realloc(this->data, size);
        if (!tmp) {
            printf("Mem realloc expand failed\n");
            exit(1);
        }
        if (!is_aligned((uintptr_t)tmp)) {
            printf("Mem realloc expand not aligned\n");
            exit(1);
        }
        if (sum(tmp, this->size) != this->checksum) {
            printf("Checksum mismatch on realloc expand copy\n");
            exit(1);
        }
        this->data = tmp;
        this->size = size;
        random_fill(tmp, size);
        this->checksum = sum(this->data, size);
    }
    else {
        size_t newChecksum = sum(this->data, size);
        debug_print_extra("Realloc shrink of %p, to %lu\n", this->data, size);
        void *tmp = mem_realloc(this->data, size);
        if (!tmp) {
            printf("Mem realloc shrink failed\n");
            exit(1);
        }
        if (!is_aligned((uintptr_t)tmp)) {
            printf("Mem realloc shrink not aligned\n");
            exit(1);
        }
        this->data = tmp;
        if (sum(this->data, size) != newChecksum) {
            printf("Checksum mismatch on realloc shrink\n");
        }

        this->checksum = newChecksum;
        this->size = size;
    }
}

void process_free(size_t n) {
    listNode *node = &head->node;
    for (size_t i = 0; i < n; i++) {
        node = node->next;
    }
    allocation *this = field_parent_ptr(node, allocation, node);
    if (this == head) {
        head = this->node.next ? field_parent_ptr(this->node.next, allocation, node) : NULL;
    }

    testingData.allocationCount--;
    mem_free(this->data);
    llist_delete_node(&this->node);
    free(this);
}

void check_allocations() {
    if (testingData.allocationCount == 0) return;
#ifdef DEBUG_PRINT_EXTRA
    size_t id = 0;
#endif
    for (listNode *node = &head->node; node; node = node->next) {
        debug_print_extra("id = %lu\n", id++);
        allocation *this = field_parent_ptr(node, allocation, node);
        block *b = payload_to_block(this->data);

#ifdef DEBUG_PRINT_EXTRA
        block_print(b);
#endif

        if (block_get_size(b) < this->size) {
            printf("Block size corrupted\n");
            exit(1);
        }

        if (!block_get_in_use_bit(b)) {
            printf("Block in use corrupted\n");
            exit(1);
        }

        if (this->checksum != sum(this->data, this->size)) {
            printf("Checksum mismatch\n");
            exit(1);
        }
    }
}

void print_testing_stats() {
    printf("Random test completed with:\n");
    printf("allocs: %lu\nreallocs: %lu\nfrees: %lu\n", testingData.nAlloc, testingData.nRealloc, testingData.nFree);
}

void stochastic_monte_carlo_testing_suite(size_t nIterations) {
    for (size_t i = 0; i < nIterations; i++) {
        debug_print_extra("Iteration %lu\n", i);
        float r = (float)rand()/(float)RAND_MAX;

        // 85% -> within default size
        // 15% -> big sizes, limit chosen arbitrarily
        float r2 = (float)rand()/(float)RAND_MAX;
        size_t allocSizeLimit = r2 < 0.85 ? MAX_SINGLE_ARENA_ALLOCATION_SIZE : 128 * DEFAULT_ARENA_SIZE;

        // 50% -> alloc
        // 20% -> realloc
        // 30% -> free
        if (r < 0.5) {
            testingData.nAlloc++;
            size_t size = (size_t)rand() % allocSizeLimit;
            debug_print_extra("Allocation of %lu\n", size);
            void *p = mem_alloc(size);
            if (!p) {
                printf("mem_alloc failed!\n");
                exit(1);
            }
            if (!is_aligned((uintptr_t)p)) {
                printf("mem_alloc result not aligned\n");
                exit(1);
            }
            process_allocation(p, size);
        }
        else if (r < 0.7) {
            if (testingData.allocationCount == 0) continue;
            testingData.nRealloc++;
            size_t n = (size_t)rand() % testingData.allocationCount;
            debug_print_extra("Realloc on %lu\n", n);
            size_t size = (size_t)rand() % allocSizeLimit;
            process_realloc(n, size);
        }
        else {
            if (testingData.allocationCount == 0) continue;
            testingData.nFree++;
            size_t n = (size_t)rand() % testingData.allocationCount;
            debug_print_extra("Free on %lu\n", n);
            process_free(n);
        }

        check_allocations();
    }
    print_testing_stats();

    while(testingData.allocationCount) {
        process_free(0);
    }
}

int main() {
    srand((unsigned int)time(NULL));

    printf("stochastic_monte_carlo_testing_suite 10000 iterations\n");
    stochastic_monte_carlo_testing_suite(10000);
}
#endif
