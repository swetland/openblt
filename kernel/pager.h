/* $Id: //depot/blt/kernel/pager.h#2 $
**
** Copyright 1998-2000, Sidney Cammeresi.  All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _PAGER_H_
#define _PAGER_H_

#include "i386.h"

struct __pager_fault_t
{
	unsigned int op, vaddr, eip;
	task_t *task;
};

extern int pager_port_no, pager_sem_no;
extern task_t *pager_task;
extern sem_t *pager_sem;

void pager (void);
void page_fault (uint32 number, regs r, uint32 error, uint32 eip, uint32 cs,
		uint32 eflags);

#endif

