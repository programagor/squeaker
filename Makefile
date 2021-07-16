C=gcc
CFLAGS=-Isrc -std=c99 -Wall -Wextra -pedantic -Ofast -lm -lSDL2 -pthread
PROJNAME=squeaker

all: bin/$(PROJNAME)
	rm bin/*.o

bin/%.o: src/%.c
	mkdir -p bin
	$(CC) $(CFLAGS) -c $< -o $@

bin/$(PROJNAME): bin/$(PROJNAME).o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf bin/*
