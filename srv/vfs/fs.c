/* $Id: //depot/blt/srv/vfs/fs.c#1 $
**
** Copyright 1998 Sidney Cammeresi
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions, and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions, and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**    derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
** OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
** IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
** INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
** NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
** THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <string.h>
#include "vfs-int.h"

int fs_register (struct fs_type *driver)
{
	struct fs_type *p;

	if (fs_drivers == NULL)
	{
		fs_drivers = driver;
		fs_drivers->next = NULL;
		return 0;
	}
	else
	{
		p = fs_drivers;
		while (p->next != NULL)
		{
			if (!strcmp (p->name, driver->name))
				return 1;
			p = p->next;
		}
		p->next = driver;
		driver->next = NULL;
		return 0;
	}
}

struct superblock *fs_find (const char *node)
{
	int len, bestlen;
	struct superblock *super, *best;

	super = mount_list;
	len = bestlen = 0;
	best = NULL;
	while (super != NULL)
	{
		if (!strncmp (super->sb_dir, node, len = strlen (super->sb_dir)))
			if (len > bestlen)
			{
				best = super;
				bestlen = len;
			}
		super = super->sb_next;
	}
	return best;
}

