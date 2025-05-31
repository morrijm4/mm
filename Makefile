CC = gcc
CFLAGS = -g -Wall -Werror -o mm
SRC = main.c

all:
	$(CC) $(CFLAGS) $(SRC)
