# Very basic checks for depencecis
CHECK_LINUX_STRIP := $(shell command -v strip 2>/dev/null)
CHECK_LINUX_CC := $(shell command -v gcc 2>/dev/null)
CHECK_LINUX_WIRELESS_TOOLS := $(shell ld -liw -o /dev/null 2>&1 | grep -Eio warning)
CHECk_WIN_STRIP := $(shell command -v i686-w64-mingw32-strip 2>/dev/null)
CHECk_WIN_CC := $(shell command -v i686-w64-mingw32-gcc 2>/dev/null)
CHECK_LWTRACE := $(shell command -v lwtrace 2>/dev/null)
CHECK_LIBWLOCATE := $(shell echo '\#include <libwlocate.h>' | cpp -H -o /dev/null 2>&1 | grep -o -P 'fatal error' 2>/dev/null)

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
WINSTRIP = i686-w64-mingw32-strip
WIN32CC = i686-w64-mingw32-gcc
HEADERFLAGS = -I.
CFLAGS = -Wall

#lwtrace specifc
LINKFLAGS = -L/usr/lib -L/usr/local/lib/ -L.
LDFLAGS = -L. -lwlocate -lm
LWTRACE_SOURCES = trace.c
LWTRACE_OBJECTS = $(LWTRACE_SOURCES:.c=.o)
LWTRACE_EXECUTABLE = lwtrace
WINLWTRACE_EXECUTABLE = lwtrace.exe

#libwlocate specifc
LIBWLOCATE_CFLAGS = -fPIC -shared -Wno-unused
LIBWLOCATE_SOURCES = connect.c wlan.c libwlocate.c iwlist.c
WINLIBWLOCATE_SOURCES = connect.c wlan.c libwlocate.c
LIBWLOCATE_OBJECTS = $(LIBWLOCATE_SOURCES:.c=.o)
WINLIBWLOCATE_OBJECTS = $(WINLIBWLOCATE_SOURCES:.c=.o)
LINK = -shared -Wl,--no-as-needed
SYSLIBRARIES= -lm
LIBS = $(SYSLIBRARIES) -liw
LIBWLOCATE_EXECUTABLE = libwlocate.so
WINLIBWLOCATE_EXECUTABLE = libwlocate.dll
LIBWLOCATE_HEADER = libwlocate.h

all: libwlocate lwtrace

libwlocate:
ifeq ($(TARGET),ENV_LINUX)
ifndef CHECK_LINUX_CC
	$(error Some reqierments are not installed please run "make check")
endif
ifndef CHECK_LINUX_STRIP
	$(error Some reqierments are not installed please run "make check")
endif
ifndef CHECK_LINUX_WIRELESS_TOOLS
	$(error Some reqierments are not installed please run "make check")
endif
	$(CC) $(CFLAGS) $(LIBWLOCATE_CFLAGS) $(VERBOUSFLAGS) $(DEBUGFLAG) $(SLIMFLAG) -D$(TARGET) $(HEADERFLAGS) -c $(LIBWLOCATE_SOURCES)
	$(CC) $(LINK) $(DEBUGFLAG) $(SLIMFLAG) -D$(TARGET) -o $(LIBWLOCATE_EXECUTABLE) $(LIBWLOCATE_OBJECTS) $(LIBS)
	$(STRIP) $(LIBWLOCATE_EXECUTABLE)
else
ifeq ($(TARGET),ENV_WINDOWS)
ifndef CHECk_WIN_CC
	$(error Some reqierments are not installed please run "make check")
endif
ifndef CHECk_WIN_STRIP
	$(error Some reqierments are not installed please run "make check")
endif
	$(WIN32CC) $(CFLAGS) $(LIBWLOCATE_CFLAGS) $(VERBOUSFLAGS) $(DEBUGFLAG) $(SLIMFLAG) -D$(TARGET) $(HEADERFLAGS) -c $(WINLIBWLOCATE_SOURCES)
	$(WIN32CC) $(LINK) -o $(WINLIBWLOCATE_EXECUTABLE) $(WINLIBWLOCATE_OBJECTS) -lws2_32
	$(WINSTRIP) $(WINLIBWLOCATE_EXECUTABLE)
else
	$(error TARGET not defined please run "make help".)
endif
endif

lwtrace:
ifeq ($(TARGET),ENV_LINUX)
ifndef CHECK_LINUX_CC
	$(error Some reqierments are not installed please run "make check")
endif
ifndef CHECK_LINUX_STRIP
	$(error Some reqierments are not installed please run "make check")
endif
	$(CC) $(CFLAGS) $(VERBOUSFLAGS) $(DEBUGFLAG) $(SLIMFLAG) -D$(TARGET) -c $(LWTRACE_SOURCES) $(LDFLAGS)
	$(CC) $(CFLAGS) $(VERBOUSFLAGS) $(DEBUGFLAG) $(SLIMFLAG) -D$(TARGET) $(LINKFLAGS) $(HEADERFLAGS) -o $(LWTRACE_EXECUTABLE) $(LWTRACE_OBJECTS) $(LDFLAGS)
	$(STRIP) $(LWTRACE_EXECUTABLE)
else
ifeq ($(TARGET),ENV_WINDOWS)
ifndef CHECk_WIN_CC
	$(error Some reqierments are not installed please run "make check")
endif
ifndef CHECk_WIN_STRIP
	$(error Some reqierments are not installed please run "make check")
endif
	$(WIN32CC) $(CFLAGS) $(VERBOUSFLAGS) $(DEBUGFLAG) $(SLIMFLAG) -D$(TARGET) -c $(LWTRACE_SOURCES)
	$(WIN32CC) $(CFLAGS) $(VERBOUSFLAGS) $(DEBUGFLAG) $(SLIMFLAG) -D$(TARGET) $(LINKFLAGS) $(HEADERFLAGS) -o $(WINLWTRACE_EXECUTABLE) $(LWTRACE_OBJECTS) -lws2_32 -llibwlocate
	$(WINSTRIP) $(WINLWTRACE_EXECUTABLE)
else
	$(error TARGET not defined please run "make help".)
endif
endif

check:
ifndef CHECK_LINUX_CC
	$(info "gcc" ist not installed)
endif
ifndef CHECK_LINUX_STRIP
	$(info strip ist not installed check if you have the package "binutils")
endif
ifndef CHECK_LINUX_WIRELESS_TOOLS
	$(info "wireless_tools" ist not installed)
endif
ifndef CHECk_WIN_CC
	$(info "mingw-w64-gcc" ist not installed. For windows cross compiling)
endif
	@exit

clean:
	rm -f $(LWTRACE_OBJECTS)
	rm -f $(LWTRACE_EXECUTABLE)
	rm -f $(LIBWLOCATE_OBJECTS)
	rm -f $(LIBWLOCATE_EXECUTABLE)
	rm -f $(WINLWTRACE_EXECUTABLE)
	rm -f $(WINLIBWLOCATE_OBJECTS)
	rm -f $(WINLIBWLOCATE_EXECUTABLE)

install:
ifndef CHECK_LWTRACE
	@echo "Install $(LWTRACE_EXECUTABLE) ..."
	install -m 0755 $(LWTRACE_EXECUTABLE) /usr/bin/$(LWTRACE_EXECUTABLE)
endif
ifdef CHECK_LIBWLOCATE
	@echo "Install $(LIBWLOCATE_EXECUTABLE) ..."
	install -m 0755 $(LIBWLOCATE_EXECUTABLE) /usr/lib/$(LIBWLOCATE_EXECUTABLE)
	@echo "Install $(LIBWLOCATE_HEADER) ..."
	install -m 0755 $(LIBWLOCATE_HEADER) /usr/include/$(LIBWLOCATE_HEADER)
endif

uninstall:
ifdef CHECK_LWTRACE
	@echo "Removing $(LWTRACE_EXECUTABLE) ..."
	rm -f /usr/bin/$(LWTRACE_EXECUTABLE)
endif
ifndef CHECK_LIBWLOCATE
	@echo "Removing $(LIBWLOCATE_EXECUTABLE) ..."
	rm -f /usr/lib/$(LIBWLOCATE_EXECUTABLE)
	@echo "Removing $(LIBWLOCATE_HEADER) ..."
	rm -f /usr/include/$(LIBWLOCATE_HEADER)
endif

help:
	@echo "Usage: make [options]"
	@echo "TARGET=<arch>		ENV_LINUX for Linux and ENV_WINDOWS for Windows"
	@echo "VERBOUS=true		For more compiling output"
	@echo "DEBUG=true		For developer debuging output"
	@echo "SLIM=true		For embedded devices that don't have many space"
	@echo "libwlocate		Library for geo tracking"
	@echo "lwtrace			Tool to tested the libwlocate lib"
	@echo "check			Check if dependencies installed"
	@echo "clean			To remove compiled files"
	@echo "install			Install the compiled file"
	@echo "uninstall		Remove installed executabel"
