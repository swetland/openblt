/* $Id: //depot/blt/srv/ide/blkdev.c#2 $
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
#include <blt/blkdev.h>
#include "ide-int.h"

int blk_open (const char *name, int flags, blkdev_t **retdev)
{
	blkdev_t *dev;

	dev = malloc (sizeof (blkdev_t));
	dev->blksize = 512;
	dev->devno = (name[4] - '0') * 2 + (name[6] - '0');
	*retdev = dev;
	return 0;
}

int blk_close (blkdev_t *dev)
{
	free (dev);
	return 0;
}

int blk_read (blkdev_t *dev, void *buf, int block, int count)
{
	int i;

	for (i = 0; i < count; i++)
		ide_dev[dev->devno]->read ((dev->devno & 0xff) / 2, dev->devno % 2,
			(char *) buf + dev->blksize * i, block + i);
	return 0;
}

int blk_write (blkdev_t *dev, const void *buf, int block, int count)
{
	return 0;
}

