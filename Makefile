VERBOUSFLAGS = -O2 -g0
STRIP=strip
ifeq ($(VERBOUS),true)
VERBOUSFLAGS = -O0 -g3
STRIP=ls -al
endif

DEBUGFLAG =
ifeq ($(DEBUG),true)
	DEBUGFLAG = -DDBUG
endif

CC = gcc
LINKFLAGS = -L/usr/lib -L/usr/local/lib/
HEADERFLAGS = -I.
CFLAGS = -Wall $(VERBOUSFLAGS) $(DEBUGFLAG)
LDFLAGS = -L. -lwlocate -lm

SOURCES=trace.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE = lwtrace

all:
ifeq ($(TARGET),ENV_LINUX)
	$(CC) -c -DENV_LINUX $(SOURCES)
	$(CC) $(CFLAGS) -DENV_LINUX $(LINKFLAGS) $(HEADERFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LDFLAGS)
	$(STRIP) $(EXECUTABLE)
else
	$(error "TARGET not defined please type make help.")
endif

clean:
	rm -f $(OBJECTS)
	rm -f $(EXECUTABLE)

install:
	@echo "Install $(EXECUTABLE) ..."
	install -m 0755 $(EXECUTABLE) /usr/local/bin/$(EXECUTABLE)

uninstall:
	@echo "Removing $(EXECUTABLE) ..."
	rm -f /usr/local/bin/$(EXECUTABLE)

help:
	@echo "Usage: make [options]"
	@echo "TARGET=<arch>		ENV_LINUX for Linux and ENV_WINDOWS for Windows"
	@echo "VERBOUS=true		For more compiling output"
	@echo "DEBUG=true		For developer debuging output"
	@echo "clean			To remove compiled files"
	@echo "Install			Install the compiled file"
