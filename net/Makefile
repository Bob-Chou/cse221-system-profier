CC = gcc
CFLAGS = -Wall -g -std=c99

OS = $(shell uname -s)

ifeq ($(OS), Linux)
    CFLAGS += -pthread
else
    $(info $(OS))
endif

DIR = ../tools
SRC = $(foreach dir,$(DIR),$(wildcard $(dir)/*.c))
EXT = $(patsubst %.c,%.o,$(SRC))
OBJ0 = $(EXT) tcp.o
OBJ1 = $(EXT) server.o
OBJ2 = $(EXT) client.o
INC=$(foreach dir,$(DIR),-I$(dir))

all: server tcp client

tcp: $(OBJ0)
	$(CC) $(CFLAGS) $(notdir $^) -o $@

server: $(OBJ1)
	$(CC) $(CFLAGS) $(notdir $^) -o $@

client: $(OBJ2)
	$(CC) $(CFLAGS) $(notdir $^) -o $@

$(EXT) server.o client.o tcp.o:%.o:%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $(notdir $@)


.PHONY:clean
clean:
	rm -f *.o server tcp client
