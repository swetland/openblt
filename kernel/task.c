/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include "kernel.h"
#include "memory.h"
#include "task.h"

extern char *gdt;
extern int live_tasks;

void thread_bootstrap(void);
void kthread_bootstrap(void);

void task_wake(task_t *task, int status)
{
	task->status = status;
	rsrc_enqueue(run_queue, task);
}

int wait_on(resource_t *rsrc)
{
	task_t *task = current;
	task->status = 0;
	rsrc_enqueue(rsrc, task);	
	swtch();
	return current->status;
}

void task_destroy(task_t *task)
{
	team_t *team = task->rsrc.owner;
	
	if(task == current) panic("cannot destroy the running task");
	
	TCLRMAGIC(task);
	
	if(task->stack_area) {
		area_destroy(team->aspace, task->stack_area->rsrc.id);
	}
	
	kfreepage(task->kstack);
	rsrc_release(&task->rsrc);
	kfree(task_t, task);
	
	team->refcount--;
	if(team->refcount == 0) team_destroy(team);
}

/* create a new task, complete with int stack */
task_t *task_create(team_t *team, uint32 ip, uint32 sp, int kernel) 
{
    task_t *t = kmalloc(task_t);	
	uint32 *SP;
	
	t->kstack = kgetpages(1);
	t->cr3 = team->aspace->pdirphys;
	
	t->esp = (uint32) ( ((char *) t->kstack) + 4092 );
	t->esp0 = t->esp;
	t->scount = 0;
	t->stack_area = NULL;
	t->team = team;
	
	t->node.data = t;
	
	/* prep the kernel stack for first switch 
	** SS
	** ESP
	** EFLAGS
	** CS
	** EIP            -- thread_bootstrap will iret into the thread 
	**
	** <thread_bootstrap>
	** EFLAGS 
	** EBP (0)
	** ESI (0)
	** EDI (0)        -- stuff for _context_switch to pop off / return to
	** EBX (0)
	*/
	
	SP = (uint32*) t->esp;
	
	if(kernel) {
		SP--; *SP = SEL_KDATA; 
		SP--; *SP = sp - 4*5;
		SP--; *SP = 0x3202;
		SP--; *SP = SEL_KCODE;
		SP--; *SP = ip;
		SP--; *SP = (uint32) thread_bootstrap;
	} else {
		SP--; *SP = SEL_UDATA | 3; 
		SP--; *SP = sp - 4*5;
		SP--; *SP = 0x3202;
		SP--; *SP = SEL_UCODE | 3;
		SP--; *SP = ip;
		SP--; *SP = (uint32) thread_bootstrap;
	}	
	SP--; *SP = 0x3002;
	SP--; *SP = 0;
	SP--; *SP = 0;
	SP--; *SP = 0;
	SP--; *SP = 0;
	
	t->esp = (uint32) SP;
	
//	kprintf("thr:%x/%x:%d",sp,ip,(kernel ? SEL_KCODE : (SEL_UCODE | 3)));
	
    t->irq = 0;
    t->flags = tREADY;
	t->waiting_on = NULL;
	team->refcount++;
	
	TSETMAGIC(t);
    return t;
}


int thr_wait(int thr_id)
{
	task_t *task = rsrc_find_task(thr_id);
	
	if(task) {
		wait_on((resource_t *)task);
		return ERR_NONE;
	} else {
		return ERR_RESOURCE;
	}
}

int thr_spawn(uint32 ip, uint32 sp, 
			  uint32 area0, uint32 vaddr0, 
			  uint32 area1, uint32 vaddr1,
			  const char *name)
{
	aspace_t *aspace;
	task_t *task;
	team_t *team;
	area_t *area;
	int id;
	void *addr;
	
	team = team_create();
	aspace = team->aspace;
	task = task_create(team, ip, sp, 0);
	task->ustack = 0;
	rsrc_bind(&task->rsrc, RSRC_TASK, team);

	id = area_clone(aspace, area0, vaddr0, &addr, 0);
	team->heap_id = id;
	
	if(area = rsrc_find_area(id)) {
		rsrc_set_owner(&area->rsrc, team);
	}
	
	id = area_clone(aspace, area1, vaddr1, &addr, 0);
	if(area = rsrc_find_area(id)) rsrc_set_owner(&area->rsrc, team);

	rsrc_set_name(&task->rsrc, name);
	rsrc_set_name(&team->rsrc, name);
	rsrc_enqueue(run_queue, task);
	live_tasks++;
	
	return task->rsrc.id;
}


