/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <i386/io.h>
#include "kernel.h"
#include "memory.h"
#include "smp.h"
#include "init.h"
#include "port.h"
#include "task.h"
#include "aspace.h"
#include "resource.h"
#include "pager.h"
#include "i386.h"

#define noHALT_ON_FAULT
#define DEBUG_ON_FAULT
#define noCRASH_ON_FAULT

static char *etable[] = {
    "Divide-by-zero",
    "Debug Exception",
    "NMI",
    "Breakpoint",
    "INTO",
    "BOUNDS",
    "Invalid Opcode",
    "Device Not Available",
    "Double-fault",
    "Coprocessor segment overrun",
    "Invalid TSS fault",
    "Segment Not Present",
    "Stack Exception",
    "General Protection",
    "Page Fault",
    "*reserved*",
    "Floating-point error",
    "Alignment Check",
    "Machine Check"
};

typedef void (*ehfunc)();

static void __fault(uint32 number, regs r, uint32 eip, uint32 cs,
		uint32 eflags);
static void __faultE(uint32 number, regs r, uint32 error, uint32 eip, uint32 cs,
		uint32 eflags);

ehfunc ehfunctab[] = 
	{ __fault, __fault, __fault, __fault,
	  __fault, __fault, __fault, __fault,
	  __faultE, __fault, __faultE, __faultE,
	  __faultE, __faultE, __faultE /*page_fault*/, __fault,
	  __fault, __fault, __fault };

extern unsigned char *screen;

task_t *irq_task_map[16];

void k_debugger(regs *r, uint32 eip, uint32 cs, uint32 eflags);
void terminate(void);

void print_regs(regs *r, uint32 eip, uint32 cs, uint32 eflags)
{
    kprintf("   EAX = %x   EBX = %x   ECX = %x   EDX = %x",
            r->eax, r->ebx, r->ecx, r->edx);
    kprintf("   EBP = %x   ESP = %x   ESI = %x   EDI = %x",
            r->ebp, r->esp, r->esi, r->edi);
    kprintf("EFLAGS = %x    CS = %x   EIP = %x",
            eflags, cs, eip);   
}

void user_debug(regs *r, uint32 *eip, uint32 *eflags)
{
#if 1
	return;
#else
	int src,p,sz;
	task_t *t0;
	uchar buf[16];
	uint32 code;
	
	p = port_create(0, "debug control");
		
	/* wake all blocking objects */
	while((t0 = list_detach_head(&current->rsrc.queue))) task_wake(t0,ERR_RESOURCE);
		
	kprintf("Waiting on debug control port (%d)... ",p);
	current->team = kernel_team; // XXX hack 
	 
	while((sz = port_recv(p, &src, buf, 16, &code)) >= 0){
	}
	kprintf("debug control error %d\n",sz);
	
#endif
}


static void __faultE(uint32 number,
            regs r, uint32 error,
            uint32 eip, uint32 cs, uint32 eflags)
{
    uint32 _cr2, _cr3;

    kprintf("");
    kprintf("*** Exception 0x%X* (%s)",number,etable[number]);
#ifdef __SMP__
    if (smp_configured)
      kprintf ("      on cpu#%d", smp_my_cpu ());
#endif
    print_regs(&r, eip, cs, eflags);
    
    asm("mov %%cr2, %0":"=r" (_cr2));    
    asm("mov %%cr3, %0":"=r" (_cr3));
    kprintf("   cr2 = %x   cr3 = %x error = %x",_cr2,_cr3,error);
    kprintf("");
    kprintf("Task %d (%s) crashed.",current->rsrc.id,current->rsrc.name);

	if((cs & 0xFFF8) == SEL_UCODE) user_debug(&r, &eip, &eflags);
	
#ifdef DEBUG_ON_FAULT
    current->flags = tDEAD;
    k_debugger(&r, eip, cs, eflags);
#endif
    
#ifdef HALT_ON_FAULT
    asm("hlt");
#endif    
    
    terminate();    
}

static void __fault(uint32 number,
           regs r,
           uint32 eip, uint32 cs, uint32 eflags)
{

    kprintf("");
    kprintf("*** Exception 0x%X (%s)",number,etable[number]);
    print_regs(&r, eip, cs, eflags);

    kprintf("");
    kprintf("Task %d (%s) crashed.",current->rsrc.id,current->rsrc.name);
	
	if((cs & 0xFFF8) == SEL_UCODE) user_debug(&r, &eip, &eflags);
	
#ifdef DEBUG_ON_FAULT
    if(number != 2){
        current->flags = tDEAD;
    }
    k_debugger(&r, eip, cs, eflags);
#endif
#ifdef HALT_ON_FAULT
    asm("hlt");
#endif    
    if(number != 2){
        terminate();    
    }
}

void irq_dispatch(regs r, uint32 number)
{
    mask_irq(number);    
    if(irq_task_map[number]){
        if(irq_task_map[number]->flags == tSLEEP_IRQ){
            preempt(irq_task_map[number],ERR_NONE);            
        }
    }    
}

int kernel_timer = 0;

unsigned char x[] = { '-', '-', '\\', '\\', '|', '|', '/', '/' };

void pulse (void)
{
#ifndef __SMP__
	screen[0] = x[kernel_timer%8];
	screen[1] = 7;
#else
	screen[smp_my_cpu () * 2] = x[kernel_timer%8];
	screen[smp_my_cpu () * 2 + 1] = 7;
#endif
}

void timer_irq(regs r, uint32 eip, uint32 cs, uint32 eflags)
{
	task_t *task;
    kernel_timer++;
    
	while((task = rsrc_queue_peek(timer_queue))){
		if(task->wait_time <= kernel_timer){
			task = rsrc_dequeue(timer_queue);
			rsrc_enqueue(run_queue, task);
		} else {
			break;
		}
	}
#ifdef PULSE
    pulse ();
#endif    
    swtch();    
}

extern void __null_irq(void);
extern void __timer_irq(void);
extern void __kbd_irq(void);
extern void __syscall(void);

#define EX(n) extern void __ex##n(void);
EX(0); EX(1); EX(2); EX(3); EX(4); EX(5); EX(6); EX(7); EX(8); EX(9);
EX(10); EX(11); EX(12); EX(13); EX(14); EX(15); EX(16); EX(17); EX(18);

#define IQ(n) extern void __irq##n(void);
IQ(1); IQ(2); IQ(3); IQ(4); IQ(5); IQ(6); IQ(7); IQ(8); IQ(9);
IQ(10); IQ(11); IQ(12); IQ(13); IQ(14); IQ(15); 

extern void __ipi_cf (void), __ipi_tlb (void), __ipi_pte (void),
	__ipi_resched (void), __ipi_stop (void);

static void set_irq(uint32 *IDT, int n, void *func)
{
    IDT[2*n+1] = (((uint32) func) & 0xFFFF0000) | 0x00008E00;
    IDT[2*n]   = (((uint32) func) & 0x0000FFFF) | (SEL_KCODE << 16);
}

static void set_irqU(uint32 *IDT, int n, void *func)
{
    IDT[2*n+1] = (((uint32) func) & 0xFFFF0000) | 0x00008E00 | 0x00006000;
    IDT[2*n]   = (((uint32) func) & 0x0000FFFF) | (SEL_KCODE << 16);
}

void __init__ init_idt(uint32 *IDT)
{
    int i;
    for(i=0;i<16;i++) irq_task_map[i] = NULL;
    
    set_irq(IDT,0x00,__ex0);
    set_irq(IDT,0x01,__ex1);
    set_irq(IDT,0x02,__ex2);
    set_irq(IDT,0x03,__ex3);
    set_irq(IDT,0x04,__ex4);
    set_irq(IDT,0x05,__ex5);
    set_irq(IDT,0x06,__ex6);
    set_irq(IDT,0x07,__ex7);
    set_irq(IDT,0x08,__ex8);
    set_irq(IDT,0x09,__ex9);
    set_irq(IDT,0x0A,__ex10);
    set_irq(IDT,0x0B,__ex11);
    set_irq(IDT,0x0C,__ex12);
    set_irq(IDT,0x0D,__ex13);
    set_irq(IDT,0x0E,__ex14);
    set_irq(IDT,0x0F,__ex15);
    set_irq(IDT,0x10,__ex16);
    set_irq(IDT,0x11,__ex17);
    set_irq(IDT,0x12,__ex18);
    
    set_irqU(IDT,0x20,__syscall);
    
    set_irq(IDT,0x30,__timer_irq);
    set_irq(IDT,0x31,__irq1);
    set_irq(IDT,0x32,__irq2);
    set_irq(IDT,0x33,__irq3);
    set_irq(IDT,0x34,__irq4);
    set_irq(IDT,0x35,__irq5);
    set_irq(IDT,0x36,__irq6);
    set_irq(IDT,0x37,__irq7);
    set_irq(IDT,0x38,__irq8);
    set_irq(IDT,0x39,__irq9);
    set_irq(IDT,0x3A,__irq10);
    set_irq(IDT,0x3B,__irq11);
    set_irq(IDT,0x3C,__irq12);
    set_irq(IDT,0x3D,__irq13);
    set_irq(IDT,0x3E,__irq14);
    set_irq(IDT,0x3F,__irq15);

#ifdef __SMP__
		set_irq(IDT,0x40,__ipi_cf);
		set_irq(IDT,0x41,__ipi_tlb);
		set_irq(IDT,0x42,__ipi_pte);
		set_irq(IDT,0x43,__ipi_resched);
		set_irq(IDT,0x44,__ipi_stop);
#endif
 
    i386lidt((uint32) IDT, 0x3FF);
    
    remap_irqs();
    unmask_irq(0);
    unmask_irq(2);
}

void restore_idt(void)
{
    unmap_irqs();    
    i386lidt(0,0x3FF);
}


#ifdef __SMP__
void ipi_dispatch (regs r, uint32 number)
{
	unsigned int config;

	kprintf ("cpu#%d got ipi %x", smp_my_cpu (), number);
	apic_write (APIC_EOI, 0);
	switch (number)
		{
			case IPI_CACHE_FLUSH:
				asm ("wbinvd");
				break;
			case IPI_INV_TLB:
				local_flush_tlb ();
				break;
			case IPI_INV_PTE:
				local_flush_tlb (); /* FIXME */
				break;
			case IPI_INV_RESCHED:
				break;
			case IPI_STOP:
				config = apic_read (APIC_LVTT);
				apic_write (APIC_LVTT, config | 0x10000);
				while (!smp_begun) ;
				apic_write (APIC_LVTT, config);
				break;
		}
}
#endif

