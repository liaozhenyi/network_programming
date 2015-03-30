#ifndef __UTIL_H_
#define __UTIL_H_

#include <sys/socket.h>		// struct sockaddr
#include <sys/types.h>
#include <netdb.h>

#define LISTENQ		1024

typedef struct sockaddr SA;
typedef void (handler_t)(int);
int open_clientfd(char *hostname, int port);
int open_listenfd(int port);

int Socket(int domain, int type, int protocol);
int Connect(int sockfd, struct sockaddr *addr, int addrlen);
int Bind(int sockfd, struct sockaddr *addr, int addrlen);
int Listen(int sockfd, int backlog);
int Accept(int listenfd, struct sockaddr *addr, socklen_t *addrlen);

int Getaddrinfo(const char *hostname, const char *service, \
		const struct addrinfo *hints, struct addrinfo **res);

int Pipe(int filedes[2]);
int Fork(void);

handler_t *Signal(int signum, handler_t *handler);

#endif	// __UTIL_H_
