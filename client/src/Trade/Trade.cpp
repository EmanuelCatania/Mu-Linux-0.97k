// Trade.cpp
// NPC shop / player-to-player trade system
//
// Packet handlers (server→client):
//   0x5a  Shop_ItemList        — NPC shop item list (inline in Net_Process)
//   0x5b  Shop_EntitySlots     @ 0x00435110  — assign shop slots to nearby entities
//   0x5c  Trade_EntityUpdate   — inline in Net_Process; entity[+0x1da]=slot, update flag
//   0x5d  Trade_EntityClear    — inline in Net_Process; clear trade slot on entity
//   0x60–0x63 son Guerra de guild en este cliente 0.97K. Net_Process los
//   despacha a los controladores de Guild; no son paquetes Trade.
//
// Helper functions:
//   FUN_00434dc0  = GuildMark_UpsertRecord — tabla de nombre/marca (implementada
//                     en Net_Process hasta recuperar su módulo Guild dedicado)
//   FUN_00423ce0  = Entity_UpdateTradeFlag — update entity[+0x2e9] for one entity
//   FUN_00423c80  = Shop_FindSlotByName — search shop table by name, return slot idx
//   UI_AddNotice  = UI_OpenWindow       — open/update a named UI window (title, mode)
//   FUN_00497870  = SetActionClass — auxiliar de acción de entidad + paquete de dirección
//   FUN_005142d0  = ShowErrorDialog     — modal dialog by message ID
//
// ── SHOP ITEM TABLE ──────────────────────────────────────────────────────────
//
//   Base: DAT_07e919b8  (entity_id column; stride 0x50 between slots)
//   Slot layout (0x50 bytes per slot):
//     +0x00  dword  entity_id       — which entity sells this item
//     +0x04  char[4] name[0..3]     — item name first 4 bytes
//     +0x08  char[4] name[4..7]     — item name last 4 bytes
//     +0x0C  byte   flag            — 0 = valid
//     +0x0D  byte[64] item_nibbles  — 0x40 nibbles (= 0x20 bytes raw item data)
//                                     packed: even nibble = high4, odd nibble = low4
//
//   Shop name search target: lpString_05826bfc (char[4], 4-byte aligned)
//   Cached slot index:        DAT_00559684  (-1 = not found)
//
// ── GLOBAL TRADE STATE ───────────────────────────────────────────────────────
//
//   DAT_05826d30  byte  trade_active     1 = trade window open
//   DAT_05826d31  byte  duel_mode        1 = duel (vs regular trade)
//   DAT_05826d32  byte  trade_param      from packet byte[0xc] in Trade_Open
//   DAT_05826c04  byte  trade_accepted   0 = not yet accepted
//   DAT_05826c00  dword temporal de solicitud heredada (la atribución anterior a 0x61 era errónea)
//   lpString_05826bfc  char[4]  active_shop_name  4-byte NPC name key
//   DAT_00559684  int   cached_shop_slot_index
//   DAT_07eaa0d0  dword  — set to 0xffffffff on trade entity clear (0x5d)
//   DAT_07eaa114  byte  — set to 0 on trade entity clear (0x5d)
//
// ── ENTITY TRADE OFFSETS (new, stride 0x394) ─────────────────────────────────
//
//   +0x1da  short  shop_slot_index  slot in shop table this entity trades; -1=none
//   +0x2e9  byte   trade_flag       0=none, 1=trading with local player, 2=selected shop
//
// ── MESSAGE STRINGS ───────────────────────────────────────────────────────────
//
//   Tablas de texto históricas y aisladas de 0x60–0x63. No son paquetes Trade
//   de 97K y el dispatcher no puede alcanzarlas intencionalmente.
//     stride 0x12C between entries at DAT_07d4fd58:
//     0 → DAT_07d4fd58   1 → DAT_07d4fe84   2 → DAT_07d4ffb0
//     3 → DAT_07d500dc   4 → DAT_07d50208   5 → DAT_07d50334   6 → DAT_07d50460
//     Label strings at DAT_05826dc8, stride 4 (short identifier per result)
//
//   Trade_ItemUpdate result codes (byte[3] in 0x63 packet):
//     0 → DAT_07d506b8   (no iVar8 change → sends opcode 0x6b+0x75)
//     1 → DAT_07d507e4   (iVar8=2 → sends opcode 0x71+0x79)
//     2 → DAT_07d50910   (iVar8=2)
//     3 → DAT_07d50a3c   (iVar8=0 → sends 0x6b+0x75)
//     4 → DAT_07d50b68   (iVar8=2 → sends 0x71+0x79)
//     5 → DAT_07d50c94   (iVar8=0)
//     6 → DAT_07d4cfa4   (iVar8=1 — special confirm)
//
// ── CLIENT→SERVER ACK PACKETS ─────────────────────────────────────────────────
//
//   Trade_Open (0x62) sends opcode 0x80 ACK after opening window
//   Trade_ItemUpdate (0x63):
//     result 0,3,5 → sends opcode 0x6b then 0x75
//     result 1,2,4 → sends opcode 0x71 then 0x79
//   Both use standard XOR handshake (key {0xe7,0x6d,0x3a,...,0xe8,0x56}) + send() retry loop

#include "stdafx.h"
#include "Trade.h"
#include "Monster/Monster.h"

// Forward declarations for functions defined later in this file or in other modules
int  __cdecl Entity_FindById(int key);
void __cdecl Entity_UpdateTradeFlag(BYTE* entity);
int  __cdecl Shop_FindSlotByName(BYTE* name);
// UI_ShowDialog = UIChatLogWindow_AddText = FUN_00480620 (declared in functions.h)
// ShowErrorDialog = SetErrorMessage = FUN_005142d0 (declared in functions.h, defined in GL_State.cpp)
void __cdecl UI_OpenWindow(char* title, int mode);

// Shop item table
#define SHOP_TABLE_BASE   0x07e919b8
#define SHOP_SLOT_STRIDE  0x50
#define SHOP_SLOT_COUNT   0x400   // approximate; bound check at 0x7ea51ec

// Global trade state
extern BYTE  g_trade_active;            // DAT_05826d30
extern BYTE  g_duel_mode;              // DAT_05826d31
extern BYTE  g_trade_param;            // DAT_05826d32
extern BYTE  g_trade_accepted;         // DAT_05826c04
extern DWORD g_trade_requester;        // DAT_05826c00
extern char  g_active_shop_name[4];    // lpString_05826bfc (at 0x05826bfc)
extern int   g_cached_shop_slot;       // DAT_00559684
extern BYTE* g_EntityBase;             // DAT_07abf5d0 (legacy alias, may be null)
extern DWORD g_LocalEntity;            // DAT_07abf5d8 (player entity ptr)

// 2026-05-07: g_EntityBase is never wired to the actual entity array — the real
// base lives in DAT_07abf5d0 (set by WinMain). Use that directly so the ENTITY
// macro doesn't yield a NULL deref.
#define ENTITY(idx)  ((BYTE*)DAT_07abf5d0 + (idx) * 0x394)


// ============================================================
// IDA: FUN_00434DC0
// Adds or updates an item in the shop table.
//   param_1 = entity_id   (dword — which entity sells this item)
//   param_2 = name[2]     (2 dwords = 8-byte item name key)
//   param_3 = item_data   (byte ptr — raw item data, 0x20 bytes)
// Returns slot index.
//
// Algorithm:
//   1. Walk table from slot 0 looking for matching name (strcmp on 8 bytes).
//   2. If found: overwrite slot in-place, return slot_idx.
//   3. If not found (walked past end): find first slot with name[0]==0, overwrite.
//   4. Pack item_data into 0x40 nibbles: even idx = byte[idx/2]>>4, odd idx = byte[idx/2]&0xf.
// ============================================================
int Shop_RegisterItem(DWORD entity_id, DWORD* name, BYTE* item_data)
{
    return 0;  // AUTO-SKIP: absolute end-bound loop (Ghidra artifact — pool not populated in our build).
    BYTE* table = (BYTE*)SHOP_TABLE_BASE - 4;  // base adjusted: entity_id 4 bytes before name
    // DAT_07e919b8 = entity_id col; DAT_07e919bc = name col (same slot, +4 offset)

    int slot = 0;
    BYTE* entry = (BYTE*)0x07e919bc;  // name column base

    while (true)
    {
        // Compare name (8 bytes) against slot
        if (memcmp(entry, name, 8) == 0)
        {
            // Match — overwrite this slot
            goto write_slot;
        }
        entry += SHOP_SLOT_STRIDE;
        slot++;
        if ((int)entry > 0x7ea51eb)
        {
            // Overflow — find first empty slot (name[0] == 0)
            slot = 0;
            entry = (BYTE*)0x07e919bc;
            do {
                if (*entry == 0) goto write_slot;
                entry += SHOP_SLOT_STRIDE;
                slot++;
            } while ((int)entry < 0x7ea51ec);
            return 0;
        }
    }

write_slot:
    {
        DWORD* slot_base = (DWORD*)(0x07e919b8 + slot * SHOP_SLOT_STRIDE);
        slot_base[0] = entity_id;          // +0x00 entity_id
        *(DWORD*)(entry + 0)  = name[0];   // +0x04 name[0..3]
        *(DWORD*)(entry + 4)  = name[1];   // +0x08 name[4..7]
        entry[8] = 0;                       // +0x0C flag

        // Pack item_data bytes into 0x40 nibbles at entry[+0x0D]
        for (UINT i = 0; i < 0x40; i++)
        {
            BYTE nibble;
            if ((i & 1) == 0)
                nibble = item_data[i / 2] >> 4;
            else
                nibble = item_data[i / 2] & 0xF;
            entry[0x0D + i] = nibble;
        }
    }
    return slot;
}


// ============================================================
// PacketHandler_0x5a  — NPC shop item list (inline in Net_Process)
// Packet: [C1][len][5a][count][item_entries...]
//   count     = byte[2]
//   per entry (stride 0x2a):
//     bytes[0..1] = entity_id (big-endian)
//     bytes[2..9] = item name key (8 bytes)
//     bytes[0xA..0x29] = item data (0x20 bytes)
//
// Calls: Shop_RegisterItem(entityId, name_ptr, data_ptr) for each entry.
// ============================================================
void PacketHandler_0x5a(BYTE* pkt)
{
    int count = pkt[2];
    BYTE* entry = pkt + 3;

    for (int i = 0; i < count; i++, entry += 0x2a)
    {
        DWORD entity_id = (entry[0] << 8) | entry[1];
        DWORD* name_ptr = (DWORD*)(entry + 2);
        BYTE*  data_ptr = entry + 0xA;
        Shop_RegisterItem(entity_id, name_ptr, data_ptr);
    }
}


// ============================================================
// IDA: FUN_00435110
// For each entity in packet, assign entity[+0x1da] = shop_slot_index.
// Also updates entity[+0x2e9] trade flag (0/1/2).
//
// Packet: [C1][len][5b][?][count][entries...]
//   count = byte[4]
//   per entry (stride 4):
//     byte[-3], byte[-2] = entity_id (big-endian)  [relative to puVar9]
//     byte[-1], byte[0]  = shop name key (2 bytes compared against table)
//
// Trade flag values for entity[+0x2e9]:
//   0 = not in any trade
//   1 = trading with same NPC as local player  (entity[+0x1da] == local_entity[+0x1da])
//   2 = trading with the searched shop (g_active_shop_name match)
// ============================================================
void Shop_EntitySlots(BYTE* pkt)
{
    int count = (BYTE)pkt[4];
    if (count == 0) return;

    BYTE* entry = pkt + 8;  // puVar9 starts at param_1+8
    for (int i = 0; i < count; i++, entry += 4)
    {
        // entity_id from bytes[-3..–2] relative to current entry ptr
        DWORD entity_id = ((BYTE)entry[-3] << 8) | (BYTE)entry[-2];
        int entity_idx = Entity_FindById(entity_id);
        if (entity_idx >= 400) continue;            // not-found: skip
        BYTE* ent = ENTITY(entity_idx);

        // 2-byte name key from bytes[-1..0]
        WORD name_key = ((BYTE)entry[-1] << 8) | (BYTE)entry[0];

        // BUG-FIX 2026-05-03: shop table is at literal `0x07e919b8` (unmapped in
        // our build) and the bound `0x7ea51e8` is also a literal. Until the
        // 1238-slot shop table is properly allocated, leave slot = -1 (no match).
        SHORT slot = -1;
        *(SHORT*)(ent + 0x1da) = slot;

        // Update trade flag
        Entity_UpdateTradeFlag(ent);
    }
}


// ============================================================
// PacketHandler_0x5c  — Entity trade/duel update (inline in Net_Process)
// Packet: [C1][len][5c][entity_hi][entity_lo][item_data 8 bytes]
//   entity_id = byte[3]*256 + byte[2]
//   Calls Shop_RegisterItem(0xffffffff, pkt+5, pkt+0xd) to register item,
//   then entity[+0x1da] = returned slot, Entity_UpdateTradeFlag(entity).
// ============================================================
void PacketHandler_0x5c(BYTE* pkt)
{
    DWORD entity_id = ((BYTE)pkt[3] << 8) | (BYTE)pkt[2];
    int entity_idx = Entity_FindById(entity_id);
    if (entity_idx >= 400) return;       // unknown entity — skip
    BYTE* ent = ENTITY(entity_idx);
    if (!ent) return;                    // entity base not yet initialised

    int slot = Shop_RegisterItem(0xFFFFFFFF, (DWORD*)(pkt + 5), pkt + 0xD);
    *(SHORT*)(ent + 0x1da) = (SHORT)slot;
    Entity_UpdateTradeFlag(ent);
}


// ============================================================
// PacketHandler_0x5d  — Trade entity clear (inline in Net_Process)
// Packet: [C1][len][5d][entity_hi][entity_lo]
//   entity_id = byte[3]*256 + byte[2]
//   entity[+0x1da] = 0xffff
//   DAT_07eaa114 = 0; DAT_07eaa0d0 = 0xffffffff
// ============================================================
void PacketHandler_0x5d(BYTE* pkt)
{
    DWORD entity_id = ((BYTE)pkt[3] << 8) | (BYTE)pkt[2];
    int entity_idx = Entity_FindById(entity_id);
    if (entity_idx >= 400) return;       // unknown entity — skip
    BYTE* ent = ENTITY(entity_idx);
    if (!ent) return;

    *(SHORT*)(ent + 0x1da) = (SHORT)0xFFFF;
    // BUG-FIX 2026-05-03: was writing to literal source-binary addresses
    // 0x07eaa114 and 0x07eaa0d0 — random memory in our build. Use the symbols
    // that the linker actually placed those values at.
    DAT_07eaa114 = 0;
    DAT_07eaa0d0 = 0xFFFFFFFF;
}


// ============================================================
// IDA: FUN_00423CE0
// Updates entity[+0x2e9] for one entity:
//   0 = entity not active (entity[+0x00] == 0)
//   1 = entity[+0x1da] == local_player[+0x1da]  (same shop as player)
//   2 = entity[+0x1da] == g_cached_shop_slot     (matches searched shop name)
// g_cached_shop_slot is lazily resolved from g_active_shop_name via Shop_FindSlotByName.
// ============================================================
void Entity_UpdateTradeFlag(BYTE* entity)
{
    BYTE* local = (BYTE*)g_LocalEntity;

    if (*entity == 0) return;  // entity not active

    entity[0x2e9] = 0;

    SHORT local_slot = *(SHORT*)(local + 0x1da);
    if (local_slot != -1 && *(SHORT*)(entity + 0x1da) == local_slot)
        entity[0x2e9] = 1;

    if (g_trade_active)
    {
        if (g_cached_shop_slot == -1)
        {
            if (g_active_shop_name[0] == 0) return;
            g_cached_shop_slot = Shop_FindSlotByName((BYTE*)g_active_shop_name);
        }
        if (g_cached_shop_slot >= 0 && *(SHORT*)(entity + 0x1da) == g_cached_shop_slot)
            entity[0x2e9] = 2;
    }
}


// ============================================================
// IDA: FUN_00423C80
// Linear search of shop item table by 4-byte name key.
// Returns slot index, or -1 if not found.
// ============================================================
int Shop_FindSlotByName(BYTE* name)
{
    return -1;  // AUTO-SKIP: absolute end-bound loop (Ghidra artifact — pool not populated in our build).
    int slot = 0;
    BYTE* entry = (BYTE*)0x07e919bc;  // name column

    while ((int)entry < 0x7ea51ec)
    {
        if (memcmp(entry, name, 4) == 0)
            return slot;
        entry += SHOP_SLOT_STRIDE;
        slot++;
    }
    return -1;
}


// ============================================================
// Clasificación histórica errónea, conservada sólo como datos de referencia no despachados.
// No asociar este cuerpo con IDA: FUN_004353E0 (ReceiveDeclareWarResult).
// Packet: [C1][len][60][result_code]
//   result_code 0-6: different outcome strings shown to player
//
// If result != 1: clears trade flags on all active entities.
//
// String table (stride 0x12C starting at 0x07d4fd58):
//   0 = request sent (or "Player is busy")
//   1 = accepted
//   2-6 = rejection reasons / errors
// Label table at DAT_05826dc8, stride 4 (short UI label per result).
// ============================================================
static void LegacyMisclassified_TradeRequestResult(BYTE* pkt)
{
    BYTE result = pkt[3];

    // BUG-FIX 2026-05-03: previous tables held literal source-binary addresses
    // (msg_table 0x07d4fd58.., label_table 0x05826dc8..) — unmapped in our
    // build → AV the moment a 0x60 response arrived. Until the localized text
    // pool is wired through GlobalText[], use ASCII placeholders.
    static const char* msg_table[] = {
        "Player is busy",
        "Trade accepted",
        "Trade rejected",
        "Trade timeout",
        "Trade cannot start (range)",
        "Trade cannot start (state)",
        "Trade error",
    };
    static const char* label_table[] = {
        "Trade", "Trade", "Trade", "Trade", "Trade", "Trade", "Trade",
    };

    if (result <= 6)
        UI_ShowDialog(label_table[result], msg_table[result], 2);  // FUN_00480620

    // Reset trade flags on all entities when trade not accepted
    if (result != 1 && !g_trade_active)
    {
        g_trade_active = 0;
        g_cached_shop_slot = -1;
        g_active_shop_name[0] = 0;

        BYTE* ent = g_EntityBase + 0x2e9;
        for (int i = 400; i > 0; i--, ent += 0x394)
        {
            if (ent[-0x2e9] != 0)  // entity is active
            {
                SHORT local_slot = *(SHORT*)((BYTE*)g_LocalEntity + 0x1da);
                *ent = 0;
                if (local_slot != -1 && *(SHORT*)(ent - 0x10F) == local_slot)
                    *ent = 1;
            }
        }
    }
}


// ============================================================
// Clasificación histórica errónea; no es IDA: FUN_00435390 (ReceiveDeclareWar).
// Packet: [C1][len][61][name4 bytes][info4 bytes][?][duel_flag]
//   bytes[3..6]  = requester name key → g_active_shop_name
//   bytes[7..10] = requester info    → g_trade_requester
//   byte[0xb]    = 1 if duel, 0 if trade
//
// Opens modal dialog 0x80 ("X wants to trade/duel with you?").
// ============================================================
static void LegacyMisclassified_TradeIncomingReq(BYTE* pkt)
{
    *(DWORD*)g_active_shop_name = *(DWORD*)(pkt + 3);  // lpString_05826bfc
    g_trade_requester = *(DWORD*)(pkt + 7);             // DAT_05826c00
    g_trade_accepted = 0;                                // DAT_05826c04

    FUN_005142d0(0x80);  // ShowErrorDialog — "wants to trade" / "wants to duel" dialog

    if (pkt[0xb] == 1)
        g_duel_mode = 1;
}


// ============================================================
// Clasificación histórica errónea; no es IDA: FUN_004354F0 (ReceiveGuildBeginWar).
// Packet: [C1][len][62][name4][info4][duel_flag][trade_param][...]
//   bytes[3..6]  = shop name key → g_active_shop_name
//   bytes[7..10] = shop info     → g_trade_requester
//   byte[0xb]    = 0=trade, 1=duel (affects window title)
//   byte[0xc]    = trade_param  → g_trade_param
//
// Trade flag state:
//   g_trade_active = 1
//   Scans shop table for name match → g_cached_shop_slot
//   Updates entity[+0x2e9] for all 400 entities
//
// Sends ACK opcode 0x80 (XOR encrypted, standard send retry loop).
// ============================================================
static void LegacyMisclassified_TradeOpen(BYTE* pkt)
{
    // BUG-FIX 2026-05-03: function originally read format strings from absolute
    // source-binary addresses (`(char*)0x07d5058c`, `(char*)0x07d50dc0`) that are
    // unmapped memory in our build, and walked a shop table at literal
    // `0x07e919bc` (also unmapped). Both would AV the moment a trade/duel
    // packet arrived. Until proper format-string globals are added and the
    // shop table is properly allocated, fall back to plain ASCII titles +
    // skip the slot scan so the rest of the trade UI can still open.
    char window_title[100];

    g_trade_accepted = 0;
    *(DWORD*)g_active_shop_name = *(DWORD*)(pkt + 3);  // lpString_05826bfc
    g_trade_requester = *(DWORD*)(pkt + 7);

    g_trade_active = 1;

    if (pkt[0xb] == 0)
        crt_sprintf(window_title, (char*)"Trade with %s");   // placeholder
    else
    {
        crt_sprintf(window_title, (char*)"Duel with %s");    // placeholder
        g_duel_mode = 1;
    }

    UI_OpenWindow(window_title, 1);  // UI_AddNotice — open trade/duel UI

    g_trade_param = pkt[0xc];

    // Shop table not allocated in our build — leave cached slot at -1.
    g_cached_shop_slot = -1;

    // Update trade flags for all active entities
    BYTE* ent = g_EntityBase + 0x2e9;
    for (int i = 400; i > 0; i--, ent += 0x394)
    {
        if (ent[-0x2e9] != 0)
        {
            SHORT local_slot = *(SHORT*)((BYTE*)g_LocalEntity + 0x1da);
            *ent = 0;
            if (local_slot != -1 && *(SHORT*)(ent - 0x10F) == local_slot)
                *ent = 1;
            if (g_cached_shop_slot >= 0 && *(SHORT*)(ent - 0x10F) == g_cached_shop_slot)
                *ent = 2;
        }
    }

    // Send ACK opcode 0x80 (trade window open confirmation)
    // Standard XOR encrypt + send() retry loop (same key as WinMain.cpp)
    // FUN_00497870(local_player, local_player, 0x80, 0x80) before send
    // [C1][03][80] XOR-encrypted
}


// ============================================================
// Clasificación histórica errónea; no es IDA: FUN_00435AA0 (ReceiveGuildEndWar).
// Packet: [C1][len][63][result_code]
//   result_code 0-6 → message string, see table above
//
// After showing message, clears all trade globals and entity flags.
// Then sends one of two ACK pairs depending on result:
//   result 0, 3, 5 → send opcode 0x6b then 0x75
//   result 1, 2, 4 → send opcode 0x71 then 0x79
//   result 6       → special (iVar8=1, no standard ACK)
// ============================================================
static void LegacyMisclassified_TradeItemUpdate(BYTE* pkt)
{
    BYTE result = pkt[3];
    char msg_buf[132];
    int ack_type = 0;  // 0=send(0x6b+0x75), 1=special, 2=send(0x71+0x79)

    // BUG-FIX 2026-05-03: result_table held literal source-binary addresses
    // (0x07d506b8..0x07d4cfa4) for result strings — unmapped in our build.
    // Replace with ASCII placeholders until proper text-pool wiring exists.
    static const struct { const char* str; int ack; } result_table[] = {
        { "Trade complete",          0 },  // 0
        { "Other player canceled",   2 },  // 1
        { "Trade failed",            2 },  // 2
        { "Trade item invalid",      0 },  // 3
        { "Inventory full",          2 },  // 4
        { "Trade canceled",          0 },  // 5
        { "Trade error",             1 },  // 6
    };

    if (result <= 6)
    {
        crt_sprintf(msg_buf, result_table[result].str);
        ack_type = result_table[result].ack;
    }

    UI_OpenWindow(msg_buf, 1);  // UI_AddNotice — show result in window

    // Clear all trade state
    g_trade_active = 0;
    g_duel_mode = 0;
    g_cached_shop_slot = -1;
    g_active_shop_name[0] = 0;

    // Reset trade flags for all active entities
    BYTE* ent_ptr = (BYTE*)g_EntityBase;
    BYTE* local = (BYTE*)g_LocalEntity;
    for (int off = 0; off < 0x394 * 400; off += 0x394)
    {
        BYTE* ent = ent_ptr + off;
        if (*ent != 0)
        {
            SHORT local_slot = *(SHORT*)(local + 0x1da);
            ent[0x2e9] = 0;
            if (local_slot != -1 && *(SHORT*)(ent + 0x1da) == local_slot)
                ent[0x2e9] = 1;
        }
    }

    // Send ACK — uses FUN_00497870 for anim check, then standard XOR+send
    if (ack_type == 0)
    {
        // Send [C1][03][6b] then [C1][03][75]
    }
    else if (ack_type == 2)
    {
        // Send [C1][03][71] then [C1][03][79]
    }
    // ack_type == 1: no standard ACK (result 6 = special confirm)
}
