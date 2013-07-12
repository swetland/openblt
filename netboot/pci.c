#include <blt/types.h>
#include <i386/io.h>


typedef struct confadd
{
	uchar reg:8;
	uchar func:3;
	uchar dev:5;
	uchar bus:8;
	uchar rsvd:7;
	uchar enable:1;
} confadd;

uint32 pci_read(int bus, int dev, int func, int reg, int bytes)
{
	uint32 base;
	
	union {
		confadd c;
		uint32 n;
	} u;
	
	u.c.enable = 1;
	u.c.rsvd = 0;
	u.c.bus = bus;
	u.c.dev = dev;
	u.c.func = func;
	u.c.reg = reg & 0xFC;
	
	outl(u.n,0xCF8);
	
	base = 0xCFC + (reg & 0x03);
		
	switch(bytes){
	case 1: return inb(base);
	case 2: return inw(base);
	case 4: return inl(base);
	default: return 0;
	}
}

void find_8390(int *addr)
{
	union {
		struct {
			uint16 vendor;
			uint16 device;
		} id;
		uint32 n;
	} u;
	uint32 n;
	int bus;
	int dev;
	
	for(bus=0;bus<8;bus++){
		for(dev=0;dev<32;dev++){
			u.n = pci_read(bus, dev, 0, 0, 4);
			if((u.id.vendor == 0x10ec) && (u.id.device == 0x8029)){
				n = pci_read(bus, dev, 0, 0x10, 4);
				n &= 0xFFF0;
				*addr = (int) n;
				return;
			}
		}
	}
}
