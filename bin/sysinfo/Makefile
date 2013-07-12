BLTHOME := ../../
include $(BLTHOME)make.conf

BINARY		:= sysinfo.bin
SRCS		:= sysinfo.cpp
LIBS		:= -lposix -lblt -lc
CXXFLAGS	+= -I ../../srv/pci
CFLAGS		+= -I ../../srv/pci

include $(BLTHOME)make.actions

