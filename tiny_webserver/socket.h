#ifndef __SOCKET_H_
#define __SOCKET_H_

#include <sys/socket.h>		// struct sockaddr
#include <sys/types.h>
#include <netdb.h>

#define LISTENQ		1024

typedef struct sockaddr SA;
int open_clientfd(char *hostname, int port);
int open_listenfd(int port);

int Socket(int domain, int type, int protocol);
int Connect(int sockfd, struct sockaddr *addr, int addrlen);
int Bind(int sockfd, struct sockaddr *addr, int addrlen);
int Listen(int sockfd, int backlog);
int Accept(int listenfd, struct sockaddr *addr, socklen_t *addrlen);

int Getaddrinfo(const char *hostname, const char *service, \
		const struct addrinfo *hints, struct addrinfo **res);

#endif	// __SOCKET_H_
