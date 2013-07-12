/* Copyright 1999, Sidney Cammeresi. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <blt/fdl.h>
#include <blt/syscall.h>
#include <blt/namer.h>
#include <blt/libsyms.h>

int __libc_bind_console(void);
ssize_t __libc_write_console(const void *buf, ssize_t len);
ssize_t __libc_read_console(void *buf, ssize_t);

static fdl_type console_input_fdl_imp = { "console_input", _console_read,
    NULL, NULL, NULL };

FILE *stdin, *stdout, *stderr;

weak_alias (_getc, getc)
weak_alias (_getchar, getchar)

void __libc_init_console_input ()
{
	int fd;

	__libc_bind_console();
	
	fd = _fdl_alloc_descriptor (&console_input_fdl_imp, NULL);
	if (fd)
		_printf ("__libc_init_input: console input not on fd 0\n");
	stdin = malloc (sizeof (FILE));
	stdin->fd = 0;
}

int _console_read (void *cookie, void *buf, size_t count)
{
	return __libc_read_console(buf, count);
}

int _getc (FILE *stream)
{
	char c;

	_read (stream->fd, &c, 1);
	return c;
}

int _getchar (void)
{
	return _getc (stdin);
}

