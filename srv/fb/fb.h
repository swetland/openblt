/* $Id$
**
** Copyright 1999 Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _FB_H
#define _FB_H

#include <blt/types.h>

void dprintf(const char *fmt, ...);

typedef struct fb_info 
{
	const char *name;
	int32 (*find)(void **cookie);
	int32 (*init)(void *cookie);
} fb_info;

#endif
