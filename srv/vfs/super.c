/* $Id: //depot/blt/srv/vfs/super.c#5 $
**
** Copyright 1999 Sidney Cammeresi
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfs-int.h"

int vfs_mount (const char *dir, const char *type, int flags, const void *data)
{
	int res;
	static int rootmounted = 0;
	struct fs_type *driver;
	struct superblock *super;

#ifdef VFS_DEBUG
	printf ("vfs_mount %s %s\n", dir, type);
#endif

	/* find filesystem driver */
	driver = fs_drivers;
	while (driver != NULL)
	{
		if (!strcmp (driver->name, type))
			break;
		else
			driver = driver->next;
	}
	if (driver == NULL)
	{
		printf ("vfs: no driver\n");
		return 1;
	}
	super = malloc (sizeof (struct superblock));
	super->sb_vops = driver->t_vops;

	super->sb_dir = malloc (strlen (dir) + 1);
	strcpy (super->sb_dir, dir);

	/* verify mount point existence */
	if (!rootmounted)
	{
		rootmounted = 1;
		super->sb_dev = "none";
	}
	else
	{
		super->sb_dev = "none";
	}

	super->sb_vnode_cache = hashtable_new (0.75);
	super->sb_next = NULL;
	res = driver->t_vops->mount (super, data, 1);
	if (!res)
	{
		if (mount_list != NULL)
			super->sb_next = mount_list;
		mount_list = super;
		if (strcmp (type, "rootfs") && strcmp (type, "bootfs"))
			printf ("vfs: mounted type %s on %s from %s\n", type,
				super->sb_dir, super->sb_dev);
	}
	return res;
}

