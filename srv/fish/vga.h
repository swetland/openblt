/* $Id: //depot/blt/srv/fish/vga.h#3 $
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
#ifndef _vga_h
#define _vga_h

void vga_set_palette(int color, int red, int green, int blue);
int vga_set_mode(int xres, int yres, int bitdepth);
/* only valid parms are (320, 200, 8); */

void vga_blit(char *bitmap, int x, int y, int w, int h);
void vga_blit_trans(char *bitmap, int x, int y, int w, int h);
void vga_blit_trans_r(char *bitmap, int x, int y, int w, int h);
void vga_blit_str(char *s, int x, int y, int c);

void vga_fill(int w, int h, int x, int y, int c);
void vga_fill_grad(int w, int h, int x, int y);

void vga_set_vram(void *addr);
void vga_set_sram(void *addr);
void vga_swap_buffers(void);
void vga_fillX(void);
void vga_modex(unsigned int w);

#endif
