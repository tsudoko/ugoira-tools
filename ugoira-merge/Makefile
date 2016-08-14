PREFIX ?= /usr/local

FILES = ugoira-merge ugoira-merge-batch

install:
	cp $(FILES) $(PREFIX)/bin

uninstall:
	for i in $(FILES); do rm $(PREFIX)/bin/$$i; done
