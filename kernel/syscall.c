/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <i386/io.h>
#include "kernel.h"
#include "memory.h"
#include "resource.h"
#include "boot.h"
#include "aspace.h"
#include "task.h"
#include "smp.h"
#include "rights.h"

#include <blt/syscall_id.h>

extern task_t *irq_task_map[16];

void k_debugger(regs *r, uint32 eip, uint32 cs, uint32 eflags);

extern int live_tasks;
extern int reaper_sem;

extern boot_dir *bdir;

/* HACK: sem_release will cause a reschedule which would not be good */
void wake_the_reaper(void)
{
	task_t *task;
	sem_t *sem = rsrc_find_sem(reaper_sem);
	sem->count++;
	if((task = rsrc_dequeue(&sem->rsrc)) != NULL) task_wake(task,ERR_NONE);
}

void terminate(void)
{
    task_t *t = current, *t0;
	
    //kprintf("Task %X terminated.",current->rsrc.id);
    live_tasks--;    
	rsrc_enqueue(reaper_queue,current);
    current->flags = tDEAD;
	
	/* wake all blocking objects */
	while((t0 = list_detach_head(&t->rsrc.queue)) != NULL)
		task_wake(t0,ERR_RESOURCE);
	
	wake_the_reaper();
	
    swtch();
    kprintf("panic: HUH? Back from the dead? %x / %x",t,current);
    asm("hlt");
}

void sleep(int ticks)
{
	/* convert from microseconds to 3ms ticks - round up slightly */
	ticks = ((ticks + 2000) / 3000); 
	
	if(ticks) {
		rsrc_enqueue_ordered(timer_queue, current, kernel_timer + ticks);
	}
	swtch();	
}

#define p_uint32(n) (esp[n])
#define p_int(n) ((int) esp[n])
#define p_voidptr(n) ((void *) esp[n])
#define p_charptr(n) ((char *) esp[n])
#define p_sizet(n) ((size_t) esp[n])
#define p_pint(n) ((int*) esp[n])
#define p_puint32(n) ((uint32*) esp[n])

#define res r.eax
int metacall (volatile uint32 *esp);

void syscall(regs r, volatile uint32 eip, uint32 cs, uint32 eflags,
	volatile uint32 *esp)
{
#if 0
	kprintf("* %d %x #%d@%x",current->rsrc.id,eip,r.eax,(uint32)esp);
#endif
		
    switch(r.eax){
    case BLT_SYS_os_terminate :
        terminate();    
        break;

    case BLT_SYS_os_console :
        kprintf("task %X> %s",current->rsrc.id,p_charptr(1));
        break;

    case BLT_SYS_os_brk :
        res = brk(p_uint32(1));
        break;

    case BLT_SYS_os_handle_irq :
            /* thread wants to listen to irq in eax */
        if(p_uint32(1) > 0 && p_uint32(1) < 16) {
            current->irq = p_uint32(1);
            irq_task_map[p_uint32(1)] = current;
        };
        
        eflags |= 2<<12 | 2<<13;
        break;

    case BLT_SYS_os_sleep_irq :
        if(current->irq){
                /* sleep */
            current->flags = tSLEEP_IRQ;
            unmask_irq(current->irq);
        }
        swtch();
        break;

    case BLT_SYS_os_debug :
#ifdef __SMP__
        if (smp_configured)
          {
            config = apic_read (APIC_LVTT);
            apic_write (APIC_LVTT, config | 0x10000);
            smp_begun = 0;
            ipi_all_but_self (IPI_STOP);
          }
#endif
        k_debugger(&r, eip, cs, eflags);
		kprintf("bye bye debugger");
		
#ifdef __SMP__
				if (smp_configured)
          {
            smp_begin ();
            apic_write (APIC_LVTT, config);
          }
#endif
        break;

    case BLT_SYS_os_sleep :
        sleep(p_uint32(1));
        break;
	
	case BLT_SYS_os_identify :
		res = rsrc_identify(p_uint32(1));
		break;

	case BLT_SYS_os_meta :
		res = metacall(esp);
		break;
		
    case BLT_SYS_sem_create :
        res = sem_create(p_uint32(1),p_charptr(2));
        break;

    case BLT_SYS_sem_destroy :
        res = sem_destroy(p_uint32(1));
        break;

    case BLT_SYS_sem_acquire :
        res = sem_acquire(p_uint32(1));
        break;

    case BLT_SYS_sem_release :
        res = sem_release(p_uint32(1));
        break;

    case BLT_SYS_port_create :
        res = port_create(p_uint32(1),p_charptr(2));
        break;

    case BLT_SYS_port_destroy :
        res = port_destroy(p_uint32(1));    
        break;

    case BLT_SYS_port_option :
        res = port_option(p_uint32(1), p_uint32(2), p_uint32(3));
        break;

    case BLT_SYS_port_send :
        res = port_send(p_int(1), p_int(2), p_voidptr(3), p_sizet(4), p_uint32(5));
        break;

    case BLT_SYS_port_recv :
        res = port_recv(p_int(1), p_pint(2), p_voidptr(3), p_sizet(4), p_puint32(5));
        break;        
		
	case BLT_SYS_right_create :
		res = right_create(p_uint32(1), p_uint32(1));
		break;
		
	case BLT_SYS_right_destroy :
		res = right_destroy(p_uint32(1));
		break;
		
	case BLT_SYS_right_revoke :
		res = right_revoke(p_uint32(1), p_uint32(2));
		break;
		
	case BLT_SYS_right_grant :
		res = right_grant(p_uint32(1), p_uint32(2));
		break;		
        
    case BLT_SYS_area_create :
        res = area_create(current->rsrc.owner->aspace, p_uint32(1), p_uint32(2), (void **)p_voidptr(3), p_uint32(4));
        break;

    case BLT_SYS_area_clone :
        res = area_clone(current->rsrc.owner->aspace, p_uint32(1), p_uint32(2), (void **)p_voidptr(3), p_uint32(4));
        break;
        
    case BLT_SYS_area_destroy :
        res = area_destroy(current->rsrc.owner->aspace, p_uint32(1));
        break;
        
    case BLT_SYS_area_resize :
        res = area_resize(current->rsrc.owner->aspace, p_uint32(1), p_uint32(2));
        break;

	case BLT_SYS_thr_create : {
		char *name;
        int i;    
        task_t *t;
        t = new_thread(current->rsrc.owner, p_uint32(1), 0);
		if(t) {
			*((unsigned int *) (t->ustack + 0xfec)) = p_uint32(2);
			name = p_charptr(3);
			if (name == NULL)
			{
				for(i=0;current->rsrc.name[i] && i<31;i++) {
					t->rsrc.name[i] = current->rsrc.name[i];
				}
				if(i<30) t->rsrc.name[i++] = '+';
				t->rsrc.name[i] = 0;    
			} else {
				rsrc_set_name((resource_t *)t, name);
			}
			res = t->rsrc.id;
		} else {
			res = -1;
		}
    }
    break;

	case BLT_SYS_thr_resume :
		break;

	case BLT_SYS_thr_suspend :
		break;

	case BLT_SYS_thr_spawn :
		res = thr_spawn(p_uint32(1), p_uint32(2),
						p_uint32(3), p_uint32(4),
						p_uint32(5), p_uint32(6),
						p_charptr(7));
		break;
		
	case BLT_SYS_thr_kill :
		break;

	case BLT_SYS_thr_wait :
		res = thr_wait(p_uint32(1));
		break;

	default:
		res = -1;
    }
}

int metacall (volatile uint32 *esp)
{
	switch (p_uint32 (0))
	{
		case META_NULL_REQUEST :
			return 0;

		default:
			return -1;
	}
}

