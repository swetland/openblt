/* $Id: //depot/blt/srv/init/init.c#11 $
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

/*
 * Since nothing is running yet, we divide the system startup into two
 * phases:  starting bootstrap servers and normal initialisation.  The
 * former requires lots of strange incantations to parse the boot image
 * to find rc.boot and run everything it says to; basically we have to
 * reimplement what execve and the vfs do for us.  Once that stuff gets
 * going, we can open rc using the vfs and proceed in a more normal
 * fashion.
 *
 * As rc.boot contains fairly critical stuff, an error from something
 * in there will probably result in a wedged system.
 *
 * Microkernels are fun.
 *
 *         - sc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <boot.h>
#include <blt/syscall.h>

static char *copyright = "\
OpenBLT Release I (built " __DATE__ ", " __TIME__ ")
    Copyright (c) 1998-1999 The OpenBLT Dev Team.  All rights reserved.
\n";

void __libc_init_fdl (void), __libc_init_console (void),
	__libc_init_vfs (void);

int boot_get_num (boot_dir *dir, const char *name)
{
	int i;

	for (i = 0; i < BOOTDIR_MAX_ENTRIES; i++)
		if (!strcmp (dir->bd_entry[i].be_name, name))
			return i;
	return -1;
}

char *boot_get_data (boot_dir *dir, int num)
{
	return (char *) dir + dir->bd_entry[num].be_offset * 0x1000;
}

int main (void)
{
	int area, sarea;
	char *c, *rcboot, *line, *boot_servers, **params;
	int i, space, p_argc, boot, fd, len, total, prog, filenum;
	void *ptr;
	boot_dir *dir;

	
	line = malloc (256);
	boot_servers = malloc (256);

	if (!(boot = area_clone (3, 0, (void **) &dir, 0)))
	{
		os_console ("no uberarea; giving up");
		os_debug ();
		for (;;) ; /* fatal */
		return 0;
	}
	else if ((filenum = boot_get_num (dir, "rc.boot")) < 0)
	{
		os_console ("no /boot/rc.boot; do you know what you're doing?");
		os_debug ();
	}
	else
	{
        *line = *boot_servers = len = total = 0;
		rcboot = boot_get_data (dir, filenum);

        while (total < dir->bd_entry[filenum].be_vsize)
        {
			line[len++] = *rcboot++;
			total++;

            if (line[len - 1] == '\n')
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
                    params[i] = NULL;
                    if (!strcmp (params[0], "exit"))
                        os_terminate (1);

					prog = boot_get_num (dir, params[0]);
					area = area_create (dir->bd_entry[prog].be_vsize, 0,
						&ptr, 0);
					memcpy (ptr, boot_get_data (dir, prog),
						dir->bd_entry[prog].be_vsize);
					sarea = area_create (0x1000, 0, &ptr, 0);
					strlcat (boot_servers, " ", 256);
					strlcat (boot_servers, params[0], 256);
					thr_wait (thr_spawn (0x1074, 0x3ffffd, area, 0x1000,
						sarea, 0x3ff000, params[0]));
                }
                len = 0;
            }
        }
	}

	/* say hello */
	__libc_init_fdl ();
	__libc_init_console ();
	__libc_init_vfs ();
	printf (copyright);
	printf ("init: bootstrap servers started.  [ %s ]\n", boot_servers + 1);

	/* if we one day pass arguments to init, we will parse them here. */

	/* do some more normal stuff */
	printf ("init: beginning automatic boot.\n\n");
	fd = open ("/boot/rc", O_RDONLY, 0);
	if (fd < 0)
		printf ("error opening /boot/rc\n");
	else
	{
		*line = len = 0;
		while (read (fd, line + len++, 1) > 0)
		{
			if (line[len - 1] == '\n')
			{
/*
				line[len - 1] = 0;
				if ((*line != '#') && *line)
				{
					// printf ("execing `%s'\n", line);
					params = malloc (sizeof (char *) * 2);
					params[0] = malloc (strlen (line) + 1);
					strcpy (params[0], line);
					params[1] = NULL;
					thr_join (thr_detach (run2), 0);
				}
				len = 0;
*/
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
                    params[i] = NULL;
                    if (!strcmp (params[0], "exit"))
                        os_terminate (1);
						
					i = execve (params[0], params, NULL);
					if(i>0) thr_wait(i);
					else printf("cannot execute \"%s\"\n",params[0]);
                }
                len = 0;
			}
		}
		close (fd);
	}

	printf ("init: nothing left to do\n");
	return 0;
}

