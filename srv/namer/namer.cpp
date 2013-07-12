/* Copyright 1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <blt/namer.h>
#include <blt/Message.h>
#include <blt/Connection.h>

#include <blt/syscall.h>
#include <string.h>
#include <stdlib.h>

using namespace BLT;

typedef struct nentry nentry;

struct nentry
{
	nentry *next;
	int32 port;
	char name[1];
};

static nentry *first = 0;

static int32 
do_namer_find(const char *name)
{
	nentry *e;
	
	if(!name) return -1;
	
	for(e = first; e; e = e->next){
		if(!strcmp(e->name,name)) return e->port;
	}
	
	return -1;
}

static int32 
do_namer_register(int32 port, const char *name)
{
	nentry *e;
	
	if(!name) return -1;
	if(port < 1) return -1;
	
	for(e = first; e; e = e->next){
		if(!strcmp(e->name,name)) return -1;
	}
	
	e = (nentry *) malloc(sizeof(nentry) + strlen(name));
	
	if(!e) return -1;
	
	e->port = port;
	strcpy(e->name,name);
	e->next = first;
	first = e;
	
	return 0;
}

int main(void)
{
	Connection *cnxn = Connection::CreateService("namer");
	Message msg, reply;

	do_namer_register(NAMER_PORT,"namer");

	while(cnxn->Recv(&msg) == 0){
		int32 op = -1;
		int32 port = -1;
		const char *name = 0;
		int32 res = -1;
	
		msg.GetInt32('code',&op);
		msg.GetInt32('port',&port);
		msg.GetString('name',&name);
		
		switch(op){
		case NAMER_FIND:
			res = do_namer_find(name);
			break;
			
		case NAMER_REGISTER:
			res = do_namer_register(port, name);
			break;
		}
		
		reply.Empty();
		reply.PutInt32('resp',res);
		msg.Reply(&reply);
	}
	
	return 0;
}
