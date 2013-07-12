BLTHOME := ../

include $(BLTHOME)make.conf

CFLAGS		+= -I../kernel
BINARY		:= boot.bin
OBJS		:= bootstub.o boot.o
LIBS		:= -lconsole
ENTRY		:= $(ENTRY_BOOTSTRAP)
CRT0		:= 

include $(BLTHOME)make.actions
