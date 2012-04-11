include ../../makefile.common

CFLAGS=-Wall -g
INCLUDES=-I../../include
LIBS=-lrt -lpthread
LIBEPN=../../lib/libepn.1.2.1.a

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

OBJ=test.o driver.o logit.o $(LIBEPN)

all: test driver echo

test: test.o
	$(CC) -o $@ test.o $(LIBEPN) $(LIBS)

driver: driver.o logit.o
	$(CC) -o $@ driver.o logit.o $(LIBEPN) $(LIBS)

echo: echo.o
	$(CC) -o $@ echo.o $(LIBEPN) $(LIBS)

clean:
	rm -f *.o *~ test driver echo

rebuild: clean all
