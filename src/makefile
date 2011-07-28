include ../makefile.common

CFLAGS=-c -Wall -fPIC
INCLUDES=-I../include

DBG_OBJDIR:=debug
DBG_OBJ:=$(addprefix $(DBG_OBJDIR)/,epn_cfg.o epn_svr.o net.o list.o queue.o rb.o)

REL_OBJDIR:=release
REL_OBJ:=$(addprefix $(REL_OBJDIR)/,epn_cfg.o epn_svr.o net.o list.o queue.o rb.o)

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
