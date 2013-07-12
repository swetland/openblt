/* $Id: //depot/blt/srv/vfs/vnode.c#4 $
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
#include "vfs-int.h"

struct vnode *vget (struct superblock *super, int vnid)
{
	struct vnode *v;

#ifdef VNODE_DEBUG
	printf ("vfs: vget %d\n", vnid);
#endif
	v = malloc (sizeof (struct vnode));
	v->v_vnid = vnid;
	v->v_refcount = 1;
	v->v_sb = super;
#ifndef VFS_SANDBOX
	v->v_lock = qsem_create (1);
#endif
	super->sb_vops->read_vnode (v);
	return v;
}

void vput (struct vnode *vnode)
{
	vnode->v_sb->sb_vops->drop_vnode (vnode);
#ifndef VFS_SANDBOX
	qsem_destroy (vnode->v_lock);
#endif
	vnode->v_refcount--;
	if (!vnode->v_refcount)
		free (vnode);
}

