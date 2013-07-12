/* $Id: //depot/blt/bin/bltsh/bltsh.c#12 $
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
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <blt/syscall.h>

void grab_console(void);
char **params;
void __libc_init_console_input (void);

void do_command(int argc, char **argv)
{
	struct stat s;
	int thid;
	
	if(!strcmp(argv[0], "exit")) os_terminate (1);
	
	if(!stat(argv[0],&s)){
		if((thid = execve(argv[0],argv,NULL)) > 0){
			thr_wait(thid);
			grab_console();
		} else {
			printf("bltsh: failed to execve(): %s\n", argv[0]);
		}	
	} else {
		/* try our "path" */
		char *x = (char *) malloc(7 + strlen(argv[0]));
		
		strcpy(x,"/boot/");
		strcpy(x+6,argv[0]);
		free(argv[0]);
		argv[0] = x;
		
		if(!stat(x,&s)){
			if((thid = execve(argv[0],argv,NULL)) > 0){
				thr_wait(thid);
				grab_console();
			} else {
				printf("bltsh: failed to execve(): %s\n", argv[0]);
			}
		} else {
			printf("bltsh: no such file or directory: %s\n", argv[0]);
		}
	}	
}

int main (void)
{
	char line[256], *c;
	int len, space, i, p_argc;

	__libc_init_console_input ();
	printf ("\n");

	for (;;)
	{
		printf ("$ ");

		*line = len = 0;
		while (read (0, line + len++, 1) > 0)
		{
			if ((line[len - 1] == 8) && (len > 1)) /* BS */
				len -= 2;
			else if (line[len - 1] == '\n')
			{
				line[len-- - 1] = 0;
				for (i = space = 0, p_argc = 2; i < len; i++)
					if ((line[i] == ' ') && !space)
						space = 1;
					else if ((line[i] != ' ') && space)
					{
						p_argc++;
						space = 0;
					}
				if ((*line != '#') && *line)
				{
					params = malloc (sizeof (char *) * p_argc);
					c = line;
					for (i = 0; i < p_argc - 1; i++)
					{
						for (len = 0; c[len] && (c[len] != ' '); len++) ;
						params[i] = malloc (len + 1);
						strlcpy (params[i], c, len + 1);
						c += len + 1;
					}
					params[p_argc - 1] = NULL;
					
					do_command(p_argc, params);
					
					for(i=0;i<p_argc;i++)
						free(params[i]);
					free(params);
				}
				len = 0;
				break;
			}
		}
	}

	return 0;
}

