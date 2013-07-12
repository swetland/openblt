/* $Id: //depot/blt/lib/conio.c#2 $
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

#include <blt/conio.h>

static int attr = 1, px = 0, py = 24;
static char *screen = (char *) CON_SCREEN;
static char *posn   = (char *) CON_SCREEN + 80*24*2;

static void scrollup(void) 
{
    for(posn = screen + 160; posn <= screen + 80*25*2; posn++)
        *(posn - 160) = *posn;
    for(posn = screen + 80*24*2; posn <= screen + 80*25*2; posn++){
        *posn++ = ' ';
        *posn   = attr;
    }
}

void con_attr(int a)
{
    attr = a;
}

#define con_init() { con_attr(CON_WHITE); con_clear(); }

void con_start(unsigned int video)
{
    screen = (char *) video;
    con_attr(CON_WHITE);
    con_clear();
}


void con_clear(void) 
{
    int i;
    
    for(posn = screen, i=0;i<80*25;i++){
        *posn++ = ' ';
        *posn++ = attr;
    }
    px = 0;
    py = 24;
    posn = screen + (24*80)*2;
}

void con_goto(int x, int y)
{
    posn = screen + ((y*80)+x)*2;
    px = x;
    py = y;
    
}

/*
void con_putc(char ch)
{
	if(ch == '\n'){
		goto roll0;
	}
	*posn++ = ch;
	*posn++ = attr;
	px++;
	if(px == 80){
roll0:
		px = 0;
		if(py == 24)
			scrollup();
		else
			py++;
		posn = screen + (py*80+px)*2;
	}
}
*/
void con_puts(char *s)
{
    while(*s){
        if(*s == '\n'){
            s++;
            goto roll1;
        }
        *posn++ = *s++;
        *posn++ = attr;
        px++;
        if(px == 80){
          roll1:
            px = 0;
            if(py == 24)
                scrollup();
            else
                py++;
            posn = screen + (py*80+px)*2;
        }
    }
}
/*
static char pbuf[9];

void con_putp(unsigned int p)
{
	int i;

	pbuf[8] = 0;
	
	for(i=7;i>=0;i--){
		pbuf[i] = (0x0F & p);
		if(pbuf[i] > 9) pbuf[i] += 'A'-10;
		else pbuf[i] += '0';
		p >>= 4;
	}
	con_puts(pbuf);
}

void con_putx(unsigned char x)
{
    pbuf[0] = (x & 0xF0) >> 4;
    if(pbuf[0] > 9) pbuf[0] += 'A'-10;
    else pbuf[0] += '0';
    pbuf[1] = (x & 0x0F);
    if(pbuf[1] > 9) pbuf[1] += 'A'-10;
    else pbuf[1] += '0';
    pbuf[2] = 0;
    con_puts(pbuf);    
}
*/

void cprintf(char *fmt, ...)
{    
    char buf[256];
    va_list pvar;
    va_start(pvar,fmt);
    va_snprintf(buf,256,fmt,pvar);
    va_end(pver);
    con_puts(buf);
}

