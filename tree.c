#include <stdio.h>
#include "tree.h"
#include "debug.h"

void tree_insert(rbnode **root, block *b) {
    debug_print_extra("tree_insert called on %p (size %lu)\n", b, block_get_size(b));
    rbnode **current = root;
    treeNode *newNode = block_to_payload(b);
    size_t size = block_get_size(b);

    rbnode *parent = NULL;

    while (*current) {
        size_t curSize = block_get_size(rbnode_to_block(*current));

        parent = *current;
        if (size < curSize) {
            current = &((*current)->left);
        }
        else if (size > curSize) {
            current = &((*current)->right);
        }
        else {
            treeNode *curNode = field_parent_ptr(*current, treeNode, tnode);
            llist_insert_after(&curNode->lnode, &newNode->lnode);
            return;
        }
    }
    newNode->lnode.next = NULL;
    newNode->lnode.prev = NULL;

    rbtree_link_node(&newNode->tnode, parent, current);
    rbtree_rebalance(root, &newNode->tnode);
}

void tree_delete(rbnode **root, block *b) {
    debug_print_extra("tree_delete called on %p (size %lu)\n", b, block_get_size(b));
    treeNode *node = block_to_payload(b);

    if (node->lnode.prev) {
        llist_delete_node(&node->lnode);
        return;
    }
    // if another node of the same length exists, we need to relocate the tree
    // node info to be in that next node
    // By design, only the head of the list holds the actual rbnode info
    if (node->lnode.next) {
        treeNode *newLoc = field_parent_ptr(node->lnode.next, treeNode, lnode);

        if (node->tnode.parent) {
            if (node->tnode.parent->left == &node->tnode) {
                node->tnode.parent->left = &newLoc->tnode;
            }
            else {
                node->tnode.parent->right = &newLoc->tnode;
            }
        }
        if (node->tnode.left) {
            node->tnode.left->parent = &newLoc->tnode;
        }
        if (node->tnode.right) {
            node->tnode.right->parent = &newLoc->tnode;
        }
        if (*root == &node->tnode) {
            *root = &newLoc->tnode;
        }

        newLoc->tnode.parent = node->tnode.parent;
        newLoc->tnode.left = node->tnode.left;
        newLoc->tnode.right = node->tnode.right;
        newLoc->tnode.color = node->tnode.color;
        llist_delete_node(&node->lnode);
        return;
    }

    rbtree_delete(root, &node->tnode);
}
