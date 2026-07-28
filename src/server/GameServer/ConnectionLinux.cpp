#include "stdafx.h"
#include "Connection.h"
#include "Log.h"
#include "Util.h"

#ifndef _WIN32

#include <fcntl.h>
#include <poll.h>
#include <system_error>

static bool SetNonBlocking(SOCKET socket)
{
	int flags = fcntl(socket, F_GETFL, 0);
	if (flags == -1)
	{
		return false;
	}

	return (fcntl(socket, F_SETFL, flags | O_NONBLOCK) != -1);
}

CConnection::CConnection()
{
	this->m_hwnd = 0;
	this->m_socket = INVALID_SOCKET;
	this->m_EventHandlerThread = NULL;
	this->m_hEvent = NULL;
	this->m_running = false;
}

CConnection::~CConnection()
{
	this->Disconnect();
}

void CConnection::Init(HWND hwnd, const char* name, ProtocolCoreFn function)
{
	this->m_hwnd = hwnd;
	this->sConnectionName = name;
	this->m_socket = socket(PF_INET, SOCK_STREAM, 0);
	SetNonBlocking(this->m_socket);
	this->wsProtocolCore = function;
}

bool CConnection::Connect(const char* IpAddress, WORD port)
{
	if (this->m_socket == INVALID_SOCKET)
	{
		return false;
	}

	SOCKADDR_IN target {};
	target.sin_family = AF_INET;
	target.sin_port = htons(port);
	inet_pton(AF_INET, IpAddress, &target.sin_addr.s_addr);

	if (target.sin_addr.s_addr == INADDR_NONE)
	{
		char port_str[16] = {};
		sprintf_s(port_str, "%d", port);

		struct addrinfo hints = {}, * addrs = nullptr;
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_TCP;

		int err = getaddrinfo(IpAddress, port_str, &hints, &addrs);
		if (err == 0 && addrs)
		{
			memcpy(&target.sin_addr.s_addr, addrs->ai_addr, addrs->ai_addrlen);
			freeaddrinfo(addrs);
		}
	}

	if (connect(this->m_socket, (SOCKADDR*)&target, sizeof(target)) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK && WSAGetLastError() != EINPROGRESS)
		{
			gLog.Output(LOG_CONNECT, "[%s] connect() failed with error: %d", this->sConnectionName.c_str(), WSAGetLastError());
			this->Disconnect();
			return false;
		}
	}

	if (this->CreateEventHandler() == false)
	{
		this->Disconnect();
		return false;
	}

	memset(this->m_RecvBuff, 0, sizeof(this->m_RecvBuff));
	this->m_RecvSize = 0;
	memset(this->m_SendBuff, 0, sizeof(this->m_SendBuff));
	this->m_SendSize = 0;

	return true;
}

bool CConnection::CreateEventHandler()
{
	if (this->m_eventThread.joinable())
	{
		if (this->m_eventThread.get_id() == std::this_thread::get_id())
		{
			this->m_eventThread.detach();
		}
		else
		{
			this->m_eventThread.join();
		}
	}

	this->m_running = true;

	try
	{
		this->m_eventThread = std::thread(&CConnection::SocketEventsThread, this);
	}
	catch (const std::system_error& e)
	{
		this->m_running = false;
		gLog.Output(LOG_CONNECT, "[%s] std::thread() failed: %s", this->sConnectionName.c_str(), e.what());
		return false;
	}

	return true;
}

DWORD CConnection::SocketEventsThread(CConnection* lpConnection)
{
	while (lpConnection->m_running)
	{
		struct pollfd pfd {};
		pfd.fd = lpConnection->m_socket;
		pfd.events = POLLIN;

		if (lpConnection->m_SendSize > 0)
		{
			pfd.events |= POLLOUT;
		}

		int res = poll(&pfd, 1, 100);
		if (res <= 0)
		{
			continue;
		}

		if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
		{
			LogAdd(LOG_RED, "[%s] Disconnected", lpConnection->sConnectionName.c_str());
			lpConnection->Disconnect();
			break;
		}

		if (pfd.revents & POLLIN)
		{
			if (!lpConnection->DataRecv())
			{
				break;
			}
		}

		if (pfd.revents & POLLOUT)
		{
			lpConnection->DataSendEx();
		}
	}

	return 0;
}

void CConnection::Disconnect()
{
	this->m_running = false;

	if (this->m_socket != INVALID_SOCKET)
	{
		closesocket(this->m_socket);
		this->m_socket = INVALID_SOCKET;
	}

	if (this->m_eventThread.joinable())
	{
		if (this->m_eventThread.get_id() == std::this_thread::get_id())
		{
			this->m_eventThread.detach();
		}
		else
		{
			this->m_eventThread.join();
		}
	}
}

bool CConnection::CheckState()
{
	return ((this->m_socket == INVALID_SOCKET) ? false : true);
}

bool CConnection::DataRecv()
{
	int count = 0, size = 0, result = 0;

	if ((result = recv(this->m_socket, (char*)&this->m_RecvBuff[this->m_RecvSize], (MAX_BUFF_SIZE - this->m_RecvSize), 0)) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			return true;
		}
		else
		{
			gLog.Output(LOG_CONNECT, "[%s] recv() failed with error: %d", this->sConnectionName.c_str(), WSAGetLastError());
			this->Disconnect();
			return false;
		}
	}

	if (result == 0)
	{
		this->Disconnect();
		return false;
	}

	this->m_RecvSize += result;

	if (this->m_RecvSize < 3)
	{
		return true;
	}

	BYTE header, head;

	while (true)
	{
		if (this->m_RecvBuff[count] == 0xC1)
		{
			header = this->m_RecvBuff[count];
			size = this->m_RecvBuff[count + 1];
			head = this->m_RecvBuff[count + 2];
		}
		else if (this->m_RecvBuff[count] == 0xC2)
		{
			header = this->m_RecvBuff[count];
			size = MAKEWORD(this->m_RecvBuff[count + 2], this->m_RecvBuff[count + 1]);
			head = this->m_RecvBuff[count + 3];
		}
		else
		{
			gLog.Output(LOG_CONNECT, "[%s] Protocol header error (Header: %x)", this->sConnectionName.c_str(), this->m_RecvBuff[count]);
			this->Disconnect();
			return false;
		}

		if (size < 3 || size > MAX_BUFF_SIZE)
		{
			gLog.Output(LOG_CONNECT, "[%s] Protocol size error (Header: %x, Size: %d, Head: %x)", this->sConnectionName.c_str(), header, size, head);
			this->Disconnect();
			return false;
		}

		if (size <= this->m_RecvSize)
		{
			this->wsProtocolCore(head, &this->m_RecvBuff[count], size);
			count += size;
			this->m_RecvSize -= size;

			if (this->m_RecvSize <= 0)
			{
				break;
			}
		}
		else
		{
			if (count > 0 && this->m_RecvSize > 0 && this->m_RecvSize <= (MAX_BUFF_SIZE - count))
			{
				memmove(this->m_RecvBuff, &this->m_RecvBuff[count], this->m_RecvSize);
			}

			break;
		}
	}

	return true;
}

bool CConnection::DataSend(BYTE* lpMsg, int size)
{
	this->m_critical.lock();

	if (this->m_socket == INVALID_SOCKET)
	{
		this->m_critical.unlock();
		return false;
	}

	if (this->m_SendSize > 0)
	{
		if ((this->m_SendSize + size) > MAX_BUFF_SIZE)
		{
			gLog.Output(LOG_CONNECT, "[%s] Max msg size (Type: 1, Size: %d)", this->sConnectionName.c_str(), (this->m_SendSize + size));
			this->Disconnect();
			this->m_critical.unlock();
			return false;
		}

		memcpy(&this->m_SendBuff[this->m_SendSize], lpMsg, size);
		this->m_SendSize += size;
		this->m_critical.unlock();
		return true;
	}

	int count = 0, result = 0;

	while (size > 0)
	{
		if ((result = send(this->m_socket, (char*)&lpMsg[count], size, 0)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				if ((this->m_SendSize + size) > MAX_BUFF_SIZE)
				{
					gLog.Output(LOG_CONNECT, "[%s] Max msg size (Type: 2, Size: %d)", this->sConnectionName.c_str(), (this->m_SendSize + size));
					this->Disconnect();
					this->m_critical.unlock();
					return false;
				}

				memcpy(&this->m_SendBuff[this->m_SendSize], &lpMsg[count], size);
				this->m_SendSize += size;
				this->m_critical.unlock();
				return true;
			}
			else
			{
				gLog.Output(LOG_CONNECT, "[%s] send() failed with error: %d", this->sConnectionName.c_str(), WSAGetLastError());
				this->Disconnect();
				this->m_critical.unlock();
				return false;
			}
		}
		else
		{
			count += result;
			size -= result;
		}
	}

	this->m_critical.unlock();
	return true;
}

bool CConnection::DataSendEx()
{
	this->m_critical.lock();

	int count = 0, result = 0;

	while (this->m_SendSize > 0)
	{
		if ((result = send(this->m_socket, (char*)&this->m_SendBuff[count], this->m_SendSize, 0)) == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				memmove(this->m_SendBuff, &this->m_SendBuff[count], this->m_SendSize);
				this->m_critical.unlock();
				return true;
			}
			else
			{
				gLog.Output(LOG_CONNECT, "[%s] send() failed with error: %d", this->sConnectionName.c_str(), WSAGetLastError());
				this->Disconnect();
				this->m_critical.unlock();
				return false;
			}
		}
		else
		{
			count += result;
			this->m_SendSize -= result;
		}
	}

	this->m_critical.unlock();
	return true;
}

#endif
