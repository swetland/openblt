/* Copyright 1999, Sidney Cammeresi. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <blt/libsyms.h>
#include <blt/blkdev.h>
#include <blt/namer.h>
#include <blt/syscall.h>

weak_alias (_blk_open, blk_open)
weak_alias (_blk_close, blk_close)
weak_alias (_blk_read, blk_read)
weak_alias (_blk_write, blk_write)

int __blk_ref;

int _blk_open (const char *name, int flags, blkdev_t **retdev)
{
	char *server;
	int i, len;
	msg_hdr_t mh;
	blkdev_t *dev;
	blktxn_t *txn;
	blkres_t res;

	for (i = 0; name[i] && (name[i] != '/') && i < BLT_MAX_NAME_LENGTH; i++) ;
	server = malloc (i);
	strncpy (server, name, i);
	server[i] = 0;

	dev = malloc (sizeof (blkdev_t));
	dev->remote_port = namer_find (server, 1);

	txn = malloc (len = sizeof (blktxn_t) + strlen (name) - i + 1);
	txn->cmd = BLK_CMD_OPEN;
	strcpy ((char *) (txn + 1), name + i + 1);

	mh.src = dev->local_port = port_create (dev->remote_port, "blkdev");
	mh.dst = dev->remote_port;
	mh.data = txn;
	mh.size = len;
	old_port_send (&mh);

	mh.src = dev->remote_port;
	mh.dst = dev->local_port;
	mh.data = &res;
	mh.size = sizeof (blkres_t);
	old_port_recv (&mh);

	if (res.status)
	{
		free (dev);
		*retdev = NULL;
		return res.status;
	}

	dev->blksize = res.data[0];
	dev->devno = res.data[1];
	*retdev = dev;
	return 0;
}

int _blk_close (blkdev_t *dev)
{
	free (dev);
	return 0;
}

int _blk_read (blkdev_t *dev, void *buf, int block, int count)
{
	int len, ret;
	msg_hdr_t mh;
	blktxn_t txn;
	blkres_t *res;

	res = malloc (len = sizeof (blkres_t) + dev->blksize);

	while (count)
	{
		txn.device = dev->devno;
		txn.cmd = BLK_CMD_READ;
		txn.block = block;
		txn.count = count;
		mh.src = dev->local_port;
		mh.dst = dev->remote_port;
		mh.data = &txn;
		mh.size = sizeof (blktxn_t);
		old_port_send (&mh);

		mh.src = dev->remote_port;
		mh.dst = dev->local_port;
		mh.data = res;
		mh.size = len;
		old_port_recv (&mh);

		if (res->status)
			goto done;
		memcpy (buf, res + 1, dev->blksize);
		buf = (char *) buf + dev->blksize;
		block++;
		count--;
	}

done:
	ret = res->status;
	free (res);
	return ret;
}

int _blk_write (blkdev_t *dev, const void *buf, int block, int count)
{
	return 0;
}

