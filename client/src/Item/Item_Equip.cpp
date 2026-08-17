// Item_Equip.cpp
// Item_TryEquip @ 0x00483B30  (1366 lines, 306 basic blocks)
//
// Validates and executes item equip / use actions for a character.
// Sends 0xC1/0x26 (equip) and 0xC1/0x12 (use) packets to the server.
//
// Ghidra metrics similarity: 85.6% match with Game_SceneUpdate — same anti-tamper
// hash-table blocks and XOR key make up most of the "lines".
//
// ── SIGNATURE ────────────────────────────────────────────────────────────────
//
//   undefined1 FUN_00483b30(void)
//   [63 phantom in_stack params — anti-tamper obfuscation, not real args]
//
//   Real parameters read via `in_stack_` offsets (passed by caller on stack):
//     in_stack_00001ad8  (float* / item_t*) — item struct pointer
//     in_stack_00001ac8  (void*)            — caller ExceptionList (used for returns)
//     in_stack_0000001c  (undefined4*)      — output: item slot index
//     in_stack_00001adc  (int)              — item context/pool pointer
//     in_stack_000000a4  (undefined**)      — packet frame vtable ptr
//     in_stack_000000aa  (char)             — packet header byte [0xC1]
//
//   Returns undefined1 = success byte (from uStack_8._3_1_)
//
// ── ITEM STRUCT LAYOUT (at in_stack_00001ad8) ────────────────────────────────
//
//   +0x00  — (general fields)
//   +0x02  ushort  item_class_req   — class restriction code (0x186 = DW)
//   +0x105 byte    item_category    — item type/category byte (see switch below)
//   +0xbb  byte    flag_field       — cleared on wing/aura equip
//
// ── PLAYER STRUCT (DAT_07abf5d8) ─────────────────────────────────────────────
//
//   +0x270  short  player_level     — current level
//   +0x2b8  short  player_class     — class ID (0x332, 0x333 = variants?)
//   +0x1c1  char[] player_name      — character name string
//
// ── ITEM REQUIREMENTS TABLE (DAT_07ea8410) ───────────────────────────────────
//
//   Stride 0x22 per entry.
//   Indexed by slot index (local_c = FUN_00482be0(3) result).
//   entry[0x00] = ushort: equip type code:
//     0x1c0 → UI_SetScene(0x21) (show equip dialog type A)
//     0x1c1-0x1c9 → UI_SetScene(0x20) (show equip dialog type B)
//
// ── ITEM CATEGORY CHECKS (uVar16 = item[+0x105]) ─────────────────────────────
//
//   Early return (invalid equip) when:
//
//   item_class == 0x186 (Dark Wizard class restrictions):
//     category == 0x5c   → return 0  (shield — DW cannot use)
//     0x37 < cat < 0x3c  → return 0  (wings range — DW restriction)
//     category == 0x51   → return 0  (staff — wrong type?)
//
//   item_class != 0x186:
//     2 < category < 5   → return 0  (wizard-only items)
//
//   category == 0x2f (e.g. Chaos weapon?):
//     player_class NOT in {0x332, 0x333} → return 0
//     item_class != 0x186 → return 0
//     player_level < 0x1f0 OR > 0x20f → return 0  (level range 496-527)
//
//   category == 0x2b:
//     item_class != 0x186 → return 0
//     player_level in 0x230-0x24f → return 0  (level range 560-591 excluded)
//
// ── LEVEL REQUIREMENT CHECK ───────────────────────────────────────────────────
//
//   FUN_0047e7a0(category, (char*)1, NULL, &out_req_level, NULL)
//     → Item_GetRequiredLevel(category, ...) — fills out_req_level
//   if (DAT_07cf1ff4[+0x1e] < out_req_level):
//     // player level (ushort at char_data+0x1e) < item required level
//     → FUN_00482be0(3) → Item_GetFreeSlot(3)  — find equip slot
//     → build and send 0xC1/0x26 equip packet (see below)
//     → check DAT_07ea8410[slot * 0x22] for dialog type
//     → return 0
//
// ── 0xC1/0x26 EQUIP PACKET ───────────────────────────────────────────────────
//
//   Opcode: 0x26 (item equip request)
//   Size: 5 bytes
//   Header: [0xC1][5][0x26]
//   Payload: [1 byte slot] [1 byte zero]
//   XOR encrypted with 4-byte key {0xe7, 0x6d, 0x3a, 0x89}
//   Send via: send(DAT_055ca168, ...) with WSAEWOULDBLOCK → queue at DAT_055ca16c
//   Cooldown timer: DAT_05826d1c = 10 (throttle, checked > 0 → skip send)
//
// ── ITEM TYPE SWITCH (secondary dispatch, *(ctx+0x25+slot)) ──────────────────
//
//   After slot lookup at `in_stack_00001adc + 0x25 + slot`:
//
//   case 0x12 (Wing / Aura equip):
//     entity[+0xbb] = 0  (clear wing flag)
//     if item_class == 0x186:
//       FUN_0043e820(entity, 0x5c)  → Particle_SpawnOnEntity(entity, 0x5c=92)
//                                      (wing aura spark for DW)
//     else:
//       FUN_00444410(entity)         → Aura_SetCharEffect(entity) (other class aura)
//     Location check:
//       strlen(DAT_00559d74) — current zone/map name
//       strstr(DAT_07abf5d8+0x1c1, DAT_00559d74) — player name contains zone string?
//     Cooldown: GetTickCount vs _DAT_05826cf4 (300ms gate)
//     If cooldown ok: send another packet (same XOR key, smaller payload)
//       _DAT_05826cf4 = GetTickCount()
//
//   [other cases]: similar item-use handling
//   FUN_00480620(&DAT_07e11de8, &DAT_07d4c89c, 2) — Widget_Draw (item preview?)
//
// ── ITEM STAT CALCULATION ─────────────────────────────────────────────────────
//
//   FUN_00483780(item, item, ctx, category)  → Item_CalcStats or Item_GetEquipInfo
//     Called after successful equip validation.
//     Result stored in uStack_8._3_1_ (return value).
//
// ── CHAR DATA UPDATE ──────────────────────────────────────────────────────────
//
//   At exit: same g_CharData XOR re-encoding as Game_EnterWorldTick:
//     operator_new(0x584) → decrypt → re-encrypt with DAT_00559050 key
//   Finalizes character state after equip.
//
// ── FUNCTION CROSS-REFERENCE ─────────────────────────────────────────────────
//
//   FUN_0047e7a0  → Item_GetRequiredLevel(category, flag, NULL, &out_level, NULL)
//   FUN_00482be0  → Item_GetFreeSlot(type) — returns slot index or -1
//   FUN_00483780  → Item_CalcStats(item, item, ctx, category)
//   FUN_0043e820  → Particle_SpawnOnEntity(entity, type)
//   FUN_00444410  → Aura_SetCharEffect(entity)  (non-DW aura)
//   FUN_00404bc0  → UI_SetScene(id, 0, 0)
//                    0x20 = equip dialog B (item in valid slot)
//                    0x21 = equip dialog A (full slot)
//   FUN_00480620  → Widget_Draw(element, tex, flag)
//   FUN_0053cc30  → Packet_Encode / CRC_Compute
//   FUN_00422df0  → HashTable_GetOrInsert (packet sequence tracking)
//   FUN_00404040  → HashTable_Decrement

#include "stdafx.h"

// @ 0x00483B30 — defined only via stack params, not standard calling convention
// See documentation above for full parameter and packet details.
// TODO: reconstruct proper C++ signature once caller is identified.
