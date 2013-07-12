/* $Id: //depot/blt/lib/libblt/disk.c#4 $
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
#include <blt/disk.h>

static int blt_internal_code (int fdisk, int fstype)
{
	if (fdisk == FDISK_TYPE_FREEBSD)
		if (fstype == BSD_FS_BSDFFS)
			return FREEBSD_FFS;
		else if (fstype == BSD_FS_SWAP)
			return FREEBSD_SWAP;
		else
			return 0;
	else if (fdisk == FDISK_TYPE_OPENBSD)
		if (fstype == BSD_FS_BSDFFS)
			return OPENBSD_FFS;
		else if (fstype == BSD_FS_SWAP)
			return OPENBSD_SWAP;
		else
			return 0;
	else
		return 0;
}

disk_t *disk_alloc (blkdev_t *dev)
{
	unsigned char *mbr, *label;
	int i, j, x;
	disk_t *disk;
	fdisk_partition *fp;
	bsd_disklabel *slice;

	disk = malloc (sizeof (disk_t));
	disk->dev = dev;
	disk->numparts = 0;
	mbr = malloc (dev->blksize);
	label = malloc (dev->blksize);
	slice = (bsd_disklabel *) label;

	/* count fdisk partitions */
	blk_read (dev, mbr, 0, 1);
	for (i = 0; i < 4; i++)
	{
		fp = (fdisk_partition *) (mbr + 0x1be) + i;
		if (fp->type)
		switch (fp->type)
		{
			case FDISK_TYPE_EMPTY:
				break;

			case FDISK_TYPE_FREEBSD:
			case FDISK_TYPE_OPENBSD:
				disk->numparts++;
				blk_read (dev, label, fp->start_sect_num + 1, 1);
				for (j = 0; j < BSD_MAXSLICES; j++)
					if (slice->d_partitions[j].p_fstype != BSD_FS_UNUSED)
						disk->numparts++;
				break;

			default:
				disk->numparts++;
				break;
		}
	}
	disk->partition = malloc (sizeof (partition_t) * disk->numparts);

	/* create partition data, fdisk partitions first */
	for (i = x = 0; i < 4; i++)
	{
		fp = (fdisk_partition *) (mbr + 0x1be) + i;
		if (fp->type)
		{
			disk->partition[x].name[0] = '0' + i;
			disk->partition[x].name[1] = 0;
			disk->partition[x].start = fp->start_sect_num;
			disk->partition[x].size = fp->num_sects;
			disk->partition[x].type = fp->type;
			x++;
		}
	}

	/* create data for bsd slices */
	for (i = 0; i < 4; i++)
	{
		fp = (fdisk_partition *) (mbr + 0x1be) + i;
		switch (fp->type)
		{
			case FDISK_TYPE_FREEBSD:
			case FDISK_TYPE_OPENBSD:
				blk_read (dev, label, fp->start_sect_num + 1, 1);
				for (j = 0; j < BSD_MAXSLICES; j++)
					if (slice->d_partitions[j].p_fstype != BSD_FS_UNUSED)
					{
						disk->partition[x].name[0] = '0' + i;
						disk->partition[x].name[1] = 'a' + j;
						disk->partition[x].name[2] = 0;
						disk->partition[x].start =
							slice->d_partitions[j].p_offset;
						disk->partition[x].size =
							slice->d_partitions[j].p_size;
						disk->partition[x].type = blt_internal_code (fp->type,
							slice->d_partitions[j].p_fstype);
						x++;
					}
				break;
		}
	}

	free (mbr);
	free (label);
	return disk;
}

void disk_free (disk_t *disk)
{
	free (disk->partition);
	free (disk);
}

const char *disk_partition_name (disk_t *disk, int partition)
{
	return disk->partition[partition].name;
}

unsigned long long disk_partition_start (disk_t *disk, int partition)
{
	return disk->partition[partition].start;
}

unsigned long long disk_partition_size (disk_t *disk, int partition)
{
	return disk->partition[partition].size;
}

unsigned int disk_partition_type (disk_t *disk, int partition)
{
	return disk->partition[partition].type;
}

