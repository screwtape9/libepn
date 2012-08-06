include ../makefile.common

ifeq ($(USE_GLIB),1)
CFLAGS=-c -Wall -fPIC -DUSE_GLIB=1 -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -DPATCH_LEVEL=$(PATCH_LEVEL)
ifeq ($(ARCH),x86_64)
INCLUDES=-I../include -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include
else
INCLUDES=-I../include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
endif

DBG_OBJDIR:=debug
DBG_OBJ:=$(addprefix $(DBG_OBJDIR)/,epn_cfg.o epn_svr.o epn_clt.o epn_err.o net.o)

REL_OBJDIR:=release
REL_OBJ:=$(addprefix $(REL_OBJDIR)/,epn_cfg.o epn_svr.o epn_clt.o epn_err.o net.o)
else
CFLAGS=-c -Wall -fPIC -DUSE_GLIB=0 -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -DPATCH_LEVEL=$(PATCH_LEVEL)
INCLUDES=-I../include

DBG_OBJDIR:=debug
DBG_OBJ:=$(addprefix $(DBG_OBJDIR)/,epn_cfg.o epn_svr.o epn_clt.o epn_err.o net.o list.o queue.o rb.o)

REL_OBJDIR:=release
REL_OBJ:=$(addprefix $(REL_OBJDIR)/,epn_cfg.o epn_svr.o epn_clt.o epn_err.o net.o list.o queue.o rb.o)
endif

$(DBG_OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -g $(INCLUDES) -o $@ $<

$(REL_OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

all: $(DBG_OBJ) $(REL_OBJ)

$(DBG_OBJ): | $(DBG_OBJDIR)

$(REL_OBJ): | $(REL_OBJDIR)

$(DBG_OBJDIR):
	mkdir $(DBG_OBJDIR)

$(REL_OBJDIR):
	mkdir $(REL_OBJDIR)

clean:
	rm -f $(DBG_OBJDIR)/* $(REL_OBJDIR)/* makefile.dep

rebuild: clean all

makefile.dep: ../include/*.[h] *.[ch]
	for i in *.[c]; do $(CC) $(INCLUDES) -MM "$${i}"; done > $@
include makefile.dep
