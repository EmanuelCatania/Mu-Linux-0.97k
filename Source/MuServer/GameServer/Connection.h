#pragma once

#include "CriticalSection.h"

#define MAX_BUFF_SIZE 524288

#include <atomic>
#include <thread>

class CConnection
{
public:

	CConnection();

	virtual ~CConnection();

	using ProtocolCoreFn = void (*)(BYTE, BYTE*, int);

	void Init(HWND hwnd, const char* name, ProtocolCoreFn function);

	bool Connect(const char* IpAddress, WORD port);

	void Disconnect();

	bool CheckState();

	bool DataRecv();

	bool DataSend(BYTE* lpMsg, int size);

	bool DataSendEx();

private:

	bool CreateEventHandler();

	static DWORD WINAPI SocketEventsThread(CConnection* lpConnection);

private:

	std::string sConnectionName;

	HWND m_hwnd;

	SOCKET m_socket;

	HANDLE m_EventHandlerThread;

	WSAEVENT m_hEvent;

	BYTE m_RecvBuff[MAX_BUFF_SIZE];

	int m_RecvSize;

	BYTE m_SendBuff[MAX_BUFF_SIZE];

	int m_SendSize;

	ProtocolCoreFn wsProtocolCore;

	CCriticalSection m_critical;

#ifndef _WIN32
	std::atomic<bool> m_running;
	std::thread m_eventThread;
#endif
};
