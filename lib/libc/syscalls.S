/* $Id: //depot/blt/lib/libc/syscalls.S#3 $
**
** Copyright 1998 Brian J. Swetland
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

#include <blt/syscall_id.h>
#include <i386/asm.h>

#define SYSCALL(name)          \
	FUNCTION(name) ;             \
	movl $BLT_SYS_##name, %eax ; \
	int $0x20 ;                  \
	ret

SYSCALL(os_meta)
SYSCALL(os_terminate)
SYSCALL(os_console)
SYSCALL(os_brk)
SYSCALL(os_handle_irq)
SYSCALL(os_sleep_irq)
SYSCALL(os_debug)
SYSCALL(os_sleep)

SYSCALL(sem_create)
SYSCALL(sem_destroy)
SYSCALL(sem_acquire)
SYSCALL(sem_release)

SYSCALL(port_create)
SYSCALL(port_destroy)
SYSCALL(port_option)
SYSCALL(port_send)
SYSCALL(port_recv)

SYSCALL(os_identify)

SYSCALL(right_create)
SYSCALL(right_destroy)
SYSCALL(right_revoke)
SYSCALL(right_attach)
SYSCALL(right_grant)

SYSCALL(thr_create)
SYSCALL(thr_spawn)
SYSCALL(thr_resume)
SYSCALL(thr_suspend)
SYSCALL(thr_kill)
SYSCALL(thr_wait)

SYSCALL(area_create)
SYSCALL(area_clone)
SYSCALL(area_destroy)
SYSCALL(area_resize)
