/* Copyright 2000, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <string.h>
#include <blt/namer.h>
#include <blt/syscall.h>
#include <blt/libsyms.h>

static int __libc_console_local_port = -1;
static int __libc_console_out_port = -1;

/* 0 = register, 1 = send, 2 = recv */

int __libc_bind_console (void) 
{	
	if(__libc_console_local_port < 0) {
		__libc_console_out_port = namer_find("console",1);
		__libc_console_local_port = port_create(0, "console_io");
		
		if(port_send(__libc_console_local_port,
					 __libc_console_out_port,
					 NULL, 0, 0) == 0) {
			return 0;
		}
		
		port_destroy(__libc_console_local_port);
		__libc_console_local_port = -1;
	}
	return -1;
}

size_t __libc_write_console(const void *buf, size_t len)
{
	const char *x = (const char *) buf;
	ssize_t l = len;
	
	while(len > 0){
		if(len > 255) {
			port_send(__libc_console_local_port, __libc_console_out_port,
					  (void*) x, 255, 1);
			len -= 255;
			x += 255;
		} else {
			port_send(__libc_console_local_port, __libc_console_out_port,
					  (void*) x, len, 1);
			len = 0;
		}		  
	}
	return l;
}

ssize_t __libc_read_console(void *buf, ssize_t len)
{
	char *x = (char *) buf;
	ssize_t r, t;
	
	t = 0;
	while(len) {
		r = port_recv(__libc_console_local_port, NULL, x, 1, NULL);
		if(r < 0) return 0;
		x++;
		t++;
		len--;
	}
	
	return t;	
}

void grab_console(void)
{
	port_send(__libc_console_local_port, __libc_console_out_port,
			  NULL, 0, 0);
}
