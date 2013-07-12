/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include "kernel.h"
#include "resource.h"
#include "memory.h"

int sem_create(int count, const char *name) 
{
    sem_t *s = (sem_t *) kmalloc(sem_t);
    s->count = count;
	rsrc_bind(&s->rsrc, RSRC_SEM, current->rsrc.owner);
	rsrc_set_name(&s->rsrc, name);
    return s->rsrc.id;
}

int sem_destroy(int id)
{
    sem_t *s;
    if(!(s = rsrc_find_sem(id))) {
        return ERR_RESOURCE;
    }
	rsrc_release(&s->rsrc);
	kfree(sem_t,s);
	return ERR_NONE;
}

int sem_acquire(int id) 
{
	int status;
    sem_t *s;
    
    if(!(s = rsrc_find_sem(id))) {
        return ERR_RESOURCE;
    }
    
    if(s->count > 0 ){
        s->count--;
    } else {
        s->count--;
		if(status = wait_on(&s->rsrc)) return status;
    }
    return ERR_NONE;
}

int sem_release(int id) 
{
    int x;
    sem_t *s;
    task_t *t;
    
    if(!(s = rsrc_find_sem(id))) {
        return ERR_RESOURCE;
    }

    s->count++;
	
    if(t = rsrc_dequeue(&s->rsrc)){
		preempt(t,ERR_NONE);
    }

    return ERR_NONE;
}
