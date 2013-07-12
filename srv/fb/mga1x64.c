/* $Id$
**
** Copyright 1999 Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include "pci.h"
#include "fb.h"

#include <stdlib.h>
#include <blt/syscall.h>

#include "mga1x64.h"


static int32 mga_find(void **cookie)
{	
	pci_cfg cfg;
	mga1x64 *mga;
	int i;
	
	if(pci_find(&cfg,VENDOR,DEVICE)) return -1;
	
	dprintf("mga_find(): card @ %d/%d/%d",cfg.bus,cfg.dev,cfg.func);
	
	if(!(mga = (mga1x64*) malloc(sizeof(mga1x64)))) return -1;
	*cookie = mga;
	
	mga->regs = (uint32 *) (cfg.base[REGS] & 0xfffffff0);
	if(area_create(0x4000, 0, (void**) &(mga->regs), AREA_PHYSMAP) < 0){
		dprintf("mga_find(): CANNOT MAP REGISTERS!?");
		free(mga);
		return -1;
	}
	
	mga->fb = (uchar *) (cfg.base[FB] & 0xfffffff0);
	mga->fbsize = 0x100000;
	if(area_create(mga->fbsize, 0, (void**) &(mga->fb), AREA_PHYSMAP) < 0){
		dprintf("mga_find(): CANNOT MAP FRAMEBUFFER!?");
		free(mga);
		return -1;
	}
	
	dprintf("mga_find():   registers @ %xv, %xp",
			mga->regs,cfg.base[REGS]&0xfffffff0);
	dprintf("mga_find(): framebuffer @ %xv, %xp (sz %x)",
			mga->fb,cfg.base[FB]&0xfffffff0,mga->fbsize);
	
	dprintf("mga_find(): FIFOSTATUS = %x",RD(FIFOSTATUS));
	
	return 0;
}

static int32 mga_init(void *cookie)
{
	mga1x64 *mga = (mga1x64*) cookie;
	
	dprintf("mga_init()");
}

fb_info fb_mga1x64 = 
{
	"MGA-1x64",
	&mga_find,
	&mga_init
};

