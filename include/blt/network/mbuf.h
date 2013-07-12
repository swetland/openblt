/* $Id: //depot/blt/include/blt/network/mbuf.h#1 $
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
 * Copyright (c) 1982, 1986, 1988, 1993
 *     The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the University of
 *        California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  @(#)mbuf.h  8.5 (Berkeley) 2/19/95
 */

#ifndef _BLT_NETWORK_MBUF_H_
#define _BLT_NETWORK_MBUF_H_

#define MSIZE       128
#define MCLBYTES    2048

#define MLEN        (MSIZE - sizeof(struct m_hdr))    /* normal data len */
#define MHLEN       (MLEN - sizeof(struct m_pkthdr))  /* data len w/pkthdr */
#define MINCLSIZE   (MHLEN+MLEN+1)  /* smallest amount to put in cluster */
#define M_MAXCOMPRESS   (MHLEN / 2) /* max amount to copy for compression */

struct m_hdr
{
	struct mbuf *mh_next, *mh_nextpkt;
	int mh_len;
	char *mh_data;
	short mh_type, mh_flags;
};

struct m_pkthdr
{
	void *rcvif;
	int len;
};

struct m_ext
{
	char *ext_buf;
	void (*ext_free) (char *, int);
	int ext_size;
	volatile int ext_refcnt;
};

struct mbuf
{
	struct m_hdr m_hdr;
	union
	{
		struct
		{
			struct m_pkthdr mh_pkthdr;
			union
			{
				struct m_ext *mh_ext;
				char mh_ext_databuf[MHLEN];
			} mh_dat;
		} mh;
		char mh_databuf[MLEN];
	} m_dat;
};

#define m_next         m_hdr.mh_next
#define m_nextpkt      m_hdr.mh_nextpkt
#define m_len          m_hdr.mh_len
#define m_data         m_hdr.mh_data
#define m_type         m_hdr.mh_type
#define m_flags        m_hdr.mh_flags
#define m_databuf      m_dat.mh_databuf
#define m_ext_databuf  m_dat.mh.mh_dat.mh_ext_databuf

#define MT_FREE        1
#define MT_DATA        2
#define MT_HEADER      3
#define MT_IFADDR      4

#ifdef _NETWORK

void mbinit (void);
struct mbuf *mget (void);
char *mgetcl (struct mbuf *mbuf);
void mput (struct mbuf *mbuf);
void mputcl (struct mbuf *mbuf);

#endif

#endif

