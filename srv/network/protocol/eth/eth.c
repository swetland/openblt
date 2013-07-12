/* $Id: //depot/blt/srv/network/protocol/eth/eth.c#1 $
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
e* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
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
#include <dlfcn.h>
#include <blt/qsem.h>
#include <blt/network/eth.h>
#include <blt/network/mbuf.h>
#include <blt/network/module.h>

eth_drv_t *drvlist = NULL;
eth_dev_t *devlist = NULL;
eth_prot_t *protlist = NULL;

static qsem_t *eth_input_lock_sem, *eth_input_length_sem;
static qsem_t *eth_output_lock_sem, *eth_output_length_sem;
static struct mbuf *eth_input_queue = NULL, *eth_output_queue = NULL;
static void eth_input_thread (void);
static void eth_output_thread (void);

int _init (void)
{
	eth_input_lock_sem = qsem_create (1);
	eth_input_length_sem = qsem_create (0);
	thr_create (eth_input_thread, 0, "network:eth:input");
	eth_output_lock_sem = qsem_create (1);
	eth_output_length_sem = qsem_create (0);
	thr_create (eth_output_thread, 0, "network:eth:output");
	return 1;
}

void _fini (void)
{
}

void register_eth_driver (eth_drv_t *drv)
{
	drv->next = drvlist;
	drvlist = drv;
}

void unregister_eth_driver (eth_drv_t *drv)
{
}

void register_eth_device (eth_dev_t *dev)
{
	dev->next = devlist;
	devlist = dev;
}

void unregister_eth_device (eth_dev_t *dev)
{
}

void register_eth_protocol (eth_prot_t *prot)
{
	prot->next = protlist;
	protlist = prot;
}

void unregister_eth_protocol (eth_prot_t *prot)
{
}

void eth_input (struct mbuf *mbuf)
{
	struct mbuf *m;

	qsem_acquire (eth_input_lock_sem);
	if (eth_input_queue == NULL)
	{
		eth_input_queue = mbuf;
		mbuf->m_nextpkt = NULL;
	}
	else
	{
		m = eth_input_queue;
		while (m->m_nextpkt != NULL)
			m = m->m_nextpkt;
		m->m_nextpkt = mbuf;
	}
	qsem_release (eth_input_lock_sem);
	qsem_release (eth_input_length_sem);
}

static void eth_input_thread (void)
{
	struct mbuf *mbuf, *mbuf_header;
	eth_prot_t *prot;
	ether_header *header;

	for (;;)
	{
		qsem_acquire (eth_input_length_sem);
		qsem_acquire (eth_input_lock_sem);
		mbuf_header = eth_input_queue;
		eth_input_queue = eth_input_queue->m_nextpkt;
		qsem_release (eth_input_lock_sem);

		mbuf = mbuf_header->m_next;
		header = (ether_header *) mbuf->m_data;
#if 0
		printf ("eth_input_thread prot: %x src: %X:%X:%X:%X:%X:%X  dst: "
			"%X:%X:%X:%X:%X:%X\n", header->frame_type, header->src[0],
			header->src[1], header->src[2], header->src[3], header->src[4],
			header->src[5], header->dst[0], header->dst[1], header->dst[2],
			header->dst[3], header->dst[4], header->dst[5]);
#endif
		prot = protlist;
		while (prot != NULL)
			if (prot->num == header->frame_type)
			{
				prot->input (mbuf_header);
				break;
			}
			else
				prot = prot->next;
	}
}

void eth_output (struct mbuf *mbuf)
{
	struct mbuf *m;

	qsem_acquire (eth_output_lock_sem);
	if (eth_output_queue == NULL)
	{
		eth_output_queue = mbuf;
		mbuf->m_nextpkt = NULL;
	}
	else
	{
		m = eth_output_queue;
		while (m->m_nextpkt != NULL)
			m = m->m_nextpkt;
		m->m_nextpkt = mbuf;
	}
	qsem_release (eth_output_lock_sem);
	qsem_release (eth_output_length_sem);
}

static void eth_output_thread (void)
{
	struct mbuf *mbuf;

	for (;;)
	{
		qsem_acquire (eth_output_length_sem);
		qsem_acquire (eth_output_lock_sem);
		mbuf = eth_output_queue;
		eth_output_queue = eth_input_queue->m_nextpkt;
		qsem_release (eth_output_lock_sem);
		printf ("eth_output_thread\n");
	}
}

void eth_config (eth_dev_t *dev, const char *prot, int state, const char *data)
{
	char *devname;
	int len;
	int (*config_up)(const char *, void (*)(struct mbuf *), const char *);
	module_t *mod;

	mod = modlist;
	while (mod != NULL)
		if (!strcmp (mod->name, prot))
		{
			printf ("network: %s%d: interface %s, ", dev->dev_driver->name,
				dev->dev_num, state ? "up" : "down");
			devname = malloc (len = 16);
			snprintf (devname, len, "%s%d", dev->dev_driver->name,
				dev->dev_num);
			config_up = dlsym (mod->handle, "ipv4_config_up"); /* XXX */
			if (config_up != NULL)
				(*config_up) (devname, eth_output, data);
			return;
		}
		else
			mod = mod->next;
	printf ("network: %s%d: unknown protocol %s\n", dev->dev_driver->name,
		dev->dev_num, prot);
}

