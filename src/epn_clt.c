#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <arpa/inet.h>
#if USE_GLIB
#include <glib.h>
#else /* USE_GLIB */
#include "list.h"
#include "queue.h"
#endif /* USE_GLIB */
#include "epn_clt.h"
#include "epn_err.h"
#include "epn_cfg.h"
#include "net.h"
#include "epn_def.h"

static int clt_run = 0;
static pthread_t clt_tid = 0;
static int clt_ep = 0;
static struct epoll_event clt_event = { 0, { 0 } };
static PCLIENT client = NULL;
static pthread_mutex_t clt_mutex = PTHREAD_MUTEX_INITIALIZER;

int (*epn_clt_connected_cb)() = NULL;
int (*epn_clt_msg_rcvd_cb)(PMSG msg) = NULL;
int (*epn_clt_closed_cb)() = NULL;

#if USE_GLIB
extern int trav_fd_svc_ready(PCLIENT pc, GSList **l, int for_clt);
extern int client_fd_is_ready(void *key, void *val, void *data);
#else /* USE_GLIB */
extern int trav_fd_svc_ready(PCLIENT pc, list *l, int for_clt);
extern int client_fd_is_ready(void *val, void *data);
#endif /* USE_GLIB */
extern int mask_sigs();
extern void set_opt(PCLIENT p, unsigned int opt, int val);

#if USE_GLIB
static int trav_clt_fd_svc_ready(PCLIENT pc, GSList **l)
#else /* USE_GLIB */
static int trav_clt_fd_svc_ready(PCLIENT pc, list *l)
#endif /* USE_GLIB */
{
  return trav_fd_svc_ready(pc, l, 1);
}

static int clt_connect_to_host()
{
  char addr[32] = { 0 };
  unsigned short port = 0;
  struct epoll_event ev = { 0, { 0 } };

  epn_clt_get_host_ip_addr(addr, sizeof(addr));
  port = epn_clt_get_host_ip_port();

#if USE_GLIB
  memset(client, 0, offsetof(CLIENT, msgq));
  memset(&client->status, 0, ((sizeof(CLIENT) + epn_clt_get_client_buf_sz() - 1) -
                               offsetof(CLIENT, status)));
#else /* USE_GLIB */
  memset(client, 0, (sizeof(CLIENT) + epn_clt_get_client_buf_sz() - 1));
#endif /* USE_GLIB */
  while (clt_run) {
    if (!sock_create(&client->fd, 1)) {
      if (!sock_connect(&client->fd, addr, port, 1)) {
        memset(&ev, 0, sizeof(ev));
        ev.events = (EPOLLIN | EPOLLOUT | EPOLLET);
        ev.data.fd = client->fd;
        if (!epoll_ctl(clt_ep, EPOLL_CTL_ADD, client->fd, &ev)) {
          EPN_SET_READABLE(client);
          EPN_SET_WRITABLE(client);
          if (epn_clt_connected_cb)
            (*epn_clt_connected_cb)();
          return 0;
        }
      }
      sock_close(&client->fd);
    }
    sleep(1);
  }

  return -1;
}

static void *clt_thread_entry_point(void *arg)
{
  sigset_t sigmask, origmask;
  int notused = 0;
  int client_is_ready = 0;
  int nfds = 0;
  int reconnect = 1;
#if USE_GLIB
  GSList *l = NULL;
#else /* USE_GLIB */
  list l = LIST_INITIALIZER;
#endif /* USE_GLIB */

  if (mask_sigs())
    return (void *)-1;


  while (clt_run) {
    if (reconnect) {
      clt_connect_to_host();
      reconnect = 0;
    }

    if (clt_run) {
      sigemptyset(&sigmask);
      sigaddset(&sigmask, SIGUSR1);
      pthread_sigmask(SIG_SETMASK, &sigmask, &origmask);

      pthread_mutex_lock(&clt_mutex);
#if USE_GLIB
      client_is_ready = client_fd_is_ready(NULL, client, &notused);
#else /* USE_GLIB */
      client_is_ready = client_fd_is_ready(client, &notused);
#endif /* USE_GLIB */
      pthread_mutex_unlock(&clt_mutex);

      nfds = epoll_pwait(clt_ep, &clt_event, 1, (client_is_ready ? 0 : -1),
                         &origmask);
      pthread_sigmask(SIG_SETMASK, &origmask, NULL);
      if (clt_run && (nfds == 1)) {
        if (client->fd == clt_event.data.fd) {
          set_opt(client, fd_readable, (clt_event.events & EPOLLIN));
          set_opt(client, fd_writable, (clt_event.events & EPOLLOUT));
        }
        else
          printf("Houston, we have a problem.\n");
      }
      if (clt_run) {
#if !USE_GLIB
        list_init(&l);
#endif /* USE_GLIB */
        pthread_mutex_lock(&clt_mutex);
        trav_clt_fd_svc_ready(client, &l);
#if USE_GLIB
        if (l) {
#else /* USE_GLIB */
        if (l.head) {
#endif /* USE_GLIB */
          /* cleanup connection */
          sock_close(&client->fd);
#if USE_GLIB
          while (!g_queue_is_empty(client->msgq))
            free(g_queue_pop_head(client->msgq));
#else /* USE_GLIB */
          queue_free(&client->msgq);
#endif /* USE_GLIB */
          if (epn_clt_closed_cb)
            (*epn_clt_closed_cb)();
          reconnect = 1;
          pthread_mutex_unlock(&clt_mutex);
#if USE_GLIB
          free(l->data);
          g_slist_free(l);
#else /* USE_GLIB */
          list_free(&l);
#endif /* USE_GLIB */
        }
        else
          pthread_mutex_unlock(&clt_mutex);
      }
    }
  }

  return 0;
}

int epn_clt_start()
{
  clt_ep = epoll_create(1);
  if (clt_ep == -1)
    return EPN_NOEPOLL;
  
  client = calloc(1, (sizeof(CLIENT) + epn_clt_get_client_buf_sz() - 1));
#if USE_GLIB
  client->msgq = g_queue_new();
#endif /* USE_GLIB */

  clt_run = 1;
  if (pthread_create(&clt_tid, 0, clt_thread_entry_point, 0)) {
    clt_run = 0;
    close(clt_ep);
    clt_ep = 0;
#if USE_GLIB
    g_queue_free(client->msgq);
#endif /* USE_GLIB */
    free(client);
    client = NULL;
    return EPN_NOTHREAD;
  }

  return 0;
}

int epn_clt_stop()
{
  int n = 0, ret = 0;
  clt_run = 0;
  pthread_kill(clt_tid, SIGUSR1);
  printf("joining on thread 0x%X...\n", (unsigned)clt_tid);
  n = pthread_join(clt_tid, (void **)&ret);
  if (!n) {
    printf("thread 0x%X returned %d\n", (unsigned)clt_tid, ret);
    clt_tid = 0;
    sock_close(&client->fd);
    close(clt_ep);
    clt_ep = 0;
  }
  return (n ? EPN_NOJOIN : 0);
}

void epn_clt_set_connected_cb(int (*callback)())
{
  epn_clt_connected_cb = callback;
}

void epn_clt_set_msg_rcvd_cb(int (*callback)(PMSG msg))
{
  epn_clt_msg_rcvd_cb = callback;
}

void epn_clt_set_closed_cb(int (*callback)())
{
  epn_clt_closed_cb = callback;
}

int epn_clt_send(const PMSG msg, const unsigned int ttl)
{
  PQMSG qmsg = NULL;
  struct timeval now = { 0, 0 }, diff = { 0, 0 };

  pthread_mutex_lock(&clt_mutex);
  qmsg = (PQMSG)malloc(msg->len + sizeof(struct timeval));
  gettimeofday(&now, 0);
  if (ttl > 1000) {
    diff.tv_sec = (ttl / 1000);
    diff.tv_usec = ((ttl % 1000) * 1000);
  }
  else
    diff.tv_usec = (ttl * 1000);
  timeradd(&now, &diff, &qmsg->ttl);
  memcpy(&qmsg->msg, msg, msg->len);
#if USE_GLIB
  g_queue_push_tail(client->msgq, qmsg);
#else /* USE_GLIB */
  queue_push(&client->msgq, qmsg);
#endif /* USE_GLIB */
  pthread_mutex_unlock(&clt_mutex);
  pthread_kill(clt_tid, SIGUSR1);
  /*printf("queued to send:  %s\n", msg->buf);*/

  return 0;
}
