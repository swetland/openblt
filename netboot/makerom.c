/* $Id: //depot/blt/netboot/makerom.c#4 $
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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define ROMSIZE 0x4000

unsigned char rom[ROMSIZE];
unsigned int sum;

void makehex(char *n)
{
    FILE *fp;
    int chk,chk2;
    int l,i;
    
    if(fp = fopen(n,"w")){
        for(l=0;l<ROMSIZE;l+=32) {
            fprintf(fp,":%02X%04X00",32,l);

            chk = 32;

            chk += ((l & 0xFF00) >> 8);
            chk += ((l & 0x00FF));
            
            for(i=0;i<32;i++){
                fprintf(fp,"%02X",rom[l+i]);
                chk += rom[l+i];
            }
            chk &= 0xFF;
            chk2 = 0x100 - chk;
            
            fprintf(fp,"%02X\r\n",(chk2 & 0xFF));
        }
        fprintf(fp,":00000001FF\r\n");
        
        fclose(fp);
    }
}


int main(int argc, char *argv[])
{
	int i, fd;
	if (argc < 2) {
		fprintf(stderr,"usage: %s rom-file hex-file\n",argv[0]);
		exit(2);
	}
	if ((fd = open(argv[1], O_RDWR)) < 0) {
		perror("unable to open file");
		return 1;
	}
	bzero(rom, ROMSIZE);
	
	if (read(fd, rom, ROMSIZE) < 0) {
		perror("read error");
		return 1;
	}

	/* store size in ROM header */	
	rom[2] = ROMSIZE / 512;

	/* store size in PCI ROM header */
	rom[0x18 + 0x10 + 4] = ROMSIZE / 512;

	rom[5] = 0;
	for (i=0,sum=0; i<ROMSIZE; i++) sum += rom[i];
	rom[5] = -sum;
	
	for (i=0,sum=0; i<ROMSIZE; i++) sum += rom[i];
	if (sum & 0x00FF) printf("checksum fails.\n");
	
	close(fd);
	
	makehex(argv[2]);
        
	return 0;
}
