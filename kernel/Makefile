BLTHOME := ../
include $(BLTHOME)make.conf

CFLAGS		+= -DASSERT $(SMP) $(SERIAL) $(SERIALDEBUG)
BINARY		:= kernel.bin
LIBS		:= -lkern -lconsole
ENTRY		:= $(ENTRY_KERNEL)
CRT0		:= 

OBJS 		:=  stub.o kernel.o i386.o jump.o ktrace.o \
			memory.o resource.o port.o sem.o aspace.o task.o rights.o \
			list.o team.o fault.o syscall.o debug.o pager.o

ifeq ($(SMP),-D__SMP__)
OBJS		+= smp.o cpuid.o trampoline.o
endif

include $(BLTHOME)make.actions
