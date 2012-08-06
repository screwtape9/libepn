include ../../makefile.common

ifeq ($(USE_GLIB),1)
CFLAGS=-Wall -g -DUSE_GLIB=1
else
CFLAGS=-Wall -g
endif

LIBEPN=../../lib/libepn.1.3.0.a
ifeq ($(USE_GLIB),1)
ifeq ($(ARCH),x86_64)
INCLUDES=-I../../include -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include
else
INCLUDES=-I../../include -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
endif
LIBS=$(LIBEPN) -lrt -lpthread -lglib-2.0
else
INCLUDES=-I../../include
LIBS=$(LIBEPN) -lrt -lpthread
endif

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

OBJ=test.o driver.o logit.o printsig.o $(LIBEPN)

all: test driver echo

test: test.o printsig.o
	$(CC) -o $@ test.o printsig.o $(LIBS)

driver: driver.o logit.o printsig.o
	$(CC) -o $@ driver.o logit.o printsig.o $(LIBS)

echo: echo.o
	$(CC) -o $@ echo.o $(LIBS)

clean:
	rm -f *.o *~ test driver echo

rebuild: clean all
