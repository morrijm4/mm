CC = gcc
CFLAGS = -g -Wall -Werror -o mm
SRC = main.c

all: $(SRC) string.c
	$(CC) $(CFLAGS) $(SRC)
