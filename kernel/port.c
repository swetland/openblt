/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include "kernel.h"    
#include "memory.h"
#include "port.h"
#include "resource.h"

#define MAX_MSGCOUNT    16

int snprintf (char *s, int len, char *fmt, ...);

int port_create(int restrict, const char *name)
{
    port_t *p;
	char qname[16];
	
        /* create new port */
    p = kmalloc(port_t);
    p->sendqueue = kmalloc(resource_t);
	
    p->msgcount = 0;
    p->first = p->last = NULL;
    p->slaved = 0;
    p->refcount = 1;
	p->nowait = 0;
	p->restrict = restrict;
	
	rsrc_bind(&p->rsrc, RSRC_PORT, current->rsrc.owner);
	rsrc_bind(p->sendqueue, RSRC_QUEUE, current->rsrc.owner);
	rsrc_set_name(&p->rsrc, name);
	snprintf(qname,16,"port:%d",p->rsrc.id);
	rsrc_set_name(p->sendqueue, name);
    return p->rsrc.id;
}

int port_destroy(int port)
{
    port_t *p;
    if(!(p = rsrc_find_port(port))) return ERR_RESOURCE;
//    if(p->rsrc.owner != current) return ERR_PERMISSION;

    if(p->refcount == 1) {                
            /* destroy port */
		rsrc_release(&p->rsrc);
		rsrc_release(p->sendqueue);
		kfree(resource_t,p->sendqueue);
        kfree(port_t,p); 
        return ERR_NONE;    
    }

        /* port is the master of one or more slaves */
    return ERR_RESOURCE;       
}

uint32 port_option(uint32 port, uint32 opt, uint32 arg)
{
    port_t *p;
    
    if(!(p = rsrc_find_port(port))) return ERR_RESOURCE;
    if(p->rsrc.owner != current->rsrc.owner) return ERR_PERMISSION;

    if(opt == PORT_OPT_SETRESTRICT){
        p->restrict = arg;
        return ERR_NONE;        
    }

    if(opt == PORT_OPT_SLAVE){
        port_t *master;

        if(arg){
                /* arg == 0 will simply release the old master */
            
            if(!(master = rsrc_find_port(arg))) return ERR_RESOURCE;
            if(master->rsrc.owner != current->rsrc.owner) return ERR_PERMISSION;
            
                /* indicate that our master has one more slave */
            master->refcount++;
        }
        
        if(p->slaved){
                /* change in slave status, deref our old master */
            if(!(master = rsrc_find_port(p->slaved)))
                panic("port_option(): master port not found?");
            
            master->refcount--;            
        }
        p->slaved = arg;
        return ERR_NONE;        
    }

	if (opt == PORT_OPT_NOWAIT) {
		p->nowait = arg;
		return ERR_NONE;
	}

    return ERR_PERMISSION;
}

static chain_t *msg_pool = NULL;


int port_send(int src, int dst, void *msg, size_t size, uint32 code)
{
    int i;
    message_t *m;
    port_t *f,*p;
    
    if(!(f = rsrc_find_port(src))) return ERR_SENDPORT;
#if 0
 	if(f->rsrc.owner != current) {
        task_t *t = current->rsrc.owner; /* XXX */
        while(t){
            if(t == f->rsrc.owner) break;
            t = t->rsrc.owner;
            
        }
        if(!t) return ERR_PERMISSION;
    }
#endif
        /* insure the port exists and we may send to it */
    if(!(p = rsrc_find_port(dst))) return ERR_RECVPORT;
/*    if((p->restrict) &&
       (p->restrict != src)) return ERR_PERMISSION; XXX */

        /* are we slaved to a different port? */
    if(p->slaved){
        if(!(p = rsrc_find_port(p->slaved))) return ERR_RESOURCE;
        if(p->slaved) return ERR_RESOURCE;
/*        if((p->restrict) &&
           (p->restrict != src)) return ERR_PERMISSION;  XXX */      
    }

	while(p->msgcount >= MAX_MSGCOUNT){
		int status;
		if(p->nowait) return ERR_WOULD_BLOCK;
		if((status = wait_on(p->sendqueue))) return status;
	}

        /* ignore invalid sizes/locations */
    if( (((uint32) msg) > 0x400000) ||
        (size > 4096)) return ERR_MEMORY;    

    m = kmalloc(message_t);
    
        /* allocate a 4k page to carry the message. klunky... */
    if(size < 256){
        m->data = size ? kmallocB(size) : NULL;        
    } else {
        if(msg_pool){
            m->data = (void *) msg_pool;
            msg_pool = (chain_t *) msg_pool->next;        
        } else {
            m->data = kgetpages(1);
        }
    }

    for(i=0;i<size;i++){
        ((unsigned char *) m->data)[i] = *((unsigned char *) msg++);
	}
	
    m->from_port = src;
    m->to_port = dst;    
	m->code = code;
    m->size = size;
    m->next = NULL;
    if(p->last){
        p->last->next = m;
    } else {
        p->first = m;
    }
    p->last = m;
    p->msgcount++;


	/* If a thread is sleeping on the destination, wake it up
	*/	
	if(p->slaved){
		port_t *p0 = rsrc_find_port(p->slaved);
		if(p0){
			task_t *task = rsrc_dequeue(&p0->rsrc);
			if(task) task_wake(task,ERR_NONE);
		}
	} else {
		task_t *task = rsrc_dequeue(&p->rsrc);
		if(task) task_wake(task,ERR_NONE);
	}	
    
    return size;
}

/*int old_port_recv(int port, void *msg, int size, int *from)*/
int port_recv(int dst, int *src, void *msg, size_t size, uint32 *code)
{
    int i;
    message_t *m;
    port_t *p;

        /* insure the port exists and we may receive on it */
    if(!(p = rsrc_find_port(dst))) return ERR_RECVPORT;
#if 0
	    if(p->rsrc.owner != current) return ERR_PERMISSION;
#endif
 
        /* bounds check the message... should be more careful */
    if((current->team != kernel_team) && (((uint32) msg) > 0x400000))
		return ERR_MEMORY;

        /* no messages -- sleep */
    while(!p->msgcount) {
		int status;
        if (p->nowait) return ERR_WOULD_BLOCK;
		if((status = wait_on(&p->rsrc))) return status;
    }
    
    m = p->first;
    for(i=0;i<m->size && (i <size);i++){
        *((unsigned char *) msg++) = ((unsigned char *) m->data)[i];
    }
    if(src) *src = m->from_port;    
	if(code) *code = m->code;
    dst = m->to_port; // XXX
    
        /* unchain from the head of the queue */
    if(!(p->first = p->first->next)) p->last = NULL;    

	if(p->sendqueue->queue.count && (p->msgcount <= MAX_MSGCOUNT)) {
		task_t *task = rsrc_dequeue (p->sendqueue);
		if(task) task_wake(task, ERR_NONE);
	}
	p->msgcount--;

        /* add to the freepool */
    if(m->size < 256){
        if(m->size) kfreeB(m->size,m->data);
    } else {
        ((chain_t *) m->data)->next = msg_pool;
        msg_pool = ((chain_t *) m->data);
    }
    kfree(message_t,m);
    return size < m->size ? size : m->size;
}

