/* Copyright 1998-1999, Sidney Cammeresi. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _INIT_H_
#define _INIT_H_

/*
 * Behold macros to place functions or variables in special elf sections.  To
 * use these jewels, write your functions like this
 *
 *         void __init__ foo_init (int a, int b)
 *         {
 *         }
 *
 * and your function prototypes like this
 *
 *         extern void foo_init (int a, int b) __init__;
 *
 * and your data like this
 *
 *         static char *foo_stuff __initdata__ = "Foo rules the world.";
 *
 * At some point in the future, the .text.init and .data.init sections of the
 * kernel may be discarded at the end of the kernel's initialisation to free
 * up a bit of memory.
 */

#define __init__            __attribute__ ((__section__ (".text.init")))
#define __initdata__        __attribute__ ((__section__ (".data.init")))

#endif

