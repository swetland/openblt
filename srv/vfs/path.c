/* $Id: //depot/blt/srv/vfs/path.c#2 $
**
** Copyright 1999 Sidney Cammeresi.
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
#include "path.h"

char *path_concat (char *s, const char *t)
{
	char *c;

	c = s;
	while (*c++) ;
	c--;

	while (*t) /* for each component */
		if (*t == '/')
			*c++ = *t++;
		else if (*t != '.') /* normal component */
			while (*t && (*t != '/'))
				*c++ = *t++;
		else /* doesn't begin with . or / */
		{   
			t++;
			if ((*t == '/') || !*t) /* ./ component */
				t++;
			else if (*t == '.') /* ../ component */
			{   
				if (c != s + 1) /* can't .. from the root dir */
				{   
					c -= 2;
					while ((c != s) && (*c != '/'))
						c--;
					c++;
					t++;
					if (*t == '/')
						t++;
				}
			}
			else /* component beginning with a period */
			{   
				*c++ = '.';
				while (*t && (*t != '/'))
					*c++ = *t++;
			}
		}

	*c = 0;
	if (((c - 1) != s) && (c[-1] == '/'))
		c[-1] = 0;

	return s;
}

char *path_combine (const char *s, const char *t, char *d)
{
	const char *c;
	char *node;

	*d = 0;
	node = d;
	if (*t == '/')
		return path_concat (node, t);

	c = s;
	while (*c)
		*d++ = *c++;
	if (*d != '/')
		*d++ = '/';

	return path_concat (node, t);
}

