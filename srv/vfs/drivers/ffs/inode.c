/* $Id: //depot/blt/srv/vfs/drivers/ffs/inode.c#2 $
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

#ifndef VFS_SANDBOX
#include <blt/blkdev.h>
#else
#include "../../sandbox/blkdev.h"
#endif

#include "ffs.h"
#include "ffs-blt.h"
#include "dinode.h"
#include "dir.h"

int ffs_read_vnode (struct vnode *vnode)
{
	char *buf;
	int block, offset;
	struct ffs_super *fs;
	struct ffs_super_data *data;
	struct ffs_dinode *di;

#ifdef FFS_DEBUG
	printf ("ffs_read_vnode %lld\n", vnode->v_vnid);
#endif
	data = vnode->v_sb->sb_data;
	fs = data->sbbuf;
	vnode->v_data = di = malloc (sizeof (struct ffs_dinode));
	buf = malloc (BLKSIZE);
	block = fsbtodb (fs, ino_to_fsba (fs, (int) vnode->v_vnid));
	offset = ino_to_fsbo (fs, (int) vnode->v_vnid);
	blk_read (data->dev, buf, block, BLKSIZE / data->dev->blksize);
	memcpy (di, (struct ffs_dinode *) buf + offset, sizeof (struct ffs_dinode));
	free (buf);
	return 0;
}

void ffs_drop_vnode (struct vnode *vnode)
{
#ifdef FFS_DEBUG
	printf ("ffs_drop_vnode %lld\n", vnode->v_vnid);
#endif
	free (vnode->v_data);
}

static struct vnode *ffs_walk_one (struct vnode *parent, const char *path)
{
	char *buf;
	int i, offset;
	struct ffs_super *fs;
	struct ffs_super_data *data;
	struct ffs_dinode *di;
	struct ffs_direct *direct;

#ifdef FFS_DEBUG
	printf ("ffs_walk_one %s\n", path);
#endif
	data = parent->v_sb->sb_data;
	fs = data->sbbuf;
	di = parent->v_data;
	buf = malloc (BLKSIZE);

	for (i = 0; i < NDADDR; i++)
		if (di->di_db[i])
		{
			blk_read (data->dev, buf, fsbtodb (fs, di->di_db[i]),
				BLKSIZE / data->dev->blksize);
			for (offset = 0; offset < BLKSIZE; offset += direct->d_reclen)
			{
				direct = (struct ffs_direct *) (buf + offset);
				if (!strcmp (direct->d_name, path))
					return vget (parent->v_sb, direct->d_ino);
			}
		}
	printf ("ffs_walk_one: failage 1!\n");

	for (i = 0; i < NIADDR; i++)
		if (di->di_ib[i])
		{
			printf ("ffs_walk_one: indirect %d\n", di->di_ib[i]);
		}

	free (buf);
	printf ("ffs_walk_one: failage 2!\n");
	return NULL;
}

struct vnode *ffs_walk (struct vnode *parent, const char *path)
{
	char *name;
	int i, j, len;
	struct vnode *vn, *vnnext;

#ifdef FFS_DEBUG
	printf ("ffs_walk %s\n", path);
#endif
	vn = parent;
	name = malloc ((len = strlen (path)) + 1);
	strcpy (name, path);

	for (i = 0; i < strlen (path); i = j + 1, vn = vnnext)
	{
		for (j = i; (name[j] != '/') && (j < len); j++) ;
		name[j] = 0;
		vnnext = ffs_walk_one (vn, name + i);
		if (vn != parent)
			vput (vn);
	}
	return vn;
}

