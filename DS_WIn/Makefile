CC=gcc
CFLAGS=-std=c11 -Werror -Wall -Wpedantic -Wextra

all: release debug

release: ./src/loesung-581323.c
	$(CC) $(CFLAGS) -O3 -o ./bin/loesung ./src/loesung-581323.c
    
debug: ./src/loesung-581323.c
	$(CC) $(CFLAGS) -ggdb -o ./bin/loesung-dbg ./src/loesung-581323.c

profile: ./src/loesung-581323.c
	$(CC) $(CFLAGS) -ggdb -pg -o ./bin/loesung-p ./src/loesung-581323.c
