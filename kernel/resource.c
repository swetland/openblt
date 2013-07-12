/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include "kernel.h"
#include "memory.h"
#include "resource.h"
#include "team.h"

typedef struct _rtab {
    resource_t *resource;
    int next;
} rtab;

static resource_t null_resource;
	
static rtab *rmap;
static uint32 rmax = 0;
static uint32 rfree = 0;

list_t resource_list;

void rsrc_init(void *map, int size)
{
    int i;

	list_init(&resource_list);
	
    null_resource.id = 0;
    null_resource.type = RSRC_NONE;
    null_resource.owner = NULL;
	list_init(&null_resource.rights);
    
    rfree = 1;    
    rmax = size / sizeof(rtab);
    rmap = (rtab *) map;
    for(i = 0; i < rmax; i++) {
        rmap[i].resource = &null_resource;
        rmap[i].next = i+1;        
    }
    rmap[rmax-1].next = 0;            
}

void *rsrc_find(int type, int id)
{    
    if((id < rmax) && (rmap[id].resource->type == type)) {
        return rmap[id].resource;
    } else {
        return NULL;    
    }
}

void rsrc_set_owner(resource_t *r, team_t *owner) 
{
    if(r->owner) list_remove(&r->owner->resources, r);
	if(r->owner = owner) list_add_tail(&owner->resources, r);
}

int rsrc_identify(uint32 id) 
{
    if((id >= rmax) || (rmap[id].resource->type == RSRC_NONE)) return 0;
    return rmap[id].resource->owner->rsrc.id; 
}

int queue_create(const char *name, team_t *team)
{
	resource_t *rsrc = (resource_t*) kmalloc(resource_t);
	rsrc_bind(rsrc,RSRC_QUEUE,team);
	rsrc_set_name(rsrc,name);
	return rsrc->id;
}

void rsrc_bind(resource_t *rsrc, rsrc_type type, team_t *owner)
{
    uint32 id;
    
    if(rfree){
        id = rfree;
        rfree = rmap[rfree].next;
    } else {
        panic("resource exhaustion");
    }
    
    rmap[id].resource = rsrc;
    rsrc->id = id;
    rsrc->type = type;
    rsrc->owner = owner;
	rsrc->name[0] = 0;
	
	list_init(&rsrc->queue);
	list_init(&rsrc->rights);
	
    if(owner) list_add_tail(&owner->resources, rsrc);
	
	list_add_tail(&resource_list, rsrc);
}

void rsrc_release(resource_t *r)
{
    uint32 id = r->id;
    task_t *t;
	
	/* unchain it from the owner */
    if(r->owner) list_remove(&r->owner->resources, r);
	
	/* unchain it from the global pool */
	list_remove(&resource_list, r);
    
	/* wake all blocking objects */
	while((t = list_detach_head(&r->queue))) {
		task_wake(t,ERR_RESOURCE);
	}
	
    r->type = RSRC_NONE;
    r->id = 0;
	rmap[id].resource = &null_resource;
	rmap[id].next = rfree;
    rfree = id;
}

void rsrc_set_name(resource_t *r, const char *name)
{
	if(name){
		int i;
		for(i=0;*name && (i<31);i++){
			r->name[i] = *name;
			name++;
		}
		r->name[i] = 0;
	} else {
		r->name[0] = 0;
	}
}

void rsrc_enqueue_ordered(resource_t *rsrc, task_t *task, uint32 wake_time)
{
/* XXX fixme*/
	list_attach_tail(&rsrc->queue, &task->node);
	task->wait_time = wake_time;
	task->flags = tWAITING;
	task->waiting_on = rsrc;	
}

void rsrc_enqueue(resource_t *rsrc, task_t *task)
{
	task->wait_time = 0;
	task->flags = tWAITING;
	task->waiting_on = rsrc;
	list_attach_tail(&rsrc->queue,&task->node);
}

task_t *rsrc_dequeue(resource_t *rsrc)
{
	task_t *task = (task_t *) list_detach_head(&rsrc->queue);
	if(task){
		task->waiting_on = NULL;
		task->flags = tREADY;
	}
	return task;
}

task_t *rsrc_queue_peek(resource_t *rsrc)
{
	if(rsrc->queue.next != (node_t*) &rsrc->queue) {
		return (task_t*) rsrc->queue.next->data;
	}
}

const char *rsrc_typename(resource_t *rsrc)
{
	switch(rsrc->type){
	case RSRC_NONE: return "none";
	case RSRC_TASK: return "task";
	case RSRC_ASPACE: return "aspace";
	case RSRC_PORT: return "port";
	case RSRC_SEM: return "sem";
	case RSRC_RIGHT: return "right";
	case RSRC_AREA: return "area";
	case RSRC_QUEUE: return "queue";
	case RSRC_TEAM: return "team";
	default: return "????";
	}
}