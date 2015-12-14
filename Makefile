CFLAGS=-c -Wall -g -DNDEBUG
CC = gcc

all: 8puzzle

dijkstra: 8puzzle.o 
	$(CC) -o 8puzzle 8puzzle.o


dijkstra.o: 8puzzle.c  dbg.h 
	$(CC) $(CFLAGS) 8puzzle.c


clean:
	rm -f 8puzzle



