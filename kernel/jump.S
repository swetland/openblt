/* $Id: //depot/blt/kernel/jump.S#8 $
**
** Copyright 1998 Brian Swetland.  All rights reserved.
** Distributed under the terms of the OpenBLT License.
*/

#include "smp.h"

.text
.globl __syscall
.globl __null_irq
.globl __timer_irq
.globl __kbd_irq
.globl __ipi_cf
.globl __ipi_tlb
.globl __ipi_pte
.globl __ipi_resched
.globl __ipi_stop
.globl ehfunctab

__null_irq:
	movb $32, %al
	outb %al, $32
	iret

__syscall:
	pusha
	call syscall
	popa
	iret

__timer_irq:
	pusha
#ifdef __SMP__
	movb smp_configured, %al
	cmpb $1, %al
	je 1f
#endif
	movb $32, %al
	outb %al, $32
#ifdef __SMP__
	jmp 2f
1:
	call apic_eoi
#endif
2:
	call timer_irq
	popa
	iret

#define FAULT(n) \
.globl __ex##n ;\
__ex##n: ;\
	pusha ;\
	push $n ;\
	mov $(n * 4 + ehfunctab), %ebx ;\
	mov (%ebx), %ebx ;\
	call *%ebx ;\
	add $4,%esp ;\
	popa ;\
	iret

#define FAULTE(n) \
.globl __ex##n ;\
__ex##n: ;\
	pusha ;\
	push $n ;\
	mov $(n * 4 + ehfunctab), %ebx ;\
	mov (%ebx), %ebx ;\
	call *%ebx ;\
	add $4,%esp ;\
	popa ;\
	add $4,%esp ;\
	iret

FAULT(0)
FAULT(1)
FAULT(2)
FAULT(3)
FAULT(4)
FAULT(5)
FAULT(6)
FAULT(7)
FAULTE(8)
FAULT(9)
FAULTE(10)
FAULTE(11)
FAULTE(12)
FAULTE(13)
FAULTE(14)
FAULT(15)
FAULT(16)
FAULT(17)
FAULT(18)

#define IRQ(n) \
.globl __irq##n ;\
__irq##n: ;\
	push $n ;\
	pusha ;\
	call irq_dispatch ;\
	movb $0x20, %al ;\
	outb %al, $0x20 ;\
	popa ;\
	add $4,%esp ;\
	iret 

#define IRQHI(n) \
.globl __irq##n ;\
__irq##n: ;\
	push $n ;\
	pusha ;\
	call irq_dispatch ;\
	movb $0x20, %al ;\
	outb %al, $0xA0 ;\
	movb $0x20, %al ;\
	outb %al, $0x20 ;\
	popa ;\
	add $4,%esp ;\
	iret			

IRQ(1)
IRQ(2)
IRQ(3)
IRQ(4)
IRQ(5)
IRQ(6)
IRQ(7)
IRQHI(8)
IRQHI(9)
IRQHI(10)
IRQHI(11)
IRQHI(12)
IRQHI(13)
IRQHI(14)
IRQHI(15)

# From: Jeff Bush <jeff@be.com>

;#       1. Stores necessary registers on old stack
;#       2. Changes old stack pointer to reflect new offset
;#       3. Change address space if necessary
;#       4. Pops state off old stack
;#       5. returns to address on old stack, resuming thread


.globl          _context_switch
.globl			thread_bootstrap
.globl			kthread_bootstrap

_context_switch:
				pushf
                pushl   %ebp
                pushl   %esi
                pushl   %edi
				pushl	%ebx
                movl    24(%esp), %eax          # Where to save stack pointer
                movl    %esp, (%eax)            # Save old stack
                movl    32(%esp), %eax          # Get new PDBR
                movl    28(%esp), %ebx          # Get new stack loc

                cmpl    $0, %eax                # Need to change addr. space?
                je      changed_pdbr            # If parameter was zero, no
                movl    %eax, %cr3              # Change address space

changed_pdbr:   movl    %ebx, %esp              # switch to new stack
				popl	%ebx
                popl    %edi
                popl    %esi
                popl    %ebp
                popf
                ret
				
thread_bootstrap:
				iret

kthread_bootstrap:
				lret


#ifdef __SMP__

__ipi_cf:
	pushl $0x40
	jmp __ipi
__ipi_tlb:
	pushl $0x41
	jmp __ipi
__ipi_pte:
	pushl $0x42
	jmp __ipi
__ipi_resched:
	pushl $0x43
	jmp __ipi
__ipi_stop:
	pushl $0x44
	jmp __ipi

__ipi:
	pusha
	call ipi_dispatch
	call apic_eoi
	popa
	add $4, %esp
	iret

#endif

