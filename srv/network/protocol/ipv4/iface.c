/* $Id: //depot/blt/srv/network/protocol/ipv4/iface.c#1 $
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

#include <stdlib.h>
#include <blt/network/ipv4.h>

ipv4_iface_t *iflist = NULL;

int _init (void)
{
	return 1;
}

int ipv4_config_up (const char *name, void (*output)(struct mbuf *),
	const char *data)
{
	char c[] = { 0, 0 }, temp[16];
	unsigned char num[8];
	int i, j, k, len, space;
	ipv4_iface_t *iface;

	for (i = j = k = space = *temp = 0; i < strlen (data); i++)
		if (data[i] == '.')
		{
			num[k++] = atoi (temp);
			j++;
			*temp = 0;
		}
		else if (data[i] == ' ')
		{
			num[k++] = atoi (temp);
			j++;
			*temp = 0;
			space = 1;
		}
		else
		{
			*c = data[j++];
			strlcat (temp, c, sizeof (temp));
		}
	iface = malloc (sizeof (ipv4_iface_t));
	iface->name = name;
	iface->addr[0] = num[0]; iface->addr[1] = num[1];
	iface->addr[2] = num[2]; iface->addr[3] = num[3];
	iface->netmask[0] = num[4]; iface->netmask[1] = num[5];
	iface->netmask[2] = num[6]; iface->netmask[3] = num[7];
	iface->output = output;
	iface->next = iflist;
	iflist = iface;
	printf ("address %d.%d.%d.%d netmask %d.%d.%d.%d\n",
		iface->addr[0], iface->addr[1], iface->addr[2], iface->addr[3],
		iface->netmask[0], iface->netmask[1], iface->netmask[2],
		iface->netmask[3]);
}

int ipv4_config_down (void)
{
}

