#pragma once

#include "User.h"

// Move Protocol
#define PROTOCOL_CODE1 0x10
// Attack Protocol
#define PROTOCOL_CODE2 0x15
// Position Protocol
#define PROTOCOL_CODE3 0x11
// Multiskill Protocol
#define PROTOCOL_CODE4 0x1D

#include "ProtocolDefines.h"

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_CHAT_RECV
{
	PBMSG_HEAD header; // C1:00
	char name[CHARACTER_NAME_SIZE];
	char message[CHAT_MESSAGE_SIZE];
};

struct PMSG_CHAT_WHISPER_RECV
{
	PBMSG_HEAD header; // C1:00
	char name[CHARACTER_NAME_SIZE];
	char message[CHAT_MESSAGE_SIZE];
};

struct PMSG_MAIN_CHECK_RECV
{
	PBMSG_HEAD header; // C1:03
	DWORD key;
};

struct PMSG_LIVE_CLIENT_RECV
{
	PBMSG_HEAD header; // C3:0E
	DWORD TickCount;
	WORD PhysiSpeed;
	WORD MagicSpeed;
};

struct PMSG_POSITION_RECV
{
	PBMSG_HEAD header; // C1:[PROTOCOL_CODE3]
	BYTE x;
	BYTE y;
};

struct PMSG_ACTION_RECV
{
	PBMSG_HEAD header; // C1:18
	BYTE dir;
	BYTE action;
	BYTE index[2];
};

struct PMSG_EVENT_REMAIN_TIME_RECV
{
	PBMSG_HEAD header; // C1:91
	BYTE EventType;
	BYTE ItemLevel;
};

struct PMSG_MOVE_RECV
{
	PBMSG_HEAD header; // C1:[PROTOCOL_CODE1]
	BYTE x;
	BYTE y;
	BYTE path[8];
};

struct PMSG_CONNECT_ACCOUNT_RECV
{
#pragma pack(1)
	PSBMSG_HEAD header; // C3:F1:01
	char account[10];
	char password[10];
	DWORD TickCount;
	BYTE ClientVersion[5];
	BYTE ClientSerial[16];
	BYTE ProtocolVersion[2]; // MU_PROTOCOL_VERSION; falta en clientes anteriores a la negociación
#pragma pack()
};

struct PMSG_CLOSE_CLIENT_RECV
{
	PSBMSG_HEAD header; // C1:F1:02
	BYTE type;
};

struct PMSG_SET_LANG_RECV
{
	PSBMSG_HEAD header; // C1:F1:04
	int lang;
};

struct PMSG_SET_HWID_RECV
{
	PSBMSG_HEAD head; // C1:F1:05
	char HardwareId[36];
};

struct PMSG_CHARACTER_CREATE_RECV
{
	PSBMSG_HEAD header; // C1:F3:01
	char name[CHARACTER_NAME_SIZE];
	BYTE Class;
};

struct PMSG_CHARACTER_DELETE_RECV
{
	PSBMSG_HEAD header; // C1:F3:02
	char name[CHARACTER_NAME_SIZE];
	char PersonalCode[10];
};

struct PMSG_CHARACTER_INFO_RECV
{
	PSBMSG_HEAD header; // C1:F3:03
	char name[CHARACTER_NAME_SIZE];
};

struct PMSG_LEVEL_UP_POINT_RECV
{
	PSBMSG_HEAD header; // C1:F3:06
	BYTE type;
};

struct PMSG_OPTION_DATA_RECV
{
#pragma pack(1)
	PSBMSG_HEAD header; // C1:F3:30
	BYTE SkillKey[10];
	BYTE GameOption;
	BYTE QKey;
	BYTE WKey;
	BYTE EKey;
	BYTE ChatWindow;
#pragma pack()
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_CHAT_SEND
{
	PBMSG_HEAD header; // C1:00
	char name[CHARACTER_NAME_SIZE];
	char message[CHAT_MESSAGE_SIZE];
};

struct PMSG_CHAT_TARGET_SEND
{
	PBMSG_HEAD header; // C1:01
	BYTE index[2];
	char message[CHAT_MESSAGE_SIZE];
};

struct PMSG_CHAT_WHISPER_SEND
{
	PBMSG_HEAD header; // C1:02
	char name[CHARACTER_NAME_SIZE];
	char message[CHAT_MESSAGE_SIZE];
};

struct PMSG_MAIN_CHECK_SEND
{
	PBMSG_HEAD header; // C1:03
	WORD key;
};

struct PMSG_EVENT_STATE_SEND
{
	PBMSG_HEAD header; // C1:0B
	BYTE state;
	BYTE event;
};

struct PMSG_SERVER_MSG_SEND
{
	PBMSG_HEAD header; // C1:0C
	BYTE MsgNumber;
};

struct PMSG_LIVE_CLIENT_SEND
{
	PBMSG_HEAD header; // C1:0E
	DWORD TickCount;
};

struct PMSG_WEATHER_SEND
{
	PBMSG_HEAD header; // C1:0F
	BYTE weather;
};

struct PMSG_DAMAGE_SEND
{
	PBMSG_HEAD header; // C1:[PROTOCOL_CODE2]
	BYTE index[2];
	BYTE damage[2];
#if(GAMESERVER_EXTRA==1)
	DWORD ViewCurHP;
	DWORD ViewDamageHP;
	BYTE rateHP;        // Fase 5 (C2): % de vida del blanco, para la barra
#endif
};

struct PMSG_POSITION_SEND
{
	PBMSG_HEAD header; // C1:[PROTOCOL_CODE3]
	BYTE index[2];
	BYTE x;
	BYTE y;
};

struct PMSG_MONSTER_DIE_SEND
{
	PBMSG_HEAD header; // C1:16
	BYTE index[2];
	BYTE experience[2];
	BYTE damage[2];
#if(GAMESERVER_EXTRA==1)
	DWORD ViewDamageHP;
#endif
};

struct PMSG_USER_DIE_SEND
{
	PBMSG_HEAD header; // C1:17
	BYTE index[2];
	BYTE skill;
	BYTE killer[2];
};

struct PMSG_ACTION_SEND
{
	PBMSG_HEAD header; // C1:18
	BYTE index[2];
	BYTE dir;
	BYTE action;
};

struct PMSG_LIFE_SEND
{
	PBMSG_HEAD header; // C1:26
	BYTE type;
	BYTE life[2];
	BYTE flag;
#if(GAMESERVER_EXTRA==1)
	DWORD ViewHP;
#endif
};

struct PMSG_MANA_SEND
{
	PBMSG_HEAD header; // C1:27
	BYTE type;
	BYTE mana[2];
	BYTE bp[2];
#if(GAMESERVER_EXTRA==1)
	DWORD ViewMP;
	DWORD ViewBP;
#endif
};

struct PMSG_ITEM_SPECIAL_TIME_SEND
{
	PBMSG_HEAD header; // C3:29
	BYTE number;
	WORD time;
};

struct PMSG_MAP_ATTR_SEND
{
	PBMSG_HEAD header; // C1:46
	BYTE type;
	BYTE attr;
	BYTE flag;
	BYTE count;
};

struct PMSG_MAP_ATTR
{
	BYTE x;
	BYTE y;
	BYTE tx;
	BYTE ty;
};

struct PMSG_EVENT_REMAIN_TIME_SEND
{
	PBMSG_HEAD header; // C1:91
	BYTE EventType;
	BYTE RemainTime;
};

struct PMSG_TIME_COUNT_SEND
{
	PBMSG_HEAD header; // C1:92
	BYTE type;
};

struct PMSG_REWARD_EXPERIENCE_SEND
{
	PBMSG_HEAD header; // C1:9C
	BYTE index[2];
	WORD experience[2];
	BYTE damage[2];
#if(GAMESERVER_EXTRA==1)
	DWORD ViewDamageHP;
	DWORD ViewExperience;
	DWORD ViewNextExperience;
#endif
};

struct PMSG_MOVE_SEND
{
	PBMSG_HEAD header; // C1:[PROTOCOL_CODE1]
	BYTE index[2];
	BYTE x;
	BYTE y;
	BYTE dir;
};

struct PMSG_CHARACTER_DELETE_LEVEL_SEND
{
	PBMSG_HEAD header; // C1:DD
	WORD Level;
};

struct PMSG_CHARACTER_CREATION_ENABLE_SEND
{
	PBMSG_HEAD header; // C1:DE
	BYTE result;
};

struct PMSG_CHARACTER_MAX_LEVEL_SEND
{
	PBMSG_HEAD header; // C1:DF
	DWORD MaxCharacterLevel;
};

// C1:F3:FA — velocidad de ataque y de magia de un jugador, a los que lo tienen
// en el viewport, cuando cambia (Fase 5, A5).  La inicial viaja en el 0x12.
struct PMSG_PLAYER_SPEED_SEND
{
	PSBMSG_HEAD header; // C1:F3:FA
	BYTE index[2];
	WORD PhysiSpeed;
	WORD MagicSpeed;
};

// C1:F3:FB — el server descartó un pedido (Fase 5, A6).  Ver Common/RejectReason.h.
struct PMSG_REJECT_SEND
{
	PSBMSG_HEAD header; // C1:F3:FB
	BYTE head;          // opcode del pedido rechazado
	BYTE reason;        // REJECT_REASON
	BYTE flags;         // 0x01 = cuenta GM: el cliente Release muestra el motivo
};

struct PMSG_CONNECT_CLIENT_SEND
{
	PSBMSG_HEAD header; // C1:F1:00
	BYTE result;
	BYTE index[2];
	BYTE ClientVersion[5];
	BYTE ProtocolVersion[2]; // MU_PROTOCOL_VERSION (los clientes viejos no lo leen)
};

struct PMSG_CONNECT_ACCOUNT_SEND
{
	PSBMSG_HEAD header; // C1:F1:01
	BYTE result;
};

struct PMSG_CLOSE_CLIENT_SEND
{
	PSBMSG_HEAD header; // C1:F1:02
	BYTE result;
};

struct PMSG_CHARACTER_LIST_SEND
{
	PSWMSG_HEAD header; // C2:F3:00
	BYTE count;
};

struct PMSG_CHARACTER_LIST
{
	BYTE slot;
	char Name[CHARACTER_NAME_SIZE];
	WORD Level;
	BYTE CtlCode;
	BYTE CharSet[11];
	WORD ExtItem[9];    // CharSet expandido (protocolo 7): items >= 512, 0xFFFF = no.  Antes F3:ED.
};

struct PMSG_CHARACTER_CREATE_SEND
{
	PSBMSG_HEAD header; // C1:F3:01
	BYTE result;
	char name[CHARACTER_NAME_SIZE];
	BYTE slot;
	BYTE equipment[24];
};

struct PMSG_CHARACTER_DELETE_SEND
{
	PSBMSG_HEAD header; // C1:F3:02
	BYTE result;
};

// Bloque de stats del personaje (Fase 5, C2, protocolo 8).  Une lo que antes
// mandaban aparte F3:E0 (NewCharacterInfo) y F3:E1 (NewCharacterCalc), en 32
// bits, y viaja al final de F3:03 (al entrar), F3:05 (subida de nivel) y F3:06
// (puntos; con result = LEVEL_UP_POINT_RECALC es sólo el recálculo, por equipo,
// buffs o comandos).
#define LEVEL_UP_POINT_RECALC 0xFE

struct VIEW_CHARACTER_STATS
{
	// Antes F3:E0
	DWORD Level;
	DWORD LevelUpPoint;
	DWORD Experience;
	DWORD NextExperience;
	DWORD Strength;
	DWORD Dexterity;
	DWORD Vitality;
	DWORD Energy;
	DWORD Life;
	DWORD MaxLife;
	DWORD Mana;
	DWORD MaxMana;
	DWORD BP;
	DWORD MaxBP;
	DWORD FruitAddPoint;
	DWORD MaxFruitAddPoint;
	DWORD ViewReset;
	DWORD ViewGrandReset;
	// Antes F3:E1
	DWORD ViewCurHP;
	DWORD ViewMaxHP;
	DWORD ViewCurMP;
	DWORD ViewMaxMP;
	DWORD ViewCurBP;
	DWORD ViewMaxBP;
	DWORD ViewPhysiSpeed;
	DWORD ViewMagicSpeed;
	DWORD ViewPhysiDamageMin;
	DWORD ViewPhysiDamageMax;
	DWORD ViewMagicDamageMin;
	DWORD ViewMagicDamageMax;
	DWORD ViewMagicDamageRate;
	DWORD ViewAttackSuccessRate;
	DWORD ViewDamageMultiplier;
	DWORD ViewDefense;
	DWORD ViewDefenseSuccessRate;
};

static_assert(sizeof(VIEW_CHARACTER_STATS) == 140, "VIEW_CHARACTER_STATS");

struct PMSG_CHARACTER_INFO_SEND
{
	PSBMSG_HEAD header; // C3:F3:03
	BYTE X;
	BYTE Y;
	BYTE Map;
	BYTE Dir;
	DWORD Experience;
	DWORD NextExperience;
	WORD LevelUpPoint;
	WORD Strength;
	WORD Dexterity;
	WORD Vitality;
	WORD Energy;
	WORD Life;
	WORD MaxLife;
	WORD Mana;
	WORD MaxMana;
	WORD BP;
	WORD MaxBP;
	DWORD Money;
	BYTE PKLevel;
	BYTE CtlCode;
	WORD FruitAddPoint;
	WORD MaxFruitAddPoint;
#if(GAMESERVER_EXTRA==1)
	VIEW_CHARACTER_STATS Stats;
#endif
};

struct PMSG_CHARACTER_REGEN_SEND
{
	PSBMSG_HEAD header; // C3:F3:04
	BYTE X;
	BYTE Y;
	BYTE Map;
	BYTE Dir;
	WORD Life;
	WORD Mana;
	WORD BP;
	DWORD Experience;
	DWORD Money;
#if(GAMESERVER_EXTRA==1)
	DWORD ViewCurHP;
	DWORD ViewCurMP;
	DWORD ViewCurBP;
#endif
};

struct PMSG_LEVEL_UP_SEND
{
	PSBMSG_HEAD header; // C1:F3:05
	WORD Level;
	WORD LevelUpPoint;
	WORD MaxLife;
	WORD MaxMana;
	WORD MaxBP;
	WORD FruitAddPoint;
	WORD MaxFruitAddPoint;
	WORD FruitSubPoint;
	WORD MaxFruitSubPoint;
#if(GAMESERVER_EXTRA==1)
	DWORD ViewPoint;
	DWORD ViewMaxHP;
	DWORD ViewMaxMP;
	DWORD ViewMaxBP;
	DWORD ViewExperience;
	DWORD ViewNextExperience;
	VIEW_CHARACTER_STATS Stats;
#endif
};

struct PMSG_LEVEL_UP_POINT_SEND
{
	PSBMSG_HEAD header; // C1:F3:06
	BYTE result;
	WORD MaxLifeAndMana;
	WORD MaxBP;
#if(GAMESERVER_EXTRA==1)
	DWORD ViewPoint;
	DWORD ViewMaxHP;
	DWORD ViewMaxMP;
	DWORD ViewMaxBP;
	DWORD ViewStrength;
	DWORD ViewDexterity;
	DWORD ViewVitality;
	DWORD ViewEnergy;
	VIEW_CHARACTER_STATS Stats;
#endif
};

struct PMSG_MONSTER_DAMAGE_SEND
{
	PSBMSG_HEAD header; // C1:F3:07
	BYTE damage[2];
#if(GAMESERVER_EXTRA==1)
	DWORD ViewCurHP;
	DWORD ViewDamageHP;
#endif
};

struct PMSG_PK_LEVEL_SEND
{
	PSBMSG_HEAD header; // C1:F3:08
	BYTE index[2];
	BYTE PKLevel;
};

struct PMSG_SUMMON_LIFE_SEND
{
	PSBMSG_HEAD header; // C1:F3:20
	BYTE life;
};

struct PMSG_TIME_VIEW_SEND
{
	PSBMSG_HEAD header; // C1:F3:22
	WORD time;
};

struct PMSG_OPTION_DATA_SEND
{
#pragma pack(1)
	PSBMSG_HEAD header; // C1:F3:30
	BYTE SkillKey[10];
	BYTE GameOption;
	BYTE QKey;
	BYTE WKey;
	BYTE EKey;
	BYTE ChatWindow;
#pragma pack()
};

struct PMSG_FIREWORKS_SEND
{
	PSBMSG_HEAD header; // C1:F3:40
	BYTE type;
	BYTE x;
	BYTE y;
};

struct PMSG_SERVER_COMMAND_SEND
{
	PSBMSG_HEAD header; // C1:F3:40
	BYTE type;
	BYTE cmd1;
	BYTE cmd2;
};

struct PMSG_HEALTH_BAR_SEND
{
	PSWMSG_HEAD header; // C2:F3:E2
	BYTE count;
};

struct PMSG_HEALTH_BAR
{
	WORD index;
	BYTE type;
	BYTE rateHP;
};

// Horarios de eventos (Fase 5, C2, protocolo 9): antes F3:E6 + F3:F2, ahora un
// solo C2:91.  El C1:91 sigue siendo la respuesta al click derecho sobre una
// invitación (PMSG_EVENT_REMAIN_TIME_SEND); el cliente los distingue por el frame.
//   BYTE count + count PMSG_EVENT_TIME
//   BYTE invasiones + por cada una PMSG_INVASION_DETAIL y sus PMSG_INVASION_DETAIL_MAP
struct PMSG_EVENT_TIME_SEND
{
	PWMSG_HEAD header; // C2:91
	BYTE count;
};

struct PMSG_EVENT_TIME
{
	char name[32];
	BYTE status;
	DWORD time;
};

// Detalle de las invasiones en curso, para el tooltip del panel de eventos:
// por cada una, cuántos monstruos quedan vivos en cada mapa. Va después del
// F3:E6 y se empareja con su fila por el nombre. Un cliente que no lo conoce
// lo ignora.
struct PMSG_INVASION_DETAIL
{
	char name[32];
	BYTE maps; // cantidad de PMSG_INVASION_DETAIL_MAP que siguen
};

struct PMSG_INVASION_DETAIL_MAP
{
	BYTE map;
	WORD alive;
	WORD total;
};

//**********************************************//
//**********************************************//
//**********************************************//

void ProtocolCore(BYTE head, BYTE* lpMsg, int size, int aIndex, int encrypt, int serial);

void CGChatRecv(PMSG_CHAT_RECV* lpMsg, int aIndex);

void CGChatWhisperRecv(PMSG_CHAT_WHISPER_RECV* lpMsg, int aIndex);

void CGMainCheckRecv(PMSG_MAIN_CHECK_RECV* lpMsg, int aIndex);

void CGLiveClientRecv(PMSG_LIVE_CLIENT_RECV* lpMsg, int aIndex);

void CGPositionRecv(PMSG_POSITION_RECV* lpMsg, int aIndex);

void CGActionRecv(PMSG_ACTION_RECV* lpMsg, int aIndex);

void CGEventRemainTimeRecv(PMSG_EVENT_REMAIN_TIME_RECV* lpMsg, int aIndex);

void CGMoveRecv(PMSG_MOVE_RECV* lpMsg, int aIndex);

void CGConnectAccountRecv(PMSG_CONNECT_ACCOUNT_RECV* lpMsg, int aIndex, int size);

void CGCloseClientRecv(PMSG_CLOSE_CLIENT_RECV* lpMsg, int aIndex);

void CGSetLangRecv(PMSG_SET_LANG_RECV* lpMsg, int aIndex);

void CGSetHwidRecv(PMSG_SET_HWID_RECV* lpMsg, int aIndex);

void CGCharacterListRecv(int aIndex);

void CGCharacterCreateRecv(PMSG_CHARACTER_CREATE_RECV* lpMsg, int aIndex);

void CGCharacterDeleteRecv(PMSG_CHARACTER_DELETE_RECV* lpMsg, int aIndex);

void CGCharacterInfoRecv(PMSG_CHARACTER_INFO_RECV* lpMsg, int aIndex);

void CGLevelUpPointRecv(PMSG_LEVEL_UP_POINT_RECV* lpMsg, int aIndex);

void CGCharacterMoveViewportEnableRecv(int aIndex);

void CGOptionDataRecv(PMSG_OPTION_DATA_RECV* lpMsg, int aIndex);

void GCChatTargetSend(LPOBJ lpObj, int aIndex, char* message);

void GCChatWhisperSend(int aIndex, char* name, char* message);

void GCMainCheckSend(int aIndex);

void GCEventStateSend(int aIndex, BYTE state, BYTE event);

void GCEventStateSendToAll(BYTE state, BYTE event);

void GCEventStateSendToAll(int map, BYTE state, BYTE event);

void GCServerMsgSend(int aIndex, BYTE msg);

void GCLiveClientSend(int aIndex, DWORD TickCount);

void GCWeatherSend(int aIndex, BYTE weather);

void GCDamageSend(int aIndex, int bIndex, BYTE flag, int damage, int type);

void GCUserDieSend(LPOBJ lpObj, int aIndex, int skill, int bIndex);

void GCActionSend(LPOBJ lpObj, BYTE action, int aIndex, int bIndex);

void GCMoneySend(int aIndex, DWORD money);

void GCLifeSend(int aIndex, BYTE type, int life);

void GCManaSend(int aIndex, BYTE type, int mana, int bp);

void GCItemUseSpecialTimeSend(int aIndex, BYTE number, int time);

void GCMapAttrSend(int aIndex, BYTE type, BYTE attr, BYTE flag, BYTE count, PMSG_MAP_ATTR* lpInfo);

void GCGuildMasterQuestionSend(int aIndex);

void GCMonsterDieSend(int aIndex, int bIndex, int experience, int damage, BYTE flag);

void GCRewardExperienceSend(int aIndex, int experience);

void GCCharacterDeleteLevelSend(int aIndex, WORD Level);

void GCCharacterCreationEnableSend(int aIndex, BYTE result);

void GCCharacterMaxLevelSend(int aIndex, DWORD MaxLevel);

void GCConnectClientSend(int aIndex, BYTE result);

void GCConnectAccountSend(int aIndex, BYTE result);

void GCConnectAccountSend(int aIndex, BYTE result, SOCKET socket);

void GCCloseClientSend(int aIndex, BYTE result);

void GCCharacterRegenSend(LPOBJ lpObj);

void GCLevelUpSend(LPOBJ lpObj);

void GCMonsterDamageSend(int aIndex, int damage);

void GCPKLevelSend(int aIndex, int PKLevel);

void GCSummonLifeSend(int aIndex, int life, int MaxLife);

void GCTimeViewSend(int aIndex, int time);

void GCFireworksSend(LPOBJ lpObj, int x, int y);

void GCServerCommandSend(int aIndex, BYTE type, BYTE cmd1, BYTE cmd2);

void GCNewCharacterInfoSend(LPOBJ lpObj);

void GCNewCharacterCalcSend(LPOBJ lpObj);
void gObjFillViewStats(LPOBJ lpObj, VIEW_CHARACTER_STATS* lpStats);

BYTE gObjHealthRate(LPOBJ lpObj);
void GCHealthBarUpdate(int aIndex);
void GCPlayerSpeedSend(LPOBJ lpObj);
void GCRejectSend(int aIndex, BYTE head, BYTE reason);