/* $Id: //depot/blt/srv/vfs/drivers/ffs/ffs-blt.h#2 $
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

#ifndef FFS_BLT_H
#define FFS_BLT_H

#include <dirent.h>

struct ffs_super_data
{
	blkdev_t *dev;
	struct ffs_super *sbbuf;
};

struct ffs_cookie
{
	char *buf1, *buf2;
	int block1, block2;
};

struct ffs_dircookie
{
	struct vfs_dirent_node *head, *current;
};

int ffs_mount (struct superblock *super, const char *data, int silent);
void ffs_unmount (struct superblock *super);
int ffs_read_vnode (struct vnode *vnode);
void ffs_drop_vnode (struct vnode *vnode);
struct vnode *ffs_walk (struct vnode *parent, const char *path);
int ffs_opendir (struct vnode *dir, void **cookie);
void ffs_closedir (struct vnode *dir, void *cookie);
void ffs_free_dircookie (void *cookie);
int ffs_rewinddir (struct vnode *dir, void *cookie);
int ffs_readdir (struct vnode *dir, struct dirent *dirent, void *cookie);
int ffs_open (struct vnode *dir, void **cookie);
int ffs_close (struct vnode *dir, void *cookie);
void ffs_free_cookie (void *cookie);
int ffs_read (struct vnode *vnode, char *buf, size_t count, off_t offset,
	size_t *numbytes, void *cookie);
int ffs_rstat (struct vnode *vnode, struct stat *buf);

#endif

