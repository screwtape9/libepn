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

#define EPN_IS_READABLE(p) (p->status & fd_readable)
#define EPN_IS_WRITABLE(p) (p->status & fd_writable)
#define EPN_SET_READABLE(p) set_opt(p, fd_readable, 1)
#define EPN_SET_WRITABLE(p) set_opt(p, fd_writable, 1)
#define EPN_CLEAR_READABLE(p) set_opt(p, fd_readable, 0)
#define EPN_CLEAR_WRITABLE(p) set_opt(p, fd_writable, 0)

extern const unsigned int fd_readable;
extern const unsigned int fd_writable;

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
  int status;
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
