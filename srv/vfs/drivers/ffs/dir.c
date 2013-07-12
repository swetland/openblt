/* $Id: //depot/blt/srv/vfs/drivers/ffs/dir.c#3 $
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
#include <dirent.h>
#include "vfs-int.h"

#ifndef VFS_SANDBOX
#include <blt/blkdev.h>
#else
#include "../../sandbox/blkdev.h"
#endif

#include "ffs.h"
#include "ffs-blt.h"
#include "dinode.h"
#include "dir.h"

int ffs_opendir (struct vnode *dir, void **cookie)
{
	char *buf;
	int i, j, offset;
	struct vfs_dirent_node *head, *p;
	struct ffs_super *fs;
	struct ffs_super_data *data;
	struct ffs_dinode *di;
	struct ffs_dircookie *fc;
	struct ffs_direct *direct;

#ifdef FFS_DEBUG
	printf ("ffs_opendir %lld\n", dir->v_vnid);
#endif

	head = NULL;
	data = dir->v_sb->sb_data;
	fs = data->sbbuf;
	di = dir->v_data;
	buf = malloc (BLKSIZE);
	for (i = 0; i < NDADDR; i++)
		if (di->di_db[i])
		{
			blk_read (data->dev, buf, fsbtodb (fs, di->di_db[i]),
				BLKSIZE / data->dev->blksize);
			for (j = offset = 0; offset < BLKSIZE; offset += direct->d_reclen)
			{
				direct = (struct ffs_direct *) (buf + offset);
				if (!direct->d_ino)
					break;
				//printf ("opendir %s %d %d\n", direct->d_name, offset, j++);
				//if (j > 10) for (;;) ;
				p = malloc (sizeof (struct vfs_dirent_node));
				p->dirent = malloc (sizeof (struct dirent));
				p->dirent->d_fileno = direct->d_ino;
				p->dirent->d_reclen = sizeof (struct dirent);
				strncpy (p->dirent->d_name, direct->d_name,
					sizeof (p->dirent->d_name));
				p->next = head;
				head = p;
			}
		}
	for (i = 0; i < NIADDR; i++)
		if (di->di_ib[i])
		{
		}
	free (buf);
	fc = malloc (sizeof (struct ffs_dircookie));
	fc->head = fc->current = head;
	*cookie = fc;
	return 0;
}

void ffs_closedir (struct vnode *dir, void *cookie)
{
#ifdef FFS_DEBUG
	printf ("ffs_closedir %lld\n", dir->v_vnid);
#endif
}

void ffs_free_dircookie (void *cookie)
{
	struct ffs_dircookie *fc;

#ifdef FFS_DEBUG
	printf ("ffs_freedircookie\n");
#endif
	fc = cookie;
	while (fc->head != NULL)
	{
		fc->current = fc->head->next;
		free (fc->head);
		fc->head = fc->current;
	}
	free (fc);
}

int ffs_rewinddir (struct vnode *dir, void *cookie)
{
	struct ffs_dircookie *fc;

#ifdef FFS_DEBUG
	printf ("ffs_rewinddir %lld\n", dir->v_vnid);
#endif
	fc = cookie;
	fc->current = fc->head;
	return 0;
}

int ffs_readdir (struct vnode *dir, struct dirent *dirent, void *cookie)
{
	struct dirent *orig;
	struct ffs_dircookie *fc;

#ifdef FFS_DEBUG
	//printf ("ffs_readdir %lld\n", dir->v_vnid);
#endif
	fc = cookie;
	if (fc->current == NULL)
		return 1;
	orig = fc->current->dirent;
	dirent->d_fileno = orig->d_fileno;
	dirent->d_reclen = orig->d_reclen;
	strncpy (dirent->d_name, orig->d_name, MAXNAMLEN);
	fc->current = fc->current->next;
	return 0;
}

