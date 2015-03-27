#include <unistd.h>		// read()
#include <errno.h>		// EINTR
#include <assert.h>		// assert()
#include <string.h>		// memcpy()

#include "socketio.h"

#define getMin(a, b)	((a) > (b) ? (b) : (a))

ssize_t readn(int fd, char *buf, size_t size) {
	char *bufp = buf;
	ssize_t nread;
	size_t left = size;

	while (left) {
		nread = read(fd, bufp, left);
		if (nread < 0) {
			if (errno == EINTR)
				continue;
			else
				return -1;
		} else if (!nread) {
			break;
		}
		left -= nread;
		bufp += nread;
	}

	return size-left;
}

ssize_t writen(int fd, char *buf, size_t size) {
	char *bufp = buf;
	ssize_t nwrite;
	size_t left = size;

	while (left) {
		nwrite = write(fd, bufp, left);
		if (nwrite < 0) {
			if (errno == EINTR)
				continue;
			else
				return -1;
		} else if (!nwrite) {
			break;
		}
		left -= nwrite;
		bufp += nwrite;
	}

	return size-left;
}

void buf_init(int fd, buf_fd_t *bp) {
	assert(bp);

	bp->fd = fd;
	bp->cnt = 0;
	bp->bufp = bp->buf;
}

ssize_t buf_read(buf_fd_t *bp, char *buf, size_t size) {
	while (!bp->cnt) {
		ssize_t cnt = read(bp->fd, bp->buf, BUFFER_SIZE);
		if (cnt < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		} else if (!cnt) {
			return 0;
		}
		bp->bufp = bp->buf;
		bp->cnt = cnt;
	}

	size_t readSize = getMin(size, bp->cnt);
	memcpy(buf, bp->bufp, readSize);
	bp->cnt -= readSize;
	bp->bufp += readSize;
	return readSize;
}

/*
 *  Return value: size of read bytes, including the '\0'
 */
ssize_t buf_readline(buf_fd_t *bp, char *buf, size_t size) {
	char *bufp = buf;
	char c;

	for (int i = 1; i < size; i++) {
		ssize_t cnt = buf_read(bp, &c, 1);
		if (cnt < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		} else if (!cnt) {
			if (i == 1)
				return 0;
			break;
		}
		*bufp++ = c;
		if (c == '\n')
			break;
	}
	*bufp = '\0';

	return bufp-buf+1;
}

ssize_t buf_readn(buf_fd_t *bp, char *buf, size_t size) {
	char *bufp = buf;
	ssize_t nread;
	size_t left = size;

	while (left) {
		nread = buf_read(bp, bufp, left);
		if (nread < 0) {
			if (errno == EINTR)
				continue;
			else
				return -1;
		} else if (!nread) {
			break;
		}
		bufp += nread;
		left -= nread;
	}

	return size-left;
}
