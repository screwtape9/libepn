#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "epn_cfg.h"
#include "epn_svr.h"
#include "printsig.h"
#if USE_GLIB
#include <glib.h>
#else /* USE_GLIB */
#include "../../src/queue.h"
#endif /* USE_GLIB */

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
#if USE_GLIB
GQueue *msgs = NULL;
#else /* USE_GLIB */
queue msgs = QUEUE_INITIALIZER;
#endif /* USE_GLIB */
int run = 0;

typedef struct _akey {
  epn_client_key ckey;
  MSG pmsg[1];
} AKEY, *PAKEY;

void sig_handler(int sig)
{
  print_sig_desc(sig);
  write(STDOUT_FILENO, "\nExiting...\n", 12);
  run = 0;
}

int handle_signals()
{
  sigset_t set;
  struct sigaction act;

  if(sigfillset(&set) == -1)
    return -1;
  if(pthread_sigmask(SIG_SETMASK, &set, NULL) == -1)
    return -1;
  memset(&act, 0, sizeof(act));
  if(sigfillset(&act.sa_mask) == -1)
    return -1;
  act.sa_handler = SIG_IGN;
  if(sigaction(SIGHUP, &act, NULL) == -1)
    return -1;
  if(sigaction(SIGQUIT, &act, NULL) == -1)
    return -1;
  if(sigaction(SIGPIPE, &act, NULL) == -1)
    return -1;
  act.sa_handler = sig_handler;
  if(sigaction(SIGINT, &act, NULL) == -1)
    return -1;
  if(sigaction(SIGTERM, &act, NULL) == -1)
    return -1;
  if(sigaction(SIGBUS, &act, NULL) == -1)
    return -1;
  if(sigaction(SIGFPE, &act, NULL) == -1)
    return -1;
  if(sigaction(SIGILL, &act, NULL) == -1)
    return -1;
  if(sigaction(SIGSEGV, &act, NULL) == -1)
    return -1;
  if(sigaction(SIGSYS, &act, NULL) == -1)
    return -1;
  if(sigaction(SIGXCPU, &act, NULL) == -1)
    return -1;
  if(sigaction(SIGXFSZ, &act, NULL) == -1)
    return -1;
  if(sigemptyset(&set) == -1)
    return -1;
  if(pthread_sigmask(SIG_SETMASK, &set, NULL) == -1)
    return -1;
  return 0;
}

int msg_rcvd_cb(PMSG msg, epn_client_key key)
{
  PAKEY pkey = (PAKEY)malloc(sizeof(AKEY) + msg->len);
  pkey->ckey = key;
  memcpy(pkey->pmsg, msg, msg->len);
  pthread_mutex_lock(&mutex);
#if USE_GLIB
  g_queue_push_tail(msgs, (void *)pkey);
#else /* USE_GLIB */
  queue_push(&msgs, (void *)pkey);
#endif /* USE_GLIB */
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
#if USE_GLIB
    while (run && g_queue_is_empty(msgs) && !n)
      n = pthread_cond_wait(&cond, &mutex);
    if (run && !g_queue_is_empty(msgs) && !n)
      pkey = (PAKEY)g_queue_pop_head(msgs);
#else /* USE_GLIB */
    while (run && queue_is_empty(&msgs) && !n)
      n = pthread_cond_wait(&cond, &mutex);
    if (run && !queue_is_empty(&msgs) && !n)
      queue_peek(&msgs, (void **)&pkey);
#endif /* USE_GLIB */
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
#if USE_GLIB
      free(pkey);
#else /* USE_GLIB */
      queue_pop(&msgs);
#endif /* USE_GLIB */
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

  if (handle_signals()) {
    printf("Failed to set up signal handling.\n");
    return -1;
  }

#if USE_GLIB
  msgs = g_queue_new();
#endif /* USE_GLIB */

  run = 1;
  pthread_create(&tid, 0, thread, 0);

  epn_svr_init("0.0.0.0", 5050, 64, 4096, 0, 1);
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
#if USE_GLIB
  g_queue_free(msgs);
#else /* USE_GLIB */
  queue_free(&msgs);
#endif /* USE_GLIB */

  return 0;
}
