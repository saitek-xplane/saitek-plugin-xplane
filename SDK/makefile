


# DIR = $(shell cd)


DIR = $(shell pwd)

#OPTIONS += -g
OPTIONS += -O3 -DAPL


INCLUDE +=-I$(DIR)/CHeaders/XPLM
INCLUDE +=-I$(DIR)/CHeaders/Widgets


CC = gcc
CXX = g++


all:
	$(CC) -c $(INCLUDE) $(OPTIONS) -Wall KeySniffer.c

	$(CC) $(INCLUDE) $(OPTIONS) -dynamiclib KeySniffer.o -o KeySniffer.xpl

clean:
	$(RM) *.o *.xpl
	


