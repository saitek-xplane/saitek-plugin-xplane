


# DIR = $(shell cd)
#  -flat_namespace -undefined warning

DIR = $(shell pwd)

OPTIONS += -g
#OPTIONS += -O3
OPTIONS += -DNO_NED_NAMESPACE -DREPLACE_SYSTEM_ALLOCATOR

INCLUDE +=-I$(DIR)/include
INCLUDE +=-I$(DIR)/include/XPLM
INCLUDE +=-I$(DIR)/../include
INCLUDE +=-I$(DIR)/../include/ptypes


CC = gcc
CXX = g++
CP = cp

all:
	$(CC) -c $(INCLUDE) $(OPTIONS) -Wall ../nedmalloc.c
	$(CXX) -c $(INCLUDE) $(OPTIONS) -Wall ../overloaded.cpp
	$(CXX) -c $(INCLUDE) $(OPTIONS) -Wall XPLMProcessing.cpp
	$(CXX) -c $(INCLUDE) $(OPTIONS) -Wall ../PanelThreads.cpp
	$(CXX) -c $(INCLUDE) $(OPTIONS) -Wall ../Saitek.cpp
	$(CXX) -c $(INCLUDE) $(OPTIONS) -Wall hid.c

	$(CXX) $(OPTIONS) -L../ -lptypes -dynamiclib nedmalloc.o overloaded.o PanelThreads.o Saitek.o hid.o XPLMProcessing.o -o saitek.xpl

	$(CXX) $(INCLUDE) $(OPTIONS) -Wall -L../ -lptypes saitek.xpl main.cpp -o main
clean:
	$(RM) *.o main *.xpl
	


