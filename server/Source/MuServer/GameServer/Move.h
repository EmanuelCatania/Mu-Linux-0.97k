#pragma once

#include "ProtocolDefines.h"
#include "User.h"

#define MAX_MOVE 100

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_TELEPORT_RECV
{
	PBMSG_HEAD header; // C1:1C
	BYTE gate;
	BYTE x;
	BYTE y;
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_TELEPORT_SEND
{
	PBMSG_HEAD header; // C3:1C
	BYTE gate;
	BYTE map;
	BYTE x;
	BYTE y;
	BYTE dir;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct MOVE_INFO
{
	int Index;
	char Name[32];
	int Money;
	int MinLevel;
	int MaxLevel;
	int MinReset;
	int MaxReset;
	int AccountLevel;
	int Gate;
};

class CMove
{
public:

	CMove();

	~CMove();

	void Load(const char* path);

	bool GetInfo(int index, MOVE_INFO* lpInfo);

	bool GetInfoByName(char* name, MOVE_INFO* lpInfo);

	void Move(LPOBJ lpObj, int index);

	void CGTeleportRecv(PMSG_TELEPORT_RECV* lpMsg, int aIndex);

	void GCTeleportSend(int aIndex, int gate, BYTE map, BYTE x, BYTE y, BYTE dir);

	// La lista viaja en el catálogo (sección F3:F5).
	const std::map<int, MOVE_INFO>& GetList() const { return this->m_MoveInfo; }

	void ReloadMoveList();

private:

	std::map<int, MOVE_INFO> m_MoveInfo;
};

extern CMove gMove;
