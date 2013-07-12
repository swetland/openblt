/* Copyright 1998-1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

/* serial tracing */

#include <stdarg.h>
#include "memory.h"
#include <i386/io.h>
#include <blt/conio.h>
#include "kernel.h"

#define NULL ((void *) 0)

#define com1 0x3f8 
#define com2 0x2f8

#define combase com1

char *kprintf_lock = 0;

void va_snprintf(char *b, int l, const char *fmt, va_list pvar);

/* catch bad configurations */
#if defined (SERIAL_DEBUG) && defined (DPRINTF)
#error cannot use both serial debugging and dprintf
#endif

#if (defined (SERIAL_DEBUG) || defined (DPRINTF)) && !defined (SERIAL) && !defined (PORT_E9)
#error cannot use serial debugging or dprintf without serial port code
#endif

#ifdef PORT_E9
/* Bochs has this special direct to console thing */

void dprintf_init(void)
{
}

static int ser_getc(void)
{
	for(;;) ;
}

static void ser_putc(int ch)
{
    while (!(inb(combase + 5) & 0x20));
    outb((unsigned char) ch, 0xe9);
}

static void ser_puts(char *s)
{
    int t;
    while(*s){
        ser_putc(*s);
        s++;
    }
}

#endif

#ifdef SERIAL

void dprintf_init(void)
{
	outb(0, combase + 4);
    outb(0, combase + 0);
    outb(0x83, combase + 3);
    outb(6, combase);                           /* 9600 bps, 8-N-1 */
    outb(0, combase+1);
    outb(0x03, combase + 3);
}

static int ser_getc(void)
{
    while (!(inb(combase + 5) & 0x01));
    return inb(combase);
}

static void ser_putc(int ch)
{
    while (!(inb(combase + 5) & 0x20));
    outb((unsigned char) ch, combase);
}

static void ser_puts(char *s)
{
    int t;
    while(*s){
        ser_putc(*s);
        s++;
    }
}

#endif

#ifdef SERIAL_DEBUG
void krefresh(void)
{
}

unsigned char *screen = NULL;

void kprintf_init(void)
{
	screen = (unsigned char *) kmappages(
#ifdef MONO
										 0xB0, 
#else 
										 0xB8, 
#endif
										 2, 3);
}

char *kgetline(char *line, int len)
{
    char c;
    int pos = 0;

    ser_puts(": ");
    
    for(;;){
        switch(c = ser_getc()){
        case 10:
        case 13:
            line[pos]=0;
            ser_puts("\r\n");
            return line;
            
        case 8:
            if(pos) {
                pos--;
                ser_puts("\b \b");
            }
            break;
            
        case 27:
            while(pos) {
                pos--;
                ser_puts("\b \b");
            }
            break;

        default:
            if((c >= ' ') && (c < 0x7f) && (pos < len-1)){
                line[pos] = c;
                pos++;
                ser_putc(c);
            }
        }
    }
}

static char Line[128];
void kprintf(const char *fmt, ...)
{
    va_list pvar;    
    va_start(pvar,fmt);
#ifdef __SMP__
	p (&kprintf_lock);
#endif
    va_snprintf(Line,128,fmt,pvar);
    Line[127]=0;
    va_end(pvar);
    ser_puts(Line);
    ser_puts("\r\n");
#ifdef __SMP__
	v (&kprintf_lock);
#endif
}

#else

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

unsigned char *screen = NULL;
static unsigned char vscreen[80*25*2];

void *kmappages(int phys, int count, int flags);

void kprintf_init()
{
	screen = (unsigned char *) kmappages(
#ifdef MONO
										 0xB0, 
#else 
										 0xB8, 
#endif
										 2, 3);
	
    con_start((uint32) vscreen);
    con_attr(CON_YELLOW|0x08);
    con_clear();
}

void krefresh(void)
{
	memcpy(screen,vscreen,80*25*2);
}

static char line[80];
void kprintf(const char *fmt, ...)
{
    va_list pvar;    
    va_start(pvar,fmt);
#ifdef __SMP__
	p (&kprintf_lock);
#endif
	va_snprintf(line,80,fmt,pvar);
    line[79] = 0;
    va_end(pvar);
    con_goto(0,24);
    con_puts(line);
    con_puts("\n");
	memcpy(screen,vscreen,80*25*2);
#ifdef __SMP__
	v (&kprintf_lock);
#endif
}

#ifdef DPRINTF

void dprintf(const char *fmt, ...)
{
    va_list pvar;    
    va_start(pvar,fmt);
#ifdef __SMP__
	p (&kprintf_lock);
#endif
    va_snprintf(line,80,fmt,pvar);
    line[79]=0;
    va_end(pvar);
    ser_puts(line);
    ser_puts("\r\n");
#ifdef __SMP__
	v (&kprintf_lock);
#endif
}

#endif

void movecursor (int x, int y)
{
    int offset;

    offset = 80 * y + x;
    outb (0xe, 0x3d4);
    outb (offset / 256, 0x3d5);
    outb (0xf, 0x3d4);
    outb (offset % 256, 0x3d5);
}   

char *kgetline(char *line, int len)
{
    int i,lp,key;
    int shift = 0;
    if(len > 80) len = 80;
    
  restart:
    for(i=1;i<len-1;i++) line[i] = ' ';
    line[0] = ':';
    line[1] = ' ';
    line[len-1] = 0;
    lp = 2;
    
    for(;;){
        con_goto(0,24);
        con_puts(line);
		movecursor(lp,24);
		
		memcpy(screen + (80*24*2), vscreen + (80*24*2), 160);
        while(!(inb(0x64) & 0x01));
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
                        line[lp] = 0;
                        kprintf(line);
                        return line + 2;
                    case ESC:                        
                        goto restart;
                    case BS:
                        if(lp > 2){
                            lp--;
                            line[lp]=' ';
                        }
                        break;
                    default:
                        if(lp < len-1);
                        line[lp] = key;
                        lp++;
                    }
                    
                }
            }
        }
    }
}

#endif
