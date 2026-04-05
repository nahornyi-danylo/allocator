#include "rbtree.h"
#include "llist.h"
#include "block.h"

#ifndef TREENODE_INCLUDE_GUARD
#define TREENODE_INCLUDE_GUARD
typedef struct {
    rbnode tnode;
    listNode lnode;
} treeNode;

#define ALIGNED_TREENODE_SIZE TO_ALIGNED(sizeof(treeNode))

static inline block *rbnode_to_block(rbnode *p) {
    treeNode *t = field_parent_ptr(p, treeNode, tnode);
    block *b = payload_to_block(t);
    return b;
}

static inline rbnode *block_to_rbnode(block *b) {
    treeNode *t = block_to_payload(b);
    return &t->tnode;
}

static inline block *lnode_to_block(listNode *p) {
    treeNode *t = field_parent_ptr(p, treeNode, lnode);
    block *b = payload_to_block(t);
    return b;
}

static inline listNode *block_to_lnode(block *b) {
    treeNode *t = block_to_payload(b);
    return &t->lnode;
}

#endif

void tree_insert(rbnode **root, block *b);
void tree_delete(rbnode **root, block *b);

