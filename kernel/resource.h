/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "types.h"
#include "list.h"

typedef enum 
{
    RSRC_NONE, RSRC_TASK, RSRC_ASPACE, RSRC_PORT, RSRC_SEM, RSRC_RIGHT,
    RSRC_AREA, RSRC_QUEUE, RSRC_TEAM, RSRC_MAX
} rsrc_type;

struct __resource_t
{
	uint32 id;    
	rsrc_type type;
	team_t *owner;
	
	list_t rights;
	list_t queue;
	
	char name[32];
};


/* initialize the resource map */
void rsrc_init(void *map, int size);

/* locate a specifically typed resource */
void *rsrc_find(int type, int id);

/* remove the resource from the table */
void rsrc_release(resource_t *r);

/* assign a portnumber and put it in the resource table */
void rsrc_bind(resource_t *r, rsrc_type type, team_t *owner);

void rsrc_set_owner(resource_t *r, team_t *owner);
void rsrc_set_name(resource_t *r, const char *name);

/* usercall - return the rsrc_id of the owner of the resource */
int rsrc_identify(uint32 id);

void rsrc_enqueue(resource_t *rsrc, task_t *task);
void rsrc_enqueue_ordered(resource_t *rsrc, task_t *task, uint32 wake_time);
task_t *rsrc_dequeue(resource_t *rsrc);
const char *rsrc_typename(resource_t *rsrc);

task_t *rsrc_queue_peek(resource_t *rsrc);

#define rsrc_find_task(id)   ((task_t *) rsrc_find(RSRC_TASK,   id))
#define rsrc_find_port(id)   ((port_t *) rsrc_find(RSRC_PORT,   id))
#define rsrc_find_aspace(id) ((aspace_t *) rsrc_find(RSRC_ASPACE, id))
#define rsrc_find_sem(id)    ((sem_t *) rsrc_find(RSRC_SEM, id))
#define rsrc_find_area(id)   ((area_t *) rsrc_find(RSRC_AREA, id))
#define rsrc_find_right(id)  ((right_t *) rsrc_find(RSRC_RIGHT, id))
#define rsrc_find_queue(id)  ((resource_t *) rsrc_find(RSRC_QUEUE, id))
#define rsrc_find_team(id)   ((team_t *) rsrc_find(RSRC_TEAM, id))

int queue_create(const char *name, team_t *team);

#endif
