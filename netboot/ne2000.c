/* $Id: //depot/blt/netboot/ne2000.c#3 $
**
** Copyright 1998 Brian J. Swetland
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
/* This file contains the shared code base for the 1997 SigOps NE2000
// network driver.  Use at your own risk!    (C) Copyright 1997,
// Douglas Armstrong	site@xnet.com	drarmstr@uiuc.edu	10/11/97
// ... 
// derived from National Semiconductor datasheets and application notes
// as well as the Linux driver by Donald Becker */

/* Change Log:
10-25-97	Setup under CVS					drarmstr
10-27-97	Added alloc_buffer() and free_buffer()		drarmstr
*/

#ifndef NULL
#define NULL	0
#endif	/* NULL */
#include "io.h"
#include "ne2000.h"
#include "ne2k.h"
#include "err.h"

extern void idle();
extern long ticks;	/* replace with a better timeout method, like wait() */

/* we may have to change inb and outb to inb_p and outb_p.
// Note, none of this code is gauranteed to be reentrant.

// Note, don't create to nics to the same card and then try to use both 
// of them.  The results will be unpredictable.

// This violates IO resource manegment if your OS handles that, but it 
// works for now.  Make sure that the driver has privlige access to the 
// mapped I/O space and the IRQ.*/
static unsigned int default_ports[] = { 0x300, 0x280, 0x320, 0x340, 0x360, 0 };

/* internal procedure, don't call this directly.*/
int nic_probe(int addr) {
	uint regd;
	uint state=inb(addr);		/* save command state */

	if(inb(addr==0xff)) return ERRNOTFOUND;

	outb(NIC_DMA_DISABLE | NIC_PAGE1 | NIC_STOP, addr);
	regd=inb(addr + 0x0d);
	outb(0xff, addr + 0x0d);
	outb(NIC_DMA_DISABLE | NIC_PAGE0, addr);
	inb(addr + FAE_TALLY);	/* reading CNTR0 resets it.*/
	if(inb(addr + FAE_TALLY)) {	/* counter didn't clear so probe fails*/
		outb(state,addr);	/* restore command state*/
		outb(regd,addr + 0x0d);
		return ERRNOTFOUND; }

	return addr;	/* network card detected at io addr;*/
}

/* Detects for presence of NE2000 card.  Will check given io addr that 
// is passed to it as well as the default_ports array.  Returns ERRNOTFOUND 
// if no card is found, i/o address otherwise.  This does conduct an ISA
// probe, so it's not always a good idea to run it.  If you already have 
// the information, then you can just use that with nic_init().*/
int nic_detect(int given) {
	int found;
	if((found=nic_probe(given))!=ERRNOTFOUND) { return found; }
	return ERRNOTFOUND;
}

/* This initializes the NE2000 card.  If it turns out the card is not
// really a NE2000 after all then it will return ERRNOTFOUND, else NOERR
// It also dumps the prom into buffer prom for the upper layers..
// Pass it a nic with a null iobase and it will initialize the structure for
// you, otherwise it will just reinitialize it. */
int nic_init(snic* nic, int addr, unsigned char *prom, unsigned char *manual) {
	uint f;
	if(!nic->iobase) {
		nic->iobase=addr;
		nic_stat_clear(&nic->stat);
		nic->pstart=0; nic->pstop=0; nic->wordlength=0; 
		nic->current_page=0;
		nic->notify=NULL;
		for(f=0;f<MAX_TX;f++) nic->tx_packet[f].len=0;
		nic->last_tx=NULL;
		nic->busy=0;
	} else {
		if(!nic->iobase || nic->iobase!=addr) return ERR;
	}	

	outb(inb(addr + NE_RESET), addr + NE_RESET);	/* reset the NE2000*/
	while(!(inb_p(addr+INTERRUPTSTATUS) & ISR_RST)) {
		/* TODO insert timeout code here.*/
	}

	outb_p(0xff,addr + INTERRUPTSTATUS);	/* clear all pending ints*/

	// Initialize registers
	outb_p(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_STOP, addr);	/* enter page 0*/
	outb_p(DCR_DEFAULT, addr + DATACONFIGURATION);
	outb_p(0x00, addr + REMOTEBYTECOUNT0);
	outb_p(0x00, addr + REMOTEBYTECOUNT1);
	outb_p(0x00, addr + INTERRUPTMASK);	/* mask off all irq's*/
	outb_p(0xff, addr + INTERRUPTSTATUS);	/* clear pending ints*/
	outb_p(RCR_MON, RECEIVECONFIGURATION);	/* enter monitor mode*/
	outb_p(TCR_INTERNAL_LOOPBACK, TRANSMITCONFIGURATION); /* internal loopback*/
	
	nic->wordlength=nic_dump_prom(nic,prom);
	if(prom[14]!=0x57 || prom[15]!=0x57) {
		return ERRNOTFOUND;
	}

	/* if the wordlength for the NE2000 card is 2 bytes, then
	// we have to setup the DP8390 chipset to be the same or 
	// else all hell will break loose.*/
	if(nic->wordlength==2) {
		outb_p(DCR_DEFAULT_WORD, addr + DATACONFIGURATION);
	}
	nic->pstart=(nic->wordlength==2) ? PSTARTW : PSTART;
	nic->pstop=(nic->wordlength==2) ? PSTOPW : PSTOP;

	outb_p(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_STOP, addr);
	outb_p(nic->pstart, addr + TRANSMITPAGE);	/* setup local buffer*/
	outb_p(nic->pstart + TXPAGES, addr + PAGESTART);
	outb_p(nic->pstop - 1, addr + BOUNDARY);
	outb_p(nic->pstop, addr + PAGESTOP);
	outb_p(0x00, addr + INTERRUPTMASK);	/* mask off all irq's*/
	outb_p(0xff, addr + INTERRUPTSTATUS);	/* clear pending ints*/
	nic->current_page=nic->pstart + TXPAGES;
	
	/* put physical address in the registers */
	outb_p(NIC_DMA_DISABLE | NIC_PAGE1 | NIC_STOP, addr);  /* switch to page 1 */
	if(manual) for(f=0;f<6;f++) outb_p(manual[f], addr + PHYSICAL + f);
	else for(f=0;f<LEN_ADDR;f++) outb_p(prom[f], addr + PHYSICAL + f);

	/* setup multicast filter to accept all packets*/
	for(f=0;f<8;f++) outb_p(0xFF, addr + MULTICAST + f);

	outb_p(nic->pstart+TXPAGES, addr + CURRENT);
	outb_p(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_STOP, addr); /* switch to page 0 */

	return NOERR;
}

/* This registers the function that will be called when a new packet 
// comes in. */
void nic_register_notify(snic *nic, void (*newnotify)(packet_buffer *newpacket)){
	nic->notify=newnotify;
}

/* start the NIC so it can actually recieve or transmit packets */
void nic_start(snic *nic, int promiscuous) {
	int iobase;
	if(!nic || !nic->iobase) {
		return; }
	iobase=nic->iobase;
	outb(0xff, iobase + INTERRUPTSTATUS);
	outb(IMR_DEFAULT, iobase + INTERRUPTMASK);
	outb(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_START, iobase);
	outb(TCR_DEFAULT, iobase + TRANSMITCONFIGURATION);
	if(promiscuous)
		outb(RCR_PRO | RCR_AM, iobase + RECEIVECONFIGURATION);
	else
		outb(RCR_DEFAULT, iobase + RECEIVECONFIGURATION);
}

/* stops the NIC */
void nic_stop(snic *nic) {
	unsigned char tmp_buffer[16];	
	if(!nic || !nic->iobase) return;    /* make sure card was initialized */
	nic_init(nic,nic->iobase,tmp_buffer,NULL);
}

void nic_isr(snic *nic) {
	uint isr;	/* Illinois Sreet Residence Hall */
	uint overload;
	if(!nic || !nic->iobase) return;    /* make sure card was initialized */
	outb_p(NIC_DMA_DISABLE | NIC_PAGE0, nic->iobase);
	overload=MAX_LOAD+1;
	while((isr=inb_p(nic->iobase+INTERRUPTSTATUS))) {
		if((--overload)<=0) break;
		if(isr & ISR_OVW)			nic_overrun(nic);
		else if(isr & (ISR_PRX | ISR_RXE))	nic_rx(nic);
		if(isr & ISR_PTX)			nic_tx(nic);
		else if(isr & ISR_TXE)			nic_tx_err(nic);
//		if(isr & ISR_CNT)			nic_counters(nic);
		if(isr & ISR_RDC) outb_p(ISR_RDC, nic->iobase+ INTERRUPTSTATUS);
		outb_p(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_START, nic->iobase);
	}
	if(isr) {
		outb_p(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_START, nic->iobase);
		if(!overload) {
				
			outb_p(ISR_ALL, nic->iobase + INTERRUPTSTATUS); // clear
		} else {
			outb_p(0xff, nic->iobase + INTERRUPTSTATUS);
								// Ack it anyway
		}
	}
}

/* You should call this before you just read the stats directlly from the
// snic struct.  This procedure updates the counters */
nic_stat nic_get_stats(snic *nic) { 
	nic->stat.errors.frame_alignment+=inb_p(nic->iobase + FAE_TALLY);
	nic->stat.errors.crc+=inb_p(nic->iobase + CRC_TALLY);
	nic->stat.errors.missed_packets+=inb_p(nic->iobase + MISS_PKT_TALLY);
	return nic->stat;
}

void nic_stat_clear(nic_stat *that) {
	that->rx_packets=0;
	that->tx_buffered=0;		that->tx_packets=0;
	that->errors.frame_alignment=0;	that->errors.crc=0;
	that->errors.missed_packets=0;	that->errors.rx=0;
	that->errors.rx_size=0;		that->errors.rx_dropped=0;
	that->errors.rx_fifo=0;		that->errors.rx_overruns=0;
	that->errors.tx_collisions=0;
}

/* Since this could be called by more than one other device, it should
// be properly protected for reentrancy.  We should put in a proper
// semaphore or something, look into this.
// NOTE: It is your responsibility to clean up the packet_buffer when you 
// are done calling this.  Also note that the buffer.ptr may change if
// you gave us a packet that was too small and we had to pad the data. 
// UPDATE: we now never change buffer.ptr, but create a whole new 
// packet_buffer instead.  We take care of deleting this, so you don't
// have to worry about it.
// This is done so we can avoid using dynamic memory allocation ourselves. */
int nic_send_packet(snic *nic, packet_buffer* buffer) {
	uint timeout;	uint f;	int iobase=nic->iobase;
	packet_buffer *pad=NULL;
	if(!buffer->len || !buffer->ptr) return ERR;	
	if(!nic || !nic->iobase) return ERR;
	if(buffer->len>MAX_LENGTH) return ERRFORMAT;
	/* the following wait for anyother tasks that are calling 
	// nic_send_packet() right now.  Note that this doesn't use
	// an atomic semaphore, so something MAY leak through. */
	timeout=ticks+10;	// wait 10 ticks
	while(nic->busy && ticks<=timeout) idle();
	/* Replace this with a proper timeout thing that doesn't use the
	// ticks method which will be diffrent on each OS. */
	if(nic->busy) {
		return ERRTIMEOUT;
	}
	nic->busy=1;	/* mark as busy, replace with semaphore */

/* XXX should not have to copy the data --- modify nic_block_output to
   optionally add padding 0's */
	if(buffer->len<MIN_LENGTH) {
/*		unsigned char *newbuffer=(unsigned char*)kalloc(MIN_LENGTH);	
		if(!newbuffer) return ERRNOMEM; */
		pad=alloc_buffer(MIN_LENGTH);
		if(!pad) {
			nic->busy=0;	/* V() */
			return ERRNOMEM; }
		for(f=0;f<buffer->len;f++) pad->ptr[f]=buffer->ptr[f];
		for(;f<MIN_LENGTH;f++) pad->ptr[f]='\0';
		/* pad the data to the minimum size. */
		buffer=pad;
	}

	outb_p(0x00, iobase + INTERRUPTMASK);	/* mask ints for now */
	for(f=0;f<MAX_TX;f++) {
		if(nic->tx_packet[f].len==0) {
			nic->tx_packet[f]=*buffer;
			nic->tx_packet[f].page=nic->pstart + (f * MAX_PAGESPERPACKET);
								/*output page */

			nic_block_output(nic,nic->tx_packet[f].ptr,
				nic->tx_packet[f].len,
				nic->tx_packet[f].page );

			nic->send=f;
			/* now let's actually trigger the transmitter to send */
			if(nic_send(nic,f)<0) break;
			/* note, the nic_tx() interrupt will mark this
			// tx_packet buffer as free again once it
			// confirms that the packet was sent. */

			nic->stat.tx_buffered++;
			outb_p(IMR_DEFAULT, iobase + INTERRUPTMASK); /* unmask */
			nic->busy=0;	/* V() */
			if(pad) free_buffer(pad);
			return NOERR;
		}
	}

	outb_p(IMR_DEFAULT, iobase + INTERRUPTMASK);	/* unmask */
	nic->busy=0;	/* V() */
	if(pad) free_buffer(pad);
	return ERR;
	/* since we passed with nic->busy not busy then we should
	// always have at least one buffer free. */
}

/* dumps the prom into a 16 byte buffer and returns the wordlength of
// the card.
// You should be able to make this procedure a wrapper of nic_block_input(). */
int nic_dump_prom(snic *nic, unsigned char *prom) {
	uint f;
	int iobase=nic->iobase;
	char wordlength=2;		/* default wordlength of 2 */
	unsigned char dump[32];
	outb_p(32, iobase + REMOTEBYTECOUNT0);	  /* read 32 bytes from DMA->IO */
	outb_p(0x00, iobase + REMOTEBYTECOUNT1);  /*  this is for the PROM dump */
	outb_p(0x00, iobase + REMOTESTARTADDRESS0); /* configure DMA for 0x0000 */
	outb_p(0x00, iobase + REMOTESTARTADDRESS1);
	outb_p(NIC_REM_READ | NIC_START, iobase);
	for(f=0;f<32;f+=2) {
		dump[f]=inb_p(iobase + NE_DATA);
		dump[f+1]=inb_p(iobase + NE_DATA);
		if(dump[f]!=dump[f+1]) wordlength=1;
	}
	/* if wordlength is 2 bytes, then collapse prom to 16 bytes */
	for(f=0;f<LEN_PROM;f++) prom[f]=dump[f+((wordlength==2)?f:0)];

	return wordlength;
}

void nic_overrun(snic *nic) {
	uint tx_status;	int iobase=nic->iobase;
	long starttime;	uint resend=0;
	if(!nic || !nic->iobase) return;
	tx_status=inb_p(iobase) & NIC_TRANSMIT;
	outb_p(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_STOP, iobase);
	nic->stat.errors.rx_overruns++;

	starttime=ticks;
/*kprintf("BEFORE\n");*/
	while(ticks-starttime<=10/*ticks to wait*/) idle();
	/* Arrgh!  TODO: Replace this whole crappy code with a decent
	// wait method.  We need to wait at least 1.6ms as per National
	// Semiconductor datasheets, but we should probablly wait a 
	// little more to be safe.
//kprintf("AFTER\n"); */

	outb_p(0x00, iobase + REMOTEBYTECOUNT0);
	outb_p(0x00, iobase + REMOTEBYTECOUNT1);
	if(tx_status) {
		uint tx_completed=inb_p(iobase + INTERRUPTSTATUS) & 
			(ISR_PTX | ISR_TXE);
		if(!tx_completed) resend=1;
	}

	outb_p(TCR_INTERNAL_LOOPBACK, iobase + TRANSMITCONFIGURATION);
	outb_p(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_START, iobase);
	nic_rx(nic);	/* cleanup RX ring */
	outb_p(ISR_OVW, iobase + INTERRUPTSTATUS);	/* ACK INT */

	outb_p(TCR_DEFAULT, iobase + TRANSMITCONFIGURATION);
	if(resend)
		outb_p(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_START | NIC_TRANSMIT,
			iobase);
}

/* This is the procedure that markst the transmit buffer as available again */
void nic_tx(snic *nic) {
	uint f;			int iobase=nic->iobase;
	uint status;
	if(!nic || !nic->iobase) return;
	status=inb(iobase + TRANSMITSTATUS);

	if(!nic->tx_packet[nic->send].len) {
		return;
	}
	nic->tx_packet[nic->send].len=0;	/* mark buffer as available */

	for(f=0;f<MAX_TX;f++) {
		if(nic->tx_packet[f].len) {
			nic->stat.tx_buffered++;
			nic_send(nic,f);	/* send a back-to-back buffer */
			break;
		}
	}

	if(status & TSR_COL) nic->stat.errors.tx_collisions++;
	if(status & TSR_PTX) nic->stat.tx_packets++;
	else {
		if(status & TSR_ABT) {
			nic->stat.errors.tx_aborts++;
			nic->stat.errors.tx_collisions+=16; }
		if(status & TSR_CRS) nic->stat.errors.tx_carrier++;
		if(status & TSR_FU) nic->stat.errors.tx_fifo++;
		if(status & TSR_CDH) nic->stat.errors.tx_heartbeat++;
		if(status & TSR_OWC) nic->stat.errors.tx_window++;
	}

	outb_p(ISR_PTX, iobase + INTERRUPTSTATUS);	/* ack int */
}

void nic_tx_err(snic *nic) {
	unsigned char tsr;	int iobase=nic->iobase;
	if(!nic || !nic->iobase) return;
	tsr=inb_p(nic->iobase);
/*	kprintf("NE2000: ERROR: TX error: ");
	if(tsr & TSR_ABT) kprintf("Too many collisions.\n");
	if(tsr & TSR_ND) kprintf("Not deffered.\n");
	if(tsr & TSR_CRS) kprintf("Carrier lost.\n");
	if(tsr & TSR_FU) kprintf("FIFO underrun.\n");
	if(tsr & TSR_CDH) kprintf("Heart attack!\n");
        */
	outb_p(ISR_TXE, iobase + INTERRUPTSTATUS);
	if(tsr & (TSR_ABT | TSR_FU)) {
		nic_tx(nic);
	}
}

void nic_rx(snic *nic) {
	uint packets=0;	uint frame;	uint rx_page;	uint rx_offset;
	uint len;	uint next_pkt;	uint numpages;
	int iobase=nic->iobase;
	buffer_header header;
	if(!nic || !nic->iobase) return;
	while(packets<MAX_RX) {
		outb_p(NIC_DMA_DISABLE | NIC_PAGE1, iobase); /*curr is on page 1 */
		rx_page=inb_p(iobase + CURRENT);	/* get current page */
		outb_p(NIC_DMA_DISABLE | NIC_PAGE0, iobase);
		frame=inb(iobase + BOUNDARY)+1;
			/* we add one becuase boundary is a page behind
			// as pre NS notes to help in overflow problems */
		if(frame>=nic->pstop) frame=nic->pstart+TXPAGES;
							/* circual buffer */

		if(frame==rx_page) break;	/* all frames read */

		rx_offset=frame << 8;  /* current ptr in bytes(not pages) */

		nic_get_header(nic,frame,&header);
		len=header.count - sizeof(buffer_header);
							/* length of packet */
		next_pkt=frame + 1 + ((len+4)>>8); /* next packet frame */

		numpages=nic->pstop-(nic->pstart+TXPAGES);
		if(	   (header.next!=next_pkt)
			&& (header.next!=next_pkt + 1)
			&& (header.next!=next_pkt - numpages)
			&& (header.next != next_pkt +1 - numpages)){
/*				kprintf("NE2000: ERROR: Index mismatch.   header.next:%X  next_pkt:%X frame:%X\n",
					header.next,next_pkt,frame);*/
				nic->current_page=frame;
				outb(nic->current_page-1, iobase + BOUNDARY);
				nic->stat.errors.rx++;
				continue;
		}

		if(len<60 || len>1518) {
/*			kprintf("NE2000: invalid packet size:%d\n",len);*/
			nic->stat.errors.rx_size++;
		} else if((header.status & 0x0f) == RSR_PRX) {
			/* We have a good packet, so let's recieve it! */

			packet_buffer *newpacket=alloc_buffer(len);
			if(!newpacket) {
/*				kprintf("NE2000: ERROR: out of memory!\n");*/
				nic->stat.errors.rx_dropped++;
				break;
			}

			nic_block_input(nic,newpacket->ptr,newpacket->len,
					rx_offset+sizeof(buffer_header));
								/* read it */

			if(nic->notify) nic->notify(newpacket);
			/* NOTE: you are responsible for deleting this buffer. */

			nic->stat.rx_packets++;

		} else {
/*			kprintf("NE2000: ERROR: bad packet.  header-> status:%X next:%X len:%x.\n",
				header.status,header.next,header.count); */
			if(header.status & RSR_FO) nic->stat.errors.rx_fifo++;
		}
/*		kprintf("frame:%x  header.next:%x  next_pkt:%x\n",
//			frame,header.next,next_pkt); */
		next_pkt=header.next;

		if(next_pkt >= nic->pstop) {
/*			kprintf("NE2000: ERROR: next frame beyond local buffer!  next:%x.\n",
				next_pkt);*/
			next_pkt=nic->pstart+TXPAGES;
		}

		nic->current_page=next_pkt;
		outb_p(next_pkt-1, iobase + BOUNDARY);
	}
	outb_p(ISR_PRX | ISR_RXE, iobase + INTERRUPTSTATUS);	/* ack int */
}

/* You should be able to make this procedure a wrapper of nic_block_input */
void nic_get_header(snic *nic, uint page, buffer_header *header) {
	int iobase=nic->iobase;	uint f;
	outb_p(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_START, iobase);
	outb_p(sizeof(buffer_header), iobase + REMOTEBYTECOUNT0);
	outb_p(0, iobase + REMOTEBYTECOUNT1);		/* read the header */
	outb_p(0, iobase + REMOTESTARTADDRESS0); 	/* page boundary */
	outb_p(page, iobase + REMOTESTARTADDRESS1); 	/* from this page */
	outb_p(NIC_REM_READ | NIC_START, iobase);	/* start reading */

	if(nic->wordlength==2) for(f=0;f<(sizeof(buffer_header)>>1);f++)
		((unsigned short *)header)[f]=inw(iobase+NE_DATA);
	else for(f=0;f<sizeof(buffer_header);f++)
		((unsigned char *)header)[f]=inb(iobase+NE_DATA);
					/* Do these need to be *_p variants??? */
	outb_p(ISR_RDC, iobase + INTERRUPTSTATUS);	/* finish DMA cycle */
}

int nic_send(snic *nic, uint buf) {
	outb_p(NIC_DMA_DISABLE |  NIC_PAGE0, nic->iobase);
        if(inb_p(nic->iobase + STATUS) & NIC_TRANSMIT) {
/*		kprintf("NE2000: ERROR: Transmitor busy.\n");*/
                nic->tx_packet[buf].len=0; /* mark as free again */
                return ERRTIMEOUT; }
        outb_p(nic->tx_packet[buf].len & 0xff,nic->iobase+TRANSMITBYTECOUNT0);
        outb_p(nic->tx_packet[buf].len >> 8,nic->iobase+TRANSMITBYTECOUNT1);
        outb_p(nic->tx_packet[buf].page,nic->iobase+TRANSMITPAGE);
        outb_p(NIC_DMA_DISABLE | NIC_TRANSMIT | NIC_START,nic->iobase);
	return NOERR;
}

void nic_block_input(snic *nic, unsigned char *buf, uint len, uint offset) {
	int iobase=nic->iobase;	uint f;
	uint xfers=len;
	uint timeout=TIMEOUT_DMAMATCH;	uint addr;
/*	kprintf("NE2000: RX: Length:%x  Offset:%x  ",len,offset);*/
	outb_p(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_START, iobase);
	outb_p(len & 0xff, iobase + REMOTEBYTECOUNT0);
	outb_p(len >> 8, iobase + REMOTEBYTECOUNT1);
	outb_p(offset & 0xff, iobase + REMOTESTARTADDRESS0);
	outb_p(offset >> 8, iobase + REMOTESTARTADDRESS1);
	outb_p(NIC_REM_READ | NIC_START, iobase);

	if(nic->wordlength==2) {
		for(f=0;f<(len>>1);f++)
			((unsigned short *)buf)[f]=inw(iobase+NE_DATA);
		if(len&0x01) {
			((unsigned char *)buf)[len-1]=inb(iobase+NE_DATA);
			xfers++;
		}
	} else for(f=0;f<len;f++)
		((unsigned char *)buf)[f]=inb(iobase+NE_DATA);
					/* Do these need to be *_p variants??? */

/*	for(f=0;f<15;f++) kprintf("%X",buf[f]); kprintf("\n");*/
	/* TODO: make this timeout a constant */
	for(f=0;f<timeout;f++) {
		uint high=inb_p(iobase + REMOTESTARTADDRESS1);
		uint low=inb_p(iobase + REMOTESTARTADDRESS0);
		addr=(high<<8)+low;
		if(((offset+xfers)&0xff)==low)	break;
	}
/*	if(f>=timeout)
		kprintf("NE2000: Remote DMA address invalid.  expected:%x - actual:%x\n",
			offset+xfers, addr);*/
	
	outb_p(ISR_RDC, iobase + INTERRUPTSTATUS);	/* finish DMA cycle */
}

void nic_block_output(snic *nic, unsigned char *buf, uint len, uint page) {
	int iobase=nic->iobase;
	int timeout=TIMEOUT_DMAMATCH;	int f;	uint addr;	int w;
	outb_p(NIC_DMA_DISABLE | NIC_PAGE0 | NIC_START, iobase);
	
	/* this next part is to supposedly fix a "read-before-write" bug... */
	outb_p(0x42, iobase + REMOTEBYTECOUNT0);
	outb_p(0x00, iobase + REMOTEBYTECOUNT1);
	outb_p(0x42, iobase + REMOTESTARTADDRESS0);
	outb_p(0x00, iobase + REMOTESTARTADDRESS1);
	outb_p(NIC_REM_READ | NIC_START, iobase);
	SLOW_DOWN_IO;	SLOW_DOWN_IO;	SLOW_DOWN_IO;

	outb_p(ISR_RDC, iobase + INTERRUPTSTATUS);	/* ack remote DMA int */
	
	outb_p(len & 0xff, iobase + REMOTEBYTECOUNT0);
	outb_p(len >> 8, iobase + REMOTEBYTECOUNT1);
	outb_p(0x00, iobase + REMOTESTARTADDRESS0);
	outb_p(page, iobase + REMOTESTARTADDRESS1);
	outb_p(NIC_REM_WRITE | NIC_START, iobase);

	if(nic->wordlength==2) {
		for(w=0;w<(len>>1);w++)
			outw(((unsigned short *)buf)[w],iobase+NE_DATA);
		if(len & 0x01) {
			short tmp=buf[len-1]; //so we can output a whole 16-bits
			// if buf were on the end of something, we would die.
			outw(tmp,iobase+NE_DATA);
		}
	} else for(f=0;f<len;f++)
		outb(((unsigned char *)buf)[f],iobase+NE_DATA);

	for(f=0;f<timeout;f++) {
		uint high=inb_p(iobase + REMOTESTARTADDRESS1);
		uint low=inb_p(iobase + REMOTESTARTADDRESS0);
		addr=(high<<8)+low;
		if(( (((page<<8)+(nic->wordlength==2 && (len&0x01))?len:len+1))
				&0xff)==low)
			break;
	}
/*	if(f>=timeout)
		kprintf("NE2000: Remote DMA address invalid.  expected:%x - actual:%x\n",
			page<<8, addr);*/
	
	outb_p(ISR_RDC, iobase + INTERRUPTSTATUS);	/* finish DMA cycle */
}
