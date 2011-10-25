#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "epn_cfg.h"
#include "epn_clt.h"

int connected()
{
  printf("We're connected!\n");
  return 0;
}

int got_msg(PMSG pmsg)
{
  printf("RECVD %d bytes : %s\n", pmsg->len, pmsg->buf);
  /*free(pmsg);*/
  return 0;
}

int closed()
{
  printf("We've been disconnected!\n");
  return 0;
}

int main(int argc, char *argv[])
{
  char buf[512];
  PMSG pmsg = (PMSG)buf;

  if (argc != 3) {
    printf("usage: %s <host> <port>\n", argv[0]);
    return -1;
  }

  epn_clt_init(argv[1], (unsigned short)atoi(argv[2]), 512, 1);
  epn_clt_set_connected_cb(&connected);
  epn_clt_set_msg_rcvd_cb(&got_msg);
  epn_clt_set_closed_cb(&closed);
  epn_clt_start();

  while (1) {
    scanf("%s", &buf[2]);
    if (!strcmp(&buf[2], "bye"))
      break;
    pmsg->len = (strlen(&buf[2]) + 2 + 1);
    epn_clt_send(pmsg, 3);
  }

  epn_clt_stop();
  epn_cleanup();

  return 0;
}
