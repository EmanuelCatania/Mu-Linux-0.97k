#include "stdafx.h"
#include "GameMain.h"
#include "JSProtocol.h"
#include "MiniDump.h"
#include "QueueTimer.h"
#include "ServerDisplayer.h"
#include "ServerInfo.h"
#include "Path.h"
#include "SocketManager.h"
#include "SocketManagerUdp.h"
#include "Util.h"

void CheckEditorReload()
{
	static DWORD lastCheck = 0;
	DWORD now = GetTickCount();
	if ((now - lastCheck) < 10000)
	{
		return;
	}
	lastCheck = now;

	const char* flagPath = gPath.GetFullPath("EditorReload.flag");
	FILE* file;
	if (fopen_s(&file, flagPath, "r") != 0)
	{
		return;
	}

	char data[512] = { 0 };
	size_t bytes = fread(data, 1, sizeof(data) - 1, file);
	fclose(file);
	remove(flagPath);

	if (bytes == 0)
	{
		return;
	}

	for (size_t i = 0; i < bytes; i++)
	{
		data[i] = (char)tolower((unsigned char)data[i]);
	}

	const char* delimiters = " ,;\t\r\n";
	char* token = strtok(data, delimiters);
	while (token != 0)
	{
		if (_stricmp(token, "all") == 0)
		{
			gServerInfo.ReloadAll();
			LogAdd(LOG_BLUE, "[ServerInfo] ReloadAll by editor flag");
		}
		else if (_stricmp(token, "common") == 0)
		{
			gServerInfo.ReadCommonInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Common reloaded by editor flag");
		}
		else if (_stricmp(token, "shop") == 0)
		{
			gServerInfo.ReadShopInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Shop reloaded by editor flag");
		}
		else if (_stricmp(token, "monster") == 0)
		{
			gServerInfo.ReloadMonsterInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Monster reloaded by editor flag");
		}
		else if (_stricmp(token, "move") == 0 || _stricmp(token, "gate") == 0)
		{
			gServerInfo.ReadMoveInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Move reloaded by editor flag");
		}
		else if (_stricmp(token, "quest") == 0)
		{
			gServerInfo.ReadQuestInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Quest reloaded by editor flag");
		}
		else if (_stricmp(token, "util") == 0)
		{
			gServerInfo.ReadUtilInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Util reloaded by editor flag");
		}
		else if (_stricmp(token, "item") == 0)
		{
			gServerInfo.ReadItemInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Item reloaded by editor flag");
		}
		else if (_stricmp(token, "skill") == 0)
		{
			gServerInfo.ReadSkillInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Skill reloaded by editor flag");
		}
		else if (_stricmp(token, "event") == 0)
		{
			gServerInfo.ReadEventInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Event reloaded by editor flag");
		}
		else if (_stricmp(token, "eventitembag") == 0)
		{
			gServerInfo.ReadEventItemBagInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] EventItemBag reloaded by editor flag");
		}
		else if (_stricmp(token, "chaosmix") == 0)
		{
			gServerInfo.ReadChaosMixInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] ChaosMix reloaded by editor flag");
		}
		else if (_stricmp(token, "command") == 0)
		{
			gServerInfo.ReadCommandInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Command reloaded by editor flag");
		}
		else if (_stricmp(token, "custom") == 0)
		{
			gServerInfo.ReadCustomInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Custom reloaded by editor flag");
		}
		else if (_stricmp(token, "character") == 0)
		{
			gServerInfo.ReadCharacterInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Character reloaded by editor flag");
		}
		else if (_stricmp(token, "map") == 0)
		{
			gServerInfo.ReadMapInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Map reloaded by editor flag");
		}
		else if (_stricmp(token, "hack") == 0)
		{
			gServerInfo.ReadHackInfo();
			LogAdd(LOG_BLUE, "[ServerInfo] Hack reloaded by editor flag");
		}

		token = strtok(0, delimiters);
	}
}

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
			CheckEditorReload();
			nextSlow = now + std::chrono::seconds(10);
		}

		Sleep(1);
	}

	CMiniDump::Clean();

	return 0;
}
