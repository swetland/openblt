/* $Id: //depot/blt/include/stdlib.h#7 $
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
#ifndef _stdlib_h
#define _stdlib_h

#include <blt/types.h>

#ifdef __cplusplus
extern "C" {
#endif

	void *_malloc(size_t size);
	void *malloc(size_t size);
	void _free(void *ptr);
	void free(void *ptr);
	void *_realloc(void *ptr, size_t size);
	void *realloc(void *ptr, size_t size);
	
	void _exit (int status);
	void exit (int status);
	
	void _qsort (void *base, size_t nmembers, size_t membsize,
		int (*compar)(const void *, const void *));
	void qsort (void *base, size_t nmembers, size_t membsize,
		int (*compar)(const void *, const void *));
	
	int _atoi (const char *a);
	int atoi (const char *a);

#ifdef __cplusplus
}
#endif


#endif

