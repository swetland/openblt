/* $Id: //depot/blt/srv/vfs/drivers/ffs/super.c#4 $
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

static struct vnode_ops ffs_vnode_ops =
{
	ffs_read_vnode, ffs_drop_vnode, NULL, NULL, ffs_walk, NULL,
	ffs_mount, ffs_unmount, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	ffs_opendir, ffs_closedir, ffs_free_dircookie, ffs_rewinddir, ffs_readdir,
	ffs_open, ffs_close, ffs_free_cookie, ffs_read, NULL, NULL, NULL,
		ffs_rstat, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL
};

static struct fs_type ffs = { "ffs", &ffs_vnode_ops, NULL };

int _init (void)
{
#ifdef FFS_DEBUG
	printf ("ffs: registering driver\n");
#endif
	fs_register (&ffs);
	return 0;
}

int ffs_mount (struct superblock *super, const char *data, int silent)
{
	struct ffs_super_data *sbdata;
	struct ffs_super *ds;

#ifdef FFS_DEBUG
	printf ("ffs_mount, data `%s'\n", data);
#endif
	sbdata = malloc (sizeof (struct ffs_super_data));
	blk_open (data, 0, &sbdata->dev);
	super->sb_data = sbdata;
	sbdata->sbbuf = ds = malloc (SBSIZE);
	blk_read (sbdata->dev, ds, SBOFF / sbdata->dev->blksize,
		SBSIZE / sbdata->dev->blksize);
	super->sb_root = vget (super, ROOTINO);
	super->sb_dev = malloc (strlen (data) + 1);
	strcpy (super->sb_dev, data);
	return 0;
}

void ffs_unmount (struct superblock *super)
{
	struct ffs_super_data *sbdata;

#ifdef FFS_DEBUG
	printf ("ffs_unmount\n");
#endif
	vput (super->sb_root);
	sbdata = super->sb_data;
	blk_close (sbdata->dev);
	free (sbdata->sbbuf);
	free (sbdata);
}

