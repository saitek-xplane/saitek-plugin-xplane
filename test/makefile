


# DIR = $(shell cd)


DIR = $(shell pwd)

OPTIONS += -g

INCLUDE +=-I$(DIR)/include
INCLUDE +=-I$(DIR)/include/XPLM
INCLUDE +=-I$(DIR)/../include
INCLUDE +=-I$(DIR)/../include/ptypes


CC = gcc
CXX = g++
CP = cp

all:
	$(CXX) -c $(INCLUDE) $(OPTIONS) -Wall XPLMProcessing.cpp
	$(CXX) -c $(INCLUDE) $(OPTIONS) -Wall ../PanelThreads.cpp
	$(CXX) -c $(INCLUDE) $(OPTIONS) -Wall ../Saitek.cpp
	$(CXX) -c $(INCLUDE) $(OPTIONS) -Wall hid.c

	$(CXX) $(OPTIONS) -L../ -lptypes -dynamiclib PanelThreads.o Saitek.o hid.o XPLMProcessing.o -o saitek.xplm

	$(CXX) $(INCLUDE) $(OPTIONS) -Wall -L../ -lptypes saitek.xplm main.cpp -o main
clean:
	$(RM) *.o main *.xplm
	


