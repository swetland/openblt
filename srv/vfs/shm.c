/* $Id: //depot/blt/srv/vfs/shm.c#6 $
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
#include <string.h>
#include <dirent.h>
#include "bootfs.h"
#include "shm.h"

void shm_write_dir (struct ofile *ofile, int skip, int doff, int len,
	int *numents, int *more)
{
	int i, res;
	struct dirent dirent;
	struct vnode *vnode;
	register struct vnode_ops *ops;

	vnode = ofile->o_vnode;
	ops = vnode->v_sb->sb_vops;
	ops->rewinddir (vnode, ofile->o_cookie);
	for (i = 0; i < skip; i++)
		ops->readdir (vnode, &dirent, ofile->o_cookie);
	for (i = res = 0; (i < (len / sizeof (struct dirent))) && !res; i++)
	{
		res = ops->readdir (vnode, &dirent, ofile->o_cookie);
		if (!res)
			memcpy (ofile->dataptr + doff + i * sizeof (struct dirent),
				&dirent, sizeof (struct dirent));
	}
	*numents = (i == 0) ? 0 : i - 1;
	*more = 0;
}

void shm_write (struct ofile *ofile, int skip, int doff, int len,
	int *numbytes, int *more)
{
#if 0
	char buf[256];
	int i, res;
	struct vnode *vnode;
	register struct vnode_ops *ops;

	vnode = ofile->o_vnode;
	ops = vnode->v_sb->sb_vops;
/*
	FIXME - this works without this code for now, but won't when we get lseek
	for (i = 0; i < skip; i++)
		ops->read (vnode, buf, 1, ofile->o_cookie);
*/
	for (i = 0, res = 1; (i < len) && res; i += res)
		if ((res = ops->read (vnode, buf, sizeof (buf), ofile->o_cookie)) > 0)
			memcpy (ofile->dataptr + doff + i, buf, res);
	*numbytes = i;
	*more = (i == len);
#ifdef SHM_DEBUG
	printf ("shm_write: %x / %x %x %x %x %x\n", ofile->dataptr + doff,
		*numbytes, i, *more, len, skip);
#endif
#endif
}

