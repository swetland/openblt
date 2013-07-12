/* $Id: //depot/blt/boot/boot.c#3 $
**
** Copyright 1998 Brian J. Swetland
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

/* Stage Two Bootloader -- init paged memory, map the kernel, and
 * relocate to k_start()
 */
#define noDIAG

#include "types.h"
#include "boot.h"
#include <multiboot.h>

#ifdef DIAG
#include "../include/blt/conio.h"
#endif

void (*k_start)(void);

struct _kinfo
{
    uint32 memsize;
    uint32 entry_ebp;
    boot_dir *bd;
    unsigned char *param;
} *kinfo = (struct _kinfo *) 0x00102000;

static unsigned int multiboot[] __attribute__ ((__section__ (".text"))) =
{
	0x1badb002,
	0x00010002,
	(unsigned int) 0 - 0x1badb002 - 0x00010002,
	(unsigned int) multiboot,
	0x100000,
	0x14a000,
	0x14a000,
	0x101074
};

static void enable_paging (void)
{
	__asm__ ("movl $0x80000001, %eax ; mov %eax, %cr0");
}
	
void bootstrap(boot_dir *bd, uint32 memsize, uint32 entry_ebp, char *p)
{
    uint32 *flat;
    uint32 _cr3;
    int i;

    kinfo->memsize = memsize;
    kinfo->entry_ebp = entry_ebp;
    kinfo->bd = bd;
    kinfo->param = p;
    
#ifdef DIAG    
    con_init();
    cprintf("memsize = %x, bdir @ %x, ebp = %x\n",memsize,(int)bd,entry_ebp);
    cprintf("name[0] = %s\n",bd->bd_entry[0].be_name);
    cprintf("name[1] = %s\n",bd->bd_entry[1].be_name);
    cprintf("name[2] = %s\n",bd->bd_entry[2].be_name);
#endif    
      
    flat = (void*) ((4096*(memsize/4096)) - 4096*3);
    
    for(i=0;i<1024;i++){
        flat[i] = 0;
        flat[1024+i] = 4096*i | 3;        
        flat[2048+i] = i > bd->bd_entry[2].be_size ? 0 : /* XXX! EEK! */
            ( (bd->bd_entry[2].be_offset*4096+0x100000) + 4096*i) | 3;

#ifdef DIAG
        if(flat[2048+i]){
            cprintf("%x -> %x\n",
                    bd->bd_entry[2].be_offset*4096+0x100000 + 4096*i,
                    0x8000000+4096*i);
        }
#endif                    
    }

    k_start = (void (*)(void)) 0x80000000 + (bd->bd_entry[2].be_code_ventr);

        /* point the pdir at ptab1 and ptab2 */
    flat[0] = (uint32) &flat[1024] | 3;
    flat[512] = (uint32) &flat[2048] | 3;

        /* map the pdir, ptab1, ptab2 starting at 0x803FD000 */
    flat[2048+1023] = ((uint32) flat + 2*4096) | 3;
    flat[2048+1022] = ((uint32) flat + 1*4096) | 3;
    flat[2048+1021] = ((uint32) flat) | 3;

    _cr3 = (uint32) flat;
    __asm__ ("mov %0, %%cr3"::"r" (_cr3));
	enable_paging ();

#ifdef DIAG
    flat = 0x803fd000;

    for(i=0;i<3000000;i++);
    for(i=0;i<3000000;i++);
    if(flat[0] & 0xffffff00 != 0x007fe000) cprintf("danger will robinson! (a)\n");
    if(flat[512] & 0xffffff00 != 0x007ff000) cprintf("danger will robinson! (b)\n");
   	cprintf("a = %x b = %x\n",flat[0],flat[512]); 
    for(i=0;i<3000000;i++);
    cprintf("go! go! go!\n\n");
    for(i=0;i<3000000;i++);

    
/*    for(i=0;i<80*4;i++){
        cprintf(".");
        *((char *) 0x100000+i) = 'X';
    }
    asm("hlt"); */
#endif
    
    k_start();
}

void grub_bootstrap (multiboot_info *minfo)
{
	const static unsigned int temp_gdt[] = { 0x00000000, 0x00000000,
		0x0000ffff, 0x00cf9a00, 0x0000ffff, 0x00cf9200 };
	unsigned int i[2];

	i[0] = 24 << 16;
	i[1] = (unsigned int) temp_gdt;
	asm ("lgdt (%0)" : : "p" (((char *) i) + 2));
	adjust_seg_regs ();
	bootstrap ((boot_dir *) 0x100000, (minfo->mem_lower + minfo->mem_upper +
		384) * 1024, 0, minfo->cmdline);
}

