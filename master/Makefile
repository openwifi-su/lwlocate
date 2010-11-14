CC      = gcc
CFLAGS  = -Wall -g0 -O3 -DENV_LINUX -D_REENTRANT -DNDEBUG -L/usr/lib -L/usr/local/lib/ -I.

LDFLAGS  = -L. -lwlocate -lm

PNAME=libtest

OBJ = test.o
SRC = test.c

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(PNAME) $(OBJ) $(LDFLAGS)
	strip $(PNAME)

clean:
	rm *.o
	rm $(PNAME)
