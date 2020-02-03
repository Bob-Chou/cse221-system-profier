CC = gcc
CFLAGS = -Wall -g

OS = $(shell uname -s)

ifeq ($(OS), Linux)
    CFLAGS += -pthread
else
    $(info $(OS))
endif

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,%.o,$(SRC))

cpu:$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -f *.o cpu
