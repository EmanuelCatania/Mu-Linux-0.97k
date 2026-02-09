#include "stdafx.h"
#include "SocketManager.h"
#include "HackCheck.h"
#include "IpManager.h"
#include "Log.h"
#include "PacketManager.h"
#include "Protocol.h"
#include "SerialCheck.h"
#include "User.h"
#include "Util.h"

#ifndef _WIN32

#include <fcntl.h>

CSocketManager gSocketManager;

static bool SetNonBlocking(SOCKET socket)
{
	int flags = fcntl(socket, F_GETFL, 0);
	if (flags == -1)
	{
		return false;
	}

	return (fcntl(socket, F_SETFL, flags | O_NONBLOCK) != -1);
}

CSocketManager::CSocketManager()
{
	this->m_listen = INVALID_SOCKET;
	this->m_CompletionPort = 0;
	this->m_port = 0;
	this->m_ServerAcceptThread = 0;
	this->m_ServerWorkerThreadCount = 0;
	this->m_ServerQueueSemaphore = 0;
	this->m_ServerQueueThread = 0;
	this->m_epollFd = -1;
	this->m_running = false;
	this->m_queueStop = false;
}

CSocketManager::~CSocketManager()
{
	this->Clean();
}

bool CSocketManager::Start(WORD port)
{
	this->m_port = port;
	this->m_running = true;
	this->m_queueStop = false;

	if (!this->CreateListenSocket())
	{
		this->Clean();
		return false;
	}

	if (!this->CreateCompletionPort())
	{
		this->Clean();
		return false;
	}

	if (!this->CreateAcceptThread())
	{
		this->Clean();
		return false;
	}

	if (!this->CreateWorkerThread())
	{
		this->Clean();
		return false;
	}

	if (!this->CreateServerQueue())
	{
		this->Clean();
		return false;
	}

	gLog.Output(LOG_CONNECT, "[SocketManager] Server started at port [%d]", this->m_port);
	return true;
}

void CSocketManager::Clean()
{
	this->m_running = false;

	{
		std::lock_guard<std::mutex> lock(this->m_queueMutex);
		this->m_queueStop = true;
	}
	this->m_queueCv.notify_all();

	if (this->m_listen != INVALID_SOCKET)
	{
		closesocket(this->m_listen);
		this->m_listen = INVALID_SOCKET;
	}

	if (this->m_epollFd != -1)
	{
		close(this->m_epollFd);
		this->m_epollFd = -1;
	}

	if (this->m_acceptThread.joinable())
	{
		this->m_acceptThread.join();
	}

	for (auto& worker : this->m_workerThreads)
	{
		if (worker.joinable())
		{
			worker.join();
		}
	}
	this->m_workerThreads.clear();

	if (this->m_queueThread.joinable())
	{
		this->m_queueThread.join();
	}

	this->m_ServerQueue.ClearQueue();
}

bool CSocketManager::CreateListenSocket()
{
	this->m_listen = ::socket(AF_INET, SOCK_STREAM, 0);
	if (this->m_listen == INVALID_SOCKET)
	{
		gLog.Output(LOG_CONNECT, "[SocketManager] socket() failed with error: %d", WSAGetLastError());
		return false;
	}

	int opt = 1;
	setsockopt(this->m_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (!SetNonBlocking(this->m_listen))
	{
		gLog.Output(LOG_CONNECT, "[SocketManager] fcntl() failed with error: %d", WSAGetLastError());
		return false;
	}

	SOCKADDR_IN SocketAddr {};
	SocketAddr.sin_family = AF_INET;
	SocketAddr.sin_addr.s_addr = htonl(0);
	SocketAddr.sin_port = htons(this->m_port);

	if (bind(this->m_listen, (sockaddr*)&SocketAddr, sizeof(SocketAddr)) == SOCKET_ERROR)
	{
		gLog.Output(LOG_CONNECT, "[SocketManager] bind() failed with error: %d", WSAGetLastError());
		return false;
	}

	if (listen(this->m_listen, 5) == SOCKET_ERROR)
	{
		gLog.Output(LOG_CONNECT, "[SocketManager] listen() failed with error: %d", WSAGetLastError());
		return false;
	}

	return true;
}

bool CSocketManager::CreateCompletionPort()
{
	this->m_epollFd = epoll_create1(0);
	if (this->m_epollFd == -1)
	{
		gLog.Output(LOG_CONNECT, "[SocketManager] epoll_create1() failed with error: %d", WSAGetLastError());
		return false;
	}

	return true;
}

bool CSocketManager::CreateAcceptThread()
{
	this->m_acceptThread = std::thread(&CSocketManager::ServerAcceptThread, this);
	return true;
}

bool CSocketManager::CreateWorkerThread()
{
	unsigned int concurrency = std::thread::hardware_concurrency();
	this->m_ServerWorkerThreadCount = (concurrency == 0) ? 1 : std::min<unsigned int>(concurrency, MAX_SERVER_WORKER_THREAD);

	for (DWORD n = 0; n < this->m_ServerWorkerThreadCount; n++)
	{
		this->m_workerThreads.emplace_back(&CSocketManager::ServerWorkerThread, this);
	}

	return true;
}

bool CSocketManager::CreateServerQueue()
{
	this->m_queueThread = std::thread(&CSocketManager::ServerQueueThread, this);
	return true;
}

bool CSocketManager::DataRecv(int index, IO_MAIN_BUFFER* lpIoBuffer)
{
	if (lpIoBuffer->size < 3)
	{
		return true;
	}

	BYTE* lpMsg = lpIoBuffer->buff;
	int count = 0, size = 0, DecSize = 0, DecSerial = 0;
	static BYTE DecBuff[MAX_MAIN_PACKET_SIZE];
	static QUEUE_INFO QueueInfo;
	BYTE header, head;

	while (true)
	{
		if (lpMsg[count] == 0xC1 || lpMsg[count] == 0xC3)
		{
			header = lpMsg[count];
			size = lpMsg[count + 1];
			head = lpMsg[count + 2];
		}
		else if (lpMsg[count] == 0xC2 || lpMsg[count] == 0xC4)
		{
			header = lpMsg[count];
			size = MAKEWORD(lpMsg[count + 2], lpMsg[count + 1]);
			head = lpMsg[count + 3];
		}
		else
		{
			gLog.Output(LOG_CONNECT, "[SocketManager] Protocol header error (Index: %d, Header: %x)", index, lpMsg[count]);
			return false;
		}

		if (size < 3 || size > MAX_MAIN_PACKET_SIZE)
		{
			gLog.Output(LOG_CONNECT, "[SocketManager] Protocol size error (Index: %d, Header: %x, Size: %d, Head: %x)", index, header, size, head);
			return false;
		}

		if (size <= lpIoBuffer->size)
		{
			if (header == 0xC3 || header == 0xC4)
			{
				if (header == 0xC3)
				{
					DecSize = gPacketManager.Decrypt(&DecBuff[1], &lpMsg[count + 2], (size - 2)) + 1;
					DecSerial = DecBuff[1];
					header = 0xC1;
					head = DecBuff[2];
					DecBuff[0] = header;
					DecBuff[1] = DecSize;

					if (!gPacketManager.AddData(&DecBuff[0], DecSize) || !gPacketManager.ExtractPacket(DecBuff))
					{
						return false;
					}

					QueueInfo.index = index;
					QueueInfo.head = head;
					memcpy(QueueInfo.buff, DecBuff, DecSize);
					QueueInfo.size = DecSize;
					QueueInfo.encrypt = 1;
					QueueInfo.serial = DecSerial;

					if (this->m_ServerQueue.AddToQueue(&QueueInfo) != 0)
					{
						this->m_queueCv.notify_one();
					}
				}
				else
				{
					DecSize = gPacketManager.Decrypt(&DecBuff[2], &lpMsg[count + 3], (size - 3)) + 2;
					DecSerial = DecBuff[2];
					header = 0xC2;
					head = DecBuff[3];
					DecBuff[0] = header;
					DecBuff[1] = HIBYTE(DecSize);
					DecBuff[2] = LOBYTE(DecSize);

					if (!gPacketManager.AddData(DecBuff, DecSize) || !gPacketManager.ExtractPacket(DecBuff))
					{
						return false;
					}

					QueueInfo.index = index;
					QueueInfo.head = head;
					memcpy(QueueInfo.buff, DecBuff, DecSize);
					QueueInfo.size = DecSize;
					QueueInfo.encrypt = 1;
					QueueInfo.serial = DecSerial;

					if (this->m_ServerQueue.AddToQueue(&QueueInfo) != 0)
					{
						this->m_queueCv.notify_one();
					}
				}
			}
			else
			{
				if (!gPacketManager.AddData(&lpMsg[count], size) || !gPacketManager.ExtractPacket(DecBuff))
				{
					return false;
				}

				QueueInfo.index = index;
				QueueInfo.head = head;
				memcpy(QueueInfo.buff, DecBuff, size);
				QueueInfo.size = size;
				QueueInfo.encrypt = 0;
				QueueInfo.serial = -1;

				if (this->m_ServerQueue.AddToQueue(&QueueInfo) != 0)
				{
					this->m_queueCv.notify_one();
				}
			}

			count += size;
			lpIoBuffer->size -= size;

			if (lpIoBuffer->size <= 0)
			{
				break;
			}
		}
		else
		{
			if (count > 0 && lpIoBuffer->size > 0 && lpIoBuffer->size <= (MAX_MAIN_PACKET_SIZE - count))
			{
				memmove(lpMsg, &lpMsg[count], lpIoBuffer->size);
			}

			break;
		}
	}

	return true;
}

static void UpdateEpollWrite(int epollFd, SOCKET socket, int index, bool enable)
{
	epoll_event ev {};
	ev.events = EPOLLIN | EPOLLRDHUP;
	if (enable)
	{
		ev.events |= EPOLLOUT;
	}
	ev.data.u32 = static_cast<uint32_t>(index);
	epoll_ctl(epollFd, EPOLL_CTL_MOD, socket, &ev);
}

static bool FlushSendBuffer(int epollFd, int index, LPOBJ lpObj, IO_SEND_CONTEXT* lpIoContext)
{
	while (lpIoContext->IoSize > 0)
	{
		while (lpIoContext->IoMainBuffer.size < lpIoContext->IoSize)
		{
			ssize_t sent = send(lpObj->Socket,
				(char*)lpIoContext->IoMainBuffer.buff + lpIoContext->IoMainBuffer.size,
				lpIoContext->IoSize - lpIoContext->IoMainBuffer.size, 0);

			if (sent > 0)
			{
				lpIoContext->IoMainBuffer.size += sent;
				continue;
			}

			if (sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			{
				UpdateEpollWrite(epollFd, lpObj->Socket, index, true);
				return false;
			}

			gLog.Output(LOG_CONNECT, "[SocketManager] send() failed with error: %d", WSAGetLastError());
			gSocketManager.Disconnect(index);
			return false;
		}

		if (lpIoContext->IoSideBuffer.size <= 0)
		{
			lpIoContext->IoSize = 0;
			lpIoContext->IoMainBuffer.size = 0;
			UpdateEpollWrite(epollFd, lpObj->Socket, index, false);
			return true;
		}

		int chunk = (lpIoContext->IoSideBuffer.size > MAX_MAIN_PACKET_SIZE) ? MAX_MAIN_PACKET_SIZE : lpIoContext->IoSideBuffer.size;
		memcpy(lpIoContext->IoMainBuffer.buff, lpIoContext->IoSideBuffer.buff, chunk);
		memmove(lpIoContext->IoSideBuffer.buff, &lpIoContext->IoSideBuffer.buff[chunk], lpIoContext->IoSideBuffer.size - chunk);
		lpIoContext->IoSideBuffer.size -= chunk;
		lpIoContext->IoSize = chunk;
		lpIoContext->IoMainBuffer.size = 0;
	}

	UpdateEpollWrite(epollFd, lpObj->Socket, index, false);
	return true;
}

bool CSocketManager::DataSend(int index, BYTE* lpMsg, int size)
{
	this->m_critical.lock();

	if (OBJECT_USER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return 0;
	}

	if (gObj[index].Socket == INVALID_SOCKET)
	{
		this->m_critical.unlock();
		return 0;
	}

	if (gObj[index].Connected == OBJECT_OFFLINE)
	{
		this->m_critical.unlock();
		return 0;
	}

	static BYTE send[MAX_MAIN_PACKET_SIZE];
	memcpy(send, lpMsg, size);

	if (lpMsg[0] == 0xC3 || lpMsg[0] == 0xC4)
	{
		if (lpMsg[0] == 0xC3)
		{
			BYTE save = lpMsg[1];
			lpMsg[1] = gSerialCheck[index].GetSendSerial();
			size = gPacketManager.Encrypt(&send[2], &lpMsg[1], (size - 1)) + 2;
			lpMsg[1] = save;
			send[0] = 0xC3;
			send[1] = size;
		}
		else
		{
			BYTE save = lpMsg[2];
			lpMsg[2] = gSerialCheck[index].GetSendSerial();
			size = gPacketManager.Encrypt(&send[3], &lpMsg[2], (size - 2)) + 3;
			lpMsg[2] = save;
			send[0] = 0xC4;
			send[1] = HIBYTE(size);
			send[2] = LOBYTE(size);
		}
	}

	if (size > MAX_MAIN_PACKET_SIZE)
	{
		gLog.Output(LOG_CONNECT, "[SocketManager] Max msg size (Type: 1, Index: %d, Size: %d)", index, size);
		this->Disconnect(index);
		this->m_critical.unlock();
		return 0;
	}

#if(ENCRYPT_STATE==1)
	EncryptData(send, size);
#endif

	IO_SEND_CONTEXT* lpIoContext = &gObj[index].PerSocketContext->IoSendContext;

	if (lpIoContext->IoSize > 0)
	{
		if ((lpIoContext->IoSideBuffer.size + size) > MAX_SIDE_PACKET_SIZE)
		{
			gLog.Output(LOG_CONNECT, "[SocketManager] Max msg size (Type: 2, Index: %d, Size: %d)", index, (lpIoContext->IoSideBuffer.size + size));
			this->Disconnect(index);
			this->m_critical.unlock();
			return 0;
		}

		memcpy(&lpIoContext->IoSideBuffer.buff[lpIoContext->IoSideBuffer.size], send, size);
		lpIoContext->IoSideBuffer.size += size;
		this->m_critical.unlock();
		return 1;
	}

	memcpy(lpIoContext->IoMainBuffer.buff, send, size);
	lpIoContext->IoType = IO_SEND;
	lpIoContext->IoSize = size;
	lpIoContext->IoMainBuffer.size = 0;

	FlushSendBuffer(this->m_epollFd, index, &gObj[index], lpIoContext);

	this->m_critical.unlock();
	return 1;
}

void CSocketManager::Disconnect(int index)
{
	this->m_critical.lock();

	if (OBJECT_USER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return;
	}

	if (gObj[index].Socket == INVALID_SOCKET)
	{
		this->m_critical.unlock();
		return;
	}

	if (gObj[index].Connected == OBJECT_OFFLINE)
	{
		this->m_critical.unlock();
		return;
	}

	epoll_ctl(this->m_epollFd, EPOLL_CTL_DEL, gObj[index].Socket, nullptr);

	if (closesocket(gObj[index].Socket) == SOCKET_ERROR && WSAGetLastError() != WSAENOTSOCK)
	{
		gLog.Output(LOG_CONNECT, "[SocketManager] closesocket() failed with error: %d", WSAGetLastError());
		this->m_critical.unlock();
		return;
	}

	gObj[index].Socket = INVALID_SOCKET;
	gObjDel(index);

	this->m_critical.unlock();
}

void CSocketManager::OnRecv(int index, DWORD, IO_RECV_CONTEXT* lpIoContext)
{
	this->m_critical.lock();

	if (OBJECT_USER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return;
	}

	if (gObj[index].Socket == INVALID_SOCKET || gObj[index].Connected == OBJECT_OFFLINE)
	{
		this->m_critical.unlock();
		return;
	}

	while (true)
	{
		int capacity = MAX_MAIN_PACKET_SIZE - lpIoContext->IoMainBuffer.size;
		if (capacity <= 0)
		{
			this->Disconnect(index);
			this->m_critical.unlock();
			return;
		}

		ssize_t received = recv(gObj[index].Socket, &lpIoContext->IoMainBuffer.buff[lpIoContext->IoMainBuffer.size], capacity, 0);

		if (received > 0)
		{
#if(ENCRYPT_STATE==1)
			DecryptData(&lpIoContext->IoMainBuffer.buff[lpIoContext->IoMainBuffer.size], received);
#endif

			lpIoContext->IoMainBuffer.size += received;

			if (this->DataRecv(index, &lpIoContext->IoMainBuffer) == 0)
			{
				this->Disconnect(index);
				this->m_critical.unlock();
				return;
			}

			continue;
		}

		if (received == 0)
		{
			this->Disconnect(index);
			this->m_critical.unlock();
			return;
		}

		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			break;
		}

		gLog.Output(LOG_CONNECT, "[SocketManager] recv() failed with error: %d", WSAGetLastError());
		this->Disconnect(index);
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

void CSocketManager::OnSend(int index, DWORD, IO_SEND_CONTEXT* lpIoContext)
{
	this->m_critical.lock();

	if (OBJECT_USER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return;
	}

	if (gObj[index].Socket == INVALID_SOCKET || gObj[index].Connected == OBJECT_OFFLINE)
	{
		this->m_critical.unlock();
		return;
	}

	FlushSendBuffer(this->m_epollFd, index, &gObj[index], lpIoContext);

	this->m_critical.unlock();
}

DWORD CSocketManager::ServerAcceptThread(CSocketManager* lpSocketManager)
{
	while (lpSocketManager->m_running)
	{
		SOCKADDR_IN SocketAddr {};
		socklen_t SocketAddrSize = sizeof(SocketAddr);
		SOCKET socket = accept(lpSocketManager->m_listen, (sockaddr*)&SocketAddr, &SocketAddrSize);

		if (socket == SOCKET_ERROR)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				Sleep(1);
				continue;
			}

			lpSocketManager->m_critical.lock();
			gLog.Output(LOG_CONNECT, "[SocketManager] accept() failed with error: %d", WSAGetLastError());
			lpSocketManager->m_critical.unlock();
			continue;
		}

		if (!SetNonBlocking(socket))
		{
			closesocket(socket);
			continue;
		}

		char IPAddress[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, &SocketAddr.sin_addr, IPAddress, INET_ADDRSTRLEN) == NULL)
		{
			closesocket(socket);
			continue;
		}

		if (gIpManager.CheckIpAddress(IPAddress) == 0)
		{
			closesocket(socket);
			continue;
		}

		lpSocketManager->m_critical.lock();

		int index = gObjAddSearch(socket, IPAddress);
		if (index == -1)
		{
			closesocket(socket);
			lpSocketManager->m_critical.unlock();
			continue;
		}

		if (gObjAdd(socket, IPAddress, index) == -1)
		{
			closesocket(socket);
			lpSocketManager->m_critical.unlock();
			continue;
		}

		LPOBJ lpObj = &gObj[index];
		lpObj->PerSocketContext->Socket = socket;
		lpObj->PerSocketContext->Index = index;

		lpObj->PerSocketContext->IoRecvContext.IoType = IO_RECV;
		lpObj->PerSocketContext->IoRecvContext.IoSize = 0;
		lpObj->PerSocketContext->IoRecvContext.IoMainBuffer.size = 0;

		lpObj->PerSocketContext->IoSendContext.IoType = IO_SEND;
		lpObj->PerSocketContext->IoSendContext.IoSize = 0;
		lpObj->PerSocketContext->IoSendContext.IoMainBuffer.size = 0;
		lpObj->PerSocketContext->IoSendContext.IoSideBuffer.size = 0;

		epoll_event ev {};
		ev.events = EPOLLIN | EPOLLRDHUP;
		ev.data.u32 = static_cast<uint32_t>(index);
		epoll_ctl(lpSocketManager->m_epollFd, EPOLL_CTL_ADD, socket, &ev);

		GCConnectClientSend(index, 1);

		lpSocketManager->m_critical.unlock();
	}

	return 0;
}

DWORD CSocketManager::ServerWorkerThread(CSocketManager* lpSocketManager)
{
	epoll_event events[64];

	while (lpSocketManager->m_running)
	{
		int count = epoll_wait(lpSocketManager->m_epollFd, events, 64, 100);
		if (count <= 0)
		{
			continue;
		}

		for (int i = 0; i < count; ++i)
		{
			int index = static_cast<int>(events[i].data.u32);
			if (OBJECT_USER_RANGE(index) == 0)
			{
				continue;
			}

			if (events[i].events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP))
			{
				lpSocketManager->Disconnect(index);
				continue;
			}

			if (events[i].events & EPOLLIN)
			{
				lpSocketManager->OnRecv(index, 0, &gObj[index].PerSocketContext->IoRecvContext);
			}

			if (events[i].events & EPOLLOUT)
			{
				lpSocketManager->OnSend(index, 0, &gObj[index].PerSocketContext->IoSendContext);
			}
		}
	}

	return 0;
}

DWORD CSocketManager::ServerQueueThread(CSocketManager* lpSocketManager)
{
	while (lpSocketManager->m_running)
	{
		std::unique_lock<std::mutex> lock(lpSocketManager->m_queueMutex);
		lpSocketManager->m_queueCv.wait(lock, [&]()
		{
			return lpSocketManager->m_queueStop || lpSocketManager->m_ServerQueue.GetQueueSize() > 0;
		});

		if (lpSocketManager->m_queueStop)
		{
			break;
		}

		lock.unlock();

		static QUEUE_INFO QueueInfo;
		while (lpSocketManager->m_ServerQueue.GetFromQueue(&QueueInfo) != 0)
		{
			if (OBJECT_RANGE(QueueInfo.index) != 0 && gObj[QueueInfo.index].Connected != OBJECT_OFFLINE)
			{
				ProtocolCore(QueueInfo.head, QueueInfo.buff, QueueInfo.size, QueueInfo.index, QueueInfo.encrypt, QueueInfo.serial);
			}
		}
	}

	return 0;
}

DWORD CSocketManager::GetQueueSize()
{
	return this->m_ServerQueue.GetQueueSize();
}

#endif
