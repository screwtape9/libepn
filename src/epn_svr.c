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
#include "epn_svr.h"
#include "epn_err.h"
#include "epn_cfg.h"
#include "net.h"
#include "rb.h"
#include "list.h"
#include "queue.h"

typedef struct _client {
  in_addr_t ip;
  int fd;
  struct timeval key;
  int ri;
  queue msgq;
  int readable;
  int writable;
  int sent;
  char buf[1];
} CLIENT, *PCLIENT;

typedef struct _qmsg {
  struct timeval ttl;
  MSG msg;
} QMSG, *PQMSG;

static int run = 0;
static pthread_t tid = 0; 
static int svrfd = 0;
static int svrfd_ready = 0;
static int ep = 0;
static struct epoll_event *events = NULL;
static int nevents = 0;
static struct rb_table *fd_tree = NULL;
static struct rb_table *ip_tree = NULL;
static struct rb_table *key_tree = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int (*epn_msg_rcvd_cb)(PMSG msg, epn_client_key key) = NULL;
int (*epn_client_closed_cb)(epn_client_key key) = NULL;
int (*epn_client_accepted_cb)(epn_client_key key) = NULL;

static void sig_handler(int sig)
{
  if (sig != SIGUSR1)
    write(STDOUT_FILENO, "caught unknown signal!\n", 23);
}

static int mask_sigs()
{
  sigset_t set;
  struct sigaction act;

  if (sigfillset(&set) == -1)
    return -1;
  if (pthread_sigmask(SIG_SETMASK, &set, NULL) == -1)
    return -1;
  memset(&act, 0, sizeof(act));
  if (sigfillset(&act.sa_mask) == -1)
    return -1;
  act.sa_handler = SIG_IGN;
  if (sigaction(SIGHUP, &act, NULL) == -1)
    return -1;
  if (sigaction(SIGINT, &act, NULL) == -1)
    return -1;
  if (sigaction(SIGQUIT, &act, NULL) == -1)
    return -1;
  if (sigaction(SIGPIPE, &act, NULL) == -1)
    return -1;
  if (sigaction(SIGTERM, &act, NULL) == -1)
    return -1;
  if (sigaction(SIGBUS, &act, NULL) == -1)
    return -1;
  if (sigaction(SIGFPE, &act, NULL) == -1)
    return -1;
  if (sigaction(SIGILL, &act, NULL) == -1)
    return -1;
  if (sigaction(SIGSEGV, &act, NULL) == -1)
    return -1;
  if (sigaction(SIGSYS, &act, NULL) == -1)
    return -1;
  if (sigaction(SIGXCPU, &act, NULL) == -1)
    return -1;
  if (sigaction(SIGXFSZ, &act, NULL) == -1)
    return -1;
  act.sa_handler = sig_handler;
  act.sa_flags |= SA_RESTART;
  if (sigaction(SIGUSR1, &act, NULL) == -1)
    return -1;
  if (sigemptyset(&set) == -1)
    return -1;
  if (pthread_sigmask(SIG_SETMASK, &set, NULL) == -1)
    return -1;
  return 0;
}

static int fd_tree_cmp(const void *a, const void *b, void *c)
{
  PCLIENT x = (PCLIENT)a;
  PCLIENT y = (PCLIENT)b;
  return memcmp(&x->fd, &y->fd, sizeof(int));
  //return ((x->fd == y->fd) ? 0 : ((y->fd < x->fd) ? -1 : 1));
  /*int x = (*((int *)a));
  int y = (*((int *)b));
  return ((x < y) ? -1 : ((x == y) ? 0 : 1));*/
}

static int ip_tree_cmp(const void *a, const void *b, void *c)
{
  PCLIENT x = (PCLIENT)a;
  PCLIENT y = (PCLIENT)b;
  return memcmp(&x->ip, &y->ip, sizeof(in_addr_t));
  //return ((x->ip == y->ip) ? 0 : ((y->ip < x->ip) ? -1 : 1));
  /*in_addr_t x = (*((in_addr_t *)a));
  in_addr_t y = (*((in_addr_t *)b));
  return ((x < y) ? -1 : ((x == y) ? 0 : 1));*/
}

static int key_tree_cmp(const void *a, const void *b, void *c)
{
  PCLIENT x = (PCLIENT)a;
  PCLIENT y = (PCLIENT)b;
  int ret = 0;
  ret = memcmp(&x->key, &y->key, sizeof(struct timeval));
  return ret;
  /*epn_client_key x = (*((epn_client_key *)a));
  epn_client_key y = (*((epn_client_key *)b));
  return ((x < y) ? -1 : ((x == y) ? 0 : 1));*/
}

static int client_fd_is_ready(/*void * key, */void * val, void * data)
{
  int ret = 0;
  PCLIENT pc = (PCLIENT)val;
  int *cnt = (int *)data;
  if (pc->readable || (pc->writable && !queue_is_empty(&pc->msgq))) {
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
  struct rb_traverser trav;
  PCLIENT pc = NULL;
  for (pc = (PCLIENT)rb_t_first(&trav, fd_tree);
       pc && !cnt;
       pc = (PCLIENT)rb_t_next(&trav))
    client_fd_is_ready(pc, &cnt);
  return cnt;
}

void free_q_msg(void * data, void * user_data)
{
  PMSG msg = (PMSG)data;
  free(msg);
  msg = NULL;
}

void free_client(PCLIENT *pc)
{
  epn_client_key key = { 0, 0 };

  printf("free_client() fd %d\n", (*pc)->fd);

  rb_delete(fd_tree, (*pc));
  if (epn_get_one_client_per_ip())
    rb_delete(ip_tree, (*pc));
  rb_delete(key_tree, (*pc));

  sock_close(&(*pc)->fd);
  queue_free(&(*pc)->msgq);

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
        pc = calloc(1, (sizeof(CLIENT) + epn_get_client_buf_sz() - 1));
        pc->ip = addr.sin_addr.s_addr;
        pc->fd = clientfd;
        gettimeofday(&pc->key, 0);
        pc->readable = pc->writable = 1;
        rb_insert(fd_tree, pc);
        if (epn_get_one_client_per_ip()) {
          dup = (PCLIENT)rb_find(ip_tree, pc);
          if (dup) {
            free_client(&dup);
            dup = NULL;
          }
          rb_insert(ip_tree, pc);
        }
        pthread_mutex_lock(&mutex);
        rb_insert(key_tree, pc);
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
  printf("free_tree_client_item() fd %d\n", pc->fd);
  sock_close(&pc->fd);
  queue_free(&pc->msgq);
  key = (*((epn_client_key *)&pc->key));
  free(pc);
  pc = NULL;
}

static int trav_fd_svc_ready(PCLIENT pc, list *l)
{
  int ret = 0;
  int n = 0;
  int ok = 1;
  PQMSG qmsg = NULL;
  struct timeval now = { 0, 0 };
  int bufsz = epn_get_client_buf_sz();
  int bytes_to_recv = (bufsz - pc->ri);
  int exp = 0;
  int *pfd = NULL;

  if (pc->readable) {
    n = recv(pc->fd, &pc->buf[pc->ri], bytes_to_recv, 0);
    switch (n) {
    case -1:
      if (errno != EAGAIN) {
        perror("recv()");
        pfd = (int *)malloc(sizeof(int));
        (*pfd) = pc->fd;
        list_add_to_tail(l, pfd);
        pfd = NULL;
        ok = 0;
      }
      else
        pc->readable = 0;
      break;
    case 0:
      printf("fd %d has been closed\n", pc->fd);
      pfd = (int *)malloc(sizeof(int));
      (*pfd) = pc->fd;
      list_add_to_tail(l, pfd);
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
          pfd = (int *)malloc(sizeof(int));
          (*pfd) = pc->fd;
          list_add_to_tail(l, pfd);
          pfd = NULL;
          ok = 0;
        }
        else {
          while ((pc->ri > 2) && (pc->ri >= exp)) {
            if (epn_msg_rcvd_cb)
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
  
  if (ok && pc->writable && !queue_is_empty(&pc->msgq)) {
    queue_peek(&pc->msgq, (void **)&qmsg);
    gettimeofday(&now, 0);
    if (timerisset(&qmsg->ttl) && timercmp(&qmsg->ttl, &now, <)) {
      pc->sent = 0;
      queue_pop(&pc->msgq);
      if (queue_is_empty(&pc->msgq))
        pc->writable = 0;
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
          pfd = (int *)malloc(sizeof(int));
          (*pfd) = pc->fd;
          list_add_to_tail(l, pfd);
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
          queue_pop(&pc->msgq);
        }
        break;
      }
    }
  }

  pc = NULL;

  return ret;
}

void rm_func(void * data, void * user_data)
{
  PCLIENT pc = NULL;
  CLIENT c;
  c.fd = (*((int *)data));
  
  pc = (PCLIENT)rb_find(fd_tree, &c);
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
  list l = LIST_INITIALIZER;
  sigset_t sigmask, origmask;
  struct rb_traverser trav;
  lnode *ln = NULL;

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
        pc = (PCLIENT)rb_find(fd_tree, &c);
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
      list_init(&l);
      pthread_mutex_lock(&mutex);
      for (pc = (PCLIENT)rb_t_first(&trav, fd_tree);
           pc;
           pc = (PCLIENT)rb_t_next(&trav))
        trav_fd_svc_ready(pc, &l);
      ln = l.head;
      while (ln) {
        rm_func(ln->pitem, 0);
        ln = ln->next;
      }
      pthread_mutex_unlock(&mutex);
      list_free(&l);
    }
  }

  return 0;
}

int epn_start()
{
  char addr[32];
  unsigned short port = 0;
  struct epoll_event ev = { 0, { 0 } };

  epn_get_bind_addr(addr, sizeof(addr));
  port = epn_get_bind_port();

  fd_tree = rb_create(fd_tree_cmp, 0, 0);
  if (epn_get_one_client_per_ip())
    ip_tree = rb_create(ip_tree_cmp, 0, 0);
  key_tree = rb_create(key_tree_cmp, 0, 0);
  
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

  nevents = epn_get_est_events();
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
  if (pthread_create(&tid, 0, thread_entry_point, 0)) {
    run = 0;
    sock_close(&svrfd);
    close(ep);
    ep = 0;
    return EPN_NOTHREAD;
  }

  return 0;
}

int epn_stop()
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
    rb_destroy(fd_tree, 0);
    fd_tree = NULL;
    if (epn_get_one_client_per_ip()) {
      rb_destroy(ip_tree, 0);
      ip_tree = NULL;
    }
    rb_destroy(key_tree, free_tree_client_item);
    key_tree = NULL;
  }
  return (n ? EPN_NOJOIN : 0);
}

void epn_set_msg_rcvd_cb(int (*callback)(PMSG msg, epn_client_key key))
{
  epn_msg_rcvd_cb = callback;
}

void epn_set_client_closed_cb(int (*callback)(epn_client_key key))
{
  epn_client_closed_cb = callback;
}

void epn_set_client_accepted_cb(int (*callback)(epn_client_key key))
{
  epn_client_accepted_cb = callback;
}

int epn_send_to_client(epn_client_key key, const PMSG msg,
                       const unsigned int ttl)
{
  PQMSG qmsg = NULL;
  PCLIENT pc = NULL;
  CLIENT c;
  struct timeval now = { 0, 0 }, diff = { 0, 0 };
  epn_client_key k = key;

  memcpy(&c.key, &k, sizeof(c.key));
  pthread_mutex_lock(&mutex);
  pc = (PCLIENT)rb_find(key_tree, &c);
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
    queue_push(&pc->msgq, qmsg);
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

