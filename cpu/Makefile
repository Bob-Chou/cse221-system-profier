.PHONY:clean

CC = gcc
CFLAGS = -Wall -g

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,%.o,$(SRC))

cpu:$(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o cpu
