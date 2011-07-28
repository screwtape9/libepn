#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/socket.h>
#include "lorem.h"
#include "logit.h"
#include "../../src/list.h"
#include "../../src/queue.h"
#include "../../src/net.h"

#define US_PER_MS 1000
#define MS_PER_SEC 1000

typedef struct _msg {
  unsigned short len;
  char buf[512];
} MSG, *PMSG;

int get_rand(double x, double y);
int get_elapsed_ms(struct timeval *t);

void print_list(list *l)
{
  lnode *ln = l->head;
  while (ln) {
    printf("%d ", (*(int *)(ln->pitem)));
    ln = ln->next;
  }
  printf("\n");
  ln = 0;
}

void test_list(list *l)
{
  int *n = 0;
  int i = 0;
  for (i = 1; i < 11; i++) {
    n = (int *)malloc(sizeof(int));
    (*n) = i;
    list_add_to_tail(l, (void *)n);
  }
  print_list(l);
  list_remove_from_head(l);
  list_remove_from_head(l);
  print_list(l);
  n = (int *)malloc(sizeof(int));
  (*n) = 69;
  list_add_to_tail(l, (void *)n);
  list_remove_from_head(l);
  print_list(l);
}

void test_queue(queue *q)
{
  int *n = 0, *item = 0;
  int i = 0;
  for (i = 1; i < 11; i++) {
    n = (int *)malloc(sizeof(int));
    (*n) = i;
    queue_push(q, n);
  }
  print_list(q);
  queue_peek(q, (void **)&item);
  printf("head of the queue is %d\n", (*item));
  queue_pop(q);
  print_list(q);
  queue_peek(q, (void **)&item);
  printf("head of the queue is %d\n", (*item));
  queue_pop(q);
  print_list(q);
  n = (int *)malloc(sizeof(int));
  (*n) = 69;
  queue_push(q, n);
  print_list(q);
}

int main(int argc, char *argv[])
{
  int i = 0, loop = 0, fd = 0, n = 0, num_recvd_msgs = 0;
  unsigned int avg, totals[4098];
  char *host = 0;
  unsigned short port = 0;
  MSG msg = { 0, { 0 } };
  struct timeval start = { 0, 0 };
  /*list l = LIST_INITIALIZER;
  queue q = QUEUE_INITIALIZER;
  
  //test_list(&l);
  list_free(&l);

  test_queue(&q);
  queue_free(&q);
  return 0;*/

  if (argc != 3) {
    printf("usage:  %s <host> <port>\n", argv[0]);
    return -1;
  }

  srand(time(0));

  host = argv[1];
  port = (unsigned short)atoi(argv[2]);

  gettimeofday(&start, 0);
  memset(totals, 0, sizeof(totals));

  if (!sock_create(&fd, 1)) {
    if (!sock_connect(&fd, host, port, 10)) {
      logit("connected!");
      sock_set_blocking(&fd, 1);
      while (strcmp(msg.buf, "bye")) {
        memset(&msg, 0, sizeof(msg));
        n = (get_rand(1.0, 26.0) - 1);
        strncpy(msg.buf, lorems[n], sizeof(msg.buf));
        msg.len = (unsigned short)(strlen(msg.buf) + 2 + 1);
        if (send(fd, &msg, msg.len, 0) != msg.len) {
          perror("send()");
          break;
        }
        else {
          //logit("SENT:  %s\n", msg.buf);
          n = recv(fd, &msg, msg.len, 0);
          if (n != msg.len) {
            if (n)
              perror("recv()");
            else
              logit("connection was closed!");
            break;
          }
          else {
            num_recvd_msgs++;
            totals[loop]++;
            //logit("REVCD:  %s\n", msg.buf);
            if (get_elapsed_ms(&start) >= 5000) {
              for (i = 0, avg = 0; i < loop; i++)
                avg += totals[i];
              avg /= (loop + 1);
              avg /= 5;
              gettimeofday(&start, 0);
              logit("total recvd msgs:  %d", num_recvd_msgs);
              logit("avg per sec:       %u\n", avg);
              if (++loop == 4098) {
                logit("PEACH OUT BITCHES!");
                break;
              }
            }
          }
        }
      }
    }
    else
      perror("connect()");
    sock_close(&fd);
  }
  else
    perror("socket()");

  return 0;
}

int get_rand(double x, double y)
{
  return (1 + (int)(y * (rand() / (RAND_MAX + 1.0))));
}

int get_elapsed_ms(struct timeval *t)
{
  int delta = 0;
  struct timeval now;
  if (gettimeofday(&now, 0) < 0)
    return -1;
  delta = (now.tv_usec - t->tv_usec) / US_PER_MS;
  delta += (now.tv_sec - t->tv_sec) * MS_PER_SEC;
  return delta;
}
