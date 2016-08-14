PREFIX ?= /usr/local

FILES = ugoira2mkv ugoira-dl ugoira-merge

install:
	@for i in $(FILES); do make -C $$i install; done

uninstall:
	@for i in $(FILES); do make -C $$i uninstall; done