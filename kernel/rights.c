/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include "blt/error.h"
#include "kernel.h"
#include "resource.h"
#include "rights.h"

int right_create(uint32 rsrc_id, uint32 flags)
{
	return ERR_NONE;
}

int right_destroy(uint32 right_id)
{
	return ERR_NONE;
}

int right_revoke(uint32 right_id, uint32 thread_id)
{
	return ERR_NONE;
}

int right_grant(uint32 right_id, uint32 thread_id)
{
	return ERR_NONE;
}

