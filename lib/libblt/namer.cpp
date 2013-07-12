/* Copyright 1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <blt/Connection.h>
#include <blt/Message.h>
#include <blt/namer.h>
#include <blt/syscall.h>

#include <string.h>

using namespace BLT;

int namer_register(int port, const char *name)
{
	int32 res = -1;
	Connection *cnxn = Connection::FindService("namer");
	
	if(cnxn){
		Message msg;
		msg.PutInt32('code',NAMER_REGISTER);
		msg.PutInt32('port',port);
		msg.PutString('name',name);
		cnxn->Call(&msg,&msg);
		msg.GetInt32('resp',&res);
		delete cnxn;
	}
	
	return res;
}

int namer_find(const char *name, int blocking)
{
	int32 res = -1;
	Connection *cnxn;
	
	cnxn = Connection::FindService("namer");
	
	if(cnxn){
		Message msg;
		msg.PutInt32('code',NAMER_FIND);
		msg.PutString('name',name);
		cnxn->Call(&msg,&msg);
		msg.GetInt32('resp',&res);
		delete cnxn;
	}
	
	return res;
}



