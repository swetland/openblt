/* $Id: //depot/blt/bin/ls/ls.c#4 $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static int dirent_compare (const void *a, const void *b)
{
	const struct dirent *c, *d;

	c = a, d = b;
	return strcmp (c->d_name, d->d_name);
}

int main (int argc, char **argv)
{
	char *path;
	int i, j, k, l, maxlen, maxnum;
	struct dirent *ent[128];
	DIR *dir;

	path = (argc == 1) ? "/" : argv[1];
	i = maxlen = 0;
	dir = opendir (path);
	if (dir == NULL)
	{
		printf ("ls: %s: no such file or directory\n", path);
		return 0;
	}
	while ((ent[i] = readdir (dir)) != NULL)
	{
		maxlen = (strlen (ent[i]->d_name) > maxlen) ? strlen (ent[i]->d_name) :
			maxlen;
		i++;
	}
	closedir (dir);
	maxnum = 80 / (maxlen + 2);
	qsort (*ent, i, sizeof (struct dirent), dirent_compare);
	for (k = 0; k < i; k+= j)
	{
		for (j = 0; ((j + k) < i) && (j < maxnum); j++)
		{
			printf ("%s", ent[j + k]->d_name);
			for (l = strlen (ent[j + k]->d_name); l < (maxlen + 2); l++)
				printf (" ");
		}
		printf ("\n");
	}

	return 0;
}

