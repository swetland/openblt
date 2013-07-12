/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include "list.h"
#include "memory.h"

#include <blt/error.h>

void list_init(list_t *list)
{
	list->next = (node_t *) list;
	list->prev = (node_t *) list;
	list->count = 0;
}

void list_add_head(list_t *list, void *data)
{
	node_t *node = kmalloc(node_t);
	node->data = data;
	node->next = list->next;
	node->next->prev = node;
	node->prev = (node_t*) list;
	list->next = node;
	list->count++;
}

void list_add_tail(list_t *list, void *data)
{
	node_t *node = kmalloc(node_t);
	node->data = data;
	node->prev = list->prev;
	node->prev->next = node;
	node->next = (node_t*) list;
	list->prev = node;
	list->count++;
}

void *list_peek_head(list_t *list)
{
	if(list->next == (node_t*) list) {
		return NULL;
	} else {
		return list->next->data;
	}
}

void *list_peek_tail(list_t *list)
{
	if(list->prev == (node_t*) list) {
		return NULL;
	} else {
		return list->prev->data;
	}
}

void *list_remove_head(list_t *list)
{
	node_t *node = list->next;
	void *data;
	
	if(node == (node_t*) list){
		return NULL;
	} else {
		data = node->data;
		node->prev->next = node->next;
		node->next->prev = node->prev;
		kfree(node_t, node);
		list->count--;
		return data;
	}
}

void *list_remove_tail(list_t *list)
{
	node_t *node = list->prev;
	void *data;
	
	if(node == (node_t*) list){
		return NULL;
	} else {
		data = node->data;
		node->prev->next = node->next;
		node->next->prev = node->prev;
		kfree(node_t, node);
		list->count--;
		return data;
	}
}

int list_remove(list_t *list, void *data)
{
	node_t *node = list->next;
	while(node != (node_t *) list){
		if(node->data == data){
			node->next->prev = node->prev;
			node->prev->next = node->next;
			kfree(node_t, node);
			list->count--;
			return ERR_NONE;
		}
		node = node->next;
	}
	return -1;
}


void list_attach_head(list_t *list, node_t *node)
{
	node->next = list->next;
	node->next->prev = node;
	node->prev = (node_t*) list;
	list->next = node;
	list->count++;
}

void list_attach_tail(list_t *list, node_t *node)
{
	node->prev = list->prev;
	node->prev->next = node;
	node->next = (node_t*) list;
	list->prev = node;
	list->count++;
}

void *list_detach_head(list_t *list)
{
	node_t *node = list->next;
	if(node == (node_t*) list){
		return NULL;
	} else {
		node->prev->next = node->next;
		node->next->prev = node->prev;
		list->count--;
		return node->data;
	}
}

void *list_detach_tail(list_t *list)
{
	node_t *node = list->prev;
	if(node == (node_t*) list){
		return NULL;
	} else {
		node->prev->next = node->next;
		node->next->prev = node->prev;
		list->count--;
		return node->data;
	}
}

int list_detach(list_t *list, void *data)
{
	node_t *node = list->next;
	while(node != (node_t *) list){
		if(node->data == data){
			node->next->prev = node->prev;
			node->prev->next = node->next;
			list->count--;
			return ERR_NONE;
		}
		node = node->next;
	}
	return -1;
}
