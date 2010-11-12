
CC = gcc
CXX = g++
CP = cp

# export ARCHFLAGS="-arch i386"
# DIR = $(shell cd)

DIR = $(shell pwd)
SDK=$(DIR)/../SDK/CHeaders

OPTIONS += -ggdb -arch i386 -DDEBUG -D__XPTESTING__
#OPTIONS += -Os
DEFS += -DNO_NED_NAMESPACE -DREPLACE_SYSTEM_ALLOCATOR -DXPLM200 -DAPL=1

INCLUDE +=-I$(DIR)/include
INCLUDE +=-I$(SDK)/XPLM
INCLUDE +=-I$(DIR)/include
INCLUDE +=-I$(DIR)/include/ptypes

LNFLAGS += -dynamiclib -framework IOKit -framework CoreFoundation -flat_namespace -undefined warning
#   -shared -rdynamic -nodefaultlibs -m32

LIBS += -lptypes32


all:
	$(CC)  -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall ./hidapi/mac/hid.c
	$(CC)  -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall nedmalloc.c
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall overloaded.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall PanelThreads.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall Saitek.cpp

	$(CXX) $(OPTIONS) -L. $(LIBS) $(LNFLAGS) nedmalloc.o overloaded.o PanelThreads.o Saitek.o hid.o -o saitek.xpl

clean:
	$(RM) *.o *.xpl
	


