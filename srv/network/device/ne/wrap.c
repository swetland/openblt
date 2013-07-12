/* $Id: //depot/blt/srv/network/device/ne/wrap.c#1 $
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
 * limitations:  can only support one card for now.
 */

#include <stdio.h>
#include <stdlib.h>
#include <blt/qsem.h>
#include <blt/network/eth.h>
#include <blt/network/mbuf.h>
#include "ne2000.h"
#include "wrap.h"

#define NE_IRQ 3

void probe (void);
static void isr (void);
static void receive (void *cb, packet_data *data);
static void send (int num, struct mbuf *data);

static eth_drv_t driver = { "ne", send, NULL };

static int maxdev = 0;
static int __irq;
static qsem_t *__mutex;
static ne_card_t *__card;
static eth_dev_t *__dev;

int _init (void)
{
	register_eth_driver (&driver);
	return 0;
}

void _fini (void)
{
}

void probe (void)
{
	int iobase;
	ne_card_t *card;
	eth_dev_t *dev;

	if (iobase = nic_detect (0))
	{
		card = malloc (sizeof (ne_card_t));
		card->nic.iobase = 0;
		card->irq = NE_IRQ;
		card->mutex = qsem_create (1);
		nic_init (&card->nic, iobase, card->prom, NULL);
		printf ("network: ne0 at port 0x%x, irq %d, mac = %X:%X:%X:%X:%X:%X\n",
			card->nic.iobase, card->irq, card->prom[0], card->prom[1],
			card->prom[2], card->prom[3], card->prom[4], card->prom[5]);
		dev = malloc (sizeof (eth_dev_t));
		dev->dev_driver = &driver;
		dev->dev_num = maxdev++;
		memcpy (dev->dev_addr, card->prom, sizeof (dev->dev_addr));
		dev->dev_data = card;

		register_eth_device (dev);
		nic_register_notify (&card->nic, receive, NULL);
		nic_start (&card->nic, 0);

		__card = card;
		__irq = card->irq;
		__mutex = card->mutex;
		__dev = dev;

		thr_create (isr, 0, "network:ne:isr");
	}
	else
		printf ("network: ne: probe failed.\n");
}

int config (int num, const char *prot, int state, const char *data)
{
	eth_config (__dev, prot, state, data);
	return 0;
}

static void isr (void)
{
	os_handle_irq (__irq);
	for (;;)
	{
		os_sleep_irq ();
		qsem_acquire (__mutex);
		nic_isr (&__card->nic);
		qsem_release (__mutex);
	}
}

static void receive (void *cb, packet_data *data)
{
	struct mbuf *mbuf, *header;

	mbuf = mget ();
	mbuf->m_type = MT_DATA;
	mbuf->m_len = data->len;
	if (data->len < MLEN)
	{
		mbuf->m_data = mbuf->m_databuf;
		memcpy (mbuf->m_data, data->ptr, data->len);
	}
	free_buffer_data (data);

	header = mget ();
	header->m_type = MT_IFADDR;
	header->m_len = 3;
	*((eth_dev_t **) header->m_data) = __dev;
	header->m_next = mbuf;

	eth_input (header);
}

struct _pbuf {
    packet_buffer pb;    /* 16 */
    packet_data pd;      /* 8 */
    int n;
} pbuf;

static void send (int num, struct mbuf *data)
{
	int res;
	packet_buffer *pb;

	printf ("send! %d\n", data->m_next->m_len);
#if 0
	pb = malloc (sizeof (packet_buffer));
	pb->count = 1;
	pb->len = data->m_next->m_len;
	pb->buf = alloc_buffer_data (pb->len);
	pb->buf->len = data->m_next->m_len;
	memcpy (pb->buf->ptr, data->m_next->m_data, pb->len);
	qsem_acquire (__mutex);
	res = nic_send_packet (&__card->nic, pb);
	qsem_release (__mutex);
#else
	pbuf.pb.count = 1;
	pbuf.pb.buf = &pbuf.pd;
	pbuf.pd.ptr = malloc (1000);
	memcpy (pbuf.pd.ptr, data->m_next->m_data, data->m_next->m_len);
	pbuf.pb.len = pbuf.pd.len = data->m_next->m_len + 100;
	qsem_acquire (__mutex);
	res = nic_send_packet (&__card->nic, &pbuf.pb);
	qsem_release (__mutex);
#endif
	printf ("sent %d\n", res);
	mput (data->m_next);
	mput (data);
}

