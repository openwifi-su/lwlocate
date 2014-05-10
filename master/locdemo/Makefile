PROGRAM = LocDemo

INCLUDEDIRS = -I/usr/X11R6/include -I.. $(shell wx-config --cflags) $(shell pkg-config gtk+-2.0 --cflags)

LIBDIRS = -L/usr/X11R6/lib -L/usr/lib

LIBS = $(shell wx-config --libs) -lwlocate

DBGFLAGS = -O2 -g0 -DNDEBUG
STRIP=strip
ifeq ($(DEBUG),1)
DBGFLAGS = -O0 -g3 -DNOSPLASH
STRIP=ls -al
endif

CXXSOURCES = LocDemoApp.cpp LocDemoWin.cpp

CXXOBJECTS = $(CXXSOURCES:.cpp=.o)
CXXFLAGS = -Wall -Wno-unused-local-typedefs -DESRI_UNIX -DENV_LINUX $(INCLUDEDIRS) $(DBGFLAGS)
CXX = g++

LDFLAGS = $(LIBDIRS) $(LIBS)

all: $(PROGRAM)

$(PROGRAM): $(CXXOBJECTS)
	$(CXX) -o $@ $(CXXOBJECTS) $(LDFLAGS)
	$(STRIP) $(PROGRAM)

clean:
	$(RM) -f $(CXXOBJECTS) $(PROGRAM)

