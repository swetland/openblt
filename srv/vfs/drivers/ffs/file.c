/* $Id: //depot/blt/srv/vfs/drivers/ffs/file.c#2 $
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

int ffs_open (struct vnode *vnode, void **cookie)
{
	struct ffs_super_data *data;
	struct ffs_dinode *inode;
	struct ffs_cookie *fc;

#ifdef FFS_DEBUG
	printf ("ffs_open %lld\n", vnode->v_vnid);
#endif
	data = vnode->v_sb->sb_data;
	inode = vnode->v_data;
	fc = malloc (sizeof (struct ffs_cookie));
	*cookie = fc;
	return 0;
}

int ffs_close (struct vnode *vnode, void *cookie)
{
#ifdef FFS_DEBUG
	printf ("ffs_close %lld\n", vnode->v_vnid);
#endif
	return 0;
}

void ffs_free_cookie (void *cookie)
{
#ifdef FFS_DEBUG
	printf ("ffs_free_cookie\n");
#endif
	free (cookie);
}

static inline int32 ffs_fbtofsb (struct ffs_super *fs, blkdev_t *dev,
	struct ffs_dinode *di, uint32 fb)
{
	int32 *buf, res;

	if (fb < NDADDR)
		return di->di_db[fb];
	else if ((fb -= NDADDR) < NINDIR (fs))
	{
		buf = malloc (BLKSIZE);
		blk_read (dev, buf, fsbtodb (fs, di->di_ib[0]), BLKSIZE / dev->blksize);
		res = buf[fb];
		free (buf);
		return res;
	}
	else if ((fb -= NINDIR (fs)) < NINDIR (fs) * NINDIR (fs))
	{
		printf ("ffs: doubly indirect blocks unsupported.\n");
		return 0;
	}
	else if ((fb -= NINDIR (fs) * NINDIR (fs)) < NINDIR (fs) * NINDIR (fs) *
		NINDIR (fs))
	{
		printf ("ffs: trebly indirect blocks unsupported.\n");
		return 0;
	}
	else
		return -1;
}

int ffs_read (struct vnode *vnode, char *buf, size_t count, off_t pos,
	size_t *numbytes, void *cookie)
{
	char *temp;
	int i, block;
	off_t offset, len;
	struct ffs_super *fs;
	struct ffs_super_data *data;
	struct ffs_dinode *di;

#ifdef FFS_DEBUG
	printf ("ffs_read %lld %d\n", pos, count);
#endif
	*numbytes = 0;
	if (!count)
		return 0;
	data = vnode->v_sb->sb_data;
	fs = data->sbbuf;
	di = vnode->v_data;
	if (pos >= di->di_size)
		return 0;
	if (pos + count > di->di_size)
		count = di->di_size - pos;
	temp = malloc (BLKSIZE);

	/*
	 * if we are not starting at a block boundary, first read up to the
	 * next block boundary.
	 */
	if ((offset = pos % BLKSIZE)) 
	{
		block = ffs_fbtofsb (fs, data->dev, di, pos / BLKSIZE);
		len = (count < (BLKSIZE - offset)) ? count : (BLKSIZE - offset);
		blk_read (data->dev, temp, fsbtodb (fs, block), BLKSIZE /
			data->dev->blksize);
		memcpy (buf, temp + offset, len);
		*numbytes += len, pos += len;
#ifdef FFS_DEBUG
		printf ("ffs_read: (1) %lld\n", len);
#endif
	}
	if (*numbytes == count)
		goto done;

	/*
	 * now we are block-aligned; read whole blocks at a time until we have
	 * less than a full block to go.
	 */
	for (i = 0; i < ((count - *numbytes) / BLKSIZE); i++, *numbytes += BLKSIZE,
		pos += BLKSIZE)
	{
		block = ffs_fbtofsb (fs, data->dev, di, pos / BLKSIZE);
		blk_read (data->dev, temp, fsbtodb (fs, block), BLKSIZE /
			data->dev->blksize);
		memcpy (buf + *numbytes, temp, BLKSIZE);
#ifdef FFS_DEBUG
		printf ("ffs_read: (2) #%d\n", i);
#endif
	}
	if (*numbytes == count)
		goto done;

	/*
	 * read in the next block and copy as much as we need.
	 */
	block = ffs_fbtofsb (fs, data->dev, di, pos / BLKSIZE);
	blk_read (data->dev, temp, fsbtodb (fs, block), BLKSIZE /
		data->dev->blksize);
	memcpy (buf + *numbytes, temp, len = count - *numbytes);
	*numbytes += len, pos += len;
#ifdef FFS_DEBUG
	printf ("ffs_read: (3) %lld\n", len);
#endif

done:
	free (temp);
	return 0;
}

int ffs_rstat (struct vnode *vnode, struct stat *buf)
{
	struct ffs_dinode *di;
#ifdef FFS_DEBUG
	printf ("ffs_rstat %lld\n", vnode->v_vnid);
#endif

	di = vnode->v_data;
	buf->st_ino = vnode->v_vnid;
	buf->st_nlink = di->di_nlink;
	buf->st_uid = di->di_uid;
	buf->st_gid = di->di_gid;
	buf->st_size = di->di_size;
	buf->st_blocks = di->di_blocks;
	return 0;
}

