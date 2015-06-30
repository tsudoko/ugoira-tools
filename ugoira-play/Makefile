CC = clang
CFLAGS = -Weverything -Wno-padded -Wno-documentation
#CFLAGS = -Wall -Wextra -Wshadow -pedantic
CFLAGS += -g
CFLAGS += -fsanitize=address

CFLAGS += -I/usr/include/SDL2
LDFLAGS += -larchive  -lSDL2_image  -lSDL2 -lpthread  -lm

SRC = list.c archive.c main.c
OBJ = $(SRC:.c=.o)

PREFIX ?= /usr/local

ugoira-play: $(OBJ)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJ) -o ugoira-play

obj: $(OBJ)

clean:
	rm $(OBJ) ugoira-play

.PHONY: clean
