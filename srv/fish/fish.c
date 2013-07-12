/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <i386/io.h>
#include <blt/namer.h>
#include <blt/syscall.h>

#include "dfp.h"
#include "vga.h"

static int WITH_NET = 1;

void keythread(void);

int myX = 2;
int myY = 2;

int st_sent = 0;
int st_recv = 0;
int st_rej = 0;
int st_count = 0;
int sticky = 0;
int collision = 0;
int snooze = 5;

int logsize = 15;


int borg = 0;
int logo = 0;
int logging = 1;
int recluse = 0;
int ticks = 0;

#define LOGGING
#define TIMEOUT 8

#define MAXKEYS 64
static char keybuf2[MAXKEYS+4];
static char *keybuf = keybuf2+2;
static int keyptr = 0;

int port_fish, port_net, port_console, port_net_xmit;

static char kpbuf[512];
static char *kp = kpbuf;
static char *x;    

typedef struct _msg 
{
    struct _msg *next;
    char buf[65];
} msg;
msg *first_msg = NULL, *last_msg = NULL;
int msgcount = 0;

int sem_fish = 0, sem_msgs = 0;

void lprintf(char *fmt,...)
{
#ifdef LOGGING
    va_list pvar;
    int n;
    msg *m = (msg *) malloc(sizeof(msg));
    
    va_start(pvar,fmt);
    va_snprintf(m->buf,64,fmt,pvar);
    va_end(pvar);

    m->buf[60]=0;
    m->next = NULL;
    sem_acquire(sem_msgs);
    msgcount++;
    if(last_msg){
        last_msg->next = m;
        last_msg = m;
    } else {
        first_msg = last_msg = m;
    }
    sem_release(sem_msgs);
#endif
}

#define DEAD 0
#define LIVE 1
#define XMIT 2
typedef struct _fish 
{
    struct _fish *next, *prev;
    int x,y;
    int dx,dy;
    char name[17];
    char *bitmap;
    int state;
    int xt;
    dfp_pkt_transfer xfer;
    int dest;
} fish;

fish *first = NULL;
fish *last = NULL;


struct 
{
    int live;
    int tx, ty;   
} cnxn[4] = { {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} };

#define C_UP 0
#define C_DN 1
#define C_LF 2
#define C_RT 3
#define NET_CONNECT  1
#define NET_SEND     2
#define NET_IP       3

void settank(int tx, int ty, int live)
{
    int i;
    for(i=0;i<4;i++){
        if(cnxn[i].tx == tx && cnxn[i].ty == ty){
            cnxn[i].live = live;
            return;
        }
    }
}


typedef struct 
{
    int cmd; 
    int port;
} net_cmd;

typedef struct 
{
    net_cmd cmd;
    dfp_pkt_transfer dfp;
} net_fish;

void newfish(int x, int y, int dx, int dy, char *n);

void pingtank(int tx, int ty)
{
	if(WITH_NET){
		net_fish f;
		msg_hdr_t msg;
		f.cmd.cmd = NET_SEND;
		f.cmd.port = 5049;
		f.dfp.base.src_tank_x = myX;
		f.dfp.base.src_tank_y = myY;
		f.dfp.base.dst_tank_x = tx;
		f.dfp.base.dst_tank_y = ty;
		f.dfp.base.size = htons(DFP_SIZE_LOCATE);
		f.dfp.base.type = DFP_PKT_PING;
		f.dfp.base.pad = 0;
		f.dfp.base.version = htons(DFP_VERSION);
		msg.flags = 0;
		msg.src = port_fish;
		msg.dst = port_net;
		msg.size = sizeof(dfp_base)+8;
		msg.data = &f;
		old_port_send(&msg);
	}
}

void initiate(dfp_pkt_transfer *send, int tx, int  ty)
{
	if(WITH_NET){
		net_fish f;
		msg_hdr_t msg;
		f.cmd.cmd = NET_SEND;
		f.cmd.port = 5049;
		f.dfp.base.src_tank_x = myX;
		f.dfp.base.src_tank_y = myY;
		f.dfp.base.dst_tank_x = tx;
		f.dfp.base.dst_tank_y = ty;
		f.dfp.base.size = ntohs(DFP_SIZE_TRANSFER);
		f.dfp.base.type = DFP_PKT_SEND_FISH;
		f.dfp.base.pad = 0;
		f.dfp.base.version = htons(DFP_VERSION);
		memcpy(&(f.dfp.uid), &(send->uid), 6);
		memcpy(&(f.dfp.fish), &(send->fish), sizeof(dfp_fish));
		msg.flags = 0;
		msg.src = port_fish;
		msg.dst = port_net;
		msg.size = DFP_SIZE_TRANSFER + 8;
		msg.data = &f;
		old_port_send(&msg);
	}

}

char *types[] = { "PING", "PING", "SEND", "ACK", "NACK", "SYNC" };

void dofish(dfp_pkt_transfer *dfp)
{
    int i;
    
    net_fish f;
    fish *ff;
    char n2[17];
    msg_hdr_t msg;
    dfp->base.version = ntohs(dfp->base.version);
    dfp->base.size = ntohs(dfp->base.size);

    if(!(dfp->base.dst_tank_x == myX && dfp->base.dst_tank_y == myY)){
        return;
    }

    
  if(dfp->base.type == DFP_PKT_SYNC_FISH)  lprintf("fish: s(%d,%d) d(%d,%d) s=%d %s",
           dfp->base.src_tank_x, dfp->base.src_tank_y,
           dfp->base.dst_tank_x, dfp->base.dst_tank_y,
           dfp->base.size,
           dfp->base.type < 6 ? types[dfp->base.type] : "????"
           );
           
    switch(dfp->base.type){
    case DFP_PKT_PING :
        f.cmd.cmd = NET_SEND;
        f.cmd.port = 5049;
        f.dfp.base.src_tank_x = myX;
        f.dfp.base.src_tank_y = myY;
        f.dfp.base.dst_tank_x = dfp->base.src_tank_x;
        f.dfp.base.dst_tank_y = dfp->base.src_tank_y;
        f.dfp.base.size = htons(DFP_SIZE_LOCATE);
        f.dfp.base.type = DFP_PKT_PONG;
        f.dfp.base.pad = 0;
        f.dfp.base.version = htons(DFP_VERSION);
        settank(dfp->base.src_tank_x,dfp->base.src_tank_y,1);
        msg.flags = 0;
        msg.src = port_fish;
        msg.dst = port_net;
        msg.size = sizeof(dfp_base)+8;
        msg.data = &f;
        old_port_send(&msg);

        
        break;
               
    case DFP_PKT_PONG :
        lprintf("Pong from %d,%d",dfp->base.src_tank_x,dfp->base.src_tank_y);
        settank(dfp->base.src_tank_x,dfp->base.src_tank_y,1);
/*        for(i=0;i<4;i++){
            if((cnxn[i].tx == dfp->base.src_tank_x) &&
               (cnxn[i].ty == dfp->base.src_tank_y)) {
                cnxn[i].live = 1;
                break;
            }
        }*/
        break;

    case DFP_PKT_SEND_FISH :
        dfp->fish.ttl = ntohs(dfp->fish.ttl);
        strncpy(n2,dfp->fish.name,16);
        n2[16]=0;
        if(recluse){
            lprintf("Ignoring \"%s\" from (%d,%d)",n2,
                   dfp->base.src_tank_x, dfp->base.src_tank_y);
            break;
        }
            /* lprintf("    : UID %X%X%X%X%X%X @(%d,%d) d(%d,%d) ttl=%d, name=\"%s\"",
               dfp->uid.creator_tank_x, dfp->uid.creator_tank_y,
               dfp->uid.timestamp[0], dfp->uid.timestamp[1],
               dfp->uid.timestamp[2], dfp->uid.timestamp[3],
               dfp->fish.x, dfp->fish.y, dfp->fish.dx, dfp->fish.dy,
               dfp->fish.ttl, n2);
               */
        f.cmd.cmd = NET_SEND;
        f.cmd.port = 5049;
        f.dfp.base.src_tank_x = myX;
        f.dfp.base.src_tank_y = myY;
        f.dfp.base.dst_tank_x = dfp->base.src_tank_x;
        f.dfp.base.dst_tank_y = dfp->base.src_tank_y;
        f.dfp.base.size = ntohs(DFP_SIZE_CONFIRM);
        f.dfp.base.type = DFP_PKT_ACK_FISH;
        f.dfp.base.pad = 0;
        f.dfp.base.version = htons(DFP_VERSION);
        memcpy(&(f.dfp.uid), &(dfp->uid), 6);
        
        msg.flags = 0;
        msg.src = port_fish;
        msg.dst = port_net;
        msg.size = DFP_SIZE_CONFIRM + 8;
        msg.data = &f;
        old_port_send(&msg);
        break;

    case DFP_PKT_ACK_FISH :
            /*  lprintf("    : UID %X%X%X%X%X%X",
               dfp->uid.creator_tank_x, dfp->uid.creator_tank_y,
               dfp->uid.timestamp[0], dfp->uid.timestamp[1],
               dfp->uid.timestamp[2], dfp->uid.timestamp[3]);
               */
        sem_acquire(sem_fish);
        for(ff = first; ff; ff=ff->next){
            if((ff->state==XMIT) &&
               (!memcmp(&(ff->xfer.uid),&(dfp->uid),6))){
                ff->state = DEAD;
                st_count--;
                sem_release(sem_fish);
                lprintf("Goodbye fish \"%s\" (%d,%d)",ff->name,
                       dfp->base.src_tank_x,dfp->base.src_tank_y);
                
                f.cmd.cmd = NET_SEND;
                f.cmd.port = 5049;
                f.dfp.base.src_tank_x = myX;
                f.dfp.base.src_tank_y = myY;
                f.dfp.base.dst_tank_x = dfp->base.src_tank_x;
                f.dfp.base.dst_tank_y = dfp->base.src_tank_y;
                f.dfp.base.size = ntohs(DFP_SIZE_TRANSFER);
                f.dfp.base.type = DFP_PKT_SYNC_FISH;
                f.dfp.base.pad = 0;
                f.dfp.base.version = htons(DFP_VERSION);
                memcpy(&(f.dfp.uid), &(ff->xfer.uid), 6);
                memcpy(&(f.dfp.fish), &(ff->xfer.fish), sizeof(dfp_fish));
                msg.flags = 0;
                msg.src = port_fish;
                msg.dst = port_net;
                msg.size = DFP_SIZE_TRANSFER + 8;
                msg.data = &f;
                old_port_send(&msg);
                st_sent++;
                goto donesync;
            }
        }
        sem_release(sem_fish);
donesync:
        break;

    case DFP_PKT_NACK_FISH :
        break;

    case DFP_PKT_SYNC_FISH :
        dfp->fish.ttl = ntohs(dfp->fish.ttl);

        strncpy(n2,dfp->fish.name,16);
        n2[16]=0;
             lprintf("    : UID %X%X%X%X%X%X @(%d,%d) d(%d,%d) ttl=%d, name=\"%s\"\n",
               dfp->uid.creator_tank_x, dfp->uid.creator_tank_y,
               dfp->uid.timestamp[0], dfp->uid.timestamp[1],
               dfp->uid.timestamp[2], dfp->uid.timestamp[3],
               dfp->fish.x, dfp->fish.y, dfp->fish.dx, dfp->fish.dy,
               dfp->fish.ttl, n2);
               
        newfish(dfp->fish.x, dfp->fish.y,
                dfp->fish.dx, dfp->fish.dy,
                n2);
/*        lprintf("Welcome, \"%s\" from (%d,%d)",n2,
               dfp->base.src_tank_x, dfp->base.src_tank_y);*/

        settank(dfp->base.src_tank_x,dfp->base.src_tank_y,1);
    default:
        break;        
    }
}

void test(int x, int y)
{
    static dfp_pkt_transfer tran;
    tran.fish.x = 100;
    tran.fish.y = 100;
    tran.fish.dx = 3;
    tran.fish.dy = 4;
    tran.fish.ttl = htons(1600);
    tran.uid.creator_tank_x = myX;
    tran.uid.creator_tank_y = myY;
    *((uint32 *) tran.uid.timestamp) = 0x10203040;
    
    strcpy(tran.fish.name,"Bitchin Fish");

    initiate(&tran,x,y);
}

unsigned char *fish_bm = 
    ".........XXXXXX.....XXX."
    ".....XXXXYYYYYYXX..XYYYX"
    "...XXYY.YYYYYYYYYXXYYYYX"
    ".XXYYYYYYYYYYYYYYYYYYYX."
    "...XXYYYYYYYYYYYYYXXYYYX"
    ".....XXXYYYYYYYXXX..XXYX"
    "........XXXYYYX.......XX"
    "...........XXXXX........"
;

unsigned char *arrow_up[2] ={
    "....rr...."
    "...rrrr..."
    "..rrrrrr.."
    ".rrrrrrrr."
    "rrrrrrrrrr"
,
    "....gg...."
    "...gggg..."
    "..gggggg.."
    ".gggggggg."
    "gggggggggg"
};
unsigned char *arrow_dn[2] ={
    "rrrrrrrrrr"
    ".rrrrrrrr."
    "..rrrrrr.."
    "...rrrr..."
    "....rr...."
,
    "gggggggggg"
    ".gggggggg."
    "..gggggg.."
    "...gggg..."
    "....gg...."
};
unsigned char *arrow_lf[2] ={
    "....r"
    "...rr"
    "..rrr"
    ".rrrr"
    "rrrrr"
    "rrrrr"
    ".rrrr"
    "..rrr"
    "...rr"
    "....r"
,
    "....g"
    "...gg"
    "..ggg"
    ".gggg"
    "ggggg"
    "ggggg"
    ".gggg"
    "..ggg"
    "...gg"
    "....g"
};
unsigned char *arrow_rt[2] = {
    "r...."
    "rr..."
    "rrr.."
    "rrrr."
    "rrrrr"
    "rrrrr"
    "rrrr."
    "rrr.."
    "rr..."
    "r...."
,
    "g...."
    "gg..."
    "ggg.."
    "gggg."
    "ggggg"
    "ggggg"
    "gggg."
    "ggg.."
    "gg..."
    "g...."
};

int dist(int x0, int y0, int x1, int y1)
{
    return((x0-x1)*(x0-x1)+(y0-y1)*(y0-y1));
}

void sendfish(fish *f, int tx, int ty)
{
    f->xfer.fish.x = f->x;
    f->xfer.fish.y = f->y;
    f->xfer.fish.dx = f->dx;
    f->xfer.fish.dy = f->dy;
    f->xfer.fish.ttl = htons(1600);
    f->state = XMIT;
    f->xt = ticks+TIMEOUT;
    initiate(&f->xfer,tx,ty);
}

void prep(void)
{
    int i;
    cnxn[C_LF].tx = myX - 1;
    cnxn[C_LF].ty = myY;
    cnxn[C_RT].tx = myX + 1;
    cnxn[C_RT].ty = myY;
    cnxn[C_UP].tx = myX;
    cnxn[C_UP].ty = myY - 1;
    cnxn[C_DN].tx = myX;
    cnxn[C_DN].ty = myY + 1;
    for(i=0;i<4;i++){
        cnxn[i].live = 0;
        pingtank(cnxn[i].tx,cnxn[i].ty);
    }
}


void vloop(void)
{
    fish *f,*lf;
    fish *f2;
    
    int x,y,i;
    int xx, xy, xd;
    msg *m;    
    
    for(i=0;i<4;i++){
        cnxn[i].live = 0;
        pingtank(cnxn[i].tx,cnxn[i].ty);
    }
    
    for(;;){
        ticks++;
        if(!(ticks % 200)){
            lprintf("pinging now...");
            for(i=0;i<4;i++){
                cnxn[i].live = 0;
                pingtank(cnxn[i].tx,cnxn[i].ty);
            }
        }
		os_sleep(snooze * 9000); /* 9ms increments */
        vga_fillX();
#ifdef LOGGING               
        sem_acquire(sem_msgs);
        if(msgcount > logsize){
            msgcount--;
            m = first_msg;
            first_msg = first_msg->next;
            free(m);
        }
        if(logging){
            for(y=16,m = first_msg; (y < 161) && m; m=m->next, y +=8){
                vga_blit_str(m->buf,15,y,'#');
            }
        }
        sem_release(sem_msgs);
#endif        
        sem_acquire(sem_fish);
        for(lf=NULL,f=first;f;f=f->next){
            if(f->state == DEAD && lf){
                lprintf("Expiring \"%s\"",f->name);
                lf->next = f->next;
                free(f);
                f = lf;
                continue;
            }
            if((f->state == XMIT) && (ticks > f->xt)) {
                cnxn[f->dest].live = 0;
                f->state = LIVE;
                st_rej++;
            }
            if(f->state == LIVE){
                xx = xy = 0;
                xd = -1;
                x = f->x;
                y = f->y;
                if(f->x < 0){
                    xx = 1;
                    if(cnxn[C_LF].live) {
                        xd = C_LF;
                        f->x = 254;
                    }
                }
                if(f->x > 255){
                    xx = 1;
                    if(cnxn[C_RT].live) {
                        xd = C_RT;
                        f->x = 1;
                    }                        
                }
                if(f->y < 0){
                    xy = 1;
                    if(cnxn[C_UP].live) {
                        if(xd == -1) {
                            xd = C_UP;
                            f->y = 254;
                        }
                    }
                }
                if(f->y > 255) {
                    xy = 1;
                    if(cnxn[C_DN].live) {
                        if(xd == -1) {
                            xd = C_DN;
                            f->y = 1;
                        }
                    }    
                }
                if(!sticky && !recluse && (xd != -1)) {
                    f->dest = xd;
                    sendfish(f,cnxn[xd].tx,cnxn[xd].ty);
                }
                if(xx){
                    f->dx = - f->dx;
                    f->x = x + f->dx;
                }                    
                if(xy){
                    f->dy = - f->dy;
                    f->y = y + f->dy;
                }
                if(xd != -1) continue;
#ifdef XXX
                    /* collision ? */
                {
                    int xc,yc;
                    xc = f->x + 12;
                    yc = f->y + 4;
                    
                    for(f2 = first; f2; f2 = f2->next){
                        if((f2 == f) || (f2->state != LIVE)) continue;
                        if(dist(xc,yc,f2->x+12,f2->y+4) < 100){
                            if(collision){
                                f->dx = - f->dx;
                                f->dy = - f->dy;
                            }
                            if(!strcmp(f->name,"killer")) {
                                f2->state = DEAD;
                            }
                            if(!strcmp(f2->name,"killer")){
                                f->state = DEAD;
                            }
                            break;
                        }
                    }
                }
                    /* collision - */
#endif
                f->x += f->dx;
                f->y += f->dy;
                x = (f->x * (320-24)) / 256;
                y = (f->y * (200-16)) / 256 + 8;
                
                if(f->dx < 0){
                    vga_blit_trans(f->bitmap,x,y,24,8);
                } else {
                    vga_blit_trans_r(f->bitmap,x,y,24,8);                
                }
                vga_blit_str(f->name,x,y-8,'w');
            }
            lf = f;
        }
        sem_release(sem_fish);


        vga_blit_trans(arrow_up[cnxn[C_UP].live],320/2-5,0,10,5);
        vga_blit_trans(arrow_dn[cnxn[C_DN].live],320/2-5,200-9,10,5);
        vga_blit_trans(arrow_lf[cnxn[C_LF].live],0,100-5,5,10);
        vga_blit_trans(arrow_rt[cnxn[C_RT].live],320-6,100-5,5,10);
       /* if(logo) vga_blit_trans(blt,319-blt_w,2,blt_w,blt_h); */
        if(sticky) vga_blit_str("STICKY",0,0,'r');
        if(recluse) vga_blit_str("RECLUSIVE",8*7,0,'r');
        if(borg) vga_blit_str("BORG",320-(8*6),0,'r');
        
        if(keybuf[0]){
            vga_fill(320,10,0,179,'b');
            vga_blit_str(keybuf2,1,180,'w');
        }
        vga_swap_buffers();
    }
}

int bc = 0;

void newfish(int x, int y, int dx, int dy, char *n)
{
    fish *f = (fish *) malloc(sizeof(fish));
    
    char bf[16];
    
    st_recv++;
    if(borg) {
        n = bf;
        snprintf(n,16,"Borg (%d)",bc++);
        bf[15]=0;
    }
    f->x = x;
    f->y = y;
    f->dx = dx;
    f->dy = dy;
    strncpy(f->name,n,16);
    f->name[16]=0;
    f->bitmap = fish_bm;
    f->state = LIVE;
    f->xfer.uid.creator_tank_x = myX;
    f->xfer.uid.creator_tank_y = myY;
    *((uint32 *)f->xfer.uid.timestamp) = 0x1000000 + ticks;
        /*XXX */
    
    strcpy(f->xfer.fish.name,f->name);
    sem_acquire(sem_fish);
    f->next = first;
    first = f;
    sem_release(sem_fish);
    st_count++;
}



void vinit(void)
{
    int i;
    void *vmem = 0xA0000;
	area_create(64*1024, 0, &vmem, AREA_PHYSMAP);

    vga_set_sram(vmem);
    vga_set_mode(320,200,8);

	vmem = malloc(64*1024);
	vga_set_vram(vmem);

    vga_set_palette('.',0,0,255);
    vga_set_palette('w',255,255,255);
    vga_set_palette('X',255,0,0);
    vga_set_palette('Y',255,255,0);
    vga_set_palette('#',0,255,0);
    vga_set_palette('g',0,255,0);
    vga_set_palette('r',255,0,0);
    vga_set_palette('b',0,0,0);
    vga_set_palette('L',0,255,0);
    for(i=0;i<50;i++){
        vga_set_palette(128+i,0,0,12+i);
    }
    
    vga_fill_grad(320,200,0,0);
    vga_swap_buffers();
    
    lprintf("fish: vinit()");

    newfish(160,160,3,3,"fish");
    newfish(100,40,-1,2,"not fish");
}

void command(char *str);

void fishmain(void)
{	
    int once = 1;
    unsigned char data[1500];
    msg_hdr_t msg;
    int i;
    int size;
    net_cmd cmd;
    unsigned char ip[4];

    sem_fish = sem_create(1, "fish_lock");
    sem_msgs = sem_create(1, "msgs_lock");
    
    keybuf2[0] = '>';
    keybuf2[1] = ' ';
    
    if((port_net = namer_find("net",0)) < 1) WITH_NET = 0;
    if((port_net_xmit = namer_find("net_xmit",0)) < 1) WITH_NET = 0;
  
    port_fish = port_create(0, "fish_tell_port");
    namer_register(port_fish, "fish:tell");

    vinit();

    os_thread(vloop);
/*    os_thread(restarter); */
    
    lprintf("fish: port=%d, console=%d, net=%d\n",
           port_fish, port_console, port_net);

	if(WITH_NET){    
		msg.flags = 0;
		msg.src = port_fish;
		msg.dst = port_net;
		msg.size = 8;
		msg.data = &cmd;
		
		cmd.cmd = NET_IP;
		cmd.port = 0;
		old_port_send(&msg);
		msg.src = port_net_xmit;
		msg.dst = port_fish;
		msg.size = 4;
		msg.data = ip;
		old_port_recv(&msg);

		lprintf("IP = %d.%d.%d.%d\n",ip[0],ip[1],ip[2],ip[3]);
		
		myX = (ip[3] >> 3) & 0x07;
		myY = (ip[3]) & 0x07;
		lprintf("tank @ %d,%d",myX, myY);
		
		msg.flags = 0;
		msg.src = port_fish;
		msg.dst = port_net;
		msg.size = 8;
		msg.data = &cmd;
		
		cmd.cmd = NET_CONNECT;
		cmd.port = 5049;
		old_port_send(&msg);
	} else {
		lprintf("no network support\n");
	}

    prep();
#if 0
    os_thread(keythread);
#endif    
    for(;;){
        msg.flags = 0;
        msg.src = 0;
        msg.dst = port_fish;
        msg.size = 1500;
        msg.data = data;

        if((size = old_port_recv(&msg)) > 0){
			if(WITH_NET && (msg.src == port_net_xmit)){
				dofish((dfp_pkt_transfer *) data);
			} else {
				uint32 response = 0;
				data[size]=0;
				command(data);
				msg.dst = msg.src;
				msg.src = port_fish;
				msg.size = 4;
				msg.data = &response;
				old_port_send(&msg);	
			}
        }
    }
}

int main(void)
{
	os_thread(fishmain);
    return 0;
}

void command(char *str)
{
    if(!strncmp(str,"tank",4)){
        int x,y;
        x = str[5]-'0';
        y = str[6]-'0';
        if(x <0 || x>9 || y<0 || x>9) return;
        myX = x;
        myY = y;
        prep();
        lprintf("relocating tank to %d,%d",myX,myY);
        return;
    }
    if(!strncmp(str,"stats",5)){
        lprintf("count: %d sent: %d recv: %d rej: %d",st_count,st_sent,st_recv,st_rej);
        return;
    }
    if(!strncmp(str,"sticky",6)){
        sticky = !sticky;
        lprintf("Sticky mode %s",sticky?"on":"off");
        return;
    }
    if(!strncmp(str,"recluse",7)){
        recluse = !recluse;
        lprintf("Recluse mode %s",recluse?"on":"off");
        return;
    }
    if(!strncmp(str,"panic",5)){
        fish *f;
        sem_acquire(sem_fish);
        for(f=first;f;f=f->next) {
            f->dx *= 2;
            f->dy *= 2;
        }
        sem_release(sem_fish);
        return;
    }
    if(!strncmp(str,"purge",5)){
        fish *f;
        sem_acquire(sem_fish);
        for(f=first;f;f=f->next) {
            if(f->state == LIVE) f->state = DEAD;
        }
        st_count = 0;
        sem_release(sem_fish);
        return;
    }
    if(!strncmp(str,"logo",4)){
        logo = !logo;
        return;
    }
    if(!strncmp(str,"collision",9)){
        collision = !collision;
        return;
    }
    if(!strncmp(str,"snooze ",7)){
        snooze = str[7]-'0';
        if(snooze < 3 || snooze > 9) snooze = 5;
        return;
    }
    if(!strncmp(str,"borg",4)){
        borg = !borg;
        lprintf("Borg mode %s",borg?"on - prepare to assimilate":"off");
        return;
    }
    if(!strncmp(str,"eject",5)){
        int i;
        lprintf("Ejecting all fish now!");
        for(i=0;i<4;i++){
            if(cnxn[i].live){
                fish *f;
                sem_acquire(sem_fish);
                for(f = first; f; f=f->next){
                    if(f->state == LIVE){
                        sendfish(f,cnxn[i].tx,cnxn[i].ty);
                    }
                }
                sem_release(sem_fish);
                return;
            }
        }
        return;
    }
	if(!strncmp(str,"debug",5)){
		os_debug();
		return;
	}
    if(!strncmp(str,"log",3)){
        logging = !logging;
        return;
    }
    if(!strncmp(str,"new ",3)) {
        str+=4;
        str[16]=0;
        if(str[0] == '*'){
            newfish(100,100,-3,-5,str+1);
            newfish(130,100, 3,-5,str+1);
            newfish(160,100,-3, 5,str+1);
            newfish(190,100, 3, 5,str+1);
            newfish(100,150,-3,-5,str+1);
            newfish(130,150, 3,-5,str+1);
            newfish(160,150,-3, 5,str+1);
            newfish(190,150, 3, 5,str+1);
        } else {
            newfish(160,160,-3,5,str);
        }
    }
    
}

                
/* keyboard handler */

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

void keythread(void)
{
    int shift = 0;    
    int key;

    os_handle_irq(1);

    for(;;) {
        os_sleep_irq();
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
                    switch(key){
                    case CR:
                        if(keyptr) command(keybuf);
                    case ESC:
                        keybuf[0] = 0;
                        keyptr = 0;
                        break;
                    case BS:
                        if(keyptr){
                            keyptr--;
                            keybuf[keyptr]=0;
                        }
                        break;
                    default:
                        if(keyptr < MAXKEYS){
                            keyptr++;
                            keybuf[keyptr]=0;
                            keybuf[keyptr-1]=key;
                        }
                        break;
                    }
                }
            }
        }
    }

}
