#include "rbtree.h"

static void left_rotate(rbnode **root, rbnode *x) {
    rbnode *y = x->right;
    x->right = y->left;

    if (y->left) {
        y->left->parent = x;
    }

    y->parent = x->parent;
    
    if (!x->parent) {
        *root = y;
    }
    else if (x == x->parent->left) {
        x->parent->left = y;
    }
    else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

static void right_rotate(rbnode **root, rbnode *y) {
    rbnode *x = y->left;
    y->left = x->right;

    if (x->right) {
        x->right->parent = y;
    }

    x->parent = y->parent;

    if (!y->parent) {
        *root = x;
    }
    else if (y == y->parent->left) {
        y->parent->left = x;
    }
    else {
        y->parent->right = x;
    }

    x->right = y;
    y->parent = x;
}

void rbtree_rebalance(rbnode **root, rbnode *node) {
    while (node->parent && node->parent->color == RB_RED) {
        rbnode *grandparent = node->parent->parent;

        if (node->parent == grandparent->left) {
            rbnode *y = grandparent->right;
            if (y && y->color == RB_RED) {
                node->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                grandparent->color = RB_RED;
                node = grandparent;
            }
            else {
                if (node == node->parent->right) {
                    node = node->parent;
                    left_rotate(root, node);
                }
                node->parent->color = RB_BLACK;
                grandparent->color = RB_RED;
                right_rotate(root, grandparent);
            }
        }
        else {
            rbnode *y = grandparent->left;
            if (y && y->color == RB_RED) {
                node->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                grandparent->color = RB_RED;
                node = grandparent;
            }
            else {
                if (node == node->parent->left) {
                    node = node->parent;
                    right_rotate(root, node);
                }
                node->parent->color = RB_BLACK;
                grandparent->color = RB_RED;
                left_rotate(root, grandparent);
            }
        }
    }
    (*root)->color = RB_BLACK;
}

// link argument holds the adress of either left or right field of parent
void rbtree_link_node(rbnode *node, rbnode *parent, rbnode **link) {
    node->color = RB_RED;
    node->parent = parent;
    node->left = NULL;
    node->right = NULL;
    *link = node;
}

static void rbtree_transplant(rbnode **root, rbnode *u, rbnode *v) {
    if (!u->parent) {
        *root = v;
    }
    else if (u == u->parent->left) {
        u->parent->left = v;
    }
    else {
        u->parent->right = v;
    }

    if (v) v->parent = u->parent;
}

rbnode *rbtree_minimum(rbnode *node) {
    while (node->left) node = node->left;
    return node;
}

static void rbtree_deletion_rebalance(rbnode **root, rbnode *node, rbnode *parent) {
    while (node != *root && IS_BLACK(node)) {
        if (node == parent->left) {
            rbnode *w = parent->right;

            if (!w) {
                node = parent;
                parent = node->parent;
                continue;
            }

            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                parent->color = RB_RED;
                left_rotate(root, parent);
                w = parent->right;
            }

            if (IS_BLACK(w->left) && IS_BLACK(w->right)) {
                w->color = RB_RED;
                node = parent;
                parent = node->parent;
            }
            else { 
                if (IS_BLACK(w->right)) {
                    w->left->color = RB_BLACK;
                    w->color = RB_RED;
                    right_rotate(root, w);
                    w = parent->right;
                }
                w->color = parent->color;
                parent->color = RB_BLACK;
                if (w->right) w->right->color = RB_BLACK;
                left_rotate(root, parent);
                node = *root;
                break;
            }
        }
        else {
            rbnode *w = parent->left;

            if (!w) {
                node = parent;
                parent = node->parent;
                continue;
            }

            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                parent->color = RB_RED;
                right_rotate(root, parent);
                w = parent->left;
            }

            if (IS_BLACK(w->left) && IS_BLACK(w->right)) {
                w->color = RB_RED;
                node = parent;
                parent = node->parent;
            }
            else { 
                if (IS_BLACK(w->left)) {
                    w->right->color = RB_BLACK;
                    w->color = RB_RED;
                    left_rotate(root, w);
                    w = parent->left;
                }
                w->color = parent->color;
                parent->color = RB_BLACK;
                if (w->left) w->left->color = RB_BLACK;
                right_rotate(root, parent);
                node = *root;
                break;
            }
        }
    }
    if (node) node->color = RB_BLACK;
}

void rbtree_delete(rbnode **root, rbnode *node) {
    rbnode *x;
    rbnode *p;
    rbnode *y = node;
    size_t originalColor = y->color;

    if (!node->left) {
        x = node->right;
        p = node->parent;
        rbtree_transplant(root, node, x);
    }
    else if (!node->right) {
        x = node->left;
        p = node->parent;
        rbtree_transplant(root, node, x);
    }
    else {
        y = rbtree_minimum(node->right);
        originalColor = y->color;
        x = y->right;

        if (y->parent == node) {
            p = y;
            if (x) {
                x->parent = y;
            }
        }
        else {
            p = y->parent;
            rbtree_transplant(root, y, x);
            y->right = node->right;
            y->right->parent = y;
        }

        rbtree_transplant(root, node, y);
        y->left = node->left;
        y->left->parent = y;
        y->color = node->color;
    }
    if (originalColor == RB_BLACK) {
        rbtree_deletion_rebalance(root, x, p);
    }
}

#ifdef RBTREE_TEST
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <time.h>

#define N_RANDOMIZED_INSERTS 10000

struct intNode {
    int n;
    rbnode node;
};

rbnode *root = NULL;

void int_alloc_insert(int n) {
    struct intNode *item = malloc(sizeof(struct intNode));
    if (!item) {
        printf("alloc failed in test\n");
        exit(1);
    }
    item->n = n;
    rbnode *parent = NULL;
    rbnode **new = &root;

    while (*new) {
        struct intNode *this = field_parent_ptr(*new, struct intNode, node);

        parent = *new;
        if (n < this->n) {
            new = &((*new)->left);
        }
        else if (n > this->n) {
            new = &((*new)->right);
        }
        else {
            free(item);
            return;
        }
    }

    rbtree_link_node(&item->node, parent, new);
    rbtree_rebalance(&root, &item->node);
}

// delete the node of n if found, else delete it's parent
void find_and_delete(int n) {
    rbnode *node = root;
    struct intNode *this = NULL;

    while (node) {
        this = field_parent_ptr(node, struct intNode, node);

        if (n < this->n) {
            node = node->left;
        }
        else if (n > this->n) {
            node = node->right;
        }
        else {
            break;
        }
    }

    if (!node) {
    // we found where the node of *n* would be, if inserted, and *this* holds the
    // parent which we delete
        if (this) {
            rbtree_delete(&root, &this->node);
            free(this);
        }
    }
    else {
        rbtree_delete(&root, node);
        free(this);
    }
}

void dfsPrint(rbnode *node) {
    if (!node) {
        printf(".");
        return;
    }

    struct intNode *this = field_parent_ptr(node, struct intNode, node);
    printf("%d", this->n);
    if (this->node.color == RB_BLACK) printf("B");
    else printf("R");
    printf("(");
    dfsPrint(node->left);
    printf(",");
    dfsPrint(node->right);
    printf(")");
}

int check_bst_range(rbnode *node, int minv, int maxv) {
    if (!node) return 1;

    struct intNode *cur = field_parent_ptr(node, struct intNode, node);

    if (cur->n <= minv || cur->n >= maxv) {
        return 0;
    }

    return check_bst_range(node->left, minv, cur->n) &&
           check_bst_range(node->right, cur->n, maxv);
}

int check_parent_links(rbnode *node, rbnode *parent) {
    if (!node) return 1;

    if (node->parent != parent) {
        return 0;
    }

    return check_parent_links(node->left, node) &&
           check_parent_links(node->right, node);
}

int check_red_property(rbnode *node) {
    if (!node) return 1;

    if (node->color == RB_RED) {
        if ((node->left && node->left->color == RB_RED) ||
            (node->right && node->right->color == RB_RED)) {
            return 0;
        }
    }

    return check_red_property(node->left) &&
           check_red_property(node->right);
}

// All paths from root to leaf must have the same amount of black nodes
// NULL nodes are black
// returns -1 if invalid
int check_black_height(rbnode *node) {
    if (!node) return 1;

    int left_bh = check_black_height(node->left);
    int right_bh = check_black_height(node->right);

    if (left_bh < 0 || right_bh < 0 || left_bh != right_bh) {
        return -1;
    }

    return left_bh + (node->color == RB_BLACK ? 1 : 0);
}

void assert_rb_invariants(rbnode *root) {
    if (!root) return;

    assert(root->parent == NULL);
    assert(root->color == RB_BLACK);

    assert(check_bst_range(root, INT_MIN, INT_MAX));
    assert(check_parent_links(root, NULL));
    assert(check_red_property(root));
    assert(check_black_height(root) >= 0);
}

int main() {
    srand(time(NULL));

    // insert 10 20 30 15 25 5 1 50 60 55
    // delete 1 25 55 20 10 30 5 15 50 60
    // manually and with printing for visual check by hand
    int_alloc_insert(10);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    int_alloc_insert(20);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    int_alloc_insert(30);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    int_alloc_insert(15);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    int_alloc_insert(25);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    int_alloc_insert(5);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");
    
    int_alloc_insert(1);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    int_alloc_insert(50);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    int_alloc_insert(60);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    int_alloc_insert(55);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    find_and_delete(1);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    find_and_delete(25);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    find_and_delete(55);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    find_and_delete(20);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    find_and_delete(10);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    find_and_delete(30);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    find_and_delete(5);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    find_and_delete(15);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    find_and_delete(50);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    find_and_delete(60);
    assert_rb_invariants(root);
    dfsPrint(root);
    printf("\n");

    for (int i = 0; i < N_RANDOMIZED_INSERTS; i++) {
        int_alloc_insert(rand());
        assert_rb_invariants(root);
    }

    for (int i = 0; i < N_RANDOMIZED_INSERTS; i++) {
        find_and_delete(rand());
        assert_rb_invariants(root);
    }

    assert(root == NULL);

    printf("All tests passed!\n");
}
#endif

