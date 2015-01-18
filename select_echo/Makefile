UTIL_SRC = ../util/np_uti.c
ECHO_SRC = client.c server.c
CFLAGS = -g -Wall

UTIL_OBJ = ../util/np_util.o
CLIENT_OBJ = client.o
SERVER_OBJ = server.o

all: client server

client: $(CLIENT_OBJ) $(UTIL_OBJ)
	cc -o client $(CFLAGS) $(CLIENT_OBJ) $(UTIL_OBJ)

server: $(SERVER_OBJ) $(UTIL_OBJ)
	cc -o server $(CFLAGS) $(SERVER_OBJ) $(UTIL_OBJ)

.PHONY: clean
clean:
	rm -f *.o *.swp *.~ client server
