/* $Id: //depot/blt/lib/libdl/load.c#8 $
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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <blt/libsyms.h>
#include <blt/syscall.h>
#include "dl-int.h"

weak_alias (_dlopen, dlopen)
weak_alias (_dlclose, dlclose)
weak_alias (_dlerror, dlerror)

const char *__dl_error = NULL;
static volatile char initialised = 0;
lib_t *file;

/*
 * Initialise the list of loaded images with our binary.
 *
 * XXX this will break when the executable is at least partially
 * dynamically linked.
 */
void __dlinit (void)
{
	initialised = 1;
	file = malloc (sizeof (lib_t));
	file->hdr = (elf32_hdr_t *) 0x1000;
	file->strtab = elf_find_section_hdr (file->hdr, ".strtab");
	file->strtab_data = elf_find_section_data (file->hdr, ".strtab");
	file->strtab_size = elf_section_size (file->hdr, ".strtab");
	file->symtab = elf_find_section_hdr (file->hdr, ".symtab");
	file->symtab_data = elf_find_section_data (file->hdr, ".symtab");
	file->symtab_size = elf_section_size (file->hdr, ".symtab");
/*
	file->dynstr = elf_find_section_hdr (file->hdr, ".dynstr");
	file->dynstr_data = elf_find_section_data (file->hdr, ".dynstr");
	file->dynstr_size = elf_section_size (file->hdr, ".dynstr");
	file->dynsym = elf_find_section_hdr (file->hdr, ".dynsym");
	file->dynsym_data = elf_find_section_data (file->hdr, ".dynsym");
	file->dynsym_size = elf_section_size (file->hdr, ".dynsym");
*/
	file->next = NULL;
}

/*
 * Loading is not completely straightforward.  There is only one hack here,
 * in that we guess that we will only need one page more memory than the
 * size of the library.  This seems to work on all libraries I can get my
 * hands on (both OpenBLT and Linux shared libraries).
 *
 * The memmove loop may look like a hack because you might think I'm not
 * completely parsing the program headers.  It's not because file offsets
 * and virtual addresses in an ELF file are equal, modulo 4k or larger
 * powers of two.  Read page 2-7 of the ELF specification for more
 * information.
 */
void *_dlopen (const char *filename, int flag)
{
	char *c;
	int i, size, fd, res, len;
	struct stat buf;
	lib_t *lib, *p;
	elf32_pgm_hdr_t *pgm;
	int (*fn)(void);

	if (!initialised)
		__dlinit ();

	__dl_error = NULL;
	if (_stat (filename, &buf))
	{
		errno = ENOENT;
		return NULL;
	}
	size = buf.st_size;
	size = (size & ~3) ? (size & ~3) + 0x1000 : size;
	fd = _open (filename, O_RDONLY, 0);
	if (fd < 0)
		return NULL;
	lib = malloc (sizeof (lib_t));
	lib->area = area_create (size, 0, (void **) &c, 0);
	len = 0;
	while ((res = _read (fd, c + len, 0x2000)) > 0)
		len += res;
	_close (fd);

	lib->hdr = (elf32_hdr_t *) c;
	pgm = (elf32_pgm_hdr_t *) ((unsigned int) lib->hdr + lib->hdr->e_phoff);
	for (i = lib->hdr->e_phnum - 1; i >= 0; i--)
		memmove ((void *) ((unsigned int) lib->hdr + pgm[i].p_vaddr),
			(void *) ((unsigned int) lib->hdr + pgm[i].p_offset),
			pgm[i].p_filesz);
	lib->strtab = elf_find_section_hdr (lib->hdr, ".strtab");
	lib->strtab_data = elf_find_section_data (lib->hdr, ".strtab");
	lib->strtab_size = elf_section_size (lib->hdr, ".strtab");
	lib->symtab = elf_find_section_hdr (lib->hdr, ".symtab");
	lib->symtab_data = elf_find_section_data (lib->hdr, ".symtab");
	lib->symtab_size = elf_section_size (lib->hdr, ".symtab");
	lib->dynstr = elf_find_section_hdr (lib->hdr, ".dynstr");
	lib->dynstr_data = elf_find_section_data (lib->hdr, ".dynstr");
	lib->dynstr_size = elf_section_size (file->hdr, ".dynstr");
	lib->dynsym = elf_find_section_hdr (lib->hdr, ".dynsym");
	lib->dynsym_data = elf_find_section_data (lib->hdr, ".dynsym");
	lib->dynsym_size = elf_section_size (file->hdr, ".dynsym");

	if (__dl_patchup (lib))
	{
		area_destroy (lib->area);
		free (lib);
		return NULL;
	}
	if ((fn = (int (*)(void)) (__dl_lookup_sym (lib, "_init") +
			(unsigned int) lib->hdr)))
		res = (*fn) ();
	if ((flag & ~RTLD_GLOBAL) || res)
	{
		p = file;
		while (p->next != NULL)
			p = p->next;
		p->next = lib;
		lib->next = NULL;
	}
	else
		lib->next = NULL;
	return lib;
}

int _dlclose (void *handle)
{
	lib_t *lib, *p;
	void (*fn)(void);

	lib = handle;
	if (file == lib)
		file = file->next;
	else
	{
		p = file;
		while ((p->next != lib) && (p->next != NULL))
			p = p->next;
		if (p->next != NULL)
			p->next = lib->next;
	}
	if ((fn = (void (*)(void)) (__dl_lookup_sym (lib, "_fini") +
			(unsigned int) lib->hdr)))
		(*fn) ();
	area_destroy (lib->area);
	free (handle);
	return 0;
}

const char *_dlerror (void)
{
	return __dl_error;
}

