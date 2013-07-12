/* $Id: //depot/blt/srv/vfs/sandbox/blkdev.c#1 $
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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "blkdev.h"

int blk_open (const char *name, int flags, blkdev_t **retdev)
{
	int fd;
	struct stat statbuf;

	if (stat (name, &statbuf))
	{
		*retdev = NULL;
		return -1;
	}
	if ((fd = open (name, O_RDONLY, 0)) < 0)
	{
		*retdev = NULL;
		return -1;
	}
	*retdev = malloc (sizeof (blkdev_t));
	(*retdev)->blksize = 512;
	(*retdev)->devno = fd;
	return 0;
}

int blk_close (blkdev_t *dev)
{
	close (dev->devno);
	free (dev);
	return 0;
}

int blk_read (blkdev_t *dev, void *buf, int block, int count)
{
	int i;

	lseek (dev->devno, block * dev->blksize, SEEK_SET);
	for (i = 0; i < count; i++)
		read (dev->devno, buf + dev->blksize * i, dev->blksize);
	return 0;
}

int blk_write (blkdev_t dev, const void *buf, int block, int count)
{
	return 0;
}

