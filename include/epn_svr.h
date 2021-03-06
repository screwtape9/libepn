#if !defined __EPN_SVR_H_
#define __EPN_SVR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "epn_msg.h"

/*! \file epn_svr.h
 *  \brief epn server functions.
 */

/*! \fn int epn_svr_start()
 *  \brief Starts the epn server thread.
 *
 *  Creates a listening server socket, an epoll structure, and spawns a thread
 *  to handle the network events.
 *
 *  \return 0 on success; otherwise, an error code from epn_errors.h
 */
int epn_svr_start();

/*! \fn int epn_svr_stop()
 *  \brief Stops the epn server thread.
 *  \return 0 on success, or EPN_JOIN if the worker thread could not be joined.
 */
int epn_svr_stop();

/*! \fn void epn_svr_set_msg_rcvd_cb(int (*callback)(PMSG msg, epn_client_key key))
 *  \brief Sets the 'client message has been received' callback function.
 *  \param[in] callback A pointer to a function that will be called upon a 
 *  message being received from a client.
 *  \return None.
 */
void epn_svr_set_msg_rcvd_cb(int (*callback)(PMSG msg, epn_client_key key));

/*! \fn void epn_svr_set_client_closed_cb(int (*callback)(epn_client_key key))
 *  \brief Sets the 'client has been closed' callback function.
 *  \param[in] callback A pointer to a function that will be called upon a 
 *  client connection having been disconnected/closed.
 *  \return None.
 */
void epn_svr_set_client_closed_cb(int (*callback)(epn_client_key key));

/*! \fn void epn_svr_set_client_accepted_cb(int (*callback)(epn_client_key key))
 *  \brief Sets the 'a new client has been accepteed' callback function.
 *  \param[in] callback A pointer to a function that will be called upon a 
 *  new client connection having been established.
 *  \return None.
 */
void epn_svr_set_client_accepted_cb(int (*callback)(epn_client_key key));

/*! \fn int epn_svr_send_to_client(epn_client_key key, PMSG msg)
 *  \brief Queues a MSG to be sent to a client connection.
 *  \param[in] key A key which correlates to a client connection.
 *  \param[in] msg The MSG to send.
 *  \param[in] ttl Time to live:  the number of millseconds in which to try
 *  to send the msg out before dropping it.
 *  \return
 */
int epn_svr_send_to_client(epn_client_key key, const PMSG msg,
                       const unsigned int ttl);

#ifdef __cplusplus
}
#endif

#endif /* __EPN_SVR_H_ */
