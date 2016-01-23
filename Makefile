VERBOUSFLAGS = -O2 -g0
STRIP=strip
ifeq ($(VERBOUS),true)
VERBOUSFLAGS = -O0 -g3
STRIP=ls -al
endif

DEBUGFLAG =
ifeq ($(DEBUG),true)
	DEBUGFLAG = -DDEBUG
endif

SLIMFLAG =
ifeq ($(SLIM),true)
	SLIMFLAG = -DSLIM
endif

#general
CC = gcc
HEADERFLAGS = -I.
CFLAGS = -Wall

#lwtrace specifc
LINKFLAGS = -L/usr/lib -L/usr/local/lib/ -L.
LDFLAGS = -L. -lwlocate -lm
LWTRACE_SOURCES = trace.c
LWTRACE_OBJECTS = $(LWTRACE_SOURCES:.c=.o)
LWTRACE_EXECUTABLE = lwtrace

#libwlocate specifc
LIBWLOCATE_CFLAGS = -fPIC -shared -Wno-unused
LIBWLOCATE_SOURCES = connect.c wlan.c libwlocate.c iwlist.c
LIBWLOCATE_OBJECTS = $(LIBWLOCATE_SOURCES:.c=.o)
LINK = -shared -Wl,--no-as-needed
SYSLIBRARIES= -lm
LIBS = $(SYSLIBRARIES) -liw
LIBWLOCATE_EXECUTABLE = libwlocate.so

all: libwlocate lwtrace

libwlocate:
ifeq ($(TARGET),ENV_LINUX)
	$(CC) $(CFLAGS) $(LIBWLOCATE_CFLAGS) $(VERBOUSFLAGS) $(DEBUGFLAG) $(SLIMFLAG) -D$(TARGET) $(HEADERFLAGS) -c $(LIBWLOCATE_SOURCES)
	$(CC) $(LINK) $(SYSLDFLAGS) -o $(LIBWLOCATE_EXECUTABLE) $(LIBWLOCATE_OBJECTS) $(LIBS)
	$(STRIP) $(LIBWLOCATE_EXECUTABLE)
else
	$(error "TARGET not defined please type make help.")
endif

lwtrace:
ifeq ($(TARGET),ENV_LINUX)
	$(CC) $(CFLAGS) $(VERBOUSFLAGS) $(DEBUGFLAG) $(SLIMFLAG) -D$(TARGET) -c $(LWTRACE_SOURCES)
	$(CC) $(CFLAGS) $(VERBOUSFLAGS) $(DEBUGFLAG) $(SLIMFLAG) -D$(TARGET) $(LINKFLAGS) $(HEADERFLAGS) -o $(LWTRACE_EXECUTABLE) $(LWTRACE_OBJECTS) $(LDFLAGS)
	$(STRIP) $(LWTRACE_EXECUTABLE)
else
	$(error "TARGET not defined please type make help.")
endif

clean:
	rm -f $(LWTRACE_OBJECTS)
	rm -f $(LWTRACE_EXECUTABLE)
	rm -f $(LIBWLOCATE_OBJECTS)
	rm -f $(LIBWLOCATE_EXECUTABLE)

install:
	@echo "Install $(LWTRACE_EXECUTABLE) ..."
	install -m 0755 $(LWTRACE_EXECUTABLE) /usr/local/bin/$(LWTRACE_EXECUTABLE)

uninstall:
	@echo "Removing $(LWTRACE_EXECUTABLE) ..."
	rm -f /usr/local/bin/$(LWTRACE_EXECUTABLE)

help:
	@echo "Usage: make [options]"
	@echo "TARGET=<arch>		ENV_LINUX for Linux and ENV_WINDOWS for Windows"
	@echo "VERBOUS=true		For more compiling output"
	@echo "DEBUG=true		For developer debuging output"
	@echo "lwtrace			Tool to tested the libwlocate lib"
	@echo "libwlocate		Library for geo tracking"
	@echo "clean			To remove compiled files"
	@echo "install			Install the compiled file"
	@echo "uninstall		Remove installed executabel"
