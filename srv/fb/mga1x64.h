/* $Id$
**
** Copyright 1999 Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _MGA_1X64_H
#define _MGA_1X64_H

#define VENDOR 0x102b
#define DEVICE 0x051a

typedef struct mga1x64
{
	volatile uint32 *regs;
	volatile uchar *fb;
	int32 fbsize;
} mga1x64;

/* base registers to use */
#define REGS 1
#define FB 2

/* registers */
#define FIFOSTATUS 0x1e10

#define RD(r) (mga->regs[(r)/4])
#define WR(r,v) (mga->regs[(r)/4] = v)

#endif
