/* Copyright 1999, Sidney Cammeresi. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <stdarg.h>
#include <string.h>
#include <blt/namer.h>
#include <blt/syscall.h>
#include <blt/libsyms.h>

void va_snprintf (char *s, int len, const char *fmt, ...);

void __libc_bind_console(void);
ssize_t __libc_write_console(const void *buf, ssize_t len);
ssize_t __libc_read_console(void *buf, ssize_t);

void __libc_init_console (void)
{
	__libc_bind_console ();
}

weak_alias (_printf, __libc_printf)
weak_alias (_printf, printf)
link_warning (__libc_printf, "warning: __libc_printf is deprecated; use printf instead.")

init_info __init_posix_console = {
	&__libc_init_console,
	2
};

int _printf(char *fmt,...)
{   
	char buf[256];

	va_list pvar;
	va_start(pvar,fmt);
	va_snprintf(buf,256,fmt,pvar);
	va_end(pvar);
	
	__libc_write_console(buf, strlen(buf));
	
	return 0;
}

