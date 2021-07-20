C=gcc
CFLAGS=-Isrc -std=c99 -Wall -Wextra -pedantic -Ofast -lm -lSDL2 -pthread
PROJNAME=squeaker

all: bin/$(PROJNAME)
	rm bin/*.o

bin/%.o: src/%.c
	mkdir -p bin
	$(CC) -c $< -o $@ $(CFLAGS)

bin/$(PROJNAME): bin/$(PROJNAME).o
	$(CC) $^ -o $@ $(CFLAGS) 

clean:
	rm -rf bin/*
