// Recv_Item.cpp — paquetes del server: items: suelo, inventario, NPC, tienda, baúl y Chaos Machine.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "Item/ItemDefines.h"
#include "Item/ChaosMixRates.h"
#include "Net/Recv/NetRecv.h"

// ── ShopInsertItem (PORT FIEL de IDA sub_4CC0E0) ──────────────────────────────
// Inserta un item de tienda en el pool de la tienda (ShopItems, 120 celdas de
// 0x44 = grid 8×15), llenando su footprint Width×Height (de
// ItemAttribute[type]).  El item de tienda son 4 bytes:
// [typeLo][levelByte][durability][flags].  Key se setea solo en la celda
// primaria (el render usa Key>0 como gate); ItemConvert completa los campos
// derivados de cada celda.
// DESVIACIÓN: en el binario el pool de tienda es el overlay
// &Inventory[32].WalkSpeed; acá es un pool dedicado (ShopItems, HUD_Pass3.cpp).
void ShopInsertItem(int slot, const BYTE* Item)
{
    int type = ConvertItemType((BYTE*)Item);
    if (type == 255 || type < 0 || type >= ITEM_MAX_EX) return;
    BYTE* attrBase = (BYTE*)(uintptr_t)DAT_07d78068;
    if ((uintptr_t)attrBase < 0x100000u || (uintptr_t)attrBase >= 0x80000000u) return;
    BYTE* attr = attrBase + type * 0x40;
    int W = attr[32]; int H = attr[33];   // ITEM_ATTRIBUTE.Width / .Height
    if (W < 1) W = 1;
    if (H < 1) H = 1;
    for (int r = 0; r < H; ++r) {
        for (int c = 0; c < W; ++c) {
            int idx = slot + r * 8 + c;
            if (idx < 0 || idx >= 120) continue;    // bound del pool 8×15
            // El pool de tienda (ShopItems) usa el mismo convenio que el overlay del
            // original (&Inventory[idx].WalkSpeed, offset +24): el render (sub_4E38B0)
            // lee Type@+0, Level@+4, Durability@+26, Option1@+27, Key@+0x38 relativo a
            // cada celda. sub_4CC0E0 escribe con el mismo convenio.
            BYTE* cell = ShopItems + idx * 0x44;
            *(short*)(cell + 0)    = (short)type;                  // Type
            *(int*)(cell + 4)      = (int)Item[1];                 // Level (raw byte)
            cell[26]               = Item[2];                      // Durability
            cell[27]               = Item[3];                      // Option1
            *(DWORD*)(cell + 0x38) = (r == 0 && c == 0) ? 1u : 0u; // Key (gate render)
            // CRÍTICO: x/y = posición-origen del item en el grid (slot%8, slot/8),
            // escrito en TODAS las celdas del footprint (igual que sub_4CC0E0 ->x=a1%8
            // ->y=a1/8). El hover (Item_ClickHandler.cpp, FUN_004d23b0) normaliza celdas de
            // footprint al origen vía `inv_base + 34*(grid_w*y+x)`; con x/y=0 toda celda
            // normalizaría al slot 0 y el tooltip mostraría siempre el primer item.
            cell[62]               = (BYTE)(slot % 8);             // x
            cell[63]               = (BYTE)(slot / 8);             // y
            ItemConvert((int)(uintptr_t)cell, (int)Item[1], (int)Item[3]);
        }
    }
}

BYTE* ItemMove_GetPool(DWORD pool)
{
    return (BYTE*)(uintptr_t)pool;
}

int ItemMove_GetGridH(BYTE* pool)
{
    if (pool == OffsetWarehouseItems) return 15;
    if (pool == OffsetTradeItems || pool == OffsetMixItems) return 4;
    return 8;
}

int ItemMove_ToGridSlot(BYTE* pool, int slot)
{
    return slot;
}

BYTE* ItemMove_GetEquipSlotPtr(int slot)
{
    static const int kEquipOffsets[12] = {
        536, 604, 672, 740, 808, 876, 944, 1012, 1080, 1148, 1216, 1284
    };

    if (slot < 0 || slot >= 12 || DAT_07cf1ffc == 0) {
        return nullptr;
    }
    return (BYTE*)(uintptr_t)DAT_07cf1ffc + kEquipOffsets[slot];
}

bool ItemMove_IsInventoryEquipSlot(BYTE* pool, int slot)
{
    return pool == OffsetInventoryItems && slot >= 0 && slot < 12;
}

void ItemMove_RestoreSlot(BYTE* pool, int slot, const BYTE* item68)
{
    if (!item68) return;

    if (ItemMove_IsInventoryEquipSlot(pool, slot)) {
        BYTE* dst = ItemMove_GetEquipSlotPtr(slot);
        if (dst) {
            memcpy(dst, item68, sizeof(ITEM));
        }
        return;
    }

    int slotIndex = (pool == OffsetInventoryItems) ? slot : ItemMove_ToGridSlot(pool, slot);
    int slotMax = (pool == OffsetInventoryItems) ? 76 : (8 * ItemMove_GetGridH(pool));
    if (slotIndex >= 0 && slotIndex < slotMax) {
        int first = (pool == OffsetInventoryItems) ? 0 : 1;
        // item68 es el ITEM struct de 68 bytes copiado del slot al hacer pickup,
        // NO formato wire. InsertInventoryItem espera 4-5 bytes wire
        // [typeLo][optByte][dur][hi][ext]: pasarle el struct crudo reinterpretaría
        // Type-high/Level-int/etc como opciones. Se reconstruye el wire desde los
        // offsets conocidos de la struct (Type@0, Level@4, Durability@26,
        // Unkown@60=byteHi, byColorState@61=ext).
        BYTE wire[ITEM_INFO_SIZE];
        ItemWire_FromItem(item68, wire);
        InsertInventoryItem(pool, 8, ItemMove_GetGridH(pool), slotIndex, wire, first);
    }
}

void ItemMove_ApplyServerSlot(BYTE* pool, int slot, const BYTE* itemWire12, const BYTE* item68)
{
    if (!itemWire12 || !item68) return;

    if (ItemMove_IsInventoryEquipSlot(pool, slot)) {
        BYTE* dst = ItemMove_GetEquipSlotPtr(slot);
        if (dst) {
            memcpy(dst, item68, sizeof(ITEM));
            memcpy(dst, itemWire12, 12);
        }
        return;
    }

    int slotIndex = (pool == OffsetInventoryItems) ? slot : ItemMove_ToGridSlot(pool, slot);
    int slotMax = (pool == OffsetInventoryItems) ? 76 : (8 * ItemMove_GetGridH(pool));
    if (slotIndex >= 0 && slotIndex < slotMax) {
        BYTE item12[12];
        memcpy(item12, item68, 12);
        memcpy(item12, itemWire12, 12);
        int first = (pool == OffsetInventoryItems) ? 0 : 1;
        InsertInventoryItem(pool, 8, ItemMove_GetGridH(pool), slotIndex, item12, first);
    }
}

BYTE* ItemMove_GetActiveSecondaryPool()
{
    if (DAT_07eaa11b) return OffsetTradeItems;
    if (DAT_07eaa119) return OffsetWarehouseItems;
    if (DAT_07eaa11a) return OffsetMixItems;
    return OffsetTradeItems;
}

void ItemMove_ClearPoolPreview(BYTE* pool, int slots)
{
    if (!pool) return;
    for (int i = 0; i < slots; ++i) {
        pool[i * 0x44 + 0x40] = 0;
    }
}

void ItemMove_UpdateInventoryDurability(int slot, BYTE durability)
{
    if (slot < 12 || slot >= 76) {
        return;
    }

    ITEM* inv = (ITEM*)OffsetInventoryItems;
    ITEM* cell = &inv[slot - 12];
    if (cell->Type == -1) {
        return;
    }

    int originX = cell->x;
    int originY = cell->y;
    if (originX < 0 || originX >= 8 || originY < 0 || originY >= 8) {
        return;
    }

    ITEM* origin = &inv[originY * 8 + originX];
    if (origin->Type == -1) {
        return;
    }

    ITEM_ATTRIBUTE* attr = (ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;
    int width = 1;
    int height = 1;
    if (attr && origin->Type >= 0 && origin->Type <= 0xFFF) {
        width = attr[origin->Type].Width;
        height = attr[origin->Type].Height;
        if (width <= 0 || width > 8) width = 1;
        if (height <= 0 || height > 8) height = 1;
    }

    for (int dy = 0; dy < height; ++dy) {
        for (int dx = 0; dx < width; ++dx) {
            int x = originX + dx;
            int y = originY + dy;
            if (x < 0 || x >= 8 || y < 0 || y >= 8) continue;
            ITEM* it = &inv[y * 8 + x];
            if (it->Type == -1) continue;
            it->Durability = durability;
        }
    }
}

extern int g_ItemMoveTargetDurBefore;

bool ItemMove_LooksLikeStackMerge(BYTE* targetPool, int targetSlot, const BYTE* sourceItem68)
{
    if (!targetPool || !sourceItem68) {
        return false;
    }

    if (targetPool != OffsetInventoryItems || targetSlot < 12 || targetSlot >= 76) {
        return false;
    }

    ITEM* inv = (ITEM*)OffsetInventoryItems;
    ITEM* target = &inv[targetSlot - 12];
    short sourceType = *(short*)sourceItem68;
    if (sourceType < 0 || target->Type < 0) {
        return false;
    }

    if (target->Type != sourceType) {
        return false;
    }

    ITEM_ATTRIBUTE* attr = (ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;
    if (!attr || sourceType > 0xFFF) {
        return false;
    }

    if (attr[sourceType].Width != 1 || attr[sourceType].Height != 1) {
        return false;
    }

    BYTE sourceOpt = sourceItem68[4];
    BYTE targetOpt = (BYTE)target->Level;
    if (sourceOpt != targetOpt) {
        return false;
    }

    if (target->Durability <= 0 || target->Durability >= 255) {
        return false;
    }

    return true;
}

void ItemMove_ClearPickedState()
{
    DAT_07e91388 = 0;
    DAT_07eaa165 = 0;
    EnableUse = 0;
    DAT_07ea5b18 = 0xFFFFFFFFu;
    DAT_07e11e78 = 0xFFFFFFFFu;
    DAT_07ea9844 = 0;
    DAT_07ea9800 = 0;
    DAT_07eaa160 = 0;
    DAT_083a4124 = 0;
    DAT_083a42eb = 0;
    g_ItemMoveSourcePool = 0;
    g_ItemMoveTargetPool = 0;

    memset(DAT_07e91350, 0, sizeof(DAT_07e91350));
    *(short*)DAT_07e91350 = (short)0xFFFF;
    pPickedItem = -1;
    Level = 0;
    byte_7E9136B = 0;

    // (Aca se borraban registros enteros de DAT_07ea8448/5b68/9880/7bc0 y
    //  DAT_07e11fb0, que eran copias sueltas sin lectores.  Ahora son alias de
    //  campo de los pools reales y ese memset los pisaria corrido 0x38.)

    ItemMove_ClearPoolPreview(OffsetInventoryItems, 64);
    ItemMove_ClearPoolPreview(OffsetTradeItems, 32);
    ItemMove_ClearPoolPreview(OffsetMixItems, 32);
    ItemMove_ClearPoolPreview(OffsetWarehouseItems, 120);
}

bool s_bPacketAfterEquipmentItem = false;   // g_bPacketAfter_EquipmentItem

BYTE s_byPacketAfterEquipmentItem[4];        // g_byPacketAfter_EquipmentItem

// 0x24
void NetRecv_24(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Server response to PMSG_ITEM_MOVE_RECV (client
    // manda 0x24 para pedir un movimiento; el server responde con el mismo
    // opcode confirming or denying).
    //
    // Per server ItemManager.h:100 PMSG_ITEM_MOVE_SEND:
    //   PBMSG_HEAD header;   // C3:24 (3 bytes)
    //   BYTE result;         // 0xFF = denied, else success (= target slot)
    //   BYTE slot;           // target slot
    //   BYTE ItemInfo[4];    // wire-format item bytes
    //
    // After our C3 unwrap, Msg layout: [hdr=C1][size][24][result]
    //   [slot][ItemInfo×4]. So:
    //   Msg[3] = result
    //   Msg[4] = slot
    //   Msg[5..8] = ItemInfo
    NetLog("NET:  → 0x24 ItemMoveSend result=%02X slot=%d size=%d",
           Msg[3], Msg[4], Size);
    BYTE* sourcePool = ItemMove_GetPool(g_ItemMoveSourcePool);
    BYTE* targetPool = ItemMove_GetPool(g_ItemMoveTargetPool);
    if (!sourcePool) sourcePool = OffsetInventoryItems;
    if (!targetPool) targetPool = OffsetInventoryItems;
    if (Msg[3] == 0xFF) {
        bool stackMergeAck = ItemMove_LooksLikeStackMerge(targetPool, (int)Msg[4], (BYTE*)DAT_07e91350);
        // Server denied. Restore picked item to source slot OR
        // sólo limpia el flag de "cargando" (el slot de origen todavía tiene
        // el item — lo limpiamos localmente al levantarlo, pero el server
        // no lo movió). Lo reinsertamos en el origen.
        if (!stackMergeAck) {
            ItemMove_RestoreSlot(sourcePool, (int)DAT_07ea5b18, (BYTE*)DAT_07e91350);
        } else if (g_ItemMoveTargetDurBefore >= 0) {
            // Merge parcial o destino lleno: lo que no entró vuelve al origen.
            // El server no avisa el origen si le queda algo (InventoryAddItemStack).
            const ITEM* target = (const ITEM*)OffsetInventoryItems + ((int)Msg[4] - 12);
            const int added = (int)target->Durability - g_ItemMoveTargetDurBefore;
            const int left = (int)((ITEM*)DAT_07e91350)->Durability - (added > 0 ? added : 0);
            if (left > 0) {
                BYTE item[sizeof(ITEM)];
                memcpy(item, DAT_07e91350, sizeof(ITEM));
                ((ITEM*)item)->Durability = (BYTE)left;
                ItemMove_RestoreSlot(sourcePool, (int)DAT_07ea5b18, item);
            }
        }
        ItemMove_ClearPickedState();
        PlayBuffer(29, 0, 0);
    } else {
        if (Size >= 5 + ITEM_INFO_SIZE) {
            // Server confirmed the move. Msg[4] = target slot,
            // Msg[5..8] = ItemInfo (4 bytes, ItemByteConvert).
            //
            // El item se construye SOLO desde los 4 bytes wire del server
            // (autoritativo), igual que el snapshot F3/10 y el buy 0x32: el ITEM struct
            // agarrado (DAT_07e91350) NO está en formato wire y sus bytes 4-11
            // (Durability/Option1/x/y/Key…) se reinterpretarían como opciones/serial.
            // InsertInventoryItem lee hasta Item[4]; dejamos ext=0.
            BYTE targetSlot = Msg[4];
            BYTE itembytes[ITEM_INFO_SIZE];
            memcpy(itembytes, &Msg[5], ITEM_INFO_SIZE);
            {
                int t = ConvertItemType(itembytes);
                NetLog("NET:    0x24 place slot=%d wire=[%02X %02X %02X %02X] type=%d",
                       (int)targetSlot, itembytes[0], itembytes[1],
                       itembytes[2], itembytes[3], t);
            }

            if (ItemMove_IsInventoryEquipSlot(targetPool, (int)targetSlot)) {
                // Equip slot: InsertInventoryItem rutea a WriteEquipmentSlot
                // internamente cuando slotIdx < 12 y pool = main inv.
                InsertInventoryItem(OffsetInventoryItems, 8, 8,
                             (int)targetSlot, itembytes, 1);
            } else {
                int slotIndex = (targetPool == OffsetInventoryItems) ? (int)targetSlot : ItemMove_ToGridSlot(targetPool, (int)targetSlot);
                int slotMax = (targetPool == OffsetInventoryItems) ? 76 : (8 * ItemMove_GetGridH(targetPool));
                if (slotIndex >= 0 && slotIndex < slotMax) {
                    int first = (targetPool == OffsetInventoryItems) ? 0 : 1;
                    InsertInventoryItem(targetPool, 8, ItemMove_GetGridH(targetPool), slotIndex, itembytes, first);
                }
            }
        }
        // Algunas ramas del server confirman el movimiento con un paquete más corto
        // pero igual esperan que el cliente suelte el estado de arrastre.
        ItemMove_ClearPickedState();
        PlayBuffer(29, 0, 0);
    }
    // IDA ReceiveEquipmentItem L110-114: reaplicar el 0x3D postergado.
    if (s_bPacketAfterEquipmentItem) {
        ReceiveTradeExit97k(s_byPacketAfterEquipmentItem, 4);
        s_bPacketAfterEquipmentItem = false;
    }
    SeedQuickPotionTypesFromInventory();
}

// 0x30
void NetRecv_30(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ── ReceiveTalk (IDA 0x4301B0, server→client) ────────────────
    // El server ordena abrir la ventana de un
    // NPC al hablarle. Msg[3] = tipo:
    //   2 = Warehouse (baúl)   3 = Chaos Machine (mezcla)
    //   4/6 = Event window     5 = Server division
    //   default = Shop (comprar/vender)
    // El anti-tamper hash-table que en IDA envuelve el set de
    // ShopOpened se omite per policy — el efecto neto es ShopOpened=1.
    // La rama cliente→server (bEncrypted=0) del IDA es el SEND de la
    // request de "hablar"; nosotros no la usamos (mandamos directo).
    NetLog("NET:  → 0x30 ReceiveTalk type=%d size=%d", Msg[3], Size);
    InventoryOpened = 1;
    switch (Msg[3]) {
        case 2:  // Warehouse
            // (Sin exclusion mutua de paneles: no esta en IDA y el click al NPC ya
            //  exige ShopOpened == 0 y WarehouseOpened == 0.)
            WarehouseOpened = 1;
            DAT_00559f5f = 0;     // byte_559F5F
            DAT_07eaa14c = 0;     // dword_7EAA14C
            break;
        case 3:  // Chaos Machine (mix)
            gChaosMixRates.Reset();
            ChaosBoxCloseAck();   // mecanismo de cierre de MuEmu (catalogo A)
            ChaosMixOpened = 1;
            DAT_07eaa140 = 0;     // MixState = 0
            for (int i = 0; i < 4; i++)
                DAT_0055a3e8[i] = Msg[4 + i];
            SetErrorMessage(143);
            break;
        case 4:  // Event window (type 0)
            EventType = 0;
            CloseInventoryRelatedWindows();
            EventWindowOpened = 1;
            InventoryOpened = 1;
            break;
        case 5:  // Server division
            CloseInventoryRelatedWindows();
            InventoryOpened = 0;
            g_bServerDivisionEnable = 1;
            g_bServerDivisionAccept = 0;
            break;
        case 6:  // Event window (type 1)
            EventType = 1;
            CloseInventoryRelatedWindows();
            EventWindowOpened = 1;
            InventoryOpened = 1;
            break;
        default:  // Shop (buy/sell)
            ShopOpened = 1;
            *((BYTE*)&DAT_07eaa150 + 2) = 0;   // BYTE2(dword_7EAA150)=0
            break;
    }
    CharacterOpened = 0;
    GuildOpened     = 0;
    PartyOpened     = 0;
    PlayBuffer(25, 0, 0);
    PlayBuffer(28, 0, 0);
    // IDA termina con SetCursorPos(260*MouseX/640 escalado, MouseY).
    // Omitido a propósito: el DLL lo anula ("Fix move cursor NPC",
    // Patchs.cpp: NOP en 0x00430B9F y 0x00430BBD).
}

// 0x31
void NetRecv_31(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Listas de inventario de NPC.  MuEmu usa PWMSG_HEAD para
    // ChaosBox::GCChaosBoxSend, por lo que el layout demostrado es:
    //   [C2][sizeHi][sizeLo][31][type=3][count]
    //   count x [slot][ItemInfo(4)]
    // No comparte el framing C1 ni el stride 13 del snapshot de
    // warehouse.  IDA ReceiveTradeInventory (0x427560) primero
    // limpia exclusivamente OffsetMixItems y luego inserta cada
    // registro en su slot 8x4.
    if (Msg[0] == 0xC2 && Size >= 6 && Msg[4] == 3) {
        const BYTE count = Msg[5];
        // MuEmu sends this authoritative empty snapshot before its
        // 0x86 result=0 after a failed mix.  IDA ReceiveTradeInventory
        // renders the failure through this Chaos refresh (text 594,
        // sounds 67/48).  The server also uses type=3 for the initial
        // open snapshot, so preserve that case by requiring a mix
        // request already in flight.
        const bool mixFailedSnapshot = (DAT_07eaa140 == 1);

        for (int i = 0; i < 32; ++i) {
            BYTE* cell = OffsetMixItems + i * 0x44;
            *(short*)cell = (short)0xFFFF;
            *(DWORD*)(cell + 0x38) = 0;
        }

        const int recordSize = 1 + ITEM_INFO_SIZE;   // slot + item (0.97.20)
        const BYTE* record = Msg + 6;
        for (int i = 0; i < count && (6 + i * recordSize + recordSize) <= Size;
             ++i, record += recordSize) {
            const BYTE slot = record[0];
            if (slot >= 32) {
                continue;
            }

            BYTE itemInfo[ITEM_INFO_SIZE];
            memcpy(itemInfo, record + 1, ITEM_INFO_SIZE);
            InsertInventoryItem(OffsetMixItems, 8, 4, (int)slot,
                         itemInfo, 1);
        }

        if (mixFailedSnapshot) {
            DAT_07eaa140 = 2;
            UIChatLogWindow_AddText("", GlobalText[594], 2);
            PlayBuffer(67, 0, 0);
            PlayBuffer(48, 0, 0);
        }

        return;
    }

    // El resto de las ramas se conserva tal cual: el formato C1
    // histórico usa offsets distintos de las listas C2 de shop.
    if (Size < 5) return;
    sub = Msg[3];
    BYTE count = Msg[4];
    int cursor = 5;
    NetLog("NET:  -> 0x31 InventoryList sub=%d count=%d size=%d hdr=%02X", sub, count, Size, hdr);
    // Restos del dump crudo del 0x31: el loop de abajo quedó vacío y no hace nada.
    {
        // Dump en chunks de 32 bytes: NetLog trunca a 256 y devuelve
        // -1 → salía vacío. DbgLogPublic no trunca.
        for (int off = 0; off < Size && off < 128; off += 32) {
        }
    }

    // Rama SHOP/BAÚL con los offsets C2 reales (IDA ReceiveTradeInventory
    // 0x427560): type=Msg[4], count=Msg[5], records desde Msg[6] stride 5 =
    // [slot(1)][info(4)].  El handler de abajo asume layout C1 (Msg[3]/Msg[4],
    // stride 13).
    {
        BYTE listType  = Msg[4];
        BYTE listCount = Msg[5];
        // El server manda la lista del BAÚL con el MISMO opcode 0x31, el MISMO type=0
        // y el mismo formato C2 que la tienda (Warehouse.cpp:276 vs Shop.cpp:251; sólo
        // el ChaosBox usa type=3). Son indistinguibles por formato, así que el destino
        // se decide por QUÉ VENTANA está abierta — como hacía el original.
        // El 0x30 (que setea Warehouse/ShopOpened) siempre llega ANTES
        // que el 0x31, así que el flag ya está puesto acá.
        bool isWarehouseList = (Msg[0] == 0xC2) && (listType != 3) && WarehouseOpened;
        bool isShopList      = (Msg[0] == 0xC2) && (listType != 3) && !WarehouseOpened;

        if (isWarehouseList) {
            // Baúl: grid 8×15 (120 slots) en OffsetWarehouseItems.
            for (int i = 0; i < 120; ++i) {
                BYTE* c = OffsetWarehouseItems + i * 0x44;
                *(short*)c = (short)0xFFFF;
                *(DWORD*)(c + 0x38) = 0;
            }
            const BYTE* rec = Msg + 6;
            int placed = 0;
            const int recordSize = 1 + ITEM_INFO_SIZE;   // slot + item (0.97.20)
            for (int i = 0; i < listCount && (6 + i * recordSize + recordSize) <= Size; ++i, rec += recordSize) {
                BYTE itembytes[ITEM_INFO_SIZE];
                memcpy(itembytes, rec + 1, ITEM_INFO_SIZE);
                InsertInventoryItem(OffsetWarehouseItems, 8, 15, (int)rec[0], itembytes, 1);
                placed++;
            }
            NetLog("NET:    0x31 WAREHOUSE populated %d/%d items", placed, listCount);
            return;
        }

        if (isShopList) {
            // Limpiar pool de tienda: overlay en &Inventory[i].WalkSpeed
            // (offset +24). Type@+0=0xFFFF (vacío), Key@+0x38=0.
            for (int i = 0; i < 120; ++i) {
                BYTE* c = ShopItems + i * 0x44;
                *(short*)c = (short)0xFFFF;
                *(DWORD*)(c + 0x38) = 0;
            }
            const BYTE* rec = Msg + 6;
            int placed = 0;
            const int recordSize = 1 + ITEM_INFO_SIZE;   // slot + item (0.97.20)
            for (int i = 0; i < listCount && (6 + i * recordSize + recordSize) <= Size; ++i, rec += recordSize) {
                ShopInsertItem(rec[0], rec + 1);
                placed++;
            }
            NetLog("NET:    0x31 SHOP populated %d/%d items", placed, listCount);
            return;
        }
    }

    auto ClearItemPool = [](BYTE* pool, int slots) {
        for (int i = 0; i < slots; ++i) {
            BYTE* cell = pool + i * 0x44;
            *(short*)cell = (short)0xFFFF;
            memset(cell + 4, 0, 0x40);
        }
    };

    if (sub == 3 || sub == 5) {
        ClearItemPool(OffsetMixItems, 32);
        if (sub == 3 || sub == 5) {
            DAT_07eaa140 = 0;
        }
        for (int i = 0; i < count && cursor + 13 <= Size; ++i) {
            BYTE slot = Msg[cursor];
            if (slot < 32) {
                InsertInventoryItem(OffsetMixItems, 8, 4, (int)slot, (BYTE*)Msg + cursor + 1, 1);
            }
            cursor += 13;
        }
        return;
    }

    if (DAT_07eaa119 != 0) {
        ClearItemPool(OffsetWarehouseItems, 120);
        for (int i = 0; i < count && cursor + 13 <= Size; ++i) {
            BYTE slot = Msg[cursor];
            if (slot < 120) {
                InsertInventoryItem(OffsetWarehouseItems, 8, 15, (int)slot, (BYTE*)Msg + cursor + 1, 1);
            }
            cursor += 13;
        }
    }
}

// 0x32
void NetRecv_32(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Buy-response del shop
    // (PMSG_ITEM_BUY_SEND, ItemManager.cpp CGItemBuyRecv):
    //   [C1][08][32][result][i0][i1][i2][i3]  (Size=8)
    //   result = slot ABSOLUTO del inventario (>=12 = grid
    //            principal en result-12) donde cayó el item,
    //            o 0xFF si la compra falló (sin zen / sin espacio).
    //   i0..i3 = 4-byte ItemInfo (ItemByteConvert): index, level/
    //            opts, durability, hi/exc.
    // Usa InsertInventoryItem, el mismo path fiel que el snapshot F3/10
    // (stride 5 = slot + 4 bytes).
    // IDA ProtocolCore L826: InsertInventoryItem(&Inv, 8, 8, byte[3],
    // body+2, 0).
    BYTE result = Msg[3];
    NetLog("NET:  → 0x32 BuyResult slot=%d size=%d", result, Size);
    if (result == 0xFF) {
        // compra rechazada — el server ya avisó (GCNoticeSend);
        // no hay item que insertar.
        ItemMove_ClearPickedState();
    } else if (Size >= 4 + ITEM_INFO_SIZE) {
        BYTE itembytes[ITEM_INFO_SIZE];
        memcpy(itembytes, (BYTE*)Msg + 4, ITEM_INFO_SIZE);
        InsertInventoryItem(OffsetInventoryItems, 8, 8,
                     (int)result, itembytes, 1);
        ItemMove_ClearPickedState();
        PlayBuffer(29, 0, 0);
    }
    SeedQuickPotionTypesFromInventory();
    // IDA `case 0x32` (ProtocolCore L832) resetea `dword_5826D18`, el
    // cooldown de COMPRA, y lo hace INCONDICIONALMENTE — incluso con
    // result == 0xFF (compra rechazada).  Ese reset faltaba: mientras
    // los dos cooldowns compartian byte funcionaba de casualidad, y
    // al separarlos (0x05826D18 vs 0x05826D1C) el de compra quedo sin
    // quien lo bajara → sólo se podía comprar UNA vez por sesión.
    BuyCost = 0;
    // El reset de DAT_05826d1c (EnableUse) en este case es un add-on
    // del port —IDA no lo hace acá—, pero hoy es lo que evita que
    // equipar/usar se trabe.  Se conserva hasta portar sus writers
    // reales (InitGame / ReceiveLife / ReceiveDurability).
    DAT_05826d1c = 0;
}

// 0x33
void NetRecv_33(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Slot de trade aceptado por el server. Limpia el
    // item flag (server confirmed the swap completed).
    // Per IDA ProtocolCore L834-845.
    NetLog("NET:  → 0x33 TradeAck sub=%d", Msg[3]);
    BYTE flag = Msg[3];
    if (flag != 0) {
        if (flag == 0xFF || flag == 0xFE) {
            ItemMove_ClearPickedState();
            DAT_05826d1c = 0;
            UIChatLogWindow_AddText(nullptr, GlobalText[733], 2);
        } else {
            ItemMove_ClearPickedState();
            DAT_05826d1c = 0;
            if (Size >= 8 && DAT_07cf1ffc != 0) {
                *(DWORD*)((BYTE*)DAT_07cf1ffc + 1352) = *(DWORD*)(Msg + 4);
            }
            PlayBuffer(29, 0, 0);
        }
    } else {
        // result == 0: el server RECHAZO la venta (no esta abierta la
        // interfaz de shop, el slot no tiene item, o el item no es vendible
        // segun ItemMove.txt). Aca se llamaba `ItemMove_ClearPickedState()`,
        // que DESCARTA el item agarrado: como el pickup ya habia limpiado su
        // celda, el item desaparecia de pantalla aunque el server nunca lo
        // borro — reaparecia al reentrar (llega el F3/10) y mientras tanto su
        // celda quedaba en un estado inconsistente.
        //
        // IDA ProtocolCore L834 gatea TODO el case con `if (byte[3] != 0)`,
        // o sea con result 0 no toca nada y el item sigue agarrado.
        //
        // Desviacion consciente: en vez de dejarlo pegado al cursor lo
        // devolvemos a su slot de origen. El efecto observable es el mismo
        // (no se pierde) y ademas libera `EquipmentItem` (DAT_07eaa165,
        // el mismo global de IDA 0x07EAA165), que si queda seteado
        // bloquea los drops siguientes.
        RestorePickedItemToSource();
        DAT_05826d1c = 0;
    }
}

// 0x34
void NetRecv_34(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    NetLog("NET:  -> 0x34 Repair size=%d", Size);
    // IDA ProtocolCore case 0x34: `*((_DWORD *)ReceiveBuffer + 1)`.
    // PMSG_ITEM_REPAIR_SEND es PBMSG_HEAD (3 bytes) + DWORD money
    // alineado a 4, o sea el zen esta en +4.  Leerlo en +3 metia el
    // byte de padding y el zen quedaba en basura (ej. -835).
    if (Size >= 8 && DAT_07cf1ffc != 0) {
        DWORD gold = *(DWORD*)(Msg + 4);
        if (gold != 0) {
            *(DWORD*)((BYTE*)DAT_07cf1ffc + 1352) = gold;
            CalculateAll((int)(uintptr_t)DAT_07cf1ffc, 0, 0);   // sub_47E3C0
            PlayBuffer(37, 0, 0);
        }
    }
    DAT_05826d1c = 0;
}

// 0x81
void NetRecv_81(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    NetLog("NET:  -> 0x81 StorageGold size=%d", Size);
    if (Size >= 12 && DAT_07cf1ffc != 0) {
        BYTE result = Msg[3];
        if (result != 0) {
            DWORD storageGold = *(DWORD*)(Msg + 4);
            DWORD gold = *(DWORD*)(Msg + 8);
            BYTE* charMachine = (BYTE*)(uintptr_t)DAT_07cf1ffc;
            *(DWORD*)(charMachine + 1356) = storageGold;
            *(DWORD*)(charMachine + 1352) = gold;
        }
    }
}

// 0x82
void NetRecv_82(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    NetLog("NET:  -> 0x82 StorageExit");
    DAT_07eaa119 = 0;
    DAT_00559f5f = 0;
    DAT_07eaa14c = 0;
}

// 0x83
void NetRecv_83(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    NetLog("NET:  -> 0x83 StorageStatus size=%d", Size);
    if (Size >= 4) {
        BYTE status = Msg[3];
        switch (status) {
        case 0:
            DAT_00559f5f = 0;
            DAT_07eaa148 = 0;
            break;
        case 1:
            DAT_00559f5f = 1;
            DAT_07eaa148 = 0;
            break;
        case 10: SetErrorMessage(134); break;   // PIN incorrecto
        case 11: SetErrorMessage(135); break;   // ya tenia candado
        case 13: SetErrorMessage(138); break;   // codigo personal invalido
        case 12:
            // IDA ReceiveStorageStatus (0x434450): PIN aceptado.  Si
            // habia una accion esperando el PIN (la arma el drop o el
            // retiro de zen), se completa ahora.
            if (DAT_00559f5f && !DAT_07eaa148) {
                if ((int)DAT_07ea9804 == -1) {
                    Send_ActionRequest((unsigned char)DAT_07ea9808, (int)DAT_07ea980c);
                } else {
                    DAT_07eaa165 = 1;   // EquipmentItem
                    g_ItemMoveSourcePool = (DWORD)(uintptr_t)&OffsetWarehouseItems[0];
                    g_ItemMoveTargetPool = (DWORD)(uintptr_t)&OffsetInventoryItems[0];
                    SendRequestEquipmentItem((int)DAT_07ea9804, (int)DAT_07ea9808,
                        (ITEM*)DAT_07e91350, (int)DAT_07ea980c, (int)DAT_07ea9810);
                }
            }
            DAT_00559f5f = 1;
            DAT_07eaa148 = 1;
            break;
        default:
            break;
        }
    }
}

// 0x86
void NetRecv_86(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA 004366C0 ReceiveMix. MuEmu envía C1:08:86:<result>:ItemInfo[4].
    // La aceptación, consumo y resultado ya fueron decididos por el
    // servidor; el cliente sólo refleja su respuesta autoritativa.
    if (Size < 4) return;
    const BYTE result = Msg[3];
    switch (result) {
    case 1:
        DAT_07eaa140 = 2;
        for (int slot = 0; slot < 32; ++slot) {
            BYTE* item = OffsetMixItems + slot * 0x44;
            *(short*)item = (short)0xFFFF;
            *(DWORD*)(item + 0x38) = 0;
        }
        if (Size >= 4 + ITEM_INFO_SIZE)
            InsertInventoryItem(OffsetMixItems, 8, 4, 0, Msg + 4, 1);
        UIChatLogWindow_AddText("", GlobalText[595], 1);
        PlayBuffer(67, 0, 0);
        PlayBuffer(49, 0, 0);
        break;
    case 2:
    case 0x0B:
        DAT_07eaa140 = 0;
        UIChatLogWindow_AddText("", GlobalText[596], 2);
        break;
    case 4:
        CreateOkMessageBox(GlobalText[649]);
        DAT_07eaa140 = 2;
        break;
    case 9:
        CreateOkMessageBox(GlobalText[689]);
        DAT_07eaa140 = 2;
        break;
    default:
        DAT_07eaa140 = 2;
        break;
    }
}

// ChaosBox.h del server: respuesta C1:88 con rate/money alineados a DWORD.
void NetRecv_88(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    if (Size < (int)sizeof(Proto::PMSG_CHAOS_MIX_RATE_SEND)) return;
    Proto::PMSG_CHAOS_MIX_RATE_SEND packet;
    memcpy(&packet, Msg, sizeof(packet));
    gChaosMixRates.Receive(packet);
}

// 0x87
void NetRecv_87(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    gChaosMixRates.Reset();
    // IDA 004367D0 ReceiveMixExit: este ACK es el punto en que el
    // cliente descarta sus vistas; MuEmu ya ejecutó ChaosBoxInit y
    // gObjInventoryCommit antes de responderlo.
    ChaosBoxCloseAck();
    InventoryOpened = 0;
    CloseInventoryRelatedWindows();
    DAT_07e91388 = 0;
    // The category selector is client-local (unlike the original
    // box lifecycle), so dismiss it if a close ACK races it.
    if (DAT_083a7c24 == 143) {
        SetErrorMessage(0);
    }
}

// 0x23
void NetRecv_23(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveDropItem @ 0x0042F690 (port FIEL).
    // Server response after hero drops/moves an item.
    //   Msg[3] == 0  → drop FAILED → reset inventory drag UI
    //   Msg[3] != 0  → drop OK
    //     If Msg[4] >= 12 → equipment slot update (UI_Main path)
    //     Else            → inventory slot drop (CharacterMachine update)
    //   En los dos casos de éxito: limpia DAT_07e91388 (arrastre activo)
    //   and reset SendDropItem = -1
    if (Size < 4) return;
    BYTE result = Msg[3];
    NetLog("NET:  → 0x23 DropItem result=%d slot=%d", result, Size >= 5 ? Msg[4] : -1);
    // El item se sacó del slot de origen al hacer pickup (UI_Main limpia el
    // footprint). Por eso:
    //  - result==0 (drop rechazado por el server): hay que RESTAURAR el item a
    //    su slot de origen o desaparece de la vista (el server no lo removió).
    //  - result!=0 (drop OK): el server removió el item; sólo hay que soltar el
    //    cursor (el slot ya está vacío desde el pickup).
    if (result == 0) {
        ItemMove_RestoreSlot(OffsetInventoryItems, (int)DAT_07ea5b18,
                             (BYTE*)DAT_07e91350);
    } else {
        PlayBuffer(29, 0, 0);
    }
    ItemMove_ClearPickedState();
    DAT_07e91388 = 0;
    DAT_07e11990 = -1;  // SendDropItem = -1
}

// 0x22
void NetRecv_22(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveGetItem @ 0x0042F360 (port FIEL).
    // Respuesta del server cuando el héroe levanta un item del piso vía el envío 0x22.
    //   Msg[3] == 0xFF → pickup failed (no inventory space, etc.)
    //   Msg[3] == 0xFE → levanta zen; el monto es el DWORD BE en Msg[4..7]
    //                    stored at CharacterMachine + 0x548
    //   si no          → item en el slot Msg[3], datos en Msg[4..]
    if (Size < 4) return;
    BYTE slot = Msg[3];
    NetLog("NET:  → 0x22 GetItem slot=0x%02x", slot);
    // IDA arma un puntero `Item` en cada rama y DESPUÉS decide el
    // sonido una sola vez con ConvertItemType(Item) — ver el bloque
    // compartido al final del case.
    const BYTE* Item = nullptr;
    if (slot == 0xFF) {
        // Pickup failed
    } else if (slot == 0xFE) {
        if (Size >= 8 && DAT_07cf1ffc != 0) {
            DWORD gold = ((DWORD)Msg[4] << 24) | ((DWORD)Msg[5] << 16)
                       | ((DWORD)Msg[6] << 8)  |  (DWORD)Msg[7];
            BYTE* charMachine = (BYTE*)(uintptr_t)DAT_07cf1ffc;
            *(DWORD*)(charMachine + 0x548) = gold;
        }
        // IDA ReceiveGetItem L38-40: en la rama del zen `Item` queda
        // apuntando a CharacterMachine, no a los bytes del paquete.
        // Es una rareza del binario, pero es lo que hace: el
        // ConvertItemType de abajo termina leyendo los primeros bytes
        // de la struct del personaje, casi nunca da un tipo de joya y
        // por eso el zen suena con pGetItem.wav (29).
        Item = (const BYTE*)(uintptr_t)DAT_07cf1ffc;
    } else {
        // PMSG_ITEM_GET_SEND es [C3][08][22][result][i0..i3] = Size 8
        // (ItemInfo son 4 bytes, MAX_ITEM_INFO): no exigir el formato de 12
        // bytes (`Size >= 16`). InsertInventoryItem lee hasta Item[4]; dejamos ext=0.
        if (Size >= 4 + ITEM_INFO_SIZE && slot < 76) {
            BYTE itembytes[ITEM_INFO_SIZE];
            memcpy(itembytes, (BYTE*)Msg + 4, ITEM_INFO_SIZE);
            InsertInventoryItem(OffsetInventoryItems, 8, 8, (int)slot, itembytes, 1);
        }
        if (Size >= 4 + ITEM_INFO_SIZE) Item = (const BYTE*)Msg + 4;
    }

    // Sonido de pickup — IDA L61-71, compartido por las dos ramas:
    //   ConvertItemType(Item) in {461,462,464,399,470} → 49 (eGem.wav)
    //   resto                                          → 29 (pGetItem.wav)
    if (slot != 0xFF && Item != nullptr) {
        // ConvertItemType (0x0047B110): Item[0] + (Item[3] & 0x80) * 2.  Con
        // la rama del zen Item apunta a CharacterMachine: sólo los 4 primeros
        // bytes tienen sentido, como en el binario.
        int type = (int)Item[0] + ((Item[3] & 0x80) ? 256 : 0);
        bool jewel = (type == 461 || type == 462 || type == 464 ||
                      type == 399 || type == 470);
        PlayBuffer(jewel ? 49 : 29, (DWORD)(uintptr_t)Hero, 0);
    }
    DAT_07e11998 = -1;  // SendGetItem
}

// 0x28
void NetRecv_28(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Borrado de slot del lado del server. Lo usa mucho
    // stackable moves (source slot emptied after merge).
    if (Size < 5) return;
    BYTE slot = Msg[3];
    NetLog("NET:  -> 0x28 ItemDelete slot=%d flag=%d", slot, Msg[4]);
    if (slot < 76) {
        UI_Main((int)slot, (short*)OffsetInventoryItems, 8u);
    }
    SeedQuickPotionTypesFromInventory();
    PlayBuffer(29, 0, 0);
}

// 0x29
void NetRecv_29(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // IDA: ReceiveHelperItem (0x004321F0) — efecto con tiempo de una
    // pocion especial.  MuEmu: PMSG_ITEM_SPECIAL_TIME_SEND
    // [C3][06][29][number][pad][WORD time] (GCItemUseSpecialTimeSend).
    //   CharacterAttribute + 42 + 2*number = 24 * time
    //   number 0 -> +40 |= 1 y recalcula la velocidad de ataque
    //   number 1 -> +40 |= 2 y recalcula el dano (fisico y magico)
    // La rama sin encriptar de IDA es la respuesta anti-hack; MuEmu
    // lo manda siempre encriptado.
    if (Size < 6 || !CharacterAttribute) return;
    const BYTE number = Msg[3];
    BYTE* ca = (BYTE*)(uintptr_t)CharacterAttribute;
    *(WORD*)(ca + 42 + 2 * number) = (WORD)(24 * *(const WORD*)(Msg + 4));
    if (number == 0) {
        ca[40] |= 1;
        CalculateAttackSpeed((int)(uintptr_t)CharacterMachine);   // CalculateAttackSpeed
    } else if (number == 1) {
        ca[40] |= 2;
        Stats_CalcBase((int)(uintptr_t)CharacterMachine);   // Stats_CalcBase
        Stats_CalcMagicDmgRange((int)(uintptr_t)CharacterMachine);   // Stats_CalcMagicDmgRange
    }
    EnableUse = 0;
}

// 0x2A
void NetRecv_2A(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Actualización de durabilidad/cantidad del lado del server. La usa
    // stackable potions/jewels after partial merge.
    if (Size < 6) return;
    BYTE slot = Msg[3];
    BYTE durability = Msg[4];
    BYTE flag = Msg[5];
    NetLog("NET:  -> 0x2A ItemDur slot=%d dur=%d flag=%d", slot, durability, flag);

    if (slot < 12) {
        BYTE* equip = ItemMove_GetEquipSlotPtr((int)slot);
        if (equip && *(short*)equip != (short)0xFFFF) {
            equip[26] = durability;
            if (durability == 0) {
                UI_Main((int)slot, (short*)OffsetInventoryItems, 8u);
            }
        }
    } else if (slot < 76) {
        if (durability == 0) {
            UI_Main((int)slot, (short*)OffsetInventoryItems, 8u);
        } else {
            ItemMove_UpdateInventoryDurability((int)slot, durability);
        }
    }

    if (flag != 0) {
        EnableUse = 0;
    }
    SeedQuickPotionTypesFromInventory();
}

// 0x2F
void NetRecv_2F(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceiveDurability @ 0x00431EA0 (port FIEL).
    // Per-equipment-slot durability update.
    //   Msg[3] = inventory slot index (0..11)
    //   Msg[4] = new durability value
    //   Msg[5] = if non-zero, EnableUse = 0 (item just consumed/broke)
    // El original escribe en CharacterMachine + 68*slot + 562, que es
    // CharacterMachine.EquipmentSlots[slot].Durability.
    if (Size < 6) return;
    BYTE slot = Msg[3];
    BYTE durVal = Msg[4];
    BYTE consumed = Msg[5];
    NetLog("NET:  → 0x2F Durability slot=%d dur=%d consumed=%d", slot, durVal, consumed);
    if (DAT_07cf1ffc != 0 && slot < 12) {
        BYTE* charMachine = (BYTE*)(uintptr_t)DAT_07cf1ffc;
        *(charMachine + 68 * slot + 562) = durVal;
    }
    if (consumed) {
        EnableUse = 0;
    }
}
