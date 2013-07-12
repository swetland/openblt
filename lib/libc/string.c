/* $Id: //depot/blt/lib/libc/string.c#4 $
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

#include <string.h>

char *strncpy(char *dst, const char *src, size_t size)
{
    char *d = dst;

    while(size && *src) size--, *dst++ = *src++;            
    if(size) *dst = 0;

    return d;        
}

int strncmp(const char *s1, const char *s2, size_t n)
{    
    while(n && *s1 && *s2){
        if(*s1 != *s2) return -1;
        s1++;
        s2++;
        n--;
        
    }
    if(n) return -1;

    return 0;    
}

char *strchr (const char *cs, int c)
{
	const char *s;

	s = cs;
	while (*s)
		{
			if (*s == c)
				return (char *) s;
			s++;
		}
	return NULL;
}

void *memset(void *s, int c, size_t n)
{
    unsigned char *u = (unsigned char *) s;
    while(n) {
        *u = c;
        u++;
        n--;
    }
    return s;
}

void *memmove(void *dest, const void *src, size_t n)
{
    unsigned char *d;
    unsigned char *s;

    if(dest < src){
        d = (unsigned char *)dest;
        s = (unsigned char *)src;
        while(n){
            *d = *s;
            d++;
            s++;
            n--;
        }
    } else {
        d = ((unsigned char *) dest)+n;
        s = ((unsigned char *) src)+n;
        while(n){
            d--;
            s--;
            *d = *s;
            n--;
        }        
    }
    return dest;
}

int memcmp(const void *dst, const void *src, size_t size)
{
    while(size) {
	if(*((char *)dst) != *((char *)src)) return 1;
	((char *) dst)++;
	((char *) src)++;
        size--;        
    }
    return 0;    
}

void *memcpy(void *dst, const void *src, size_t size)
{
    void *r = dst;
    
    while(size) {
        *(((unsigned char *) dst)++) = *(((unsigned char *) src)++);
        size--;        
    }
    return r;
}

