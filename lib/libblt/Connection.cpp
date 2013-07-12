/* Copyright 1999, Brian J. Swetland. All rights reserved.
** Distributed under the terms of the OpenBLT License
*/

#include <blt/Connection.h>
#include <blt/Message.h>
#include <blt/namer.h>
#include <blt/syscall.h>

#include <string.h>

using namespace BLT;

Connection::~Connection()
{
	port_destroy(_local_port);
}

int 
Connection::Send(const Message *msg)
{
	msg_hdr_t mh;
	const void *data;
	uint32 len;
	int res;
	
	msg->GetPackedData(&data,&len);

	mh.flags = 0;
	mh.src = _local_port;
	mh.dst = _remote_port;
	mh.data = (void*) data;
	mh.size = len;
	
	res = old_port_send(&mh);
	if(res != len) {
		return res;
	} else {
		return 0;
	}
}

int 
Connection::Recv(Message *msg)
{
	msg_hdr_t mh;
	int res;
	char buf[1024];
	
	mh.flags = 0;
	mh.dst = _local_port;
	mh.size = 1024;
	mh.data = buf;
	
	if((res = old_port_recv(&mh)) < 0) return res;
	
	return msg->PutPackedData(buf,res,mh.src);
}

int 
Connection::Call(const Message *send, Message *recv)
{
	int res;
	if((res = Send(send))) {
		return res;
	} else {
		return Recv(recv);
	}
}

Connection *
Connection::FindService(const char *name)
{
	Connection *cnxn;
	int port;
	
	if(!strcmp(name,"namer")){
		port = NAMER_PORT;
	} else {
		port = namer_find(name, 0);
	}
	if(port < 1) return 0;
	
	cnxn = new Connection();
	cnxn->_remote_port = port;
	
	return cnxn;
}

Connection *
Connection::CreateService(const char *name)
{
	Connection *cnxn = new Connection();

	if(!strcmp(name,"namer")){
		port_destroy(cnxn->_local_port);
		cnxn->_local_port = NAMER_PORT;
	} else {
		if(namer_register(cnxn->_local_port, (char*) name)){
			delete cnxn;
			cnxn = 0;
		}
	}
	
	return cnxn;
}


Connection::Connection()
{
	_local_port = port_create(0,"cnxn:local_port");
	_remote_port = 0;
}
