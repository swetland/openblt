BLTHOME := ../../
include $(BLTHOME)make.conf

SRCS		:= main.c identify.c disk.c blkdev.c
BINARY		:= ide.bin
CFLAGS		+= -finline-functions
LIBS		:= -lposix -lblt -lc

include $(BLTHOME)make.actions
