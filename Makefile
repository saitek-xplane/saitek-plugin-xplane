
CC=gcc
CXX=g++
CP=cp

#DIR=$(shell cd)
DIR=$(shell pwd)
SDK=$(DIR)/../SDK/CHeaders


#OPTIONS+=-ggdb -arch i386 -D__XPTESTING__ -DDEBUG
OPTIONS+=-Os -arch i386 -D__XPTESTING__ -D__XPTESTING__
DEFS+=-DNO_NED_NAMESPACE -DREPLACE_SYSTEM_ALLOCATOR -DXPLM200 -DAPL=1

INCLUDE+=-I$(DIR)/include
INCLUDE+=-I$(SDK)/XPLM
INCLUDE+=-I$(DIR)/include
INCLUDE+=-I$(DIR)/include/ptypes

LIBS=-framework IOKit -framework CoreFoundation
LIBS+=-lptypes32

# -shared -rdynamic -nodefaultlibs -m32 ARCHFLAGS="-arch i386"
LNFLAGS+=-dynamiclib  -flat_namespace -undefined warning

all:
	$(CC)  -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall ./hidapi/mac/hid.c
	$(CC)  -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall nedmalloc.c
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall overloaded.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall PanelThreads.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall Saitek.cpp

	$(CXX) $(OPTIONS) -L. $(LIBS) $(LNFLAGS) nedmalloc.o overloaded.o PanelThreads.o Saitek.o hid.o -o saitek.xpl

clean:
	$(RM) *.o *.xpl
	


