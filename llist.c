#include "llist.h"


void llist_insert_after(listNode *ptr, listNode *item){
    item->next = ptr->next;
    item->prev = ptr;
    if (ptr->next) ptr->next->prev = item;
    ptr->next = item;
}

void llist_delete_node(listNode *ptr){
  if (ptr->prev) ptr->prev->next = ptr->next;
  if (ptr->next) ptr->next->prev = ptr->prev;
}
 

#ifdef LLIST_TEST
#include <stdio.h>

struct item {
    int i;
    listNode node;
};

int main(){
  listNode *head;

  struct item one = {.i = 1};
  struct item two = {.i = 2};
  struct item three = {.i = 3};

  head = &one.node;
  
  llist_insert_after(head, &two.node);
  llist_insert_after(head, &three.node);


  for(listNode *t = head; t != NULL; t = t->next){
    struct item *tmp = field_parent_ptr(t, struct item, node);
    printf("%d\n", tmp->i);
  }

  llist_delete_node(&two.node);

  for(listNode *t = head->next; t != NULL; t = t->next){
    struct item *tmp = field_parent_ptr(t, struct item, node);
    printf("%d\n", tmp->i);
  }
}
#endif
