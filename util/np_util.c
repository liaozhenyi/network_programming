#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "np_util.h"

/**********************************************************************
  Error Handling functions.
**********************************************************************/
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

void *Malloc(size_t size)
{
	void *mem;

	mem = malloc(size);
	if (mem == NULL)
		err_sys("malloc error");
	return mem;
}

Sigfunc *Signal(int signo, Sigfunc *func)
{
	Sigfunc *handler;

	if ((handler = signal(signo, func)) == SIG_ERR)
		err_sys("error signal");
	return handler;
}

struct hostent *Gethostbyname(const char *name)
{
	struct hostent *host;

	host = gethostbyname(name);
	if (host == NULL)
		err_exit("gethostbyname failed");
	return host;
}


/**********************************************************************
  Wrapper functions for socket handling.
**********************************************************************/
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


/**********************************************************************
  Wrapper function for socket IO, they mainly offer two functionality:
   1. handling "short cut" and signal interrupt properly;
   2. add an application-buffer for efficient socket IO.
**********************************************************************/
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
			return -1;
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

void buf_init(int fd, buf_fd_t *bp)
{
	bp->fd = fd;
	bp->cnt = 0;
	bp->bufp = bp->buf;
	bzero(bp->buf, sizeof(bp->buf));
}

ssize_t buf_read(buf_fd_t *bp, char *buf, size_t bytes)
{
	while (bp->cnt == 0) {
		ssize_t cnt = read(bp->fd, bp->buf, BUFFER_SIZE);
		if (cnt == -1) {
			if (errno == EINTR)
				continue;
			return -1;
		} else if (!cnt) {
			return 0;
		}
		bp->cnt = cnt;
		bp->bufp = bp->buf;
	}

	size_t readBytes = (bp->cnt > bytes ? bytes : bp->cnt);
	bcopy(bp->bufp, buf, readBytes);
	bp->cnt -= readBytes;
	bp->bufp += readBytes;

	return readBytes;
}

ssize_t buf_readline(buf_fd_t *bp, char *buf, size_t bytes)
{
	char c;
	char *bufp = buf;

	for (int i = 1; i < bytes; i++) {
		ssize_t count = buf_read(bp, &c, 1);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		} else if (!count) {
			break;
		}
		*bufp++ = c;
		if (c == '\n')
			break;
	}
	*bufp = '\0';

	return bufp-buf;
}

ssize_t buf_readn(buf_fd_t *bp, char *buf, size_t bytes)
{
	char *bufp = buf;
	ssize_t nread;
	size_t nleft = bytes;
	
	while (nleft) {
		nread = buf_read(bp, bufp, nleft);
		if (nread < 0) {
			if (errno == EINTR)
				continue;
			else
				return -1;
		} else if (!nread) {
			break;
		}
		bufp += nread;
		nleft -= nread;
	}

	return bytes-nleft;
}
