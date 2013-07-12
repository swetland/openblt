#include <blt/namer.h>
#include <blt/syscall.h>
#include <blt/conio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Connection.h"

const int kSendBufferSize = 0x1000;
const int kMaxCommandSize = 270;
const int kReceiveBufferSize = 1024;

Connection::Connection(int sendPort, int receivePort)
	:	fSendPort(sendPort),
		fReceivePort(receivePort),
		fSendBufferSize(0),
		fReceiveBufferSize(0),
		fReceiveBufferPos(0)
{
	fSendBuffer = (char*) malloc(kSendBufferSize);
	fReceiveBuffer = (char*) malloc(kReceiveBufferSize);
}

Connection::~Connection()
{
	free(fSendBuffer);
	free(fReceiveBuffer);
	port_destroy(fReceivePort);
}

void Connection::Write(const void *data, int size)
{
	int sizeWritten = 0;
	while (sizeWritten < size) {
		int sizeToWrite = size - sizeWritten;
		if (fSendBufferSize + sizeToWrite > kSendBufferSize)
			sizeToWrite = kSendBufferSize - fSendBufferSize;
	
		if (sizeToWrite == 0) {
			Flush();
			continue;
		}

		memcpy((void*) (fSendBuffer + fSendBufferSize), (const void*) ((const char*) data
			+ sizeWritten), sizeToWrite);
		fSendBufferSize += sizeToWrite; 
		sizeWritten += sizeToWrite;
	}
}

void Connection::Read(void *data, int size)
{
	int sizeRead = 0;
	while (sizeRead < size) {
		int sizeToRead = size - sizeRead;
		
		if (fReceiveBufferSize - fReceiveBufferPos < sizeToRead)
			sizeToRead = fReceiveBufferSize - fReceiveBufferPos;
	
		if (sizeToRead == 0) {
			Receive();
			continue;
		}

		memcpy((void*) ((char*) data + sizeRead),
			(void*) (fReceiveBuffer + fReceiveBufferPos), sizeToRead);
		fReceiveBufferPos += sizeToRead; 
		sizeRead += sizeToRead;
	}
}

void Connection::EndCommand()
{
	if (fSendBufferSize > kSendBufferSize - kMaxCommandSize)
		Flush();
}

void Connection::Flush()
{
	msg_hdr_t header;
	header.src = fReceivePort;
	header.dst = fSendPort;
	header.data = fSendBuffer;
	header.size = fSendBufferSize;

	old_port_send(&header);
	
	fSendBufferSize = 0;
}

void Connection::Receive()
{
	msg_hdr_t header;
	header.src = 0;
	header.dst = fReceivePort;
	header.data = fReceiveBuffer;
	header.size = kReceiveBufferSize;

	fReceiveBufferSize = old_port_recv(&header);
	fReceiveBufferPos = 0;
}

int Connection::ReadInt32()
{
	int outval;
	Read(&outval, 4);
	return outval;
}

short Connection::ReadInt16()
{
	short outval;
	Read(&outval, 2);
	return outval;
}

char Connection::ReadInt8()
{
	char outval;
	Read(&outval, 1);
	return outval;
}

void Connection::WriteInt32(int i)
{
	Write(&i, 4);
}

void Connection::WriteInt16(short s)
{
	Write(&s, 2);
}

void Connection::WriteInt8(char c)
{
	Write(&c, 1);
}

