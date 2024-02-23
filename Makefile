.POSIX:
.SUFFIXES:

CC=gcc
CFLAGS=-g -Wall -Werror -Wextra
LDLIBS=-lncursesw
OUTPUT=minesweeper

$(OUTPUT): minesweeper.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

minesweeper.o: minesweeper.c minesweeper.h
	$(CC) -c $(CFLAGS) -o $@ $< 

.PHONY: clean bear
clean:
	rm -f minesweeper.o $(OUTPUT)

bear:
	make clean; bear -- make
