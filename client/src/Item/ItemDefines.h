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
