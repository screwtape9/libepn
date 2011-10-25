#if !defined __EPN_CFG_H_
#define __EPN_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*! \file epn_cfg.h
 *  \brief Initialization/configuration functions.
 */

/*! \fn void epn_svr_init(const char *bind_addr, const unsigned short bind_port,
 *                        const unsigned int est_events,
 *                        const unsigned int client_buf_sz,
 *                        const unsigned int one_client_per_ip,
 *                        const unsigned int queue_max_msgs)
 *  \brief Initializes the epn library for server use.
 *
 *  Sets the IP address and port on which to bind and listen for incoming
 *  connections. \a est_events doesn't have to be exact, just a guess as to how
 *  many file descriptors will be monitored; it is passed directly into
 *  epoll_create();
 *
 *  \param[in] bind_addr The IP address to bind on.
 *  \param[in] bind_port The IP port to bind on.
 *  \param[in] est_events The estimated number of file descriptors to monitor.
 *  \param[in] client_buf_sz The size of each client's receive buffer.
 *  \param[in] one_client_per_ip Limit one client connection per IP address.
 *  \param[in] queue_max_msgs Max number of messages that can be queued per
 *  client. If this is set to 1, the incoming message to be queued will replace
 *  the currently queued message, if one exists.
 *  \return none
 */
void epn_svr_init(const char *bind_addr, const unsigned short bind_port,
                  const unsigned int est_events,
                  const unsigned int client_buf_sz,
                  const unsigned int one_client_per_ip,
                  const unsigned int queue_max_msgs);

/*! \fn void epn_svr_get_bind_addr(char *buf, int sz)
 *  \brief Gets the IP address to bind on.
 *  \param[out] buf The buffer in which to store the bind address.
 *  \param[in] sz The size of the buffer.
 *  \return none
 */
void epn_svr_get_bind_addr(char *buf, int sz);

/*! \fn unsigned short epn_svr_get_bind_port()
 *  \brief Gets the IP port to bind on.
 *  \return The IP port.
 */
unsigned short epn_svr_get_bind_port();

/*! \fn unsigned int epn_svr_get_est_events()
 *  \brief Gets the estimated number of file descriptors to be monitored.
 *  \return The number of estimated monitored events.
 */
unsigned int epn_svr_get_est_events();

/*! \fn unsigned int epn_svr_get_client_buf_sz()
 *  \brief Gets the receive buffer size for each client.
 *  \return The receive buffer size for each client.
 */
unsigned int epn_svr_get_client_buf_sz();

/*! \fn unsigned int epn_svr_get_one_client_per_ip()
 *  \brief Returns whether or not to limit one client connection per IP.
 *  \return TRUE or FALSE on whether or not to limit one client connection per IP.
 */
unsigned int epn_svr_get_one_client_per_ip();

/*! \fn unsigned int epn_svr_get_queue_max_msgs()
 *  \brief Returns the max number of outbound MSGs that can be queued up for a
 *  particular client.
 *  \return The max number of outbound MSGs that can be queued up for a
 *  particular client.
 */
unsigned int epn_svr_get_queue_max_msgs();

/*! \fn void epn_cleanup()
 *  \brief Unitializes the epn library.
 *
 *  Stops the epn thread (if not already stopped) and frees all resources.
 *
 *  \return none
 */
void epn_cleanup();

/*! \fn void epn_clt_init(const char *ip, const unsigned short port,
 *                        const unsigned int client_buf_sz,
 *                        const unsigned int queue_max_msgs)
 *  \brief Initializes the epn library for client use.
 *
 *  Sets the IP address and port to connect to.
 *
 *  \param[in] ip The host IP address.
 *  \param[in] port The host IP port.
 *  \param[in] client_buf_sz The size of the client's receive buffer.
 *  \param[in] queue_max_msgs Max number of messages that can be queued on the
 *  connection. If this is set to 1, the incoming message to be queued will
 *  replace the currently queued message, if one exists.
 *  \return none
 */
void epn_clt_init(const char *ip, const unsigned short port,
                  const unsigned int client_buf_sz,
                  const unsigned int queue_max_msgs);

/*! \fn void epn_clt_get_host_ip_addr(char *buf, int sz)
 *  \brief Gets the IP address to connect to.
 *  \param[out] buf The buffer in which to store the address.
 *  \param[in] sz The size of the buffer.
 *  \return none
 */
void epn_clt_get_host_ip_addr(char *buf, int sz);

/*! \fn unsigned short epn_clt_get_host_ip_port()
 *  \brief Gets the IP port to connect to.
 *  \return The IP port.
 */
unsigned short epn_clt_get_host_ip_port();

/*! \fn unsigned int epn_clt_get_client_buf_sz()
 *  \brief Gets the receive buffer size.
 *  \return The receive buffer size.
 */
unsigned int epn_clt_get_client_buf_sz();

/*! \fn unsigned int epn_clt_get_queue_max_msgs()
 *  \brief Returns the max number of outbound MSGs that can be queued up.
 *  \return The max number of outbound MSGs that can be queued up.
 */
unsigned int epn_clt_get_queue_max_msgs();

#ifdef __cplusplus
}
#endif

#endif /* __EPN_CFG_H_ */
