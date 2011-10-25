#if !defined __EPN_CLT_H_
#define __EPN_CLT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "epn_msg.h"

/*! \file epn_clt.h
 *  \brief epn client functions.
 */

/*! \fn int epn_clt_start()
 *  \brief Starts a 'socket connection' thread.
 *
 *  The thread will continuously try to connect to the host. Once connected, it
 *  processes incoming/outgoing MSGs. If the connection is ever lost, it goes
 *  back to continuously connecting, and then back to servicing MSGs, and on
 *  and on.
 *
 *  \return 0 on success; otherwise, an error code from epn_errors.h
 */
int epn_clt_start();

/*! \fn int epn_clt_stop()
 *  \brief Stops the 'socket connection' thread.
 *  \return 0 on success, or EPN_JOIN if the worker thread could not be joined.
 */
int epn_clt_stop();

/*! \fn void epn_clt_set_connected_cb(int (*callback)())
 *  \brief Sets the 'client has connected' callback function.
 *  \param[in] callback A pointer to a function that will be called upon the
 *  client socket connection having been established.
 *  \return None.
 */
void epn_clt_set_connected_cb(int (*callback)());

/*! \fn void epn_clt_set_msg_rcvd_cb(int (*callback)(PMSG msg))
 *  \brief Sets the 'message has been received' callback function.
 *  \param[in] callback A pointer to a function that will be called upon a 
 *  message having been received.
 *  \return None.
 */
void epn_clt_set_msg_rcvd_cb(int (*callback)(PMSG msg));

/*! \fn void epn_clt_set_closed_cb(int (*callback)())
 *  \brief Sets the 'connection has been closed' callback function.
 *  \param[in] callback A pointer to a function that will be called upon the
 *  connection having been disconnected/closed.
 *  \return None.
 */
void epn_clt_set_closed_cb(int (*callback)());

/*! \fn int epn_clt_send(const PMSG msg, const unsigned int ttl)
 *  \brief Queues a MSG to be sent out the socket connection.
 *  \param[in] msg The MSG to send.
 *  \param[in] ttl Time to live:  the number of millseconds in which to try
 *  to send the msg out before dropping it.
 *  \return
 */
int epn_clt_send(const PMSG msg, const unsigned int ttl);

#ifdef __cplusplus
}
#endif

#endif /* __EPN_CLT_H_ */
