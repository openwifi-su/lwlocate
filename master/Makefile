DBGFLAGS = -O2 -g0 -DNDEBUG
STRIP=strip
ifeq ($(DEBUG),1)
DBGFLAGS = -O0 -g3
STRIP=ls -al
endif

CC      = gcc
CFLAGS  = -Wall $(DBGFLAGS) -DENV_LINUX -D_REENTRANT -DNDEBUG -L/usr/lib -L/usr/local/lib/ -I.

LDFLAGS  = -L. -lwlocate -lm

PNAME=lwtrace

OBJ = trace.o
SRC = trace.c

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(PNAME) $(OBJ) $(LDFLAGS)
	$(STRIP) $(PNAME)

clean:
	rm -f *.o
	rm -f $(PNAME)
