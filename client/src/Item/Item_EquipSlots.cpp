// Item_EquipSlots.cpp
// Casilleros de equipo del inventario: qué item puede ir en cada uno.
#include <cstdint>
#include "stdafx.h"
#include "UI/ChatListBox.h"
extern "C" const char* Guild_GetMarkName(int row);
#include "Text/TextCore.h"
#include "Party/Party.h"
#include "Entity/EntityName.h"
#include "Net/NetText.h"
#include "Render/TerrainDraw.h"
void Terrain_BeginBufferedPass();
#include "Render/BufferedDraw.h"
#include "Game/RenderFxSnapshots.h"
#include "Entity/ObjectBounds.h"
#include "Platform/Text.h"
#include "Platform/ISocket.h"
#include "Platform/ITimer.h"
#include "Platform/IInput.h"
#include "Item/ContentCatalog.h"
#include "Item/ItemDefines.h"
#include "Item/RightClickMove.h"
#include "Config/UserSettings.h"
#include "Game/MapManager.h"
#include "structs.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

// El rango de guild pide C1 plano (Encrypt=0).
extern "C" void GuildCreator_CloseFromResult(void);

// Inventory pool bases — defined in src/Render/HUD_Pass3.cpp.
// Lo usan los llamadores de FUN_004d23b0 de abajo para recorrer los arrays de grilla correctos.
extern "C" BYTE OffsetInventoryItems[];
extern "C" BOOL ChaosBoxRequestClose(void);
extern "C" BYTE Inventory[];
extern "C" BYTE ShopItems[];   // pool dedicado de la tienda
extern "C" BYTE OffsetTradeItems[];
extern "C" BYTE OffsetWarehouseItems[];
extern "C" BYTE OffsetMixItems[];

// Regla de sub_4CDC70 para soltar `item` en el casillero `a5`: clase, dos
// manos, flechas, stats, nivel y restricciones de mapa.  La comparte el
// click derecho para equipar (RightClickMove.cpp).
bool Equip_CanPlace(const ITEM* item, int a5)
{
    if (!item || !CharacterMachine || !CharacterAttribute || !DAT_07abf5d8 || !DAT_07d78068) return false;
    BYTE* const CM = (BYTE*)CharacterMachine;
    BYTE* const CA = (BYTE*)CharacterAttribute;
    ITEM_ATTRIBUTE* const IA = (ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;
    // 0.97.20: las reglas por rango de tipo usan el vanilla que imita un
    // agregado; la clase requerida es la del propio item.
    const short pt    = (short)ItemBehaviorType(item->Type);
    // 0.97.20: los agregados se validan como el vanilla que imitan.
    const short left  = (short)ItemBehaviorType(*(short*)(CM + 536));
    const short right = (short)ItemBehaviorType(*(short*)(CM + 604));
    const int   cls   = CA[11] & 7;
    const BYTE* req   = IA[item->Type].RequireClass;
    bool ok = true;

    if ((*(BYTE*)&((CHARACTER*)(uintptr_t)(DAT_07abf5d8))->Class & 7) == 3) {       // Magic Gladiator
        if (!req[3] && !req[0] && !req[1]) ok = false;
    } else if (!req[*(BYTE*)&((CHARACTER*)(uintptr_t)(DAT_07abf5d8))->Class & 7]) {
        ok = false;
    }

    bool rightHandChecks = false;
    if (pt != 135 && pt != 143) {
        if (left != -1 && left != 135 && left != 143 && a5 == 1) {
            if (IA[pt].Width >= 2 && (pt < 192 || pt >= 224)) ok = false;
            if (IA[left].Width >= 2) ok = false;
        }
        if (right != -1 && right != 135 && right != 143 && a5 == 0) {
            if (IA[pt].Width >= 2) ok = false;
            if (IA[right].Width >= 2 && (right < 192 || right >= 224)) ok = false;
            rightHandChecks = true;                         // LABEL_118
        }
    }
    if (!rightHandChecks && a5 == 1) {
        if (pt == 143) ok = false;
        if (cls == 0 || cls == 2) {
            if (pt >= 0 && pt < 128) ok = false;
            if (pt >= 160 && pt < 192) ok = false;
            if (left == 143) {
                if (pt == 135 || pt < 128 || pt > 160) ok = false;
            } else if (pt == 135 && left != -1 && (left < 128 || left > 160)) {
                ok = false;
            }
        }
    } else if (rightHandChecks || a5 == 0) {
        // LABEL_118
        if (pt == 135) ok = false;
        if (cls == 2 && right == 135 && (pt < 128 || pt >= 160 || pt == 143)) ok = false;
    }

    // LABEL_164
    if (pt >= 430 && pt <= 435) ok = false;
    if (item->RequireStrength  > *(WORD*)(CA + 20)) ok = false;
    if (item->RequireDexterity > *(WORD*)(CA + 22)) ok = false;
    if (item->RequireEnergy    > *(WORD*)(CA + 26)) ok = false;
    const BYTE part = item->Part;
    if ((part >= 7 && part <= 11) && item->RequireLevel > *(WORD*)(CA + 14)) ok = false;

    bool invalid = false;
    if (part == 7) {
        if (pt >= 392 && pt < 416) invalid = true;
    } else if (part == 8) {
        if ((int)gMapManager.GetCurrentMap() == 7) {
            if (pt == 418 || pt == 419) invalid = true;
        } else if ((int)gMapManager.GetCurrentMap() == 10 && pt == 418) {
            invalid = true;
        }
    }
    if (!invalid && !ok) invalid = true;                    // LABEL_231
    if (!invalid && part != (BYTE)a5) {
        if (part == 10) {
            if (a5 != 11) invalid = true;
        } else if (part != 0 || a5 != 1 || IA[pt].Width >= 2) {
            invalid = true;
        }
    }
    return !invalid;
}

// FUN_004d1fc0 @ 0x004D1FC0 — Render Character Equipment Slots (12 slots).
// Port FIEL del IDA: 12 llamadas a sub_4CDC70(x, y, w, h, slotIdx) renderizando
// los slots del Character panel. STRUCT_DECRYPT/ENCRYPT (HashTable obfuscation)
// skipped per project policy.
//
// Slot layout (per IDA decompile):
//   slot  0 @ ( 15,  89) 40x60  Weapon LEFT     (CharacterMachine + 536)
//   slot  1 @ (134,  89) 40x60  Weapon RIGHT    (+ 604)
//   slot  2 @ ( 75,  46) 40x40  Helmet          (+ 672)  — se saltea si clase == 3 (MG)
//   slot  3 @ ( 75,  89) 40x60  Armor           (+ 740)
//   slot  4 @ ( 75, 152) 40x40  Pants           (+ 808)
//   slot  5 @ ( 15, 152) 40x40  Gloves          (+ 876)
//   slot  6 @ (134, 152) 40x40  Boots           (+ 944)
//   slot  7 @ (115,  46) 60x40  Wings           (+1012)
//   slot  8 @ ( 15,  46) 40x40  Helper / Pet    (+1080)
//   slot  9 @ ( 55,  89) 20x20  Ring 1          (+1148)
//   slot 10 @ ( 55, 152) 20x20  Ring 2          (+1216)
//   slot 11 @ (115, 152) 20x20  Pendant         (+1284)
//
// IDA: sub_4CDC70 (0x004CDC70) — casillero de equipo del inventario.
// NO dibuja: es la logica del casillero (el item lo dibuja RenderEquipment3D).
//   * Sin item levantado: hover (tooltip + Color 2), martillo de reparar (0x34)
//     y levantar el item equipado (SetCharacterClass, DeleteBug del pet).
//   * Con item levantado: valida si se puede equipar en este casillero (clase,
//     dos manos, flechas, stats, nivel, restricciones de mapa), pinta la casilla
//     (Color 2 = valido, 3 = invalido) y con click manda el 0x24.
extern "C" void __cdecl SyncPickedItemVisualState(void);
extern "C" void __cdecl FUN_004cdc70(float sx, float sy, float w, float h, int slotIdx)
{
    if ((int)EnableUse > 0 || DAT_07eaa165 /* EquipmentItem */) return;
    if (!CharacterMachine || !CharacterAttribute || !DAT_07abf5d8 || !DAT_07d78068) return;

    const int  a5  = slotIdx;
    const int  x0  = (int)DAT_07ea5288 + (int)sx;          // InventoryStartX + a1
    const int  y0  = (int)DAT_07ea5284 + (int)sy;          // InventoryStartY + a2
    const int  mx  = (int)DAT_083a427c;
    const int  my  = (int)DAT_083a4278;
    const bool inside = mx >= x0 && (double)x0 + w > (double)mx &&
                        my >= y0 && (double)y0 + h > (double)my;
    BYTE* const CM   = (BYTE*)CharacterMachine;
    BYTE* const CA   = (BYTE*)CharacterAttribute;
    BYTE* const slot = CM + 536 + 68 * a5;
    ITEM_ATTRIBUTE* const IA = (ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;
    ITEM* const picked = (ITEM*)DAT_07e91350;              // pPickedItem

    // ── Sin item levantado ────────────────────────────────────────────────
    if ((int)DAT_07e91388 <= 0) {
        const short type = *(short*)slot;
        if (type == -1 || !inside) return;
        DAT_07eaa164 = 1;                                   // byte_7EAA164
        // DESVIACION (DLL RightClickMove.cpp CheckUnequipItem): click derecho
        // manda el item al primer hueco libre del inventario.
        const bool rightClick = !DAT_083a4124 && MouseRButtonPush != 0;
        int unequipTarget = -1;
        if (rightClick) {
            MouseRButtonPush = 0;
            if (DAT_07eaa134) return;                       // modo reparar
            unequipTarget = RightClickMove_FindInventorySlot(type);
            if (unequipTarget < 0) return;
        }
        if (!DAT_083a4124 && !rightClick) {                 // MouseLButtonPush
            slot[64] = 2;                                   // Color: hover
            DAT_07ea9844 = 0;                               // byte_7EA9844
            DAT_07eaa160 = (uintptr_t)slot;          // CheckInventory
            DAT_07ea8408 = (DWORD)y0;                       // sy
            DAT_07ea840c = (DWORD)(x0 + (int)w / 2);        // sx
            return;
        }
        if (Teleport) return;                           // Teleport
        if (DAT_07eaa134) {                                 // RepairEnable_0
            const int kind = ItemBehaviorType(type);        // 0.97.20
            if ((kind >= 416 && kind <= 419) || kind == 426 || kind == 135 ||
                kind == 143 || kind >= 448 || (kind >= 391 && kind <= 403) ||
                (kind >= 430 && kind <= 435))
                return;
            DAT_083a4124 = 0;
            BYTE pkt[5] = { 0xC1, 0x05, 0x34, (BYTE)a5, (BYTE)DAT_07eaa138 };
            gNetwork.Send(pkt, sizeof(pkt));
            return;
        }
        DAT_083a4124 = 0;
        if ((int)gMapManager.GetCurrentMap() == 10) {
            // Icarus: no se puede sacar el unico item que permite volar
            // (alas 384..390 o Dinorant 419).
            int flying = 0;
            // 0.97.20: alas y Dinorant se reconocen por el comportamiento.
            const short wing = (short)ItemBehaviorType(*(short*)(CM + 1012));
            if (wing >= 384 && wing <= 390) flying = 1;
            if (ItemBehaviorType(*(short*)(CM + 1080)) >= 419) ++flying;
            const int kind = ItemBehaviorType(type);
            if (flying <= 1 && ((kind >= 384 && kind <= 390) || kind == 419))
                return;
        }
        DAT_07ea9800 = &OffsetInventoryItems[0];
        memcpy(DAT_07e91350, slot, 0x44);                   // pPickedItem = *slot
        DAT_07e91388 = 1;                                   // pPickedItem.Key (copia del port)
        SyncPickedItemVisualState();
        g_ItemMoveSourcePool = &OffsetInventoryItems[0];
        g_ItemMoveTargetPool = 0;
        *(short*)slot = -1;
        *(int*)(slot + 4) = 0;
        slot[27] = 0;
        DAT_07ea5b18 = (DWORD)a5;                           // Inventory[32].Type
        SetCharacterClass(AsCharacter(DAT_07abf5d8));         // SetCharacterClass(Hero)
        DAT_07eaa160 = 0;                                   // CheckInventory
        PlayBuffer(29, 0, 0);
        if (a5 == 8)
            DeleteBug((intptr_t)DAT_07abf5d8);
        if (rightClick) {
            DAT_07e11e78 = (DWORD)(unequipTarget + 12);
            DAT_07eaa165 = 1;                               // EquipmentItem
            g_ItemMoveTargetPool = &OffsetInventoryItems[0];
            SendRequestEquipmentItem(0, a5, picked, 0, unequipTarget + 12);
        }
        return;
    }

    // ── Con item levantado ────────────────────────────────────────────────
    if (!inside) return;
    const short pt    = picked->Type;
    const short left  = *(short*)(CM + 536);
    const bool  invalid = !Equip_CanPlace(picked, a5);
    if (invalid) {                                          // LABEL_220
        slot[64] = 3;
        if (DAT_083a4124) {
            DAT_083a4124 = 0;
            if (!DAT_07eaa165) Item_ReturnPickedItem();     // sub_4CD3B0
        }
        return;
    }

    slot[64] = 2;
    if (!DAT_083a4124) return;
    DAT_07eaa164 = 1;
    DAT_083a4124 = 0;
    DAT_07e11e78 = (DWORD)a5;
    if (a5 == 1 && pt >= 0 && pt < 96 && left == -1)
        DAT_07e11e78 = 0;

    const BYTE* src = DAT_07ea9800;
    BYTE srcFlag;
    if (src == &OffsetInventoryItems[0])      srcFlag = 0;
    else if (src == &OffsetWarehouseItems[0]) srcFlag = 2;
    else if (src == &OffsetMixItems[0])       srcFlag = 3;
    else                                      srcFlag = 1;  // trade

    if (srcFlag == 2 && DAT_00559f5f && !DAT_07eaa148) {
        // Baul con candado: pedir el PIN antes de sacar el item.
        DAT_07ea9810 = (DWORD)a5;
        DAT_07ea9808 = DAT_07ea5b18;
        DAT_07ea9804 = 2;
        DAT_07ea980c = 0;
        short* digits = (short*)&DAT_07e91394;
        for (short i = 0; i < 10; ++i) digits[i] = i;
        for (int n = 0; n < 20; ++n) {
            const int a = rand() % 10, b = rand() % 10;
            if (a != b) {
                digits[a] ^= digits[b];
                digits[b] ^= digits[a];
                digits[a] ^= digits[b];
            }
        }
        DAT_07eaa14c = 1;
        memset(DAT_07ea9814, 0, sizeof(DAT_07ea9814));
        return;
    }

    if (DAT_07eaa165) return;                               // EquipmentItem
    DAT_07eaa165 = 1;
    g_ItemMoveSourcePool = DAT_07ea9800 ? DAT_07ea9800
                                        : &OffsetInventoryItems[0];
    g_ItemMoveTargetPool = &OffsetInventoryItems[0];
    SendRequestEquipmentItem(srcFlag, (int)DAT_07ea5b18, picked, 0, (int)DAT_07e11e78);
}

void __cdecl FUN_004d1fc0(void) {
    if (!CharacterAttribute) return;
    FUN_004cdc70(15.0f,  46.0f,  40.0f, 40.0f, 8);
    FUN_004cdc70(115.0f, 46.0f,  60.0f, 40.0f, 7);

    BYTE classByte = *(BYTE*)((BYTE*)CharacterAttribute + 11);
    if ((classByte & 7) != 3) {  // not SM (Soul Master)
        FUN_004cdc70(75.0f,  46.0f,  40.0f, 40.0f, 2);
    }
    FUN_004cdc70(75.0f,  89.0f,  40.0f, 60.0f, 3);
    FUN_004cdc70(75.0f,  152.0f, 40.0f, 40.0f, 4);
    FUN_004cdc70(15.0f,  89.0f,  40.0f, 60.0f, 0);
    FUN_004cdc70(134.0f, 89.0f,  40.0f, 60.0f, 1);
    FUN_004cdc70(15.0f,  152.0f, 40.0f, 40.0f, 5);
    FUN_004cdc70(134.0f, 152.0f, 40.0f, 40.0f, 6);
    FUN_004cdc70(55.0f,  89.0f,  20.0f, 20.0f, 9);
    FUN_004cdc70(55.0f,  152.0f, 20.0f, 20.0f, 10);
    FUN_004cdc70(115.0f, 152.0f, 20.0f, 20.0f, 11);
}

// FUN_004d23b0 @ 0x004D23B0 — Inventory grid render + click dispatcher.
// Vive en `Item/Item_ClickHandler.cpp`. Signatura real (per IDA):
//   void __cdecl FUN_004d23b0(char* origin_x, int origin_y, short* inv_base,
//                             int grid_w, int grid_h, char mode_flag);
