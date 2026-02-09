#include "stdafx.h"
#include "AllowableIpList.h"
#include "MiniDump.h"
#include "JoinServerProtocol.h"
#include "QueryManager.h"
#include "ServerDisplayer.h"
#include "SocketManager.h"
#include "SocketManagerUdp.h"
#include "Util.h"

BOOL CaseSensitive = 0;
int MD5Encryption = 0;
char GlobalPassword[11] = { 0 };

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

		GetPrivateProfileString("DataBaseInfo", "DataBaseODBC", "", DataBaseODBC, sizeof(DataBaseODBC), "./JoinServer.ini");
		GetPrivateProfileString("DataBaseInfo", "DataBaseUser", "", DataBaseUser, sizeof(DataBaseUser), "./JoinServer.ini");
		GetPrivateProfileString("DataBaseInfo", "DataBasePass", "", DataBasePass, sizeof(DataBasePass), "./JoinServer.ini");
#else
		char DataBaseHost[64] = { 0 };
		WORD DataBasePort = 3306;
		char DataBaseUser[32] = { 0 };
		char DataBasePass[32] = { 0 };
		char DataBaseName[32] = { 0 };

		GetPrivateProfileString("DataBaseInfo", "DataBaseHost", "", DataBaseHost, sizeof(DataBaseHost), "./JoinServer.ini");
		DataBasePort = GetPrivateProfileInt("DataBaseInfo", "DataBasePort", 3306, "./JoinServer.ini");
		GetPrivateProfileString("DataBaseInfo", "DataBaseUser", "", DataBaseUser, sizeof(DataBaseUser), "./JoinServer.ini");
		GetPrivateProfileString("DataBaseInfo", "DataBasePass", "", DataBasePass, sizeof(DataBasePass), "./JoinServer.ini");
		GetPrivateProfileString("DataBaseInfo", "DataBaseName", "", DataBaseName, sizeof(DataBaseName), "./JoinServer.ini");
#endif

		WORD JS_TCP_Port = GetPrivateProfileInt("JoinServerInfo", "JS_TCP_Port", 55970, "./JoinServer.ini");

		char ConnectServerAddress[16] = { 0 };
		GetPrivateProfileString("JoinServerInfo", "ConnectServerAddress", "127.0.0.1", ConnectServerAddress, sizeof(ConnectServerAddress), "./JoinServer.ini");
		WORD ConnectServerUDPPort = GetPrivateProfileInt("JoinServerInfo", "ConnectServerUDPPort", 55557, "./JoinServer.ini");

		CaseSensitive = GetPrivateProfileInt("AccountInfo", "CaseSensitive", 0, "./JoinServer.ini");
		MD5Encryption = GetPrivateProfileInt("AccountInfo", "MD5Encryption", 0, "./JoinServer.ini");
		GetPrivateProfileString("AccountInfo", "GlobalPassword", "XwefDastoD", GlobalPassword, sizeof(GlobalPassword), "./JoinServer.ini");

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
			if (gSocketManager.Start(JS_TCP_Port) == false)
			{
				gQueryManager.Disconnect();
			}
			else
			{
				if (gSocketManagerUdp.Connect(ConnectServerAddress, ConnectServerUDPPort) == false)
				{
					gSocketManager.Clean();
					gQueryManager.Disconnect();
				}
				else
				{
					gAllowableIpList.Load("AllowableIpList.txt");
				}
			}
		}
	}
	else
	{
		LogAdd(LOG_RED, "WSAStartup() failed with error: %d", WSAGetLastError());
	}

	auto nextTick = std::chrono::steady_clock::now();

	while (true)
	{
		auto now = std::chrono::steady_clock::now();
		if (now >= nextTick)
		{
			JoinServerLiveProc();
			nextTick = now + std::chrono::seconds(1);
		}

		Sleep(1);
	}

	CMiniDump::Clean();

	return 0;
}
