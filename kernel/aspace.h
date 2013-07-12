/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _ASPACE_H_
#define _ASPACE_H_

#include "resource.h"
#include "list.h"

#define SEL_KCODE  (1*8)
#define SEL_KDATA  (2*8)
#define SEL_KDATA2 (3*8)
#define SEL_UCODE  (4*8)
#define SEL_UDATA  (5*8)
#define SEL_KTSS   (6*8)
#define SEL_UTSS   (7*8)

typedef struct __phys_page_t {
    struct __phys_page_t *next;
    uint32 refcount;
    uint32 addr[6];
} phys_page_t; /* 32 bytes */

typedef struct __pagegroup_t {
    uint32 size;           /* number of pages */
    uint32 refcount;       /* number of areamaps */
    uint32 flags;          /* page info */
	list_t areas;          /* areas sharing this pagegroup */
    phys_page_t *pages;    /* physical pages that compose this pagegroup */
} pagegroup_t;  
        
struct __area_t {
    resource_t rsrc;
    pagegroup_t *pgroup;
    uint32 virt_addr;     /* virtual PAGE address */
    uint32 length;        /* length in pages */
    uint32 maxlength;     /* maxlength in pages */
}; 

struct __aspace_t {
    resource_t rsrc;
    list_t areas;         /* list of mapped areas            */
    uint32 maxvirt;       /* highest virt addr pagetabs maps */
        /* platform specific */
    uint32 *pdir;         /* page directory  -- 4k */
    uint32 *ptab;         /* page table      -- 4k */
    uint32 *high;         /* high page table -- 4k (0x8000000 -> ) */
	uint32 pdirphys;      /* physical address of page directory */
};


aspace_t *aspace_create(void);
void aspace_destroy(aspace_t *a);
void aspace_map(aspace_t *a, uint32 phys, uint32 virt,
                uint32 len, uint32 flags);
void aspace_maphi(aspace_t *a, uint32 phys, uint32 virt,
                  uint32 len, uint32 flags);
void aspace_clr(aspace_t *a, uint32 virt, uint32 len);
void aspace_print(aspace_t *a);
void aspace_protect(aspace_t *a, uint32 virt, uint32 flags);

/* userland stuff */
int area_create(aspace_t *aspace, off_t size, off_t virt, void **addr, uint32 flags);
int area_create_uber(off_t size, void *addr);
int area_destroy(aspace_t *aspace, int area_id);
int area_clone(aspace_t *aspace, int area_id, off_t virt, void **addr, uint32 flags);
int area_destroy(aspace_t *aspace, int area_id);
int area_resize(aspace_t *aspace, int area_id, off_t size);   


#endif
