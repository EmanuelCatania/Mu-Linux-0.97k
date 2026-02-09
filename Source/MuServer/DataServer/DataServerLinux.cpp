#include "stdafx.h"
#include "AllowableIpList.h"
#include "BadSyntax.h"
#include "GuildManager.h"
#include "MiniDump.h"
#include "QueryManager.h"
#include "ServerDisplayer.h"
#include "SocketManager.h"
#include "Util.h"

int main()
{
	setlocale(LC_ALL, "C");

	CMiniDump::Start();

	gServerDisplayer.Init(nullptr);

	WSADATA wsa {};

	if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0)
	{
#ifndef MYSQL
		char DataBaseODBC[64] = { 0 };
		char DataBaseUser[32] = { 0 };
		char DataBasePass[32] = { 0 };

		GetPrivateProfileString("DataBaseInfo", "DataBaseODBC", "", DataBaseODBC, sizeof(DataBaseODBC), "./DataServer.ini");
		GetPrivateProfileString("DataBaseInfo", "DataBaseUser", "", DataBaseUser, sizeof(DataBaseUser), "./DataServer.ini");
		GetPrivateProfileString("DataBaseInfo", "DataBasePass", "", DataBasePass, sizeof(DataBasePass), "./DataServer.ini");
#else
		char DataBaseHost[64] = { 0 };
		WORD DataBasePort = 3306;
		char DataBaseUser[32] = { 0 };
		char DataBasePass[32] = { 0 };
		char DataBaseName[32] = { 0 };

		GetPrivateProfileString("DataBaseInfo", "DataBaseHost", "", DataBaseHost, sizeof(DataBaseHost), "./DataServer.ini");
		DataBasePort = GetPrivateProfileInt("DataBaseInfo", "DataBasePort", 3306, "./DataServer.ini");
		GetPrivateProfileString("DataBaseInfo", "DataBaseUser", "", DataBaseUser, sizeof(DataBaseUser), "./DataServer.ini");
		GetPrivateProfileString("DataBaseInfo", "DataBasePass", "", DataBasePass, sizeof(DataBasePass), "./DataServer.ini");
		GetPrivateProfileString("DataBaseInfo", "DataBaseName", "", DataBaseName, sizeof(DataBaseName), "./DataServer.ini");
#endif

		WORD DS_TCP_Port = GetPrivateProfileInt("DataServerInfo", "DS_TCP_Port", 55960, "./DataServer.ini");

#ifndef MYSQL
		if (gQueryManager.Connect(DataBaseODBC, DataBaseUser, DataBasePass) == false)
#else
		if (gQueryManager.Init(DataBaseHost, DataBasePort, DataBaseUser, DataBasePass, DataBaseName) == false)
#endif
		{
			LogAdd(LOG_RED, "Could not connect to database");
		}
		else
		{
			if (gSocketManager.Start(DS_TCP_Port) == false)
			{
				gQueryManager.Disconnect();
			}
			else
			{
				gAllowableIpList.Load("AllowableIpList.txt");
				gBadSyntax.Load("BadSyntax.txt");
				gGuildManager.Init();
			}
		}
	}
	else
	{
		LogAdd(LOG_RED, "WSAStartup() failed with error: %d", WSAGetLastError());
	}

	while (true)
	{
		Sleep(1);
	}

	CMiniDump::Clean();

	return 0;
}
