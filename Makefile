# 
#
#
#

CC=gcc
CXX=g++
CP=cp

# -rdynamic -nodefaultlibs -m32 export ARCHFLAGS ="-arch i386"

HOSTOS=$(shell uname | tr A-Z a-z)
ifeq ($(HOSTOS),darwin)
 HIDAPIPATH=./hidapi/mac
 LIBS=-framework IOKit -framework CoreFoundation
 LNFLAGS=-dynamiclib -flat_namespace -undefined warning
 OPTIONS=-arch i386
 DIR=$(shell pwd)
else
 ifeq ($(HOSTOS),linux)
  HIDAPIPATH=./hidapi/linux
  LIBS=
  LNFLAGS=-shared
  OPTIONS=-march=i386
  DIR=$(shell pwd)
 else # windows
  HIDAPIPATH=./hidapi/windows
  LIBS=
  LNFLAGS=-shared
  OPTIONS=-march=i386
  DIR=$(shell cd)
 endif
endif

SDK=$(DIR)/SDK/CHeaders
LIBS+=-lptypes32

#OPTIONS+=-ggdb -D__XPTESTING__ -DDEBUG
OPTIONS+=-O3
DEFS=-DXPLM200 -DAPL=1 -DNO_PANEL_CHECK

INCLUDE=-I$(DIR)/include
INCLUDE+=-I$(SDK)/XPLM
INCLUDE+=-I$(DIR)/include
INCLUDE+=-I$(DIR)/include/ptypes

all:
#	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall overloaded.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall $(HIDAPIPATH)/hid.c
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall multipanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall PanelThreads.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall radiopanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall SaitekProPanels.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall switchpanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(OPTIONS) -Wall utils.c

#overloaded.o
	$(CXX) $(OPTIONS) -L. $(LIBS) $(LNFLAGS) \
										hid.o \
										multipanel.o \
										PanelThreads.o \
										radiopanel.o \
										SaitekProPanels.o \
										switchpanel.o \
										utils.o \
										-o SaitekProPanels.xpl

clean:
	$(RM) *.o *.xpl



