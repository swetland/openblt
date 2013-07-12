BLTHOME := ../../
include $(BLTHOME)make.conf

BINARY		:= vfs.bin
OBJS		:= vfs.o fs.o super.o vnode.o shm.o path.o rootfs.o bootfs.o
CRT0		:= $(BLTHOME)lib/crtb.o
LIBS		:= -lposix -lblt -ldl -lc
SUBDIRS		:= drivers

#CFLAGS 	+= -DVFS_SANDBOX -Isandbox -Wall

sandbox: fs.o super.o vnode.o

include $(BLTHOME)make.actions
