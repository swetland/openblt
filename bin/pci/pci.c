/* $Id$
**
** Copyright 1999 Brian J. Swetland
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions, and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions, and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <i386/io.h>

#include "pci.h"

uint32 read_pci(int bus, int dev, int func, int reg, int bytes)
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

void write_pci(int bus, int dev, int func, int reg, uint32 v, int bytes)
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
	
	base = 0xCFC + (reg & 0x03);
	outl(u.n,0xCF8);
	switch(bytes){
	case 1: outb(v,base); break;
	case 2: outw(v,base); break;
	case 4: outl(v,base); break;
	}
	
}

void probe(int bus, int dev, int func)
{
	union {
		pci_cfg cfg;
		uint32 word[4];
	} x;
	pci_cfg *cfg = &x.cfg;
	uint32 v;	
	int i;
	for(i=0;i<4;i++){
		x.word[i] = read_pci(bus,dev,func,4*i,4);
	}
	if(cfg->vendor_id == 0xffff) return;
	
	printf("Device Info: /bus/pci/%d/%d/%d\n",bus,dev,func);
	printf("  * Vendor: %S   Device: %S  Class/SubClass/Interface %X/%X/%X\n",
		   cfg->vendor_id,cfg->device_id,cfg->base_class,cfg->sub_class,cfg->interface);
	printf("  * Status: %S  Command: %S  BIST/Type/Lat/CLS: %X/%X/%X/%X\n",
		   cfg->status, cfg->command, cfg->bist, cfg->header_type, 
		   cfg->latency_timer, cfg->cache_line_size);
	
	switch(cfg->header_type & 0x7F){
	case 0: /* normal device */
		for(i=0;i<6;i++){
			v = read_pci(bus,dev,func,i*4 + 0x10, 4);
			if(v) {
				int v2;
				write_pci(bus,dev,func,i*4 + 0x10, 0xffffffff, 4);
				v2 = read_pci(bus,dev,func,i*4+0x10, 4) & 0xfffffff0;
				v2 = 1 + ~v2;
				if(v & 1) {
					printf("  * Base Register %d IO: %x (%x)\n",i,v&0xfff0,v2&0xffff);
				} else {
					printf("  * Base Register %d MM: %x (%x)\n",i,v&0xfffffff0,v2);
				}
			}
		}
		v = read_pci(bus,dev,func,0x3c,1);
		if((v != 0xff) && (v != 0)) printf("  * Interrupt Line: %X\n",v);
		break;
	case 1:
		printf("  * PCI <-> PCI Bridge\n");
		break;
	default:
		printf("  * Unknown Header Type\n");
	}
	
	if(cfg->header_type & 0x80){
		for(i=1;i<8;i++){
			probe(bus,dev,i);
		}
	}
	
//	v = read_pci(bus,dev,func,0x3C);
//	printf("  * Lat/Gnt/IntPin/IntLine: %x\n",v);
}

int main (int argc, char **argv)
{
	int bus,dev;
	
	for(bus=0;bus<255;bus++){
		for(dev=0;dev<32;dev++) {
			probe(bus,dev,0);
		}
	}
	
	return 0;
}

