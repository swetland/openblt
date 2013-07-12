/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _RIGHTS_H_
#define _RIGHTS_H_

#include "resource.h"
 
#define RIGHT_PERM_READ      0x0001   /* allow 'read' access to something         */
#define RIGHT_PERM_WRITE     0x0002   /* allow 'write' access to something        */
#define RIGHT_PERM_DESTROY   0x0004   /* allow the something to be destroyed      */
#define RIGHT_PERM_ATTACH    0x0008   /* allows other rights to be attached       */
#define RIGHT_PERM_GRANT     0x0010   /* this right may be granted to another     */
                                      /* thread by a thread that is not the owner */
#define RIGHT_MODE_INHERIT   0x0020   /* automatically granted to child           */
#define RIGHT_MODE_DISSOLVE  0x0040   /* When the owner thread terminates,        */
                                      /* the right is destroyed                   */   
					   
int right_create(uint32 rsrc_id, uint32 flags);
int right_destroy(uint32 right_id);
int right_revoke(uint32 right_id, uint32 thread_id); 
int right_grant(uint32 right_id, uint32 thread_id);

struct __right_t
{
	resource_t rsrc;
	
	resource_t *attached;
	uint32 flags;
	uint32 refcount;
	struct __tnode_t *holders;
};



#endif