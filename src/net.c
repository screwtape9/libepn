#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include "net.h"

int sock_create(int *fd, int is_stream)
{
  (*fd) = (is_stream ? socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)
                     : socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP));
  return (((*fd) == -1) ? -1 : 0);
}

int sock_shutdown(int *fd)
{
  return shutdown((*fd), SHUT_RDWR);
}

int sock_close(int *fd)
{
  int n = close((*fd));
  if (!n)
    (*fd) = 0;
  return n;
}

int sock_bind(int *fd, unsigned short port, const char *addr)
{
  struct sockaddr_in sa;
  struct hostent *host = NULL;

  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  if (!addr || !strlen(addr))
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
  else {
    sa.sin_addr.s_addr = inet_addr(addr);
    if (sa.sin_addr.s_addr == INADDR_NONE) {
      host = gethostbyname(addr);
      if (host) {
        sa.sin_addr.s_addr = ((struct in_addr *)host->h_addr)->s_addr;
        host = NULL;
      }
      else
        return -1;
    }
  }
  sa.sin_port = htons(port);
  return bind((*fd), (struct sockaddr *)&sa, sizeof(sa));
}

int sock_listen(int *fd, int backlog)
{
  return listen((*fd), backlog);
}

int sock_accept(int *server_fd, int *client_fd, struct sockaddr *addr)
{
  struct sockaddr dummy;
  socklen_t len = sizeof(dummy);

  (*client_fd) = (addr ? (accept((*server_fd), addr, &len))
                       : (accept((*server_fd), &dummy, &len)));
  return (*client_fd);
}

int sock_send(int *fd, void *buf, int len, int timeout_secs)
{
  char *p = (char *)buf;
  int n, rem = len, elapsed_ms = 0;
  struct timeval start, now;

  gettimeofday(&start, 0);
  while (rem > 0) {
    n = send((*fd), p, rem, 0);
    if (n == -1) {
      if (errno != EWOULDBLOCK)
        break;

      gettimeofday(&now, 0);
      elapsed_ms = ((now.tv_usec - start.tv_usec) / 1000);
      elapsed_ms += ((now.tv_sec - start.tv_sec) * 1000);
      if (elapsed_ms > (timeout_secs * 1000))
        break;
    }
    else {
      p += n;
      rem -= n;
    }
  }
  p = NULL;
  return (len - rem);  
}

int sock_set_blocking(int *fd, int blocking)
{
  int val = fcntl((*fd), F_GETFL);
  if (val == -1)
    return -1;
  if (blocking)
    val &= ~O_NONBLOCK;
  else
    val |= O_NONBLOCK;
  return fcntl((*fd), F_SETFL, val);
}

int sock_connect(int *fd, const char *host, unsigned short port,
                 int timeout_secs)
{
  int n = 0;
  socklen_t len = sizeof(n);
  struct sockaddr_in addr;
  struct hostent *lphost = NULL;
  fd_set writefds;
  struct timeval timeout;
  
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(host);
  if (addr.sin_addr.s_addr == INADDR_NONE) {
    lphost = gethostbyname(host);
    if (lphost) {
      addr.sin_addr.s_addr = ((struct in_addr *)lphost->h_addr)->s_addr;
      lphost = NULL;
    }
    else
      return -1;
  }
  addr.sin_port = htons(port);
  
  if (sock_set_blocking(fd, 0))
    return -1;
  
  n = connect((*fd), (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
  if (n == -1) {
    if (errno == EINPROGRESS) {
      FD_ZERO(&writefds);
      FD_SET((*fd), &writefds);
      
      timeout.tv_sec  = timeout_secs;
      timeout.tv_usec = 0;
      
      n = select(((*fd) + 1), 0, &writefds, 0, &timeout);
      if ((n > 0) && FD_ISSET((*fd), &writefds)) {
        getsockopt((*fd), SOL_SOCKET, SO_ERROR, &n, &len);
        return (n == 0) ? 0 : -1;
      }
    }
    else
      return -1;
  }
  else
    return 0;

  return -1;
}

int sock_reuseaddr(int *fd)
{
  int val = 1;
  return setsockopt((*fd), SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
}

