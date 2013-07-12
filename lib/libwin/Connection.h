#ifndef _CONNECTION_H
#define _CONNECTION_H

class Connection {
public:
	Connection(int sendPort, int receivePort);
	~Connection();

	int ReadInt32();
	short ReadInt16();
	char ReadInt8();
	void WriteInt32(int);
	void WriteInt16(short);
	void WriteInt8(char);

	void Write(const void*, int);
	void Flush();
	void EndCommand();
	void Read(void*, int);
	
private:
	void Receive();

	int fSendPort;
	int fReceivePort;
	char *fSendBuffer;
	char *fReceiveBuffer;
	int fSendBufferSize;
	int fReceiveBufferSize;
	int fReceiveBufferPos;
};


#endif