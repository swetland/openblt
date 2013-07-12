/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "types.h"
#include "i386.h"
#include "aspace.h"
#include "task.h"
#include "port.h"
#include "sem.h"
#include "team.h"

#include <blt/types.h>
#include <blt/error.h>

void panic(char *reason);

task_t *new_thread(team_t *team, uint32 ip, int kernel);

int brk(uint32 addr);

void destroy_kspace(void);

/* debugger functions */
void kprintf_init(void);
void kprintf(const char *fmt, ...);
char *kgetline(char *line, int maxlen);
void krefresh(void);

#ifdef SERIAL
void dprintf_init(void);
void dprintf(const char *fmt, ...);
#endif

void preempt(task_t *t, int status);
void swtch(void);
extern char *idt, *gdt;
extern uint32 _cr3;

extern int kernel_timer;

extern task_t *current;
extern resource_t *run_queue;
extern resource_t *reaper_queue;
extern resource_t *timer_queue;
extern team_t     *kernel_team;

void user_debug(regs *r, uint32 *eip, uint32 *eflags);

#endif
