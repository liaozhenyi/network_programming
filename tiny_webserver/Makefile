CC = gcc
CFLAGS = -Wall -g -I.
SRCS = socket.c error.c socketio.c httpd.c
OBJS = $(SOURCES:.c=.o)
INCLUDE = error.h

all: httpd

httpd: $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f *.o *~ httpd
