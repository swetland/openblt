/* $Id: //depot/blt/srv/ide/identify.c#6 $
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
#include <stdlib.h>
#include <blt/syscall.h>
#include <blt/types.h>
#include <blt/disk.h>
#include <blt/blkdev.h>
#include <i386/io.h>
#include "ide-int.h"

int total_busses = 0;
ide_dev_t **ide_dev;

static void ide_string_conv (char *str, int len)
{
	int i;
	unsigned short *w;

	w = (unsigned short *) str;
	for (i = 0; i < len / sizeof (unsigned short); i++)
		w[i] = ntohs (w[i]);

	str[len - 1] = 0;
	for (i = len - 1; (i >= 0) && ((str[i] == ' ') || !str[i]); i--)
		str[i] = 0;
}

int ide_reset (int bus, int device)
{
	int i, base, fail = 0;

	outb (6, IDE_DIGITAL_OUTPUT);
	for (i = 0; i < 1000000; i++) ; /* XXX */
	outb (0, IDE_DIGITAL_OUTPUT);
	//IDE_WAIT_1 (STATUS, 6);

	base = ide_base (bus);
	IDE_WAIT_0 (STATUS, 7);
	IDE_SEL_DH (bus, device, 0);
	IDE_WAIT_0 (STATUS, 7);
	IDE_WAITFAIL_1 (STATUS, 6);
	if (fail)
		return 1;

	outb (0, IDE_REG_CYLLSB);
	outb (0, IDE_REG_CYLMSB);
	outb (0, IDE_REG_SECNUM);
	outb (0, IDE_REG_SECCNT);
	outb (IDE_OP_RECALIBRATE, IDE_REG_COMMAND);
	return 0;
}

const ide_hw_id_t *ide_identify_device (int bus, int device)
{
	short *buf;
	int i, base, fail, mb;
	ide_hw_id_t *hwdev;

	if ((base = ide_base (bus)) < 0)
		return NULL;
	if ((device < 0) || (device > 1))
		return NULL;

	fail = 0;
	buf = (short *) hwdev = malloc (sizeof (ide_hw_id_t));
	IDE_WAIT_0 (STATUS, 7);

	IDE_SEL_DH (bus, device, 0);
	IDE_WAIT_0 (STATUS, 7);
	IDE_WAITFAIL_1 (STATUS, 6);
	if (fail)
		return NULL;

	outb (IDE_OP_IDENTIFY_DEVICE, IDE_REG_COMMAND);
	IDE_WAIT_1 (STATUS, 3);

	for (i = 0; i < sizeof (ide_hw_id_t) / sizeof (short); i++)
		buf[i] = inw (IDE_REG_DATA);

	ide_string_conv (hwdev->serial, sizeof (hwdev->serial));
	ide_string_conv (hwdev->firmware, sizeof (hwdev->firmware));
	ide_string_conv (hwdev->model, sizeof (hwdev->model));
	mb = hwdev->cyls * hwdev->heads * hwdev->sectors * 512 / 1024 / 1024;

	printf ("ide: disk at bus %d, device %d, <%s>\n", bus, device,
		hwdev->model);
	printf ("ide/%d/%d: %dMB; %d cyl, %d head, %d sec, 512 bytes/sec\n",
		bus, device, mb, hwdev->cyls, hwdev->heads, hwdev->sectors);
	return hwdev;
}

int ide_attach_bus (int io, int irq)
{
	total_busses++;
	return 0;
}

int ide_attach_device (int bus, int device)
{
	char name[9];
	int i;
	blkdev_t *bdev;
	disk_t *disk;
	const ide_hw_id_t *hwdev;
	ide_dev_t *dev;

	if ((hwdev = ide_identify_device (bus, device)) == NULL)
		return 0;

	dev = ide_dev[bus * 2 + device] = malloc (sizeof (ide_dev_t));
	dev->hwdev = hwdev;
	dev->iobase = ide_base (bus);
	dev->read = ide_disk_read;
	dev->write = ide_disk_write;

	snprintf (name, sizeof (name), "ide/%d/%d", bus, device);
	printf ("%s: partitions:", name);
	blk_open (name, 0, &bdev);
	dev->disk = disk = disk_alloc (bdev);
	for (i = 0; i < disk->numparts; i++)
		printf (" %s", disk_partition_name (disk, i));
	printf ("\n");
	blk_close (bdev);
	return 1;
}

int ide_probe_devices (void)
{
	int i, found;

	found = 0;
	os_handle_irq (14);
	ide_attach_bus (0x1f0, 14); /* XXX find this properly */
	ide_dev = malloc (sizeof (ide_dev_t *) * total_busses * 2);

	for (i = 0; i < total_busses; i++)
	{
		//if (!ide_reset (i, 0))
			found += ide_attach_device (i, 0);
		//if (!ide_reset (i, 1))
			found += ide_attach_device (i, 1);
	}
	return found;
}

