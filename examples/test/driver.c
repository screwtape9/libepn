#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#define __USE_GNU    /* For TIMESPEC_TO_TIMEVAL and TIMEVAL_TO_TIMESPEC */
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include "lorem.h"
#include "epn_cfg.h"
#include "epn_clt.h"
#include "printsig.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int rc = 0, num_recvd_msgs = 0, msg_recvd = 0, run = 1, is_connected = 0;
static unsigned short len_recvd = 0;
static struct timeval start = { 0, 0 };
static unsigned int avg, totals[4098];

static void sig_handler(int sig)
{
  print_sig_desc(sig);
  write(STDOUT_FILENO, "Exiting...\n", 12);
  if ((sig == SIGINT) || (sig == SIGTERM))
    _exit(0);
  else
    _exit(1);
}

static int handle_signals()
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

static int get_rand(double x, double y)
{
  return (1 + (int)(y * (rand() / (RAND_MAX + 1.0))));
}

static int get_elapsed_ms(struct timeval *t)
{
  struct timeval now, diff;
  if (gettimeofday(&now, 0) < 0)
    return -1;
  timersub(&now, t, &diff);
  return (diff.tv_sec * 1000) + (diff.tv_usec / 1000);
}

static int connected()
{
  printf("We're connected!\n");
  pthread_mutex_lock(&mutex);
  is_connected = 1;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  return 0;
}

static int got_msg(PMSG pmsg)
{
  /*printf("RECVD %d bytes : %s\n", pmsg->len, pmsg->buf);*/
  pthread_mutex_lock(&mutex);
  msg_recvd = 1;
  len_recvd = pmsg->len;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  return 0;
}

static int closed()
{
  printf("We've been disconnected!\n");
  pthread_mutex_lock(&mutex);
  is_connected = 0;
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mutex);
  return 0;
}

static void set_time_to_wait(struct timespec *time_to_wait, __time_t sec,
                             __suseconds_t usec)
{
  struct timespec now = { 0, 0 };
  struct timeval vtime_to_wait = { 0, 0 }, vnow = { 0, 0 },
                 vtime_to_add = { 0, 0 };
  vtime_to_add.tv_sec = sec;
  vtime_to_add.tv_usec = usec;
  clock_gettime(CLOCK_REALTIME, &now);
  TIMESPEC_TO_TIMEVAL(&vnow, &now);
  timeradd(&vnow, &vtime_to_add, &vtime_to_wait);
  TIMEVAL_TO_TIMESPEC(&vtime_to_wait, time_to_wait);
}

int main(int argc, char *argv[])
{
  char *host = 0;
  unsigned short port = 0;
  char buf[512], err[512];
  PMSG pmsg = (PMSG)buf;
  int n = 0, loop = 0, i = 0;
  struct timespec time_to_wait = { 0, 0 };
  struct timeval lstart = { 0, 0 };
  struct timeval now = { 0, 0 };
  struct timeval diff = { 0, 0 };

  if (argc != 3) {
    printf("usage: %s <host> <port>\n", argv[0]);
    return -1;
  }

  if (handle_signals()) {
    printf("Failed to set up signal handling.\n");
    return -1;
  }

  epn_get_ver_str(buf, sizeof(buf));
  printf("Using %s\n", buf);

  srand(time(0));

  host = argv[1];
  port = (unsigned short)atoi(argv[2]);

  memset(totals, 0, sizeof(totals));

  epn_clt_init(host, port, 2048, 1);
  epn_clt_set_connected_cb(&connected);
  epn_clt_set_msg_rcvd_cb(&got_msg);
  epn_clt_set_closed_cb(&closed);
  pthread_mutex_lock(&mutex);
  epn_clt_start();
  rc = 0;
  while (run && !is_connected && !rc)
    rc = pthread_cond_wait(&cond, &mutex);
  pthread_mutex_unlock(&mutex);

  gettimeofday(&start, 0);
  gettimeofday(&lstart, 0);
  while (run && (num_recvd_msgs < 100000)) {
    memset(pmsg, 0, sizeof(buf));
    n = (get_rand(1.0, 26.0) - 1);
    strncpy(pmsg->buf, lorems[n], (sizeof(buf) - 2));
    pmsg->len = (unsigned short)(strlen(pmsg->buf) + 2 + 1);
    pthread_mutex_lock(&mutex);
    rc = msg_recvd = 0;
    epn_clt_send(pmsg, 3000);
    set_time_to_wait(&time_to_wait, 1, 0);
    while (run && is_connected && !msg_recvd && !rc)
      rc = pthread_cond_timedwait(&cond, &mutex, &time_to_wait);
    if (!rc) {
      if (!is_connected) {
        /* reconnect */
        is_connected = rc = 0;
        while (run && !is_connected && !rc)
          rc = pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex);
      }
      else if (len_recvd == pmsg->len) {
        pthread_mutex_unlock(&mutex);
        num_recvd_msgs++;
        totals[loop]++;
        //printf("REVCD:  %s\n\n", msg.buf);
        if (get_elapsed_ms(&start) >= 5000) {
          for (i = 0, avg = 0; i < loop; i++)
            avg += totals[i];
          avg /= (loop + 1);
          avg /= 5;
          gettimeofday(&start, 0);
          printf("total recvd msgs:  %d\n", num_recvd_msgs);
          printf("avg per sec:       %u\n\n", avg);
          if (++loop == 4098) {
            printf("PEACE OUT BITCHES!\n");
            break;
          }
        }
      }
      else {
        pthread_mutex_unlock(&mutex);
        printf("something has gone awry...\n");
        break;
      }
    }
    else {
      pthread_mutex_unlock(&mutex);
      if (rc == ETIMEDOUT)
        printf("timed out waiting for response...\n");
      else {
        if (!strerror_r(rc, err, sizeof(err)))
          printf("ERROR:  %s\n", err);
        else
          printf("unknown error!\n");
      }
    }
  }
  gettimeofday(&now, 0);
  timersub(&now, &lstart, &diff);
  printf("Time:  %lds %ldms\n", diff.tv_sec, diff.tv_usec);

  epn_clt_stop();
  epn_cleanup();

  return 0;
}
