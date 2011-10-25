#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "epn_cfg.h"
#include "epn_svr.h"
#include "../../src/queue.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
queue msgs = QUEUE_INITIALIZER;
int run = 0;

typedef struct _akey {
  epn_client_key ckey;
  MSG pmsg[1];
} AKEY, *PAKEY;

int msg_rcvd_cb(PMSG msg, epn_client_key key)
{
  PAKEY pkey = (PAKEY)malloc(sizeof(AKEY) + msg->len);
  pkey->ckey = key;
  memcpy(pkey->pmsg, msg, msg->len);
  pthread_mutex_lock(&mutex);
  queue_push(&msgs, (void *)pkey);
  /*printf("msg_rcvd_cb() queue'd up msg\n");*/
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  pkey = NULL;
  return 0;
}

int client_closed_cb(epn_client_key key)
{
  printf("client key %li has been closed\n", key.tv_usec);
  return 0;
}

int client_accepted_cb(epn_client_key key)
{
  printf("accepted new client key %li\n", key.tv_usec);
  return 0;
}

void *thread(void *arg)
{
  int n = 0;
  PAKEY pkey = NULL;
  PMSG resp = NULL;

  while (run) {
    n = 0;
    pthread_mutex_lock(&mutex);
    while (run && queue_is_empty(&msgs) && !n)
      n = pthread_cond_wait(&cond, &mutex);
    if (run && !queue_is_empty(&msgs) && !n)
      queue_peek(&msgs, (void **)&pkey);
    pthread_mutex_unlock(&mutex);
    if (pkey) {
      /*printf("RECVD:  %s\n", pkey->pmsg->buf);*/
      resp = malloc(pkey->pmsg->len);
      memcpy(resp, pkey->pmsg, pkey->pmsg->len);
      if (epn_svr_send_to_client(pkey->ckey, resp, 3000))
        printf("could not send back!\n");
      /*else
        printf("sending msg back to sender...\n");*/
      free(resp);
      resp = NULL;
      pthread_mutex_lock(&mutex);
      queue_pop(&msgs);
      pthread_mutex_unlock(&mutex);
      pkey = NULL;
    }
  }

  return 0;
}

int main(int argc, char *argv[])
{
  char buf[256];
  char addr[32];
  unsigned short port = 0;
  pthread_t tid = 0;
  int ret = 0;

  run = 1;
  pthread_create(&tid, 0, thread, 0);

  epn_svr_init("0.0.0.0", 5050, 32, 4096, 0, 1);
  epn_svr_set_msg_rcvd_cb(&msg_rcvd_cb);
  epn_svr_set_client_closed_cb(&client_closed_cb);
  epn_svr_set_client_accepted_cb(&client_accepted_cb);
  epn_svr_start();
  epn_svr_get_bind_addr(addr, sizeof(addr));
  port = epn_svr_get_bind_port();
  printf("Listening on %s:%d\n", addr, port);
  do {
    scanf("%s", buf);
  } while (strcmp(buf, "bye"));
  epn_svr_stop();
  epn_cleanup();

  run = 0;
  pthread_mutex_lock(&mutex);
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  pthread_join(tid, (void **)&ret);
  tid = 0;
  queue_free(&msgs);

  return 0;
}
