CC = gcc
CFLAGS = -Wall -g

OS = $(shell uname -s)

ifeq ($(OS), Linux)
    CFLAGS += -pthread
else
    $(info $(OS))
endif

DIR = ../tools
SRC = $(foreach dir,$(DIR),$(wildcard $(dir)/*.c))
EXT = $(patsubst %.c,%.o,$(SRC))
OBJ1 = $(EXT) server.o
OBJ2 = $(EXT) client.o
INC=$(foreach dir,$(DIR),-I$(dir))

all: server client

server: $(OBJ1)
	$(CC) $(CFLAGS) $(notdir $^) -o $@

client: $(OBJ2)
	$(CC) $(CFLAGS) $(notdir $^) -o $@

$(EXT) server.o client.o:%.o:%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $(notdir $@)


.PHONY:clean
clean:
	rm -f *.o server client
