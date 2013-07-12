/* $Id: //depot/blt/srv/ide/ide-int.h#5 $
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

#ifndef IDE_INT_H
#define IDE_INT_H

#include <blt/disk.h>

#define IDE_PRI_BASE                0x1f0
#define IDE_SEC_BASE                0x170
#define IDE_DIGITAL_OUTPUT          0x3f6
#define IDE_DRIVE_ADDR              0x3f7

#define IDE_REG_OFF_DATA            0 /* R/W, 16 bits */
#define IDE_REG_OFF_ERROR           1 /* R   */
#define IDE_REG_OFF_PRECOMP         1 /* W   */
#define IDE_REG_OFF_SECCNT          2 /* R/W */
#define IDE_REG_OFF_SECNUM          3 /* R/W */
#define IDE_REG_OFF_CYLLSB          4 /* R/W */
#define IDE_REG_OFF_CYLMSB          5 /* R/W */
#define IDE_REG_OFF_DH              6 /* R/W */
#define IDE_REG_OFF_STATUS          7 /* R   */
#define IDE_REG_OFF_COMMAND         7 /* W   */

#define IDE_REG_DATA                (base + IDE_REG_OFF_DATA)
#define IDE_REG_ERROR               (base + IDE_REG_OFF_ERROR)
#define IDE_REG_PRECOMP             (base + IDE_REG_OFF_PRECOMP)
#define IDE_REG_SECCNT              (base + IDE_REG_OFF_SECCNT)
#define IDE_REG_SECNUM              (base + IDE_REG_OFF_SECNUM)
#define IDE_REG_CYLLSB              (base + IDE_REG_OFF_CYLLSB)
#define IDE_REG_CYLMSB              (base + IDE_REG_OFF_CYLMSB)
#define IDE_REG_DH                  (base + IDE_REG_OFF_DH)
#define IDE_REG_STATUS              (base + IDE_REG_OFF_STATUS)
#define IDE_REG_COMMAND             (base + IDE_REG_OFF_COMMAND)

#define IDE_SEL_DH(bus, device, head) \
	outb (0x1010000 | (device << 4) | head, IDE_REG_DH)

#define IDE_WAIT_0(reg, bit) \
	while (inb (IDE_REG_##reg) & (1 << bit)) ;
#define IDE_WAIT_1(reg, bit) \
	while (!(inb (IDE_REG_##reg) & (1 << bit))) ;

/* XXX - hack */
#define IDE_WAITFAIL_1(reg, bit) \
	{ \
		int __spin__ = 0; \
		while (!(inb (IDE_REG_##reg) & (1 << bit)) && (__spin__ < 10000)) \
			__spin__++; \
		if (__spin__ == 10000) \
			fail = 1; \
	}

#define IDE_OP_RECALIBRATE          0x10
#define IDE_OP_READ                 0x20
#define IDE_OP_WRITE                0x30
#define IDE_OP_IDENTIFY_DEVICE      0xec

#define IDE_IDENTIFY_DATA_SIZE      256

typedef struct
{
	unsigned short config;               /* obsolete stuff */
	unsigned short cyls;                 /* logical cylinders */
	unsigned short _reserved_2;
	unsigned short heads;                /* logical heads */
	unsigned short _vendor_4;
	unsigned short _vendor_5;
	unsigned short sectors;              /* logical sectors */
	unsigned short _vendor_7;
	unsigned short _vendor_8;
	unsigned short _vendor_9;
	char serial[20];                     /* serial number */
	unsigned short _vendor_20;
	unsigned short _vendor_21;
	unsigned short vend_bytes_long;      /* no. vendor bytes on long cmd */
	char firmware[8];
	char model[40];
	unsigned short mult_support;         /* vendor stuff and multiple cmds */
	unsigned short _reserved_48;
	unsigned short capabilities;
	unsigned short _reserved_50;
	unsigned short pio;
	unsigned short dma;
	unsigned short _reserved_53;
	unsigned short curr_cyls;            /* current logical cylinders */
	unsigned short curr_heads;           /* current logical heads */
	unsigned short curr_sectors;         /* current logical sectors */
	unsigned int capacity;               /* capacity in sectors */
	unsigned short _pad[256-59];         /* don't need this stuff for now */
} ide_hw_id_t;

typedef struct
{
	const ide_hw_id_t *hwdev;
	int iobase;
	disk_t *disk;
	int (*read) (int bus, int device, void *buf, int block);
	int (*write) (int bus, int device, const void *buf, int block);
} ide_dev_t;

extern ide_dev_t **ide_dev;
extern int total_busses;

void ide_btochs (int block, ide_dev_t *dev, int *cyl, int *head, int *sect);

int ide_probe_devices (void);

int ide_disk_read (int bus, int device, void *data, int block);
int ide_disk_write (int bus, int device, const void *data, int block);

static inline int ide_base (int bus)
{
    if (bus == 0)
        return IDE_PRI_BASE;
    else if (bus == 1)
        return IDE_SEC_BASE;
    else
        return -1;
}

#endif

