/* $Id: //depot/blt/srv/ide/disk.c#4 $
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

#include "ide-int.h"
#include <stdio.h>
#include <i386/io.h>

int ide_disk_read (int bus, int device, void *data, int block)
{
	unsigned short *buf;
	int i, base, cyl, head, sect;

	base = ide_base (bus);
	buf = (unsigned short *) data;
	ide_btochs (block, ide_dev[bus * 2 + device], &cyl, &head, &sect);
	IDE_WAIT_0 (STATUS, 7);

	IDE_SEL_DH (bus, device, head);
	IDE_WAIT_0 (STATUS, 7);
	//IDE_WAIT_1 (STATUS, 6);

	outb (cyl / 256, IDE_REG_CYLMSB);
	outb (cyl % 256, IDE_REG_CYLLSB);
	outb (sect + 1, IDE_REG_SECNUM);
	outb (1, IDE_REG_SECCNT);
	outb (IDE_OP_READ, IDE_REG_COMMAND);
	IDE_WAIT_0 (STATUS, 7);
	IDE_WAIT_1 (STATUS, 3);

	for (i = 0; i < 256; i++)
		buf[i] = inw (IDE_REG_DATA);
	return 0;
}

int ide_disk_write (int bus, int device, const void *data, int block)
{
#if 0
	short *buf;
	int i, base, cyl, head, sect;

	base = ide_base (bus);
	buf = (short *) data;
	ide_btochs (block, ide_dev[bus * 2 + device], &cyl, &head, &sect);
	IDE_WAIT_0 (STATUS, 7);

	/* select device */
	IDE_SEL_DH (bus, device, head);
	IDE_WAIT_0 (STATUS, 7);
	IDE_WAIT_1 (STATUS, 6);

	outb (cyl / 256, IDE_REG_CYLMSB);
	outb (cyl % 256, IDE_REG_CYLLSB);
	outb (sect, IDE_REG_SECNUM);
	outb (1, IDE_REG_SECCNT);
	outb (IDE_OP_WRITE, IDE_REG_COMMAND);
	IDE_WAIT_1 (STATUS, 3);

	for (i = 0; i < 256; i++)
		outw (buf[i], IDE_REG_DATA);
#endif

	return 0;
}

