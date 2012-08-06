#if !defined __EPN_DEF_H_
#define __EPN_DEF_H_

#include <time.h>
#include <netinet/in.h>
#if USE_GLIB
#include <glib.h>
#else /* USE_GLIB */
#include "queue.h"
#endif /* USE_GLIB */
#include "epn_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _client {
  in_addr_t ip;
  int fd;
  struct timeval key;
  int ri;
#if USE_GLIB
  GQueue *msgq;
#else /* USE_GLIB */
  queue msgq;
#endif /* USE_GLIB */
  int readable;
  int writable;
  int sent;
  char buf[1];
} CLIENT, *PCLIENT;

typedef struct _qmsg {
  struct timeval ttl;
  MSG msg;
} QMSG, *PQMSG;

#ifdef __cplusplus
}
#endif

#endif /* __EPN_DEF_H_ */
