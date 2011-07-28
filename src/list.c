#include <stdlib.h>
#include "list.h"

void list_init(list *l)
{
  l->head = l->tail = 0;
}

int list_item_cnt(list *l)
{
  lnode *curr = l->head;
  int cnt = 0;
  while (curr) {
    cnt++;
    curr = curr->next;
  }
  return cnt;
}

int list_add_to_tail(list *l, void *item)
{
  lnode *node = (lnode *)calloc(1, sizeof(lnode));
  if (!node)
    return -1;
  node->pitem = item;
  if (!l->head)
    l->head = l->tail = node;
  else {
    l->tail->next = node;
    node->prev = l->tail;
    l->tail = node;
  }
  node = 0;
  return 0;
}

int list_remove_from_head(list *l)
{
  lnode *tmp = 0;
  if (!l->head)
    return 1;
  if (l->head == l->tail) {
    free(l->head->pitem);
    free(l->head);
    l->head = l->tail = 0;
  }
  else {
    tmp = l->head;
    l->head = l->head->next;
    l->head->prev = 0;
    free(tmp->pitem);
    free(tmp);
    tmp = 0;
  }
  return 0;
}

int list_is_empty(list *l)
{
  return !l->head;
}

void list_free(list *l)
{
  lnode *curr = l->head, *tmp = 0;
  while (curr) {
    tmp = curr;
    curr = curr->next;
    free(tmp->pitem);
    free(tmp);
  }
  curr = tmp = 0;
}
