/* $Id: //depot/blt/lib/libposix/fdl.c#1 $
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

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <blt/qsem.h>
#include <blt/libsyms.h>
#include <blt/fdl.h>
#include <blt/os.h>

static __filedesc **fd_table;
static qsem_t *fd_table_lock;

weak_alias (_read, read)
weak_alias (_write, write)
weak_alias (_ioctl, ioctl)
weak_alias (_close, close)

void __libc_init_fdl (void)
{
	int i;

	fd_table = malloc (sizeof (__filedesc *) * MAX_FDS);
	for (i = 0; i < MAX_FDS; i++)
		fd_table[i] = NULL;
	fd_table_lock = qsem_create (1);
}

init_info __init_posix_fdl = {
	&__libc_init_fdl,
	1
};

void __libc_fini_fdl (void)
{
	free (fd_table);
	qsem_destroy (fd_table_lock);
}

int _fdl_alloc_descriptor (fdl_type *handler, void *cookie)
{
	int i;

	qsem_acquire (fd_table_lock);
	for (i = 0; i < MAX_FDS; i++)
		if (fd_table[i] == NULL)
		{
			fd_table[i] = malloc (sizeof (__filedesc));
			fd_table[i]->imp = handler;
			fd_table[i]->cookie = cookie;
			qsem_release (fd_table_lock);
			return i;
		}
	qsem_release (fd_table_lock);
	return -1;
}

void _fdl_free_descriptor (int desc)
{
	fd_table[desc] = NULL;
}

ssize_t _read (int fd, void *buf, size_t count)
{
	if (fd_table[fd]->imp->read != NULL)
		return fd_table[fd]->imp->read (fd_table[fd]->cookie, buf, count);
	else
	{
		errno = ENOSYS;
		return -1;
	}
}

ssize_t _write (int fd, const void *buf, size_t count)
{
	if (fd_table[fd]->imp->write != NULL)
		return fd_table[fd]->imp->write (fd_table[fd]->cookie, buf, count);
	else
	{
		errno = ENOSYS;
		return -1;
	}
}

int _ioctl (int fd, unsigned long request, char *argp)
{
	if (fd_table[fd]->imp->ioctl != NULL)
		return fd_table[fd]->imp->ioctl (fd_table[fd]->cookie, request, argp);
	else
	{
		errno = ENOSYS;
		return -1;
	}
}

int _close (int fd)
{
	int res;

	if (fd_table[fd]->imp->close != NULL)
	{
		res = fd_table[fd]->imp->close (fd_table[fd]->cookie);
		fd_table[fd] = NULL;
		return res;
	}
	else
	{
		errno = ENOSYS;
		return -1;
	}
}

