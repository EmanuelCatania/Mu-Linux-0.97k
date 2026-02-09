#include "stdafx.h"
#include "SocketManager.h"
#include "DataServerProtocol.h"
#include "ServerManager.h"
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

	LogAdd(LOG_GREEN, "[SocketManager] Server started at port [%d]", this->m_port);
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
		LogAdd(LOG_RED, "[SocketManager] socket() failed with error: %d", WSAGetLastError());
		return false;
	}

	int opt = 1;
	setsockopt(this->m_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (!SetNonBlocking(this->m_listen))
	{
		LogAdd(LOG_RED, "[SocketManager] fcntl() failed with error: %d", WSAGetLastError());
		return false;
	}

	SOCKADDR_IN SocketAddr {};
	SocketAddr.sin_family = AF_INET;
	SocketAddr.sin_addr.s_addr = htonl(0);
	SocketAddr.sin_port = htons(this->m_port);

	if (bind(this->m_listen, (sockaddr*)&SocketAddr, sizeof(SocketAddr)) == SOCKET_ERROR)
	{
		LogAdd(LOG_RED, "[SocketManager] bind() failed with error: %d", WSAGetLastError());
		return false;
	}

	if (listen(this->m_listen, 5) == SOCKET_ERROR)
	{
		LogAdd(LOG_RED, "[SocketManager] listen() failed with error: %d", WSAGetLastError());
		return false;
	}

	return true;
}

bool CSocketManager::CreateCompletionPort()
{
	this->m_epollFd = epoll_create1(0);
	if (this->m_epollFd == -1)
	{
		LogAdd(LOG_RED, "[SocketManager] epoll_create1() failed with error: %d", WSAGetLastError());
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
			LogAdd(LOG_RED, "[SocketManager] Protocol header error (Index: %d, Header: %x)", index, lpMsg[count]);
			return false;
		}

		if (size < 3 || size > MAX_MAIN_PACKET_SIZE)
		{
			LogAdd(LOG_RED, "[SocketManager] Protocol size error (Index: %d, Header: %x, Size: %d, Head: %x)", index, header, size, head);
			return false;
		}

		if (size <= lpIoBuffer->size)
		{
			static QUEUE_INFO QueueInfo;
			QueueInfo.index = index;
			QueueInfo.head = head;
			memcpy(QueueInfo.buff, &lpMsg[count], size);
			QueueInfo.size = size;

			if (this->m_ServerQueue.AddToQueue(&QueueInfo) != false)
			{
				this->m_queueCv.notify_one();
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

static bool FlushSendBuffer(int epollFd, int index, CServerManager* lpServerManager, IO_SEND_CONTEXT* lpIoContext)
{
	while (lpIoContext->IoSize > 0)
	{
		while (lpIoContext->IoMainBuffer.size < lpIoContext->IoSize)
		{
			ssize_t sent = send(lpServerManager->m_socket,
				(char*)lpIoContext->IoMainBuffer.buff + lpIoContext->IoMainBuffer.size,
				lpIoContext->IoSize - lpIoContext->IoMainBuffer.size, 0);

			if (sent > 0)
			{
				lpIoContext->IoMainBuffer.size += sent;
				continue;
			}

			if (sent == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
			{
				UpdateEpollWrite(epollFd, lpServerManager->m_socket, index, true);
				return false;
			}

			LogAdd(LOG_RED, "[SocketManager] send() failed with error: %d", WSAGetLastError());
			gSocketManager.Disconnect(index);
			return false;
		}

		if (lpIoContext->IoSideBuffer.size <= 0)
		{
			lpIoContext->IoSize = 0;
			lpIoContext->IoMainBuffer.size = 0;
			UpdateEpollWrite(epollFd, lpServerManager->m_socket, index, false);
			return true;
		}

		int chunk = (lpIoContext->IoSideBuffer.size > MAX_MAIN_PACKET_SIZE) ? MAX_MAIN_PACKET_SIZE : lpIoContext->IoSideBuffer.size;
		memcpy(lpIoContext->IoMainBuffer.buff, lpIoContext->IoSideBuffer.buff, chunk);
		memmove(lpIoContext->IoSideBuffer.buff, &lpIoContext->IoSideBuffer.buff[chunk], lpIoContext->IoSideBuffer.size - chunk);
		lpIoContext->IoSideBuffer.size -= chunk;
		lpIoContext->IoSize = chunk;
		lpIoContext->IoMainBuffer.size = 0;
	}

	UpdateEpollWrite(epollFd, lpServerManager->m_socket, index, false);
	return true;
}

bool CSocketManager::DataSend(int index, BYTE* lpMsg, int size)
{
	ConsoleProtocolLog(CON_PROTO_TCP_SEND, lpMsg, size);

	this->m_critical.lock();

	if (SERVER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return false;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	if (lpServerManager->CheckState() == false)
	{
		this->m_critical.unlock();
		return false;
	}

	if (size > MAX_MAIN_PACKET_SIZE)
	{
		LogAdd(LOG_RED, "[SocketManager] Max msg size (Type: 1, Index: %d, Size: %d)", index, size);
		this->m_critical.unlock();
		return false;
	}

	IO_SEND_CONTEXT* lpIoContext = lpServerManager->m_IoSendContext;
	if (lpIoContext == nullptr)
	{
		this->m_critical.unlock();
		return false;
	}

	if (lpIoContext->IoSize > 0)
	{
		if ((lpIoContext->IoSideBuffer.size + size) > MAX_SIDE_PACKET_SIZE)
		{
			LogAdd(LOG_RED, "[SocketManager] Max msg size (Type: 2, Index: %d, Size: %d)", index, (lpIoContext->IoSideBuffer.size + size));
			this->m_critical.unlock();
			return false;
		}

		memcpy(&lpIoContext->IoSideBuffer.buff[lpIoContext->IoSideBuffer.size], lpMsg, size);
		lpIoContext->IoSideBuffer.size += size;
		this->m_critical.unlock();
		return true;
	}

	memcpy(lpIoContext->IoMainBuffer.buff, lpMsg, size);
	lpIoContext->IoSize = size;
	lpIoContext->IoMainBuffer.size = 0;

	FlushSendBuffer(this->m_epollFd, index, lpServerManager, lpIoContext);

	this->m_critical.unlock();
	return true;
}

void CSocketManager::Disconnect(int index)
{
	this->m_critical.lock();

	if (SERVER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	if (lpServerManager->CheckState() == false)
	{
		this->m_critical.unlock();
		return;
	}

	epoll_ctl(this->m_epollFd, EPOLL_CTL_DEL, lpServerManager->m_socket, nullptr);

	if (closesocket(lpServerManager->m_socket) == SOCKET_ERROR && WSAGetLastError() != WSAENOTSOCK)
	{
		LogAdd(LOG_RED, "[SocketManager] closesocket() failed with error: %d", WSAGetLastError());
		this->m_critical.unlock();
		return;
	}

	lpServerManager->m_socket = INVALID_SOCKET;
	lpServerManager->DelServer();

	this->m_critical.unlock();
}

void CSocketManager::OnRecv(int index, DWORD, IO_RECV_CONTEXT* lpIoContext)
{
	this->m_critical.lock();

	if (SERVER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	if (lpServerManager->CheckState() == false)
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

		ssize_t received = recv(lpServerManager->m_socket, &lpIoContext->IoMainBuffer.buff[lpIoContext->IoMainBuffer.size], capacity, 0);

		if (received > 0)
		{
			lpIoContext->IoMainBuffer.size += received;

			if (this->DataRecv(index, &lpIoContext->IoMainBuffer) == false)
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

		LogAdd(LOG_RED, "[SocketManager] recv() failed with error: %d", WSAGetLastError());
		this->Disconnect(index);
		this->m_critical.unlock();
		return;
	}

	this->m_critical.unlock();
}

void CSocketManager::OnSend(int index, DWORD, IO_SEND_CONTEXT* lpIoContext)
{
	this->m_critical.lock();

	if (SERVER_RANGE(index) == 0)
	{
		this->m_critical.unlock();
		return;
	}

	CServerManager* lpServerManager = &gServerManager[index];

	if (lpServerManager->CheckState() == false)
	{
		this->m_critical.unlock();
		return;
	}

	FlushSendBuffer(this->m_epollFd, index, lpServerManager, lpIoContext);

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

			LogAdd(LOG_RED, "[SocketManager] accept() failed with error: %d", WSAGetLastError());
			continue;
		}

		if (!SetNonBlocking(socket))
		{
			closesocket(socket);
			continue;
		}

		lpSocketManager->m_critical.lock();

		int index = GetFreeServerIndex();
		if (index == -1)
		{
			lpSocketManager->m_critical.unlock();
			closesocket(socket);
			continue;
		}

		char IPAddress[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, &SocketAddr.sin_addr, IPAddress, INET_ADDRSTRLEN) == NULL)
		{
			LogAdd(LOG_RED, "[SocketManager] inet_ntop() failed with error: %d", WSAGetLastError());
			lpSocketManager->m_critical.unlock();
			closesocket(socket);
			continue;
		}

		CServerManager* lpServerManager = &gServerManager[index];
		lpServerManager->AddServer(index, IPAddress, socket);

		epoll_event ev {};
		ev.events = EPOLLIN | EPOLLRDHUP;
		ev.data.u32 = static_cast<uint32_t>(index);
		epoll_ctl(lpSocketManager->m_epollFd, EPOLL_CTL_ADD, socket, &ev);

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
			if (SERVER_RANGE(index) == 0)
			{
				continue;
			}

			CServerManager* lpServerManager = &gServerManager[index];
			if (lpServerManager->CheckState() == false)
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
				lpSocketManager->OnRecv(index, 0, lpServerManager->m_IoRecvContext);
			}

			if (events[i].events & EPOLLOUT)
			{
				lpSocketManager->OnSend(index, 0, lpServerManager->m_IoSendContext);
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
		while (lpSocketManager->m_ServerQueue.GetFromQueue(&QueueInfo) != false)
		{
			if (SERVER_RANGE(QueueInfo.index) != 0 && gServerManager[QueueInfo.index].CheckState() != false)
			{
				DataServerProtocolCore(QueueInfo.index, QueueInfo.head, QueueInfo.buff, QueueInfo.size);
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
