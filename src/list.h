#if !defined __EPN_LIST_H_
#define __EPN_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/*! \file epn_list.h
 *  \brief a simple list.
 */

#define LIST_INITIALIZER { 0, 0 }

typedef struct _lnode {
  void *pitem;
  struct _lnode *prev;
  struct _lnode *next;
} lnode;

typedef struct _list {
  lnode *head;
  lnode *tail;
} list;

void list_init(list *l);
int list_item_cnt(list *l);
int list_add_to_tail(list *l, void *item);
int list_remove_from_head(list *l);
int list_is_empty(list *l);
void list_free(list *l);

#ifdef __cplusplus
}
#endif

#endif /* __EPN_LIST_H_ */
