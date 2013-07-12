/* $Id: //depot/blt/bin/uname/uname.c#3 $
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
#include <unistd.h>

#define ATTR(attr, value) \
	if (attr) \
	{ \
		if (start) \
			printf (" "); \
		else \
			start = 1; \
		printf (value); \
	}

extern char *ostype, *osrelease, *osversion;

int main (int argc, char **argv)
{
	int opt, any, start, machine, nodename, processor, system, release, version;

	any = machine = nodename = processor = system = release = version = 0;
	while ((opt = getopt (argc, argv, "amnpsrv")) != -1)
		switch (opt)
		{
			case 'a':
				any = machine = nodename = processor = system = release =
					version = 1;
				break;

			case 'm':
				any = machine = 1;
				break;

			case 'n':
				any = nodename = 1;
				break;

			case 'p':
				any = processor = 1;
				break;

			case 's':
				any = system = 1;
				break;

			case 'r':
				any = release = 1;
				break;

			case 'v':
				any = version = 1;
				break;

			default:
				return 1;
		}

	if (!any)
		system = 1;
	start = 0;
	ATTR (system, ostype)
	ATTR (nodename, "(noname)")
	ATTR (release, osrelease)
	ATTR (version, osversion)
	ATTR (machine, "Intel")
	printf ("\n");
	return 0;
}

