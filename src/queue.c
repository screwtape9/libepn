#include "queue.h"

int queue_peek(queue *q, void **pitem)
{
  if (q->head) {
    (*pitem) = q->head->pitem;
    return 0;
  }
  return -1;
}
