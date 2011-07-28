#if !defined __EPN_SVR_H_
#define __EPN_SVR_H_

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \file epn_svr.h
 *  \brief epn server functions.
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

/*! \fn int epn_start()
 *  \brief Starts the epn server.
 *
 *  Creates a listening server socket, an epoll structure, and spawns a thread
 *  to handle the network events.
 *
 *  \return 0 on success; otherwise, an error code from epn_errors.h
 */
int epn_start();

/*! \fn int epn_stop()
 *  \brief Stops the epn server.
 *  \return 0 on success, or EPN_JOIN if the worker thread could not be joined.
 */
int epn_stop();

/*! \fn void epn_set_msg_rcvd_cb(int (*callback)(PMSG msg, epn_client_key key))
 *  \brief Sets the 'client message has been received' callback function.
 *  \param[in] callback A pointer to a function that will be called upon a 
 *  message being received from a client.
 *  \return None.
 */
void epn_set_msg_rcvd_cb(int (*callback)(PMSG msg, epn_client_key key));

/*! \fn void epn_set_client_closed_cb(int (*callback)(epn_client_key key))
 *  \brief Sets the 'client has been closed' callback function.
 *  \param[in] callback A pointer to a function that will be called upon a 
 *  client connection having been disconnected/closed.
 *  \return None.
 */
void epn_set_client_closed_cb(int (*callback)(epn_client_key key));

/*! \fn void epn_set_client_accepted_cb(int (*callback)(epn_client_key key))
 *  \brief Sets the 'a new client has been accepteed' callback function.
 *  \param[in] callback A pointer to a function that will be called upon a 
 *  new client connection having been established.
 *  \return None.
 */
void epn_set_client_accepted_cb(int (*callback)(epn_client_key key));

/*! \fn int epn_send_to_client(epn_client_key key, PMSG msg)
 *  \brief Queues a MSG to be sent to a client connection.
 *  \param[in] key A key which correlates to a client connection.
 *  \param[in] msg The MSG to send.
 *  \return
 */
int epn_send_to_client(epn_client_key key, PMSG msg);

#ifdef __cplusplus
}
#endif

#endif /* __EPN_SVR_H_ */
