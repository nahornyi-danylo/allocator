#ifndef field_parent_ptr
#include <stddef.h>
#define field_parent_ptr(ptr, type, field) ((type *)((char *)(ptr) - offsetof(type, field)))
#endif

#ifndef RBTREE_INCLUDE_GUARD
#define RBTREE_INCLUDE_GUARD

#define RB_BLACK 0
#define RB_RED 1
#define IS_BLACK(n) (!(n) || (n)->color == RB_BLACK)

typedef struct _rbnode {
    size_t color;
    struct _rbnode *left;
    struct _rbnode *right;
    struct _rbnode *parent;
} rbnode;


#endif

void rbtree_rebalance(rbnode **root, rbnode *node);
void rbtree_link_node(rbnode *node, rbnode *parent, rbnode **link);
rbnode *rbtree_minimum(rbnode *node);
void rbtree_delete(rbnode **root, rbnode *node);
