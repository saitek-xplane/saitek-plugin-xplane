# 
CXX=g++
WINDLLMAIN=
# -m32 export ARCHFLAGS ="-arch i386"

HOSTOS=$(shell uname | tr A-Z a-z)
ifeq ($(HOSTOS),darwin)
 HIDAPIPATH=./hidapi/mac
 LIBS=-lptypes -framework IOKit -framework CoreFoundation
 LNFLAGS=-arch i386 -dynamiclib -flat_namespace -undefined warning -L.
 CFLAGS=-arch i386 -Wall -O3 -DAPL=1 -DIBM=0 -DLIN=0
else
 ifeq ($(HOSTOS),linux)
  HIDAPIPATH=./hidapi/linux
  LIBS=-lptypes
  LNFLAGS=-shared -rdynamic -nodefaultlibs -L.
  CFLAGS=-march=i386 -Wall -O3 -DAPL=0 -DIBM=0 -DLIN=1 -fvisibility=hidden
 else # windows
  HOSTOS=windows
  HIDAPIPATH=./hidapi/windows
  LIBS=-lptypes -lXPLM -lSetupAPI
  LNFLAGS=-m32 -Wl,-O1 -shared -L. -L./SDK/Libraries/Win/
  CFLAGS=-m32 -DAPL=0 -DIBM=1 -DLIN=0 -Wall -fpermissive
  WINDLLMAIN=SaitekProPanelsWin.o
 endif
endif

# To set user/compiler debug mode (use DPRINTF for stdio): -DDEBUG
# To dynamically check for USB connected saitek panels: -DDO_USBPANEL_CHECK
DEFS=-DXPLM200 -DLOGPRINTF

INCLUDE=-I./include
INCLUDE+=-I./SDK/CHeaders/XPLM
INCLUDE+=-I./include
INCLUDE+=-I./include/ptypes

all:
ifeq ($(HOSTOS),windows)
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) SaitekProPanelsWin.cpp
endif
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) $(HIDAPIPATH)/hid.c
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) multipanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) PanelThreads.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) radiopanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) SaitekProPanels.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) switchpanel.cpp
	$(CXX) -c $(INCLUDE) $(DEFS) $(CFLAGS) utils.c

	$(CXX) -o SaitekProPanels.xpl hid.o multipanel.o PanelThreads.o radiopanel.o SaitekProPanels.o switchpanel.o utils.o $(WINDLLMAIN) $(LNFLAGS) $(LIBS) 

clean:
	$(RM) *.o *.xpl
