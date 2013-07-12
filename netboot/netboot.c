/* $Id: //depot/blt/netboot/netboot.c#8 $
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
#include "string.h"

#include <blt/conio.h>

#include "net.h"
#include "boot.h"

#define NULL ((void *) 0)

#include "ne2k.h"
#include "io.h"

snic TheSNIC;

int nic_addr = 0x300;

char blah[1500]; /* inbound packet buffer */

char defaults[] = "ip=0.0.0.0";

char zz[] = "128.174.38.207";

char param[80];

extern char end[];

#define printf cprintf

int got_server = 0;

int id = 1;

int memsize = 0;

int reset = 0;

int got_ip = 0;
char *ipmsg = "";

unsigned char prom[32];

unsigned char ip0[8] = { 'i', 'p', '=', 0, 0, 0, 0, 0 };
unsigned char *ip = ip0 + 3;

unsigned char server_ip[4] = { 0, 0, 0, 0 };
unsigned char server_mac[6] = { 0, 0, 0, 0, 0, 0 };
unsigned short port = 0;

#define SKIP 0
#define LHS 1
#define RHS 2

void read_ip(char *s, unsigned char *ip)
{
    int i=4;
    int v;

    for(i=4;i>0;i--,ip++){
        v = 0;
        while(*s && *s != '.') {
            v = v*10 + *s-'0';
            s++;
        }
        *ip = v;
        if(!*s) break;
        s++;        
    }
}

void option(char *lhs, char *rhs)
{
    if(!strcmp(lhs,"ip")){
        read_ip(rhs,ip);
        ipmsg = "(manual)";
        return;        
    }
/*    if(!strcmp(lhs,"server")){
        read_ip(rhs,server_ip);        
        return;        
    }
    if(!strcmp(lhs,"gateway")){
        read_ip(rhs,gateway_ip);        
        return;        
    }
    if(!strcmp(lhs,"port")){
        port = 0;
        while(*rhs){
            port = port*10 + *rhs - '0';
            rhs++;            
        }
    }    */
    printf("netboot: option %s=\"%s\" unknown\n",lhs,rhs);    
}

void parse(char *p)
{
    int state = SKIP;
    char *lhs;    
    char *rhs;
    
    while(*p){
        switch(state){
        case SKIP :
            if(*p > ' '){
                lhs = p;
                state = LHS;
            } else {
                p++;
                break;                
            }
        case LHS :
            if(*p == '='){
                *p = 0;
                rhs = ++p;
                state = RHS;                
            } else {
                p++;                
            }
            break;
        case RHS :
            if(*p <= ' '){
                *p = 0;
                option(lhs,rhs);
                state = SKIP;
            }
            p++;                
            break;            
        }
    }
    if(state == RHS){
        option(lhs,rhs);
    }
}

#define ESC 27
#define BS 8
#define TAB 9
#define CR 13

char ScanTable [] =  {' ', ESC, '1', '2', '3', '4', '5', '6', '7', '8',
                      '9', '0', '-', '=', BS,  TAB, 'q', 'w', 'e', 'r',
                      't', 'y', 'u', 'i', 'o', 'p', '[', ']', CR,  ' ',
                      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
                      '\'', '~', ' ', '\\', 'z', 'x', 'c', 'v', 'b', 'n',
                      'm', ',', '.', '/', ' ', ' ', ' ', ' ', ' '};
char ShiftTable [] = {' ', ESC, '!', '@', '#', '$', '%', '^', '&', '*',
                      '(', ')', '_', '+', ' ', ' ', 'Q', 'W', 'E', 'R',
                      'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', CR,  ' ',
                      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
                      '\"', '~', ' ', '|', 'Z', 'X', 'C', 'V', 'B', 'N',
                      'M', '<', '>', '?', ' ', ' ', ' ', ' ', ' '};
#define LSHIFT 42
#define RSHIFT 54

char linebuf[80];
int lp;


void newline(void)
{
    memcpy(linebuf,"netboot> ",9);

    for(lp=9;lp<79;lp++) linebuf[lp]=' ';
    linebuf[79]=0;
    lp = 9;
    con_goto(0,24);
    con_puts(linebuf);
    
}

int command(char *cmd)
{
    if(cmd[0] == 'g' && cmd[1] == 'o' && cmd[2] == 0) return 1;

    if(cmd[0] == 'b' && cmd[1] == 'o' && cmd[2] == 'o' &&
       cmd[3] == 't' && cmd[4] == '='){
        char *p = param;
        cmd+=5;
        while(*cmd) {
            *p = *cmd;
            cmd++;
            p++;
        }
        *p = 0;
        return 0;
        
    }

    parse(cmd);
    
    return 0;
}

int keypress(int key)
{
    switch(key){
    case ESC:
        for(lp=9;lp<79;lp++) linebuf[lp]=' ';
        lp = 9;
        break;
    case CR:
        printf("\n");
        linebuf[lp]=0;
        if(command(linebuf+9)) return 1;
        newline();
        break;
    case BS:
        if(lp > 9){
            linebuf[lp-1]=' ';
            lp--;
        }
        break;
    default:
        if(lp < 79){
            linebuf[lp]=key;
            lp++;
        }
        break;
    }
    con_goto(0,24);
    con_puts(linebuf);
    return 0;
}

/* keyboard crap */
int console(void)
{
    int key;
    int shift = 0;
    newline();
    
    for(;;){
        while(inb(0x64) & 0x01) {
            key = inb(0x60);
            
            switch(key){
            case LSHIFT:
            case RSHIFT:
                shift = 1;
                break;
            case LSHIFT | 0x80:
            case RSHIFT | 0x80:
                shift = 0;
                break;
            default:
                if(key & 0x80){
                        /* break */
                } else {
                    if(key < 59){
                        key = shift ? ShiftTable[key] : ScanTable[key];
                        if(keypress(key)) return 1;
                    }
                }
            }
        }
    }
}



/* end keyboard crap */
packet_buffer *alloc_buffer(uint size)
{
    static packet_buffer pb;

    pb.len = size;
    pb.page = 0;
    pb.ptr = (unsigned char *) blah;    
    return &pb;
}

void free_buffer(packet_buffer *ptr)
{    
    int i;
    return;
}

int ticks = 0;

void idle(void)
{
    int i;
    for(i=0;i<10000;i++);
    ticks++;
}

int memcmp(const void *dst, const void *src, size_t size)
{
    while(size) {
        if(*(((unsigned char *) dst)++) != *(((unsigned char *) src)++))
            return 1;        
        size--;        
    }
    return 0;    
}

void *memcpy(void *dst, const void *src, size_t size)
{
    while(size) {
        *(((unsigned char *) dst)++) = *(((unsigned char *) src)++);
        size--;        
    }      
	return NULL; /* XXX */
}

typedef struct 
{
    net_ether ether;
    net_arp arp;    
} arp_packet;

typedef struct 
{
    net_ether ether;
    net_ip ip;    
} ip_packet;

typedef struct 
{
    net_ether ether;
    net_ip ip;
    net_udp udp;
} udp_packet;

#include "netboot.h"

typedef struct 
{
    net_ether ether;
    net_ip ip;
    net_udp udp;
    net_boot boot;
} netboot_packet;


int ipchksum(unsigned short *ip, int len)
{
    unsigned long sum = 0;
    len >>= 1;
    while (len--) {
        sum += *(ip++);
        if (sum > 0xFFFF)
            sum -= 0xFFFF;
    }
    return((~sum) & 0x0000FFFF);
}

static netboot_packet nbr;

void handle_udp(udp_packet *pkt)
{
    packet_buffer pbuf;
    
/*    printf("handle_udp: %d -> %d, l=%d, ck=%d\n",
           ntohs(pkt->udp.udp_src),
           ntohs(pkt->udp.udp_dst),
           ntohs(pkt->udp.udp_len),
           ntohs(pkt->udp.udp_chk));*/

    if(ntohs(pkt->udp.udp_dst) == NETBOOT_PORT){
        netboot_packet *nb = (netboot_packet *) pkt;
        unsigned int addr = ntohs(nb->boot.blk) * 1024 + NETBOOT_BASE;
        
        if(ntohs(nb->boot.cmd) != NETBOOT_CMD_LOAD &&
           ntohs(nb->boot.cmd) != NETBOOT_CMD_EXEC){
            printf("netboot: invalid command %d\n",
                    ntohs(nb->boot.cmd));
            return;
        }

        if(!got_server){
            memcpy(&server_mac,&(pkt->ether.src),6);
            memcpy(&server_ip,&(pkt->ip.ip_src),4);
            printf("netboot: server %d.%d.%d.%d @ %X:%X:%X:%X:%X:%X\n",
                   server_ip[0],server_ip[1],server_ip[2],server_ip[3],
                   server_mac[0],server_mac[1],server_mac[2],server_mac[3],
                   server_mac[4],server_mac[5]);
            got_server = 1;            
        }
        if(ntohs(nb->boot.cmd) == NETBOOT_CMD_LOAD){
/*            printf("netboot: loading 1024 bytes, net -> 0x%x\n",
                   addr);*/
            if(addr == NETBOOT_BASE) {
                printf("loading ");
            } else {
                printf(".");
            }

            memcpy(((void *) addr), &(nb->boot.data), 1024);
        }
        
        memcpy(&(nbr.ether.src),&(prom),6);
        memcpy(&(nbr.ether.dst),&(server_mac),6);
        nbr.ether.type = ntohs(0x0800);
        
        memcpy(&(nbr.ip.ip_src),&ip,4);
        memcpy(&(nbr.ip.ip_dst),&(server_ip),4);
        nbr.ip.ip_hdr_len = 5;
        nbr.ip.ip_version = 4;
        nbr.ip.ip_tos = 0;
        nbr.ip.ip_len = ntohs(20+8+8);
        nbr.ip.ip_id = ntohs(id++);
        nbr.ip.ip_off = 0;
        nbr.ip.ip_ttl = 128;
        nbr.ip.ip_proto = IP_PROTO_UDP;
        nbr.ip.ip_chk = 0;
        nbr.ip.ip_chk = ipchksum(&(nbr.ip),sizeof(net_ip));
        
        nbr.udp.udp_dst = pkt->udp.udp_src;
        nbr.udp.udp_src = pkt->udp.udp_dst;
        nbr.udp.udp_chk = 0;
        nbr.udp.udp_len = ntohs(8 + 4);

        nbr.boot.cmd = htons(NETBOOT_CMD_ACK);
        nbr.boot.blk = nb->boot.blk;
        pbuf.ptr = (unsigned char *) &nbr;
        pbuf.page = 0;
        pbuf.len = 14 + 20 + 8 + 4;

        if(ntohs(nb->boot.cmd) == NETBOOT_CMD_EXEC){
            boot_dir *bd = (boot_dir *) NETBOOT_BASE;            
            volatile void (*start)(int, char *, boot_dir *) =
                bd->bd_entry[1].be_code_ventr +
                0x1000 + NETBOOT_BASE;
            
            printf("\nnetboot: executing at 0x%x\n", (int) start);
            start(memsize, param[0] ? param : (char *) ip0, bd);            
            asm("hlt");           
        } else {
            nic_send_packet(&TheSNIC, &pbuf);
        }

    }
}


void handle_ip(ip_packet *pkt)
{
    unsigned char *src = (unsigned char *) &( pkt->ip.ip_src );
    unsigned char *dst = (unsigned char *) &( pkt->ip.ip_dst );
    int i;
    
/*    printf("handle_ip: IP: %d.%d.%d.%d -> %d.%d.%d.%d\n",
           src[0],src[1],src[2],src[3],
           dst[0],dst[1],dst[2],dst[3]);*/

    if(!memcmp(dst,ip,4)){        
        if(ipchksum(&(pkt->ip),sizeof(net_ip))) {
            return;
        }        
/*        printf("handle_ip: for me? whoah!\n");*/
        if(pkt->ip.ip_proto == IP_PROTO_UDP) handle_udp((udp_packet *) pkt);
        
    }
    
}

void print_arp(unsigned char *p);


unsigned char bcast[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
unsigned char zilch[6] = { 0, 0, 0, 0, 0, 0 };

void issue_rarp_request(void)
{
    arp_packet req;
    packet_buffer pbuf;            

    req.ether.type = htons(0x8035);    
    memcpy(&(req.ether.src),prom,6);
    memcpy(&(req.ether.dst),bcast,6);
    req.arp.arp_hard_type = htons(1);
    req.arp.arp_prot_type = htons(0x0800);
    req.arp.arp_hard_size = 6;
    req.arp.arp_prot_size = 4;            
    req.arp.arp_op = htons(RARP_OP_REQUEST);
    memcpy(&(req.arp.arp_enet_sender),prom,6);
    memcpy(&(req.arp.arp_ip_sender),bcast,4);
    memcpy(&(req.arp.arp_enet_target),prom,6);
    memcpy(&(req.arp.arp_ip_target),bcast,4);

    pbuf.ptr = (unsigned char *) &req;
    pbuf.page = 0;
    pbuf.len = sizeof(arp_packet);            
    nic_send_packet(&TheSNIC, &pbuf);                   
}

void handle_rarp(arp_packet *req)
{
    unsigned char *x;
    
    if(htons(req->arp.arp_op) == RARP_OP_REPLY){
        if(!memcmp(&(req->arp.arp_enet_target),prom,6)){
            if(memcmp(&(req->arp.arp_ip_target),ip,4)){
                x = &(req->arp.arp_ip_target);
                ip[0] = x[0];
                ip[1] = x[1];
                ip[2] = x[2];
                ip[3] = x[3];
                got_ip = 1;
                ipmsg = "(RARP'd)";
                reset = 1;
            }
        }
    }
}

void handle_arp(arp_packet *req)
{
    if(htons(req->arp.arp_op) == ARP_OP_REQUEST){
        if(!memcmp(&(req->arp.arp_ip_target),ip,4)){
            packet_buffer pbuf;            
            arp_packet resp;            
/*            printf("handle_arp: IS the one!\n");*/
/*            printf("handle_arp: replying to arp request\n");            */
            memcpy(&(resp.ether.src),prom,6);
            memcpy(&(resp.ether.dst),&(req->ether.src),6);
            resp.ether.type = htons(0x0806);
            resp.arp.arp_hard_type = htons(1);
            resp.arp.arp_prot_type = htons(0x0800);
            resp.arp.arp_hard_size = 6;
            resp.arp.arp_prot_size = 4;            
            resp.arp.arp_op = htons(ARP_OP_REPLY);
            memcpy(&(resp.arp.arp_enet_sender),prom,6);
            memcpy(&(resp.arp.arp_ip_sender),ip,4);
            memcpy(&(resp.arp.arp_enet_target),&(req->arp.arp_enet_sender),6);
            memcpy(&(resp.arp.arp_ip_target),&(req->arp.arp_ip_sender),4);
/*            print_arp((unsigned char *) &(resp.arp));*/
            pbuf.ptr = (unsigned char *) &resp;
            pbuf.page = 0;
            pbuf.len = sizeof(arp_packet);            
            nic_send_packet(&TheSNIC, &pbuf);            
        } else {
/*            printf("handle_arp: NOT the one.\n");            */
        }
        return;        
    }
}

void print_arp(unsigned char *p)
{
#ifdef ARP_DEBUG
    net_arp *arp = (net_arp *) p;
    unsigned char *b;    
    unsigned short t;
    
    printf("  ARP: ");
    t = htons(arp->arp_op);
    if(t == ARP_OP_REQUEST) printf("req ");
    else if(t == ARP_OP_REPLY) printf("rep ");
    else printf("??? ");

    b = (unsigned char *) &(arp->arp_enet_sender);
    printf("source:  %X:%X:%X:%X:%X:%X ",b[0],b[1],b[2],b[3],b[4],b[5]);
    b = (unsigned char *) &(arp->arp_ip_sender);
    printf("(%d.%d.%d.%d)\n",b[0],b[1],b[2],b[3]);

    printf("  ARP:     target:  ");    
    
    b = (unsigned char *) &(arp->arp_enet_target);
    printf("%X:%X:%X:%X:%X:%X ",b[0],b[1],b[2],b[3],b[4],b[5]);
    b = (unsigned char *) &(arp->arp_ip_target);
    printf("(%d.%d.%d.%d)\n",b[0],b[1],b[2],b[3]);    
#endif
}


void receive(packet_buffer *packet)
{
    unsigned char *b = packet->ptr;
    int i;

    if(b[12] == 0x80 && b[13] == 0x35) {
        handle_rarp((arp_packet *) b);
        return;
    }
    if(b[12] != 0x08) return; /* ignore non IP/ARP packets */
    
    if(b[12] == 0x08 && b[13] == 0x06) {
/*        print_arp(b+14);*/
        handle_arp((arp_packet *) b);
        return;
    }

    if(b[12] == 0x08 && b[13] == 0x00) {
        handle_ip((ip_packet *) b);
        return;
    } 
}

void find_8390(int *addr);

extern char *blagh;

void main(int mem)
{
    int i,mono;
    int nh;    
    char buf[128];
    int snic_irq = 3;    
    char *x;
	
    memsize = mem;
    param[0] = 0;
    
    mono = (((*((unsigned char *) 0x410)) & 0x30) == 0x30);

    parse(defaults);
    
	find_8390(&nic_addr);
	
	ipmsg = "(default)";
	ip[0] = ip[1] = ip[2] = ip[3] = 10;
	
    for(;;){
        reset = 0;
        
        TheSNIC.iobase = 0;
        nic_init(&TheSNIC, nic_addr, prom, NULL);

        con_start( mono ? 0xB0000 : 0xB8000);

        if(!mono){
            con_fgbg(CON_WHITE, CON_BLUE /*WHITE,CON_BLUE*/);
        }
/*              01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
		printf("\n");
		printf("    ##   ##  ######  ######  ######    #####    #####   ######\n");
		printf("    ###  ##  ##        ##    ##   ##  ##   ##  ##   ##    ##\n");
		printf("    ## # ##  #####     ##    ######   ##   ##  ##   ##    ##\n");
		printf("    ##  ###  ##        ##    ##   ##  ##   ##  ##   ##    ##     Version 1.5\n");
		printf("    ##   ##  ######    ##    ######    #####    #####     ##     " __DATE__);
		
        if(!mono){
            con_fgbg(CON_WHITE,CON_BLACK);
        }
        printf("\n\n\n");
		printf("0x00090000 - bootloader start\n");
        printf("0x%x - bootloader end\n", (int) end);
        printf("0x00100000 - target load address\n");
        printf("0x%x - top of memory\n",mem);
        printf("\n");
        
        printf("NE2000 @ 0x%S (%X:%X:%X:%X:%X:%X)\n", nic_addr,
               prom[0],prom[1],prom[2],prom[3],prom[4],prom[5]);    
        
        printf("ip = %d.%d.%d.%d %s\n",ip[0],ip[1],ip[2],ip[3],
               ipmsg);
        if(param[0]) printf("boot = \"%s\"\n",param);
        
        nic_register_notify(&TheSNIC,receive);
        
        nic_start(&TheSNIC,0);
        
        printf("\nready. (ESC for console)\n");

        if(!got_ip){
            issue_rarp_request();
            issue_rarp_request();
            issue_rarp_request();
        }
		
        while(!reset){
            nic_isr(&TheSNIC);
            if(inb(0x64) & 0x01) {
                if(inb(0x60) == 1){
                    con_fgbg(CON_YELLOW,CON_BLACK);
                    if(console()) break;
                }
            }
        }
        nic_stop(&TheSNIC);            
    }
}
