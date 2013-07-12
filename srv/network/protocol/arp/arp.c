/* $Id: //depot/blt/srv/network/protocol/arp/arp.c#1 $
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

#include <blt/network/mbuf.h>
#include <blt/network/eth.h>
#include <blt/network/ipv4.h>

void arp_input (struct mbuf *mbuf);
static eth_prot_t protocol = { "arp", 0, arp_input, NULL };

int _init (void)
{
	protocol.num = htons (ETH_PROT_ARP);
	register_eth_protocol (&protocol);
	return 0;
}

void arp_print_addrs (arp_packet *arp)
{
	printf ("%X:%X:%X:%X:%X:%X %X:%X:%X:%X:%X:%X\n", arp->header.src[0],
	arp->header.src[1], arp->header.src[2], arp->header.src[3],
	arp->header.src[4], arp->header.src[5], arp->header.dst[0],
	arp->header.dst[1], arp->header.dst[2], arp->header.dst[3],
	arp->header.dst[4], arp->header.dst[5]);
}

/*
 * this doesn't take too long, so we do it synchronously.
 */
void arp_input (struct mbuf *mbuf)
{
	arp_packet *arp_request, *arp_reply;
	ipv4_iface_t *iface;
	eth_dev_t *dev;
	struct mbuf *reply_header, *reply;

	arp_request = (arp_packet *) mbuf->m_next->m_data;
	iface = iflist;
	while (iface != NULL)
		if (!memcmp (iface->addr, arp_request->targ_ip_addr, 4))
		{
			printf ("ARP: who-has %d.%d.%d.%d tell %d.%d.%d.%d\n",
				arp_request->targ_ip_addr[0], arp_request->targ_ip_addr[1],
				arp_request->targ_ip_addr[2], arp_request->targ_ip_addr[3],
				arp_request->send_ip_addr[0], arp_request->send_ip_addr[1],
				arp_request->send_ip_addr[2], arp_request->send_ip_addr[3]);

			dev = *((eth_dev_t **) mbuf->m_data);
			reply_header = mget ();
			reply_header->m_next = reply = mget ();
			arp_reply = (arp_packet *) (reply->m_data = reply->m_databuf);
			memcpy (arp_reply->header.dst, arp_request->header.src, 6);
			memcpy (arp_reply->header.src, dev->dev_addr, 6);
			arp_reply->header.frame_type = htons (ETH_PROT_ARP);
			arp_reply->hard_type = htons (1);
			arp_reply->prot_type = htons (ETH_PROT_IP);
			arp_reply->hard_size = 6;
			arp_reply->prot_size = 4;
			arp_reply->op = htons (ETH_ARP_OP_REPLY);
			memcpy (arp_reply->send_eth_addr, dev->dev_addr, 6);
			memcpy (arp_reply->send_ip_addr, iface->addr, 4);
			memcpy (arp_reply->targ_eth_addr, arp_request->header.src, 6);
			memcpy (arp_reply->targ_ip_addr, arp_request->send_ip_addr, 4);

			printf ("ARP: reply %d.%d.%d.%d is-at %X:%X:%X:%X:%X:%X\n",
				arp_reply->send_ip_addr[0], arp_reply->send_ip_addr[1],
				arp_reply->send_ip_addr[2], arp_reply->send_ip_addr[3],
				arp_reply->send_eth_addr[0], arp_reply->send_eth_addr[1],
				arp_reply->send_eth_addr[2], arp_reply->send_eth_addr[3],
				arp_reply->send_eth_addr[4], arp_reply->send_eth_addr[5]);

			arp_print_addrs (arp_request);
			arp_print_addrs (arp_reply);
			reply->m_len = sizeof (arp_packet);
			dev->dev_driver->output (dev->dev_num, reply_header);
			mput (mbuf->m_next);
			mput (mbuf);
			return;
		}
		else
			iface = iface->next;
}

