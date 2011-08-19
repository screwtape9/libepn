include ../makefile.common

DBG_OBJ=../src/debug/*.o
DBG_STATIC_LIB=libepn.$(MAJOR_VERSION).$(MINOR_VERSION).$(PATCH_LEVEL).a

REL_OBJ=../src/release/*.o
REL_SHARED_LIB=libepn.so.$(MAJOR_VERSION).$(MINOR_VERSION).$(PATCH_LEVEL)

all: $(DBG_STATIC_LIB) $(REL_SHARED_LIB)

$(DBG_STATIC_LIB):
	$(MAKE) -C ../src
	$(AR) rvu $@ $(DBG_OBJ)

$(REL_SHARED_LIB):
	$(MAKE) -C ../src
	$(CC) -shared -Wl,-soname,libepn.so.$(MAJOR_VERSION) -o $@ $(REL_OBJ) -lc
	ln -s libepn.so.$(MAJOR_VERSION).$(MINOR_VERSION).$(PATCH_LEVEL) libepn.so.$(MAJOR_VERSION).$(MINOR_VERSION)
	ln -s libepn.so.$(MAJOR_VERSION).$(MINOR_VERSION) libepn.so.$(MAJOR_VERSION)
	ln -s libepn.so.$(MAJOR_VERSION) libepn.so

clean:
	$(MAKE) clean -C ../src
	rm -f libepn.*

install: installdirs
	cp -a libepn.so* $(DESTDIR)/usr/lib

install-devel: installdirs-devel
	cp $(DBG_STATIC_LIB) $(DESTDIR)/usr/lib
	cp ../include/*.h $(DESTDIR)/usr/include/epn

installdirs: ../mkinstalldirs
	../mkinstalldirs $(DESTDIR)/usr/lib

installdirs-devel: ../mkinstalldirs
	../mkinstalldirs $(DESTDIR)/usr/lib $(DESTDIR)/usr/include/epn

uninstall:
	rm -f $(DESTDIR)/usr/lib/libepn.so*

uninstall-devel:
	rm -f $(DESTDIR)/usr/lib/$(DBG_STATIC_LIB)
	rm -rf $(DESTDIR)/usr/include/epn

rebuild: clean all
