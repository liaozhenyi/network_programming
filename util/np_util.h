#ifndef __NP_UTIL_H_
#define __NP_UTIL_H_

#include <errno.h>		// errno
#include <stdarg.h>		// VA_LIST
#include <unistd.h>		// read(), write(), fork()
#include <signal.h>		// signal()
#include <netdb.h>		// gethostbyname()

#include <netinet/in.h>		// struct sockaddr_in

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>		// WNOHANG

#define BUFFER_SIZE	4096
#define MAX_LINE	1024
#define BACKLOG		1024

typedef struct sockaddr SA;
typedef void Sigfunc(int );

/*
 * Implementation of wrapper function for socket flow IO 
 * for handling short cut.
 */
ssize_t readn(int fd, char *buf, size_t bytes);
ssize_t writen(int fd, char *buf, size_t bytes);

/*
 * Buffer associate with specific file descriptor.
 * cnt: how many bytes unread in the buffer,
 * bufp: pointer the next unread position
 */
typedef struct {
	int 	fd;
	size_t	cnt;
	char	buf[BUFFER_SIZE];
	char 	*bufp;
} buf_fd_t;

/*
 * Socket flow IO with buffer.
 */
void buf_init(int fd, buf_fd_t *bp);
ssize_t buf_read(buf_fd_t *bp, char *buf, size_t bytes);
ssize_t buf_readline(buf_fd_t *bp, char *buf, size_t bytes);
ssize_t buf_readn(buf_fd_t *bp, char *buf, size_t bytes);

pid_t Fork(void);
void *Malloc(size_t size);
Sigfunc *Signal(int signo, Sigfunc *func);
struct hostent *Gethostbyname(const char *name);

/*
 * Warpper function of socket functions.
 */
int Socket(int family, int type, int protocol);
int Connect(int sockfd, const SA *servaddr, socklen_t addrlen);
int Bind(int sockfd, const SA *servaddr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Accept(int sockfd, SA *cliaddr, socklen_t *addrlen);


void err_msg(const char *, ...);
void err_exit(const char *, ...);
void err_ret(const char *, ...);
void err_sys(const char *, ...);

#endif // __NP_UTIL_H_
