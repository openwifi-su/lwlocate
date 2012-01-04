PROGRAM = wlocserv

INCLUDEDIRS = -I.. -Iliboapc/ -I/opt/include/ -I../../../OpenAPC/liboapc/ -I../../../OpenAPC/plugins -I../../../
#$(shell wx-config --cflags) $(shell pkg-config gtk+-2.0 --cflags)


LIBDIRS = \
	-L/usr/lib/mysql -L/usr/lib -L/opt/lib

LIBS = \
	 -loapc -lmysqlclient
#$(shell wx-config --libs)

DBGFLAGS = -O2 -g0
ifeq ($(DEBUG),1)
DBGFLAGS = -O0 -g3 -D_DEBUG
endif

CXXSOURCES = wlocservInc.cpp

CXXOBJECTS = $(CXXSOURCES:.cpp=.o)
CXXFLAGS = -Wall -DESRI_UNIX -DENV_LINUX $(INCLUDEDIRS) $(DBGFLAGS)
CXX = g++

LDFLAGS = $(LIBDIRS) $(LIBS)

all: $(PROGRAM)

$(PROGRAM): $(CXXOBJECTS)
	$(CXX) -o $@ $(CXXOBJECTS) $(LDFLAGS)
	strip $(PROGRAM)

clean:
	$(RM) -f $(CXXOBJECTS) $(PROGRAM)

