###########################################
# Simple Makefile for HIDAPI test program
#
# Alan Ott
# Signal 11 Software
# 2010-07-03
###########################################

all: hidtest

CC=gcc
CXX=g++
COBJS=../hidapi/mac/hid.o
CPPOBJS=hidtest.o
OBJS=$(COBJS) $(CPPOBJS)
CFLAGS+=-I../include -Wall -g -c 
LIBS=-framework IOKit -framework CoreFoundation

hidtest: $(OBJS)
	g++ -Wall -g $^ $(LIBS) -o hidtest

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -f *.o hidtest $(CPPOBJS) ../hidapi/mac/*.o

.PHONY: clean
