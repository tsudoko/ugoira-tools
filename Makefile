PREFIX ?= /usr/local

install:
	@cd ugoira-dl; make install
	@cd ugoira2mkv; make install

uninstall:
	@cd ugoira-dl; make uninstall
	@cd ugoira2mkv; make uninstall
