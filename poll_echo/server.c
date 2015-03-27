/*
 * Service each handler with IO Multiplexing(poll).
 * int poll(struct pollfd *fds, nfds_t nfds, int timeout);
 * On success, select() return the number of file descriptors contained 
 * in three returned descriptor sets.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/select.h>
#include <sys/time.h>

#include "../util/np_util.h"

struct clientfd {
	int fd;
	rio_t *rp;
};

int main(int argc, char *argv[])
{
	int i;
	int sockfd, acceptfd;
	int portno;
	struct clientfd clientfd[BACKLOG];
	int maxfd, maxi, readynum;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t addrlen;
	fd_set rset, allset;
	ssize_t nread;
	char buf[MAX_LINE];

	portno = atoi(argv[1]);
	addrlen = sizeof(cliaddr);

	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(portno);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	Bind(sockfd, (SA *)&servaddr, sizeof(servaddr));
	Listen(sockfd, BACKLOG);

	for (i = 0; i < BACKLOG; i++) {
		clientfd[i].fd = -1;
		clientfd[i].rp = NULL;
	}
	FD_ZERO(&allset);
	FD_SET(sockfd, &allset);
	maxfd = sockfd + 1;
	maxi = -1;

	while (1) {
		rset = allset;
		readynum = select(maxfd, &rset, NULL, NULL, NULL);
		if (FD_ISSET(sockfd, &rset)) {
			// in this situation, accept will not be blocked
			acceptfd = Accept(sockfd, (SA *)&cliaddr, &addrlen);
			for (i = 0; i < BACKLOG; i++)
				if (clientfd[i].fd < 0) {
					clientfd[i].fd = acceptfd;
					clientfd[i].rp = (rio_t *)Malloc(sizeof(rio_t));
					rio_create(clientfd[i].fd, clientfd[i].rp);
					break;
				}

			if (i == BACKLOG)
				err_exit("too many clients");

			FD_SET(acceptfd, &allset);
			maxfd = (maxfd > acceptfd+1 ? maxfd : acceptfd+1);

			if (i > maxi)
				maxi = i;

			if (--readynum < 1)
				continue;
		}
		for (i = 0; i <= maxi; i++) {
			if (clientfd[i].fd > 0 && FD_ISSET(clientfd[i].fd, &rset)) {
				nread = rio_readline(clientfd[i].rp, buf, MAX_LINE);
				// client connection was closed.
				if (nread == 0) {
					close(clientfd[i].fd);
					FD_CLR(clientfd[i].fd, &allset);
					free(clientfd[i].rp);
					clientfd[i].fd = -1;
				} else if (nread < 0) {
					err_sys("error rio_readline");
				} else {
					writen(clientfd[i].fd, buf, nread);
				}
			
				if (--readynum < 1)
					break;
			}
		}
	}

	return 0;
}
