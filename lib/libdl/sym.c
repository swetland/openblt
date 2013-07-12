/* $Id: //depot/blt/lib/libdl/sym.c#8 $
**
** Copyright 1999 Sidney Cammeresi
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
#include <string.h>
#include <elf.h>
#include <dlfcn.h>
#include <blt/libsyms.h>
#include "dl-int.h"

weak_alias (_dlsym, dlsym)

extern lib_t *file;

void *_dlsym (void *handle, const char *symbol)
{
	int i;
	lib_t *lib;

	lib = handle;
	for (i = 0; i < lib->dynsym_size / sizeof (elf32_sym_t); i++)
		if (!strcmp (lib->dynstr_data + lib->dynsym_data[i].st_name, symbol))
			return (void *) ((unsigned int) lib->hdr +
				lib->dynsym_data[i].st_value);
	for (i = 0; i < lib->symtab_size / sizeof (elf32_sym_t); i++)
		if (!strcmp (lib->strtab_data + lib->symtab_data[i].st_name, symbol))
			return (void *) ((unsigned int) lib->hdr +
				lib->symtab_data[i].st_value);
	return NULL;
}

unsigned int __dl_lookup_sym (lib_t *lib, const char *name)
{
	int i;

	for (i = 0; i < lib->symtab_size / sizeof (elf32_sym_t); i++)
		if (!strcmp (lib->strtab_data + lib->symtab_data[i].st_name, name))
			return lib->symtab_data[i].st_value;
	return 0;
}

int __dl_patch_section (lib_t *lib, elf32_rel_t *rel, int size)
{
	char *name;
	int i;
	unsigned int *word, sym = 0;
	lib_t *p;

	for (i = 0; i < size; i++)
	{
		//_printf ("patching at %x, type %d\n", rel[i].r_offset,
		//	ELF32_R_TYPE (rel[i].r_info));
		word = (unsigned int *) ((unsigned int) lib->hdr + rel[i].r_offset);
		if (ELF32_R_SYM (rel[i].r_info))
		{
			name = lib->dynstr_data +
				lib->dynsym_data[ELF32_R_SYM (rel[i].r_info)].st_name;
			if (!(sym = __dl_lookup_sym (lib, name)))
			{
				p = file;
				while ((p != NULL) && !sym)
					if (!(sym = __dl_lookup_sym (p, name)))
						p = p->next;
				if (!sym)
				{
					//_printf ("unresolved symbol %s\n", name);
					return -1;
				}
				if (p != file)
					sym += (unsigned int) p->hdr;
			}
			else
				sym += (unsigned int) lib->hdr;
		}
		switch (ELF32_R_TYPE (rel[i].r_info))
		{
			case R_386_32:
				*word += sym;
				break;

			case R_386_PC32:
				*word += sym - (unsigned int) word;
				break;

			case R_386_RELATIVE:
				*word += (unsigned int) lib->hdr;
				break;

			default:
				//_printf ("unknown relocation type %d; crashing soon...\n",
				//	ELF32_R_TYPE (rel[i].r_info));
				break;
		}
	}
	return 0;
}

int __dl_patchup (lib_t *lib)
{
	int size;
	elf32_rel_t *rel;

	rel = (elf32_rel_t *) elf_find_section_data (lib->hdr, ".rel.text");
	size = elf_section_size (lib->hdr, ".rel.text") / sizeof (elf32_rel_t);
	if (__dl_patch_section (lib, rel, size))
		return -1;
	rel = (elf32_rel_t *) elf_find_section_data (lib->hdr, ".rel.data");
	size = elf_section_size (lib->hdr, ".rel.data") / sizeof (elf32_rel_t);
	if (__dl_patch_section (lib, rel, size))
		return -1;
	return 0;
}

