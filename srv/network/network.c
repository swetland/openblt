/* $Id: //depot/blt/srv/network/network.c#5 $
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
#include <dlfcn.h>
#include <blt/syscall.h>
#include <blt/namer.h>
#include <blt/tell.h>
#include <blt/os.h>
#include <blt/network/module.h>
#include <blt/network/mbuf.h>

static int ctl_port, tell_port;
module_t *modlist = NULL;

void network_cmd (const char *cmd, const char *arg)
{
	char c[] = { 0, 0 }, *name, *num, *prot;
	int i, len, ifnum, state;
	void *handle;
	void (*probe)(void);
	void (*config)(int, const char *, int, const char *);
	module_t *m;

	if (!strcmp (cmd, "load"))
	{
		printf (" %s", arg);
		name = malloc (len = BLT_MAX_NAME_LENGTH);
		strlcpy (name, "/boot/", len);
		strlcat (name, arg, len);
		strlcat (name, ".so", len);
		handle = dlopen (name, 0);
		if (handle == NULL)
		{
			printf ("(error)", arg);
			return;
		}
		free (name);
		m = malloc (sizeof (module_t));
		m->name = malloc (strlen (arg) + 1);
		strcpy (m->name, arg);
		m->handle = handle;
		m->next = modlist;
		modlist = m;
	}
	else if (!strcmp (cmd, "probe"))
	{
		m = modlist;
		while (m != NULL)
		{
			if (!strcmp (m->name, arg))
			{
				probe = dlsym (m->handle, "probe");
				if (probe != NULL)
				{
					(*probe) ();
					return;
				}
			}
			else
				m = m->next;
		}
	}
	else if (!strcmp (cmd, "config"))
	{
		name = malloc (len = BLT_MAX_NAME_LENGTH);
		for (i = 0; (i < len) && arg[i]; i++)
			if ((arg[i] >= '0') && (arg[i] <= '9'))
			{
				name[i] = 0;
				break;
			}
			else
				name[i] = arg[i];

		num = malloc (BLT_MAX_NAME_LENGTH);
		*num = 0;
		i++;
		while ((arg[i] != ' ') && arg[i])
		{
			*c = arg[i];
			strlcat (num, c, len);
		}
		ifnum = atoi (num);

		prot = malloc (BLT_MAX_NAME_LENGTH);
		*prot = 0;
		i++;
		while ((arg[i] != ' ') && arg[i])
		{
			*c = arg[i++];
			strlcat (prot, c, len);
		}

		*num = 0;
		i++;
		while ((arg[i] != ' ') && arg[i])
		{
			*c = arg[i++];
			strlcat (num, c, len);
		}
		if (!strcmp (num, "up"))
			state = 1;
		else if (!strcmp (num, "down"))
			state = 0;
		else
			;

		m = modlist;
		while (m != NULL)
		{
			if (!strcmp (m->name, name))
			{
				config = dlsym (m->handle, "config");
				if (config != NULL)
				{
					(*config) (ifnum, prot, state, arg + i + 1);
					return;
				}
			}
			else
				m = m->next;
		}
		free (name);
		free (num);
		free (prot);
	}
}

void network_tell (const char *msg)
{
	const char *c, *cmd;
	char *full;
	char *d;
	int i, whole, len;

	c = msg;
	cmd = NULL;
	whole = 0;
	while (*c)
	{
		i = 0;
		while ((c[i] != ' ') && c[i])
			i++;
		d = malloc (i + 1);
		strlcpy (d, c, i + 1);
		if (cmd == NULL)
		{
			cmd = d;
			if (!strcmp (cmd, "load"))
				printf ("network: loading drivers.  [");
			else if (!strcmp (cmd, "config"))
			{
				whole = 1;
				full = malloc (len = strlen (msg + 1));
				*full = 0;
			}
		}
		else if (!whole)
			network_cmd (cmd, d);
		else
		{
			strlcat (full, d, len);
			strlcat (full, " ", len);
		}
		if (i != strlen (c))
			c += i + 1;
		else
			break;
	}
	if (!strcmp (cmd, "load"))
		printf (" ]\n");
	else if (!strcmp (cmd, "config"))
	{
		network_cmd (cmd, full);
		free (full);
	}
}

int network_main (volatile int *ready)
{
	char *buffer;
	int len;
	msg_hdr_t mh;

	ctl_port = port_create (0, "network_ctl_port");
	tell_port = port_create (0, "network_tell_port");
	port_slave (ctl_port, tell_port);
	namer_register (ctl_port, "network");
	namer_register (tell_port, "network:tell");

	printf ("network: ready.\n");
	buffer = malloc (len = 256);
	mbinit ();
	*ready = 1;

	for (;;)
	{
		mh.src = 0;
		mh.dst = ctl_port;
		mh.data = buffer;
		mh.size = len;
		old_port_recv (&mh);

		if (mh.dst == tell_port)
		{
			network_tell (buffer);
			mh.dst = mh.src;
			mh.src = tell_port;
			mh.data = &tell_port;
			mh.size = 1;
			old_port_send (&mh);
		}
		else if (mh.dst == ctl_port)
		{
		}
	}
		
	return 0;
}

int main (int argc, char **argv)
{
	volatile int ready = 0;

	thr_create (network_main, (int *) &ready, "network");
	while (!ready) ;
	return 0;
}

