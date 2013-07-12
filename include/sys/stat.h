/* $Id: //depot/blt/include/sys/stat.h#4 $
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

#ifndef _SYS_STAT_H_
#define _SYS_STAT_H_

#include <sys/types.h>

struct stat
{
	dev_t st_dev;                  /* device on which filesystem resides */
	ino_t st_ino;                  /* inode number */
	mode_t st_mode;                /* inode protection mode */
	nlink_t st_nlink;              /* number of hard links to file */
	uid_t st_uid;                  /* user id of owner */
	gid_t st_gid;                  /* group id of owner */
	dev_t st_rdev;                 /* device type for device inodes */
	off_t st_size;                 /* file size in bytes */
	uint32 st_blocks;              /* file size in blocks */
	uint32 st_blksize;             /* size of block */
	struct timespec st_atimespec;  /* time of last access */
	struct timespec st_mtimespec;  /* time of last modification */
	struct timespec st_ctimespec;  /* time of last status change */
	uint32 st_flags;               /* user-defined flags */
	uint32 st_gen;                 /* file generation number */
};

#ifdef __cplusplus
extern "C" {
#endif

int _stat (const char *path, struct stat *buf);
int stat (const char *path, struct stat *buf);

int _mkdir (const char *path, mode_t mode);
int mkdir (const char *path, mode_t mode);

#ifdef __cplusplus
}
#endif


#endif

