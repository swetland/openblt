/* $Id: //depot/blt/include/blt/conio.h#6 $
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

#ifndef _CONIO_H
#define _CONIO_H

#include <stdarg.h>

#ifdef MONO
#define CON_SCREEN	0x000B0000
#else
#define CON_SCREEN	0x000B8000
#endif

#define CON_BLACK       0
#define CON_BLUE        1
#define CON_GREEN       2
#define CON_CYAN        3
#define CON_RED         4
#define CON_MAGENTA     5
#define CON_YELLOW      6
#define CON_WHITE       7

#ifdef __cplusplus
extern "C" {
#endif

	void con_attr(int a);
	void con_clear(void);
	void con_putc(char ch);
	void con_puts(char *s);
	void con_putp(unsigned int p);
	void con_putx(unsigned char x);
	void con_goto(int x, int y);
	
	#define con_fgbg(fg,bg) con_attr((bg) << 4 | (fg));
	
	#define con_init() { con_attr(CON_WHITE); con_clear(); }
	
	void con_start(unsigned int video);
	
	void cprintf(char *fmt, ...);

#ifdef __cplusplus
}
#endif


#endif /* _CONIO_H */
