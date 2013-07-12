/* Copyright 1999, Sidney Cammeresi. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <blt/syscall.h>
#include <blt/namer.h>
#include <blt/os.h>
#include <blt/blkdev.h>
#include <blt/disk.h>
#include "ide-int.h"

void ide_btochs (int block, ide_dev_t *dev, int *cyl, int *head, int *sect)
{
	*cyl = block / (dev->hwdev->heads * dev->hwdev->sectors);
	block %= dev->hwdev->heads * dev->hwdev->sectors;
	*head = block / dev->hwdev->sectors;
	block %= dev->hwdev->sectors;
	*sect = block;
}

void ide_main (int *ready)
{
	char *c, dev[3];
	int a, b, i, port, txnlen, reslen, bus, device, offset;
	msg_hdr_t mh;
	blktxn_t *txn;
	blkres_t *res;
	disk_t *disk;

	if (!ide_probe_devices ())
	{
		printf ("ide: no devices found; exiting.\n");
		os_terminate (1);
	}

	port = port_create (0, "ide");
	namer_register (port, "ide");
	txn = malloc (txnlen = sizeof (blktxn_t) + 1024);
	res = malloc (reslen = sizeof (blkres_t) + 1024);
	*ready = 1;

	for (;;)
	{
		mh.src = 0;
		mh.dst = port;
		mh.data = txn;
		mh.size = txnlen;
		old_port_recv (&mh);

		switch (txn->cmd)
		{
			case BLK_CMD_OPEN:
				c = (char *) (txn + 1);
				if (!isdigit (c[0]) || !isdigit (c[2]) || (c[1] != '/') ||
						(c[3] != '/'))
				{
					res->status = ENOENT;
					break;
				}
				bus = c[0] - '0';
				device = c[2] - '0';
				if ((bus >= total_busses) || ((device != 0) && (device != 1)))
					res->status = ENOENT;
				else if (ide_dev[bus * 2 + device] == NULL)
					res->status = ENOENT;
				else if (!strcmp (c + 4, "raw"))
				{
					res->status = 0;
					res->data[0] = 512;
					res->data[1] = (bus * 2 + device) | (0xff << 8);
					mh.size = sizeof (blkres_t);
				}
				else if (isdigit (c[4]) && !c[5])
				{
					res->status = 0;
					res->data[0] = 512;
					res->data[1] = (bus * 2 + device) | ((c[4] - '0' + 1) << 8);
					mh.size = sizeof (blkres_t);
				}
				else if (isdigit (c[4]) && c[5])
				{
					res->status = 0;
					res->data[0] = 512;
					res->data[1] = (bus * 2 + device) | ((c[4] - '0' + 1) <<
						8) | ((c[5] - 'a' + 1) << 16);
					mh.size = sizeof (blkres_t);
				}
				else
					res->status = ENOENT;
				break;

			case BLK_CMD_READ:
				if ((txn->device & 0xff) > (total_busses * 2))
				{
					res->status = ENOENT;
					break;
				}
				offset = 0;
				if ((txn->device >> 8))
				{
					a = ((txn->device & 0x0000ff00) >> 8);
					b = ((txn->device & 0x00ff0000) >> 16);
					dev[0] = a ? a - 1 + '0' : 0;
					dev[1] = b ? b - 1 + 'a' : 0;
					dev[2] = 0;
					disk = ide_dev[txn->device & 0xff]->disk;
					for (i = 0; i < disk->numparts; i++)
						if (!strcmp (disk_partition_name (disk, i), dev))
							offset = disk_partition_start (disk, i);
					txn->device &= 0xff;
				}
				res->status = ide_dev[txn->device]->read (txn->device / 2,
					txn->device % 2, res + 1, txn->block + offset);
				mh.size = res->status ? sizeof (blkres_t) : sizeof (blkres_t) +
					512;
				break;
		}

		mh.dst = mh.src;
		mh.src = port;
		mh.data = res;
		old_port_send (&mh);
	}
}

int main (void)
{
	volatile int ready;

	ready = 0;
	thr_create (ide_main, (void *) &ready, "ide");
	while (!ready) ;
	return 0;
}

