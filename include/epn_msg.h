#if !defined __EPN_MSG_H_
#define __EPN_MSG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

/*! \file epn_msg.h
 *  \brief epn msg definition
 */

/*! \typedef typedef struct timeval epn_client_key;
 *  \brief The key type used for identifiying clients.
 */
typedef struct timeval epn_client_key;

/*! \typedef typedef struct _msg MSG
 *  \brief The base for every message sent/received.
 */
/*! \typedef typedef struct _msg *PMSG
 *  \brief Convenience type definition for a pointer to a MSG;
 */
typedef struct _msg {
  unsigned short len;
  char buf[1];
} MSG, *PMSG;


#ifdef __cplusplus
}
#endif

#endif /* __EPN_MSG_H_ */
