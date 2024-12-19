#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <WinSock2.h>
#include <string>

enum TypeSocket { BlockingSocket, NonBlockingSocket };

using std::string;

class Socket
{
protected:
	friend class SocketServer;
	friend class SocketSelect;

private:
	static void Start();
	static void End();

private:
	static int nofSockets_;

public:
	Socket(const Socket&);
	virtual ~Socket();

protected:
	Socket(SOCKET s);
	Socket();

public:
	string ReceiveLine();
	string ReceiveBytes();
	void Close();
	void SendLine(string);
	void SendBytes(const string&);

protected:
	SOCKET s_;
	int* refCounter_;

public:
	Socket& operator=(Socket&);
};

class SocketClient : public Socket
{
public:
	SocketClient(const string& host, int port);
};

class SocketServer : public Socket
{
public:
	SocketServer(int port, int connections = 1, TypeSocket type = BlockingSocket);

public:
	Socket* Accept();
};

class SocketSelect
{
public:
	SocketSelect(Socket const * const s1, Socket const * const s2 = NULL, TypeSocket type = BlockingSocket);

public:
	bool Readable(Socket const * const s);

private:
	fd_set fds_;
};
#endif