/* $Id: //depot/blt/kernel/pager.c#3 $
**
** Copyright 2000, Sidney Cammeresi.  All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include "kernel.h"
#include "memory.h"
#include "types.h"
#include "sem.h"
#include "list.h"
#include "pager.h"

int pager_port_no, pager_sem_no;
task_t *pager_task;
sem_t *pager_sem;
list_t pager_queue;

void terminate (void);
void k_debugger (regs *r, uint32 eip, uint32 cs, uint32 eflags);

void pager (void)
{
	pager_fault_t *fault;
	list_t *list;
	node_t *node;
	area_t *area;

	pager_sem = rsrc_find_sem (pager_sem_no);
	list_init (&pager_queue);

	for (;;)
	{
		sem_acquire (pager_sem_no);
		fault = list_remove_head (&pager_queue);

		kprintf ("pager: got paging request %d for vaddr %x", fault->op,
			fault->vaddr);
		list = &fault->task->team->aspace->areas;
		node = list->next;
		while (node->next != (node_t *) list)
		{
			area = node->data;
			kprintf ("pager: checking %x+%x", area->virt_addr, area->length);
			if (((fault->vaddr >> 12) >= area->virt_addr) &&
					(fault->vaddr >> 12) < (area->virt_addr + area->length))
			{
				/* shouldn't happen yet */
				kprintf ("pager: found it");
				continue;
			}
			node = node->next;
		}
		kprintf ("pager: task %d (%s) SEGV: eip = %x, addr = %x",
			fault->task->rsrc.id, fault->task->rsrc.name, fault->eip,
			fault->vaddr);
		task_wake (fault->task, ERR_SEGV);
	}
}

void page_fault (uint32 number, regs r, uint32 error, uint32 eip, uint32 cs,
		uint32 eflags)
{
	pager_fault_t *fault;

	/* queue the paging request */
	fault = kmalloc (pager_fault_t);
	fault->op = 666;
	asm ("mov %%cr2, %0" : "=r" (fault->vaddr));
	fault->eip = eip;
	fault->task = current;
	list_add_tail (&pager_queue, fault);

	/* wake up the kernel pager */
	current->flags = tSLEEP_PAGING;
	pager_sem->count++;
	task_wake (pager_task, ERR_NONE); /* XXX: sem_release requeues us */
	swtch ();

	/* paging is done; check result */
	if (current->status == ERR_SEGV) {
		if((cs & 0xFFF8) == SEL_UCODE) {
			user_debug(&r, &eip, &eflags);
		} else {
			kprintf("pf: cs = %x\n",cs& 0xfff8);
		}
		
		current->flags = tDEAD;
		k_debugger (&r, eip, cs, eflags);
		terminate ();
	}
}

