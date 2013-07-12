/* $Id: //depot/blt/srv/network/mbuf.c#1 $
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

#include <unistd.h>
#include <blt/qsem.h>
#include <blt/network/mbuf.h>

#define MBUF_INIT_MEM     0x1000  /* start with one page of mbufs */
#define MCL_INIT_MEM      0x4000  /* start with eight clusters */

static struct mbuf *free_mbuf = NULL;
static unsigned int **free_cluster = NULL;

static qsem_t *free_mbuf_sem, *free_cluster_sem;

void mbinit (void)
{
	unsigned int i, **cl;
	struct mbuf *m;
	
	m = sbrk (MBUF_INIT_MEM);
	for (i = 0; i < MBUF_INIT_MEM / sizeof (struct mbuf); i++)
	{
		m[i].m_next = free_mbuf;
		m[i].m_nextpkt = NULL;
		m[i].m_len = MLEN;
		m[i].m_data = (char *) &m[i] + sizeof (struct mbuf);
		m[i].m_type = MT_FREE;
		m[i].m_flags = 0;
		free_mbuf = &m[i];
	}

	cl = sbrk (MCL_INIT_MEM);
	for (i = 0; i < MCL_INIT_MEM / 0x800; i++, cl += 0x800 / sizeof (int))
	{
		*cl = (unsigned int *) free_cluster;
		free_cluster = cl;
	}

	free_mbuf_sem = qsem_create (1);
	free_cluster_sem = qsem_create (1);
}

struct mbuf *mget (void)
{
	struct mbuf *mbuf;

	qsem_acquire (free_mbuf_sem);
	if ((mbuf = free_mbuf) == NULL)
	{
		/* allocate more memory */
	}
	else
		free_mbuf = free_mbuf->m_next;
	qsem_release (free_mbuf_sem);
	mbuf->m_next = NULL;
	return mbuf;
}

char *mgetcl (struct mbuf *mbuf)
{
}

void mput (struct mbuf *mbuf)
{
	mbuf->m_nextpkt = NULL;
	mbuf->m_len = MLEN;
	mbuf->m_data = (char *) mbuf + sizeof (struct mbuf);
	mbuf->m_type = MT_FREE;
	mbuf->m_flags = 0;
	mputcl (mbuf);
	qsem_acquire (free_mbuf_sem);
	mbuf->m_next = free_mbuf;
	free_mbuf = mbuf->m_next;
	qsem_release (free_mbuf_sem);
}

void mputcl (struct mbuf *mbuf)
{
}

