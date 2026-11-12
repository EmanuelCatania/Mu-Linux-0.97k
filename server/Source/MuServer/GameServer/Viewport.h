#pragma once

#include "ItemManager.h"
#include "ProtocolDefines.h"

enum eViewportState
{
	VIEWPORT_NONE = 0,
	VIEWPORT_SEND = 1,
	VIEWPORT_WAIT = 2,
	VIEWPORT_DESTROY = 3,
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_VIEWPORT_DESTROY_SEND
{
	PBMSG_HEAD header; // C1:14
	BYTE count;
};

struct PMSG_VIEWPORT_DESTROY_ITEM_SEND
{
	PWMSG_HEAD header; // C2:21
	BYTE count;
};

struct PMSG_VIEWPORT_DESTROY
{
	BYTE index[2];
};

struct PMSG_VIEWPORT_SEND
{
	PWMSG_HEAD header; // C2:[12:13:1F:20:45:65:68]
	BYTE count;
};

struct PMSG_VIEWPORT_PLAYER
{
	BYTE index[2];
	BYTE x;
	BYTE y;
	BYTE CharSet[11];
	WORD ViewSkillState;
	char name[CHARACTER_NAME_SIZE];
	BYTE tx;
	BYTE ty;
	BYTE DirAndPkLevel;
	// Fase 5 (A5, protocolo 3): velocidad de ataque y de magia del jugador, para
	// que los demás clientes animen sus ataques a la velocidad real.
	WORD PhysiSpeed;
	WORD MagicSpeed;
	// CharSet expandido (Fase 5, C2, protocolo 7): índice de los items >= 512
	// equipados en los slots 0..8; 0xFFFF = vanilla o vacío.  Antes F3:EB.
	WORD ExtItem[9];
};

// Llena ExtItem[9] con el equipo agregado (>= 512) de los slots 0..8.
void gObjFillExtItems(LPOBJ lpObj, WORD* item);

struct PMSG_VIEWPORT_MONSTER
{
	BYTE index[2];
	BYTE type;
	WORD ViewSkillState;
	BYTE x;
	BYTE y;
	BYTE tx;
	BYTE ty;
	BYTE DirAndPkLevel;
	// Fase 5 (C2, protocolo 6): % de vida, para la barra del cliente.  Ocupa el
	// byte de relleno que ya tenía la entrada: sigue midiendo 12.
	BYTE rateHP;
};

struct PMSG_VIEWPORT_SUMMON
{
	BYTE index[2];
	BYTE type;
	WORD ViewSkillState;
	BYTE x;
	BYTE y;
	BYTE tx;
	BYTE ty;
	BYTE DirAndPkLevel;
	char name[CHARACTER_NAME_SIZE];
};

struct PMSG_VIEWPORT_ITEM
{
	BYTE index[2];
	BYTE x;
	BYTE y;
	BYTE ItemInfo[MAX_ITEM_INFO + 1];
};

struct PMSG_VIEWPORT_CHANGE
{
	BYTE index[2];
	BYTE x;
	BYTE y;
	BYTE skin;
	WORD ViewSkillState;
	char name[CHARACTER_NAME_SIZE];
	BYTE tx;
	BYTE ty;
	BYTE DirAndPkLevel;
	BYTE CharSet[11];
};

struct PMSG_VIEWPORT_GUILD_INFO
{
	BYTE index[2];
	char GuildName[GUILD_NAME_SIZE];
	BYTE Mark[32];
};

struct PMSG_VIEWPORT_GUILD_MEMBER
{
	BYTE userIndex[2];
	BYTE guildIndex[2];
};

//**********************************************//
//**********************************************//
//**********************************************//

class CViewport
{
public:

	CViewport();

	~CViewport();

	bool CheckViewportObjectPosition(int aIndex, int map, int x, int y, int view);

	bool CheckViewportObject1(int aIndex, int bIndex, int type);

	bool CheckViewportObject2(int aIndex, int bIndex, int type);

	bool CheckViewportObjectItem(int aIndex, int bIndex, int type);

	bool AddViewportObject1(int aIndex, int bIndex, int type);

	bool AddViewportObject2(int aIndex, int bIndex, int type);

	bool AddViewportObjectItem(int aIndex, int bIndex, int type);

	void DestroyViewportPlayer1(int aIndex);

	void DestroyViewportPlayer2(int aIndex);

	void DestroyViewportMonster1(int aIndex);

	void DestroyViewportMonster2(int aIndex);

	void DestroyViewportItem(int aIndex);

	void CreateViewportPlayer(int aIndex);

	void CreateViewportMonster(int aIndex);

	void CreateViewportItem(int aIndex);

	void GCViewportDestroySend(int aIndex);

	void GCViewportDestroyItemSend(int aIndex);

	void GCViewportPlayerSend(int aIndex);

	void GCViewportMonsterSend(int aIndex);

	void GCViewportSummonSend(int aIndex);

	void GCViewportItemSend(int aIndex);

	void GCViewportChangeSend(int aIndex);

	void GCViewportGuildInfoSend(int aIndex);

	void GCViewportGuildMemberSend(int aIndex);

	void GCViewportSimpleDestroySend(LPOBJ lpObj);

	void GCViewportSimplePlayerSend(LPOBJ lpObj);

	void GCViewportSimpleMonsterSend(LPOBJ lpObj);

	void GCViewportSimpleSummonSend(LPOBJ lpObj);

	void GCViewportSimpleChangeSend(LPOBJ lpObj);

	void GCViewportSimpleGuildInfoSend(LPOBJ lpObj);

	void GCViewportSimpleGuildMemberSend(LPOBJ lpObj);
};

extern CViewport gViewport;