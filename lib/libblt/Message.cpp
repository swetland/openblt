// Copyright 1999, Brian J. Swetland.  All Rights Reserved.
// This file is provided under the terms of the OpenBLT License

#include <stdlib.h>
#include <string.h>

#include <blt/Message.h>
#include <blt/syscall.h>

#define GROW_BY 128
#define BLT_ID_MESSAGE 0x42023017

using namespace BLT;

struct chunk
{
	uint32 type;
	uint32 id;
	uint32 len;
	uchar data[4];
};

Message::Message()
{
	chunk *c;
	
	_data = malloc(GROW_BY);
	_length = 12;
	_available = GROW_BY - 12;
	
	c = (chunk*) _data;
	
	c->type = BLT_TYPE_MESSAGE;
	c->id = BLT_ID_MESSAGE;
	c->len = 0;
	
	_reply_port = 0;
}


Message::~Message()
{
	if(_data) free(_data);
}

int 
Message::PutData(uint32 type, uint32 id, const void *data, uint32 len)
{
	chunk *c;
	uint32 size = 12 + ((len + 3) & ~3);
	
	if(_available < size) {
		while(_available < size) {
			_available += GROW_BY;
		}
		_data = realloc(_data, (_length + _available));
	}
	
	c = (chunk*) ((char*) _data + _length);
	c->type = type;
	c->id = id;
	c->len = len;
	memcpy(c->data, data, len);
	
	_length += size;	
	((chunk*)_data)->len = _length - 12;
}

int 
Message::GetData(uint32 type, uint32 id, void *data, uint32 len) const
{
	chunk *c;
	char *x = (char *) _data + 12;
	uint32 count = _length - 12;
	uint32 size;
	
	while(count > 12){
		c = (chunk*) x;
		size = (12 + ((c->len + 3) & ~3));
		if((type == c->type) && (id == c->id) && (len == c->len)){
			memcpy(data, c->data, c->len);
			return 0;
		}
		
		x += size;
		count -= size;
	}
	
	return -1;
}

int 
Message::GetData(uint32 type, uint32 id, const void **data, uint32 *len) const
{
	chunk *c;
	char *x = (char *) _data + 12;
	uint32 count = _length - 12;
	uint32 size;
	
	while(count > 12){
		c = (chunk*) x;
		size = (12 + ((c->len + 3) & ~3));
		if((type == c->type) && (id == c->id)){
			*data = (void*) c->data;
			*len = c->len;
			return 0;
		}
		
		x += size;
		count -= size;
	}
	
	*data = NULL;
	return -1;
}

int 
Message::GetEntry(uint32 n, uint32 *type, uint32 *id, const void **data, uint32 *len) const
{
	chunk *c;
	char *x = (char *) _data + 12;
	uint32 count = _length - 12;
	uint32 size;
	uint32 _n = 0;
	
	while(count > 12){
		c = (chunk*) x;
		if(_n == n){
			*type = c->type;
			*id = c->id;
			*len = c->len;
			*data = (void*) c->data;
		}
		
		size = (12 + ((c->len + 3) & 3));
		x += size;
		count -= size;
	}
	
	*data = NULL;
	return -1;
}

int 
Message::GetPackedData(const void **data, uint32 *length) const
{
	*data = _data;
	*length = _length;
	
	return 0;
}

int 
Message::PutPackedData(const void *data, uint32 length, int reply_port)
{
	chunk *c = (chunk*) data;
	
	if(length < 12) return -1;
	
	if(c->type != BLT_TYPE_MESSAGE) return -1;
	if(c->id != BLT_ID_MESSAGE) return -1;
	if(c->len != (length - 12)) return -1;
	
	if(_data) free(_data);
	_data = malloc(length);
	
	memcpy(_data, data, length);
	_available = 0;
	_length = length;
	
	_reply_port = reply_port;
	
	return 0;
}
		
int 
Message::PutInt32(uint32 id, int32 data)
{
	return PutData(BLT_TYPE_INT32, id, &data, sizeof(int32));
}

int 
Message::PutString(uint32 id, const char *data)
{
	return PutData(BLT_TYPE_STRING, id, data, strlen(data)+1);
}

int 
Message::PutPointer(uint32 id, void *data)
{
	return PutData(BLT_TYPE_POINTER, id, data, sizeof(void*)); 
}

int 
Message::PutMessage(uint32 id, Message *msg)
{
	return PutData(BLT_TYPE_MESSAGE, id, msg->_data, msg->_length);
}

int 
Message::GetInt32(uint32 id, int32 *data) const
{
	return GetData(BLT_TYPE_INT32, id, (void*) data, sizeof(int32));
}

int 
Message::GetString(uint32 id, const char **data) const
{
	uint32 discard; 
	return GetData(BLT_TYPE_STRING, id, (const void**) data, &discard); 
}

int 
Message::GetPointer(uint32 id, void **ptr) const
{
	return GetData(BLT_TYPE_POINTER, id, (void*) ptr, sizeof(void *));
}

int 
Message::GetMessage(uint32 id, Message *msg) const
{
	const void *data;
	uint32 len;
	
	GetData(BLT_TYPE_MESSAGE, id, &data, &len);
	return msg->PutPackedData(data,len);
}

void
Message::Empty()
{
	_available += _length - 12;
	_length = 12;
	_reply_port = 0;
	((chunk*)_data)->len = 0;
}

int
Message::Reply(const Message *msg) const
{
	msg_hdr_t mh;
	int res;
	
	if(_reply_port > 0){
		if((res = port_create(0,"reply_port")) < 0) return res;
		mh.flags = 0;
		mh.src = res;
		mh.dst = _reply_port;
		mh.size = msg->_length;
		mh.data = msg->_data;
		res = old_port_send(&mh);
		port_destroy(mh.src);
		if(res == msg->_length) {
			return 0;
		} else {
			return res;
		}
	} else {
		return -1;
	}
}

#if 0

void Message::dump()
{
	int i,j;
	fprintf(stderr,"bMessage - %d of %d\n",_length,_length+_available);
	
	for(i=0;i<_length;i+=16){
		fprintf(stderr,"%04x: ",i);
		for(j=0;j<16;j++){
			if((i+j) < _length){ 
				fprintf(stderr,"%02x ",((uchar*)_data)[i+j]);
			} else {
				fprintf(stderr,"   ");
			}
		}
		fprintf(stderr,"  ");
		for(j=0;j<16;j++){
			if((i+j) < _length){ 
				uchar c = ((uchar*)_data)[i+j];
				if((c<' ')||(c>126)) c = '.';
				fprintf(stderr,"%c",c);
			} else {
				fprintf(stderr," ");
			}
		}			
		fprintf(stderr,"\n");
	}
	
}

int main(int argc, char *argv[])
{
	int32 n;
	const char *s;
	
	Message msg;
	msg.PutInt32('foob',42);
	msg.PutString('blah',"Hello, Message");
	
	if(msg.GetInt32('foob',&n)) exit(1);
	fprintf(stderr,"foob = %d\n",n);
	
	if(msg.GetString('blah',&s)) exit(1);
	fprintf(stderr,"blah = %s\n",s);
	
	msg.dump();
	return 0;
}
#endif
