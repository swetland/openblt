/* $Id: //depot/blt/include/blt/disk.h#4 $
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

#ifndef _BLT_DISK_H_
#define _BLT_DISK_H_

#include <blt/blkdev.h>

#define FDISK_TYPE_EMPTY         0x00
#define FDISK_TYPE_FAT12         0x01
#define FDISK_TYPE_FAT16_SMALL   0x04
#define FDISK_TYPE_EXTENDED      0x05
#define FDISK_TYPE_FAT16_BIG     0x06
#define FDISK_TYPE_NTFS          0x07
#define FDISK_TYPE_LINUX_SWAP    0x82
#define FDISK_TYPE_EXT2          0x83
#define FDISK_TYPE_FREEBSD       0xa5
#define FDISK_TYPE_OPENBSD       0xa6
#define FDISK_TYPE_BFS           0xeb

#define FREEBSD_FFS              0x100
#define FREEBSD_SWAP             0x101
#define OPENBSD_FFS              0x102
#define OPENBSD_SWAP             0x103

typedef struct
{
	unsigned char bootflag;
	unsigned char start0;
	unsigned char start1;
	unsigned char start2;
	unsigned char type;
	unsigned char end0;
	unsigned char end1;
	unsigned char end2;
	unsigned int start_sect_num;
	unsigned int num_sects;
} fdisk_partition;

typedef struct
{
	char name[3]; /* 0, 0a, etc. */
	int type;
	unsigned long long start, size;
} partition_t;

typedef struct
{
	blkdev_t *dev;
	int numparts;
	partition_t *partition;
} disk_t;

#ifdef __cplusplus
extern "C" {
#endif

	disk_t *disk_alloc (blkdev_t *dev);
	void disk_free (disk_t *disk);
	const char *disk_partition_name (disk_t *disk, int partition);
	unsigned long long disk_partition_start (disk_t *disk, int partition);
	unsigned long long disk_partition_size (disk_t *disk, int partition);
	unsigned int disk_partition_type (disk_t *disk, int partition);

#ifdef __cplusplus
}
#endif


/*
 * Copyright (c) 1987, 1988, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the University of
 *        California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  @(#)disklabel.h 8.2 (Berkeley) 7/10/94
 */

#define NDDATA 5
#define NSPARE 5
#define BSD_MAXSLICES 16
#define BSD_DISKLABEL_MAGIC    0x82564557

typedef struct
{
    unsigned int d_magic;      /* the magic number */
    unsigned short d_type;     /* drive type */
    unsigned short d_subtype;  /* controller/d_type specific */
    char d_typename[16];       /* type name, e.g. "eagle" */

    /*
     * d_packname contains the pack identifier and is returned when
     * the disklabel is read off the disk or in-core copy.
     * d_boot0 and d_boot1 are the (optional) names of the
     * primary (block 0) and secondary (block 1-15) bootstraps
     * as found in /usr/mdec.  These are returned when using
     * getdiskbyname(3) to retrieve the values from /etc/disktab.
     */
    union
	{
        char un_d_packname[16];  /* pack identifier */
        struct
		{
            char *un_d_boot0;    /* primary bootstrap name */
            char *un_d_boot1;    /* secondary bootstrap name */
        } un_b;
    } d_un;

    /* disk geometry: */
    unsigned int d_secsize;        /* # of bytes per sector */
    unsigned int d_nsectors;       /* # of data sectors per track */
    unsigned int d_ntracks;        /* # of tracks per cylinder */
    unsigned int d_ncylinders;     /* # of data cylinders per unit */
    unsigned int d_secpercyl;      /* # of data sectors per cylinder */
    unsigned int d_secperunit;     /* # of data sectors per unit */

    /*
     * Spares (bad sector replacements) below are not counted in
     * d_nsectors or d_secpercyl.  Spare sectors are assumed to
     * be physical sectors which occupy space at the end of each
     * track and/or cylinder.
     */
    unsigned short d_sparespertrack; /* # of spare sectors per track */
    unsigned short d_sparespercyl;   /* # of spare sectors per cylinder */

    /*
     * Alternate cylinders include maintenance, replacement, configuration
     * description areas, etc.
     */
    unsigned int d_acylinders;     /* # of alt. cylinders per unit */

    /*
     * d_interleave, d_trackskew and d_cylskew describe perturbations
     * in the media format used to compensate for a slow controller.
     * Interleave is physical sector interleave, set up by the
     * formatter or controller when formatting.  When interleaving is
     * in use, logically adjacent sectors are not physically
     * contiguous, but instead are separated by some number of
     * sectors.  It is specified as the ratio of physical sectors
     * traversed per logical sector.  Thus an interleave of 1:1
     * implies contiguous layout, while 2:1 implies that logical
     * sector 0 is separated by one sector from logical sector 1.
     * d_trackskew is the offset of sector 0 on track N relative to
     * sector 0 on track N-1 on the same cylinder.  Finally, d_cylskew
     * is the offset of sector 0 on cylinder N relative to sector 0
     * on cylinder N-1.
     */
    unsigned short d_rpm;              /* rotational speed */
    unsigned short d_interleave;       /* hardware sector interleave */
    unsigned short d_trackskew;        /* sector 0 skew, per track */
    unsigned short d_cylskew;          /* sector 0 skew, per cylinder */
    unsigned int d_headswitch;         /* head switch time, usec */
    unsigned int d_trkseek;            /* track-to-track seek, usec */
    unsigned int d_flags;              /* generic flags */
    unsigned int d_drivedata[NDDATA];  /* drive-type specific information */
    unsigned int d_spare[NSPARE];      /* reserved for future use */
    unsigned int d_magic2;             /* the magic number (again) */
    unsigned short d_checksum;         /* xor of data incl. partitions */

    /* filesystem and partition information: */
    unsigned short d_npartitions;      /* number of partitions in following */
    unsigned int d_bbsize;             /* size of boot area at sn0, bytes */
    unsigned int d_sbsize;             /* max size of fs superblock, bytes */

    struct partition                   /* the partition table */
	{                          
        unsigned int p_size;           /* number of sectors in partition */
        unsigned int p_offset;         /* starting sector */
        unsigned int p_fsize;          /* filesystem basic fragment size */
        unsigned char p_fstype;        /* filesystem type, see below */
        unsigned char p_frag;          /* filesystem fragments per block */
        union
		{
            unsigned short cpg;        /* UFS: FS cylinders per group */
            unsigned short sgs;        /* LFS: FS segment shift */
        } p_u1;
    } d_partitions[BSD_MAXSLICES];     /* actually may be more */
} bsd_disklabel;

#define BSD_FS_UNUSED   0       /* unused */
#define BSD_FS_SWAP     1       /* swap */
#define BSD_FS_V6       2       /* Sixth Edition */
#define BSD_FS_V7       3       /* Seventh Edition */
#define BSD_FS_SYSV     4       /* System V */
#define BSD_FS_V71K     5       /* V7 with 1K blocks (4.1, 2.9) */
#define BSD_FS_V8       6       /* Eighth Edition, 4K blocks */
#define BSD_FS_BSDFFS   7       /* 4.2BSD fast file system */
#define BSD_FS_MSDOS    8       /* MSDOS file system */
#define BSD_FS_BSDLFS   9       /* 4.4BSD log-structured file system */
#define BSD_FS_OTHER    10      /* in use, but unknown/unsupported */
#define BSD_FS_HPFS     11      /* OS/2 high-performance file system */
#define BSD_FS_ISO9660  12      /* ISO 9660, normally CD-ROM */
#define BSD_FS_BOOT     13      /* partition contains bootstrap */
#define BSD_FS_ADOS     14      /* AmigaDOS fast file system */
#define BSD_FS_HFS      15      /* Macintosh HFS */
#define BSD_FS_ADFS     16      /* Acorn Disk Filing System */
#define BSD_FS_EXT2FS   17      /* ext2fs */
#define BSD_FS_CCD      18      /* ccd component */

#endif

