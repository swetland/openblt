/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifdef __SMP__
#include "smp.h"
#endif

void kmain(void);

/*
 * Our state here depends on how we got here.  If we are called from the
 * first stage bootstrap, we go into the kernel setup code.  We can also
 * arrive here executing on an application processor that has just completed
 * its move into the wonderful world of protected mode.  If this is the case,
 * we jump into smp code to finish our setup and wait for word from the
 * bootstrap processor.
 */
void _start(void)
{
#ifdef __SMP__
    /* have we detected more processors yet? */
    if (smp_num_cpus > 1)
				smp_cpu_ready ();
    else
#endif 
        kmain();
    /* not reached */
}

