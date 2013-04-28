##################################################
#
#	file:			Makefile	
#	author: 		Joseph Burns
#
#	Makefile for a hashmap 
#
##################################################

CC = gcc
CFLAGS = -Wall -Wno-implicit -Wshadow -g

TESTOBJS = testmap.o map.o

all: testmap

# the object files
OFILES = testmap.o map.o mapconf.o mapfn.o

testmap: 	$(OFILES) 
				$(CC) $(CFLAGS) -o testmap $(OFILES)

testmap.o:  testmap.c map.h mapconf.h
				$(CC) $(CFLAGS) -c -o testmap.o testmap.c

map.o:      mapconf.o map.h mapconf.h
				$(CC) $(CFLAGS) -c -o map.o map.c

mapconf.o:  mapconf.c mapconf.h map.h
				$(CC) $(CFLAGS) -c -o mapconf.o mapconf.c

mapfn.o:    mapfn.c mapfn.h map.h
				$(CC) $(CFLAGS) -c -o mapfn.o mapfn.c

clean:
				rm -rf *.o testmap 
