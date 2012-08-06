#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "epn_cfg.h"

static char *epn_svr_bind_addr = NULL;
static unsigned short epn_svr_bind_port = 0;
static unsigned int epn_svr_est_events = 0;
static unsigned int epn_svr_client_buf_sz = 4096;
static unsigned int epn_svr_one_client_per_ip = 0;
static unsigned int epn_svr_queue_max_msgs = 0;

static char *epn_clt_host_ip_addr = NULL;
static unsigned short epn_clt_host_ip_port = 0;
static unsigned int epn_clt_client_buf_sz = 4096;
static unsigned int epn_clt_queue_max_msgs = 0;

void epn_svr_init(const char *bind_addr, const unsigned short bind_port,
                  const unsigned int est_events,
                  const unsigned int client_buf_sz,
                  const unsigned int one_client_per_ip,
                  const unsigned int queue_max_msgs)
{
  int len = 0;
  if (bind_addr) {
    len = strlen(bind_addr);
    if (len) {
      epn_svr_bind_addr = (char *)malloc(len + 1);
      strcpy(epn_svr_bind_addr, bind_addr);
    }
  }
  epn_svr_bind_port = bind_port;

  epn_svr_est_events = est_events;

  if (client_buf_sz > 1)
    epn_svr_client_buf_sz = client_buf_sz;

  epn_svr_one_client_per_ip = one_client_per_ip;

  epn_svr_queue_max_msgs = queue_max_msgs;
}

void epn_svr_get_bind_addr(char *buf, int sz)
{
  if (epn_svr_bind_addr)
    strncpy(buf, epn_svr_bind_addr, sz);
  else
    buf[0] = 0;
}

unsigned short epn_svr_get_bind_port()
{
  return epn_svr_bind_port;
}

unsigned int epn_svr_get_est_events()
{
  return epn_svr_est_events;
}

unsigned int epn_svr_get_client_buf_sz()
{
  return epn_svr_client_buf_sz;
}

unsigned int epn_svr_get_one_client_per_ip()
{
  return epn_svr_one_client_per_ip;
}

unsigned int epn_svr_get_queue_max_msgs()
{
  return epn_svr_queue_max_msgs;
}

void epn_cleanup()
{
  if (epn_svr_bind_addr) {
    free(epn_svr_bind_addr);
    epn_svr_bind_addr = NULL;
  }

  if (epn_clt_host_ip_addr) {
    free(epn_clt_host_ip_addr);
    epn_clt_host_ip_addr = NULL;
  }
}

void epn_clt_init(const char *ip, const unsigned short port,
                  const unsigned int client_buf_sz,
                  const unsigned int queue_max_msgs)
{
  int len = 0;
  if (ip) {
    len = strlen(ip);
    if (len) {
      epn_clt_host_ip_addr = (char *)malloc(len + 1);
      strcpy(epn_clt_host_ip_addr, ip);
    }
  }
  epn_clt_host_ip_port = port;

  if (client_buf_sz > 1)
    epn_clt_client_buf_sz = client_buf_sz;

  epn_clt_queue_max_msgs = queue_max_msgs;
}

void epn_clt_get_host_ip_addr(char *buf, int sz)
{
  if (epn_clt_host_ip_addr)
    strncpy(buf, epn_clt_host_ip_addr, sz);
  else
    buf[0] = 0;
}

unsigned short epn_clt_get_host_ip_port()
{
  return epn_clt_host_ip_port;
}

unsigned int epn_clt_get_client_buf_sz()
{
  return epn_clt_client_buf_sz;
}

unsigned int epn_clt_get_queue_max_msgs()
{
  return epn_clt_queue_max_msgs;
}

void epn_get_ver_str(char *buf, int sz)
{
  snprintf(buf, sz, "libepn %d.%d.%d", MAJOR_VERSION, MINOR_VERSION, PATCH_LEVEL);
}
