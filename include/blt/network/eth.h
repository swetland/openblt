/* $Id: //depot/blt/include/blt/network/eth.h#1 $
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
 * XXX all multibyte quanities are in network byte order
 */

#ifndef _BLT_NETWORK_ETH_H_
#define _BLT_NETWORK_ETH_H_

#include <blt/types.h>

#pragma pack(2)

#define ETH_PROT_IP     0x0800
#define ETH_PROT_ARP    0x0806
#define ETH_PROT_RARP   0x8035

#define ETH_ARP_OP_REQUEST    1
#define ETH_ARP_OP_REPLY      2
#define ETH_RARP_OP_REQUEST   3
#define ETH_RARP_OP_REPLY     4

struct mbuf;

typedef struct __eth_drv_t
{
	const char *name;
	void (*output) (int num, struct mbuf *);
	struct __eth_drv_t *next;
} eth_drv_t;

typedef struct __eth_dev_t
{
	eth_drv_t *dev_driver;
	int dev_num;
	char dev_addr[6];
	void *dev_data;
	struct __eth_dev_t *next;
} eth_dev_t;

typedef struct
{
	uint8 dst[6];
	uint8 src[6];
	uint16 frame_type;
} ether_header;

typedef struct
{
	ether_header header;
	uint16 hard_type;
	uint16 prot_type;
	uint8 hard_size;
	uint8 prot_size;
	uint16 op;
	uint8 send_eth_addr[6];
	uint8 send_ip_addr[4];
	uint8 targ_eth_addr[6];
	uint8 targ_ip_addr[4];
	uint8 __pad__[18];
} arp_packet;

typedef struct __eth_prot_t
{
	const char *name;
	int num;
	void (*input) (struct mbuf *);
	struct __eth_prot_t *next;
} eth_prot_t;

char eth_broadcast_addr[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

#ifdef _NETWORK

extern eth_drv_t *drvlist;
extern eth_dev_t *devlist;
extern eth_prot_t *protlist;

void register_eth_driver (eth_drv_t *drv);
void unregister_eth_driver (eth_drv_t *drv);
void register_eth_device (eth_dev_t *dev);
void unregister_eth_device (eth_dev_t *dev);
void register_eth_protocol (eth_prot_t *prot);
void unregister_eth_protocol (eth_prot_t *prot);
void eth_input (struct mbuf *mbuf);

#endif

#endif

