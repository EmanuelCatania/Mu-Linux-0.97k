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
// Usado por los handlers y por Protocol_Check.cpp, sin tipos de Windows.
#pragma once

#include "Net/Protocol/ProtocolBase.h"

namespace Proto {

// ── C1:15 (PROTOCOL_CODE2) ─ daño ── Protocol.h:203 ────────────────────────
// Net_Process.cpp case 0x15.
struct PMSG_DAMAGE_SEND
{
    PBMSG_HEAD header;      // C1:15
    std::uint8_t index[2];
    std::uint8_t damage[2];
    std::uint32_t ViewCurHP;        // GAMESERVER_EXTRA
    std::uint32_t ViewDamageHP;     // GAMESERVER_EXTRA
};
static_assert(sizeof(PMSG_DAMAGE_SEND) == 16, "C1:15");
static_assert(offsetof(PMSG_DAMAGE_SEND, index) == 3, "C1:15 index");
static_assert(offsetof(PMSG_DAMAGE_SEND, damage) == 5, "C1:15 damage");
static_assert(offsetof(PMSG_DAMAGE_SEND, ViewCurHP) == 8, "C1:15 ViewCurHP");        // padding en +7
static_assert(offsetof(PMSG_DAMAGE_SEND, ViewDamageHP) == 12, "C1:15 ViewDamageHP");

// ── C1:25 ─ cambio de equipo visible ── ItemManager.h:108 ─────────────────────
// Net_Process.cpp Recv_ChangePlayer.
const std::int32_t MAX_ITEM_INFO = 7;    // ItemManager.h:21 (0.97.20: layout 5.2)
struct PMSG_ITEM_CHANGE_SEND
{
    PBMSG_HEAD header;      // C1:25
    std::uint8_t index[2];
    std::uint8_t ItemInfo[MAX_ITEM_INFO];
};
static_assert(sizeof(PMSG_ITEM_CHANGE_SEND) == 12, "C1:25");
static_assert(offsetof(PMSG_ITEM_CHANGE_SEND, index) == 3, "C1:25 index");
static_assert(offsetof(PMSG_ITEM_CHANGE_SEND, ItemInfo) == 5, "C1:25 ItemInfo");

// ── C2:52 ─ lista de guild ── Guild.h:247 / Guild.h:256 ───────────────────────
// Net_Process.cpp case 0x52 (kHeaderSize = 16, entradas de 42 bytes).
struct PMSG_GUILD_LIST_SEND
{
    PWMSG_HEAD header;      // C2:52
    std::uint8_t result;
    std::uint8_t count;
    std::uint32_t TotalScore;
    std::uint8_t score;
};
static_assert(sizeof(PMSG_GUILD_LIST_SEND) == 16, "C2:52");
static_assert(offsetof(PMSG_GUILD_LIST_SEND, result) == 4, "C2:52 result");
static_assert(offsetof(PMSG_GUILD_LIST_SEND, count) == 5, "C2:52 count");
static_assert(offsetof(PMSG_GUILD_LIST_SEND, TotalScore) == 8, "C2:52 TotalScore");  // padding en +6
static_assert(offsetof(PMSG_GUILD_LIST_SEND, score) == 12, "C2:52 score");

struct PMSG_GUILD_LIST
{
    char name[kCharacterNameSize];
    std::uint8_t number;
    std::uint8_t connected;
};
static_assert(sizeof(PMSG_GUILD_LIST) == 42, "C2:52 entrada");

// ── C1:88 ─ porcentaje de Chaos Mix ── ChaosBox.h:31 ──────────────────────────
// Consulta: ChaosBox.h::PMSG_CHAOS_MIX_RATE_RECV, C1:88, padding en +3.
struct PMSG_CHAOS_MIX_RATE_RECV
{
    PBMSG_HEAD header;
    std::int32_t type;
};
static_assert(sizeof(PMSG_CHAOS_MIX_RATE_RECV) == 8, "C1:88 consulta");
static_assert(offsetof(PMSG_CHAOS_MIX_RATE_RECV, type) == 4, "C1:88 type");

struct PMSG_CHAOS_MIX_RATE_SEND
{
    PBMSG_HEAD header;      // C1:88
    std::uint32_t rate;
    std::uint32_t money;
};
static_assert(sizeof(PMSG_CHAOS_MIX_RATE_SEND) == 12, "C1:88");
static_assert(offsetof(PMSG_CHAOS_MIX_RATE_SEND, rate) == 4, "C1:88 rate");
static_assert(offsetof(PMSG_CHAOS_MIX_RATE_SEND, money) == 8, "C1:88 money");

// ── C1:8E ─ niveles de Devil Square ── DevilSquare.h:37 ───────────────────────
// Net/Net_Events.cpp Recv_DevilSquareRequiredLevels.
const std::int32_t MAX_DS_LEVEL = 4;     // DevilSquare.h:6
struct PMSG_DEVIL_SQUARE_REQ_LEVELS_SEND
{
    PBMSG_HEAD header;      // C1:8E
    std::int32_t m_DevilSquareRequiredLevel[MAX_DS_LEVEL][2];
};
static_assert(sizeof(PMSG_DEVIL_SQUARE_REQ_LEVELS_SEND) == 36, "C1:8E");
static_assert(offsetof(PMSG_DEVIL_SQUARE_REQ_LEVELS_SEND, m_DevilSquareRequiredLevel) == 4, "C1:8E niveles"); // padding en +3

// ── C1:8F ─ niveles de Blood Castle ── BloodCastle.h:39 ───────────────────────
// Net/Net_Events.cpp Recv_BloodCastleRequiredLevels.
const std::int32_t MAX_BC_LEVEL = 6;     // BloodCastle.h:7
struct PMSG_BLOOD_CASTLE_REQ_LEVELS_SEND
{
    PBMSG_HEAD header;      // C1:8F
    std::int32_t m_BloodCastleRequiredLevel[MAX_BC_LEVEL][4];
};
static_assert(sizeof(PMSG_BLOOD_CASTLE_REQ_LEVELS_SEND) == 100, "C1:8F");
static_assert(offsetof(PMSG_BLOOD_CASTLE_REQ_LEVELS_SEND, m_BloodCastleRequiredLevel) == 4, "C1:8F niveles"); // padding en +3

// ── C1:9C ─ experiencia por matar ── Protocol.h:309 ───────────────────────────
// Net_Process.cpp case 0x9C.
struct PMSG_REWARD_EXPERIENCE_SEND
{
    PBMSG_HEAD header;      // C1:9C
    std::uint8_t index[2];
    std::uint16_t experience[2];     // [0] = palabra alta, [1] = palabra baja
    std::uint8_t damage[2];
    std::uint32_t ViewDamageHP;         // GAMESERVER_EXTRA
    std::uint32_t ViewExperience;       // GAMESERVER_EXTRA
    std::uint32_t ViewNextExperience;   // GAMESERVER_EXTRA
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
    std::uint16_t Level;
};
static_assert(sizeof(PMSG_CHARACTER_DELETE_LEVEL_SEND) == 6, "C1:DD");
static_assert(offsetof(PMSG_CHARACTER_DELETE_LEVEL_SEND, Level) == 4, "C1:DD Level");   // padding en +3

// ── C1:DE ─ creación de personaje habilitada ── Protocol.h:337 ────────────────
struct PMSG_CHARACTER_CREATION_ENABLE_SEND
{
    PBMSG_HEAD header;      // C1:DE
    std::uint8_t result;
};
static_assert(sizeof(PMSG_CHARACTER_CREATION_ENABLE_SEND) == 4, "C1:DE");
static_assert(offsetof(PMSG_CHARACTER_CREATION_ENABLE_SEND, result) == 3, "C1:DE result");

// ── C1:DF ─ nivel máximo de personaje ── Protocol.h:343 ───────────────────────
struct PMSG_CHARACTER_MAX_LEVEL_SEND
{
    PBMSG_HEAD header;      // C1:DF
    std::uint32_t MaxCharacterLevel;
};
static_assert(sizeof(PMSG_CHARACTER_MAX_LEVEL_SEND) == 8, "C1:DF");
static_assert(offsetof(PMSG_CHARACTER_MAX_LEVEL_SEND, MaxCharacterLevel) == 4, "C1:DF MaxCharacterLevel");

// ── C3:F3:03 ─ datos del personaje al entrar al mapa ── Protocol.h:399 ────────
// Net_Process.cpp Recv_JoinMapServer.
struct PMSG_CHARACTER_INFO_SEND
{
    PSBMSG_HEAD header;     // C3:F3:03
    std::uint8_t X;
    std::uint8_t Y;
    std::uint8_t Map;
    std::uint8_t Dir;
    std::uint32_t Experience;
    std::uint32_t NextExperience;
    std::uint16_t LevelUpPoint;
    std::uint16_t Strength;
    std::uint16_t Dexterity;
    std::uint16_t Vitality;
    std::uint16_t Energy;
    std::uint16_t Life;
    std::uint16_t MaxLife;
    std::uint16_t Mana;
    std::uint16_t MaxMana;
    std::uint16_t BP;
    std::uint16_t MaxBP;
    std::uint32_t Money;
    std::uint8_t PKLevel;
    std::uint8_t CtlCode;
    std::uint16_t FruitAddPoint;
    std::uint16_t MaxFruitAddPoint;
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
    std::uint8_t X;
    std::uint8_t Y;
    std::uint8_t Map;
    std::uint8_t Dir;
    std::uint16_t Life;
    std::uint16_t Mana;
    std::uint16_t BP;
    std::uint32_t Experience;
    std::uint32_t Money;
    std::uint32_t ViewCurHP;        // GAMESERVER_EXTRA
    std::uint32_t ViewCurMP;        // GAMESERVER_EXTRA
    std::uint32_t ViewCurBP;        // GAMESERVER_EXTRA
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
    std::uint16_t Level;
    std::uint16_t LevelUpPoint;
    std::uint16_t MaxLife;
    std::uint16_t MaxMana;
    std::uint16_t MaxBP;
    std::uint16_t FruitAddPoint;
    std::uint16_t MaxFruitAddPoint;
    std::uint16_t FruitSubPoint;
    std::uint16_t MaxFruitSubPoint;
    std::uint32_t ViewPoint;            // GAMESERVER_EXTRA
    std::uint32_t ViewMaxHP;            // GAMESERVER_EXTRA
    std::uint32_t ViewMaxMP;            // GAMESERVER_EXTRA
    std::uint32_t ViewMaxBP;            // GAMESERVER_EXTRA
    std::uint32_t ViewExperience;       // GAMESERVER_EXTRA
    std::uint32_t ViewNextExperience;   // GAMESERVER_EXTRA
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
    std::uint8_t result;            // 16 + tipo; 0 = rechazado
    std::uint16_t MaxLifeAndMana;
    std::uint16_t MaxBP;
    std::uint32_t ViewPoint;        // GAMESERVER_EXTRA
    std::uint32_t ViewMaxHP;        // GAMESERVER_EXTRA
    std::uint32_t ViewMaxMP;        // GAMESERVER_EXTRA
    std::uint32_t ViewMaxBP;        // GAMESERVER_EXTRA
    std::uint32_t ViewStrength;     // GAMESERVER_EXTRA
    std::uint32_t ViewDexterity;    // GAMESERVER_EXTRA
    std::uint32_t ViewVitality;     // GAMESERVER_EXTRA
    std::uint32_t ViewEnergy;       // GAMESERVER_EXTRA
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
    std::uint32_t Level;
    std::uint32_t LevelUpPoint;
    std::uint32_t Experience;
    std::uint32_t NextExperience;
    std::uint32_t Strength;
    std::uint32_t Dexterity;
    std::uint32_t Vitality;
    std::uint32_t Energy;
    std::uint32_t Life;
    std::uint32_t MaxLife;
    std::uint32_t Mana;
    std::uint32_t MaxMana;
    std::uint32_t BP;
    std::uint32_t MaxBP;
    std::uint32_t FruitAddPoint;
    std::uint32_t MaxFruitAddPoint;
    std::uint32_t ViewReset;
    std::uint32_t ViewGrandReset;
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
    std::uint32_t ViewCurHP;
    std::uint32_t ViewMaxHP;
    std::uint32_t ViewCurMP;
    std::uint32_t ViewMaxMP;
    std::uint32_t ViewCurBP;
    std::uint32_t ViewMaxBP;
    std::uint32_t ViewPhysiSpeed;
    std::uint32_t ViewMagicSpeed;
    std::uint32_t ViewPhysiDamageMin;
    std::uint32_t ViewPhysiDamageMax;
    std::uint32_t ViewMagicDamageMin;
    std::uint32_t ViewMagicDamageMax;
    std::uint32_t ViewMagicDamageRate;
    std::uint32_t ViewAttackSuccessRate;
    std::uint32_t ViewDamageMultiplier;
    std::uint32_t ViewDefense;
    std::uint32_t ViewDefenseSuccessRate;
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
    std::uint8_t count;
};
static_assert(sizeof(PMSG_HEALTH_BAR_SEND) == 6, "F3:E2");
static_assert(offsetof(PMSG_HEALTH_BAR_SEND, count) == 5, "F3:E2 count");

struct PMSG_HEALTH_BAR
{
    std::uint16_t index;
    std::uint8_t type;
    std::uint8_t rateHP;
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
    std::uint8_t count;
};
static_assert(sizeof(PMSG_ITEM_STACK_LIST_SEND) == 6, "F3:E3");
static_assert(offsetof(PMSG_ITEM_STACK_LIST_SEND, count) == 5, "F3:E3 count");

struct ITEM_STACK
{
    std::int32_t ItemIndex;
    std::int32_t Level;
    std::int32_t MaxStack;
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
    std::uint8_t count;
};
static_assert(sizeof(PMSG_ITEM_VALUE_LIST_SEND) == 6, "F3:E4");
static_assert(offsetof(PMSG_ITEM_VALUE_LIST_SEND, count) == 5, "F3:E4 count");

struct ITEM_VALUE_INFO
{
    std::int32_t Index;
    std::int32_t Level;
    std::int32_t BuyValue;
    std::int32_t SellValue;
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
    std::uint8_t PKLimitFree;
    std::uint8_t count;
};
static_assert(sizeof(PMSG_MOVE_LIST_SEND) == 7, "F3:E5");
static_assert(offsetof(PMSG_MOVE_LIST_SEND, PKLimitFree) == 5, "F3:E5 PKLimitFree");
static_assert(offsetof(PMSG_MOVE_LIST_SEND, count) == 6, "F3:E5 count");

struct MOVE_LIST_INFO
{
    std::uint8_t MapNumber;
    char MapName[32];
    std::uint8_t CanMove; // Booleano de red: 0/1; validado al recibir.
    std::int16_t MinLevel;
    std::int16_t MaxLevel;
    std::int16_t MinReset;
    std::int16_t MaxReset;
    std::int16_t AccountLevel;
    std::uint32_t Money;
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
// Consumido por UI/EventTimer.cpp.
struct PMSG_EVENT_TIME_SEND
{
    PSWMSG_HEAD header;     // C2:F3:E6
    std::uint8_t count;
};
static_assert(sizeof(PMSG_EVENT_TIME_SEND) == 6, "F3:E6");
static_assert(offsetof(PMSG_EVENT_TIME_SEND, count) == 5, "F3:E6 count");

struct PMSG_EVENT_TIME
{
    char name[32];
    std::uint8_t status;
    std::uint32_t time;
};
static_assert(sizeof(PMSG_EVENT_TIME) == 40, "F3:E6 entrada");
static_assert(offsetof(PMSG_EVENT_TIME, status) == 32, "F3:E6 status");
static_assert(offsetof(PMSG_EVENT_TIME, time) == 36, "F3:E6 time");          // padding en +33

// GoldenArcher.h del server: PMSG_NPC_GOLDEN_ARCHER_SEND (C1:94).
struct PMSG_NPC_GOLDEN_ARCHER_SEND
{
    PBMSG_HEAD header;
    std::uint8_t Type;
    std::int16_t Count;
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
    std::int32_t count;
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
    char GuildName[kGuildNameSize];
    std::uint8_t type;
};
static_assert(sizeof(PMSG_GUILD_WAR_DECLARE_SEND) == 36, "C1:61");
static_assert(offsetof(PMSG_GUILD_WAR_DECLARE_SEND, GuildName) == 3, "C1:61 GuildName");
static_assert(offsetof(PMSG_GUILD_WAR_DECLARE_SEND, type) == 35, "C1:61 type");

// Protocol.h del server: PMSG_LIVE_CLIENT_RECV (C3:0E).
struct PMSG_LIVE_CLIENT_RECV
{
    PBMSG_HEAD header;
    std::uint32_t TickCount;
    std::uint16_t PhysiSpeed;
    std::uint16_t MagicSpeed;
};
static_assert(sizeof(PMSG_LIVE_CLIENT_RECV) == 12, "C3:0E");
static_assert(offsetof(PMSG_LIVE_CLIENT_RECV, TickCount) == 4, "C3:0E TickCount");
static_assert(offsetof(PMSG_LIVE_CLIENT_RECV, PhysiSpeed) == 8, "C3:0E PhysiSpeed");
static_assert(offsetof(PMSG_LIVE_CLIENT_RECV, MagicSpeed) == 10, "C3:0E MagicSpeed");

// Protocol.h del server: PMSG_LIVE_CLIENT_SEND (C1:0E), eco con padding Win32.
struct PMSG_LIVE_CLIENT_SEND
{
    PBMSG_HEAD header;
    std::uint32_t TickCount;
};
static_assert(sizeof(PMSG_LIVE_CLIENT_SEND) == 8, "C1:0E respuesta");
static_assert(offsetof(PMSG_LIVE_CLIENT_SEND, TickCount) == 4, "C1:0E eco");

// ── C2:F3:E7..E9 + C1:F3:EA ─ catálogo de contenido ── ContentCatalog.h ───────
// Item/ContentCatalog.cpp.  El server es la autoridad de las definiciones de
// items, monstruos y fog; manda la tabla completa antes de la respuesta del
// login, troceada en paquetes con esta cabecera.
const std::uint8_t CATALOG_VERSION = 1;
const std::uint8_t CATALOG_ITEM_HAS_MODEL = 0x01;
const std::uint8_t CATALOG_ITEM_HAS_GLOW = 0x02;
const std::uint8_t CATALOG_ITEM_HAS_POSE = 0x04;
const std::uint8_t CATALOG_ITEM_HAS_ENTITY = 0x08;   // modelo propio puesto en el personaje
const std::uint16_t CATALOG_MONSTER_HAS_MODEL = 0x01;

struct PMSG_CATALOG_HEAD
{
    PSWMSG_HEAD header;     // C2:F3:[E7|E8|E9]
    std::uint8_t version;
    std::uint16_t recordSize;
    std::uint16_t count;
    std::uint16_t chunk;
    std::uint16_t totalChunks;
};
static_assert(sizeof(PMSG_CATALOG_HEAD) == 14, "F3:E7 cabecera");
static_assert(offsetof(PMSG_CATALOG_HEAD, recordSize) == 6, "F3:E7 recordSize");
static_assert(offsetof(PMSG_CATALOG_HEAD, count) == 8, "F3:E7 count");

struct CATALOG_ITEM
{
    std::uint16_t Index;
    std::uint16_t Behavior;
    std::uint8_t Slot;
    std::uint8_t Skill;
    std::uint8_t Width;
    std::uint8_t Height;
    std::uint8_t TwoHand;
    std::uint8_t Flags;
    std::uint8_t Durability;
    std::uint8_t MagicDurability;
    char Name[32];
    std::uint16_t Level;
    std::uint16_t DamageMin;
    std::uint16_t DamageMax;
    std::uint16_t AttackSpeed;
    std::uint16_t WalkSpeed;
    std::uint16_t Defense;
    std::uint16_t MagicDefense;
    std::uint16_t DefenseSuccessRate;
    std::uint16_t MagicDamageRate;
    std::uint16_t RequireLevel;
    std::uint16_t RequireStrength;
    std::uint16_t RequireDexterity;
    std::uint16_t RequireEnergy;
    std::uint16_t RequireVitality;
    std::uint8_t RequireClass[4];
    std::uint8_t Resistance[4];
    std::uint32_t Value;
    std::uint32_t BuyMoney;
    std::uint32_t GlowColor;
    float PositionX;
    float PositionY;
    float RotationX;
    float RotationY;
    float RotationZ;
    float Scale;
    char ModelFolder[40];
    char ModelName[28];
    std::uint8_t CustomWing;        // índice + 1 de CustomWing.txt (0 = no es ala custom)
    std::uint8_t Reserved;
    std::uint16_t WingDefenseConstA;
    std::uint16_t WingIncDamageConstA;
    std::uint16_t WingIncDamageConstB;
    std::uint16_t WingDecDamageConstA;
    std::uint16_t WingDecDamageConstB;
    // Modelo con que se dibuja el item puesto (montura, pet) cuando no es el
    // mismo del inventario.
    char EntityFolder[40];
    char EntityName[28];
};
static_assert(sizeof(CATALOG_ITEM) == 264, "F3:E7 entrada");
static_assert(offsetof(CATALOG_ITEM, EntityFolder) == 196, "F3:E7 EntityFolder");
static_assert(offsetof(CATALOG_ITEM, CustomWing) == 184, "F3:E7 CustomWing");
static_assert(offsetof(CATALOG_ITEM, WingDefenseConstA) == 186, "F3:E7 WingDefenseConstA");
static_assert(offsetof(CATALOG_ITEM, Name) == 12, "F3:E7 Name");
static_assert(offsetof(CATALOG_ITEM, Level) == 44, "F3:E7 Level");
static_assert(offsetof(CATALOG_ITEM, RequireClass) == 72, "F3:E7 RequireClass");
static_assert(offsetof(CATALOG_ITEM, Value) == 80, "F3:E7 Value");
static_assert(offsetof(CATALOG_ITEM, GlowColor) == 88, "F3:E7 GlowColor");
static_assert(offsetof(CATALOG_ITEM, PositionX) == 92, "F3:E7 PositionX");
static_assert(offsetof(CATALOG_ITEM, ModelFolder) == 116, "F3:E7 ModelFolder");
static_assert(offsetof(CATALOG_ITEM, ModelName) == 156, "F3:E7 ModelName");

struct CATALOG_MONSTER
{
    std::uint16_t Index;
    std::uint8_t Kind;              // 0 NPC, 1 monstruo, 0xFF = el vanilla
    std::uint8_t Golden;
    float Scale;            // 0 = la vanilla
    std::uint16_t Level;
    std::uint16_t Flags;
    char Name[32];
    char ModelFolder[32];
    char ModelName[20];
};
static_assert(sizeof(CATALOG_MONSTER) == 96, "F3:E8 entrada");
static_assert(offsetof(CATALOG_MONSTER, Scale) == 4, "F3:E8 Scale");
static_assert(offsetof(CATALOG_MONSTER, Name) == 12, "F3:E8 Name");
static_assert(offsetof(CATALOG_MONSTER, ModelFolder) == 44, "F3:E8 ModelFolder");
static_assert(offsetof(CATALOG_MONSTER, ModelName) == 76, "F3:E8 ModelName");

struct CATALOG_MAP_FOG
{
    std::uint8_t Map;
    std::uint8_t DayRGB[3];
    std::uint8_t NightRGB[3];
    std::uint8_t Reserved;
};
static_assert(sizeof(CATALOG_MAP_FOG) == 8, "F3:E9 entrada");

// F3:EC — efectos de un item (Data/Custom/Items/*.json del server).
const std::uint8_t CATALOG_EFFECT_ON_EQUIPPED = 0x01;
const std::uint8_t CATALOG_EFFECT_SPRITE = 1;      // sprite en huesos, cada frame
const std::uint8_t CATALOG_EFFECT_PARTICLE = 2;    // partícula vanilla en huesos
const std::int32_t  CATALOG_EFFECT_MAX_BONES = 32;

struct CATALOG_EFFECT
{
    std::uint16_t Item;
    std::uint8_t On;
    std::uint8_t Type;
    std::uint16_t Bitmap;        // textura vanilla; 0xFFFF = la de Texture
    std::uint16_t Particle;      // tipo de partícula vanilla
    std::uint8_t SubType;
    std::uint8_t Chance;        // 1 de cada N frames
    std::uint8_t BoneCount;
    std::uint8_t Reserved;
    std::uint8_t Bones[CATALOG_EFFECT_MAX_BONES];
    float Color[3];
    float Scale;
    float PulseSpeed;   // pulso |sin(WorldTime * speed)|
    float PulseScale;
    float PulseColor;
    float Offset[3];
    char Texture[48];
};
static_assert(sizeof(CATALOG_EFFECT) == 132, "F3:EC entrada");

// F3:EF — líneas propias del tooltip de un item ("tooltip" en
// Data/Custom/Items del server).  Color: el índice de color del tooltip.
struct CATALOG_TOOLTIP
{
    std::uint16_t Item;
    std::uint8_t Color;
    std::uint8_t Reserved;
    char Text[60];      // UTF-8 (0.97.40)
};
static_assert(sizeof(CATALOG_TOOLTIP) == 64, "F3:EF entrada");

// F3:EE — pets custom (Data/Custom/Pets/*.json del server).
const std::uint8_t CATALOG_PET_MOVE_FOLLOW = 1;    // vuela detrás del dueño
const std::uint8_t CATALOG_PET_MOVE_ORBIT = 2;     // da vueltas alrededor del dueño
const std::uint8_t CATALOG_PET_FLAG_COLLECT = 0x01; // vuela hasta el zen cercano antes de levantarlo

struct CATALOG_PET
{
    std::uint16_t Item;
    std::uint8_t Movement;
    std::uint8_t BlendMesh;     // malla aditiva; 0xFF = ninguna
    float Radius;
    float Period;       // ms por vuelta
    float Height;
    float Scale;
    std::uint8_t Action;        // animación del modelo
    std::uint8_t Flags;         // CATALOG_PET_FLAG_*
    std::uint8_t CollectRange;  // en casillas: hasta dónde va a buscar el zen
    std::uint8_t Reserved;
};
static_assert(sizeof(CATALOG_PET) == 24, "F3:EE entrada");

// F3:F0 — textos traducidos (0.97.40, Data/Text/<idioma> del server).  Sólo
// llegan los del idioma que el cliente pidió con el F1:04.
const std::uint8_t CATALOG_TEXT_ITEM = 1;
const std::uint8_t CATALOG_TEXT_MONSTER = 2;
const std::uint8_t CATALOG_TEXT_SKILL = 3;
const std::uint8_t CATALOG_TEXT_CLIENT = 4;   // textos propios del cliente (ClientTextId)
const std::uint8_t CATALOG_TEXT_TOOLTIP = 5;  // línea de tooltip de un item: Index = item, Reserved = línea (0 a 5)

struct CATALOG_TEXT
{
    std::uint8_t Table;     // CATALOG_TEXT_*
    std::uint8_t Reserved;  // en los tooltips, el número de línea
    std::uint16_t Index;    // item, monstruo o skill
    char Text[64];          // UTF-8 en NFC
};
static_assert(sizeof(CATALOG_TEXT) == 68, "F3:F0 entrada");

struct PMSG_CATALOG_END_SEND
{
    PSBMSG_HEAD header;     // C1:F3:EA
    std::uint16_t itemCount;
    std::uint16_t monsterCount;
    std::uint32_t hash;
};
static_assert(sizeof(PMSG_CATALOG_END_SEND) == 12, "F3:EA");
static_assert(offsetof(PMSG_CATALOG_END_SEND, hash) == 8, "F3:EA hash");

} // namespace Proto
