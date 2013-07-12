/* $Id: //depot/blt/include/blt/libsyms.h#1 $
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

#ifndef __BLT_LIBSYMS_H__
#define __BLT_LIBSYMS_H__

/*
 * this creates a weak alias from the symbol original to the symbol alias.
 * functions like printf should actually be defined as _printf in libc,
 * with weak aliases linking printf to _printf.  by doing this, a user can
 * put a wrapper around the actual libc functions without rebuilding
 * libc (for profiling, say).  i.e.,
 *
 *     weak_alias (_printf, printf)
 *     int _printf (char *fmt, ...);
 *
 *     printf ("foo");
 *
 * results in a call to _printf.
 */
#define weak_alias(original, alias) \
	asm (".weak " #alias " ; " #alias " = " #original);

/*
 * this causes the GNU linker to emit a warning when this symbol is
 * referenced during a linking stage.
 *
 * only the name of the section matters.  this must be char[], not char *
 */
#define link_warning(symbol, text) \
	static const char ___link_warning_##symbol[] \
	__attribute__ ((section (".gnu.warning." #symbol))) = text;

#endif

