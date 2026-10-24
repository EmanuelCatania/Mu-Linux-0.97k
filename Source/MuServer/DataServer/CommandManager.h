#pragma once

#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_GLOBAL_POST_RECV
{
	PSBMSG_HEAD header; // C1:05:03
	BYTE type;
	char name[CHARACTER_NAME_BUFFER];
	char message[60];
	char serverName[60];
};

struct SDHP_COMMAND_RESET_RECV
{
	PSBMSG_HEAD header; // C1:05:04 | C1:05:05
	WORD index;
	char account[11];
	char name[CHARACTER_NAME_BUFFER];
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_GLOBAL_POST_SEND
{
	PSBMSG_HEAD header; // C1:05:03
	BYTE type;
	char name[CHARACTER_NAME_BUFFER];
	char message[60];
	char serverName[60];
};

struct SDHP_COMMAND_RESET_SEND
{
	PSBMSG_HEAD header; // C1:05:04 | C1:05:05
	WORD index;
	char account[11];
	char name[CHARACTER_NAME_BUFFER];
	UINT ResetDay;
	UINT ResetWek;
	UINT ResetMon;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CCommandManager
{
public:

	CCommandManager();

	~CCommandManager();

	void GDGlobalPostRecv(SDHP_GLOBAL_POST_RECV* lpMsg, int index);

	void GDCommandResetRecv(SDHP_COMMAND_RESET_RECV* lpMsg, int index);

	void GDCommandGrandResetRecv(SDHP_COMMAND_RESET_RECV* lpMsg, int index);

};

extern CCommandManager gCommandManager;