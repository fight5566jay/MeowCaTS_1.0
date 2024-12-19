#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

#define MAXRECV 256

#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

using std::vector;
using std::string;

class ClientSocket
{
public:
	ClientSocket() {};
	ClientSocket(const string &sIp, const int &iPort) { initSocket(sIp, iPort); };

public:
	inline void closeSocket() const { closesocket(m_sock); };
	
	void initSocket(const string &sIp, const int &iPort);
	void sendData(const string &sData) const;
	void recvData(string &sData) const;
	void sendData(const vector<float>& vData);
	void recvData(vector<float>& vData);
	void sendData(const int& iData);
	void recvData(int& iData);
	const ClientSocket& operator<<(const string &) const;
	const ClientSocket& operator>>(string &) const;

private:
	SOCKET m_sock;
};

#endif