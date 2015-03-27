#include <stdlib.h>
#include <sys/socket.h>		// SOCK_STREAM
#include <sys/types.h>
#include <netinet/in.h>		// struct sockaddr_in, AF_INET
#include <arpa/inet.h>		// inet_addr()
#include <netdb.h>		// getaddrinfo()
#include <string.h>

#include "socket.h"
#include "error.h"
//void err_sys(const char *fmt, ...);

/*
 * Open a socket and make a connection to the <hostname, port>
 */
int open_clientfd(char *hostname, int port) {
	struct sockaddr_in serveraddr;
	int socketfd;
	struct in_addr ipaddr;
	struct addrinfo *res = NULL;
	
	if (inet_aton(hostname, &ipaddr)) {
		;
	} else {
		Getaddrinfo(hostname, NULL, NULL, &res);
		ipaddr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
	}

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons((unsigned short)port);
	serveraddr.sin_addr = ipaddr;

	socketfd = Socket(AF_INET, SOCK_STREAM, 0);
	Connect(socketfd, (SA *)&serveraddr, sizeof(serveraddr));

	free(res);
	return socketfd;
}

/*
 * Open a socket, bind <ip, port> to that socket, 
 * and listen request on the port
 */
int open_listenfd(int port) {
	struct sockaddr_in serveraddr;
	int socketfd;

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons((unsigned short)port);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	socketfd = Socket(AF_INET, SOCK_STREAM, 0);
	Bind(socketfd, (SA *)&serveraddr, sizeof(serveraddr));
	Listen(socketfd, LISTENQ);
	
	return socketfd;
}

int Socket(int domain, int type, int protocol) {
	int sockfd = socket(domain, type, protocol);

	if (sockfd < 0)
		err_sys("socket() error!");
	return sockfd;
}

int Connect(int sockfd, struct sockaddr *addr, int addrlen) {
	int ret = connect(sockfd, addr, addrlen);

	if (ret < 0)
		err_sys("socket connect() error!");
	return ret;
}

int Bind(int sockfd, struct sockaddr *addr, int addrlen) {
	int ret = bind(sockfd, addr, addrlen);

	if (ret < 0)
		err_sys("socket bind() error!");
	return ret;
}

int Listen(int sockfd, int backlog) {
	int ret = listen(sockfd, backlog);

	if (ret < 0)
		err_sys("socket listen() error");
	return ret;
}

int Accept(int listenfd, struct sockaddr *addr, socklen_t *addrlen) {
	int ret = accept(listenfd, addr, addrlen);

	if (ret < 0)
		err_sys("socket accept() error");
	return ret;
}

int Getaddrinfo(const char *hostname, const char *service, \
		const struct addrinfo *hints, struct addrinfo **res) {
	int ret = getaddrinfo(hostname, service, hints, res);

	if (ret < 0)
		err_sys("getaddrinfo() error");
	return ret;
}
