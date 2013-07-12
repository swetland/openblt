/* Copyright 1999, Sidney Cammeresi. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <blt/syscall.h>
#include <blt/namer.h>

int main (int argc, char **argv)
{
	char *text;
	int i, loc_port, rem_port;
	msg_hdr_t mh;

	if (argc < 3)
	{
		printf ("tell: syntax: tell [server] [message]\n");
		return 0;
	}
	text = malloc (256);
	strlcpy (text, argv[1], 256);
	strlcat (text, ":tell", 256);

	rem_port = namer_find (text, 0);
	if (rem_port < 1)
	{
		printf ("tell: no such server or server not using tell\n");
		return 0;
	}
	loc_port = port_create (rem_port,"port");
	strlcpy (text, argv[2], 256);
	for (i = 3; i < argc; i++)
	{
		strlcat (text, " ", 256);
		strlcat (text, argv[i], 256);
	}

	mh.src = loc_port;
	mh.dst = rem_port;
	mh.data = text;
	mh.size = strlen (text) + 1;
	old_port_send (&mh);
	mh.src = rem_port;
	mh.dst = loc_port;
	old_port_recv (&mh);
	return 0;
}

