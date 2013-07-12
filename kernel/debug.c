/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef KDEBUG

#include <i386/io.h>
#include <blt/conio.h>

#include <string.h>
#include "kernel.h"
#include "port.h"
#include "rights.h"
#include "resource.h"
#include "list.h"
#include "i386.h"

extern list_t resource_list;

#define RMAX 1024

static char *tstate[] =
{ "KERNL", "RUNNG", "READY", "DEAD ", "WAIT ", "S/IRQ", "S/TMR", "S/PAG" };

uint32 readnum(const char *s);

char *taskstate(task_t *task)
{
	static char ts[60];
	if(task->waiting_on){
		snprintf(ts,60,"Waiting on %s #%d \"%s\"",
				 rsrc_typename(task->waiting_on), task->waiting_on->id,
				 task->waiting_on->name);
		return ts;
	} else {
		return "Running";
	}
}

void printres(resource_t *r)
{
    switch(r->type){
    case RSRC_PORT:
        kprintf("    PORT %U: (slave=%d) (size=%d) \"%s\"",r->id,
		((port_t*)r)->slaved, ((port_t*)r)->msgcount,r->name);
        break;
    case RSRC_TASK:
        kprintf("    TASK %U: \"%s\"",r->id,r->name);		
		kprintf("             : %s",taskstate((task_t*)r));
        break;
    case RSRC_ASPACE:
        kprintf("  ASPACE %U: @ %x",r->id,((aspace_t*)r)->pdir[0]&0xFFFFF000);
        break;
    case RSRC_RIGHT:
        kprintf("   RIGHT %U: %x",r->id,((right_t*)r)->flags);
        break;
    case RSRC_SEM:
        kprintf("     SEM %U: (count=%d) \"%s\"",
				r->id,((sem_t*)r)->count,r->name);
        break;
    case RSRC_AREA:
        kprintf("    AREA %U: virt %x size %x pgroup %x (refcnt=%d)",r->id,
                ((area_t*)r)->virt_addr * 0x1000, ((area_t*)r)->length * 0x1000,
                ((area_t*)r)->pgroup, ((area_t*)r)->pgroup->refcount);
		break;
	case RSRC_QUEUE:
		kprintf("   QUEUE %U: (count=%d) \"%s\"",r->id,r->queue.count,r->name);
		break;
	case RSRC_TEAM:
		kprintf("    TEAM %U: \"%s\"",r->id,r->name);
		break;
    }
}

void dumprsrc(list_t *rl)
{
	node_t *rn = rl->next;
	while(rn != (node_t*) rl) {
		printres((resource_t*)rn->data);
		rn = rn->next;
	}
}

void dumponersrc(const char *num)
{
	int n;
	node_t *rn;

	n = readnum (num);
	rn = resource_list.next;
	while(rn != (node_t*) &resource_list) {
		if (((resource_t*)rn->data)->id == n) {
			printres((resource_t*) rn->data);
			break;
		} else {
			rn = rn->next;
		}
	}
}

void dumptasks(void)
{
    int i,j,n;
    task_t *t;
    aspace_t *a;
    team_t *team;
	
    kprintf("Task Team Aspc State Wait esp      scount   Name");
    kprintf("---- ---- ---- ----- ---- -------- -------- --------------------------------");

    for(i=1;i<RMAX;i++){
        if((t = rsrc_find_task(i))){
            team = t->rsrc.owner;
            a = team->aspace;
            {
                area_t *area = rsrc_find_area(t->rsrc.owner->heap_id);
                if(area) j = area->virt_addr + area->length;
                else j =0;
            }
			
            kprintf("%U %U %U %s %U %x %x %s",
                    i,team->rsrc.id,a->rsrc.id,tstate[t->flags],
			(t->waiting_on ? t->waiting_on->id : 0),t->esp /*j*4096*/,t->scount,
					t->rsrc.name);
            
        }
    }
}


void dumpports()
{
    int i;
    port_t *p;

    kprintf("Port Task Rstr Slav Size Owner Name");
    kprintf("---- ---- ---- ---- ---- --------------------------------");
    
    for(i=1;i<RMAX;i++){
        if((p = rsrc_find_port(i))){
            kprintf("%U %U %U %U %U %s",
                    i, p->rsrc.owner->rsrc.id, p->restrict, p->slaved,
                    p->msgcount, p->rsrc.owner->rsrc.name);
        }
    }

}


static void trace(uint32 ebp,uint32 eip)
{
    int f = 1;

    kprintf("f# EBP      EIP");    
    kprintf("00 xxxxxxxx %x",eip);    
    do {        
/*        kprintf("%X %x %x",f,ebp, *((uint32*)ebp));*/
        kprintf("%X %x %x",f,ebp+4, *((uint32*)(ebp+4)));
        ebp = *((uint32*)ebp);
        f++;        
    } while(ebp < 0x00400000 && ebp > 0x00200000);    
}


static void dump(uint32 addr, int sections)
{
    int i;
    unsigned char *x;
    
    for(i=0;i<sections;i++){
        x = (unsigned char *) (addr + 16*i);
        kprintf("%x: %X %X %X %X  %X %X %X %X  %X %X %X %X  %X %X %X %X",
                addr+16*i,x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7],
                x[8],x[9],x[10],x[11],x[12],x[13],x[14],x[15]);
    }
}

static char *hex = "0123456789abcdef";

#define atoi readhex
uint32 readhex(const char *s)
{
    uint32 n=0;
    char *x;
    while(*s) {
        x = hex;
        while(*x) {
            if(*x == *s){
                n = n*16 + (x - hex);
                break;
            }
            x++;
        }
        s++;
    }
    return n;
}

uint32 readnum(const char *s)
{
	uint32 n=0;
	if((*s == '0') && (*(s+1) == 'x')) return readhex(s+2);
	//while(isdigit(*s)) {
	while(*s) {
		n = n*10 + (*s - '0');
		s++;
	}
	return n;
}

void reboot(void)
{
    i386lidt(0,0);
    asm("int $0");
}

extern aspace_t *flat;

void dumpaddr(int id)
{
    node_t *an;
	area_t *area;
	aspace_t *a = rsrc_find_aspace(id);
	
	if(id == 0){
		aspace_kprint(flat);
		return;
	}
	
	if(!a) {
		kprintf("no such aspace %d",id);
		return;
	}
	
	aspace_print(a);
    for(an = a->areas.next; an != (node_t *) &a->areas; an = an->next){
		area = (area_t*) an->data;
		
        kprintf("area %U virtaddr %x size %x pgroup %x (refcnt=%d)",
                area->rsrc.id, 
                area->virt_addr * 0x1000, 
                area->length * 0x1000, 
                area->pgroup, area->pgroup->refcount);
    }
    	
}

void memory_status(void);
void print_regs(regs *r, uint32 eip, uint32 cs, uint32 eflags);

void dumpteams(void)
{
	node_t *rn = resource_list.next;
	resource_t *rsrc;
	
	while(rn != (node_t*) &resource_list) {
		rsrc = (resource_t*)rn->data;
		if(rsrc->type == RSRC_TEAM){
			kprintf("Team %d (%s)",rsrc->id,rsrc->name);
		}
		rn = rn->next;
	}	
}

void dumpteam(int id)
{
	node_t *rn;
	
	team_t *team = rsrc_find_team(id);
	if(team){
		kprintf("team %d (%s)...",team->rsrc.id,team->rsrc.name);
		rn = team->resources.next;
		while(rn != (node_t*) &team->resources) {
			printres((resource_t*) rn->data);
			rn = rn->next;
		}	
	} else {
		kprintf("no such team %d",id);
	}
	
}

void dumpqueue(int num)
{
	resource_t *rsrc;
	node_t *n;
	
	int i;
	
	for(i=1;i<RSRC_MAX;i++){
		if(rsrc = rsrc_find(i,num)){
			task_t *task;
			kprintf("resource: %d \"%s\"",rsrc->id,rsrc->name);
			kprintf("type    : %s",rsrc_typename(rsrc));
			if(rsrc->owner){
				kprintf("owner   : %d \"%s\"",
						rsrc->owner->rsrc.id,rsrc->owner->rsrc.name);
			}			
			kprintf("count   : %d",rsrc->queue.count);

			for(n = rsrc->queue.next; n != (node_t*) &rsrc->queue; n = n->next){
				kprintf("queue   : task %d \"%s\"",((task_t*)n->data)->rsrc.id,
						((task_t*)n->data)->rsrc.name);
			}
			return;
		}
	}
	kprintf("no such resource %d",n);
}

static int ipchksum(unsigned short *ip, int len)
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

void checksum (char *range)
{
	char *s;
	unsigned int i, good, begin, end;

	if (!*range)
		return;
	for (i = good = 0; (i < strlen (range)) && !good; i++)
	{
		if (range[i] == ' ')
		{
			*(s = range + i) = 0;
			s++;
			good = 1;
		}
	}
	if ((!good) || !*s)
		return;
	begin = atoi (range);
	end = atoi (s);
	kprintf ("%x", ipchksum ((unsigned short *) begin, end - begin));
}

static void dumppgroup(uint32 addr)
{
	pagegroup_t *pg = (pagegroup_t*) addr;
	phys_page_t *pp = pg->pages;
	node_t *an = pg->areas.next;
	int size = pg->size;
	
	kprintf("pgroup @ 0x%x rc=%d sz=%d",addr,pg->refcount,pg->size);
	while(an != (node_t*) &pg->areas){
		kprintf("  area @ 0x%x (id %d) (owner #%d \"%s\")",
				an->data,((area_t*)an->data)->rsrc.id,
				((area_t*)an->data)->rsrc.owner->rsrc.id,
				((area_t*)an->data)->rsrc.owner->rsrc.name);
		an = an->next;
	}
	while(size > 0){
		kprintf("  pages %U %U %U %U %U %U",
				pp->addr[0],pp->addr[1],pp->addr[2],
				pp->addr[3],pp->addr[4],pp->addr[5]);
		size -= 6;
		pp = pp->next;
	}
}

int findpage(uint32 n)
{
	node_t *rn = resource_list.next;
	resource_t *rsrc;
	int count = 0;
	int size,i;
	
	while(rn != (node_t*) &resource_list) {
		rsrc = (resource_t*)rn->data;
		if(rsrc->type == RSRC_AREA){
			area_t *area = (area_t*) rsrc;
			phys_page_t *pp = area->pgroup->pages;
			size = area->pgroup->size;
			while(size > 0){
				for(i=0;i<6;i++,size--){
					if(pp->addr[i] == n){
						kprintf("area %U pgroup %x slot %d",rsrc->id,area->pgroup,i);
						count ++;
					}
				}
				pp = pp->next;
			}
		}
		rn = rn->next;
	}	
	return count;
}

static char linebuf[80];
void k_debugger(regs *r,uint32 eip, uint32 cs, uint32 eflags)
{
    char *line;
    uint32 n;
	
    kprintf("OpenBLT Kernel Debugger");

    for(;;){
        krefresh();
        line = kgetline(linebuf,80);

		if(!strncmp(line,"pgroup ",7)) { dumppgroup(readnum(line+7)); continue; }
        if(!strncmp(line,"resource ", 9)) { dumponersrc(line+9); continue; }
        if(!strcmp(line,"resources")) { dumprsrc(&resource_list); continue; }
		if(!strncmp(line,"queue ",6)) { dumpqueue(readnum(line+6)); continue; }
        if(!strcmp(line,"tasks")) { dumptasks(); continue; }
        if(!strcmp(line,"ports")) { dumpports(); continue; }
        if(!strcmp(line,"memory")) { memory_status(); continue; }
        if(!strcmp(line,"trace")) { trace(r->ebp,eip); continue; }
        if(!strcmp(line,"regs")) { print_regs(r,eip,cs,eflags); continue; }
        if(!strncmp(line,"dump ",5)) { dump(readnum(line+5),16); continue; }
        if(!strncmp(line,"aspace ",7)) { dumpaddr(readnum(line+7)); continue; }
        if(!strcmp(line,"reboot")) { reboot(); }
        if(!strncmp(line,"checksum ",9)) { checksum(line+9); continue; }
		if(!strncmp(line,"team ",5)) { dumpteam(readnum(line+5)); continue; }
		if(!strncmp(line,"find ",5)) { findpage(readnum(line+5)); continue; }
		if(!strcmp(line,"teams")) { dumpteams(); continue; }
		
        if(!strcmp(line,"exit")) break;
        if(!strcmp(line,"x")) break;
        if(!strcmp(line,"c")) break;
    }
}

void DEBUGGER(void)
{
	regs r;
	k_debugger(&r, 0, 0, 0);
}

#endif   
