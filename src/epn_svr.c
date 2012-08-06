#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
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
#include "rb.h"
#include "list.h"
#include "queue.h"
#endif /* USE_GLIB */
#include "epn_svr.h"
#include "epn_err.h"
#include "epn_cfg.h"
#include "net.h"
#include "epn_def.h"

static int run = 0;
static pthread_t tid = 0, parent_tid = 0; 
static int svrfd = 0;
static int svrfd_ready = 0;
static int ep = 0;
static struct epoll_event *events = NULL;
static int nevents = 0;
#if USE_GLIB
static GTree *g_fd_tree = NULL;
static GTree *g_ip_tree = NULL;
static GTree *g_key_tree = NULL;
#else /* USE_GLIB */
static struct rb_table *fd_tree = NULL;
static struct rb_table *ip_tree = NULL;
static struct rb_table *key_tree = NULL;
#endif /* USE_GLIB */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int (*epn_msg_rcvd_cb)(PMSG msg, epn_client_key key) = NULL;
int (*epn_client_closed_cb)(epn_client_key key) = NULL;
int (*epn_client_accepted_cb)(epn_client_key key) = NULL;
extern int (*epn_clt_msg_rcvd_cb)(PMSG msg);

static void sig_handler(int sig)
{
  if (sig != SIGUSR1) {
    pthread_kill(parent_tid, sig);
    /*write(STDOUT_FILENO, "caught unhandled signal!\n", 25);*/
  }
}

int mask_sigs()
{
  sigset_t set, oldset;
  struct sigaction act;

  if (sigfillset(&set) == -1)
    return -1;
  if (pthread_sigmask(SIG_SETMASK, &set, &oldset) == -1)
    return -1;
  memset(&act, 0, sizeof(act));
  if (sigfillset(&act.sa_mask) == -1)
    return -1;
  act.sa_handler = sig_handler;
  act.sa_flags |= SA_RESTART;
  if (sigaction(SIGUSR1, &act, NULL) == -1)
    return -1;
  if (pthread_sigmask(SIG_SETMASK, &oldset, NULL) == -1)
    return -1;
  return 0;
}

#if USE_GLIB
static gint fd_tree_cmp(gconstpointer a, gconstpointer b)
{
  return memcmp(a, b, sizeof(int));
}
#else /* USE_GLIB */
static int fd_tree_cmp(const void *a, const void *b, void *c)
{
  PCLIENT x = (PCLIENT)a;
  PCLIENT y = (PCLIENT)b;
  return memcmp(&x->fd, &y->fd, sizeof(int));
}
#endif /* USE_GLIB */

#if USE_GLIB
static gint ip_tree_cmp(gconstpointer a, gconstpointer b)
{
  return memcmp(a, b, sizeof(in_addr_t));
}
#else /* USE_GLIB */
static int ip_tree_cmp(const void *a, const void *b, void *c)
{
  PCLIENT x = (PCLIENT)a;
  PCLIENT y = (PCLIENT)b;
  return memcmp(&x->ip, &y->ip, sizeof(in_addr_t));
}
#endif /* USE_GLIB */

#if USE_GLIB
static gint key_tree_cmp(gconstpointer a, gconstpointer b)
{
  return memcmp(a, b, sizeof(struct timeval));
}
#else /* USE_GLIB */
static int key_tree_cmp(const void *a, const void *b, void *c)
{
  PCLIENT x = (PCLIENT)a;
  PCLIENT y = (PCLIENT)b;
  int ret = 0;
  ret = memcmp(&x->key, &y->key, sizeof(struct timeval));
  return ret;
}
#endif /* USE_GLIB */

#if USE_GLIB
int client_fd_is_ready(void *key, void *val, void *data)
#else /* USE_GLIB */
int client_fd_is_ready(void *val, void *data)
#endif /* USE_GLIB */
{
  int ret = 0;
  PCLIENT pc = (PCLIENT)val;
  int *cnt = (int *)data;
#if USE_GLIB
  if (pc->readable || (pc->writable && !g_queue_is_empty(pc->msgq))) {
#else /* USE_GLIB */
  if (pc->readable || (pc->writable && !queue_is_empty(&pc->msgq))) {
#endif /* USE_GLIB */
    (*cnt)++;
    ret = 1;
  }
  cnt = NULL;
  pc = NULL;
  return ret;
}

static int find_ready_client()
{
  int cnt = 0;
#if USE_GLIB
  g_tree_foreach(g_fd_tree, client_fd_is_ready, &cnt);
#else /* USE_GLIB */
  struct rb_traverser trav;
  PCLIENT pc = NULL;
  for (pc = (PCLIENT)rb_t_first(&trav, fd_tree);
       pc && !cnt;
       pc = (PCLIENT)rb_t_next(&trav))
    client_fd_is_ready(pc, &cnt);
#endif /* USE_GLIB */
  return cnt;
}

void free_q_msg(void *data, void *user_data)
{
  PMSG msg = (PMSG)data;
  free(msg);
  msg = NULL;
}

void free_client(PCLIENT *pc)
{
  epn_client_key key = { 0, 0 };

  /*printf("free_client() fd %d\n", (*pc)->fd);*/

#if USE_GLIB
  g_tree_remove(g_fd_tree, &((*pc)->fd));
  if (epn_svr_get_one_client_per_ip())
    g_tree_remove(g_ip_tree, &((*pc)->ip));
  g_tree_remove(g_key_tree, &((*pc)->key));
#else /* USE_GLIB */
  rb_delete(fd_tree, (*pc));
  if (epn_svr_get_one_client_per_ip())
    rb_delete(ip_tree, (*pc));
  rb_delete(key_tree, (*pc));
#endif /* USE_GLIB */

  sock_close(&(*pc)->fd);
#if USE_GLIB
  while (!g_queue_is_empty((*pc)->msgq))
    free(g_queue_pop_head((*pc)->msgq));
  g_queue_free((*pc)->msgq);
#else /* USE_GLIB */
  queue_free(&(*pc)->msgq);
#endif /* USE_GLIB */

  key = (*((epn_client_key *)&(*pc)->key));
  free((*pc));
  (*pc) = NULL;

  if (epn_client_closed_cb)
    (*epn_client_closed_cb)(key);
}

static int accept_client()
{
  int clientfd = 0;
  struct sockaddr_in addr;
  struct epoll_event ev = { 0, { 0 } };
  PCLIENT pc = NULL, dup = NULL;
  
  memset(&addr, 0, sizeof(addr));
  if (sock_accept(&svrfd, &clientfd, (struct sockaddr *)&addr) == -1) {
    if (errno == EAGAIN) {
      svrfd_ready = 0;
    }
    else
      return -1;
  }
  else {
    if (sock_set_blocking(&clientfd, 0) == -1) {
      sock_close(&clientfd);
      return -1;
    }
    else {
      ev.events = (EPOLLIN | EPOLLOUT | EPOLLET);
      ev.data.fd = clientfd;
      if (epoll_ctl(ep, EPOLL_CTL_ADD, clientfd, &ev) == -1) {
        sock_close(&clientfd);
        return -1;
      }
      else {
        pc = calloc(1, (sizeof(CLIENT) + epn_svr_get_client_buf_sz() - 1));
#if USE_GLIB
        pc->msgq = g_queue_new();
#endif /* USE_GLIB */
        pc->ip = addr.sin_addr.s_addr;
        pc->fd = clientfd;
        gettimeofday(&pc->key, 0);
        pc->readable = pc->writable = 1;
#if USE_GLIB
        g_tree_insert(g_fd_tree, &pc->fd, pc);
#else /* USE_GLIB */
        rb_insert(fd_tree, pc);
#endif /* USE_GLIB */
        if (epn_svr_get_one_client_per_ip()) {
#if USE_GLIB
          dup = (PCLIENT)g_tree_lookup(g_ip_tree, &pc->ip);
#else /* USE_GLIB */
          dup = (PCLIENT)rb_find(ip_tree, pc);
#endif /* USE_GLIB */
          if (dup) {
            free_client(&dup);
            dup = NULL;
          }
#if USE_GLIB
          g_tree_insert(g_ip_tree, &pc->ip, pc);
#else /* USE_GLIB */
          rb_insert(ip_tree, pc);
#endif /* USE_GLIB */
        }
        pthread_mutex_lock(&mutex);
#if USE_GLIB
        g_tree_insert(g_key_tree, &pc->key, pc);
#else /* USE_GLIB */
        rb_insert(key_tree, pc);
#endif /* USE_GLIB */
        pthread_mutex_unlock(&mutex);
        printf("accepted fd %d from %s\n", pc->fd, inet_ntoa(addr.sin_addr));
        if (epn_client_accepted_cb)
          (*epn_client_accepted_cb)((*((epn_client_key *)&pc->key)));
      }
    }
  }
  return 0;
}

void free_tree_client_item(void *item, void *param)
{
  epn_client_key key = { 0, 0 };
  PCLIENT pc = (PCLIENT)item;
  /*printf("free_tree_client_item() fd %d\n", pc->fd);*/
  sock_close(&pc->fd);
#if USE_GLIB
  while (!g_queue_is_empty(pc->msgq))
    free(g_queue_pop_head(pc->msgq));
  g_queue_free(pc->msgq);
#else /* USE_GLIB */
  queue_free(&pc->msgq);
#endif /* USE_GLIB */
  key = (*((epn_client_key *)&pc->key));
  free(pc);
  pc = NULL;
}

#if USE_GLIB
int trav_fd_svc_ready(PCLIENT pc, GSList **l, int for_clt)
#else /* USE_GLIB */
int trav_fd_svc_ready(PCLIENT pc, list *l, int for_clt)
#endif /* USE_GLIB */
{
  int ret = 0;
  int n = 0;
  int ok = 1;
  PQMSG qmsg = NULL;
  struct timeval now = { 0, 0 };
  int bufsz = epn_svr_get_client_buf_sz();
  int bytes_to_recv = (bufsz - pc->ri);
  int exp = 0;
  int *pfd = NULL;

  if (pc->readable) {
    n = recv(pc->fd, &pc->buf[pc->ri], bytes_to_recv, 0);
    switch (n) {
    case -1:
      if (errno != EAGAIN) {
        perror("recv()");
#if USE_GLIB
        (*l) = g_slist_append((*l), GINT_TO_POINTER(pc->fd));
#else /* USE_GLIB */
        pfd = (int *)malloc(sizeof(int));
        (*pfd) = pc->fd;
        list_add_to_tail(l, pfd);
#endif /* USE_GLIB */
        pfd = NULL;
        ok = 0;
      }
      else
        pc->readable = 0;
      break;
    case 0:
      /*printf("fd %d has been closed\n", pc->fd);*/
#if USE_GLIB
      (*l) = g_slist_append((*l), GINT_TO_POINTER(pc->fd));
#else /* USE_GLIB */
      pfd = (int *)malloc(sizeof(int));
      (*pfd) = pc->fd;
      list_add_to_tail(l, pfd);
#endif /* USE_GLIB */
      pfd = NULL;
      ok = 0;
      break;
    default:
      if (n < bytes_to_recv)
        pc->readable = 0;
      pc->ri += n;
      if (pc->ri > 1) {
        exp = (int)(*((unsigned short *)pc->buf));
        if (exp > bufsz) {
          printf("closing fd %d, expected msg too big\n", pc->fd);
#if USE_GLIB
          (*l) = g_slist_append((*l), GINT_TO_POINTER(pc->fd));
#else /* USE_GLIB */
          pfd = (int *)malloc(sizeof(int));
          (*pfd) = pc->fd;
          list_add_to_tail(l, pfd);
#endif /* USE_GLIB */
          pfd = NULL;
          ok = 0;
        }
        else {
          while ((pc->ri > 2) && (pc->ri >= exp)) {
            if (for_clt && epn_clt_msg_rcvd_cb)
              (*epn_clt_msg_rcvd_cb)((PMSG)pc->buf);
            else if (!for_clt && epn_msg_rcvd_cb)
              (*epn_msg_rcvd_cb)((PMSG)pc->buf, (*((epn_client_key *)&pc->key)));
            memmove(pc->buf, &pc->buf[exp], (pc->ri - exp));
            pc->ri -= exp;
            exp = (int)(*((unsigned short *)pc->buf));
          }
        }
      }
      break;
    }
  }
  
#if USE_GLIB
  if (ok && pc->writable && !g_queue_is_empty(pc->msgq)) {
    qmsg = g_queue_peek_head(pc->msgq);
#else /* USE_GLIB */
  if (ok && pc->writable && !queue_is_empty(&pc->msgq)) {
    queue_peek(&pc->msgq, (void **)&qmsg);
#endif /* USE_GLIB */
    gettimeofday(&now, 0);
    /* TODO: what if timer is set, time has expired, but we've sent
     * partial msg? */
    if (timerisset(&qmsg->ttl) && timercmp(&qmsg->ttl, &now, <)) {
      printf("dropping expired msg...   sent=%d\n", pc->sent);
      pc->sent = 0;
#if USE_GLIB
      free(qmsg);
      g_queue_pop_head(pc->msgq);
#else /* USE_GLIB */
      queue_pop(&pc->msgq);
#endif /* USE_GLIB */
      /*if (queue_is_empty(&pc->msgq))
        pc->writable = 0;*/
    }
    else {
      n = send(pc->fd, &((char *)&qmsg->msg)[pc->sent],
               ((int)qmsg->msg.len - pc->sent), 0);
      switch (n) {
      case -1:
        if (errno == EAGAIN)
          pc->writable = 0;
        else {
          perror("send()");
#if USE_GLIB
          (*l) = g_slist_append((*l), GINT_TO_POINTER(pc->fd));
#else /* USE_GLIB */
          pfd = (int *)malloc(sizeof(int));
          (*pfd) = pc->fd;
          list_add_to_tail(l, pfd);
#endif /* USE_GLIB */
          pfd = NULL;
        }
        break;
      case 0:
        printf("bailing out.  handle this...\n");
        assert(0);
        break;
      default:
        pc->sent += n;
        if (pc->sent == (int)qmsg->msg.len) {
          pc->sent = 0;
#if USE_GLIB
          free(qmsg);
          g_queue_pop_head(pc->msgq);
#else /* USE_GLIB */
          queue_pop(&pc->msgq);
#endif /* USE_GLIB */
        }
        break;
      }
    }
  }

  pc = NULL;

  return ret;
}

#if USE_GLIB
static int trav_svr_fd_svc_ready(void *key, void *val, void *l)
{
  return trav_fd_svc_ready((PCLIENT)val, (GSList **)l, 0);
}
#else /* USE_GLIB */
static int trav_svr_fd_svc_ready(PCLIENT pc, list *l)
{
  return trav_fd_svc_ready(pc, l, 0);
}
#endif /* USE_GLIB */

void rm_func(void *data, void *user_data)
{
  PCLIENT pc = NULL;
#if USE_GLIB
  int fd = GPOINTER_TO_INT(data);/*(*((int *)data));*/
  pc = (PCLIENT)g_tree_lookup(g_fd_tree, &fd);
#else /* USE_GLIB */
  CLIENT c;
  c.fd = (*((int *)data));
  pc = (PCLIENT)rb_find(fd_tree, &c);
#endif /* USE_GLIB */
  if (pc) {
    free_client(&pc);
    pc = NULL;
  }
  else {
    printf("the sky is falling!\n");
  }
}

static void *thread_entry_point(void *arg)
{
  int someone_is_ready = 0;
  int nfds = 0;
  int i = 0;
  PCLIENT pc = NULL;
  CLIENT c;
  sigset_t sigmask, origmask;
#if USE_GLIB
  GSList *l = NULL;
#else /* USE_GLIB */
  list l = LIST_INITIALIZER;
  struct rb_traverser trav;
  lnode *ln = NULL;
#endif /* USE_GLIB */

  if (mask_sigs())
    return (void *)-1;

  while (run) {
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGUSR1);
    pthread_sigmask(SIG_SETMASK, &sigmask, &origmask);

    pthread_mutex_lock(&mutex);
    if (!(someone_is_ready = svrfd_ready))
      someone_is_ready = find_ready_client();
    pthread_mutex_unlock(&mutex);

    nfds = epoll_pwait(ep, events, nevents, (someone_is_ready ? 0 : -1),
                       &origmask);
    pthread_sigmask(SIG_SETMASK, &origmask, NULL);
    for (i = 0; (run && (i < nfds)); i++) {
      if (events[i].data.fd == svrfd)
        svrfd_ready = 1;
      else {
        /*printf("*** About to look for fd %d through these guys:\n",
               events[i].data.fd);
        for (pc = (PCLIENT)rb_t_first(&trav, fd_tree);
             pc;
             pc = (PCLIENT)rb_t_next(&trav))
          printf("*** CLIENT fd %d\tIP %s\n", pc->fd, inet_ntoa(*(struct in_addr *)&pc->ip));*/
        c.fd = events[i].data.fd;
#if USE_GLIB
        pc = (PCLIENT)g_tree_lookup(g_fd_tree, &events[i].data.fd);
#else /* USE_GLIB */
        pc = (PCLIENT)rb_find(fd_tree, &c);
#endif /* USE_GLIB */
        if (pc) {
          pc->readable = (events[i].events & EPOLLIN);
          pc->writable = (events[i].events & EPOLLOUT);
          pc = NULL;
        }
        else /* what the hell!? */
          printf("Could not find fd %d client!\n", events[i].data.fd);
      }
    }

    if (run) {
      if (svrfd_ready)
        accept_client();
#if USE_GLIB
      l = NULL;
#else /* USE_GLIB */
      list_init(&l);
#endif /* USE_GLIB */
      pthread_mutex_lock(&mutex);

#if USE_GLIB
      g_tree_foreach(g_fd_tree, trav_svr_fd_svc_ready, &l);
      g_slist_foreach(l, rm_func, 0);
#else /* USE_GLIB */
      for (pc = (PCLIENT)rb_t_first(&trav, fd_tree);
           pc;
           pc = (PCLIENT)rb_t_next(&trav))
        trav_svr_fd_svc_ready(pc, &l);
      ln = l.head;
      while (ln) {
        rm_func(ln->pitem, 0);
        ln = ln->next;
      }
#endif /* USE_GLIB */
      pthread_mutex_unlock(&mutex);
#if USE_GLIB
      g_slist_free(l);
#else /* USE_GLIB */
      list_free(&l);
#endif /* USE_GLIB */
    }
  }

  return 0;
}

int epn_svr_start()
{
  char addr[32];
  unsigned short port = 0;
  struct epoll_event ev = { 0, { 0 } };

  epn_svr_get_bind_addr(addr, sizeof(addr));
  port = epn_svr_get_bind_port();

#if USE_GLIB
  g_fd_tree = g_tree_new(fd_tree_cmp);
  if (epn_svr_get_one_client_per_ip())
    g_ip_tree = g_tree_new(ip_tree_cmp);
  g_key_tree = g_tree_new(key_tree_cmp);
#else /* USE_GLIB */
  fd_tree = rb_create(fd_tree_cmp, 0, 0);
  if (epn_svr_get_one_client_per_ip())
    ip_tree = rb_create(ip_tree_cmp, 0, 0);
  key_tree = rb_create(key_tree_cmp, 0, 0);
#endif /* USE_GLIB */
  
  if (sock_create(&svrfd, 1)) {
    return EPN_NOSOCK;
  }
  if (sock_reuseaddr(&svrfd)) {
    sock_close(&svrfd);
    return EPN_NORUADDR;
  }
  if (sock_bind(&svrfd, port, addr)) {
    sock_close(&svrfd);
    return EPN_NOBIND;
  }
  if (sock_set_blocking(&svrfd, 0)) {
    sock_close(&svrfd);
    return EPN_NOSETBLK;
  }
  if (sock_listen(&svrfd, 32)) {
    sock_close(&svrfd);
    return EPN_NOLISTEN;
  }
  svrfd_ready = 1;

  nevents = epn_svr_get_est_events();
  ep = epoll_create(nevents);
  if (ep == -1) {
    sock_close(&svrfd);
    return EPN_NOEPOLL;
  }
  ev.events = (EPOLLIN | EPOLLET);
  ev.data.fd = svrfd;
  if (epoll_ctl(ep, EPOLL_CTL_ADD, svrfd, &ev) == -1) {
    sock_close(&svrfd);
    close(ep);
    ep = 0;
    return EPN_NOEPADD;
  }
  events = (struct epoll_event *)malloc((sizeof(struct epoll_event) * nevents));

  run = 1;
  parent_tid = pthread_self();
  if (pthread_create(&tid, 0, thread_entry_point, 0)) {
    run = 0;
    sock_close(&svrfd);
    close(ep);
    ep = 0;
    return EPN_NOTHREAD;
  }

  return 0;
}

int epn_svr_stop()
{
  int n = 0, ret = 0;
  run = 0;
  pthread_kill(tid, SIGUSR1);
  printf("joining on thread 0x%X...\n", (unsigned)tid);
  n = pthread_join(tid, (void **)&ret);
  if (!n) {
    printf("thread 0x%X returned %d\n", (unsigned)tid, ret);
    tid = 0;
    sock_close(&svrfd);
    close(ep);
    ep = 0;
    free(events);
    events = NULL;
#if USE_GLIB
    g_tree_destroy(g_fd_tree);
    g_fd_tree = NULL;
    if (epn_svr_get_one_client_per_ip()) {
      g_tree_destroy(g_ip_tree);
      g_ip_tree = NULL;
    }
    g_tree_destroy(g_key_tree);
    g_key_tree = NULL;
#else /* USE_GLIB */
    rb_destroy(fd_tree, 0);
    fd_tree = NULL;
    if (epn_svr_get_one_client_per_ip()) {
      rb_destroy(ip_tree, 0);
      ip_tree = NULL;
    }
    rb_destroy(key_tree, free_tree_client_item);
    key_tree = NULL;
#endif /* USE_GLIB */
  }
  return (n ? EPN_NOJOIN : 0);
}

void epn_svr_set_msg_rcvd_cb(int (*callback)(PMSG msg, epn_client_key key))
{
  epn_msg_rcvd_cb = callback;
}

void epn_svr_set_client_closed_cb(int (*callback)(epn_client_key key))
{
  epn_client_closed_cb = callback;
}

void epn_svr_set_client_accepted_cb(int (*callback)(epn_client_key key))
{
  epn_client_accepted_cb = callback;
}

int epn_svr_send_to_client(epn_client_key key, const PMSG msg,
                       const unsigned int ttl)
{
  PQMSG qmsg = NULL;
  PCLIENT pc = NULL;
  CLIENT c;
  struct timeval now = { 0, 0 }, diff = { 0, 0 };
  epn_client_key k = key;

  memcpy(&c.key, &k, sizeof(c.key));
  pthread_mutex_lock(&mutex);
#if USE_GLIB
  pc = (PCLIENT)g_tree_lookup(g_key_tree, &k);
#else /* USE_GLIB */
  pc = (PCLIENT)rb_find(key_tree, &c);
#endif /* USE_GLIB */
  if (pc) {
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
    g_queue_push_tail(pc->msgq, qmsg);
#else /* USE_GLIB */
    queue_push(&pc->msgq, qmsg);
#endif /* USE_GLIB */
    pthread_mutex_unlock(&mutex);
    pthread_kill(tid, SIGUSR1);
    /*printf("queued to send:  %s\n", msg->buf);*/
  }
  else {
    pthread_mutex_unlock(&mutex);
    return -1;
  }

  pc = NULL;
  return 0;
}
