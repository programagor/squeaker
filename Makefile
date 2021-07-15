C=gcc
CFLAGS=-Isrc -std=c99 -Wall -Wextra -pedantic -Ofast -lm -lSDL
PROJNAME=squeaker

all: bin/$(PROJNAME)
	rm bin/*.o

bin/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

bin/$(PROJNAME): bin/$(PROJNAME).o
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf bin/*
