#include "stdafx.h"
#include "SocketManagerUdp.h"
#include "ServerList.h"
#include "Util.h"

#ifndef _WIN32

CSocketManagerUdp gSocketManagerUdp;

CSocketManagerUdp::CSocketManagerUdp()
{
	this->m_socket = INVALID_SOCKET;
	this->m_ServerRecvThread = 0;
	this->m_RecvSize = 0;
	this->m_SendSize = 0;
	this->m_running = false;
	memset(&this->m_SocketAddr, 0, sizeof(this->m_SocketAddr));
}

CSocketManagerUdp::~CSocketManagerUdp()
{
	this->Clean();
}

bool CSocketManagerUdp::Start(WORD port)
{
	this->m_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (this->m_socket == INVALID_SOCKET)
	{
		LogAdd(LOG_RED, "[SocketManagerUdp] socket() failed with error: %d", WSAGetLastError());
		return false;
	}

	SOCKADDR_IN SocketAddr {};
	SocketAddr.sin_family = AF_INET;
	SocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	SocketAddr.sin_port = htons(port);

	if (bind(this->m_socket, (sockaddr*)&SocketAddr, sizeof(SocketAddr)) == SOCKET_ERROR)
	{
		LogAdd(LOG_RED, "[SocketManagerUdp] bind() failed with error: %d", WSAGetLastError());
		closesocket(this->m_socket);
		this->m_socket = INVALID_SOCKET;
		return false;
	}

	this->m_running = true;
	this->m_recvThread = std::thread(&CSocketManagerUdp::ServerRecvThread, this);

	return true;
}

bool CSocketManagerUdp::Connect(char* IpAddress, WORD port)
{
	this->m_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (this->m_socket == INVALID_SOCKET)
	{
		LogAdd(LOG_RED, "[SocketManagerUdp] socket() failed with error: %d", WSAGetLastError());
		return false;
	}

	char port_str[16] = {};
	sprintf_s(port_str, "%d", port);

	struct addrinfo hints = {}, * addrs = nullptr;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	int err = getaddrinfo(IpAddress, port_str, &hints, &addrs);
	if (err != 0 || addrs == nullptr)
	{
		LogAdd(LOG_RED, "[SocketManagerUdp] getaddrinfo() failed with error: %d", err);
		return false;
	}

	memcpy(&this->m_SocketAddr, addrs->ai_addr, addrs->ai_addrlen);
	freeaddrinfo(addrs);

	return true;
}

void CSocketManagerUdp::Clean()
{
	this->m_running = false;

	if (this->m_recvThread.joinable())
	{
		this->m_recvThread.join();
	}

	if (this->m_socket != INVALID_SOCKET)
	{
		closesocket(this->m_socket);
		this->m_socket = INVALID_SOCKET;
	}
}

bool CSocketManagerUdp::DataRecv()
{
	if (this->m_RecvSize < 3)
	{
		return true;
	}

	BYTE* lpMsg = this->m_RecvBuff;
	int count = 0, size = 0;
	BYTE header, head;

	while (true)
	{
		if (lpMsg[count] == 0xC1)
		{
			header = lpMsg[count];
			size = lpMsg[count + 1];
			head = lpMsg[count + 2];
		}
		else if (lpMsg[count] == 0xC2)
		{
			header = lpMsg[count];
			size = MAKEWORD(lpMsg[count + 2], lpMsg[count + 1]);
			head = lpMsg[count + 3];
		}
		else
		{
			LogAdd(LOG_RED, "[SocketManagerUdp] Protocol header error (Header: %x)", lpMsg[count]);
			return false;
		}

		if (size < 3 || size > MAX_UDP_PACKET_SIZE)
		{
			LogAdd(LOG_RED, "[SocketManagerUdp] Protocol size error (Header: %x, Size: %d, Head: %x)", header, size, head);
			return false;
		}

		if (size <= this->m_RecvSize)
		{
			gServerList.ServerProtocolCore(head, &lpMsg[count], size);

			count += size;
			this->m_RecvSize -= size;

			if (this->m_RecvSize <= 0)
			{
				break;
			}
		}
		else
		{
			if (count > 0 && this->m_RecvSize > 0 && this->m_RecvSize <= (MAX_UDP_PACKET_SIZE - count))
			{
				memmove(lpMsg, &lpMsg[count], this->m_RecvSize);
			}

			break;
		}
	}

	return true;
}

bool CSocketManagerUdp::DataSend(BYTE* lpMsg, int size)
{
	if (size > MAX_UDP_PACKET_SIZE)
	{
		LogAdd(LOG_RED, "[SocketManagerUdp] Max msg size (Size: %d)", size);
		return false;
	}

	memcpy(this->m_SendBuff, lpMsg, size);
	this->m_SendSize = size;

	int sent = sendto(this->m_socket, (char*)this->m_SendBuff, this->m_SendSize, 0, (sockaddr*)&this->m_SocketAddr, sizeof(this->m_SocketAddr));
	if (sent == SOCKET_ERROR)
	{
		LogAdd(LOG_RED, "[SocketManagerUdp] sendto() failed with error: %d", WSAGetLastError());
		return false;
	}

	return true;
}

DWORD CSocketManagerUdp::ServerRecvThread(CSocketManagerUdp* lpSocketManagerUdp)
{
	SOCKADDR_IN SocketAddr {};
	socklen_t SocketAddrSize = sizeof(SocketAddr);

	while (lpSocketManagerUdp->m_running)
	{
		int result = recvfrom(lpSocketManagerUdp->m_socket,
			(char*)&lpSocketManagerUdp->m_RecvBuff[lpSocketManagerUdp->m_RecvSize],
			(MAX_UDP_PACKET_SIZE - lpSocketManagerUdp->m_RecvSize), 0, (sockaddr*)&SocketAddr, &SocketAddrSize);

		if (result == SOCKET_ERROR)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				Sleep(1);
				continue;
			}

			LogAdd(LOG_RED, "[SocketManagerUdp] recvfrom() failed with error: %d", GetLastError());
			memset(lpSocketManagerUdp->m_RecvBuff, 0, sizeof(lpSocketManagerUdp->m_RecvBuff));
			lpSocketManagerUdp->m_RecvSize = 0;
			continue;
		}

		lpSocketManagerUdp->m_RecvSize += result;
		lpSocketManagerUdp->DataRecv();
	}

	return 0;
}

#endif
