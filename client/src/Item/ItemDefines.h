#pragma once
// ItemDefines.h — numeración y formato de item compartidos con el server
// (0.97.20, ver Source/MuServer/GameServer/ItemManager.h del server).
//
// - Vanilla: index = sección*32 + sub, 0..511.  Ningún literal cambia.
// - Agregados: sub-índices 32..511 de cada sección -> 512 + sección*480 + (sub-32).
// - Item en la red: 7 bytes, layout 5.2.  El nibble alto del byte 5 lleva los
//   bits 9-12 del índice; los bytes 4 y 6 quedan reservados.  Para un vanilla
//   los bytes 0-3 son los del formato de 4.

#include <windows.h>
#include <string.h>

constexpr int ITEM_MAX_SECTION = 16;
constexpr int ITEM_MAX_TYPE_VANILLA = 32;
constexpr int ITEM_MAX_VANILLA = ITEM_MAX_SECTION * ITEM_MAX_TYPE_VANILLA;   // 512
constexpr int ITEM_MAX_EX_PER_SECTION = 512 - ITEM_MAX_TYPE_VANILLA;          // 480
constexpr int ITEM_MAX_EX = ITEM_MAX_VANILLA + ITEM_MAX_SECTION * ITEM_MAX_EX_PER_SECTION; // 8192
constexpr int ITEM_INFO_SIZE = 7;
constexpr int ITEM_MODEL_BASE = 400;

// IDA: OpenItems (0x005079D0), RenderItem3D (0x004E1BE0).
// Bases vanilla: índices de item y slots de modelo son dominios distintos.
// Para agregados se mantiene ItemModel/ItemBehaviorType del catálogo.
constexpr short ITEM_SWORD_BASE = 0 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_AXE_BASE = 1 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_MACE_BASE = 2 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_SPEAR_BASE = 3 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_BOW_BASE = 4 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_STAFF_BASE = 5 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_SHIELD_BASE = 6 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_HELM_BASE = 7 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_ARMOR_BASE = 8 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_PANTS_BASE = 9 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_GLOVES_BASE = 10 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_BOOTS_BASE = 11 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_WING_BASE = 12 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_HELPER_BASE = 13 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_POTION_BASE = 14 * ITEM_MAX_TYPE_VANILLA;
constexpr short ITEM_ETC_BASE = 15 * ITEM_MAX_TYPE_VANILLA;

constexpr int MODEL_SWORD = ITEM_MODEL_BASE + ITEM_SWORD_BASE;
constexpr int MODEL_AXE = ITEM_MODEL_BASE + ITEM_AXE_BASE;
constexpr int MODEL_MACE = ITEM_MODEL_BASE + ITEM_MACE_BASE;
constexpr int MODEL_SPEAR = ITEM_MODEL_BASE + ITEM_SPEAR_BASE;
constexpr int MODEL_BOW = ITEM_MODEL_BASE + ITEM_BOW_BASE;
constexpr int MODEL_STAFF = ITEM_MODEL_BASE + ITEM_STAFF_BASE;
constexpr int MODEL_SHIELD = ITEM_MODEL_BASE + ITEM_SHIELD_BASE;
constexpr int MODEL_HELM = ITEM_MODEL_BASE + ITEM_HELM_BASE;
constexpr int MODEL_ARMOR = ITEM_MODEL_BASE + ITEM_ARMOR_BASE;
constexpr int MODEL_PANTS = ITEM_MODEL_BASE + ITEM_PANTS_BASE;
constexpr int MODEL_GLOVES = ITEM_MODEL_BASE + ITEM_GLOVES_BASE;
constexpr int MODEL_BOOTS = ITEM_MODEL_BASE + ITEM_BOOTS_BASE;
constexpr int MODEL_WING = ITEM_MODEL_BASE + ITEM_WING_BASE;
constexpr int MODEL_HELPER = ITEM_MODEL_BASE + ITEM_HELPER_BASE;
constexpr int MODEL_POTION = ITEM_MODEL_BASE + ITEM_POTION_BASE;
constexpr int MODEL_ETC = ITEM_MODEL_BASE + ITEM_ETC_BASE;
constexpr int MODEL_EVENT = 947;

constexpr int ITEM_WING_ELF = ITEM_WING_BASE + 0;
constexpr int MODEL_WING_ELF = ITEM_MODEL_BASE + ITEM_WING_ELF;
constexpr int ITEM_WING_HEAVEN = ITEM_WING_BASE + 1;
constexpr int MODEL_WING_HEAVEN = ITEM_MODEL_BASE + ITEM_WING_HEAVEN;
constexpr int ITEM_WING_SATAN = ITEM_WING_BASE + 2;
constexpr int MODEL_WING_SATAN = ITEM_MODEL_BASE + ITEM_WING_SATAN;
constexpr int ITEM_WING_SPIRITS = ITEM_WING_BASE + 3;
constexpr int MODEL_WING_SPIRITS = ITEM_MODEL_BASE + ITEM_WING_SPIRITS;
constexpr int ITEM_WING_SOUL = ITEM_WING_BASE + 4;
constexpr int MODEL_WING_SOUL = ITEM_MODEL_BASE + ITEM_WING_SOUL;
constexpr int ITEM_WING_DRAGON = ITEM_WING_BASE + 5;
constexpr int MODEL_WING_DRAGON = ITEM_MODEL_BASE + ITEM_WING_DRAGON;
constexpr int ITEM_WING_DARKNESS = ITEM_WING_BASE + 6;
constexpr int MODEL_WING_DARKNESS = ITEM_MODEL_BASE + ITEM_WING_DARKNESS;

constexpr int ITEM_HELPER_ANGEL = ITEM_HELPER_BASE + 0;
constexpr int MODEL_HELPER_ANGEL = ITEM_MODEL_BASE + ITEM_HELPER_ANGEL;
constexpr int ITEM_HELPER_IMP = ITEM_HELPER_BASE + 1;
constexpr int MODEL_HELPER_IMP = ITEM_MODEL_BASE + ITEM_HELPER_IMP;
constexpr int ITEM_HELPER_UNIRIA = ITEM_HELPER_BASE + 2;
constexpr int MODEL_HELPER_UNIRIA = ITEM_MODEL_BASE + ITEM_HELPER_UNIRIA;
constexpr int ITEM_HELPER_DINORANT = ITEM_HELPER_BASE + 3;
constexpr int MODEL_HELPER_DINORANT = ITEM_MODEL_BASE + ITEM_HELPER_DINORANT;

// Item.txt del server: (12,15) Chaos; (14,13/14/16/22) Bless/Soul/Life/Creation.
constexpr int ITEM_JEWEL_CHAOS = ITEM_WING_BASE + 15;
constexpr int ITEM_JEWEL_BLESS = ITEM_POTION_BASE + 13;
constexpr int ITEM_JEWEL_SOUL = ITEM_POTION_BASE + 14;
constexpr int ITEM_JEWEL_LIFE = ITEM_POTION_BASE + 16;
constexpr int ITEM_JEWEL_CREATION = ITEM_POTION_BASE + 22;

static_assert(MODEL_HELPER_ANGEL == 816 && MODEL_HELPER_DINORANT == 819, "Helpers vanilla");
static_assert(ITEM_WING_ELF == 384 && ITEM_WING_DARKNESS == 390, "Alas vanilla");
static_assert(MODEL_WING_ELF == 784 && MODEL_WING_DARKNESS == 790, "Modelos de alas vanilla");


// Tabla de modelos: 963 fijos del binario (OpenPlayers) más slots dinámicos
// para los modelos que define el catálogo del server (items >= 512 y
// monstruos agregados).
constexpr int MODEL_MAX_VANILLA = 963;
constexpr int MODEL_MAX_DYNAMIC = 512;
constexpr int MODEL_MAX_TOTAL = MODEL_MAX_VANILLA + MODEL_MAX_DYNAMIC;

// Tabla de texturas: 1450 del binario más una región propia del catálogo,
// que no se recicla al cambiar de mapa.
constexpr int BITMAP_MAX_VANILLA = 1450;
constexpr int BITMAP_MAX_CATALOG = 1024;
constexpr int BITMAP_MAX_TOTAL = BITMAP_MAX_VANILLA + BITMAP_MAX_CATALOG;

constexpr int GetItemSection(int index)
{
    return (index < ITEM_MAX_VANILLA) ? (index / ITEM_MAX_TYPE_VANILLA)
                                      : ((index - ITEM_MAX_VANILLA) / ITEM_MAX_EX_PER_SECTION);
}

constexpr int GetItemSub(int index)
{
    return (index < ITEM_MAX_VANILLA) ? (index % ITEM_MAX_TYPE_VANILLA)
                                      : (((index - ITEM_MAX_VANILLA) % ITEM_MAX_EX_PER_SECTION) + ITEM_MAX_TYPE_VANILLA);
}

inline int ItemWire_GetType(const BYTE* wire)
{
    return wire[0] | ((wire[3] & 0x80) << 1) | ((wire[5] & 0xF0) << 5);
}

// Escribe el índice en los bytes 0, 3 (bit 7) y 5 (nibble alto) sin tocar el
// resto de los bits del byte 3 (opción > 3 y excellent).
inline void ItemWire_SetType(BYTE* wire, int type)
{
    wire[0] = (BYTE)(type & 0xFF);
    wire[3] = (BYTE)((wire[3] & 0x7F) | (((type >> 8) & 1) << 7));
    wire[5] = (BYTE)(((type >> 9) & 0x0F) << 4);
}

// Arma los 7 bytes desde la struct ITEM (68 bytes) que guarda el cliente:
// Type@0, Level@4 (byte de opciones crudo), Durability@26, byte alto@60.
inline void ItemWire_FromItem(const BYTE* item68, BYTE* wire)
{
    memset(wire, 0, ITEM_INFO_SIZE);
    wire[1] = item68[4];
    wire[2] = item68[26];
    wire[3] = item68[60];
    wire[4] = item68[61];   // byColorState, como el formato viejo
    ItemWire_SetType(wire, *(const short*)item68);
}
