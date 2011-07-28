include makefile.common

all:
	$(MAKE) -C lib
	$(MAKE) -C examples

clean:
	$(MAKE) clean -C lib
	$(MAKE) clean -C examples
	rm -f include/*~
	rm -rf doc/dox/*

rebuild: clean all
