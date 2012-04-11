#include <stdlib.h>
#include "list.h"

void list_init(list *l)
{
  l->head = l->tail = NULL;
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
  node = NULL;
  return 0;
}

int list_remove_from_head(list *l)
{
  lnode *tmp = NULL;
  if (!l->head)
    return 1;
  if (l->head == l->tail) {
    free(l->head->pitem);
    free(l->head);
    l->head = l->tail = NULL;
  }
  else {
    tmp = l->head;
    l->head = l->head->next;
    l->head->prev = NULL;
    free(tmp->pitem);
    free(tmp);
    tmp = NULL;
  }
  return 0;
}

int list_is_empty(list *l)
{
  return !l->head;
}

void list_free(list *l)
{
  lnode *curr = l->head, *tmp = NULL;
  while (curr) {
    tmp = curr;
    curr = curr->next;
    free(tmp->pitem);
    free(tmp);
  }
  l->head = l->tail = curr = tmp = NULL;
}
