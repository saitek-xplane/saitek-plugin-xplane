
CC=gcc
CXX=g++
CP=cp

#DIR=$(shell cd)
DIR=$(shell pwd)
SDK=$(DIR)/SDK/CHeaders


#OPTIONS+=-ggdb -arch i386 -D__XPTESTING__ -DDEBUG
OPTIONS+=-Os -arch i386
# -DREPLACE_SYSTEM_ALLOCATOR
DEFS+=-DNO_NED_NAMESPACE -DXPLM200 -DAPL=1 -DNO_PANEL_CHECK

INCLUDE+=-I$(DIR)/include
INCLUDE+=-I$(SDK)/XPLM
INCLUDE+=-I$(DIR)/include
INCLUDE+=-I$(DIR)/include/ptypes

LIBS=-framework IOKit -framework CoreFoundation
LIBS+=-lptypes32

# -shared -rdynamic -nodefaultlibs -m32 export ARCHFLAGS ="-arch i386"
LNFLAGS+=-dynamiclib  -flat_namespace -undefined warning

all:
#	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) nedmalloc.c
#	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall overloaded.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall ./hidapi/mac/hid.c
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall utils.c
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall radiopanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall multipanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall switchpanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall PanelThreads.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall SaitekProPanels.cpp

#overloaded.o  nedmalloc.o
	$(CXX) $(OPTIONS) -L. $(LIBS) $(LNFLAGS) PanelThreads.o \
							radiopanel.o multipanel.o switchpanel.o \
							SaitekProPanels.o hid.o utils.o -o SaitekProPanels.xpl

clean:
	$(RM) *.o *.xpl



