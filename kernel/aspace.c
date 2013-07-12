/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <blt/os.h>
#include "kernel.h"
#include "memory.h"
#include "aspace.h"

extern uint32 _cr3;
extern aspace_t *flat;

int snprintf (char *s, int len, const char *fmt, ...);

void aspace_pprint(uint32 *page, uint32 vbase)
{
    int i,j;

    char buf[80];
    
    for(i=0;i<1024;i++){
        if(page[i]){
            j = page[i];
            snprintf(buf,80,"%s %s %s ",
                    (j & 4) ? "USR" : "SYS",
                    (j & 2) ? "RW" : "RO",
                    (j & 1) ? "P" : "X");

            snprintf(buf+9,80-9,"%xv  %xp", i*4096+vbase, j&0xFFFFFF00);
            
            for(j=1;(i+j < 1024) &&
                    ( (0xFFFFFF00 & page[i+j]) ==
                      (0xFFFFFF00 & page[i+j-1])+0x1000 );j++);
            
            if(j>1) { 
                i += j-1;
                snprintf(buf+29,80-29," - %xp (%x)",
                        0x0FFF + (page[i]&0xFFFFFF00), j);
            }
            kprintf(buf);
        }
    }
}

void aspace_kprint(aspace_t *a)
{
    aspace_pprint(a->high,0x80000000);       
}

void aspace_print(aspace_t *a)
{
    aspace_pprint(a->ptab,0);        
}

aspace_t *aspace_create(void) 
{
    int i;
    uint32 phys0,phys1;
	uint32 *raw0,*raw1;
    
    aspace_t *a = kmalloc(aspace_t);
	raw0 = kgetpage(&phys0);
	raw1 = kgetpage(&phys1);
	a->pdir = raw0;
	a->ptab = raw1;
	a->high = flat->high;
	a->pdirphys = phys0;
	list_init(&a->areas);
	
    for(i=0;i<1024;i++){
        a->pdir[i] = 0;
        a->ptab[i] = 0;
    }
    a->pdir[0] = phys1 | 7;
    a->pdir[512] = (_cr3 + 2*4096) | 3;
	rsrc_bind(&a->rsrc, RSRC_ASPACE, kernel_team);
    return a;
}

void aspace_protect(aspace_t *a, uint32 virt, uint32 flags)
{
    a->ptab[virt] = ((a->ptab[virt] & 0xFFFFF000) | (flags & 0x00000FFF));    
}

void aspace_map(aspace_t *a, uint32 phys, uint32 virt, uint32 len, uint32 flags)
{
    int i;
    for(i=0;i<len;i++){
        a->ptab[virt+i] = (((phys+i)*4096) & 0xFFFFF000) |
            (flags & 0x00000FFF);
        local_flush_pte(0x1000*((virt)+i));
    }    
}

void aspace_maphi(aspace_t *a, uint32 phys, uint32 virt, uint32 len, uint32 flags)
{
    int i;
    for(i=0;i<len;i++){
        a->high[(virt&0x3FF)+i] = (phys+i)*4096 | flags;
        local_flush_pte(0x1000*((virt)+i));
    }
    
}

void aspace_clr(aspace_t *a, uint32 virt, uint32 len)
{
    int i;
    for(i=0;i<len;i++){
        a->ptab[virt+i] = 0;
	}
}

void aspace_destroy(aspace_t *a)
{
	area_t *area;
		
	/* tear down all attached areas */
	while((area = (area_t*) list_peek_head(&a->areas))){
		area_destroy(a, area->rsrc.id);
	}
		
	rsrc_release(&a->rsrc);
	/* release page directory and table(s) */
	kfreepage(a->pdir);
	kfreepage(a->ptab);
	kfree(aspace_t, a);
}

/* find a span of contig virtual pages */
static int locate_span(aspace_t *a, uint32 start, uint32 len)
{
    uint32 *map = a->ptab;
    uint32 found = 0;
    uint32 foundat = 0;
    
    /* default to 2mb line if unspec or invalid */
    if((start < 1) || (start > 1023)) start = 512;
    
    while((found < len) && (start < 1024)){
        if(map[start]){
            found = 0;
            foundat = 0;
        } else {
            if(found == 0) foundat = start;
            found++;
        }
        start++;
    }
    return foundat;
}

/* userland calls */
int area_create(aspace_t *aspace, off_t size, off_t virt, void **addr, uint32 flags)
{
    int ppo,i,p=0,at;
    area_t *area;
    pagegroup_t *pg;
    phys_page_t *pp;
    size = (size & 0x0FFF) ? (size / 0x1000 + 1) : (size / 0x1000);
    
    if(size < 1) size = 1;

    if(flags & AREA_PHYSMAP) {
        p = ((uint32) *addr) / 0x1000;
    } 
    
    /* find a virtaddr */
    if(!(at = locate_span(aspace, virt/0x1000, size))){
        return ERR_MEMORY;
    }
    
    /* create a fresh pagegroup and enough phys_page_t's */    
    pg = (pagegroup_t *) kmalloc(pagegroup_t);
    pg->flags = flags;
    pg->refcount = 1;
    pg->size = size;
    pg->pages = NULL;
	list_init(&pg->areas);
    for(i=0;i<size;i+=6){
        pp = (phys_page_t *) kmalloc(phys_page_t);
        pp->refcount = 0;
        pp->next = pg->pages;
        pg->pages = pp;
    };
    
    /* create an area to ref the pagegroup */
    area = (area_t *) kmalloc(area_t);
    area->pgroup = pg;
    area->virt_addr = at;
    area->length = size;
    area->maxlength = size;
    
    /* link this area into the new pagegroup */
	list_add_tail(&pg->areas, area);
    
    /* link this area into the aspace's arealist */
	list_add_tail(&aspace->areas, area);	

    /* check for valid ptr */
    *addr = (void *) (at * 0x1000);
        
    /* allocate pages, fill phys_page_t's, and map 'em */
    for(ppo=0,pp=pg->pages,i=0;i<size;i++){
        if(!(flags & AREA_PHYSMAP)) {
            p = getpage();
        }
        aspace_map(aspace, p, at, 1, 7);
        pp->addr[ppo] = p;
        ppo++;
        if(ppo == 6) {
            ppo = 0;
            pp = pp->next;
        }
        if(flags & AREA_PHYSMAP) p++;
        at++;
    }
    
    /*
	 * zero extra space in last phys_page_t.
	 * careful, ppo == 0 and pp == NULL if size == 6.
	 */
    while((ppo < 6) && (pp != NULL)){
        pp->addr[ppo] = 0;
        ppo++;
    }
    
    /* bind the resource and return the id */
    rsrc_bind(&(area->rsrc), RSRC_AREA, current->rsrc.owner);
    return area->rsrc.id;
}

int area_create_uber(off_t size, void *addr)
{
    int ppo,i,p,at=0;
    area_t *area;
    pagegroup_t *pg;
    phys_page_t *pp;

    size = (size & 0x0FFF) ? (size / 0x1000 + 1) : (size / 0x1000);
    if(size < 1) size = 1;

    p = ((uint32) addr) / 0x1000;
    
    /* create a fresh pagegroup and enough phys_page_t's */    
    pg = (pagegroup_t *) kmalloc(pagegroup_t);
    pg->flags = AREA_PHYSMAP;
    pg->refcount = 1;
    pg->size = size;
    pg->pages = NULL;
	list_init(&pg->areas);
    for(i=0;i<size;i+=6){
        pp = (phys_page_t *) kmalloc(phys_page_t);
        pp->refcount = 0;
        pp->next = pg->pages;
        pg->pages = pp;
    };
    
    /* create an area to ref the pagegroup */
    area = (area_t *) kmalloc(area_t);
    area->pgroup = pg;
    area->virt_addr = at;
    area->length = size;
    area->maxlength = size;
    
    /* link this area into the new pagegroup */
	list_add_tail(&pg->areas, area);    
    
    /* allocate pages, fill phys_page_t's, and map 'em */
    for(ppo=0,pp=pg->pages,i=0;i<size;i++){
        pp->addr[ppo] = p;
        ppo++;
        if(ppo == 6) {
            ppo = 0;
            pp = pp->next;
        }
        p++;
        at++;
    }
    
    /*
	 * zero extra space in last phys_page_t.
	 * careful, ppo == 0 and pp == NULL if size == 6.
	 */
    while((ppo < 6) && (pp != NULL)){
        pp->addr[ppo] = 0;
        ppo++;
    }
    
    /* bind the resource and return the id */
    rsrc_bind(&(area->rsrc), RSRC_AREA, kernel_team);
    return area->rsrc.id;
}

int area_clone(aspace_t *aspace, int area_id, off_t virt, void **addr, uint32 flags)
{
    area_t *area0, *area1;
    uint32 at, size;
    phys_page_t *pp;
    int i,ppo;
    
    /* make sure the area exists */
    /* xxx : check perm */
    if(!(area0 = rsrc_find_area(area_id))) return ERR_RESOURCE;
	
    /* find virt space for it */
    if(!(at = locate_span(aspace, virt/0x1000, area0->length))){
        return ERR_MEMORY;
    }

	
/* xxx lock area1 and area1->pgroup */
        
    /* allocate the clone area and init it from the orig */
    area1 = (area_t *) kmalloc(area_t);
    area0->pgroup->refcount++;
    area1->pgroup = area0->pgroup;
    size = area1->length = area0->length;
    area1->maxlength = area0->maxlength;
    area1->virt_addr = at;
	
    /* link this area into the new pagegroup */
	list_add_tail(&area0->pgroup->areas, area1);
   
    /* link this area into the aspace's arealist */
	list_add_tail(&aspace->areas, area1);

    /* check for valid ptr */
    *addr = (void *) (at * 0x1000);
        
    /* map the phys_page_t's into the clone area */
    for(ppo=0,i=0,pp=area1->pgroup->pages;i<size;i++){
        aspace_map(aspace, pp->addr[ppo], at, 1, 7);
        at++;
        ppo++;
        if(ppo == 6){
            ppo = 0;
            pp = pp->next;
        }
    }
    /*
    kprintf("area0 pgroup %x",area1->pgroup);
    kprintf("area1 pgroup %x",area1->pgroup);
    */
    /* bind and return an id */
    rsrc_bind(&(area1->rsrc), RSRC_AREA, current->rsrc.owner);
//	kprintf("area_clone(%d) -> %d",area_id,area1->rsrc.id);
    return area1->rsrc.id;
}

int area_destroy(aspace_t *aspace, int area_id)
{
	area_t *area;
	pagegroup_t *pg;
	
    if(!(area = rsrc_find_area(area_id))) return ERR_RESOURCE;
	
	/* find and unchain the area from its aspace -- complain if it is foreign */
	if(list_remove(&aspace->areas, area)) return ERR_RESOURCE;
	
	/* unmap the memory */
	aspace_clr(aspace, area->virt_addr, area->length);
	
	pg = area->pgroup;
	
	/* remove this area from the pgroup's area list */
	list_remove(&pg->areas, area);
	
	/* decr the pagegroup refcount and tear it down if zero */
	pg->refcount--;
	
	if(pg->refcount == 0){
		int release = !(pg->flags & AREA_PHYSMAP);
		int count = 0;
		phys_page_t *pp;

		while((pp = pg->pages)){
			pg->pages = pp->next;
			if(release){
				for(count=0;pg->size && (count < 6);count++){
					putpage(pp->addr[count]);
					pg->size--;
				}
			}
			kfree(phys_page_t, pp);
		}	
		kfree(pagegroup_t, pg);
	}

	rsrc_release(&area->rsrc);
	
	kfree(area_t, area);
	return ERR_NONE;
}

int area_resize(aspace_t *aspace, int area_id, off_t size)
{
    area_t *area;
    pagegroup_t *pg;
    int ppo;
    phys_page_t *pp,*pp0;
    uint32 at,p;
    
    if(!(area = rsrc_find_area(area_id))) return ERR_RESOURCE;
    
    pg = area->pgroup;
    pp = area->pgroup->pages;
    
//	kprintf("area_resize(%x,%d,%d)",aspace,area_id,size);
	    
    if(size <= pg->size*0x1000) return 0;

    size = size/0x1000 - pg->size; /* pages to add */

//    kprintf("area_resize: %x %x %x + %x",area,pg,pp,size);

    at = locate_span(aspace, area->virt_addr + pg->size, size);
    if(at != (area->virt_addr + pg->size)) {
        kprintf("oops: cannot grow area (%x != %x)",at,area->virt_addr+pg->size);
        return ERR_MEMORY;
    }
    
    while(pp->next) pp = pp->next;
    ppo = pg->size % 6;
    
    pg->size += size;
    area->length += size;
    area->maxlength += size;
    
    while(size){
        if(!ppo){
            pp0 = (phys_page_t *) kmalloc(phys_page_t);
            pp0->next = NULL;
            pp->next = pp0;
            pp0->refcount = 0;
            pp = pp0;
        }
        p = getpage();
        aspace_map(aspace, p, at, 1, 7);
        pp->addr[ppo] = p;
        at++;
        ppo++;
        if(ppo == 6) ppo = 0;
        size--;
    }
    
    return 0;
}






