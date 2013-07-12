/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "types.h"

#define KM16   0
#define KM32   1
#define KM64   2
#define KM96   3
#define KM128  4
#define KM192  5
#define KM256  6
#define KMMAX  7

uint32 getpage(void);                 /* allocate a single physical page */
void putpage(uint32);                 /* release a single physical page */

void *kgetpage(uint32 *phys);         /* allocate one page (and return phys addr) */
void *kgetpages(int count);           /* allocate count pages */
void kfreepage(void *vaddr);
void *kmallocP(int pool);             /* allocate from pool (eg KM16, KM128, ...)*/
void kfreeP(int pool, void *block);   /* release back to pool */

void *kmallocB(int size);             /* allocate from appropriate pool for size */
void kfreeB(int size, void *block);   /* release back to appropriate pool */
#define kmalloc(type) kmallocB(sizeof(type))
#define kfree(type,ptr) kfreeB(sizeof(type),ptr)

void memory_init(uint32 bottom, uint32 top); /* only call once - on kernel launch */
void memory_status(void);             /* dump memory usage information */

#endif

