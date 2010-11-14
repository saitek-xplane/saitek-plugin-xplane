
CC=gcc
CXX=g++
CP=cp

# DIR=$(shell cd)
#  -flat_namespace -undefined warning

DIR=$(shell pwd)

OPTIONS+=-ggdb
#OPTIONS+=-O3
OPTIONS+=-DNO_NED_NAMESPACE -DREPLACE_SYSTEM_ALLOCATOR -D__TESTING__ -Wall
CFLAGS+=-I$(DIR)/include -I$(DIR)/include/XPLM -I$(DIR)/../include -I$(DIR)/../include/ptypes
LIBS=-framework IOKit -framework CoreFoundation


all:
	$(CC)  -c $(CFLAGS) $(OPTIONS) hid.c
	$(CC)  -c $(CFLAGS) $(OPTIONS) ../nedmalloc.c
	$(CXX) -c $(CFLAGS) $(OPTIONS) ../overloaded.cpp
	$(CXX) -c $(CFLAGS) $(OPTIONS) XPLMProcessing.cpp
	$(CXX) -c $(CFLAGS) $(OPTIONS) XPLMUtilities.cpp
	$(CXX) -c $(CFLAGS) $(OPTIONS) ../PanelThreads.cpp
	$(CXX) -c $(CFLAGS) $(OPTIONS) ../Saitek.cpp

	$(CXX) $(OPTIONS) -L../ -lptypes -dynamiclib  \
		nedmalloc.o overloaded.o PanelThreads.o Saitek.o hid.o XPLMUtilities.o XPLMProcessing.o -o saitek.xpl

	$(CXX) $(CFLAGS) $(OPTIONS) -Wall -L../ -lptypes saitek.xpl main.cpp -o main
clean:
	$(RM) *.o main *.xpl
	


