// Copyright 1999, Brian J. Swetland.  All Rights Reserved.
// This file is provided under the terms of the OpenBLT License

#ifndef __BLT_MESSAGE_H
#define __BLT_MESSAGE_H

#include <blt/types.h>

#define BLT_TYPE_INT32      'bI32'
#define BLT_TYPE_STRING     'bSTR'
#define BLT_TYPE_POINTER    'bPTR'
#define BLT_TYPE_MESSAGE    'bMSG'

namespace BLT {

class Message 
{
public:
	Message();
	~Message();
	
	/* -------------------------------------------------------- */
	int PutData(uint32 type, uint32 id, const void *data, uint32 len);
	int GetData(uint32 type, uint32 id, void *data, uint32 len) const;
	int GetData(uint32 type, uint32 id, const void **data, uint32 *len) const;
	int GetEntry(uint32 n, uint32 *type, uint32 *id, const void **data, uint32 *len) const;
	
	/* -------------------------------------------------------- */
	int PutInt32(uint32 id, int32 data);
	int PutString(uint32 id, const char *data);
	int PutPointer(uint32 id, void *data);
	int PutMessage(uint32 id, Message *msg);
			
	/* -------------------------------------------------------- */
	int GetInt32(uint32 id, int32 *data) const;
	int GetString(uint32 id, const char **data) const;		
	int GetPointer(uint32 id, void **ptr) const;
	int GetMessage(uint32 id, Message *msg) const;
	
	/* -------------------------------------------------------- */
	int GetPackedData(const void **data, uint32 *length) const;
	int PutPackedData(const void *data, uint32 length, int reply_port = -1);
	
	int Reply(const Message *msg) const;
	
	void Empty();
	
	void dump();
	
private:
	void *_data;
	uint32 _length;
	uint32 _available;
	
	int _reply_port;
};

}
#endif

