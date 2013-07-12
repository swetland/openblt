#include <blt/Message.h>
#include <blt/Connection.h>
#include <blt/namer.h>
#include <blt/syscall.h>

using namespace BLT;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pci.h"

static pci_cfg *pci_dev[128];
static int pci_count = 0;

void server(void *data)
{
	Connection *cnxn = (Connection *) data;
	Message msg,reply;
	int res;
	
	while(cnxn->Recv(&msg) == 0){
		int32 op = -1;
		msg.GetInt32('code',&op);
		
		reply.Empty();
		
		switch(op){
		case 1: { // get_nth_pci_cfg
			int32 n = -1;
			msg.GetInt32('arg0', &n);
			if((n >= 0) && (n < pci_count)){
				reply.PutData('ret0','pcic',pci_dev[n],sizeof(pci_cfg));
				res = 0;
			} else {
				res = -1;
			}
			break;
		}
					
		case 2: { // pci_read
			int32 bus = 0;
			int32 dev = 0;
			int32 func = 0;
			int32 reg = 0;
			int32 size = 0;
			msg.GetInt32('arg0',&bus);
			msg.GetInt32('arg1',&dev);
			msg.GetInt32('arg2',&func);
			msg.GetInt32('arg3',&reg);
			msg.GetInt32('arg4',&size);
			
			printf("pci: read(%d,%d,%d,%d,%d)\n",bus,dev,func,reg,size);
			size = (int32) pci_read(bus,dev,func,reg,size);
			
			reply.PutInt32('ret0',size);
			res = 0;
			break;
		}
		
		case 3: { // pci_write
			int32 bus = 0;
			int32 dev = 0;
			int32 func = 0;
			int32 reg = 0;
			int32 size = 0;
			int32 val = 0;
			
			msg.GetInt32('arg0',&bus);
			msg.GetInt32('arg1',&dev);
			msg.GetInt32('arg2',&func);
			msg.GetInt32('arg3',&reg);
			msg.GetInt32('arg4',&val);
			msg.GetInt32('arg5',&size);
			pci_write(bus,dev,func,reg,(uint32)val,size);
			res = 0;
			break;
		}
		
		default:
			res = -1;
		}
		
		reply.PutInt32('resp',res);
		res = msg.Reply(&reply);
	}
}

int pci_scan()
{
	pci_cfg cfg;
	int bus,dev,func;
	
	for(bus=0;bus<255;bus++){
		for(dev=0;dev<32;dev++) {
			if(pci_probe(bus,dev,0,&cfg)) continue;
			pci_dev[pci_count] = (pci_cfg*) malloc(sizeof(pci_cfg));
			memcpy(pci_dev[pci_count],&cfg,sizeof(pci_cfg));
			pci_count++;
			if(cfg.header_type & 0x80){
				for(func=1;func<8;func++){
					if(!pci_probe(bus,dev,func,&cfg)){
						pci_dev[pci_count] = (pci_cfg*) malloc(sizeof(pci_cfg));
						memcpy(pci_dev[pci_count],&cfg,sizeof(pci_cfg));
						pci_count++;
					}
				}
			}
		}
	}
}

int main(int argc, char **argv[])
{
	Connection *cnxn;
	
	if(namer_find("pci",0) > 0) {
//		printf("pci: server is already running\n");
		return 0;
	}
	
	cnxn = Connection::CreateService("pci");

	pci_scan();
		
	if(cnxn){
//		printf("pci: server started\n",cnxn);
		thr_resume(thr_create(server,cnxn,"pci server"));
	}
	return 0;
}
