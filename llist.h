#ifndef field_parent_ptr
#include <stddef.h>
#define field_parent_ptr(ptr, type, field) ((type *)((char *)(ptr) - offsetof(type, field)))
#endif

#ifndef LLIST_INCLUDE_GUARD
#define LLIST_INCLUDE_GUARD

typedef struct _listNode {
  struct _listNode *next;
  struct _listNode *prev;
} listNode;

#endif

void llist_insert_after(listNode *ptr, listNode *item);
void llist_delete_node(listNode *ptr);
