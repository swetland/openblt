/* $Id: //depot/blt/include/blt/blkdev.h#2 $
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

#ifndef _BLT_BLKDEV_H_
#define _BLT_BLKDEV_H_

#include <blt/types.h>

#define BLK_CMD_OPEN         1
#define BLK_CMD_READ         2
#define BLK_CMD_WRITE        3

typedef struct
{
	int blksize;
	int local_port, remote_port;
	dev_t devno;
} blkdev_t;

typedef struct
{
	dev_t device;
	unsigned int cmd, block, count;
} blktxn_t;

typedef struct
{
	dev_t device;
	int status;
	int data[2];
} blkres_t;

#ifdef __cplusplus
extern "C" {
#endif

	int _blk_open (const char *name, int flags, blkdev_t **dev);
	int blk_open (const char *name, int flags, blkdev_t **dev);
	int _blk_close (blkdev_t *dev);
	int blk_close (blkdev_t *dev);
	
	int _blk_read (blkdev_t *dev, void *buf, int block, int count);
	int blk_read (blkdev_t *dev, void *buf, int block, int count);
	int _blk_write (blkdev_t *dev, const void *buf, int block, int count);
	int blk_write (blkdev_t *dev, const void *buf, int block, int count);

#ifdef __cplusplus
}
#endif


#endif

