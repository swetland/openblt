/* $Id: //depot/blt/srv/vfs/sandbox/vfs.c#2 $
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
#include <dirent.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include "../vfs-int.h"

#define NAME_PREFIX    "../drivers/"
#define NAME_SUFFIX    ".so"

struct fs_type *fs_drivers;
struct superblock *mount_list = NULL;

void usage (void)
{
	printf ("usage: vfs driver file\n");
	exit (1);
}

int main (int argc, char **argv)
{
	char *buf, c;
	int i, len, num, pos, res;
	void *fs, *cookie;
	struct stat statbuf;
	struct dirent dirent;
	struct vnode *vn;

	if (argc < 3)
		usage ();

	buf = malloc (strlen (NAME_PREFIX) + strlen (NAME_SUFFIX) +
		strlen (argv[1]) * 2 + 2);
	strcpy (buf, NAME_PREFIX);
	strcat (buf, argv[1]);
	strcat (buf, "/");
	strcat (buf, argv[1]);
	strcat (buf, NAME_SUFFIX);

	if (stat (buf, &statbuf))
	{
		printf ("driver `%s' not found\n", argv[1]);
		exit (1);
	}
	if ((fs = dlopen (buf, 0)) == NULL)
	{
		printf ("error loading driver `%s'\n", argv[1]);
		exit (1);
	}

	vfs_mount ("/", argv[1], 0, argv[2]);
	free (buf);

/*
	vn = fs_drivers->t_vops->walk (mount_list->sb_root, "etc/afs/ThisCell");
	vput (vn);
	vn = fs_drivers->t_vops->walk (mount_list->sb_root,
		"etc/kerberosIV/krb.conf");
	vput (vn);
	vn = fs_drivers->t_vops->walk (mount_list->sb_root, "bin/df");
	vput (vn);
*/
/*
	vn = fs_drivers->t_vops->walk (mount_list->sb_root, "etc");
	fs_drivers->t_vops->opendir (vn, &cookie);
	vput (vn);
*/
/*
	vn = fs_drivers->t_vops->walk (mount_list->sb_root, "bin");
	printf ("vnid is %lld\n", vn->v_vnid);
	fs_drivers->t_vops->opendir (vn, &cookie);
	while (!fs_drivers->t_vops->readdir (vn, &dirent, cookie))
		printf ("name %s\n", dirent.d_name);
	fs_drivers->t_vops->rewinddir (vn, cookie);
	fs_drivers->t_vops->closedir (vn, cookie);
	fs_drivers->t_vops->free_dircookie (cookie);
	vput (vn);
*/
/*
	buf = malloc (len = 13062);
	vn = fs_drivers->t_vops->walk (mount_list->sb_root, "etc/rc");
	fs_drivers->t_vops->open (vn, &cookie);
	res = fs_drivers->t_vops->read (vn, buf, len, pos = 0, &num, cookie);
	printf ("read %d %d bytes\n", res, num);
	for (i = 0; i < len; i++)
		printf ("%c", buf[i]);
	vput (vn);
*/
/*
	pos = 0;
	vn = fs_drivers->t_vops->walk (mount_list->sb_root, "etc/rc");
	fs_drivers->t_vops->open (vn, &cookie);
	do
	{
		res = fs_drivers->t_vops->read (vn, &c, 1, pos++, &num, cookie);
		printf ("%c", c);
	}
	while (!res && num);
	vput (vn);
*/
/*
	//pos = 12 * 8192;
	pos = 0;
	buf = malloc (len = 8192);
	vn = fs_drivers->t_vops->walk (mount_list->sb_root, "etc/magic");
	fs_drivers->t_vops->open (vn, &cookie);
	do
	{
		res = fs_drivers->t_vops->read (vn, buf, len, pos, &num, cookie);
		pos += num;
		for (i = 0; i < num; i++)
			printf ("%c", buf[i]);
	}
	while (!res && num);
	vput (vn);
	free (buf);
*/

	vn = fs_drivers->t_vops->walk (mount_list->sb_root, "etc/magic");
	fs_drivers->t_vops->rstat (vn, &statbuf);
	printf ("inode is %lld, nlink is %d, uid is %d, gid is %d, size is %d\n",
		statbuf.st_ino, statbuf.st_nlink, statbuf.st_uid, statbuf.st_gid,
		statbuf.st_size);
	vput (vn);

	return 0;
}

