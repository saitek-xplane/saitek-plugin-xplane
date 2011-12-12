# 
#
#
#
CXX=g++
LNK=g++
WINDLLMAIN=
OBJ=o
HIDOBJ=hid.$(OBJ)
# -m32 export ARCHFLAGS ="-arch i386"

HOSTOS=$(shell uname | tr A-Z a-z)
ifeq ($(HOSTOS),darwin)
 HIDAPIPATH=./hidapi/mac
 LIBS=-lptypes -framework IOKit -framework CoreFoundation
 LNFLAGS=-dynamiclib -flat_namespace -undefined warning
 CFLAGS=-arch i386 -Wall -O3
else
 ifeq ($(HOSTOS),linux)
  HIDAPIPATH=./hidapi/linux
  LIBS=-lptypes
  LNFLAGS=-shared -rdynamic -nodefaultlibs
  CFLAGS=-march=i386 -Wall -O3
 else # windows
  HOSTOS=windows
  HIDAPIPATH=./hidapi/windows
  LIBS=-lptypes.lib -lhidapi.lib -l./SDK/Libraries/Win/XPLM.lib
  LNFLAGS=-WD -mn
  CFLAGS=
  CC=../dm/bin/dmc
  CXX=../dm/bin/dmc
  WINDLLMAIN=SaitekProPanelsWin.obj
  OBJ=obj
  HIDOBJ=
  LNK=link
 endif
endif

#OPTIONS+=-ggdb -D__XPTESTING__ -DDEBUG

# for USB panel checking pass: -DDO_USBPANEL_CHECK
DEFS=-DXPLM200 -DAPL=1

INCLUDE=-I./include
INCLUDE+=-I./SDK/CHeaders/XPLM
INCLUDE+=-I./include
INCLUDE+=-I./include/ptypes

all:
#	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) overloaded.cpp
ifneq ($(HOSTOS),windows)
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) $(HIDAPIPATH)/hid.c
else
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) SaitekProPanelsWin.cpp
endif
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) multipanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) PanelThreads.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) radiopanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) SaitekProPanels.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) switchpanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) utils.c

#overloaded.o
	$(LNK) -L. $(LIBS) $(LNFLAGS) $(HIDOBJ) multipanel.$(OBJ) PanelThreads.$(OBJ) radiopanel.$(OBJ) SaitekProPanels.$(OBJ) switchpanel.$(OBJ) utils.$(OBJ) $(WINDLLMAIN) 


clean:
	$(RM) *.o *.xpl



