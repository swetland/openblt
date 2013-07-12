/* $Id: //depot/blt/srv/console2/console.c#15 $
**
** Copyright 1998 Brian J. Swetland
** All rights reserved.
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

#include <blt/syscall.h>
#include <blt/error.h>
#include <blt/namer.h>
#include <blt/conio.h>
#include <blt/qsem.h>
#include <i386/io.h>
#include <string.h>

#include "vt100.h"

int console_port = -1;
int send_port = -1;
int remote_port = -1;

volatile int ready = 0;

#define CLEAR "\033[2J"
#define FG_BLACK  "\033[30m"
#define FG_BLUE   "\033[31m"
#define FG_GREEN  "\033[32m"
#define FG_CYAN   "\033[33m"
#define FG_RED    "\033[34m"
#define FG_PURPLE "\033[35m"
#define FG_BROWN  "\033[36m"
#define FG_WHITE  "\033[37m"

#define BG_BLACK  "\033[40m"
#define BG_BLUE   "\033[41m"
#define BG_GREEN  "\033[42m"
#define BG_CYAN   "\033[43m"
#define BG_RED    "\033[44m"
#define BG_PURPLE "\033[45m"
#define BG_BROWN  "\033[46m"
#define BG_WHITE  "\033[47m"   


#define MONOx

#ifdef MONO
#define SCREEN 0xB0000
#else
#define SCREEN 0xB8000
#endif
void *screen = (void *) SCREEN;

void movecursor (int x, int y)
{
	int offset;

	offset = 80 * y + x;
	outb (0xe, 0x3d4);
	outb (offset / 256, 0x3d5);
	outb (0xf, 0x3d4);
	outb (offset % 256, 0x3d5);
}

struct virtscreen con[10];
struct virtscreen statbar;
struct virtscreen *active;

void move_cursor(struct virtscreen *cur)
{
	if(cur == active) movecursor(cur->xpos,cur->ypos);
}

void vprintf(struct virtscreen *vscr, char *fmt, ...);
void printf(char *fmt, ...);

void keypress(int key)
{
	char c;
	
	sem_acquire(active->lock);
	char_to_virtscreen(active, key);
	sem_release(active->lock);	
	if(remote_port > 0) {
		c = key;
		port_send(send_port, remote_port, &c, 1, 0);
	}
}

void vputs(struct virtscreen *vscr, char *s)
{
    sem_acquire(vscr->lock);
	while(*s) {
		char_to_virtscreen(vscr, *s);
        if(*s == '\n') char_to_virtscreen(vscr, '\r');
		s++;
	}
    sem_release(vscr->lock);
}


void printf(char *fmt, ...)
{
    static char line[128];
    va_list pvar;
    va_start(pvar,fmt);
    va_snprintf(line,128,fmt,pvar);
    line[127]=0;
    va_end(pvar);
    vputs(active,line);
}

void vprintf(struct virtscreen *vscr, char *fmt, ...)
{
    static char line[128];
    va_list pvar;
    va_start(pvar,fmt);
    va_snprintf(line,128,fmt,pvar);
    line[127]=0;
    va_end(pvar);
    vputs(vscr,line);
}
                                    
void status(int n)
{
	vprintf(&statbar,FG_WHITE BG_BLUE CLEAR "### OpenBLT Console mkII ### VC %d",n);
}

#define ESC 27
#define BS 8
#define TAB 9
#define CR 13
#define LF 10

char ScanTable [] =  {' ', ESC, '1', '2', '3', '4', '5', '6', '7', '8',
                      '9', '0', '-', '=', BS,  TAB, 'q', 'w', 'e', 'r',
                      't', 'y', 'u', 'i', 'o', 'p', '[', ']', LF,  ' ',
                      'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
                      '\'', '`', ' ', '\\', 'z', 'x', 'c', 'v', 'b', 'n',
                      'm', ',', '.', '/', ' ', ' ', ' ', ' ', ' '};
char ShiftTable [] = {' ', ESC, '!', '@', '#', '$', '%', '^', '&', '*',
                      '(', ')', '_', '+', ' ', ' ', 'Q', 'W', 'E', 'R',
                      'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', LF,  ' ',
                      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
                      '\"', '~', ' ', '|', 'Z', 'X', 'C', 'V', 'B', 'N',
                      'M', '<', '>', '?', ' ', ' ', ' ', ' ', ' '};
#define LSHIFT 42
#define RSHIFT 54
#define CONTROL 0x1d
#define ALT 0x38

#define F1  0x3b
#define F2  0x3c
#define F3  0x3d
#define F4  0x3e
#define F5  0x3f
#define F6  0x40
#define F7  0x41
#define F8  0x42
#define F9  0x43
#define F10 0x44

void movecursor (int x, int y);
void keypress(int key);

void save(struct virtscreen *vscr)
{
    sem_acquire(vscr->lock);
    if(vscr->data != vscr->back) {
        memcpy(vscr->back,vscr->data,vscr->num_bytes);
        vscr->data = vscr->back;
    }
    sem_release(vscr->lock);
}

void load(struct virtscreen *vscr)
{
    sem_acquire(vscr->lock);
    if(vscr->data == vscr->back) {
        vscr->data = screen;
        memcpy(vscr->data,vscr->back,vscr->num_bytes);
    }
    active = vscr;
    movecursor(vscr->xpos,vscr->ypos);
    sem_release(vscr->lock);
    status(vscr - con);
}

void function(int number)
{
    save(active);
    active = &con[number];
    load(active);
}

void keyboard_irq_thread(void)
{
    int shift = 0;    
	int control = 0;
	int alt = 0;
	
    int key;

    os_handle_irq(1);
	
    for(;;) {
        os_sleep_irq();
#ifdef MULTI
        while(inb(0x64) & 0x01) {
#endif
            key = inb(0x60);
            if(alt && (key == 1)) {
                save(active);
                os_debug();
                load(active);
                alt = 0;
                continue;
			}
			
            switch(key){
            case F1:
            case F2:
            case F3:
            case F4:
            case F5:
            case F6:
            case F7:
            case F8:
            case F9:
            case F10:
                function(key - F1);
                break;
                
			case ALT:
				alt = 1;
				break;
			case ALT | 0x80:
				alt = 0;
				break;
			case CONTROL:
				control = 1;
				break;
			case CONTROL | 0x80:
				control = 0;
				break;
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
						if(control){
							key = ScanTable[key];
							if(key >= 'a' && key <= 'z'){
								keypress(key - 'a' + 1);
							}
						} else {
							key = shift ? ShiftTable[key] : ScanTable[key];
							keypress(key);
						} 
                    }
                }
            }
#ifdef MULTI
        }
#endif
    }

}

void console_thread(void)
{
    int l, src;
	uint32 code;
    char data[257];
        
#ifdef CONSOLE_DEBUG
    vprintf(active, "console: " FG_GREEN "listener ready" FG_WHITE " (port %d)\n",console_port);
#endif
    
	while((l = port_recv(console_port, &src, data, 256, &code)) >= 0){
		if(code == 0) {
			remote_port = src;			
		} else {
			data[l] = 0;
			vputs(active, data);
		}
    }
    vprintf(active, "console: output listener has left the building (%d)\n",l);
    os_terminate(0);
}

int console_main(void)
{
    int err,i;    
	area_create(0x2000, 0, &screen, AREA_PHYSMAP);
    
    console_port = port_create(0,"console_listen_port");
    send_port = port_create(0,"console_send_port");
	port_option(send_port, PORT_OPT_NOWAIT, 1);
	
    err = namer_register(console_port,"console");

	init_virtscreen(&statbar, 1, 80);
    statbar.back = statbar.data;
	statbar.data = (unsigned short *) (((uint32) screen) + 80*24*2);    
    statbar.lock = sem_create(1,"statbar_lock");
    
    for(i=0;i<10;i++){
        init_virtscreen(&con[i], 24, 80);
        con[i].back = con[i].data;
        con[i].lock = sem_create(1,"vscr_lock");
        vputs(&con[i],CLEAR);
    }
    load(&con[0]);
	
    status(0);
    if(err) vprintf(active,"console: the namer hates us\n");
    else
#ifdef CONSOLE_DEBUG
	    vprintf(active,"console: " FG_GREEN "online." FG_WHITE "\n");
#else
        ;
#endif
    
	thr_create(keyboard_irq_thread, NULL, "console:kbd");
    ready = 1;
    console_thread();
    
    return 0;
}

int main(void)
{
	thr_create(console_main, NULL, "console:main");
	while(!ready) ;
	return 0;
}

