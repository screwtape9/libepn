#if !defined __EPN_NETUTIL_H_
#define __EPN_NETUTIL_H_

#include <netinet/in.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \file epn_netutil.h
 *  \brief IP socket utility functions.
 */

/*! \fn int sock_create(int *fd, int is_stream)
 *  \brief Creates a socket.
 *  \param[in] fd A pointer to an int in which the newly created socket file
 *  desciptor will be stored.
 *  \param[in] is_stream If non-zero, then a datagram (UDP) socket will be
 *  created; otherwise, a stream (TCP) socket will be created.
 *  \return 0 on success, -1 on failure
 */
int sock_create(int *fd, int is_stream);

/*! \fn int sock_shutdown(int *fd)
 *  \brief Shuts down receiving and transmitting on the socket.
 *  \param[in] fd A pointer to the socket file descriptor.
 *  \return 0 on success, -1 on failure
 */
int sock_shutdown(int *fd);

/*! \fn int sock_close(int *fd)
 *  \brief Closes the socket file descriptor.
 *  \param[in] fd A pointer to the socket file descriptor.
 *  \return 0 on success, -1 on failure
 */
int sock_close(int *fd);

/*! \fn int sock_bind(int *fd, unsigned short port, const char *addr)
 *  \brief Assigns the IP port and address to the socket.
 *  \param[in] fd A pointer to the socket file descriptor.
 *  \param[in] port The IP port on which to bind.
 *  \param[in] addr Optional IP address on which to bind.
 *  \return 0 on success, -1 on failure
 */
int sock_bind(int *fd, unsigned short port, const char *addr);

/*! \fn int sock_listen(int *fd, int backlog)
 *  \brief Marks the socket passive. i.e. a socket that will receive incoming
 *  connections.
 *  \param[in] fd A pointer to the socket file descriptor.
 *  \param[in] backlog How many pending connections to queue up.
 *  \return 0 on success, -1 on failure
 */
int sock_listen(int *fd, int backlog);

/*! \fn int sock_accept(int *server_fd, int *client_fd, struct sockaddr *addr)
 *  \brief Accepts an incoming TCP socket connection.
 *  \param[in] server_fd A pointer to a listening socket file descriptor.
 *  \param[out] client_fd A pointer to an int in which the file descriptor of
 *  the newly accepted connection's socket will be stored.
 *  \param[out] addr Optional pointer to a sockaddr struct. If not null, it will
 *  be filled in with the address information of the new client socket.
 *  \return 0 on success, -1 on failure
 */
int sock_accept(int *server_fd, int *client_fd, struct sockaddr *addr);

/*! \fn int sock_send(int *fd, void *buf, int len, int timeout_secs)
 *  \brief Sends \a len bytes out across the socket.
 *  \param[in] fd A pointer to the socket file descriptor.
 *  \param[in] buf A pointer to the buffer containing the bytes to be sent.
 *  \param[in] len The number of bytes to send.
 *  \param[in] timeout_secs How long to try 'brute force' sending the bytes
 *  across before giving up.
 *  \return 0 on success, -1 on failure
 */
int sock_send(int *fd, void *buf, int len, int timeout_secs);

/*! \fn int sock_set_blocking(int *fd, int blocking)
 *  \brief Sets the socket's blocking mode.
 *  \param[in] fd A pointer to the socket file descriptor.
 *  \param[in] blocking If non-zero, the socket will be set to blocking mode;
 *  otherwise, non-blocking mode.
 *  \return The number of bytes actually sent.
 */
int sock_set_blocking(int *fd, int blocking);

/*! \fn int sock_connect(int *fd, const char *host, unsigned short port,
                         int timeout_secs)
 *  \brief Connects the socket to a remote host socket.
 *  \param[in] fd A pointer to the socket file descriptor.
 *  \param[in] host The IP address or hostname of the remote host to connect to.
 *  \param[in] port The IP port to connect on.
 *  \param[in] timeout_secs How long wait for the connection to complete before
 *  giving up.
 *  \return 0 on success, -1 on failure
 */
int sock_connect(int *fd, const char *host, unsigned short port,
                 int timeout_secs);

/*! \fn int sock_reuseaddr(int *fd)
 *  \brief Allows the reuse of local addresses when binding a socket.
 *  \param[in] fd A pointer to the socket file descriptor.
 *  \return 0 on success, -1 on failure
 */
int sock_reuseaddr(int *fd);

#ifdef __cplusplus
}
#endif

#endif /* __EPN_NETUTIL_H_ */
