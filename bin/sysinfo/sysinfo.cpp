#include <blt/Message.h>
#include <blt/Connection.h>

#include <blt/syscall.h>

#include <stdio.h>

#define PCI_STUB

#include <pci.h>
#include <string.h>
#include <stdlib.h>

using namespace BLT;

int pci_dump(PCI *pci)
{
	Message msg;
	pci_cfg cfg;
	int n;
	
	printf("bus  dev  func vndr devc cls  sub  ifc  irq  base[0]  size[0]  base[1]  size[1] \n");
	printf("---- ---- ---- ---- ---- ---- ---- ---- ---- -------- -------- -------- --------\n");
 
	for(n=0;pci->get_nth_cfg(n,&cfg) == 0;n++){		
		printf("%U %U %U %S %S %U %U %U ",
			   cfg.bus, cfg.dev, cfg.func,
			   cfg.vendor_id, cfg.device_id,
			   cfg.base_class, cfg.sub_class, cfg.interface);
		switch(cfg.header_type & 0x7f){
		case 0:
			printf("%U %x %x %x %x\n", cfg.irq,
				   cfg.base[0], cfg.size[0],
				   cfg.base[1], cfg.size[1]);
			break;
			
		case 1:
			printf(" (pci <-> pci bridge)\n");
			break;
			
		default:
			printf(" (unknown header type)\n");
		}
	}
}
	
void dump_rom(PCI *pci, int bus, int dev, int func)
{
	int i;
	uchar *rom = (uchar *) 0xf0000000;
	
	area_create(4096, 0, (void**) &rom, AREA_PHYSMAP);
	printf("device: %d / %d / %d\n",bus,dev,func);
	
	pci->write(bus,dev,func,0x30,0xf0000001,4);
	
	for(i=0;i<256;i+=16){
		printf("%S: %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X %X\n",
			   i, rom[i+0], rom[i+1], rom[i+2], rom[i+3],
			   rom[i+4], rom[i+5], rom[i+6], rom[i+7],
			   rom[i+8], rom[i+9], rom[i+10], rom[i+11],
			   rom[i+12], rom[i+13], rom[i+14], rom[i+15]);
	}
}

int main(int argc, char *argv[])
{
	PCI *pci = PCI::FindService();
	
	if(argc == 5){
		char *cmd = argv[1];
		int bus = atoi(argv[2]);
		int dev = atoi(argv[3]);
		int func = atoi(argv[4]);
		
		if(!strcmp(cmd,"rom")){
			dump_rom(pci,bus,dev,func);
		}
		
		return 0;	
	}
	
	if(!pci) {
		printf("sysinfo: cannot find pci service\n");
	} else {
		pci_dump(pci);
	}
	
	return 0;
}
