#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "np_util.h"

/*
 * Print a message and return to caller.
 * Caller specifies "errnoflag".
 */
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
	char buf[BUFFER_SIZE];

	vsnprintf(buf, BUFFER_SIZE, fmt, ap);
	if (errnoflag)
		snprintf(buf+strlen(buf), BUFFER_SIZE-strlen(buf), ":%s", strerror(error));
	strcat(buf, "\n");
	fflush(stdout);
	fputs(buf, stderr);
	fflush(NULL);
}

/*
 * Nonfatal error unrelated to a system call.
 * Print a message and return.
 */
void err_msg(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(0, 0, fmt, ap);
	va_end(ap);
}

/*
 * Fatal error unrelated to a system call.
 * Error code passed as explicit parameter.
 * Print a message and terminate.
 */
void err_exit(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}

/*
 * Nonfatal error related to a system call.
 * Print a message and return.
 */
void err_ret(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void err_sys(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	err_doit(1, errno, fmt, ap);
	va_end(ap);
	exit(1);
}


pid_t Fork(void)
{
	pid_t pid;

	pid = fork();
	if (pid < 0)
		err_sys("err fork");
	return pid;
}

struct hostent *Gethostbyname(const char *name)
{
	struct hostent *host;

	host = gethostbyname(name);
	if (host == NULL)
		err_exit("gethostbyname failed");
	return host;
}


int Socket(int family, int type, int protocol)
{
	int sockfd;

	sockfd = socket(family, type, protocol);
	if (sockfd < 0)
		err_sys("socket error");
	return sockfd;
}

int Connect(int sockfd, const SA *servaddr, socklen_t addrlen)
{
	int ret;

	ret = connect(sockfd, servaddr, addrlen);
	if (ret < 0)
		err_sys("connect error");
	return ret;
}

int Bind(int sockfd, const SA *servaddr, socklen_t addrlen)
{
	int ret;

	ret = bind(sockfd, servaddr, addrlen);
	if (ret < 0)
		err_sys("error bind");
	return ret;
}

int Listen(int sockfd, int backlog)
{
	int ret;

	ret = listen(sockfd, backlog);
	if (ret < 0)
		err_sys("error listen");
	return ret;
}

int Accept(int sockfd, SA *cliaddr, socklen_t *addrlen)
{
	int acceptfd;

	acceptfd = accept(sockfd, cliaddr, addrlen);
	if (acceptfd < 0)
		err_sys("error accept");
	return acceptfd;
}

/*
 * deal with the short cut of read() system call.
 */
ssize_t readn(int fd, char *buf, size_t bytes)
{
	size_t nleft = bytes;
	ssize_t nread;
	char *bufp = buf;

	while (nleft) {
		nread = read(fd, bufp, nleft);
		if (nread < 0) {
			// read process was interrupted
			if (errno == EINTR) {
				continue;
			}
			return nread;
		} else if (nread == 0) {
			break;
		}
		nleft -= nread;
		bufp += nread;
	}

	return bytes - nleft;
}

ssize_t writen(int fd, char *buf, size_t bytes)
{
	size_t nleft = bytes;
	ssize_t nwrite;
	char *bufp = buf;

	while (nleft) {
		nwrite = write(fd, bufp, nleft);
		if (nwrite < 0) {
			if (errno == EINTR)
				continue;
			return nwrite;
		} else if (nwrite == 0) {
			break;
		}
		nleft -= nwrite;
		bufp += nwrite;
	}

	return bytes - nleft;
}

ssize_t readline(int fd, char *buf, size_t bytes)
{
	int i;
	ssize_t nread;
	char c, *bufp = buf;

	for (i = 1; i <= bytes; i++) {
		// UNP use read() instead of readn(),
		// With readn() I don't need to check
		// errno.
		nread = readn(fd, &c, 1);
		if (nread < 0) {
			return nread;	
		} else if (nread == 0) {
			*bufp = '\0';
			return i-1;
		}
		*bufp++ = c;
		if (c == '\n')
			break;
	}
	*bufp = '\0';

	return i;
}

void rio_create(int fd, rio_t *rp)
{
	rp->fd = fd;
	rp->cnt = 0;
	rp->bufp = rp->buf;
}

ssize_t rio_read(rio_t *rp, char *buf, size_t bytes)
{
	size_t maxread;
	ssize_t readn;

	while (rp->cnt == 0) {
		// whether to use readn() instead of read()
		// readn doesn't use buffer, so read is good!
		readn = read(rp->fd, rp->buf, BUFFER_SIZE);
		if (readn < 0) {
			//if (errno == EINTR)
			//	continue;
			//else 
			// the short cut handler should leave to the higher level function.
			return -1;
		} else if (readn == 0) {
			break;
		} else {
			rp->cnt = readn;
		}
	}
	maxread = bytes>rp->cnt ? rp->cnt : bytes;
	bcopy(rp->bufp, buf, maxread);
	rp->cnt -= maxread;
	rp->bufp += maxread;

	return maxread;
}

ssize_t rio_readline(rio_t * rp, char *buf, size_t bytes)
{
	int i;
	ssize_t nread;
	char c, *bufp = buf;

	for (i = 1; i <= bytes; i++) {
		nread = rio_read(rp, &c, 1);
		if (nread < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		} else if (nread == 0) {
			*bufp = '\0';
			return i-1;
		}
		*bufp++ = c;
		if (c == '\n')
			break;
	}
	*bufp = '\0';
	return i;
}


