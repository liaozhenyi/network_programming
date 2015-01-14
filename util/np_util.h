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
 * Without buffer.
 */
ssize_t readn(int fd, char *buf, size_t bytes);
ssize_t writen(int fd, char *buf, size_t bytes);
ssize_t readline(int fd, char *buf, size_t bytes);

typedef struct {
	int 	fd;
	size_t	cnt;
	char	buf[BUFFER_SIZE];
	char *	bufp;
} rio_t;

/*
 * With buffer.
 */
void rio_create(int fd, rio_t *rp);
ssize_t rio_read(rio_t *rp, char *buf, size_t bytes);
ssize_t rio_readline(rio_t *rp, char *buf, size_t bytes);

pid_t Fork(void);

struct hostent *Gethostbyname(const char *name);

/*
 * Warpper function of socket functions.
 */
int Socket(int family, int type, int protocol);
int Connect(int sockfd, const SA *servaddr, socklen_t addrlen);
int Bind(int sockfd, const SA *servaddr, socklen_t addrlen);
int Listen(int sockfd, int backlog);
int Accept(int sockfd, SA *cliaddr, socklen_t *addrlen);

// TODO: add warpper functions + Signal()

void err_msg(const char *, ...);
void err_exit(const char *, ...);
void err_ret(const char *, ...);
void err_sys(const char *, ...);

#endif // __NP_UTIL_H_
