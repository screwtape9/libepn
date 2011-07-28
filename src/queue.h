#if !defined __EPN_QUEUE_H_
#define __EPN_QUEUE_H_

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QUEUE_INITIALIZER LIST_INITIALIZER

typedef list queue;

#define queue_init      list_init
#define queue_item_cnt  list_item_cnt
#define queue_push      list_add_to_tail
#define queue_pop       list_remove_from_head
#define queue_is_empty  list_is_empty
#define queue_free      list_free

int queue_peek(queue *q, void **pitem);
/*
void queue_init(queue *q);
int queue_item_cnt(queue *q);
int queue_push(queue *q, void *item);
int queue_pop(queue *q);
int queue_is_empty(queue *q);
void queue_free(queue *q);
*/
#ifdef __cplusplus
}
#endif

#endif /* __EPN_QUEUE_H_ */
