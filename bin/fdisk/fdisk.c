/* $Id: //depot/blt/bin/fdisk/fdisk.c#6 $
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
#include <blt/blkdev.h>
#include <blt/disk.h>

const static char *def_device = "ide/0/0/raw";

int main (int argc, char **argv)
{
	const char *devname;
	unsigned char *buf;
	int i;
	blkdev_t *dev;
	disk_t *disk;

	if (argc == 1)
	{
		printf ("fdisk: no device specified; using default of %s\n",
			def_device);
		devname = def_device;
	}
	else
		devname = argv[1];
	
	blk_open (devname, 0, &dev);
	if (dev == NULL)
	{
		printf ("fdisk: error opening %s\n", devname);
		return 1;
	}
	buf = malloc (dev->blksize * 2);
	blk_read (dev, buf, 0, 2);

	disk = disk_alloc (dev);
	for (i = 0; i < disk->numparts; i++)
		printf ("partition %s%s, type %X%X, start = %d, size = %d\n",
			disk_partition_name (disk, i),
			disk_partition_name (disk, i)[1] ? "" : " ",
			((int) disk_partition_type (disk, i) & 0xff00) >> 8,
			(int) disk_partition_type (disk, i) & 0xff,
			(int) disk_partition_start (disk, i),
			(int) disk_partition_size (disk, i));
	disk_free (disk);

	blk_close (dev);
	free (buf);

	return 0;
}

