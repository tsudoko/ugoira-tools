CC = clang
CFLAGS = -Wall -Wextra -pedantic

CFLAGS += -I/usr/include/SDL2
LDFLAGS += -lSDL2 -lpthread

SRC = main.c
OBJ = $(SRC:.c=.o)

PREFIX ?= /usr/local

ugoira-play: $(OBJ)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJ) -o ugoira-play

obj: $(OBJ)
#	$(CC) -lSDL -lpthread $(CFLAGS) $(OBJ) -o $@

clean:
	rm $(OBJ) ugoira-play

.PHONY: clean
