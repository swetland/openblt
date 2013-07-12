/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include "kernel.h"
#include "memory.h"
#include "list.h"

team_t *team_create(void)
{
	team_t *team = kmalloc(team_t);
	
	if(!(team->aspace = aspace_create())) return NULL;

	list_init(&team->resources);
	team->refcount = 0;
	team->text_area = 0;
	team->heap_id = 0;
	
	rsrc_bind(&team->rsrc, RSRC_TEAM, kernel_team);
	rsrc_set_owner(&team->aspace->rsrc, team);

	return team;
}

void team_destroy(team_t *team)
{
	resource_t *rsrc;
	
//	kprintf("death to team #%d (%s)",team->rsrc.id,team->rsrc.name);
//	DEBUGGER();
	
	while(rsrc = list_remove_head(&team->resources)){
		rsrc->owner = NULL;
//		kprintf("- %x %s %d",rsrc,rsrc_typename(rsrc),rsrc->id);
		
		switch(rsrc->type){
		case RSRC_TASK:
			kprintf("oops... cannot destroy team %d because of task %d!?",
					team->rsrc.id,rsrc->id);
			DEBUGGER();
		case RSRC_PORT:
			port_destroy(rsrc->id);
			break;
		case RSRC_SEM:
			sem_destroy(rsrc->id);
			break;
		case RSRC_ASPACE:
			aspace_destroy((aspace_t*) rsrc);
			break;
		case RSRC_AREA:
		case RSRC_QUEUE:
		case RSRC_TEAM:
			/* skip for now - teams don't get destroyed, areas and queues get
			destroyed with their relatives */
			break;
			
		default:
			kprintf("what the hell is %d (type %d)?",rsrc->id,rsrc->type);
		}
	}
	
	rsrc_release(team);
	kfree(team_t, team);
}

