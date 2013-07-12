/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _TEAM_H
#define _TEAM_H

#include "resource.h"

struct __team_t {
	resource_t rsrc;
	
	list_t resources;	   /* all resources owned by the team */
	int refcount;          /* number of attached tasks */

    aspace_t *aspace;     /* address space which it all exists in */
	
	int heap_id;
    int text_area;	
};

team_t *team_create(void);
void team_destroy(team_t *team);

#endif
