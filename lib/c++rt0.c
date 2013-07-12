/* $Id: //depot/blt/lib/c++rt0.c#5 $
**
** Copyright 1998 Brian J. Swetland
** All rights reserved.
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

#include <string.h>
#include <elf.h>
#include <blt/syscall.h>
#include <blt/libsyms.h>

int main(int argc, char **argv);
static void _init(void);

char *__progname;

void _start(int argc, char **argv)
{
	_init();
	__progname = *argv;
	os_terminate (main (argc, argv));
}

#define MAXINIT 8

void __libc_init_memory(unsigned int top_of_binary,
                        unsigned int start_bss, unsigned int bss_length);

static void _init(void)
{
	char *strtab;
	int symtablen, top, i, j, p;
	elf32_hdr_t *hdr;
	elf32_sym_t *symtab;
	elf32_sec_hdr_t *last;
	init_info *inits[MAXINIT];
	
	hdr = (elf32_hdr_t *) 0x1000;
	symtab = _elf_find_section_data (hdr, ".symtab");
	strtab = _elf_find_section_data (hdr, ".strtab");
	symtablen = _elf_section_size (hdr, ".symtab") / sizeof (elf32_sym_t);
	last = (elf32_sec_hdr_t *) ((unsigned int) hdr + hdr->e_shoff +
		hdr->e_shentsize * (hdr->e_shnum - 1));
	top = (unsigned int) hdr + last->sh_offset + last->sh_size;
	
	/* Find any __init_... symbols and build a list of 'em
	** These will all be called before main(), but after
	** the .bss is zero'd
	*/
	for (j = 0, i = 0; i < symtablen; i++){
		if (!strncmp(strtab + symtab[i].st_name, "__init_", 7) &&
			(symtab[i].st_shndx != SHN_UNDEF)){
			inits[j++] = (init_info *) symtab[i].st_value;
			if(j == MAXINIT) break;
		}
	}
	
	__libc_init_memory((unsigned int) top, (unsigned int)
		_elf_find_section_data (hdr, ".bss"), _elf_section_size (hdr, ".bss"));

	for(p=0;p<5;p++){	
		for(i = 0; i < j; i++){
			if(inits[i]->priority == p) inits[i]->func();
		}
	}
}

elf32_sec_hdr_t *_elf_find_section_hdr (elf32_hdr_t *hdr, char *name)
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

void *_elf_find_section_data (elf32_hdr_t *hdr, char *name)
{
	elf32_sec_hdr_t *sec_hdr;

	sec_hdr = _elf_find_section_hdr (hdr, name);
	return (sec_hdr == NULL) ? NULL : (void *) ((unsigned int) hdr +
	    sec_hdr->sh_offset);
}

int _elf_section_size (elf32_hdr_t *hdr, char *name)
{
	elf32_sec_hdr_t *sec_hdr;

	sec_hdr = _elf_find_section_hdr (hdr, name);
	return (sec_hdr == NULL) ? 0 : sec_hdr->sh_size;
}

/****** this crap is to make ld happy when we build shared executables ******/

int ___brk_addr;
int __environ;

void __attribute__ ((noreturn)) abort ()
{
	for (;;) ;
}

void atexit ()
{
}

/********************************* end crap *********************************/

