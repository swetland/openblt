/* $Id: //depot/blt/srv/vfs/bootfs.c#9 $
**
** Copyright 1999 Sidney Cammeresi.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
** 1. Redistributions of source code must retain the above copyright
**	notice, this list of conditions, and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**	notice, this list of conditions, and the following disclaimer in the
**	documentation and/or other materials provided with the distribution.
** 3. The name of the author may not be used to endorse or promote products
**	derived from this software without specific prior written permission.
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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <blt/syscall.h>
#include "vfs-int.h"
#include "bootfs.h"

static int inode_max = 0;

static struct vnode_ops bootfs_vnode_ops =
{
	bootfs_read_vnode, bootfs_drop_vnode, NULL, NULL, bootfs_walk, NULL,
	bootfs_mount, bootfs_unmount, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	bootfs_opendir, bootfs_closedir, NULL, bootfs_rewinddir, bootfs_readdir,
	bootfs_open, bootfs_close, bootfs_free_cookie, bootfs_read, NULL,
		NULL, NULL, bootfs_rstat, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL
};

struct fs_type bootfs = { "bootfs", &bootfs_vnode_ops, NULL };

static struct bootfs_inode *bootfs_inew (const char *name, int offset,
	int size)
{
	struct bootfs_inode *inode;

	inode = malloc (sizeof (struct bootfs_inode));
	inode->i_ino = inode_max++;
	inode->i_offset = offset;
	inode->i_size = size;
	strlcpy (inode->i_name, name, sizeof (inode->i_name));
	inode->i_next = NULL;
	return inode;
}

int bootfs_mount (struct superblock *super, const char *data, int silent)
{
	int i;
	struct bootfs_sb_data *sb_data;
	struct bootfs_inode *inode;
	boot_entry *be;

#ifdef BOOTFS_DEBUG
	printf ("bootfs_mount\n");
#endif
	sb_data = (struct bootfs_sb_data *) malloc (sizeof (struct bootfs_sb_data));
	sb_data->d_bootdir_area = area_clone (3, 0, (void **) &sb_data->d_bootdir,
		0);
	sb_data->inode_list = NULL;

	/* create inode list */
	sb_data->inode_list = bootfs_inew (".", 0, 0);
	sb_data->inode_list->i_next = bootfs_inew ("..", 0, 0);
	for (i = 0; i < BOOTDIR_MAX_ENTRIES; i++)
		if ((sb_data->d_bootdir->bd_entry[i].be_type != BE_TYPE_NONE) &&
			strcmp (sb_data->d_bootdir->bd_entry[i].be_name,
			BOOTDIR_DIRECTORY))
		{
			be = &sb_data->d_bootdir->bd_entry[i];
			inode = bootfs_inew (be->be_name, be->be_offset, be->be_vsize);
			inode->i_next = sb_data->inode_list;
			sb_data->inode_list = inode;
		}

	super->sb_data = sb_data;
	super->sb_root = vget (super, 0);
	return 0;
}

void bootfs_unmount (struct superblock *sb)
{
#ifdef BOOTFS_DEBUG
	printf ("bootfs_unmount\n");
#endif
}

int bootfs_read_vnode (struct vnode *vnode)
{
	struct bootfs_sb_data *sb_data;
	struct bootfs_inode *inode;

#ifdef BOOTFS_DEBUG
	printf ("bootfs_read_vnode %llx\n", vnode->v_vnid);
#endif
	sb_data = vnode->v_sb->sb_data;
	inode = sb_data->inode_list;
	while (inode != NULL)
		if (inode->i_ino == vnode->v_vnid)
		{
			vnode->v_data = inode;
			return 0;
		}
		else
			inode = inode->i_next;
		
	return 0;
}

void bootfs_drop_vnode (struct vnode *vnode)
{
#ifdef BOOTFS_DEBUG
	vnode->v_data = NULL;
#endif
}

struct vnode *bootfs_walk (struct vnode *parent, const char *path)
{
	struct bootfs_sb_data *data;
	struct bootfs_inode *inode;

#ifdef BOOTFS_DEBUG
	printf ("bootfs_walk %s\n", path);
#endif
	if (parent->v_vnid)
		return NULL;
	else
	{
		data = parent->v_sb->sb_data;
		inode = data->inode_list;
		while (inode != NULL)
			//if (!strncmp (inode->i_name, path, BOOTDIR_NAMELEN))
			if (!strcmp (inode->i_name, path))
				return vget (parent->v_sb, inode->i_ino);
			else
				inode = inode->i_next;
		return NULL;
	}
}

int bootfs_opendir (struct vnode *dir, void **cookie)
{
	struct bootfs_sb_data *data;
	struct vfs_dirent_node *head, *p;
	struct bootfs_inode *inode;
	union bootfs_cookie *bc;

#ifdef BOOTFS_DEBUG
	printf ("bootfs_opendir\n");
#endif
	if (dir->v_vnid)
		return ENOTDIR; /* paranoia */

	head = NULL;
	data = dir->v_sb->sb_data;
	inode = data->inode_list;
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

	bc = malloc (sizeof (union bootfs_cookie));
	bc->u_dir.head = bc->u_dir.current = head;
	*cookie = bc;
	return 0;
}

void bootfs_closedir (struct vnode *dir, void *cookie)
{
#ifdef BOOTFS_DEBUG
	printf ("bootfs_closedir\n");
#endif
}

int bootfs_rewinddir (struct vnode *dir, void *cookie)
{
	union bootfs_cookie *bc;

#ifdef BOOTFS_DEBUG
	printf ("bootfs_rewinddir\n");
#endif

	bc = cookie;
	bc->u_dir.current = bc->u_dir.head;
	return 0;
}

int bootfs_readdir (struct vnode *dir, struct dirent *dirent, void *cookie)
{
	struct dirent *orig;
	union bootfs_cookie *bc;

	bc = cookie;
	if (bc->u_dir.current == NULL)
	    return 1;
	orig = bc->u_dir.current->dirent;
	dirent->d_fileno = orig->d_fileno;
	dirent->d_reclen = orig->d_reclen;
	strncpy (dirent->d_name, orig->d_name, BLT_MAX_NAME_LENGTH);
	bc->u_dir.current = bc->u_dir.current->next;
#ifdef BOOTFS_DEBUG
	printf ("bootfs_readdir %s\n", dirent->d_name);
#endif
	return 0;
}

int bootfs_open (struct vnode *vnode, void **cookie)
{
	struct bootfs_sb_data *data;
	struct bootfs_inode *inode;
	union bootfs_cookie *bc;

#ifdef BOOTFS_DEBUG
	printf ("bootfs_open %llx\n", vnode->v_vnid);
#endif
	data = vnode->v_sb->sb_data;
	inode = vnode->v_data;
	bc = malloc (sizeof (union bootfs_cookie));
	bc->u_file.begin = (char *) data->d_bootdir + inode->i_offset * 0x1000;
	bc->u_file.pos = 0;
	*cookie = bc;
	return 0;
}

int bootfs_close (struct vnode *vnode, void *cookie)
{
#ifdef BOOTFS_DEBUG
	printf ("bootfs_close %llx\n", vnode->v_vnid);
#endif
	return 0;
}

void bootfs_free_cookie (void *cookie)
{
#ifdef BOOTFS_DEBUG
	printf ("bootfs_free_cookie\n");
#endif
	free (cookie);
}

int bootfs_read (struct vnode *vnode, char *buf, size_t count, off_t offset,
	size_t *numread, void *cookie)
{
	char *src;
	struct bootfs_sb_data *data;
	struct bootfs_inode *inode;
	union bootfs_cookie *bc;

#ifdef BOOTFS_DEBUG
	printf ("bootfs_read %llx\n", vnode->v_vnid);
#endif
	data = vnode->v_sb->sb_data;
	inode = vnode->v_data;
	bc = cookie;
	if (offset >= inode->i_size)
	{
		*numread = 0;
		return 0;
	}
	src = (char *) data->d_bootdir + inode->i_offset * 0x1000 + offset;
	*numread = (count <= inode->i_size - offset) ? count : (inode->i_size -
		offset);
	memcpy (buf, src, *numread);
	return 0;
}

int bootfs_rstat (struct vnode *vnode, struct stat *buf)
{
	struct bootfs_inode *inode;

#ifdef BOOTFS_DEBUG
	printf ("bootfs_rstat %llx\n", vnode->v_vnid);
#endif
	inode = vnode->v_data;

	buf->st_ino = inode->i_ino;
	buf->st_nlink = 0;
	buf->st_uid = 0;
	buf->st_gid = 0;
	buf->st_blksize = 512;
	buf->st_size = inode->i_size;
	buf->st_blocks = (inode->i_size & 0xfff) ? (inode->i_size >> 12) + 1 :
		inode->i_size >> 12;
	return 0;
}

