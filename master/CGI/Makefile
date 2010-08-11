CC      = gcc
CFLAGS  = -Wall -g0 -O3 -DENV_LINUX -D_REENTRANT -DOAPC_EXT_API="" -D_DEBUG -L/usr/lib -L/usr/local/lib/ -I.. -I/usr/local/include

LDFLAGS  = -lwlocate -lm

PNAME=geolocation.cgi

OBJ = JSON_parser.o main.o
SRC = JSON_parser.c main.c

all: $(OBJ)
	$(CC) $(CFLAGS) -o $(PNAME) $(OBJ) $(LDFLAGS)
	strip $(PNAME)
	cp $(PNAME) ~/openwlanmap.org/

clean:
	rm *.o
	rm $(PNAME)
