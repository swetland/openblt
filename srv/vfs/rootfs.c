/* $Id: //depot/blt/srv/vfs/rootfs.c#3 $
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "vfs-int.h"
#include "rootfs.h"

static int inode_max = 0;

static struct vnode_ops rootfs_vnode_ops =
{
	rootfs_read_vnode, rootfs_drop_vnode, NULL, NULL, rootfs_walk, NULL,
	rootfs_mount, rootfs_unmount, NULL, NULL, NULL, NULL,
	NULL, rootfs_mkdir, NULL, NULL, NULL, NULL, NULL, NULL,
	rootfs_opendir, rootfs_closedir, NULL, rootfs_rewinddir, rootfs_readdir,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL
};

struct fs_type rootfs = { "rootfs", &rootfs_vnode_ops, NULL };

static struct rootfs_inode *rootfs_inew (const char *name)
{
	struct rootfs_inode *i;

	i = malloc (sizeof (struct rootfs_inode));
	i->i_ino = inode_max++;
	i->i_name = malloc (strlen (name) + 1);
	strcpy (i->i_name, name);
	i->i_next = NULL;
	return i;
}

int rootfs_mount (struct superblock *super, const char *data, int silent)
{
	struct rootfs_inode *ri;

#ifdef ROOTFS_DEBUG
	printf ("rootfs_mount\n");
#endif
	super->sb_data = ri = rootfs_inew (".");
	ri->i_next = rootfs_inew ("..");
	super->sb_root = vget (super, 0);
	return 0;
}

void rootfs_unmount (struct superblock *super)
{
#ifdef ROOTFS_DEBUG
	printf ("rootfs_unmount\n");
#endif
}

int rootfs_read_vnode (struct vnode *vnode)
{
	struct rootfs_inode *inode;

#ifdef ROOTFS_DEBUG
	printf ("rootfs_read_vnode %llx\n", vnode->v_vnid);
#endif
	inode = vnode->v_sb->sb_data;
	while (inode != NULL)
		if (inode->i_ino == vnode->v_vnid)
		{
			vnode->v_data = inode;
			return 0;
		}
		else
			inode = inode->i_next;
	return -1;
}

void rootfs_drop_vnode (struct vnode *vnode)
{
	vnode->v_data = NULL;
}

struct vnode *rootfs_walk (struct vnode *parent, const char *path)
{
	struct rootfs_inode *inode;

#ifdef ROOTFS_DEBUG
	printf ("rootfs_walk %llx %s\n", parent->v_vnid, path);
#endif
	inode = parent->v_data;
	while (inode != NULL)
		if (strcmp (inode->i_name, path))
			inode = inode->i_next;
		else
			return vget (parent->v_sb, inode->i_ino);
	return NULL;
}

int rootfs_mkdir (struct vnode *parent, const char *name, mode_t mode)
{
	struct rootfs_inode *inode, *p;

#ifdef ROOTFS_DEBUG
	printf ("rootfs_mkdir %llx %s\n", parent->v_vnid, name);
#endif
	if (parent->v_vnid)
		return -1; /* paranoia */

	inode = malloc (sizeof (struct rootfs_inode));
	inode->i_ino = inode_max++;
	inode->i_name = malloc (strlen (name) + 1);
	strcpy (inode->i_name, name);
	inode->i_next = NULL;

	p = parent->v_sb->sb_data;
	while (p->i_next != NULL)
		p = p->i_next;
	p->i_next = inode;
	return 0;
}

int rootfs_opendir (struct vnode *dir, void **cookie)
{
	struct vfs_dirent_node *head, *p;
	struct rootfs_inode *inode;
	union rootfs_cookie *rc;

#ifdef ROOTFS_DEBUG
	printf ("rootfs_opendir\n");
#endif
	if (dir->v_vnid)
		return ENOTDIR; /* paranoia */

	head = NULL;
	inode = dir->v_sb->sb_data;
	while (inode != NULL)
	{
		p = malloc (sizeof (struct vfs_dirent_node));
		p->dirent = malloc (sizeof (struct dirent));
		p->dirent->d_fileno = inode->i_ino;
		p->dirent->d_reclen = sizeof (struct dirent);
		strncpy (p->dirent->d_name, inode->i_name, sizeof (p->dirent->d_name));

		p->next = head;
		head = p;
		inode = inode->i_next;
	}

	rc = malloc (sizeof (union rootfs_cookie));
	rc->u_dir.head = rc->u_dir.current = head;
	*cookie = rc;
	return 0;
}

void rootfs_closedir (struct vnode *dir, void *cookie)
{
#ifdef ROOTFS_DEBUG
	printf ("rootfs_closedir\n");
#endif
}

int rootfs_rewinddir (struct vnode *dir, void *cookie)
{
	union rootfs_cookie *rc;

#ifdef ROOTFS_DEBUG
	printf ("rootfs_rewinddir\n");
#endif

	rc = cookie;
	rc->u_dir.current = rc->u_dir.head;
	return 0;
}

int rootfs_readdir (struct vnode *dir, struct dirent *dirent, void *cookie)
{
	struct dirent *orig;
	union rootfs_cookie *rc;

	rc = cookie;
	if (rc->u_dir.current == NULL)
		return 1;
	orig = rc->u_dir.current->dirent;
	dirent->d_fileno = orig->d_fileno;
	dirent->d_reclen = orig->d_reclen;
	strncpy (dirent->d_name, orig->d_name, BLT_MAX_NAME_LENGTH);
	rc->u_dir.current = rc->u_dir.current->next;
#ifdef ROOTFS_DEBUG
	printf ("rootfs_readdir %s\n", dirent->d_name);
#endif
	return 0;
}

