BLTHOME := ../../../../
include $(BLTHOME)make.conf

SHLIB		:= ffs.so
OBJS		:= super.o inode.o file.o dir.o
CFLAGS		+= -I. -I../..
#CFLAGS 	+= -DVFS_SANDBOX -I. -I../..

include $(BLTHOME)make.actions
