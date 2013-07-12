BLTHOME := ../../
include $(BLTHOME)make.conf

TARGETS		:= rtld
SRCS		:= load.c sym.c elf.c
LIBRARY		:= libdl.a
SHLIB		:= libdl.so
SHLIB_MAJOR	:= 1
SHLIB_MINOR	:= 0

RTLD_OBJS = rtld.o sym.o elf.o

rtld: $(RTLD_OBJS)
	ld -dN -Ttext 0x200074 -o rtld $(RTLD_OBJS) -L../../lib/obj -lc -lblt -lc

include $(BLTHOME)make.actions
