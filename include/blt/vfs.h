/* $Id: //depot/blt/include/blt/vfs.h#7 $
**
** Copyright 1998-1999 Sidney Cammeresi
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

#ifndef _BLT_VFS_H_
#define _BLT_VFS_H_

#include <blt/types.h>
#include <blt/fdl.h>

/* operations */
#define VFS_OPENCONN                 1
#define VFS_CLOSECONN                2
#define VFS_SCROLL_AREA              19

#define VFS_MOUNT                    3
#define VFS_UNMOUNT                  4
#define VFS_INITIALISE               5
#define VFS_SYNC                     6
#define VFS_RFSTAT                   7
#define VFS_WFSTAT                   8

#define VFS_CREATE                   9
#define VFS_MKDIR                    10
#define VFS_SYMLINK                  11
#define VFS_LINK                     12
#define VFS_RENAME                   13
#define VFS_UNLINK                   14
#define VFS_RMDIR                    15
#define VFS_READLINK                 16

#define VFS_OPENDIR                  17
#define VFS_CLOSEDIR                 18

#define VFS_OPEN                     21
#define VFS_CLOSE                    22
#define VFS_READ                     23
#define VFS_WRITE                    24
#define VFS_IOCTL                    25
#define VFS_SETFLAGS                 26
#define VFS_RSTAT                    27
#define VFS_WSTAT                    28
#define VFS_FSYNC                    29

#define VFS_OPEN_ATTRDIR             30
#define VFS_CLOSE_ATTRDIR            31
#define VFS_REWIND_ATTRDIR           32
#define VFS_READ_ATTRDIR             33
#define VFS_READ_ATTR                34
#define VFS_WRITE_ATTR               35
#define VFS_REMOVE_ATTR              36
#define VFS_RENAME_ATTR              37
#define VFS_STAT_ATTR                38

#define VFS_OPEN_INDEXDIR            39
#define VFS_CLOSE_INDEXDIR           40
#define VFS_REWIND_INDEXDIR          41
#define VFS_READ_INDEXDIR            42
#define VFS_READ_INDEX               43
#define VFS_WRITE_INDEX              44
#define VFS_REMOVE_INDEX             45
#define VFS_RENAME_INDEX             46
#define VFS_STAT_INDEX               47

#define VFS_OPEN_QUERY               48
#define VFS_CLOSE_QUERY              49
#define VFS_READ_QUERY               50

/* vfs result codes */
#define VFS_OK                        1
#define VFS_ERROR                     2
#define VFS_MORE_DATA                 4

typedef struct
{
	unsigned int cmd;
	unsigned int data[6];
} vfs_cmd_t;

typedef struct
{
	unsigned int status, errno;
	unsigned int data[6];
} vfs_res_t;

typedef struct
{
	char *buf;
	int srv_fd, area_offset;
	int offset, length, more;
} vfs_fd;

#endif

