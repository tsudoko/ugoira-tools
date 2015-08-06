include config.mk

CFLAGS += -I/usr/include/SDL2
LDFLAGS += -larchive  -ljansson  -lSDL2_image  -lSDL2 -lpthread  -lm

VPATH = src
BUILDDIR = build

SRC = list.c archive.c frame.c main.c
OBJ = $(SRC:.c=.o)

DESTDIR ?=
PREFIX ?= /usr/local

all: $(BUILDDIR)/ugoira-play

install: $(BUILDDIR)/ugoira-play
	install -m755 $(BUILDDIR)/ugoira-play $(DESTDIR)$(PREFIX)/bin/ugoira-play

uninstall:
	rm $(DESTDIR)$(PREFIX)/bin/ugoira-play

$(BUILDDIR)/ugoira-play: $(OBJ)
	if [ ! -d $(BUILDDIR) ]; then mkdir $(BUILDDIR); fi
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJ) -o "$@"

obj: $(OBJ)

clean:
	rm $(OBJ) $(BUILDDIR)/ugoira-play

.PHONY: all clean install uninstall
