// GameServerProtocol.h — espejo de los paquetes GameServer -> cliente.
//
// Origen: Mu-Linux-0.97k @ 25ceccb, Source/MuServer/GameServer/*.h. Cada struct
// indica archivo:línea del server. El server compila con GAMESERVER_EXTRA=1
// (GameServer/stdafx.h:8), así que los campos View* están incluidos.
//
// Los structs del server NO usan #pragma pack (salvo los marcados): tienen el
// padding natural de x86 (WORD alineado a 2, DWORD/int a 4). Los static_assert
// fijan sizeof y el offsetof de los campos que el cliente lee hoy en
// src/Net/Net_Process.cpp (o en el archivo indicado).
//
// Incluir después de stdafx.h; usado por los handlers y por Protocol_Check.cpp.
#pragma once

#include "Net/Protocol/ProtocolBase.h"

namespace Proto {

// ── C1:15 (PROTOCOL_CODE2) ─ daño ── Protocol.h:203 ────────────────────────
// Net_Process.cpp case 0x15.
struct PMSG_DAMAGE_SEND
{
    PBMSG_HEAD header;      // C1:15
    BYTE index[2];
    BYTE damage[2];
    DWORD ViewCurHP;        // GAMESERVER_EXTRA
    DWORD ViewDamageHP;     // GAMESERVER_EXTRA
};
static_assert(sizeof(PMSG_DAMAGE_SEND) == 16, "C1:15");
static_assert(offsetof(PMSG_DAMAGE_SEND, index) == 3, "C1:15 index");
static_assert(offsetof(PMSG_DAMAGE_SEND, damage) == 5, "C1:15 damage");
static_assert(offsetof(PMSG_DAMAGE_SEND, ViewCurHP) == 8, "C1:15 ViewCurHP");        // padding en +7
static_assert(offsetof(PMSG_DAMAGE_SEND, ViewDamageHP) == 12, "C1:15 ViewDamageHP");

// ── C1:25 ─ cambio de equipo visible ── ItemManager.h:108 ─────────────────────
// Net_Process.cpp Recv_ChangePlayer.
const int MAX_ITEM_INFO = 4;    // ItemManager.h:10
struct PMSG_ITEM_CHANGE_SEND
{
    PBMSG_HEAD header;      // C1:25
    BYTE index[2];
    BYTE ItemInfo[MAX_ITEM_INFO];
};
static_assert(sizeof(PMSG_ITEM_CHANGE_SEND) == 9, "C1:25");
static_assert(offsetof(PMSG_ITEM_CHANGE_SEND, index) == 3, "C1:25 index");
static_assert(offsetof(PMSG_ITEM_CHANGE_SEND, ItemInfo) == 5, "C1:25 ItemInfo");

// ── C2:52 ─ lista de guild ── Guild.h:247 / Guild.h:256 ───────────────────────
// Net_Process.cpp case 0x52 (kHeaderSize = 16, entradas de 12 bytes).
struct PMSG_GUILD_LIST_SEND
{
    PWMSG_HEAD header;      // C2:52
    BYTE result;
    BYTE count;
    DWORD TotalScore;
    BYTE score;
};
static_assert(sizeof(PMSG_GUILD_LIST_SEND) == 16, "C2:52");
static_assert(offsetof(PMSG_GUILD_LIST_SEND, result) == 4, "C2:52 result");
static_assert(offsetof(PMSG_GUILD_LIST_SEND, count) == 5, "C2:52 count");
static_assert(offsetof(PMSG_GUILD_LIST_SEND, TotalScore) == 8, "C2:52 TotalScore");  // padding en +6
static_assert(offsetof(PMSG_GUILD_LIST_SEND, score) == 12, "C2:52 score");

struct PMSG_GUILD_LIST
{
    char name[10];
    BYTE number;
    BYTE connected;
};
static_assert(sizeof(PMSG_GUILD_LIST) == 12, "C2:52 entrada");

// ── C1:88 ─ porcentaje de Chaos Mix ── ChaosBox.h:31 ──────────────────────────
// Consulta: ChaosBox.h::PMSG_CHAOS_MIX_RATE_RECV, C1:88, padding en +3.
struct PMSG_CHAOS_MIX_RATE_RECV
{
    PBMSG_HEAD header;
    int type;
};
static_assert(sizeof(PMSG_CHAOS_MIX_RATE_RECV) == 8, "C1:88 consulta");
static_assert(offsetof(PMSG_CHAOS_MIX_RATE_RECV, type) == 4, "C1:88 type");

struct PMSG_CHAOS_MIX_RATE_SEND
{
    PBMSG_HEAD header;      // C1:88
    DWORD rate;
    DWORD money;
};
static_assert(sizeof(PMSG_CHAOS_MIX_RATE_SEND) == 12, "C1:88");
static_assert(offsetof(PMSG_CHAOS_MIX_RATE_SEND, rate) == 4, "C1:88 rate");
static_assert(offsetof(PMSG_CHAOS_MIX_RATE_SEND, money) == 8, "C1:88 money");

// ── C1:8E ─ niveles de Devil Square ── DevilSquare.h:37 ───────────────────────
// Net/Net_Events.cpp Recv_DevilSquareRequiredLevels.
const int MAX_DS_LEVEL = 4;     // DevilSquare.h:6
struct PMSG_DEVIL_SQUARE_REQ_LEVELS_SEND
{
    PBMSG_HEAD header;      // C1:8E
    int m_DevilSquareRequiredLevel[MAX_DS_LEVEL][2];
};
static_assert(sizeof(PMSG_DEVIL_SQUARE_REQ_LEVELS_SEND) == 36, "C1:8E");
static_assert(offsetof(PMSG_DEVIL_SQUARE_REQ_LEVELS_SEND, m_DevilSquareRequiredLevel) == 4, "C1:8E niveles"); // padding en +3

// ── C1:8F ─ niveles de Blood Castle ── BloodCastle.h:39 ───────────────────────
// Net/Net_Events.cpp Recv_BloodCastleRequiredLevels.
const int MAX_BC_LEVEL = 6;     // BloodCastle.h:7
struct PMSG_BLOOD_CASTLE_REQ_LEVELS_SEND
{
    PBMSG_HEAD header;      // C1:8F
    int m_BloodCastleRequiredLevel[MAX_BC_LEVEL][4];
};
static_assert(sizeof(PMSG_BLOOD_CASTLE_REQ_LEVELS_SEND) == 100, "C1:8F");
static_assert(offsetof(PMSG_BLOOD_CASTLE_REQ_LEVELS_SEND, m_BloodCastleRequiredLevel) == 4, "C1:8F niveles"); // padding en +3

// ── C1:9C ─ experiencia por matar ── Protocol.h:309 ───────────────────────────
// Net_Process.cpp case 0x9C.
struct PMSG_REWARD_EXPERIENCE_SEND
{
    PBMSG_HEAD header;      // C1:9C
    BYTE index[2];
    WORD experience[2];     // [0] = palabra alta, [1] = palabra baja
    BYTE damage[2];
    DWORD ViewDamageHP;         // GAMESERVER_EXTRA
    DWORD ViewExperience;       // GAMESERVER_EXTRA
    DWORD ViewNextExperience;   // GAMESERVER_EXTRA
};
static_assert(sizeof(PMSG_REWARD_EXPERIENCE_SEND) == 24, "C1:9C");
static_assert(offsetof(PMSG_REWARD_EXPERIENCE_SEND, index) == 3, "C1:9C index");
static_assert(offsetof(PMSG_REWARD_EXPERIENCE_SEND, experience) == 6, "C1:9C experience");   // padding en +5
static_assert(offsetof(PMSG_REWARD_EXPERIENCE_SEND, damage) == 10, "C1:9C damage");
static_assert(offsetof(PMSG_REWARD_EXPERIENCE_SEND, ViewDamageHP) == 12, "C1:9C ViewDamageHP");
static_assert(offsetof(PMSG_REWARD_EXPERIENCE_SEND, ViewExperience) == 16, "C1:9C ViewExperience");
static_assert(offsetof(PMSG_REWARD_EXPERIENCE_SEND, ViewNextExperience) == 20, "C1:9C ViewNextExperience");

// ── C1:DD ─ nivel máximo para borrar personaje ── Protocol.h:331 ──────────────
struct PMSG_CHARACTER_DELETE_LEVEL_SEND
{
    PBMSG_HEAD header;      // C1:DD
    WORD Level;
};
static_assert(sizeof(PMSG_CHARACTER_DELETE_LEVEL_SEND) == 6, "C1:DD");
static_assert(offsetof(PMSG_CHARACTER_DELETE_LEVEL_SEND, Level) == 4, "C1:DD Level");   // padding en +3

// ── C1:DE ─ creación de personaje habilitada ── Protocol.h:337 ────────────────
struct PMSG_CHARACTER_CREATION_ENABLE_SEND
{
    PBMSG_HEAD header;      // C1:DE
    BYTE result;
};
static_assert(sizeof(PMSG_CHARACTER_CREATION_ENABLE_SEND) == 4, "C1:DE");
static_assert(offsetof(PMSG_CHARACTER_CREATION_ENABLE_SEND, result) == 3, "C1:DE result");

// ── C1:DF ─ nivel máximo de personaje ── Protocol.h:343 ───────────────────────
struct PMSG_CHARACTER_MAX_LEVEL_SEND
{
    PBMSG_HEAD header;      // C1:DF
    DWORD MaxCharacterLevel;
};
static_assert(sizeof(PMSG_CHARACTER_MAX_LEVEL_SEND) == 8, "C1:DF");
static_assert(offsetof(PMSG_CHARACTER_MAX_LEVEL_SEND, MaxCharacterLevel) == 4, "C1:DF MaxCharacterLevel");

// ── C3:F3:03 ─ datos del personaje al entrar al mapa ── Protocol.h:399 ────────
// Net_Process.cpp Recv_JoinMapServer.
struct PMSG_CHARACTER_INFO_SEND
{
    PSBMSG_HEAD header;     // C3:F3:03
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
};
static_assert(sizeof(PMSG_CHARACTER_INFO_SEND) == 52, "F3:03");      // 50 + 2 de padding final
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, X) == 4, "F3:03 X");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, Y) == 5, "F3:03 Y");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, Map) == 6, "F3:03 Map");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, Dir) == 7, "F3:03 Dir");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, Experience) == 8, "F3:03 Experience");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, NextExperience) == 12, "F3:03 NextExperience");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, LevelUpPoint) == 16, "F3:03 LevelUpPoint");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, Strength) == 18, "F3:03 Strength");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, Dexterity) == 20, "F3:03 Dexterity");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, Vitality) == 22, "F3:03 Vitality");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, Energy) == 24, "F3:03 Energy");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, Life) == 26, "F3:03 Life");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, MaxLife) == 28, "F3:03 MaxLife");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, Mana) == 30, "F3:03 Mana");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, MaxMana) == 32, "F3:03 MaxMana");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, BP) == 34, "F3:03 BP");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, MaxBP) == 36, "F3:03 MaxBP");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, Money) == 40, "F3:03 Money");               // padding en +38
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, PKLevel) == 44, "F3:03 PKLevel");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, CtlCode) == 45, "F3:03 CtlCode");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, FruitAddPoint) == 46, "F3:03 FruitAddPoint");
static_assert(offsetof(PMSG_CHARACTER_INFO_SEND, MaxFruitAddPoint) == 48, "F3:03 MaxFruitAddPoint");

// ── C3:F3:04 ─ respawn ── Protocol.h:426 ──────────────────────────────────────
// Net_Process.cpp Recv_Revival.
struct PMSG_CHARACTER_REGEN_SEND
{
    PSBMSG_HEAD header;     // C3:F3:04
    BYTE X;
    BYTE Y;
    BYTE Map;
    BYTE Dir;
    WORD Life;
    WORD Mana;
    WORD BP;
    DWORD Experience;
    DWORD Money;
    DWORD ViewCurHP;        // GAMESERVER_EXTRA
    DWORD ViewCurMP;        // GAMESERVER_EXTRA
    DWORD ViewCurBP;        // GAMESERVER_EXTRA
};
static_assert(sizeof(PMSG_CHARACTER_REGEN_SEND) == 36, "F3:04");
static_assert(offsetof(PMSG_CHARACTER_REGEN_SEND, X) == 4, "F3:04 X");
static_assert(offsetof(PMSG_CHARACTER_REGEN_SEND, Y) == 5, "F3:04 Y");
static_assert(offsetof(PMSG_CHARACTER_REGEN_SEND, Map) == 6, "F3:04 Map");
static_assert(offsetof(PMSG_CHARACTER_REGEN_SEND, Dir) == 7, "F3:04 Dir");
static_assert(offsetof(PMSG_CHARACTER_REGEN_SEND, Life) == 8, "F3:04 Life");
static_assert(offsetof(PMSG_CHARACTER_REGEN_SEND, Mana) == 10, "F3:04 Mana");
static_assert(offsetof(PMSG_CHARACTER_REGEN_SEND, BP) == 12, "F3:04 BP");
static_assert(offsetof(PMSG_CHARACTER_REGEN_SEND, Experience) == 16, "F3:04 Experience");     // padding en +14
static_assert(offsetof(PMSG_CHARACTER_REGEN_SEND, Money) == 20, "F3:04 Money");
static_assert(offsetof(PMSG_CHARACTER_REGEN_SEND, ViewCurHP) == 24, "F3:04 ViewCurHP");

// ── C1:F3:05 ─ subida de nivel ── Protocol.h:445 ──────────────────────────────
// Net_Process.cpp Recv_LevelUp.
struct PMSG_LEVEL_UP_SEND
{
    PSBMSG_HEAD header;     // C1:F3:05
    WORD Level;
    WORD LevelUpPoint;
    WORD MaxLife;
    WORD MaxMana;
    WORD MaxBP;
    WORD FruitAddPoint;
    WORD MaxFruitAddPoint;
    WORD FruitSubPoint;
    WORD MaxFruitSubPoint;
    DWORD ViewPoint;            // GAMESERVER_EXTRA
    DWORD ViewMaxHP;            // GAMESERVER_EXTRA
    DWORD ViewMaxMP;            // GAMESERVER_EXTRA
    DWORD ViewMaxBP;            // GAMESERVER_EXTRA
    DWORD ViewExperience;       // GAMESERVER_EXTRA
    DWORD ViewNextExperience;   // GAMESERVER_EXTRA
};
static_assert(sizeof(PMSG_LEVEL_UP_SEND) == 48, "F3:05");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, Level) == 4, "F3:05 Level");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, LevelUpPoint) == 6, "F3:05 LevelUpPoint");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, MaxLife) == 8, "F3:05 MaxLife");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, MaxMana) == 10, "F3:05 MaxMana");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, MaxBP) == 12, "F3:05 MaxBP");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, FruitAddPoint) == 14, "F3:05 FruitAddPoint");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, MaxFruitAddPoint) == 16, "F3:05 MaxFruitAddPoint");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, ViewPoint) == 24, "F3:05 ViewPoint");             // padding en +22
static_assert(offsetof(PMSG_LEVEL_UP_SEND, ViewMaxHP) == 28, "F3:05 ViewMaxHP");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, ViewMaxMP) == 32, "F3:05 ViewMaxMP");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, ViewMaxBP) == 36, "F3:05 ViewMaxBP");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, ViewExperience) == 40, "F3:05 ViewExperience");
static_assert(offsetof(PMSG_LEVEL_UP_SEND, ViewNextExperience) == 44, "F3:05 ViewNextExperience");

// ── C1:F3:06 ─ punto de nivel asignado ── Protocol.h:467 ──────────────────────
// Net_Process.cpp case 0xF3 / 0x06.
struct PMSG_LEVEL_UP_POINT_SEND
{
    PSBMSG_HEAD header;     // C1:F3:06
    BYTE result;            // 16 + tipo; 0 = rechazado
    WORD MaxLifeAndMana;
    WORD MaxBP;
    DWORD ViewPoint;        // GAMESERVER_EXTRA
    DWORD ViewMaxHP;        // GAMESERVER_EXTRA
    DWORD ViewMaxMP;        // GAMESERVER_EXTRA
    DWORD ViewMaxBP;        // GAMESERVER_EXTRA
    DWORD ViewStrength;     // GAMESERVER_EXTRA
    DWORD ViewDexterity;    // GAMESERVER_EXTRA
    DWORD ViewVitality;     // GAMESERVER_EXTRA
    DWORD ViewEnergy;       // GAMESERVER_EXTRA
};
static_assert(sizeof(PMSG_LEVEL_UP_POINT_SEND) == 44, "F3:06");
static_assert(offsetof(PMSG_LEVEL_UP_POINT_SEND, result) == 4, "F3:06 result");
static_assert(offsetof(PMSG_LEVEL_UP_POINT_SEND, MaxLifeAndMana) == 6, "F3:06 MaxLifeAndMana");  // padding en +5
static_assert(offsetof(PMSG_LEVEL_UP_POINT_SEND, MaxBP) == 8, "F3:06 MaxBP");
static_assert(offsetof(PMSG_LEVEL_UP_POINT_SEND, ViewPoint) == 12, "F3:06 ViewPoint");          // padding en +10
static_assert(offsetof(PMSG_LEVEL_UP_POINT_SEND, ViewMaxHP) == 16, "F3:06 ViewMaxHP");
static_assert(offsetof(PMSG_LEVEL_UP_POINT_SEND, ViewMaxMP) == 20, "F3:06 ViewMaxMP");
static_assert(offsetof(PMSG_LEVEL_UP_POINT_SEND, ViewMaxBP) == 24, "F3:06 ViewMaxBP");
static_assert(offsetof(PMSG_LEVEL_UP_POINT_SEND, ViewStrength) == 28, "F3:06 ViewStrength");
static_assert(offsetof(PMSG_LEVEL_UP_POINT_SEND, ViewDexterity) == 32, "F3:06 ViewDexterity");
static_assert(offsetof(PMSG_LEVEL_UP_POINT_SEND, ViewVitality) == 36, "F3:06 ViewVitality");
static_assert(offsetof(PMSG_LEVEL_UP_POINT_SEND, ViewEnergy) == 40, "F3:06 ViewEnergy");

// ── C1:F3:E0 ─ stats extendidos (DWORD) ── Protocol.h:543 ─────────────────────
// Net_Process.cpp Recv_NewCharacterInfo.
struct PMSG_NEW_CHARACTER_INFO_SEND
{
    PSBMSG_HEAD header;     // C1:F3:E0
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
};
static_assert(sizeof(PMSG_NEW_CHARACTER_INFO_SEND) == 76, "F3:E0");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, Level) == 4, "F3:E0 Level");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, LevelUpPoint) == 8, "F3:E0 LevelUpPoint");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, Experience) == 12, "F3:E0 Experience");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, NextExperience) == 16, "F3:E0 NextExperience");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, Strength) == 20, "F3:E0 Strength");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, Dexterity) == 24, "F3:E0 Dexterity");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, Vitality) == 28, "F3:E0 Vitality");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, Energy) == 32, "F3:E0 Energy");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, Life) == 36, "F3:E0 Life");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, MaxLife) == 40, "F3:E0 MaxLife");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, Mana) == 44, "F3:E0 Mana");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, MaxMana) == 48, "F3:E0 MaxMana");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, BP) == 52, "F3:E0 BP");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, MaxBP) == 56, "F3:E0 MaxBP");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, FruitAddPoint) == 60, "F3:E0 FruitAddPoint");
static_assert(offsetof(PMSG_NEW_CHARACTER_INFO_SEND, MaxFruitAddPoint) == 64, "F3:E0 MaxFruitAddPoint");

// ── C1:F3:E1 ─ stats calculados ── Protocol.h:566 ─────────────────────────────
// Net_Process.cpp Recv_NewCharacterCalc.
struct PMSG_NEW_CHARACTER_CALC_SEND
{
    PSBMSG_HEAD header;     // C1:F3:E1
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
static_assert(sizeof(PMSG_NEW_CHARACTER_CALC_SEND) == 72, "F3:E1");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewCurHP) == 4, "F3:E1 ViewCurHP");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewMaxHP) == 8, "F3:E1 ViewMaxHP");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewCurMP) == 12, "F3:E1 ViewCurMP");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewMaxMP) == 16, "F3:E1 ViewMaxMP");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewCurBP) == 20, "F3:E1 ViewCurBP");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewMaxBP) == 24, "F3:E1 ViewMaxBP");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewPhysiSpeed) == 28, "F3:E1 ViewPhysiSpeed");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewMagicSpeed) == 32, "F3:E1 ViewMagicSpeed");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewMagicDamageMin) == 44, "F3:E1 ViewMagicDamageMin");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewMagicDamageMax) == 48, "F3:E1 ViewMagicDamageMax");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewAttackSuccessRate) == 56, "F3:E1 ViewAttackSuccessRate");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewDefense) == 64, "F3:E1 ViewDefense");
static_assert(offsetof(PMSG_NEW_CHARACTER_CALC_SEND, ViewDefenseSuccessRate) == 68, "F3:E1 ViewDefenseSuccessRate");

// ── C2:F3:E2 ─ barras de vida ── Protocol.h:588 / Protocol.h:594 ──────────────
// UI/HealthBar.cpp valida y consume la lista completa.
struct PMSG_HEALTH_BAR_SEND
{
    PSWMSG_HEAD header;     // C2:F3:E2
    BYTE count;
};
static_assert(sizeof(PMSG_HEALTH_BAR_SEND) == 6, "F3:E2");
static_assert(offsetof(PMSG_HEALTH_BAR_SEND, count) == 5, "F3:E2 count");

struct PMSG_HEALTH_BAR
{
    WORD index;
    BYTE type;
    BYTE rateHP;
};
static_assert(sizeof(PMSG_HEALTH_BAR) == 4, "F3:E2 entrada");
static_assert(offsetof(PMSG_HEALTH_BAR, index) == 0, "F3:E2 index");
static_assert(offsetof(PMSG_HEALTH_BAR, type) == 2, "F3:E2 type");
static_assert(offsetof(PMSG_HEALTH_BAR, rateHP) == 3, "F3:E2 rateHP");

// ── C2:F3:E3 ─ máximo de apilado ── ItemStack.h:9 / ItemStack.h:15 ────────────
// Item/Item_ServerValue.cpp Recv_ItemStackList (encabezado de 6, entradas de 12).
struct PMSG_ITEM_STACK_LIST_SEND
{
    PSWMSG_HEAD header;     // C2:F3:E3
    BYTE count;
};
static_assert(sizeof(PMSG_ITEM_STACK_LIST_SEND) == 6, "F3:E3");
static_assert(offsetof(PMSG_ITEM_STACK_LIST_SEND, count) == 5, "F3:E3 count");

struct ITEM_STACK
{
    int ItemIndex;
    int Level;
    int MaxStack;
};
static_assert(sizeof(ITEM_STACK) == 12, "F3:E3 entrada");
static_assert(offsetof(ITEM_STACK, ItemIndex) == 0, "F3:E3 ItemIndex");
static_assert(offsetof(ITEM_STACK, Level) == 4, "F3:E3 Level");
static_assert(offsetof(ITEM_STACK, MaxStack) == 8, "F3:E3 MaxStack");

// ── C2:F3:E4 ─ precios fijos ── ItemValue.h:10 / ItemValue.h:20 ───────────────
// Item/Item_ServerValue.cpp Recv_ItemValueList (encabezado de 6, entradas de 16).
struct PMSG_ITEM_VALUE_LIST_SEND
{
    PSWMSG_HEAD header;     // C2:F3:E4
    BYTE count;
};
static_assert(sizeof(PMSG_ITEM_VALUE_LIST_SEND) == 6, "F3:E4");
static_assert(offsetof(PMSG_ITEM_VALUE_LIST_SEND, count) == 5, "F3:E4 count");

struct ITEM_VALUE_INFO
{
    int Index;
    int Level;
    int BuyValue;
    int SellValue;
};
static_assert(sizeof(ITEM_VALUE_INFO) == 16, "F3:E4 entrada");
static_assert(offsetof(ITEM_VALUE_INFO, Index) == 0, "F3:E4 Index");
static_assert(offsetof(ITEM_VALUE_INFO, Level) == 4, "F3:E4 Level");
static_assert(offsetof(ITEM_VALUE_INFO, BuyValue) == 8, "F3:E4 BuyValue");
static_assert(offsetof(ITEM_VALUE_INFO, SellValue) == 12, "F3:E4 SellValue");

// ── C2:F3:E5 ─ lista de /move ── Move.h:34 / Move.h:41 ────────────────────────
// Consumido por UI/MoveList.cpp; la autorización final corresponde al server.
struct PMSG_MOVE_LIST_SEND
{
    PSWMSG_HEAD header;     // C2:F3:E5
    BYTE PKLimitFree;
    BYTE count;
};
static_assert(sizeof(PMSG_MOVE_LIST_SEND) == 7, "F3:E5");
static_assert(offsetof(PMSG_MOVE_LIST_SEND, PKLimitFree) == 5, "F3:E5 PKLimitFree");
static_assert(offsetof(PMSG_MOVE_LIST_SEND, count) == 6, "F3:E5 count");

struct MOVE_LIST_INFO
{
    BYTE MapNumber;
    char MapName[32];
    bool CanMove;
    short MinLevel;
    short MaxLevel;
    short MinReset;
    short MaxReset;
    short AccountLevel;
    DWORD Money;
};
static_assert(sizeof(MOVE_LIST_INFO) == 48, "F3:E5 entrada");
static_assert(offsetof(MOVE_LIST_INFO, CanMove) == 33, "F3:E5 CanMove");
static_assert(offsetof(MOVE_LIST_INFO, MinLevel) == 34, "F3:E5 MinLevel");
static_assert(offsetof(MOVE_LIST_INFO, MaxLevel) == 36, "F3:E5 MaxLevel");
static_assert(offsetof(MOVE_LIST_INFO, MinReset) == 38, "F3:E5 MinReset");
static_assert(offsetof(MOVE_LIST_INFO, MaxReset) == 40, "F3:E5 MaxReset");
static_assert(offsetof(MOVE_LIST_INFO, AccountLevel) == 42, "F3:E5 AccountLevel");
static_assert(offsetof(MOVE_LIST_INFO, Money) == 44, "F3:E5 Money");

// ── C2:F3:E6 ─ horarios de eventos ── Protocol.h:601 / Protocol.h:607 ─────────
// El cliente hoy no lo parsea.
struct PMSG_EVENT_TIME_SEND
{
    PSWMSG_HEAD header;     // C2:F3:E6
    BYTE count;
};
static_assert(sizeof(PMSG_EVENT_TIME_SEND) == 6, "F3:E6");
static_assert(offsetof(PMSG_EVENT_TIME_SEND, count) == 5, "F3:E6 count");

struct PMSG_EVENT_TIME
{
    char name[32];
    BYTE status;
    DWORD time;
};
static_assert(sizeof(PMSG_EVENT_TIME) == 40, "F3:E6 entrada");
static_assert(offsetof(PMSG_EVENT_TIME, status) == 32, "F3:E6 status");
static_assert(offsetof(PMSG_EVENT_TIME, time) == 36, "F3:E6 time");          // padding en +33

// GoldenArcher.h del server: PMSG_NPC_GOLDEN_ARCHER_SEND (C1:94).
struct PMSG_NPC_GOLDEN_ARCHER_SEND
{
    PBMSG_HEAD header;
    BYTE Type;
    short Count;
    char LuckyNumber[13];
};
static_assert(sizeof(PMSG_NPC_GOLDEN_ARCHER_SEND) == 20, "C1:94");
static_assert(offsetof(PMSG_NPC_GOLDEN_ARCHER_SEND, Type) == 3, "C1:94 Type");
static_assert(offsetof(PMSG_NPC_GOLDEN_ARCHER_SEND, Count) == 4, "C1:94 Count");
static_assert(offsetof(PMSG_NPC_GOLDEN_ARCHER_SEND, LuckyNumber) == 6, "C1:94 LuckyNumber");

// GoldenArcher.h: PMSG_GOLDEN_ARCHER_LIST_SEND / LUCKY_NUMBER_INFO (C2:97:01).
struct PMSG_GOLDEN_ARCHER_LIST_SEND
{
    PSWMSG_HEAD header;
    int count;
};
struct LUCKY_NUMBER_INFO
{
    char LuckyNumber[13];
};
static_assert(sizeof(PMSG_GOLDEN_ARCHER_LIST_SEND) == 12, "C2:97:01");
static_assert(offsetof(PMSG_GOLDEN_ARCHER_LIST_SEND, count) == 8, "C2:97:01 count");
static_assert(sizeof(LUCKY_NUMBER_INFO) == 13, "C2:97:01 entrada");
static_assert(offsetof(LUCKY_NUMBER_INFO, LuckyNumber) == 0, "C2:97:01 LuckyNumber");

// Guild.h del server: PMSG_GUILD_WAR_DECLARE_SEND (C1:61).
struct PMSG_GUILD_WAR_DECLARE_SEND
{
    PBMSG_HEAD header;
    char GuildName[8];
    BYTE type;
};
static_assert(sizeof(PMSG_GUILD_WAR_DECLARE_SEND) == 12, "C1:61");
static_assert(offsetof(PMSG_GUILD_WAR_DECLARE_SEND, GuildName) == 3, "C1:61 GuildName");
static_assert(offsetof(PMSG_GUILD_WAR_DECLARE_SEND, type) == 11, "C1:61 type");

// Protocol.h del server: PMSG_LIVE_CLIENT_RECV (C3:0E).
struct PMSG_LIVE_CLIENT_RECV
{
    PBMSG_HEAD header;
    DWORD TickCount;
    WORD PhysiSpeed;
    WORD MagicSpeed;
};
static_assert(sizeof(PMSG_LIVE_CLIENT_RECV) == 12, "C3:0E");
static_assert(offsetof(PMSG_LIVE_CLIENT_RECV, TickCount) == 4, "C3:0E TickCount");
static_assert(offsetof(PMSG_LIVE_CLIENT_RECV, PhysiSpeed) == 8, "C3:0E PhysiSpeed");
static_assert(offsetof(PMSG_LIVE_CLIENT_RECV, MagicSpeed) == 10, "C3:0E MagicSpeed");

} // namespace Proto
