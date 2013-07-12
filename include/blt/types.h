/* Copyright 1998-2000 Brian J. Swetland.  All rights reserved.
** Distributed under the terms of the OpenBLT License.
*/

#ifndef _BLT_TYPES_H
#define _BLT_TYPES_H

#define CHAR_BIT 8
#define NULL ((void *) 0)

#define ntohs(n) ( (((n) & 0xFF00) >> 8) | (((n) & 0x00FF) << 8) )
#define htons(n) ( (((n) & 0xFF00) >> 8) | (((n) & 0x00FF) << 8) )

typedef unsigned long long uint64;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
typedef long long int64;
typedef int int32;
typedef short int16;
typedef char int8;
typedef unsigned char uchar;

typedef int ssize_t;
typedef unsigned long int size_t;

typedef unsigned int time_t;

typedef unsigned int off_t;
typedef unsigned int mode_t;
typedef unsigned int dev_t;
typedef unsigned int ino_t;
typedef unsigned int nlink_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;

/* system resources */
typedef int port_id;
typedef int area_id;
typedef int sem_id;
typedef int thread_id;

#endif
