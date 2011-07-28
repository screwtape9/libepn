include ../../makefile.common

CFLAGS=-Wall -g
INCLUDES=-I../../include
LIBS=-lrt -lpthread -L../../lib -lepn

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $<

OBJ=test.o driver.o logit.o

all: test driver

test: test.o
	$(CC) -o $@ test.o $(LIBS)

driver: driver.o logit.o
	$(CC) -o $@ driver.o logit.o $(LIBS)

clean:
	rm -f *.o *~ test driver
