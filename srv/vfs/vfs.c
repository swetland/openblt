/* Copyright 1998-1999, Sidney Cammeresi. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <boot.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <blt/syscall.h>
#include <blt/namer.h>
#include <blt/error.h>
#include <blt/hash.h>
#include <blt/libsyms.h>
#include <blt/vfs.h>
#include "vfs-int.h"
#include "path.h"
#include "shm.h"

#define FG_GREEN  "\033[32m"
#define FG_RED    "\033[34m"
#define FG_WHITE  "\033[37m"

int vfs_port;
struct fs_type *fs_drivers;
struct superblock *mount_list = NULL;
hashtable_t *conn_table;

void __libc_init_vfs (void), __dlinit (void);

extern struct fs_type rootfs, bootfs;
extern void __libc_init_console ();
extern int __blk_ref;

vfs_res_t *vfs_openconn (int rport, int area)
{
	int i, private_port;
	vfs_res_t *res;
	struct client *client;

	res = malloc (sizeof (vfs_res_t));
	res->status = VFS_OK;
	res->errno = 0;
	private_port = port_create (rport, "vfs_private_port");
	port_slave (vfs_port, private_port);
	res->data[0] = private_port;

	client = malloc (sizeof (struct client));
	client->in = rport;
	client->out = private_port;
	client->filename_area = area;
	area_clone (area, 0, (void **) &client->nameptr, 0);
	client->ioctx.cwd = malloc (BLT_MAX_NAME_LENGTH);
	strcpy (client->ioctx.cwd, "/");
	for (i = 0; i < MAX_FDS; i++)
		client->ioctx.fdarray.ofile[i] = NULL;
	hashtable_insert (conn_table, client->in, client, sizeof (struct client));

#ifdef VFS_DEBUG
	printf ("vfs_openconn: from port %d, assigned port %d, area %d "
		"mapped to %p\n", rport, private_port, area, client->nameptr);
#endif
	return res;
}

vfs_res_t *vfs_scroll_area (struct client *client, vfs_cmd_t *vc)
{
	struct ofile *ofile;
	vfs_res_t *res;

	res = malloc (sizeof (vfs_res_t));
	ofile = client->ioctx.fdarray.ofile[vc->data[0]];
	shm_write (ofile, vc->data[1], 0, ofile->length, &res->data[0],
		&res->data[1]);
	res->status = VFS_OK;
	res->errno = 0;
	return res;
}

int vfs_mkdir (const char *dir, mode_t mode)
{
	const char *s;
	int res;
	struct superblock *super;
	struct vnode *vnode;

#ifdef VFS_DEBUG
	printf ("vfs_mkdir %s %d\n", dir, mode);
#endif

	/* find superblock and check if mkdir supported */
	super = fs_find (dir);
	if (super->sb_vops->mkdir == NULL)
		return ENOSYS;

	/* XXX dispatch to root vnode for now */
	vnode = super->sb_root;
	s = dir + strlen (super->sb_dir);
	res = super->sb_vops->mkdir (vnode, s, mode);
	return res;
}

vfs_res_t *vfs_opendir (struct client *client, vfs_cmd_t *vc)
{
	char *reldir, *dir;
	int i;
	vfs_res_t *res;
	struct ofile *ofile;
	struct superblock *super;
	struct vnode *vnode;

	reldir = client->nameptr + vc->data[0];
	res = malloc (sizeof (vfs_res_t));
#ifdef VFS_DEBUG
	printf ("vfs_opendir %s\n", reldir);
#endif

	/* get a file descriptor */
	for (i = 0; i < MAX_FDS; i++)
		if (client->ioctx.fdarray.ofile[i] == NULL)
			break;
	if (i == MAX_FDS)
	{
		res->status = VFS_ERROR;
		res->errno = EMFILE;
		return res;
	}

	/* is opendir supported? */
	dir = malloc (BLT_MAX_NAME_LENGTH);
	strlcpy (dir, client->ioctx.cwd, BLT_MAX_NAME_LENGTH);
	path_combine (client->ioctx.cwd, reldir, dir);
	super = fs_find (dir);
	if (super == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = ENOENT;
		free (dir);
		return res;
	}
	if (super->sb_vops->opendir == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = ENOSYS;
		free (dir);
		return res;
	}

	/* find directory's vnode */
	if (!strcmp (dir, super->sb_dir))
		vnode = super->sb_root;
	else
		vnode = super->sb_vops->walk (super->sb_root, dir +
			strlen (super->sb_dir) + 1);
	if (vnode == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = ENOENT;
		free (dir);
		return res;
	}

	/* stat here to check for directory */

	/* call filesystem's opendir and read the directory */
	ofile = client->ioctx.fdarray.ofile[i] = malloc (sizeof (struct ofile));
	ofile->o_vnode = vnode;
	ofile->area = area_clone (vc->data[1], 0, &ofile->dataptr, 0);
	ofile->offset = vc->data[2];
	ofile->length = vc->data[3];
	res->errno = super->sb_vops->opendir (vnode, &ofile->o_cookie);
	res->status = res->errno ? VFS_ERROR : VFS_OK;
	res->data[0] = i;
	free (dir);
	return res;
}

vfs_res_t *vfs_closedir (struct client *client, vfs_cmd_t *vc)
{
	vfs_res_t *res;
	struct ofile *ofile;
	struct vnode *vnode;

#ifdef VFS_DEBUG
	printf ("vfs_closedir %d\n", vc->data[0]);
#endif
	res = malloc (sizeof (vfs_res_t));

	/* is file descriptor valid? */
	if ((ofile = client->ioctx.fdarray.ofile[vc->data[0]]) == NULL)
	{
		res->errno = EBADF;
		res->status = VFS_ERROR;
		return res;
	}

	/* is closedir supported? */
	vnode = ofile->o_vnode;
	if (vnode->v_sb->sb_vops->closedir == NULL)
	{
		res->errno = ENOSYS;
		res->status = VFS_ERROR;
		return res;
	}

	/* call filesystem */
	vnode->v_sb->sb_vops->closedir (vnode, ofile->o_cookie);
	if (!res->errno && (vnode->v_sb->sb_vops->free_dircookie != NULL))
		vnode->v_sb->sb_vops->free_dircookie (ofile->o_cookie);
	free (ofile);
	client->ioctx.fdarray.ofile[vc->data[0]] = NULL;

	res->status = VFS_OK;
	res->errno = 0;
	return res;
}

vfs_res_t *vfs_open (struct client *client, vfs_cmd_t *vc)
{
	int i;
	char *path;
	vfs_res_t *res;
	struct ofile *ofile;
	struct superblock *super;
	struct vnode *vnode;

	res = malloc (sizeof (vfs_res_t));
	path = client->nameptr + vc->data[0];
#ifdef VFS_DEBUG
	printf ("vfs_open %s\n", path);
#endif

	/* get a file descriptor */
	for (i = 0; i < MAX_FDS; i++)
		if (client->ioctx.fdarray.ofile[i] == NULL)
			break;
	if (i == MAX_FDS)
	{
		res->status = VFS_ERROR;
		res->errno = EMFILE;
		return res;
	}

	/* is open supported? */
	super = fs_find (path);
	if (super == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = ENOENT;
		return res;
	}
	if (super->sb_vops->open == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = ENOSYS;
		return res;
	}

	/* find file's vnode */
	vnode = super->sb_vops->walk (super->sb_root, path +
		strlen (super->sb_dir) + 1);
	if (vnode == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = ENOENT;
		return res;
	}

	/* call filesystem */
	ofile = client->ioctx.fdarray.ofile[i] = malloc (sizeof (struct ofile));
	ofile->o_vnode = vnode;
	ofile->o_pos = 0;
	ofile->area = area_clone (vc->data[1], 0, &ofile->dataptr, 0);
	ofile->offset = vc->data[2];
	ofile->length = vc->data[3];
	res->errno = super->sb_vops->open (vnode, &ofile->o_cookie);
	res->status = res->errno ? VFS_ERROR : VFS_OK;
	res->data[0] = i;
	return res;
}

vfs_res_t *vfs_close (struct client *client, vfs_cmd_t *vc)
{
	vfs_res_t *res;
	struct ofile *ofile;
	struct superblock *super;

	res = malloc (sizeof (vfs_res_t));
#ifdef VFS_DEBUG
	printf ("vfs_close\n");
#endif

	ofile = client->ioctx.fdarray.ofile[vc->data[0]];
	if (ofile == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = EBADF;
		return res;
	}
	super = ofile->o_vnode->v_sb;
	if (super->sb_vops->close == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = ENOSYS;
		return res;
	}
	res->errno = super->sb_vops->close (ofile->o_vnode,
		ofile->o_cookie);

	if (super->sb_vops->free_cookie != NULL)
		super->sb_vops->free_cookie (ofile->o_cookie);

	res->status = res->errno ? VFS_ERROR : VFS_OK;
	return res;
}

vfs_res_t *vfs_read (struct client *client, vfs_cmd_t *vc, void **data,
	int *len)
{
	size_t numread;
	void *buf;
	vfs_res_t *res;
	struct ofile *ofile;
	struct superblock *super;

#ifdef VFS_DEBUG
	printf ("vfs_read\n");
#endif
	res = malloc (sizeof (vfs_res_t));
	buf = malloc (vc->data[1]);

	ofile = client->ioctx.fdarray.ofile[vc->data[0]];
	if (ofile == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = EBADF;
		return res;
	}
	super = ofile->o_vnode->v_sb;
	if (super->sb_vops->read == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = ENOSYS;
		return res;
	}

	res->errno = super->sb_vops->read (ofile->o_vnode, buf, vc->data[1],
		ofile->o_pos, &numread, ofile->o_cookie);
	res->status = res->errno ? VFS_ERROR : VFS_OK;
	res->data[0] = numread;
	ofile->o_pos += *len = numread;
	*data = buf;
	return res;
}

vfs_res_t *vfs_rstat (struct client *client, vfs_cmd_t *vc)
{
	char *path;
	vfs_res_t *res;
	struct superblock *super;
	struct vnode *vnode;
	struct stat *buf;

#ifdef VFS_DEBUG
	printf ("vfs_rstat\n");
#endif
	res = malloc (sizeof (vfs_res_t) + sizeof (struct stat));
	buf = (void *) res + sizeof (vfs_res_t);
	path = client->nameptr + vc->data[0];

	/* is stat supported? */
	super = fs_find (path);
	if (super == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = ENOENT;
		return res;
	}
	if (super->sb_vops->rstat == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = ENOSYS;
		return res;
	}

	/* find file's vnode */
	vnode = super->sb_vops->walk (super->sb_root, path +
		strlen (super->sb_dir) + 1);
	if (vnode == NULL)
	{
		res->status = VFS_ERROR;
		res->errno = ENOENT;
		return res;
	}

	/* call filesystem */
	res->errno = super->sb_vops->rstat (vnode, buf);
	res->status = res->errno ? VFS_ERROR : VFS_OK;
	return res;
}

void vfs_tell_cmd (const char *cmd, char *arg)
{
	char *name;
	const char *type, *dir, *data;
	int i, len;
	void *handle;

	if (!strcmp (cmd, "load"))
	{
		printf (" %s", arg);
        name = malloc (len = BLT_MAX_NAME_LENGTH);
        strlcpy (name, "/boot/", len);
        strlcat (name, arg, len);
        strlcat (name, ".so", len);
        handle = dlopen (name, 0);
        if (handle == NULL)
        {
            printf ("(error)");
            return;
        }
        free (name);
	}
	else if (!strcmp (cmd, "mount"))
	{
		type = arg;
		for (i = 0; arg[i] != ' '; i++) ;
		arg[i] = 0;
		dir = arg + i + 1;
		for (arg++; arg[i] != ' '; i++) ;
		arg[i] = 0;
		data = arg + i + 1;
		vfs_mount (dir, type, 0, data);
	}
}

void vfs_tell_parse (const char *msg)
{
    const char *c, *cmd;
    char *full;
    char *d;
    int i, whole, len;

    c = msg;
    cmd = full = NULL;
    whole = len = 0;
    while (*c)
    {
        i = 0;
        while ((c[i] != ' ') && c[i])
            i++;
        d = malloc (i + 1);
        strlcpy (d, c, i + 1);
        if (cmd == NULL)
        {
            cmd = d;
            if (!strcmp (cmd, "load"))
                printf ("vfs: loading drivers.  [");
            else if (!strcmp (cmd, "mount"))
            {
                whole = 1;
                full = malloc (len = strlen (msg + 1));
                *full = 0;
            }
        }
        else if (!whole)
            vfs_tell_cmd (cmd, d);
        else
        {
            strlcat (full, d, len);
            strlcat (full, " ", len);
        }
        if (i != strlen (c))
            c += i + 1;
        else
            break;
    }
	if (whole)
		full[strlen (full) - 1] = 0;
    if (!strcmp (cmd, "load"))
        printf (" ]\n");
	else if (!strcmp (cmd, "mount"))
		vfs_tell_cmd (cmd, full);
}

void vfs_tell (void)
{
	char buf[64];
	int port;
	msg_hdr_t mh;

	__libc_init_fdl ();
	__libc_init_vfs ();
	__dlinit ();
	__blk_ref++;

	port = port_create (0, "vfs:tell");
	namer_register (port, "vfs:tell");

	for (;;)
	{
		mh.src = 0;
		mh.dst = port;
		mh.data = buf;
		mh.size = sizeof (buf);
		old_port_recv (&mh);
		vfs_tell_parse (mh.data);

		mh.dst = mh.src;
		mh.src = port;
		mh.data = &port;
		mh.size = 1;
		old_port_send (&mh);
	}
}

int vfs_main (volatile int *ready)
{
	int i, size, len;
	void *data;
	msg_hdr_t msg, reply;
	vfs_cmd_t vc;
	vfs_res_t *res;
	struct client *client;

	/* open a connection to the console */
	__libc_init_console ();

	/* get a public port and register ourself with the namer */
	vfs_port = port_create (0, "vfs_listen_port");
	namer_register (vfs_port, "vfs");

	/* say hello */
#ifdef VFS_DEBUG
	printf ("vfs: " FG_GREEN "listener ready" FG_WHITE " (port %d)\n",
		vfs_port);
#endif

	/* initialise structures */
	fs_drivers = NULL;
	conn_table = hashtable_new (0.75);
	res = NULL;
	client = NULL;

	/* mount the root and boot filesystems */
	fs_register (&rootfs);
	fs_register (&bootfs);
	vfs_mount ("/", "rootfs", 0, NULL);
	vfs_mkdir ("/boot", 755);
	vfs_mount ("/boot", "bootfs", 0, NULL);
	thr_create (vfs_tell, 0, "vfs:tell");
	*ready = 1;

	for (;;)
	{
		/*
		 * listen for commands.  all ports will be slaved to the one
		 * we just created, so we only need to listen on one port.
		 */
		msg.src = 0;
		msg.dst = vfs_port;
		msg.data = &vc;
		msg.size = sizeof (vc);
		old_port_recv (&msg);

		if (vc.cmd != VFS_OPENCONN)
			client = hashtable_lookup (conn_table, msg.src, NULL);
		size = sizeof (vfs_res_t);
		data = NULL;

		switch (vc.cmd)
		{
			case VFS_OPENCONN:
				res = vfs_openconn (msg.src, vc.data[0]);
				break;

			case VFS_SCROLL_AREA:
				res = vfs_scroll_area (client, &vc);
				break;

			case VFS_OPENDIR:
				res = vfs_opendir (client, &vc);
				if (res->status == VFS_OK)
					shm_write_dir (client->ioctx.fdarray.ofile[res->data[0]],
						0, 0, vc.data[3], &res->data[1], &res->data[2]);
				break;

			case VFS_CLOSEDIR:
				res = vfs_closedir (client, &vc);
				break;

			case VFS_OPEN:
				res = vfs_open (client, &vc);
				if (res->status == VFS_OK)
					shm_write (client->ioctx.fdarray.ofile[res->data[0]],
						0, 0, vc.data[3], &res->data[1], &res->data[2]);
				break;

			case VFS_CLOSE:
				res = vfs_close (client, &vc);
				break;

			case VFS_READ:
				res = vfs_read (client, &vc, &data, &len);
				break;

			case VFS_RSTAT:
				res = vfs_rstat (client, &vc);
				size = sizeof (vfs_res_t) + sizeof (struct stat);
				break;

			case VFS_MKDIR:
				res = malloc (sizeof (vfs_res_t));
				res->status = vfs_mkdir (client->nameptr + vc.data[0],
					vc.data[1]);
				res->errno = 0;
				break;
		}

		if (res != NULL)
		{
			reply.src = (vc.cmd == VFS_OPENCONN) ? vfs_port : client->out;
			reply.dst = msg.src;
			reply.data = res;
			reply.size = size;
			old_port_send (&reply);
			free (res);

			if (data != NULL)
			{
				reply.src = client->out;
				reply.dst = msg.src;
				if (len < 0x1000)
				{
					reply.data = data;
					reply.size = len;
					old_port_send (&reply);
				}
				else
				{
					for (i = 0; len > 0x1000; i += 0x1000, len -= 0x1000)
					{
						reply.data = (char *) data + i;
						reply.size = 0x1000;
						old_port_send (&reply);
					}
					reply.data = (char *) data + i;
					reply.size = len;
					old_port_send (&reply);
				}
				free (data);
			}
		}
	}

	/* not reached */
	return 0;
}

int main (void)
{
	volatile int ready = 0;

	thr_create (vfs_main, (int *) &ready, "vfs");
	while (!ready) ;
	return 0;
}

