include ../../makefile.common

CFLAGS=-Wall -g
INCLUDES=-I../../include
LIBS=-lrt -lpthread

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

OBJ=test.o driver.o logit.o ../../lib/libepn.1.0.0.a

all: test driver

test: test.o
	$(CC) -o $@ test.o ../../lib/libepn.1.0.0.a $(LIBS)

driver: driver.o logit.o
	$(CC) -o $@ driver.o logit.o ../../lib/libepn.1.0.0.a $(LIBS)

clean:
	rm -f *.o *~ test driver
