/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _SEM_H
#define _SEM_H

#include "resource.h"

struct __sem_t 
{
	resource_t rsrc;

    int count;
};

int sem_create(int count, const char *name);
int sem_destroy(int sem);
int sem_acquire(int id);
int sem_release(int id);

#endif
