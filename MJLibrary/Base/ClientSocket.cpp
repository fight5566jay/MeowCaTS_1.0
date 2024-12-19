#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "ClientSocket.h"
#include "Ws2tcpip.h"

using std::cout;
using std::cerr;
using std::endl;

int inet_pton_self_define(int af, const char *src, void *dst);//for XP

void ClientSocket::initSocket(const string &sIp, const int &iPort)
{
	SOCKET lhSocket;
	SOCKADDR_IN lSockAddr;
	WSADATA wsaData;
	int lConnect;
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		std::cout << "[ClientSocket::initSocket] Socket Initialization Error. Program aborted\n";
		return;
	}
	lhSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (lhSocket == INVALID_SOCKET) {
		std::cout << "[ClientSocket::initSocket] Invalid Socket " << GetLastError() << ". Program Aborted\n" << std::endl;
	}
	memset(&lSockAddr, 0, sizeof(lSockAddr));
	lSockAddr.sin_family = AF_INET;
	lSockAddr.sin_port = htons(iPort);
	//lSockAddr.sin_addr.s_addr = inet_addr(sIp.c_str());//compiler error in VS2015
	//inet_pton(AF_INET, sIp.c_str(), (PVOID *)(&lSockAddr.sin_addr.s_addr));
	inet_pton_self_define(AF_INET, sIp.c_str(), (PVOID *)(&lSockAddr.sin_addr.s_addr));//for XP
	lConnect = connect(lhSocket, (SOCKADDR *)&lSockAddr, sizeof(SOCKADDR_IN));
	if (lConnect != 0) {
		std::cout << "[ClientSocket::initSocket] lConnect = " << lConnect << "  Connect Error. Program aborted\n";
		return;
	}
	m_sock = lhSocket;

	std::cerr << "[ClientSocket::initSocket] socket is intialized." << std::endl;
}

void ClientSocket::sendData(const string &sData) const
{
	int iLength = send(m_sock, sData.c_str(), sData.size(), 0);
	if (iLength < (int)sData.size()) { std::cout << "Send Error.\n"; }
}

void ClientSocket::recvData(string &sData) const
{
	char buf[2];
	sData = "";
	memset(buf, 0, 2);

	while (true) {
		int status = recv(m_sock, buf, 1, 0);
		if (status == -1) {
			std::cout << "[ClientSocket::recvData] status == -1   errno == " << errno << "  in Socket::recv\n";
			exit(1);
			return;
		} else if (status == 0) return;
		else {
			if (buf[0] != '\n') sData += buf[0];
			else {
				sData += "\n\0";
				return;
			}
		}
	}
}

void ClientSocket::sendData(const vector<float>& vData)
{
	send(m_sock, (char*)&vData[0], vData.size() * sizeof(float), 0);
}

void ClientSocket::recvData(vector<float>& vData)
{
	recv(m_sock, (char*)&vData[0], vData.size() * sizeof(float), 0);
}

void ClientSocket::sendData(const int& iData)
{
	send(m_sock, (char*)&iData, sizeof(int), 0);
}

void ClientSocket::recvData(int& iData)
{
	recv(m_sock, (char*)&iData, sizeof(int), 0);
}

int inet_pton_self_define(int af, const char *src, void *dst)//for XP
{
	struct sockaddr_storage ss;
	int size = sizeof(ss);
	char src_copy[INET6_ADDRSTRLEN + 1];//old
	//wchar_t src_copy[INET6_ADDRSTRLEN + 1];//new
	size_t convertSize;

	ZeroMemory(&ss, sizeof(ss));
	//stupid non-const API
	strncpy_s(src_copy, src, INET6_ADDRSTRLEN + 1);//old
	/*if (mbstowcs_s(&convertSize, src_copy, src, INET6_ADDRSTRLEN + 1) != 0) {//new
		char errMsg[1000];
		strerror_s(errMsg, 1000, errno);
		std::cerr << "[ClientSocket::inet_pton_self_define] mbstowcs_s error: " << errMsg << std::endl;
	}*/
	src_copy[INET6_ADDRSTRLEN] = 0;

	if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
		switch (af) {
		case AF_INET:
			*(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
			return 1;
		case AF_INET6:
			*(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
			return 1;
		}
	}
	return 0;
}

const ClientSocket& ClientSocket::operator<<(const std::string & s) const
{
	sendData(s);
	return *this;
}

const ClientSocket& ClientSocket::operator >> (std::string & s) const
{
	recvData(s);
	return *this;
}