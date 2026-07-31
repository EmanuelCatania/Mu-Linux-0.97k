#pragma once

#include "DataServerProtocol.h"

//**********************************************//
//********** GameServer -> DataServer **********//
//**********************************************//

struct SDHP_GLOBAL_POST_RECV
{
	PSBMSG_HEAD header; // C1:05:03
	BYTE type;
	char name[11];
	char message[60];
	char serverName[60];
};

struct SDHP_GLOBAL_POST_LINK_RECV
{
	PSBMSG_HEAD header; // C1:05:06
	BYTE type;
	char name[11];
	char message[60];
	char serverName[60];
	BYTE linkStart;
	BYTE linkLength;
	BYTE ItemInfo[4];
};

struct SDHP_COMMAND_RESET_RECV
{
	PSBMSG_HEAD header; // C1:05:04 | C1:05:05
	WORD index;
	char account[11];
	char name[11];
};

//**********************************************//
//********** DataServer -> GameServer **********//
//**********************************************//

struct SDHP_GLOBAL_POST_SEND
{
	PSBMSG_HEAD header; // C1:05:03
	BYTE type;
	char name[11];
	char message[60];
	char serverName[60];
};

struct SDHP_GLOBAL_POST_LINK_SEND
{
	PSBMSG_HEAD header; // C1:05:06
	BYTE type;
	char name[11];
	char message[60];
	char serverName[60];
	BYTE linkStart;
	BYTE linkLength;
	BYTE ItemInfo[4];
};

static_assert(sizeof(SDHP_GLOBAL_POST_LINK_RECV) == 142, "Invalid global post item link receive packet size");

static_assert(sizeof(SDHP_GLOBAL_POST_LINK_SEND) == 142, "Invalid global post item link send packet size");

struct SDHP_COMMAND_RESET_SEND
{
	PSBMSG_HEAD header; // C1:05:04 | C1:05:05
	WORD index;
	char account[11];
	char name[11];
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

	void GDGlobalPostLinkRecv(SDHP_GLOBAL_POST_LINK_RECV* lpMsg, int index);

	void GDCommandResetRecv(SDHP_COMMAND_RESET_RECV* lpMsg, int index);

	void GDCommandGrandResetRecv(SDHP_COMMAND_RESET_RECV* lpMsg, int index);

};

extern CCommandManager gCommandManager;
