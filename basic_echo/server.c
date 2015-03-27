#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../util/np_util.h"

void echo_serv(int connfd);
void sig_chld(int signo);

int main(int argc, char *argv[])
{
	int sockfd, acceptfd;
	int portno;
	pid_t pid;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t addrlen;

	portno = atoi(argv[1]);
	addrlen = sizeof(cliaddr);
	// TODO: handler zombie processes, write wrapper function of signal()

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(portno);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	Bind(sockfd, (SA *)&servaddr, sizeof(servaddr));
	Listen(sockfd, BACKLOG);
	Signal(SIGCHLD, sig_chld);

	while (1) {
		acceptfd = accept(sockfd, (SA *)&cliaddr, &addrlen);
		if (acceptfd < 0) {
			if (errno == EINTR)
				continue;
			else
				err_sys("accept error");
		}
		pid = Fork();
		if (pid == 0) {
			close(sockfd);
			echo_serv(acceptfd);
			exit(0);
		}
		close(acceptfd);
	}

	return 0;
}

void echo_serv(int connfd)
{
	ssize_t nread;
	buf_fd_t bf;
	char buf[MAX_LINE];

	bzero(buf, sizeof(buf));
	buf_init(connfd, &bf);
	while ((nread = buf_readline(&bf, buf, MAX_LINE))) {
		if (nread < 0) {
			err_msg("error read from socket");
			break;
		}
		writen(connfd, buf, nread);
	}

	return ;
}

void sig_chld(int signo)
{
	pid_t pid;
	int stat;

	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child %d terminated\n", pid);
	return ;
}
