UTIL_SRC = ../util/np_util.c
TEST_SRC = test_util_io.c
CFLAGS = -g -Wall

UTIL_OBJ = ../util/np_util.o
TEST_OBJ = test_util_io.o

all: test_util_io

test_util_io: $(TEST_OBJ) $(UTIL_OBJ)
	gcc -o test_util_io $(CFLAGS) $(TEST_OBJ) $(UTIL_OBJ)

.PHONY: clean
clean:
	rm -f *.o *.swp *.~ util_io
