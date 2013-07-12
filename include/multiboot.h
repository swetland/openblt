/* $Id$
**
** Copyright 1999 Sidney Cammeresi.
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 
** 1. Redistributions of source code must retain the above copyright notice,
**    this list of conditions and the following disclaimer.
** 
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS' AND ANY EXPRESS OR IMPLIED
** WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
** NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _MULTIBOOT_H_
#define _MULTIBOOT_H_

#define MBI_MEM               0x00000001
#define MBI_BOOTDEV           0x00000002
#define MBI_CMDLINE           0x00000004
#define MBI_MODS              0x00000008
#define MBI_SYMS_AOUT         0x00000010
#define MBI_SYMS_ELF          0x00000020
#define MBI_MMAP              0x00000040

typedef struct
{
	unsigned int flags, mem_lower, mem_upper, boot_device;
	char *cmdline;
	unsigned int mods_count, mods_addr;
	union
	{
		struct
		{
			unsigned int tabsize, strsize, addr, _pad_;
		} sym_aout;
		struct
		{
			unsigned int num, size, addr, shndx;
		} sym_elf;
	} mb_sym_un;
	unsigned int mmap_length, mmap_addr;
} multiboot_info;


#endif

