/* $Id: //depot/blt/lib/libdl/elf.c#3 $
**
** Copyright 1998-1999 Sidney Cammeresi
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

#include <stddef.h>
#include <string.h>
#include <elf.h>
#include <blt/libsyms.h>

elf32_sec_hdr_t *elf_find_section_hdr (elf32_hdr_t *hdr, char *name)
{
	char *section_name;
	int i;
	elf32_sec_hdr_t *sec_hdr;

	sec_hdr = (elf32_sec_hdr_t *) ((unsigned int) hdr + hdr->e_shoff +
	    hdr->e_shstrndx * hdr->e_shentsize);
	section_name = (char *) ((unsigned int) hdr + sec_hdr->sh_offset);
	sec_hdr = (elf32_sec_hdr_t *) ((unsigned int) hdr + hdr->e_shoff);
	for (i = 0; i < hdr->e_shnum; i++, sec_hdr = (elf32_sec_hdr_t *)
	        ((unsigned int) sec_hdr + hdr->e_shentsize))
	    if (!strcmp (section_name + sec_hdr->sh_name, name))
	        return sec_hdr;
	return NULL;
}

void *elf_find_section_data (elf32_hdr_t *hdr, char *name)
{
	elf32_sec_hdr_t *sec_hdr;

	sec_hdr = elf_find_section_hdr (hdr, name);
	return (sec_hdr == NULL) ? NULL : (void *) ((unsigned int) hdr +
	    sec_hdr->sh_offset);
}

int elf_section_size (elf32_hdr_t *hdr, char *name)
{
	elf32_sec_hdr_t *sec_hdr;

	sec_hdr = elf_find_section_hdr (hdr, name);
	return (sec_hdr == NULL) ? 0 : sec_hdr->sh_size;
}

