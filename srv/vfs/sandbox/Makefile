# $Id: //depot/blt/srv/vfs/sandbox/Makefile#2 $

CFLAGS = $(CF) -I. -Wall -g -DVFS_SANDBOX
LIBS =
OBJS = vfs.o blkdev.o ../fs.o ../super.o ../vnode.o ../../../lib/libblt/hash.o
EXEC = vfs

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LIBS)

clean:
	rm -f *.o *~ core *.core $(EXEC)
