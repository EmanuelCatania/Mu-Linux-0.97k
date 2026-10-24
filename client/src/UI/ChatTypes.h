#pragma once
#include "types.h"
#include "Entity/EntityTypes.h"
#include <cstddef>

// IDA: CHAT, 0x254 bytes; burbujas de chat sobre los personajes, pool de 100
// en 0x07E016F8 (DAT_07e016f8).
//
// Los nombres siguen al struct CHAT de MU 5.2 (ZzzInterface.cpp). En el 0.97k
// ID y Guild son más cortos y no está szShopTitle. Anclajes en IDA:
// CreateChat (0x00481BA0) escribe ID, Guild, Color, GuildColor, IDLifeTime,
// Text, LifeTime y Owner; MoveChat (0x004821A0) descuenta los tiempos.
#pragma pack(push, 1)
struct CHAT {
    char        ID[24];                // 0x000
    char        Guild[12];             // 0x018  "[nombre] "
    char        Color;                 // 0x024
    char        GuildColor;            // 0x025
    BYTE        _unk026[2];
    int         IDLifeTime;            // 0x028
    char        Text[2][256];          // 0x02C
    int         LifeTime[2];           // 0x22C
    CHARACTER*  Owner;                 // 0x234
    int         x;                     // 0x238
    int         y;                     // 0x23C
    int         Width;                 // 0x240
    int         Height;                // 0x244
    vec3_t      Position;              // 0x248
};
#pragma pack(pop)

#define MU_ASSERT_FIELD(T, f, off) static_assert(offsetof(T, f) == off, #T "::" #f)
static_assert(sizeof(CHAT) == 0x254, "CHAT");
MU_ASSERT_FIELD(CHAT, Guild, 0x018);
MU_ASSERT_FIELD(CHAT, Color, 0x024);
MU_ASSERT_FIELD(CHAT, GuildColor, 0x025);
MU_ASSERT_FIELD(CHAT, IDLifeTime, 0x028);
MU_ASSERT_FIELD(CHAT, Text, 0x02C);
MU_ASSERT_FIELD(CHAT, LifeTime, 0x22C);
MU_ASSERT_FIELD(CHAT, Owner, 0x234);
MU_ASSERT_FIELD(CHAT, x, 0x238);
MU_ASSERT_FIELD(CHAT, Position, 0x248);
#undef MU_ASSERT_FIELD

inline CHAT* AsChat(uintptr_t p) { return reinterpret_cast<CHAT*>(p); }

// Nombre y prefijo de guild para mostrar (0.97.40). CHAT::ID y Guild conservan
// el tamaño del binario (24 y 12 bytes); el texto completo sale del Owner.
const char* ChatDisplayName(const CHAT* c);
const char* ChatDisplayGuild(const CHAT* c);
inline CHAT* AsChat(void* p) { return static_cast<CHAT*>(p); }
inline const CHAT* AsChat(const void* p) { return static_cast<const CHAT*>(p); }
