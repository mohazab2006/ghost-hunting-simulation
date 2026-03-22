CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -pthread
LDFLAGS = -pthread

OBJS = main.o helpers.o room.o stack.o casefile.o house.o movement.o hunter.o ghost.o

all: project

project: $(OBJS)
	$(CC) $(CFLAGS) -o project $(OBJS) $(LDFLAGS)

%.o: %.c defs.h helpers.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) project log_*.csv

.PHONY: all clean
