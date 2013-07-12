/* $Id: //depot/blt/srv/vfs/rootfs.h#2 $
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

#ifndef _ROOTFS_H_
#define _ROOTFS_H_

#include "vfs-int.h"

struct rootfs_inode
{
	int i_ino;
	char *i_name;
	struct rootfs_inode *i_next;
};

union rootfs_cookie
{
	struct
	{
		struct vfs_dirent_node *head, *current;
	} u_dir;
	struct
	{
		int pos;
	} u_file;
};

static struct rootfs_inode *rootfs_inew (const char *name);

int rootfs_mount (struct superblock *sb, const char *data, int silent);
void rootfs_unmount (struct superblock *super);
int rootfs_read_vnode (struct vnode *vnode);
void rootfs_drop_vnode (struct vnode *vnode);
struct vnode *rootfs_walk (struct vnode *vnode, const char *path);
int rootfs_mkdir (struct vnode *vnode, const char *name, mode_t mode);
int rootfs_opendir (struct vnode *dir, void **cookie);
void rootfs_closedir (struct vnode *dir, void *cookie);
int rootfs_readdir (struct vnode *dir, struct dirent *dirent, void *cookie);
int rootfs_rewinddir (struct vnode *dir, void *cookie);

#endif

