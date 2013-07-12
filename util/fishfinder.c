/* $Id: //depot/blt/util/fishfinder.c#2 $
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

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>

#undef uint32

#define NEED_TYPES
#include "dfp.h"

char *types[] = { "PING", "PONG", "SEND", "ACK ", "NAK ", "SYNC" };

int main(int argc, char *argv[])
{
    struct sockaddr_in dst,src;
    dfp_pkt_transfer dfp,dfp2;
    char n2[17];
    int s;

    
    src.sin_family = AF_INET;
    src.sin_addr.s_addr = htonl(INADDR_ANY);
    src.sin_port = htons(DEFAULT_DFP_PORT);
    
    if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        perror("socket");
    
    if(bind(s, (struct sockaddr *) &src, sizeof(src)) == -1)
        perror("bind");

    for(;;){
        if(recvfrom(s,(void *)&dfp,sizeof(dfp),0,NULL,NULL) < 0) {
            perror("recvfrom");
        }
        dfp.base.size = ntohs(dfp.base.size);
        dfp.base.version = ntohs(dfp.base.version);

        if(dfp.base.version != DFP_VERSION){
            printf("version mismatch!\n");
            continue;
        }

        if(dfp.base.type > 5) continue;

        printf("%s: (%d,%d) -> (%d,%d) ", types[dfp.base.type],
               dfp.base.src_tank_x, dfp.base.src_tank_y,
               dfp.base.dst_tank_x, dfp.base.dst_tank_y,
               dfp.base.size);
        
        switch(dfp.base.type){
        case DFP_PKT_PING:
        case DFP_PKT_PONG:
            printf("\n");
	break;
        case DFP_PKT_SYNC_FISH:
        case DFP_PKT_SEND_FISH:
            printf("[%02x%02x%02x%02x%02x%02x] ",
                   dfp.uid.creator_tank_x, dfp.uid.creator_tank_y,
                   dfp.uid.timestamp[0], dfp.uid.timestamp[1],
                   dfp.uid.timestamp[2], dfp.uid.timestamp[3]);
            strncpy(n2,dfp.fish.name,16);
            n2[16]=0;
            printf("@(%3d,%3d) d(%3d,%3d) \"%s\"\n",
                   dfp.fish.x, dfp.fish.y, dfp.fish.dx, dfp.fish.dy, n2);
            break;
        case DFP_PKT_NACK_FISH:
        case DFP_PKT_ACK_FISH:
            printf("[%02x%02x%02x%02x%02x%02x]\n",
                   dfp.uid.creator_tank_x, dfp.uid.creator_tank_y,
                   dfp.uid.timestamp[0], dfp.uid.timestamp[1],
                   dfp.uid.timestamp[2], dfp.uid.timestamp[3]);
            break;
        }
    }
    
    close(s);    
    return 0;
}
