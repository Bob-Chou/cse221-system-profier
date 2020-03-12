TARGET = mem

CC = gcc
CFLAGS = -Wall -g -std=gnu99

OS = $(shell uname -s)

ifeq ($(OS), Linux)
    CFLAGS += -pthread
else
    $(info $(OS))
endif

DIR= ../tools .
SRC=$(foreach dir,$(DIR),$(wildcard $(dir)/*.c))
OBJ=$(patsubst %.c,%.o,$(SRC))
INC=$(foreach dir,$(DIR),-I$(dir))

$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) $(notdir $^) -o $@

$(OBJ):%.o:%.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $(notdir $@)

.PHONY:clean
clean:
	rm -f *.o $(TARGET)

.PHONY:cleanall
cleanall:
	rm -f *.o $(TARGET) swap
