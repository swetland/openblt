/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _LIST_H
#define _LIST_H

#include "types.h"

struct __list_t
{
	node_t *next; /* aka head */
	node_t *prev; /* aka tail */
	uint32 count;
};

/* generic or template dlinklist node */
struct __node_t
{
	node_t *next;
	node_t *prev;
	void *data;
};

void list_init(list_t *list);

/* these functions allocate and destroy node_t's to store the items */
void list_add_head(list_t *list, void *data);
void list_add_tail(list_t *list, void *data);
void *list_peek_head(list_t *list);
void *list_peek_tail(list_t *list);
void *list_remove_head(list_t *list);
void *list_remove_tail(list_t *list);
int list_remove(list_t *list, void *data);

/* these functions are for items that "own" the node_t */
void list_attach_head(list_t *list, node_t *node);
void list_attach_tail(list_t *list, node_t *node);
void *list_detach_head(list_t *list);
void *list_detach_tail(list_t *list);
int list_detach(list_t *list, void *data);

#endif