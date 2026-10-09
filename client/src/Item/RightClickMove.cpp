#include "stdafx.h"
#include "Item/RightClickMove.h"
#include "globals.h"
#include "functions.h"

namespace {
bool IsCellRangeFree(int x, int y, int width, int height)
{
    if (x + width > 8 || y + height > 8) return false;
    for (int dy = 0; dy < height; ++dy)
        for (int dx = 0; dx < width; ++dx)
            if (*(const short*)(OffsetInventoryItems + ((y + dy) * 8 + x + dx) * sizeof(ITEM)) != -1)
                return false;
    return true;
}
}

// DLL CItemManager::GetInventoryEmptySlot: de arriba-izquierda hacia abajo.
// Como en sub_4D5F20, la ocupación se decide sólo por el Type de la celda.
int RightClickMove_FindInventorySlot(int type)
{
    const ITEM_ATTRIBUTE* attr = (const ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;
    if (!attr || type < 0 || type >= 1024) return -1;
    const int width = attr[type].Width, height = attr[type].Height;
    if (width <= 0 || width > 8 || height <= 0 || height > 8) return -1;
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            if (IsCellRangeFree(x, y, width, height)) return y * 8 + x;
    return -1;
}

// DLL CItemManager::GetTargetSlotEquiped: el casillero del Part del item; las
// armas de una mano pasan a la otra mano y los anillos al otro anillo si el
// primero está ocupado.  Sólo casilleros vacíos: no hay intercambio.
int RightClickMove_FindEquipSlot(const ITEM* item)
{
    if (!item || item->Type < 0 || !CharacterMachine) return -1;
    const BYTE part = item->Part;
    int candidates[2] = { part, -1 };
    if (part == 0) candidates[1] = 1;
    else if (part == 10) candidates[1] = 11;
    else if (part > 11) return -1;

    const BYTE* cm = (const BYTE*)(uintptr_t)CharacterMachine;
    for (int slot : candidates) {
        if (slot < 0) continue;
        if (*(const short*)(cm + 536 + 68 * slot) != -1) continue;
        if (Equip_CanPlace(item, slot)) return slot;
    }
    return -1;
}
