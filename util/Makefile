# $Id: //depot/blt/util/Makefile#3 $

CC=cc
CFLAGS=-O2 
#-DxBIG_ENDIAN
#NETSTUFF=-lsocket -lnsl

all: netboot bootmaker dumphex 

netboot: netboot.o
	$(CC) -o netboot netboot.o $(NETSTUFF)

bootmaker: bootmaker.o
	$(CC) -o bootmaker bootmaker.o

dumphex: dumphex.o
	$(CC) -o dumphex dumphex.o

dumph: dumph.o
	$(CC) -o dumph dumph.o

bootblock.h: dumph bootsect.bin
	./dumph bootsect.bin > bootblock.h

clean:
	rm -f netboot bootmaker dumphex *.o *~
