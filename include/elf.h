/* $Id: //depot/blt/include/elf.h#4 $
**
** Copyright 1998 Sidney Cammeresi
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
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

#ifndef __ELF_H__
#define __ELF_H__

#define EI_NIDENT 16

/* ELF header */
typedef struct
{
	unsigned char e_ident [EI_NIDENT];
	unsigned short e_type, e_machine;
	unsigned int e_version, e_entry, e_phoff, e_shoff, e_flags;
	unsigned short e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum,
			e_shstrndx;
} elf32_hdr_t;

#define ELF_MAGIC        0x464c457f

/* e_ident indices */
#define EI_MAG0          0 /* 0x7F */
#define EI_MAG1          1 /* 'E' */
#define EI_MAG2          2 /* 'L' */
#define EI_MAG3          3 /* 'F' */
#define EI_CLASS         4 /* file class */
#define EI_DATA          5 /* data encoding */
#define EI_VERSION       6 /* file version */
#define EI_PAD           7 /* start of padding bytes */

/* e_ident data */
#define ELFCLASSNONE     0 /* invalid class */
#define ELFCLASS32       1 /* 32-bit objects */
#define ELFCLASS64       2 /* 64-bit objects */
#define ELFDATANONE      0 /* invalid data encoding */
#define ELFDATA2LSB      1 /* little-endian */
#define ELFDATA2MSB      2 /* big-endian */

/* e_type */
#define ET_NONE          0 /* no file type */
#define ET_REL           1 /* relocatable file */
#define ET_EXEC          2 /* executable file */
#define ET_DYN           3 /* shared object file */
#define ET_CORE          4 /* core file */
#define ET_LOPROC   0xFF00 /* beginning processor specific range */
#define ET_HIPROC   0xFFFF /* end of processor specific range */

/* e_machine */
#define EM_NONE          0 /* no machine */
#define EM_M32           1 /* AT&T WE 32100 */
#define EM_SPARC         2 /* SPARC */
#define EM_386           3 /* Intel 386 */
#define EM_68K           4 /* Motorola 68000 */
#define EM_88K           5 /* Motorola 88000 */
#define EM_486           6 /* Intel 486 - unused */
#define EM_860           7 /* Intel 80860 */
#define EM_MIPS          8 /* MIPS R3000 big-endian */
#define EM_MIPS_RS4_BE  10 /* MIPS R4000 big-endian */
#define EM_SPARC64      11 /* SPARC V9 64-bit */
#define EM_HPPA         15 /* HP PA-RISC */
#define EM_PPC          20 /* PowerPC */

/* e_version */
#define EV_NONE          0 /* invalid version */
#define EV_CURRENT       1 /* current version */

/* section header */
typedef struct
{
	unsigned int sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size,
			sh_link, sh_info, sh_addralign, sh_entsize;
} elf32_sec_hdr_t;

/* sh_type */
#define SHT_NULL         0 /* section header is inactive */
#define SHT_PROGBITS     1 /* program specific information */
#define SHT_SYMTAB       2 /* symbol table, only one of these */
#define SHT_STRTAB       3 /* string table */
#define SHT_RELA         4 /* relocation entries with addends - ? */
#define SHT_HASH         5 /* symbol hash table, only one of these */
#define SHT_DYNAMIC      6 /* dynamic linking information, only one of these */
#define SHT_NOTE         7 /* file marking information */
#define SHT_NOBITS       8 /* occupies no space, but similar to SHT_PROGBITS */
#define SHT_REL          9 /* relocation entries without addends - ? */
#define SHT_SHLIB       10 /* reserved and unspecified */
#define SHT_DYNSYM      11 /* symbol table for dynamic linking, only one */

#define SHF_WRITE        1 /* section should be writable */
#define SHF_ALLOC        2 /* section occupies memory during execution */
#define SHF_EXECINSTR    4 /* section contains executable text */
#define SHF_MASKPROC     0xF0000000 /* reserved for machine specific data */

/* program header */
typedef struct
{
	unsigned int p_type, p_offset, p_vaddr, p_paddr, p_filesz, p_memsz, p_flags,
			p_align;
} elf32_pgm_hdr_t;

/* p_type */
#define PT_NULL          0 /* array element unused */
#define PT_LOAD          1 /* loadable segment */
#define PT_DYNAMIC       2 /* contains dynamic linking information */
#define PT_INTERP        3 /* location and size of path to interpreter */
#define PT_NOTE          4 /* location and size of auxillary information */
#define PT_SHLIB         5 /* reserved and unspecified */
#define PT_PHDR          6 /* location and size of program header table */
#define PT_LOPROC        0x70000000 /* reserved for processor specific use */
#define PT_HIPROC        0x7FFFFFFF /* end of range */

typedef struct
{
	int d_tag;
	union
	{
		unsigned int d_val, d_ptr;
	} d_un;
} elf32_dyn_t;

/* d_tag */
#define DT_NULL           0 /* ignored, but mandatory */
#define DT_NEEDED         1 /* name of needed library */
#define DT_PLTRELSZ       2 /* size of plt */
#define DT_PLTGOT         3
#define DT_HASH           4
#define DT_STRTAB         5 /* string table with symbol and library names */
#define DT_SYMTAB         6
#define DT_RELA           7
#define DT_REL           17 /* reloc entries for data */
#define DT_RELSZ         18
#define DT_RELENT        19
#define DT_PLTREL        20
#define DT_JMPREL        23 /* reloc entries associated with plt */

/* relocation entry without addend */
typedef struct
{
	unsigned int r_offset, r_info;
} elf32_rel_t;

/* relocation entry with addend */
typedef struct
{
	unsigned int r_offset, r_info, r_addend;
} elf32_rela_t;

#define ELF32_R_SYM(i)    ((i) >> 8)
#define ELF32_R_TYPE(i)   ((unsigned char) (i))
#define ELF32_R_INFO(i,j) (((i) << 8) + (unsigned char) (j))

#define R_386_NONE        0 /* none */
#define R_386_32          1
#define R_386_PC32        2
#define R_386_GOT32       3
#define R_386_PLT32       4
#define R_386_COPY        5
#define R_386_GLOB_DAT    6
#define R_386_JMP_SLOT    7 /* location of procedure linkage table entry */
#define R_386_RELATIVE    8
#define R_386_GOTOFF      9
#define R_386_GOTPC      10

typedef struct
{
	unsigned int st_name, st_value, st_size;
	unsigned char st_info, st_other;
	unsigned short st_shndx;
} elf32_sym_t;

#define ELF32_ST_BIND(i)    ((i) >> 4)
#define ELF32_ST_TYPE(i)    ((i) & 0xF)
#define ELF32_ST_INFO(i,j)  (((i) << 4) + ((j) & 0xF))

#define STT_NOTYPE          0 /* type not specified */
#define STT_OBJECT          1 /* symbol references data */
#define STT_FUNC            2 /* symbol represents text */
#define STT_SECTION         3 /* symbol is a section */
#define STT_FILE            4 /* object file */
#define STT_LOPROC         13 /* reserved for processor */
#define STT_MEDPROC        14 /* reserved for processor */
#define STT_HIPROC         15 /* reserved for processor */

#define SHN_UNDEF           0
#define SHN_LORESERVE  0xff00
#define SHN_LOPROC     0xff00
#define SHN_HIPROC     0xff1f
#define SHN_ABS        0xfff1
#define SHN_COMMON     0xfff2
#define SHN_HIRESERVE  0xffff

#ifdef __cplusplus
extern "C" {
#endif

	elf32_sec_hdr_t *_elf_find_section_hdr (elf32_hdr_t *hdr, char *name);
	elf32_sec_hdr_t *elf_find_section_hdr (elf32_hdr_t *hdr, char *name);
	void *_elf_lookup_sym (int filenum, const char *name);
	void *elf_lookup_sym (int filenum, const char *name);
	void *_elf_find_section_data (elf32_hdr_t *hdr, char *name);
	void *elf_find_section_data (elf32_hdr_t *hdr, char *name);
	int _elf_section_size (elf32_hdr_t *hdr, char *name);
	int elf_section_size (elf32_hdr_t *hdr, char *name);

#ifdef __cplusplus
}
#endif


#endif

