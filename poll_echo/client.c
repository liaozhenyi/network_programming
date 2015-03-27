#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <poll.h>
#include <sys/time.h>

#include "../util/np_util.h"

void echo_cli(int connfd);

int main(int argc, char *argv[])
{
	int sockfd;
	int portno;
	struct sockaddr_in serveraddr;
	struct hostent *host;

	portno = atoi(argv[2]);
	host = Gethostbyname(argv[1]);

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(portno);
	bcopy((char *)(host->h_addr), \
		(char *)&(serveraddr.sin_addr.s_addr), \
		host->h_length);

	Connect(sockfd, (SA *)&serveraddr, sizeof(serveraddr));

	echo_cli(sockfd);

	return 0;
}

void echo_cli(int connfd)
{
	char buf[MAX_LINE];
	fd_set rset, allset;
	rio_t rio;
	int stdiofd, maxfd;
	
	stdiofd = 0;
	maxfd = connfd + 1;
	
	FD_ZERO(&allset);
	FD_SET(connfd, &allset);
	FD_SET(stdiofd, &allset);

	rio_create(connfd, &rio);
	while (1) {
		rset = allset;
		select(maxfd, &rset, NULL, NULL, NULL);
		// check whether we had data unread from the stdio
		if (FD_ISSET(stdiofd, &rset)) {
			fgets(buf, MAX_LINE, stdin);
			writen(connfd, buf, strlen(buf));
		}
		// check whether we had data unread from the socket
		if (FD_ISSET(connfd, &rset)) {
			rio_readline(&rio, buf, MAX_LINE);
			fputs(buf, stdout);
		}
	}
	return ;
}
