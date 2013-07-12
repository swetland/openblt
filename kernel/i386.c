/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Copyright 1998-1999, Sidney Cammeresi. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include "types.h"
#include "i386.h"
#include <i386/io.h>

void i386SetSegment(void *entry,
                    uint32 base, uint32 limit,
                    uint8 rights, uint8 gran)
{
    *((uint32 *) entry) = (limit & 0xFFFF) | ((base & 0xFFFF) << 16);   

    *((uint32 *) ( ((char *) entry) + 4 ) ) =
        ((base & 0x00FF0000) >> 16) | (base & 0xFF000000) |
        (rights << 8) | ((gran & 0xF0) << 16) |
        ((limit & 0x000F000) << 4);
}

void i386SetTaskGate(void *entry, uint16 selector, uint8 rights)
{
    *((uint32 *) entry) = selector << 16;
    *((uint32 *) ( ((char *) entry) + 4 ) ) = (0x05 | (rights & 0xF0)) << 8;  
}

/* thanks to paul swanson for these... */

void i386ltr(uint32 selector) 
{
    __asm__ __volatile__ ("ltr %%ax": :"eax" (selector));
}

void i386lidt(uint32 base, uint32 limit) 
{
    uint32 i[2];

    i[0] = limit << 16;
    i[1] = (uint32) base;
    __asm__ __volatile__ ("lidt (%0)": :"p" (((char *) i)+2));
}

void i386lgdt(uint32 base, uint32 limit) 
{
    uint32 i[2];

    i[0] = limit << 16;
    i[1] = base;
    __asm__ __volatile__ ("lgdt (%0)": :"p" (((char *) i)+2));
}

uint32 *i386sgdt(uint32 *limit) 
{
    uint32 gdtptr[2];
    __asm__ __volatile__ ("sgdt (%0)": :"p" (((char *) gdtptr)+2));
    *limit = gdtptr[0] >> 16;
    return (uint32 *) gdtptr[1];
}

#if 0
//#ifdef __SMP__

void remap_irqs (void)
{
	unsigned int i, config;

	for (i = 0; i < 16; i++)
		{
			config = 
			ioapic_write (IOAPIC_REDIR_TABLE + 2 * i, config);
			config = 0xff << 24; /* logical destination address */
			ioapic_write (IOAPIC_REDIR_TABLE + 2 * i + 1, config);
		}
}

void unmap_irqs (void)
{
}

void unmask_irq (int irq)
{
}

void mask_irq (int irq)
{
}

#else /* __SMP__ */

#define PORTA0 0x20
#define PORTB0 0x21
#define PORTA1 0xA0
#define PORTB1 0xA1

void remap_irqs(void)
{
    outb_p(0x11, PORTA0);
    outb_p(0x30, PORTB0);
    outb_p(0x04, PORTB0);
    outb_p(0x01, PORTB0);
    outb_p(0xff, PORTB0);                      
    
    outb_p(0x11, PORTA1);
    outb_p(0x38, PORTB1);
    outb_p(0x02, PORTB1);
    outb_p(0x01, PORTB1);
    outb_p(0xff, PORTB1);                       
}

void unmap_irqs(void)
{
    outb_p(0x11, PORTA0);
    outb_p(0x08, PORTB0);
    outb_p(0x04, PORTB0);
    outb_p(0x01, PORTB0);
    outb_p(0x00, PORTB0);                       
    
    outb_p(0x11, PORTA1);
    outb_p(0x70, PORTB1);
    outb_p(0x02, PORTB1);
    outb_p(0x01, PORTB1);    
    outb_p(0x00, PORTB1);                       
}

void unmask_irq(int irq)
{
    if(irq < 8)
        outb((inb(PORTB0) & ~(1 << irq)), PORTB0);
    else
        outb((inb(PORTB1) & ~(1 << (irq-8))), PORTB1);
}

void mask_irq(int irq)
{
    if(irq < 8)
        outb((inb(PORTB0) | (1 << irq)), PORTB0);
    else
        outb((inb(PORTB1) | (1 << (irq-8))), PORTB1);
}

#endif /* __SMP__ */

static int PIT_COUNTER[2][3]={ { 0x40, 0x41, 0x42 },
                              { 0x48, 0x49, 0x4a } };
static int PIT_CONTROL[2] = { 0x43, 0x4b };

#define PIT_DOSRESET    1       // resert PIT to DOS condition
#define PIT_IKU         2       // start preemptivlly multitasking

void init_timer(void)
{
    outb(0x24,PIT_CONTROL[0]);
        /* write to counter 0, binary, high byte, mode 2 */
#if 0
    outb(0x2f,PIT_COUNTER[0][0]);
        /*0x2f00=12032 * .8380965us ~= 10.083ms*/
#else
    outb(0x0e,PIT_COUNTER[0][0]);
        /*0x0e00=3584 * .8380965us ~= 3.004ms*/
#endif
}

void cli (void)
{
	asm ("cli");
}

void sti (void)
{
	asm ("sti");
}

void local_flush_tlb (void)
{
	int junk;

	asm ("mov %%cr3, %0 ; mov %0, %%cr3" : : "r" (junk));
}

void local_flush_pte (unsigned int addr)
{
#ifdef I386
	local_flush_tlb ();
#else
	asm ("invlpg (%0)" : : "r" (addr));
#endif
}

void p (unsigned char *lock)
{
	int res;

	res = 1;
	asm ("lock ; xchgb %1, %0" : "=m" (*lock), "=r" (res) : "1" (res) : "memory");
	while (res)
		asm ("lock ; xchgb %1, %0" : "=m" (*lock), "=r" (res) : "1" (res) :
			"memory");
}

void v (unsigned char *lock)
{
	asm ("lock ; btrl $0, %0" : "=m" (*lock));
}

