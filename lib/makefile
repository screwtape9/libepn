include ../makefile.common

DBG_OBJDIR:=debug
DBG_OBJ=../src/debug/*.o
DBG_STATIC_LIB=$(addprefix $(DBG_OBJDIR)/,libepnd.1.0.0.a)
DBG_SHARED_LIB=$(addprefix $(DBG_OBJDIR)/,libepnd.so.1.0.0)

REL_OBJDIR:=release
REL_OBJ=../src/release/*.o
REL_STATIC_LIB=$(addprefix $(REL_OBJDIR)/,libepn.1.0.0.a)
REL_SHARED_LIB=$(addprefix $(REL_OBJDIR)/,libepn.so.1.0.0)

all: $(DBG_STATIC_LIB) $(DBG_SHARED_LIB) $(REL_STATIC_LIB) $(REL_SHARED_LIB)

$(DBG_STATIC_LIB): | $(DBG_OBJDIR)
	$(MAKE) -C ../src
	$(AR) rvu $@ $(DBG_OBJ)

$(DBG_SHARED_LIB): | $(DBG_OBJDIR)
	$(CC) -shared -Wl,-soname,libepnd.so.1 -o $@  $(DBG_OBJ) -lc
#	ln -s libepnd.so.1.0.0 libepnd.so.1.0
#	ln -s libepnd.so.1.0 libepnd.so.1
#	ln -s libepnd.so.1 libepnd.so

$(REL_STATIC_LIB): | $(REL_OBJDIR)
	$(MAKE) -C ../src
	$(AR) rvu $@ $(REL_OBJ)

$(REL_SHARED_LIB): | $(REL_OBJDIR)
	$(CC) -shared -Wl,-soname,libepn.so.1 -o $@  $(REL_OBJ) -lc
#	ln -s $(REL_OBJ)/libepn.so.1.0.0 libepn.so.1.0
#	ln -s $(REL_OBJ)/libepn.so.1.0 libepn.so.1
#	ln -s $(REL_OBJ)/libepn.so.1 libepn.so

$(DBG_OBJDIR):
	mkdir $(DBG_OBJDIR)

$(REL_OBJDIR):
	mkdir $(REL_OBJDIR)

clean:
	$(MAKE) clean -C ../src
	rm -f $(DBG_OBJDIR)/* $(REL_OBJDIR)/* *.so*

rebuild: clean all
