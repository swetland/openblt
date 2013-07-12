/* $Id$
**
** Copyright 1999 Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <stdio.h>
#include <stdarg.h>
#include <blt/syscall.h>
#include "fb.h"

#define SERIAL_DEBUG 0

void va_snprintf(char *b, int l, char *fmt, va_list pvar);


void dprintf(const char *fmt, ...)
{
	char buf[256];
	va_list ap;
	va_start(ap,fmt);
	va_snprintf(buf,256,fmt,ap);
	va_end(pvar);
#if SERIAL_DEBUG
	os_console(buf);
#else
	printf(buf);
	printf("\n");
#endif
}

extern fb_info fb_mga1x64;

int main (int argc, char **argv)
{
	void *cookie;	
	fb_info *fb = &fb_mga1x64;
	
	if(fb->find(&cookie) == 0){
		printf("Frame Buffer \"%s\" found.\n",fb->name);
		fb->init(cookie);
	}
	
	return 0;
}

