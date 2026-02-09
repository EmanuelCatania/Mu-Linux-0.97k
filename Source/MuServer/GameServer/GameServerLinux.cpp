#include "stdafx.h"
#include "GameMain.h"
#include "JSProtocol.h"
#include "MiniDump.h"
#include "QueueTimer.h"
#include "ServerDisplayer.h"
#include "ServerInfo.h"
#include "SocketManager.h"
#include "SocketManagerUdp.h"
#include "Util.h"

int main()
{
	setlocale(LC_ALL, "C");

	CMiniDump::Start();

	SetLargeRand();

	gServerInfo.ReadStartupInfo("GameServerInfo", "./Data/GameServerInfo - StartUp.dat");

	gServerDisplayer.Init(nullptr);

	WSADATA wsa {};

	if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0)
	{
		if (gSocketManager.Start((WORD)gServerInfo.m_ServerPort) == 0)
		{
			LogAdd(LOG_RED, "Could not start GameServer");
		}
		else
		{
			GameMainInit(nullptr);

			JoinServerConnect();

			DataServerConnect();

			gSocketManagerUdp.Connect(gServerInfo.m_ConnectServerAddress, (WORD)gServerInfo.m_ConnectServerPort);

			gQueueTimer.CreateTimer(QUEUE_TIMER_MONSTER, 100, &QueueTimerCallback);
			gQueueTimer.CreateTimer(QUEUE_TIMER_MONSTER_MOVE, 100, &QueueTimerCallback);
			gQueueTimer.CreateTimer(QUEUE_TIMER_EVENT, 100, &QueueTimerCallback);
			gQueueTimer.CreateTimer(QUEUE_TIMER_VIEWPORT, 1000, &QueueTimerCallback);
			gQueueTimer.CreateTimer(QUEUE_TIMER_FIRST, 1000, &QueueTimerCallback);
			gQueueTimer.CreateTimer(QUEUE_TIMER_CLOSE, 1000, &QueueTimerCallback);
			gQueueTimer.CreateTimer(QUEUE_TIMER_ACCOUNT_LEVEL, 60000, &QueueTimerCallback);
		}
	}
	else
	{
		LogAdd(LOG_RED, "WSAStartup() failed with error: %d", WSAGetLastError());
	}

	auto nextFast = std::chrono::steady_clock::now();
	auto nextSlow = std::chrono::steady_clock::now();

	while (true)
	{
		auto now = std::chrono::steady_clock::now();
		if (now >= nextFast)
		{
			GJServerUserInfoSend();
			ConnectServerInfoSend();
			nextFast = now + std::chrono::seconds(1);
		}

		if (now >= nextSlow)
		{
			JoinServerReconnect(nullptr);
			DataServerReconnect(nullptr);
			nextSlow = now + std::chrono::seconds(10);
		}

		Sleep(1);
	}

	CMiniDump::Clean();

	return 0;
}
