#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include "./logit.h"

void logit(const char *fmt, ...)
{
  char buf[2048];
  struct tm t;
  struct timeval tv;
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  gettimeofday(&tv, 0);
  localtime_r((const time_t *)&tv.tv_sec, &t);
  printf("%02d:%02d:%02d.%03ld - %s\n", t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec / 1000, buf);
}

void logerr(const char *fmt, ...)
{
  char buf[2048];
  struct tm t;
  struct timeval tv;
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  syslog(LOG_ERR, "%s %s", buf, strerror(errno));
  gettimeofday(&tv, 0);
  localtime_r((const time_t *)&tv.tv_sec, &t);
  printf("%02d:%02d:%02d.%03ld - %s : %s\n", t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec / 1000, buf, strerror(errno));
}
