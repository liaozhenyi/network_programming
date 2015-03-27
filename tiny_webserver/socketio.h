#ifndef __SOCKETIO_H_
#define __SOCKETIO_H_

#include <sys/types.h>

#define BUFFER_SIZE 4096
#define LINE_SIZE 1024

typedef struct {
	int fd;
	size_t cnt;
	char buf[BUFFER_SIZE];
	char *bufp;
} buf_fd_t;

ssize_t readn(int fd, char *buf, size_t size);
ssize_t writen(int fd, char *buf, size_t size);

void buf_init(int fd, buf_fd_t *bp);
ssize_t buf_read(buf_fd_t *bp, char *buf, size_t size);
ssize_t buf_readline(buf_fd_t *bp, char *buf, size_t size);
ssize_t buf_readn(buf_fd_t *bp, char *buf, size_t size);

#endif // __SOCKETIO_H_
