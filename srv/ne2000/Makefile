BLTHOME := ../../
include $(BLTHOME)make.conf

SRCS		:= ne2k.c ne2000.c pciglue.cpp
BINARY		:= ne2000.bin
LIBS		:= -lposix -lblt -lc
CFLAGS		+= -I../pci
CXXFLAGS	+= -I../pci

include $(BLTHOME)make.actions
