// Copyright 1999, Brian J. Swetland.  All Rights Reserved.
// This file is provided under the terms of the OpenBLT License

#ifndef __BLT_CONNECTION_H
#define __BLT_CONNECTION_H

#include <blt/types.h>

namespace BLT {

class Message;
	
class Connection
{
public:
	~Connection();
	
	int Send(const Message *msg);
	int Recv(Message *msg);
	
	int Call(const Message *send, Message *recv);
	
	static Connection *FindService(const char *name);
	static Connection *CreateService(const char *name);
	
private:
	Connection();
	
	int _local_port;
	int _remote_port;
};

}

#endif
