#define PCI_STUB

#include <pci.h>

extern "C" {
	int find_pci(uint16 vendor, uint16 device, int *iobase, int *irq);
}

using namespace BLT;

int find_pci(uint16 vendor, uint16 device, int *iobase, int *irq)
{
	Message msg;
	pci_cfg cfg;
	int n;
	int res = 0;
	
	PCI *pci = PCI::FindService();
	
	if(pci){
		for(n=0;pci->get_nth_cfg(n,&cfg) == 0;n++){
			if((cfg.device_id == device) &&
			   (cfg.vendor_id == vendor)){
				*iobase = cfg.base[0] & 0xfff8;
				*irq = cfg.irq;
				res = 1;
				break;
			}
		}
		delete pci;
	}
	
	return res;
}

