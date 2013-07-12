/* Copyright 1998-2000 Brian J. Swetland.  All rights reserved.
** Distributed under the terms of the OpenBLT License.
*/

#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <blt/types.h>
#include <blt/os.h>

#ifdef __cplusplus
extern "C" {
#endif

	void  os_terminate(int status);
	void  os_console(char *string);
	int   os_brk(int addr);
	void  os_handle_irq(int irq);
	void  os_sleep_irq(void);
	void  os_debug(void);
	void  os_sleep(int ticks);
	int   os_identify(int rsrc); /* returns the team_id of the owner of a resource */
	
	sem_id sem_create(int value, const char *name);
	int sem_destroy(sem_id sem);
	int sem_acquire(sem_id sem);
	int sem_release(sem_id sem);

	port_id port_create(int restrict, const char *name);
	int port_destroy(port_id port);
	int port_option(port_id port, uint32 opt, uint32 arg);
	ssize_t port_send(port_id src, port_id dst, const void *data, size_t len, uint32 code);
	ssize_t port_recv(port_id dst, port_id *src, void *data, size_t max, uint32 *code);
	ssize_t port_peek(port_id *src, port_id *dst, int *code);
	
	#define port_set_restrict(port, restrict) port_option(port,PORT_OPT_SETRESTRICT,restrict);
	#define port_slave(master, slave) port_option(slave,PORT_OPT_SLAVE,master)
	#define port_set_nonblocking(port) port_option(port, PORT_OPT_NOWAIT, 1);
	#define port_set_blocking(port) port_option(port, PORT_OPT_NOWAIT, 0);
	#define port_set_timeout(port, useconds)
		
	thread_id thr_create(void *addr, void *data, const char *name);
	int thr_resume(thread_id thr_id);
	int thr_suspend(thread_id thr_id);
	int thr_kill(thread_id thr_id);
	int thr_detach(thread_id (*addr)(void));
	int thr_wait(thread_id thr_id);
	
	thread_id thr_spawn(uint32 eip, uint32 esp,
				  area_id area0, uint32 vaddr0,
				  area_id area1, uint32 vaddr1,
				  const char *name);
	
	area_id area_create(off_t size, off_t virt, void **addr, uint32 flags);
	area_id area_clone(area_id aid, off_t virt, void **addr, uint32 flags);
	int area_destroy(area_id aid);
	int area_resize(area_id aid, off_t size);   
	
	int right_create(int rsrc_id, uint32 flags);
	int right_destroy(int right_id);
	int right_revoke(int right_id, int thread_id); 
	int right_grant(int right_id, int thread_id);
	
	/* look up a resource by name or id number and fill in information about it */
	int rsrc_find_id (rsrc_info *info, int rsrc_id, int rsrc_type);
	int rsrc_find_name (rsrc_info *info, const char *name, int rsrc_type);
	
	/*
	 * the metacall is for random things that are not compiled into the kernel
	 * by default, but might be useful for debugging, etc.  if you think you
	 * need to add a syscall temporarily in order to debug something, using
	 * this will save you some time, since you only have to edit two files
	 * instead of four.
	 *
	 * the request parameter is used to multiplex the call among many different
	 * purposes.  the range META_MIN_RESERVED to META_MAX_RESERVED is for
	 * temporary purposes that are not submitted to the repository.  request
	 * defines are in include/blt/os.h.
	 */
	
	int os_meta(unsigned int request, ...);

#ifdef __cplusplus
}
#endif

/* compatability defines */
#define os_thread(addr) thr_create(addr,NULL,NULL);
#define thr_join(thr_id,opt) thr_wait(thr_id);
#define thr_detach(addr) (0)

/* deprecated messaging system */
typedef struct {
    int flags;
    int src;
    int dst;
    int size;
    void *data;    
} msg_hdr_t;

#define old_port_send(mh) port_send((mh)->src, (mh)->dst, (mh)->data, (mh)->size, 0)
#define old_port_recv(mh) port_recv((mh)->dst, &((mh)->src), (mh)->data, (mh)->size, 0)

#endif
