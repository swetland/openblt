/* $Id: //depot/blt/srv/vfs/bootfs.h#6 $
**
** Copyright 1999 Sidney Cammeresi.
** All rights reserved.
** 
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright notice,
**	this list of conditions and the following disclaimer.
** 
** 2. Redistributions in binary form must reproduce the above copyright
**	notice, this list of conditions and the following disclaimer in the
**	documentation and/or other materials provided with the distribution.
** 
** 3. The name of the author may not be used to endorse or promote products
**	derived from this software without specific prior written permission.
** 
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR `AS IS' AND ANY EXPRESS OR IMPLIED
** WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
** NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _BOOTFS_H_
#define _BOOTFS_H_

#include <boot.h>
#include "vfs-int.h"

struct bootfs_inode
{
	int i_ino, i_offset, i_size;
	char i_name[BOOTDIR_NAMELEN];
	struct bootfs_inode *i_next;
};

struct bootfs_sb_data
{
	int d_bootdir_area;
	boot_dir *d_bootdir;
	struct bootfs_inode *inode_list;
};

union bootfs_cookie
{
	struct
	{
	    struct vfs_dirent_node *head, *current;
	} u_dir;
	struct
	{
		char *begin;
	    int pos;
	} u_file;
};

int bootfs_mount (struct superblock *sb, const char *data, int silent);
void bootfs_unmount (struct superblock *sb);
int bootfs_read_vnode (struct vnode *vnode);
void bootfs_drop_vnode (struct vnode *vnode);
struct vnode *bootfs_walk (struct vnode *parent, const char *path);
int bootfs_opendir (struct vnode *dir, void **cookie);
void bootfs_closedir (struct vnode *dir, void *cookie);
int bootfs_rewinddir (struct vnode *dir, void *cookie);
int bootfs_readdir (struct vnode *dir, struct dirent *dirent, void *cookie);
int bootfs_open (struct vnode *dir, void **cookie);
int bootfs_close (struct vnode *dir, void *cookie);
void bootfs_free_cookie (void *cookie);
int bootfs_read (struct vnode *dir, char *buf, size_t count, off_t offset,
	size_t *res, void *cookie);
int bootfs_rstat (struct vnode *vnode, struct stat *buf);

#endif

