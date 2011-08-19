include makefile.common

all:
	$(MAKE) -C lib
	$(MAKE) -C examples

clean:
	$(MAKE) clean -C lib
	$(MAKE) clean -C examples
	rm -f include/*~
	rm -rf doc/dox/*

install:
	$(MAKE) install -C lib

uninstall:
	$(MAKE) uninstall -C lib

install-devel:
	$(MAKE) install-devel -C lib

uninstall-devel:
	$(MAKE) uninstall-devel -C lib

rebuild: clean all
