#include <string.h>
#include <stdlib.h>
#include "epn_cfg.h"

static char *epn_bind_addr = 0;
static unsigned short epn_bind_port = 0;
static unsigned int epn_est_events = 0;
static unsigned int epn_client_buf_sz = 4096;
static unsigned int epn_one_client_per_ip = 0;
static unsigned int epn_queue_max_msgs = 0;

void epn_init(const char *bind_addr, const unsigned short bind_port,
              const unsigned int est_events, const unsigned int client_buf_sz,
              const unsigned int one_client_per_ip,
              const unsigned int queue_max_msgs)
{
  int len = 0;
  if (bind_addr) {
    len = strlen(bind_addr);
    if (len) {
      epn_bind_addr = (char *)malloc(len + 1);
      strcpy(epn_bind_addr, bind_addr);
    }
  }
  epn_bind_port = bind_port;
  epn_est_events = est_events;
  if (client_buf_sz > 1)
    epn_client_buf_sz = client_buf_sz;

  epn_one_client_per_ip = one_client_per_ip;
  epn_queue_max_msgs = queue_max_msgs;
}

void epn_get_bind_addr(char *buf, int sz)
{
  if (epn_bind_addr)
    strncpy(buf, epn_bind_addr, sz);
  else
    buf[0] = 0;
}

unsigned short epn_get_bind_port()
{
  return epn_bind_port;
}

unsigned int epn_get_est_events()
{
  return epn_est_events;
}

unsigned int epn_get_client_buf_sz()
{
  return epn_client_buf_sz;
}

unsigned int epn_get_one_client_per_ip()
{
  return epn_one_client_per_ip;
}

unsigned int epn_get_queue_max_msgs()
{
  return epn_queue_max_msgs;
}

void epn_cleanup()
{
  int len = 0;
  if (epn_bind_addr) {
    len = strlen(epn_bind_addr);
    free(epn_bind_addr);
    epn_bind_addr = 0;
  }
}
