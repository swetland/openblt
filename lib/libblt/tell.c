/* Copyright 1999, Sidney Cammeresi. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <blt/syscall.h>
#include <blt/libsyms.h>
#include <blt/namer.h>
#include <blt/tell.h>

static char *name;
static volatile int ready = 0;
static void (*callback)(const char *);

weak_alias (_tell_init, tell_init)

void __tell_impl (void)
{
	char *buf, junk;
	int port, len;
	msg_hdr_t mh;

	port = port_create (0, "tell_listen_port");
	buf = malloc (len = TELL_MAX_LEN);
	strlcpy (buf, name, len);
	strlcat (buf, ":tell", len);
	namer_register (port, buf);
	ready = 1;

	for (;;)
	{
		mh.src = 0;
		mh.dst = port;
		mh.data = buf;
		mh.size = len;
		old_port_recv (&mh);
		(*callback) (buf);
		mh.dst = mh.src;
		mh.src = port;
		mh.data = &junk;
		mh.size = 1;
		old_port_send (&mh);
	}
}

int _tell_init (char *n, void (*c)(const char *))
{
	name = n;
	callback = c;
	os_thread (__tell_impl);
	while (!ready) ;
	return ready;
}

