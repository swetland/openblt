/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _SMP_H_
#define _SMP_H_

#include <blt/os.h>

#ifndef __SMP__
#  ifndef __SMP_ONLY__
#    define UP_OR_SMP(u,s)     { u; }
#  else
#    error defined __SMP_ONLY__ without __SMP__
#  endif
#else
#  if !defined (__SMP_ONLY__)
#    define UP_OR_SMP(u,s)     if (!smp_configured) { u; } else { s; }
#  else
#    define UP_OR_SMP(u,s)     { s; }
#  endif
#endif

#define CPUID_GEN_EBX      0x756e6547 /* Genu */
#define CPUID_GEN_ECX      0x49656369 /* ineI */
#define CPUID_GEN_EDX      0x6c65746e /* ntel */

#define MP_FLT_SIGNATURE   (('_' << 24) | ('P' << 16) | ('M' << 8) | ('_'))
#define MP_CTH_SIGNATURE   (('P' << 24) | ('C' << 16) | ('M' << 8) | ('P'))

#define MP_DEF_LOCAL_APIC  0xfee00000 /* default local apic address */
#define MP_DEF_IO_APIC     0xfec00000 /* default i/o apic address */

#define APIC_DM_INIT       (5 << 8)
#define APIC_DM_STARTUP    (6 << 8)
#define APIC_LEVEL_TRIG    (1 << 14)
#define APIC_ASSERT        (1 << 13)

#define APIC_ENABLE        0x100
#define APIC_FOCUS         (~(1 << 9))
#define APIC_SIV           (0xff)

#define MP_EXT_PE          0
#define MP_EXT_BUS         1
#define MP_EXT_IO_APIC     2
#define MP_EXT_IO_INT      3
#define MP_EXT_LOCAL_INT   4

#define MP_EXT_PE_LEN          20
#define MP_EXT_BUS_LEN         8
#define MP_EXT_IO_APIC_LEN     8
#define MP_EXT_IO_INT_LEN      8
#define MP_EXT_LOCAL_INT_LEN   8

#define SET_APIC_DEST(x)       (x << 24)

#ifndef __ASM__

#include "kernel.h"

extern unsigned int *apic, *apic_virt;

#define SMP_ERROR(x)       { kprintf (x); return; }

#define APIC_ID            ((unsigned int *) ((unsigned int) apic_virt + 0x020))
#define APIC_VERSION       ((unsigned int *) ((unsigned int) apic_virt + 0x030))
#define APIC_TPRI          ((unsigned int *) ((unsigned int) apic_virt + 0x080))
#define APIC_EOI           ((unsigned int *) ((unsigned int) apic_virt + 0x0b0))
#define APIC_SIVR          ((unsigned int *) ((unsigned int) apic_virt + 0x0f0))
#define APIC_ESR           ((unsigned int *) ((unsigned int) apic_virt + 0x280))
#define APIC_ICR1          ((unsigned int *) ((unsigned int) apic_virt + 0x300))
#define APIC_ICR2          ((unsigned int *) ((unsigned int) apic_virt + 0x310))
#define APIC_LVTT          ((unsigned int *) ((unsigned int) apic_virt + 0x320))
#define APIC_LINT0         ((unsigned int *) ((unsigned int) apic_virt + 0x350))
#define APIC_LVT3          ((unsigned int *) ((unsigned int) apic_virt + 0x370))
#define APIC_ICRT          ((unsigned int *) ((unsigned int) apic_virt + 0x380))
#define APIC_CCRT          ((unsigned int *) ((unsigned int) apic_virt + 0x390))
#define APIC_TDCR          ((unsigned int *) ((unsigned int) apic_virt + 0x3e0))

#define APIC_TDCR_2        0x00
#define APIC_TDCR_4        0x01
#define APIC_TDCR_8        0x02
#define APIC_TDCR_16       0x03
#define APIC_TDCR_32       0x08
#define APIC_TDCR_64       0x09
#define APIC_TDCR_128      0x0a
#define APIC_TDCR_1        0x0b

#define APIC_LVTT_VECTOR   0x000000ff
#define APIC_LVTT_DS       0x00001000
#define APIC_LVTT_M        0x00010000
#define APIC_LVTT_TM       0x00020000

#define APIC_LVT_DM        0x00000700
#define APIC_LVT_IIPP      0x00002000
#define APIC_LVT_TM        0x00008000
#define APIC_LVT_M         0x00010000
#define APIC_LVT_OS        0x00020000

#define APIC_TPR_PRIO      0x000000ff
#define APIC_TPR_INT       0x000000f0
#define APIC_TPR_SUB       0x0000000f

#define APIC_SVR_SWEN      0x00000100
#define APIC_SVR_FOCUS     0x00000200

#define APIC_DEST_STARTUP  0x00600

#define LOPRIO_LEVEL       0x00000010

#define APIC_DEST_FIELD          (0)
#define APIC_DEST_SELF           (1 << 18)
#define APIC_DEST_ALL            (2 << 18)
#define APIC_DEST_ALL_BUT_SELF   (3 << 18)

#define IOAPIC_ID          0x0
#define IOAPIC_VERSION     0x1
#define IOAPIC_ARB         0x2
#define IOAPIC_REDIR_TABLE 0x10

#define IPI_CACHE_FLUSH    0x40
#define IPI_INV_TLB        0x41
#define IPI_INV_PTE        0x42
#define IPI_INV_RESCHED    0x43
#define IPI_STOP           0x44

typedef struct
{
	unsigned int signature;       /* "PCMP" */
	unsigned short table_len;     /* length of this structure */
	unsigned char mp_rev;         /* spec supported, 1 for 1.1 or 4 for 1.4 */
	unsigned char checksum;       /* checksum, all bytes add up to zero */
	char oem[8];                  /* oem identification, not null-terminated */
	char product[12];             /* product name, not null-terminated */
	void *oem_table_ptr;          /* addr of oem-defined table, zero if none */
	unsigned short oem_len;       /* length of oem table */
	unsigned short num_entries;   /* number of entries in base table */
	unsigned int apic;            /* address of apic */
	unsigned short ext_len;       /* length of extended section */
	unsigned char ext_checksum;   /* checksum of extended table entries */
} mp_config_table;

typedef struct
{
	unsigned int signature;       /* "_MP_" */
	mp_config_table *mpc;         /* address of mp configuration table */
	unsigned char mpc_len;        /* length of this structure in 16-byte units */
	unsigned char mp_rev;         /* spec supported, 1 for 1.1 or 4 for 1.4 */
	unsigned char checksum;       /* checksum, all bytes add up to zero */
	unsigned char mp_feature_1;   /* mp system configuration type if no mpc */
	unsigned char mp_feature_2;   /* imcrp */
	unsigned char mp_feature_3, mp_feature_4, mp_feature_5; /* reserved */
} mp_flt_ptr;

typedef struct
{
	unsigned char type;
	unsigned char apic_id;
	unsigned char apic_version;
	unsigned char cpu_flags;
	unsigned int signature;       /* stepping, model, family, each four bits */
	unsigned int feature_flags;
	unsigned int res1, res2;
} mp_ext_pe;

typedef struct
{
	unsigned char type;
	unsigned char ioapic_id;
	unsigned char ioapic_version;
	unsigned char ioapic_flags;
	unsigned int *addr;
} mp_ext_ioapic;

extern char smp_num_cpus, *trampoline, *trampoline_end;
extern volatile char smp_configured, smp_begun;

int smp_check_cpu (void);
void smp_init (void);
void smp_cpu_setup (void);
void smp_cpu_ready (void);
void smp_begin (void);
void ioapic_init (void);

/* these are more interesting functions for use by the rest of the kernel */

int smp_my_cpu (void);

void ipi_dest (int num, int pe);
void ipi_self (int num);
void ipi_all (int num);
void ipi_all_but_self (int num);

volatile unsigned int ioapic_read (unsigned int offset);
void ioapic_write (unsigned int offset, unsigned int data);

#endif /* __ASM__ */

#endif /* __SMP__ */

