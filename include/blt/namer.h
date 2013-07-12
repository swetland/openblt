/* Copyright 1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _NAMER_H_
#define _NAMER_H_

#include <blt/types.h>

#define NAMER_PORT 1

#define NAMER_FIND      1
#define NAMER_REGISTER  2

#ifdef __cplusplus
extern "C" {
#endif

int namer_register(int port, const char *name);
int namer_find(const char *name, int blocking);

#ifdef __cplusplus
}
#endif

#endif

