#include "stdafx.h"
#include "JoinServerProtocol.h"
#include "MD5.h"
#include "AccountManager.h"
#include "Log.h"
#include "QueryManager.h"
#include "ServerManager.h"
#include "SocketManager.h"
#include "SocketManagerUdp.h"
#include "Util.h"
#include "../Common/PasswordHash.h"

// Contraseñas con bcrypt (Fase 5). Lo que ya está guardado como hash se
// verifica con bcrypt; lo viejo (texto o MD5, según MD5Encryption) se verifica
// como siempre y, si entra, se reemplaza por el hash en ese mismo login.
static int PasswordHashEnabled = 1;
static int PasswordHashCost = PasswordHash::DefaultCost;
static bool PasswordColumnWide = false;

// La contraseña global entra a cualquier cuenta: vacía queda apagada. Antes
// venía con un valor por defecto conocido, y con la global vacía una
// contraseña vacía habría entrado a todas.
static bool IsGlobalPassword(const char* password)
{
	return GlobalPassword[0] != 0 && strcmp(password, GlobalPassword) == 0;
}

void PasswordHashSetup(const char* path)
{
	PasswordHashEnabled = GetPrivateProfileInt("AccountInfo", "PasswordHash", 1, path);

	PasswordHashCost = GetPrivateProfileInt("AccountInfo", "PasswordHashCost", PasswordHash::DefaultCost, path);

	if (PasswordHashCost < PasswordHash::MinCost || PasswordHashCost > PasswordHash::MaxCost)
	{
		PasswordHashCost = PasswordHash::DefaultCost;
	}

	int length = 0;

#ifndef MYSQL
	if (gQueryManager.ExecQuery("SELECT COL_LENGTH('MEMB_INFO', 'memb__pwd') AS len") != false && gQueryManager.Fetch() != SQL_NO_DATA)
#else
	if (gQueryManager.ExecResultQuery("SELECT CHARACTER_OCTET_LENGTH AS len FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = 'MEMB_INFO' AND COLUMN_NAME = 'memb__pwd'") != false && gQueryManager.Fetch() != false)
#endif
	{
		length = gQueryManager.GetAsInteger("len");
	}

	gQueryManager.Close();

	// Sin lugar para los 60 caracteres el hash quedaría cortado y la cuenta sin
	// poder entrar: en ese caso no se migra nada.
	PasswordColumnWide = length >= PasswordHash::HashLength;

	if (PasswordHashEnabled != 0 && PasswordColumnWide == false)
	{
		LogAdd(LOG_RED, "[PasswordHash] memb__pwd has %d bytes, 60 needed: run Migrate-password-hash.sql. Passwords are not migrated until then.", length);
	}
}

// Reemplaza la contraseña vieja por el hash bcrypt. Si falla, la cuenta sigue
// entrando con la contraseña vieja y se reintenta en el próximo login.
static void PasswordHashUpgrade(const char* account, const char* password)
{
	if (PasswordHashEnabled == 0 || PasswordColumnWide == false)
	{
		return;
	}

	char hash[PasswordHash::HashLength + 1];

	if (PasswordHash::Make(password, PasswordHashCost, hash, sizeof(hash)) == false)
	{
		LogAdd(LOG_RED, "[PasswordHash] Could not hash the password of '%s'", account);

		return;
	}

	// El hash sólo tiene [./$0-9A-Za-z]: no hace falta escaparlo.
#ifndef MYSQL
	gQueryManager.ExecQuery("UPDATE MEMB_INFO SET memb__pwd = CONVERT(VARBINARY(60), '%s') WHERE memb___id = '%s'", hash, account);
#else
	gQueryManager.ExecUpdateQuery("UPDATE MEMB_INFO SET memb__pwd = '%s' WHERE memb___id = '%s'", hash, account);
#endif

	gQueryManager.Close();
}

void JoinServerProtocolCore(int index, BYTE head, BYTE* lpMsg, int size)
{
	ConsoleProtocolLog(CON_PROTO_TCP_RECV, lpMsg, size);

	gServerManager[index].m_PacketTime = GetTickCount();

	switch (head)
	{
		case 0x00:
		{
			GJServerInfoRecv((SDHP_SERVER_INFO_RECV*)lpMsg, index);

			break;
		}

		case 0x01:
		{
			GJConnectAccountRecv((SDHP_CONNECT_ACCOUNT_RECV*)lpMsg, index);

			break;
		}

		case 0x02:
		{
			GJDisconnectAccountRecv((SDHP_DISCONNECT_ACCOUNT_RECV*)lpMsg, index);

			break;
		}

		case 0x03:
		{
			GJAccountLevelRecv((SDHP_ACCOUNT_LEVEL_RECV*)lpMsg, index);

			break;
		}

		case 0x04:
		{
			GJAccountLevelSaveRecv((SDHP_ACCOUNT_LEVEL_SAVE_RECV*)lpMsg, index);

			break;
		}

		case 0x06:
		{
			GJServerUserInfoRecv((SDHP_SERVER_USER_INFO_RECV*)lpMsg, index);

			break;
		}

		case 0x07:
		{
			GJAccountCountRecv((SDHP_COUNT_ONLINE_USER_RECV*)lpMsg, index);

			break;
		}
	}
}

void JoinServerLiveProc()
{
	SDHP_JOIN_SERVER_LIVE_SEND pMsg;

	pMsg.header.set(0x02, sizeof(pMsg));

	pMsg.QueueSize = gSocketManager.GetQueueSize();

	gSocketManagerUdp.DataSend((BYTE*)&pMsg, pMsg.header.size);
}

void GJServerInfoRecv(SDHP_SERVER_INFO_RECV* lpMsg, int index)
{
	gServerManager[index].SetServerInfo(lpMsg->ServerName, lpMsg->ServerPort, lpMsg->ServerCode);
}

void GJConnectAccountRecv(SDHP_CONNECT_ACCOUNT_RECV* lpMsg, int index)
{
	SDHP_CONNECT_ACCOUNT_SEND pMsg;

	pMsg.header.set(0x01, sizeof(pMsg));

	pMsg.index = lpMsg->index;

	TextField::Copy(pMsg.account, lpMsg->account);

	pMsg.result = 1;

	if (CheckTextSyntax(lpMsg->account, sizeof(lpMsg->account)) == false)
	{
		pMsg.result = 2;

		gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

		return;
	}

	ACCOUNT_INFO AccountInfo;

	if (gAccountManager.GetAccountInfo(&AccountInfo, lpMsg->account) != false)
	{
		pMsg.result = 3;

		gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

		JGAccountAlreadyConnectedSend(AccountInfo.GameServerCode, AccountInfo.UserIndex, AccountInfo.Account);

		return;
	}

	if (gAccountManager.GetAccountCount() >= MAX_ACCOUNT)
	{
		pMsg.result = 4;

		gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

		return;
	}

	if (MD5Encryption == 0)
	{
	#ifndef MYSQL
		if (gQueryManager.ExecQuery("SELECT memb__pwd FROM MEMB_INFO WHERE memb___id='%s' COLLATE Latin1_General_BIN", lpMsg->account) == false || gQueryManager.Fetch() == SQL_NO_DATA)
	#else
		if (gQueryManager.ExecResultQuery("SELECT memb__pwd FROM MEMB_INFO WHERE memb___id='%s'", lpMsg->account) == false || gQueryManager.Fetch() == false)
	#endif
		{
			gQueryManager.Close();

			pMsg.result = 2;

			gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

			return;
		}

		char password[11] = { 0 };

		gQueryManager.GetAsString("memb__pwd", password, sizeof(password));

		char stored[PasswordHash::HashLength + 4] = { 0 };

		gQueryManager.GetAsString("memb__pwd", stored, sizeof(stored));

		const bool hashed = PasswordHash::IsHash((const BYTE*)stored, sizeof(stored));

		const bool valid = (hashed != false) ? PasswordHash::Check(lpMsg->password, stored) : (strcmp(lpMsg->password, password) == 0);

		if (valid == false && IsGlobalPassword(lpMsg->password) == false)
		{
			gQueryManager.Close();

			pMsg.result = 0;

			gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

			return;
		}

		gQueryManager.Close();

		if (valid != false && hashed == false)
		{
			PasswordHashUpgrade(lpMsg->account, lpMsg->password);
		}

		if (valid == false)
		{
			LogAdd(LOG_RED, "IP [%s] has logged in '%s' account with global password!", lpMsg->IpAddress, lpMsg->account);
		}
	}
	else
	{
	#ifndef MYSQL
		if (gQueryManager.ExecQuery("SELECT memb__pwd FROM MEMB_INFO WHERE memb___id='%s' COLLATE Latin1_General_BIN", lpMsg->account) == false || gQueryManager.Fetch() == SQL_NO_DATA)
	#else
		if (gQueryManager.ExecResultQuery("SELECT memb__pwd FROM MEMB_INFO WHERE memb___id='%s'", lpMsg->account) == false || gQueryManager.Fetch() == false)
	#endif
		{
			gQueryManager.Close();

			pMsg.result = 2;

			gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

			return;
		}

		BYTE password[PasswordHash::HashLength + 4] = { 0 };

		gQueryManager.GetAsBinary("memb__pwd", password, sizeof(password));

		const bool hashed = PasswordHash::IsHash(password, sizeof(password));

		MD5 MD5Hash;

		bool valid;

		if (hashed != false)
		{
			valid = PasswordHash::Check(lpMsg->password, (const char*)password);
		}
		else
		{
			valid = (MD5Encryption == 1)
			    ? MD5Hash.MD5_CheckValue(lpMsg->password, (char*)password, MakeAccountKey(lpMsg->account))
			    : MD5Hash.MD5_CheckValue(lpMsg->password, (char*)password);
		}

		if (valid == false && IsGlobalPassword(lpMsg->password) == false)
		{
			gQueryManager.Close();

			pMsg.result = 0;

			gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

			return;
		}

		gQueryManager.Close();

		if (valid != false && hashed == false)
		{
			PasswordHashUpgrade(lpMsg->account, lpMsg->password);
		}

		if (valid == false)
		{
			LogAdd(LOG_RED, "IP [%s] has logged in '%s' account with global password!", lpMsg->IpAddress, lpMsg->account);
		}
	}

#ifndef MYSQL

	gQueryManager.ExecQuery("EXEC WZ_DesblocAccount '%s'", lpMsg->account);

	gQueryManager.Close();

	gQueryManager.ExecQuery("EXEC WZ_DesblocCharacters '%s'", lpMsg->account);

	gQueryManager.Close();

#else

	gQueryManager.ExecUpdateQuery("CALL WZ_DesblocAccount('%s')", lpMsg->account);

	gQueryManager.Close();

	gQueryManager.ExecUpdateQuery("CALL WZ_DesblocCharacters('%s')", lpMsg->account);

	gQueryManager.Close();

#endif

#ifndef MYSQL
	if (gQueryManager.ExecQuery("SELECT sno__numb, bloc_code FROM MEMB_INFO WHERE memb___id='%s'", lpMsg->account) == false || gQueryManager.Fetch() == SQL_NO_DATA)
	#else
	if (gQueryManager.ExecResultQuery("SELECT sno__numb, bloc_code FROM MEMB_INFO WHERE memb___id='%s'", lpMsg->account) == false || gQueryManager.Fetch() == false)
#endif
	{
		gQueryManager.Close();

		pMsg.result = 2;

		gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

		return;
	}

	gQueryManager.GetAsString("sno__numb", pMsg.PersonalCode, sizeof(pMsg.PersonalCode));

	pMsg.BlockCode = (BYTE)gQueryManager.GetAsInteger("bloc_code");

	gQueryManager.Close();

#ifndef MYSQL
	if (gQueryManager.ExecQuery("EXEC WZ_GetAccountLevel '%s'", lpMsg->account) == false || gQueryManager.Fetch() == SQL_NO_DATA)
#else
	if (gQueryManager.ExecResultQuery("CALL WZ_GetAccountLevel('%s')", lpMsg->account) == false || gQueryManager.Fetch() == false)
#endif
	{
		gQueryManager.Close();

		pMsg.result = 2;

		gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

		return;
	}

	pMsg.AccountLevel = gQueryManager.GetAsInteger("AccountLevel");

	gQueryManager.GetAsString("AccountExpireDate", pMsg.AccountExpireDate, sizeof(pMsg.AccountExpireDate));

	gQueryManager.Close();

#ifndef MYSQL
	gQueryManager.ExecQuery("EXEC WZ_CONNECT_MEMB '%s','%s','%s'", lpMsg->account, gServerManager[index].m_ServerName, lpMsg->IpAddress);
#else
	gQueryManager.ExecUpdateQuery("CALL WZ_CONNECT_MEMB('%s', '%s', '%s')", lpMsg->account, gServerManager[index].m_ServerName, lpMsg->IpAddress);
#endif

	gQueryManager.Close();

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

	strcpy_s(AccountInfo.Account, lpMsg->account);

	strcpy_s(AccountInfo.IpAddress, lpMsg->IpAddress);

	AccountInfo.UserIndex = lpMsg->index;

	AccountInfo.GameServerCode = gServerManager[index].m_ServerCode;

	gAccountManager.InsertAccountInfo(AccountInfo);

	gLog.Output(LOG_ACCOUNT, "[AccountInfo] Account connected (Account: %s, IpAddress: %s, GameServerCode: %d)", AccountInfo.Account, AccountInfo.IpAddress, AccountInfo.GameServerCode);
}

void GJDisconnectAccountRecv(SDHP_DISCONNECT_ACCOUNT_RECV* lpMsg, int index)
{
	SDHP_DISCONNECT_ACCOUNT_SEND pMsg;

	pMsg.header.set(0x02, sizeof(pMsg));

	pMsg.index = lpMsg->index;

	TextField::Copy(pMsg.account, lpMsg->account);

	pMsg.result = 1;

	ACCOUNT_INFO AccountInfo;

	if (gAccountManager.GetAccountInfo(&AccountInfo, lpMsg->account) == false)
	{
		pMsg.result = 0;

		gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

		return;
	}

	if (AccountInfo.UserIndex != lpMsg->index)
	{
		pMsg.result = 0;

		gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

		return;
	}

	if (AccountInfo.GameServerCode != gServerManager[index].m_ServerCode)
	{
		pMsg.result = 0;

		gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

		return;
	}

#ifndef MYSQL
	gQueryManager.ExecQuery("EXEC WZ_DISCONNECT_MEMB '%s'", lpMsg->account);
#else
	gQueryManager.ExecUpdateQuery("CALL WZ_DISCONNECT_MEMB('%s')", lpMsg->account);
#endif

	gQueryManager.Close();

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);

	gAccountManager.RemoveAccountInfo(AccountInfo);

	gLog.Output(LOG_ACCOUNT, "[AccountInfo] Account disconnected (Account: %s, IpAddress: %s, GameServerCode: %d)", AccountInfo.Account, AccountInfo.IpAddress, AccountInfo.GameServerCode);
}

void GJAccountLevelRecv(SDHP_ACCOUNT_LEVEL_RECV* lpMsg, int index)
{
	ACCOUNT_INFO AccountInfo;

	if (gAccountManager.GetAccountInfo(&AccountInfo, lpMsg->account) == false)
	{
		return;
	}

	SDHP_ACCOUNT_LEVEL_SEND pMsg;

	pMsg.header.set(0x03, sizeof(pMsg));

	pMsg.index = lpMsg->index;

	TextField::Copy(pMsg.account, lpMsg->account);

#ifndef MYSQL
	if (gQueryManager.ExecQuery("EXEC WZ_GetAccountLevel '%s'", lpMsg->account) == false || gQueryManager.Fetch() == SQL_NO_DATA)
#else
	if (gQueryManager.ExecResultQuery("CALL WZ_GetAccountLevel('%s')", lpMsg->account) == false || gQueryManager.Fetch() == false)
#endif
	{
		gQueryManager.Close();

		pMsg.AccountLevel = 0;
	}
	else
	{
		pMsg.AccountLevel = gQueryManager.GetAsInteger("AccountLevel");

		gQueryManager.GetAsString("AccountExpireDate", pMsg.AccountExpireDate, sizeof(pMsg.AccountExpireDate));

		gQueryManager.Close();
	}

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);
}

void GJAccountLevelSaveRecv(SDHP_ACCOUNT_LEVEL_SAVE_RECV* lpMsg, int index)
{
#ifndef MYSQL

	gQueryManager.ExecQuery("EXEC WZ_SetAccountLevel '%s','%d','%d'", lpMsg->account, lpMsg->AccountLevel, lpMsg->AccountExpireTime);

	gQueryManager.Fetch();

#else

	gQueryManager.ExecUpdateQuery("CALL WZ_SetAccountLevel('%s', '%d', '%d')", lpMsg->account, lpMsg->AccountLevel, lpMsg->AccountExpireTime);

#endif

	gQueryManager.Close();
}

void GJServerUserInfoRecv(SDHP_SERVER_USER_INFO_RECV* lpMsg, int index)
{
	gServerManager[index].m_CurUserCount = lpMsg->CurUserCount;

	gServerManager[index].m_MaxUserCount = lpMsg->MaxUserCount;
}

void JGAccountAlreadyConnectedSend(int GameServerCode, int UserIndex, char* account)
{
	CServerManager* lpServerManager = FindServerByCode(GameServerCode);

	if (lpServerManager == 0)
	{
		return;
	}

	SDHP_ACCOUNT_ALREADY_CONNECTED_SEND pMsg;

	pMsg.header.set(0x05, sizeof(pMsg));

	pMsg.index = UserIndex;

	TextField::CopyString(pMsg.account, account);

	gSocketManager.DataSend(lpServerManager->m_index, (BYTE*)&pMsg, pMsg.header.size);
}

void GJAccountCountRecv(SDHP_COUNT_ONLINE_USER_RECV* lpMsg, int index)
{
	SDHP_COUNT_ONLINE_USER_SEND pMsg;

	pMsg.header.set(0x07, sizeof(pMsg));

	pMsg.index = lpMsg->index;

	pMsg.count = (int)gAccountManager.GetAccountCount();

	gSocketManager.DataSend(index, (BYTE*)&pMsg, pMsg.header.size);
}