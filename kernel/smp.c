/* $Id: //depot/blt/kernel/smp.c#6 $
**
** Copyright 1998 Sidney Cammeresi
** All rights reserved.
** Copyright (c) 1996, by Steve Passe
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

#include "kernel.h"
#include "i386/io.h"
#include "init.h"
#include "smp.h"

#undef SMP_DEBUG

#ifdef SMP_DEBUG
#define SMP_PRINTF(f,a...)     kprintf (f, ## a)
#else
#define SMP_PRINTF(f,a...)
#endif

const char *cpu_family[] __initdata__ = { "", "", "", "", "Intel 486",
	"Intel Pentium", "Intel Pentium Pro", "Intel Pentium II" };
const unsigned int temp_gdt[] = {0x00000000, 0x00000000,  /* null descriptor */
                                 0x0000ffff, 0x00cf9a00,  /* kernel text */
                                 0x0000ffff, 0x00cf9200 };/* kernel data */

char mp_num_def_config, smp_num_cpus = 1, smp_num_running_cpus;
volatile char smp_cpus_ready[BLT_MAX_CPUS], smp_configured = 0, smp_begun = 0;
unsigned int cpuid_max_level, cpuid_eax, cpuid_edx, apic_addr,
	cpu_apic_id[BLT_MAX_CPUS], cpu_os_id[BLT_MAX_CPUS],
	cpu_apic_version[BLT_MAX_CPUS], *apic, *apic_virt, *ioapic;

mp_flt_ptr *mp_config;

extern aspace_t *flat;
extern void (*flush) (void);

volatile char ipi_lock = 0;

volatile unsigned int apic_read (unsigned int *addr)
{
	return *addr;
}

void apic_write (unsigned int *addr, unsigned int data)
{
	*addr = data;
}

/* XXX - hack until we can figure this out */
int bus_clock (void)
{
	return 66000000;
}

/********** adapted from FreeBSD's i386/i386/mpapic.c **********/

void __init__ apic_set_timer (int value)
{
	unsigned long lvtt;
	long ticks_per_us;

	/* calculate divisor and count from value */
	apic_write (APIC_TDCR, APIC_TDCR_1);
	ticks_per_us = bus_clock () / 1000000;

	/* configure timer as one-shot */
	lvtt = apic_read (APIC_LVTT) & ~(APIC_LVTT_VECTOR | APIC_LVTT_DS |
		APIC_LVTT_M | APIC_LVTT_TM) | APIC_LVTT_M | 0xff;
	apic_write (APIC_LVTT, lvtt);
	apic_write (APIC_ICRT, value * ticks_per_us);
}

/************************* end FreeBSD *************************/

int apic_read_timer (void)
{
	return apic_read (APIC_CCRT);
}

void __init__ u_sleep (int count)
{
	int i;

	apic_set_timer (count);
	while (i = apic_read (APIC_CCRT)) ;
}

void __init__ mp_probe (int base, int limit)
{
	unsigned int i, *ptr;

	for (ptr = (unsigned int *) base; (unsigned int) ptr < limit; ptr++)
		if (*ptr == MP_FLT_SIGNATURE)
		{
			SMP_PRINTF ("smp: found floating pointer structure at %x", ptr);
			mp_config = (mp_flt_ptr *) ptr;
			return;
		}
}

int __init__ mp_verify_data (void)
{
	char *ptr, total;
	int i;

	/* check signature */
	if (mp_config->signature != MP_FLT_SIGNATURE)
		return 0;

	/* compute floating pointer structure checksum */
	ptr = (unsigned char *) mp_config;
	for (i = total = 0; i < (mp_config->mpc_len * 16); i++)
		total += ptr[i];
	if (total)
		return 0;
	kprintf ("smp: CHECKSUMMED");

	/* compute mp configuration table checksum if we have one*/
	if ((ptr = (unsigned char *) mp_config->mpc) != NULL)
	{
		for (i = total = 0; i < sizeof (mp_config_table); i++)
			total += ptr[i];
		if (total)
			return 0;
	}
	else
		kprintf ("smp: no configuration table\n");

	return 1;
}

void __init__ mp_do_config (void)
{
	char *ptr;
	int i;
	mp_ext_pe *pe;
	mp_ext_ioapic *io;

	/*
	 * we are not running in standard configuration, so we have to look through
	 * all of the mp configuration table crap to figure out how many processors
	 * we have, where our apics are, etc.
	 */
	smp_num_cpus = 0;

	/* print out our new found configuration. */
	ptr = (char *) &(mp_config->mpc->oem[0]);
	kprintf ("smp: oem id: %c%c%c%c%c%c%c%c product id: "
		"%c%c%c%c%c%c%c%c%c%c%c%c", ptr[0], ptr[1], ptr[2], ptr[3], ptr[4],
		ptr[5], ptr[6], ptr[7], ptr[8], ptr[9], ptr[10], ptr[11], ptr[12],
		ptr[13], ptr[14], ptr[15], ptr[16], ptr[17], ptr[18], ptr[19],
		ptr[20]);
	SMP_PRINTF ("smp: base table has %d entries, extended section %d bytes",
		mp_config->mpc->num_entries, mp_config->mpc->ext_len);
	apic = (unsigned int *) mp_config->mpc->apic;

	ptr = (char *) ((unsigned int) mp_config->mpc + sizeof (mp_config_table));
	for (i = 0; i < mp_config->mpc->num_entries; i++)
		switch (*ptr)
		{
			case MP_EXT_PE:
				pe = (mp_ext_pe *) ptr;
				cpu_apic_id[smp_num_cpus] = pe->apic_id;
				cpu_os_id[pe->apic_id] = smp_num_cpus;
				cpu_apic_version [smp_num_cpus] = pe->apic_version;
				kprintf ("smp: cpu#%d: %s, apic id %d, version %d%s",
					smp_num_cpus++, cpu_family[(pe->signature & 0xf00) >> 8],
					pe->apic_id, pe->apic_version, (pe->cpu_flags & 0x2) ?
					", BSP" : "");
				ptr += 20;
				break;
			case MP_EXT_BUS:
				ptr += 8;
				break;
			case MP_EXT_IO_APIC:
				io = (mp_ext_ioapic *) ptr;
				ioapic = io->addr;
				SMP_PRINTF ("smp: found io apic with apic id %d, version %d",
					io->ioapic_id, io->ioapic_version);
				ptr += 8;
				break;
			case MP_EXT_IO_INT:
				ptr += 8;
				break;
			case MP_EXT_LOCAL_INT:
				ptr += 8;
				break;
		}

	kprintf ("smp: apic @ 0x%x, i/o apic @ 0x%x, total %d processors detected",
		(unsigned int) apic, (unsigned int) ioapic, smp_num_cpus);
}

/* FIXME - this is a very long stretch that needs to be broken up. */
void __init__ smp_init (void)
{
	unsigned char *ptr, init_val;
	unsigned int i, j, config, num_startups, *new_stack, *dir, *table, *page,
		*common;

	SMP_PRINTF ("smp: initialising");
	/*
	 * first let's check the kind of processor this is since only intel chips
	 * support the MP specification.
	 */
#if 0
	/* FIXME - this check doesn't work */
	if (!smp_check_cpu ())
		SMP_ERROR ("smp: processor is not Genuine Intel or is a 386 or 486\n");
#else
	j = smp_check_cpu ();
	SMP_PRINTF ("smp: cpu is %x", j);
#endif

	/*
	 * scan the three spec-defined regions for the floating pointer
	 * structure.  if we find one, copy its address into mp_config,
	 * otherwise, abort smp initialisation.  after that, check the
	 * integrity of the structures.
	 */
	mp_config = (mp_flt_ptr *) 0x1000; /* small hack */
	/* mp_probe (0, 0x400); */
	mp_probe (0x9fc00, 0xa0000);
	mp_probe (0xf0000, 0x100000);
	if (mp_config == (mp_flt_ptr *) 0x1000) /* if unchanged */
		SMP_ERROR ("smp: no floating pointer structure detected\n");

#if 0
	/* FIXME - this check doesn't work */
	if (!smp_verify_data ())
		SMP_ERROR ("smp: bad configuration information\n");
	kprintf ("smp: it's all good\n");
#endif

	/*
	 * whee, we're running on an smp machine with valid configuration
	 * information.  print out some of this new intelligence.
	 */
	kprintf ("smp: intel mp version %s, %s", (mp_config->mp_rev == 1) ? "1.1" :
		"1.4", (mp_config->mp_feature_2 & 0x80) ?
		"imcr and pic compatibility mode." : "virtual wire compatibility mode.");

	if (!mp_config->mpc)
	{
		/* this system conforms to one of the default configurations */
		mp_num_def_config = mp_config->mp_feature_1;
		kprintf ("smp: standard configuration %d", mp_num_def_config);
		smp_num_cpus = 2;
		cpu_apic_id[0] = 0;
		cpu_apic_id[1] = 1;
		apic = (unsigned int *) 0xfee00000;
		ioapic = (unsigned int *) 0xfec00000;
		kprintf ("smp: WARNING: standard configuration code is untested");
	else
	{
		/*
		 * we don't have a default configuration, so now we have to locate all
		 * of our hardware.  boy, do we have a lot of work to do.
		 */
		SMP_PRINTF ("smp: not a standard configuration");
		mp_num_def_config = 0;
		mp_do_config ();
	}
	smp_configured = 1;

	/* set up the apic */
	aspace_maphi (flat, 0xfee00, 0x3fc, 1, 0x13);
	apic_virt = (unsigned int *) 0x803fc000;
	asm ("mov %0, %%cr3" : : "r" (_cr3));

	config = apic_read ((unsigned int *) APIC_SIVR) & APIC_FOCUS | APIC_ENABLE |
		0xff; /* set spurious interrupt vector to 0xff */
	apic_write (APIC_SIVR, config);
	config = apic_read (APIC_TPRI) & 0xffffff00; /* accept all interrupts */
	apic_write (APIC_TPRI, config);

	apic_read (APIC_SIVR);
	apic_write (APIC_EOI, 0);

	config = apic_read (APIC_LVT3) & 0xffffff00 | 0xfe; /* XXX - set vector */
	apic_write (APIC_LVT3, config);

	/* grab a page at 0x9000 for communicating with the aps */
	aspace_map (flat, 0x9, 0x9, 1, 3);
	common = (unsigned int *) 0x9000;
	for (i = 0; i < 6; i++)
		common [i + 1] = temp_gdt [i];
	*((unsigned int *) 0x9024) = _cr3;
	aspace_map (flat, 0xa, 0xa, 1, 3);
	ptr = (unsigned char *) 0xa000;
	for (j = 0; j < ((unsigned int) &trampoline_end - (unsigned int)
			&trampoline + 1); j++)
		ptr[j] = ((unsigned char *) &trampoline)[j];

	/*
	 * okay, we're ready to go.  boot all of the ap's now.  we loop through
	 * using the kernel pe id numbers which were set up in smp_do_config ()
	 *
	 * XXX - we assume the bsp is the first detected.  trying to boot it here
	 * would be, ahem, bad.
	 */
	for (i = 1; i < smp_num_cpus; i++)
	{
		kprintf ("smp: booting cpu#%d with stack 0x%x", i, i * 0x1000);

		/* map stacks and copy bootstrap code onto them */
		aspace_map (flat, i, i, 1, 3);
		ptr = (unsigned char *) (0x1000 * i);
		for (j = 0; j < ((unsigned int) &trampoline_end - (unsigned int)
				&trampoline + 1); j++)
			ptr[j] = ((unsigned char *) &trampoline)[j];

		/*
		 * modify the bootstrap code, specifically, the third highest order
		 * byte of the ljmp.  we need to do this if we have more than two
		 * processors, i.e. more than one ap.
		 */
		ptr = (char *) flush;
		*(ptr - 5) = i << 4;

		/* write the location of the stack into a fixed spot in memory */
		*common = 0x1000 * i;

		/* set shutdown code and warm reset vector */
		ptr = (unsigned char *) 0xf;
		*ptr = 0xa;
		ptr = (unsigned char *) 0x467;
		*ptr = 0x1000 * i;
		ptr = (unsigned char *) 0x469;
		*ptr = 0;

		/* reset cpu ready bit */
		smp_cpus_ready[i] = 0;

		/* clear apic errors */
		if (cpu_apic_version[i] & 0xf0)
		{
			apic_write (APIC_ESR, 0);
			apic_read (APIC_ESR);
		}

		/* send (aka assert) INIT IPI */
		SMP_PRINTF ("smp: asserting INIT");
		config = apic_read (APIC_ICR2) & 0xf0ffffff | (cpu_apic_id[i] << 24);
		apic_write (APIC_ICR2, config); /* set target pe */
		config = apic_read (APIC_ICR1) & 0xfff0f800 | APIC_DM_INIT | 
			APIC_LEVEL_TRIG | APIC_ASSERT;
		apic_write (APIC_ICR1, config);

		/* deassert INIT */
		SMP_PRINTF ("smp: deasserting INIT");
		config = apic_read (APIC_ICR2) & 0xffffff | (cpu_apic_id[i] << 24);
		apic_write (APIC_ICR2, config);
		config = apic_read (APIC_ICR1) & ~0xcdfff | APIC_LEVEL_TRIG |
			APIC_DM_INIT;

		/* wait 10ms */
		u_sleep (10000);

		/* is this a local apic or an 82489dx ? */
		num_startups = (cpu_apic_version[i] & 0xf0) ? 2 : 0;
		for (j = 0; j < num_startups; j++)
		{
			/* it's a local apic, so send STARTUP IPIs */
			SMP_PRINTF ("smp: sending STARTUP");
			apic_write (APIC_ESR, 0);

			/* set target pe */
			config = apic_read (APIC_ICR2) & 0xf0ffffff | (cpu_apic_id[i] <<
				24);
			apic_write (APIC_ICR2, config);

			/* send the IPI */
			config = apic_read (APIC_ICR1) & 0xfff0f800 | APIC_DM_STARTUP |
				((0x1000 * i) >> 12);
			apic_write (APIC_ICR1, config);

			/* wait */
			u_sleep (200);
			while (apic_read (APIC_ICR1) & 0x1000) ;
		}

		/* wait for processor to boot */
		SMP_PRINTF ("smp: waiting for cpu#%d to come online", i);
		apic_set_timer (5000000); /* 5 seconds */
		while (apic_read_timer ())
			if (smp_cpus_ready[i])
				break;
		if (!smp_cpus_ready[i])
			kprintf ("smp: initialisation of cpu#%d failed", i);
	}

	/* this processor is already booted */
	smp_cpus_ready[smp_my_cpu ()] = 1;

	//ioapic_init ();
	//apic_write (APIC_LVTT, 0x100fe);
	//ipi_all_but_self (0x40);
}

void __init__ smp_cpu_setup (void)
{
	unsigned int config;

	/* load the correct values into the idtr and gdtr */
	i386lidt ((uint32) idt, 0x3ff);
	i386lgdt ((uint32) gdt, 0x400 / 8);

	/* set up the local apic */
	config = apic_read ((unsigned int *) APIC_SIVR) & APIC_FOCUS | APIC_ENABLE |
		0xff; /* set spurious interrupt vector to 0xff */
	apic_write (APIC_SIVR, config);
	config = apic_read (APIC_TPRI) & 0xffffff00; /* accept all interrupts */
	apic_write (APIC_TPRI, config);

	apic_read (APIC_SIVR);
	apic_write (APIC_EOI, 0);

	config = apic_read (APIC_LVT3) & 0xffffff00; /* XXX - set vector */
	apic_write (APIC_LVT3, config);
}

void __init__ smp_final_setup (void)
{
	unsigned int config, ticks_per_us;

	ticks_per_us = bus_clock () / 1000000;
	apic_write (APIC_ICRT, ticks_per_us * 10000); /* 10 ms */
	config = 0x20030;
	apic_write (APIC_LVTT, config);

	if (smp_my_cpu ())
		sti ();
	else
		mask_irq (0);
}

void __init__ smp_cpu_ready (void)
{
	/*
	 * C code entry point for aps.  we finish initialisation and wait for the
	 * signal from the bsp before heading off into the wild blue yonder.
	 */

	/* finish processor initialisation. */
	smp_cpu_setup ();

	/* inform the world of our presence. */
	kprintf ("smp: cpu#%d is online", smp_my_cpu ());
	smp_cpus_ready[smp_my_cpu ()] = 1;

	/* wait for signal from bsp. */
	while (!smp_begun) ;
	kprintf ("smp: cpu#%d running", smp_my_cpu ());

	/* set up local apic to generate timer interrupts. */
	smp_final_setup ();

	/* bon voyage. */
	//swtch ();
	while (1) ;
}

void __init__ smp_begin (void)
{
	/*
	 * begin smp.  we set the begun bit to release the application processors
	 * out of their cages.  they storm out, fly into the scheduler, and off
	 * we go.
	 */
	smp_begun = 1;
}

int smp_my_cpu (void)
{
	if (!smp_configured)
		return 0;
	else
		return cpu_os_id [(apic_read (APIC_ID) & 0xffffffff) >> 24];
}

void ipi_dest (int num, int pe)
{
	int config;

	p (&ipi_lock);
	cli ();
	config = apic_read (APIC_ICR2) & 0xffffff | (pe << 24);
	apic_write (APIC_ICR2, config);
	config = apic_read (APIC_ICR1) & 0xfff0f800 | APIC_DEST_FIELD | num |
		0x4000;
	apic_write (APIC_ICR1, config);
	sti ();
	v (&ipi_lock);
}

void ipi_self (int num)
{
	int config;

	p (&ipi_lock);
	cli ();
	config = apic_read (APIC_ICR2) & 0xffffff;
	apic_write (APIC_ICR2, config);
	config = apic_read (APIC_ICR1) & ~0xfdfff | APIC_DEST_SELF | num;
	apic_write (APIC_ICR1, config);
	sti ();
	v (&ipi_lock);
}

void ipi_all (int num)
{
	int config;

	p (&ipi_lock);
	cli ();
	config = apic_read (APIC_ICR2) & 0xffffff;
	apic_write (APIC_ICR2, config);
	config = apic_read (APIC_ICR1) & ~0xfdfff | APIC_DEST_ALL | num;
	apic_write (APIC_ICR1, config);
	sti ();
	v (&ipi_lock);
}

void ipi_all_but_self (int num)
{
	int config;

	p (&ipi_lock);
	cli ();
	config = apic_read (APIC_ICR2) & 0xffffff;
	apic_write (APIC_ICR2, config);
	config = apic_read (APIC_ICR1) & ~0xfdfff | APIC_DEST_ALL_BUT_SELF | num;
	apic_write (APIC_ICR1, config);
	sti ();
	v (&ipi_lock);
}

volatile unsigned int ioapic_read (unsigned int offset)
{
	*ioapic = offset;
	return *(ioapic + 4); /* ioapic + 16 bytes */
}

void ioapic_write (unsigned int offset, unsigned int data)
{
	*ioapic = offset;
	*(ioapic + 4) = data; /* ioapic + 16 bytes */
}

void ioapic_init (void)
{
}

void apic_eoi (void)
{
	apic_write (APIC_EOI, 0);
}

