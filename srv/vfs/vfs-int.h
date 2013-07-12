/* $Id: //depot/blt/srv/vfs/vfs-int.h#11 $
**
** Copyright 1999 Sidney Cammeresi.
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

#ifndef _VFS_INT_H_
#define _VFS_INT_H_


#include <dirent.h>
#include <sys/stat.h>

#ifndef VFS_SANDBOX
#include <blt/types.h>
#include <blt/hash.h>
#include <blt/qsem.h>
#include <blt/vfs.h>
#include <blt/syscall.h>
#else
#include "hash.h"
#define MAX_FDS 256
#endif

struct fs_type
{
	char *name;
    struct vnode_ops *t_vops;
	struct fs_type *next;
};

struct superblock
{
	char *sb_dev, *sb_dir;
	struct vnode *sb_root;
	struct vnode_ops *sb_vops;
	void *sb_data;
	hashtable_t *sb_vnode_cache;
	struct superblock *sb_next;
};

/*
 * This stuff was mercilessly stolen from *Practical File System Design
 * With the Be File System* by Dominic Giampaolo.  According to rumours,
 * it is a lot like the way BeOS handles stuff internally.
 */

/*
 * don't be alarmed by all of the voids; i've just not yet come to the
 * semantics of all of these yet.  patience, please.
 */
struct vnode_ops
{
	int (*read_vnode) (struct vnode *vnode);
	void (*drop_vnode) (struct vnode *vnode);
	void (*remove_vnode) (void);
	void (*secure_vnode) (void);
	struct vnode *(*walk) (struct vnode *parent, const char *path);
	void (*access) (void);

	int (*mount) (struct superblock *sb, const char *data, int silent);
	void (*unmount) (struct superblock *sb);
	void (*initialise) (void);
	void (*sync) (void);
	void (*rfstat) (void);
	void (*wfstat) (void);

	void (*create) (void);
	int (*mkdir) (struct vnode *parent, const char *name, mode_t mode);
	void (*symlink) (void);
	void (*link) (void);
	void (*rename) (void);
	void (*unlink) (void);
	void (*rmdir) (void);
	void (*readlink) (void);

	int (*opendir) (struct vnode *dir, void **cookie);
	void (*closedir) (struct vnode *dir, void *cookie);
	void (*free_dircookie) (void *cookie);
	int (*rewinddir) (struct vnode *dir, void *cookie);
	int (*readdir) (struct vnode *vnode, struct dirent *dirent, void *cookie);

	int (*open) (struct vnode *vnode, void **cookie);
	int (*close) (struct vnode *vnode, void *cookie);
	void (*free_cookie) (void *cookie);
	int (*read) (struct vnode *vnode, char *buf, size_t count, off_t offset,
		size_t *res, void *cookie);
	void (*write) (struct vnode *vnode, const char *buf, size_t count,
			void *cookie);
	void (*ioctl) (void);
	void (*setflags) (void);
	int (*rstat) (struct vnode *vnode, struct stat *buf);
	int (*wstat) (struct vnode *vnode, struct stat *buf);
	void (*fsync) (void);

	void (*open_attrdir) (void);
	void (*close_attrdir) (void);
	void (*free_attrdircookie) (void);
	void (*rewind_attrdir) (void);
	void (*read_attrdir) (void);
	void (*read_attr) (void);
	void (*write_attr) (void);
	void (*remove_attr) (void);
	void (*rename_attr) (void);
	void (*stat_attr) (void);

	void (*open_indexdir) (void);
	void (*close_indexdir) (void);
	void (*free_indexdircookie) (void);
	void (*rewind_indexdir) (void);
	void (*read_indexdir) (void);
	void (*create_index) (void);
	void (*remove_index) (void);
	void (*rename_index) (void);
	void (*stat_index) (void);

	void (*open_query) (void);
	void (*close_query) (void);
	void (*free_query_cookie) (void);
	void (*read_query) (void);
};

struct vnode
{
	unsigned long long v_vnid;
	unsigned int v_refcount;
	struct superblock *v_sb;
	void *v_data;
#ifndef VFS_SANDBOX
	qsem_t *v_lock;
#endif
};

struct ofile
{
	struct vnode *o_vnode;
	void *o_cookie;
	int area, offset, length, flags;
	int o_pos;
	void *dataptr;
};

struct fdarray
{
	struct ofile *ofile[MAX_FDS];
};

struct ioctx
{
	char *cwd;
	struct fdarray fdarray;
};

struct client
{
	int in, out, filename_area;
	struct ioctx ioctx;
	char *nameptr;
};

/* functions for use by modules */
int fs_register (struct fs_type *type);
void fs_unregister (struct fs_type *type);

/* utility functions */
struct superblock *fs_find (const char *name);
struct vnode *vget (struct superblock *super, int num);
void vput (struct vnode *vnode);

#ifndef VFS_SANDBOX
vfs_res_t *vfs_openconn (int rport, int area);
vfs_res_t *vfs_scroll_area (struct client *client, vfs_cmd_t *vc);
vfs_res_t *vfs_opendir (struct client *client, vfs_cmd_t *vc);
vfs_res_t *vfs_closedir (struct client *client, vfs_cmd_t *vc);
#endif

struct vfs_dirent_node
{
	struct dirent *dirent;
	struct vfs_dirent_node *next;
};

extern struct fs_type *fs_drivers;
extern struct superblock *mount_list;

int vfs_mount (const char *dir, const char *type, int flags, const void *data);

#endif

