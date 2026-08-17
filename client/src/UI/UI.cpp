// UI.cpp
// Sistema de UI — Inventario, Equipamiento, Stats, Items
//
// Cubre las siguientes funciones originales del binario:
//   ItemTable_SetSlot      @ 0x004cce00  (352 lines)
//   ItemTable_UpdateSlot   @ 0x004cc660  (475 lines)
//   Item_GetType           @ 0x0047b110
//   Entity_UpdateEquip     @ 0x0045c130  (310 lines)
//   CharData_RecalcStats   @ 0x0047e3c0  (47 lines)
//   Stats_CalcBase         @ 0x0047d410  (219 lines)
//   Mount_UpdateSlot       @ 0x00451b20
//
// ── ESTRUCTURAS DE DATOS ──────────────────────────────────────────────────────
//
// INVENTORY SLOT (stride 0x44 = 68 bytes, en DAT_07ea8410):
//   +0x00 (short)  item_id          — tipo de ítem (-1 = vacío)
//   +0x02          ?
//   +0x04 (int)    item_flags       — packed: durability, luck, option bits
//   +0x1a (byte)   item_level       — nivel del ítem (0..15)
//   +0x1b (byte)   item_option      — opción adicional (excelente, ancient)
//   +0x1e (byte)   grid_col         — columna en la grilla
//   +0x1f (byte)   grid_row         — fila en la grilla
//   +0x38 (short)  is_root_slot     — 1 = slot maestro del ítem, 0 = slot ocupado
//   +0x3a (short)  ?
//
// ITEM DEFINITION (stride 0x40 = 64 bytes, en DAT_07d78068):
//   +0x00           item_id
//   +0x20 (byte)   width            — ancho en celdas (grilla inventario)
//   +0x21 (byte)   height           — alto en celdas
//   +0x26 (byte)   max_anim
//   +0x29 (byte)   speed_factor     — para calc de ataque (FUN_0047d330)
//
// CHARDATA (g_CharData, DAT_07cf1ffc, ~0x584 bytes):
//   +0x0b (byte)   class_id & 7     — 0=DK, 1=DW, 2=Elf, 3=MG (bVar2 = byte & 7)
//   +0x14 (short)  stat_str         — STR
//   +0x16 (short)  stat_agi         — AGI
//   +0x1a (short)  stat_vit         — VIT
//   +0x3c (short)  attack_min       — ataque mínimo calculado
//   +0x3e (short)  attack_max_shield— ataque máximo / defensa escudo
//   +0x40 (short)  attack_min2
//   +0x42 (short)  attack_max2
//   +0x4c (short)  dodge_base       — esquiva base (de AGI)
//   +0x4e (short)  defense          — defensa física
//   +0x55e (short) crit_roll_min    — rango inferior para tirada crítica
//   +0x560 (short) crit_roll_max    — rango superior
//   +0x562 (short) attack_speed_bonus
//   +0x566 (short) dodge_raw        — AGI/skill derived
//   +0x568 (short) crit_raw
//   +0x56c (short) attack_speed_calc
//   +0x56e (short) attack_speed_final
//   +0x574 (short) attack_speed_net  — final - bonus
//   +0x576 (short) attack_speed_net2
//   +0x578 (short) attack_roll      — rand() en rango crit_roll_min..max - defense
//   +0x57a (short) crit_rate%       — crit_raw - dodge_raw, clamped 0..100
//   +0x57c (short) dodge_rate%      — dodge_base - defense_raw, clamped 0..100
//   +0x57e (byte)  did_crit         — 1 si tirada < crit_rate%
//   +0x57f (byte)  did_dodge        — 1 si tirada < dodge_rate%
//
// EQUIPMENT SLOTS en g_CharData (+0x86..+0x10f), y su mapeo al entity player:
//
//   g_CharData offset | Slot        | Entity player offset | Notas
//   ──────────────────┼─────────────┼──────────────────────┼───────────────────
//   +0x86 / +0x87    | weapon      | +0x270 / +0x272      | id+400 = entity type
//   +0x97 / +0x98    | shield      | +0x288 / +0x28a      |
//   +0xa8 / ?        | ring1       | +0x1f8               | fallback: raza+sprite
//   +0xb9 / ?        | ring2       | +0x210               |
//   +0xca / +0xcb    | earring1    | +0x228 / +0x22a      |
//   +0xdb / +0xdc    | earring2    | +0x240 / +0x242      |
//   +0xec / +0xed    | pendant     | +0x258 / +0x25a      |
//   +0xfd / +0xfe    | mount       | +0x2a0 / +0x2a2      |
//   +0x10e / +0x10f  | class_item  | +0x2b8 / +0x2ba      |
//   +0x233           |             | +0x273 (byte)         | weapon flag byte
//   +0x277           |             | +0x28b (byte)         | shield flag byte
//
//   Conversión: entity_type = item_id + 400
//   Si item_id == -1: entity_type = 0xffff (sin equipo)
//   Nivel del ítem: (chardata[offset+1] >> 3) & 0xF  → entity+1 (byte)
//
// INVENTORY TABLES:
//   DAT_07ea8410  — inventario principal (equipo + bags), stride 0x44
//   DAT_07ea5298  — inventario secundario (bodega / mini-bag)
//   DAT_07d78068  — definiciones de ítems, stride 0x40
//   DAT_07ea9800  — puntero al inventario activo (comparado con DAT_07ea8410)
//   DAT_07e91388  — count ítems equipados relevantes para buff
//   DAT_07ea5b18  — slot activo de buff (0=arma, 1=escudo, 9=acc)
//   DAT_07e91350  — buffer temp de slot buff (override temporal)
//
// ── ITEM_GETTYPE (0x0047b110) ─────────────────────────────────────────────────
//
//   int Item_GetType(byte* item_slot):
//     // item_slot apunta a un slot en DAT_07ea8410 (stride 0x44)
//     // byte0 = item_id_low, byte3 = flags (bit7 = type_high_bit)
//     int type = item_slot[0] + (item_slot[3] & 0x80) * 2;
//     if (type == 0xFF) return -1;   // slot vacío
//     return type;
//     // Retorna 0xFF (==-1 como signed byte) → slot vacío
//     // Retorna 0x00..0xFE → type de ítem (9-bit)
//
//   Tipos de ítem notables:
//     0x00..0x05  Swords / Axes / Spears / Bows / Staffs / Misc weapons
//     0x06        Shields
//     0x07        Helmets / Armors
//     0x09        Boots
//     0x0C        Rings
//     0x0D        Pendants
//     0x0E        Earrings
//     0x0F        Mounts (horse, fenrir)
//     (0x80..0xA0 + 0xA0..0xC0 = set items / ancient)
//
// ── ITEMTABLE_SETSLOT (0x004cce00) ────────────────────────────────────────────
//
//   void ItemTable_SetSlot(int slot_idx, void* table_ptr, void* item_data):
//     // Si slot_idx == 0xFF: no-op
//     // Si table_ptr == DAT_07ea5298: llamar FUN_004cc530(slot_idx, 0x7ea5298)
//     //   → limpia bodega/mini-bag
//     //
//     // Si table_ptr == DAT_07ea8410 (inventario principal):
//     //   Si slot_idx < 0xC (= primeros 12 slots = equipo):
//     //     → hace lookup en HashTable de g_CharData, decrementa refcount
//     //     → limpia el slot de equipo correspondiente en la tabla
//     //
//     // Función auxiliar: FUN_004cc530 = ClearInventorySlot(idx, table)
//     //   → pone item_id = -1, flags = 0, etc. en DAT_07ea8410[idx]
//
//   Slot indices para equipo (< 0x0C):
//     0x00  arma (weapon)
//     0x01  escudo (shield)
//     0x02  casco (helmet)
//     0x03  armadura (armor)
//     0x04  pantalón (pants)
//     0x05  botas (boots)
//     0x06  guantes (gloves)
//     0x07  ala/capa (wing/cape)
//     0x08  class_item (Dark Lord scepter / MG wings)
//     0x09  ring 1
//     0x0A  ring 2
//     0x0B  pendant
//
// ── ITEMTABLE_UPDATESLOT (0x004cc660) ─────────────────────────────────────────
//
//   void ItemTable_UpdateSlot(short* table, int grid_stride, ?, int* pos,
//                             byte* item_raw, ?):
//     // 1. Llama Item_GetType(item_raw) → si retorna 0xFF, return (slot vacío)
//     // 2. Si table != DAT_07ea8410 (es bodega):
//     //    Lee item_def = DAT_07d78068 + type * 0x40
//     //    Itera width×height celdas del ítem
//     //    Para cada celda:
//     //      slot_ptr = table + (row * grid_stride + col) * 0x22
//     //      slot_ptr[0x00] = type (short)
//     //      slot_ptr[0x0d] = item_raw[2]   (level/flags byte)
//     //      slot_ptr[0x1b] = item_raw[3]   (option byte)
//     //      slot_ptr[0x1f] = col_fraction
//     //      slot_ptr[0x3f] = row_fraction
//     //      Si (row==0 && col==0): slot_ptr[0x1c]=1, slot_ptr[0x1d]=0  ← root slot
//     //      Else: slot_ptr[0x1c]=0, slot_ptr[0x1d]=0
//     //      Llama FUN_0047b910(slot_ptr, item_raw[1], ...) → valida/encripta
//
//   Stride de la tabla: 0x22 por celda (inventario bodega)
//   Stride inventario principal: 0x44 (= 2 × 0x22)
//
//   FUN_0047b910 (Item_ValidateSlot, @ aprox 0x0047b910):
//     Valida flags/encriptación del slot; llamada para cada celda ocupada.
//
// ── ENTITY_UPDATEEQUIP (0x0045c130) ───────────────────────────────────────────
//
//   void Entity_UpdateEquip(int entity_ptr):
//     // Guarda: solo para entity con type==0x186 (=390 = player entity)
//     if (entity[+0x02] != 0x186) return;
//
//     // Hashstable ref-count (anti-tamper, ignorar)
//
//     // Weapon → entity visual
//     if (g_CharData[+0x86] == -1): entity[+0x270] = 0xFFFF   // sin arma
//     else: entity[+0x270] = g_CharData[+0x86] + 400           // entity type
//
//     // Shield → entity visual
//     if (g_CharData[+0x97] == -1): entity[+0x288] = 0xFFFF
//     else: entity[+0x288] = g_CharData[+0x97] + 400
//
//     // Mount → entity visual
//     if (g_CharData[+0xfd] == -1): entity[+0x2a0] = 0xFFFF
//     else: entity[+0x2a0] = g_CharData[+0xfd] + 400
//
//     // Class item → entity visual
//     if (g_CharData[+0x10e] == -1): entity[+0x2b8] = 0xFFFF
//     else: entity[+0x2b8] = g_CharData[+0x10e] + 400
//
//     // Niveles de ítems (byte): (chardata >> 3) & 0xF
//     entity[+0x272] = (g_CharData[+0x87] >> 3) & 0xF   // weapon level
//     entity[+0x28a] = (g_CharData[+0x98] >> 3) & 0xF   // shield level
//     entity[+0x2a2] = (g_CharData[+0xfe] >> 3) & 0xF   // mount level
//     entity[+0x2ba] = (g_CharData[+0x10f] >> 3) & 0xF  // class_item level
//
//     // Copiar flag bytes adicionales
//     entity[+0x273] = g_CharData[+0x233]   // weapon extra flag
//     entity[+0x28b] = g_CharData[+0x277]   // shield extra flag
//
//     // Si class_item_type en rango 0x85..0x8C: skip walk anim
//     // Si class_item fuera de rango 0x22..0x5B y no en 0x85..0x8C:
//     //   Entity_SetWalkAnimation(entity)   (0x004430c0)
//
//     // Ring 1 (g_CharData[+0xa8]):
//     //   Si == -1: usar sprite basado en raza (entity[+0x1bc])
//     //     entity[+0x1f8] = (entity[+0x1bc] & 0xFF07) + 0x390 + (byte>>3)*4
//     //   Else: entity[+0x1f8] = g_CharData[+0xa8] + 400
//
//     // Ring 2 (g_CharData[+0xb9]):
//     //   Si == -1: entity[+0x210] = (entity[+0x1bc] & 7) + 0x397 + (byte>>3)*4
//     //   Else: entity[+0x210] = g_CharData[+0xb9] + 400
//
//     // Earrings y pendant: mismo patrón (+0x228/+0x240/+0x258)
//
// ── CHARDATA_RECALCSTATS (0x0047e3c0) ─────────────────────────────────────────
//
//   int CharData_RecalcStats(int chardata_ptr):
//     Stats_CalcBase(param_1);       // 0x0047d410 — ataque base (ver abajo)
//     FUN_0047dae0(param_1);         // Stats_CalcDefense
//     FUN_0047dd50(param_1);         // Stats_CalcHP_Max
//     FUN_0047dd80(param_1);         // Stats_CalcMP_Max
//     FUN_0047dfe0(param_1);         // Stats_CalcAttackSpeed
//     FUN_0047e160(param_1);         // Stats_CalcCritBase
//     FUN_0047e2e0(param_1);         // Stats_CalcDodgeBase
//     FUN_0047e310(param_1);         // Stats_CalcFinal
//
//     // Attack speed neto:
//     chardata[+0x574] = chardata[+0x56c] - chardata[+0x562]
//     chardata[+0x576] = chardata[+0x56e] - chardata[+0x562]
//
//     // Tirada de ataque (rand en rango):
//     chardata[+0x578] = rand() % (crit_roll_max - crit_roll_min + 1)
//                        + crit_roll_min - chardata[+0x4e]
//
//     // Crit rate = crit_raw - dodge_raw, clamped [0, 100]
//     chardata[+0x57a] = clamp(chardata[+0x3a] - chardata[+0x568], 0, 100)
//
//     // Dodge rate = dodge_base - defense, clamped [0, 100]
//     chardata[+0x57c] = clamp(chardata[+0x4c] - chardata[+0x566], 0, 100)
//
//     // Tirada crítica:
//     if (rand() % 100 < crit_rate%): did_crit = 1
//     else: did_crit = 0
//
//     // Tirada esquiva:
//     if (rand() % 100 < dodge_rate%): did_dodge = 1
//     else: did_dodge = 0
//
// ── STATS_CALCBASE (0x0047d410) ───────────────────────────────────────────────
//
//   void Stats_CalcBase(int chardata_ptr):
//     // Lee slots de arma (chardata[+0x218]) y escudo (chardata[+0x25c])
//     // Si DAT_07e91388 > 0 && inventario activo == principal:
//     //   override temporal de slot según DAT_07ea5b18 (para preview buff)
//     //
//     // Comprueba si arma es bow/staff (id 0x88..0x8E o 0x80..0x86):
//     //   Si sí → fórmula con AGI+STR o solo STR
//     //
//     // Selección por clase (chardata[+0x0b] & 7):
//     //   2 (DW/wizard):  attack = (STR+AGI)/7; attack_max = (STR+AGI)/4
//     //   1 (DK):         attack = STR/6;       attack_max = STR/4
//     //   3 (MG):         attack = STR/3 + VIT/12; attack_max = STR/4 + VIT/8
//     //   0/default:      attack = STR/8;       attack_max = STR/4
//     //
//     // Luego: lee item_def del arma (DAT_07d78068 + weapon_id * 0x40)
//     //   FUN_0047d330(item_level, speed_factor, item_flags, option) → bonus_ataque
//     //   Agrega bonus_ataque y bonus_escudo a chardata[+0x3c..+0x42]
//     //
//     // Si arma en rango 0x80..0x9F (double-hand 2H):
//     //   bonus /= 2 para el segundo par de valores
//     // Si arma = 0x87 (específico) con option bits != 0: duplica bonus
//
//   FUN_0047d330 (Item_CalcAttackBonus, @ 0x0047d330):
//     Calcula bonus de ataque de un ítem dado level/flags/option.
//     Usa speed_factor del item_def[+0x29].
//
//   FUN_0047cef0 (Stats_ApplyBonus, @ 0x0047cef0):
//     Aplica bonus de stat con opcode 0x3C (add) o 0x49 (set).
//
//   FUN_0047cf40 (Stats_ApplyBonus2, @ 0x0047cf40):
//     Aplica bonus con opcode 0x4A y divisor 2.
//
// ── MOUNT_UPDATESLOT (0x00451b20) ─────────────────────────────────────────────
//
//   void Mount_UpdateSlot(byte* item_raw):
//     // Valida que el ítem es un mount válido (type 0x0D..0x0F o similar)
//     // Actualiza globals de estado de mount:
//     //   DAT_07abf5dc  — mount entity ptr (caballo/fenrir)
//     //   DAT_07abf5e0  — mount state flag
//     // Si ítem inválido: limpia los globals de mount
//
// ── FUNCIONES AUXILIARES ──────────────────────────────────────────────────────
//
//   FUN_004cc530 (ClearInventorySlot, @ 0x004cc530):
//     void ClearInventorySlot(int slot_idx, int table_base):
//       Pone -1 en item_id, 0 en flags/level/option del slot.
//
//   FUN_0047b910 (Item_ValidateSlot, @ 0x0047b910):
//     void Item_ValidateSlot(short* slot_ptr, byte level, ...):
//       Valida integridad del slot; posible anti-cheat check.
//
// ── GLOBALS ───────────────────────────────────────────────────────────────────
//
//   DAT_07cf1ffc   — g_CharData (puntero a datos del personaje local, ~0x584 bytes)
//   DAT_07cf1ff4   — puntero alternativo a datos del personaje (usado en Stats_CalcBase)
//   DAT_07ea8410   — inventario principal (tabla, stride 0x44)
//   DAT_07ea5298   — inventario secundario (bodega, stride 0x22)
//   DAT_07d78068   — definiciones de ítems (stride 0x40)
//   DAT_07ea9800   — puntero al inventario activo
//   DAT_07e91388   — count de ítems equipados con buff activo
//   DAT_07ea5b18   — slot de buff activo (0=weapon, 1=shield, 9=acc)
//   DAT_07e91350   — slot override temporal para buff preview
//   DAT_07abf5d8   — player entity ptr (local player)
//   DAT_07abf5dc   — mount entity ptr
//   DAT_07abf5e0   — mount state flag
//   DAT_055c9bc8   — HashTable base (anti-tamper, no game logic)
//
// ── CROSS-REFERENCE ───────────────────────────────────────────────────────────
//
//   CharData_RecalcStats  @ 0x0047e3c0  — llamado después de equipar/desequipar
//   Entity_UpdateEquip    @ 0x0045c130  — aplica equipo de g_CharData al entity visual
//   ItemTable_UpdateSlot  @ 0x004cc660  — escribe ítem en tabla de inventario
//   ItemTable_SetSlot     @ 0x004cce00  — limpia slot de inventario/equipo
//   Item_GetType          @ 0x0047b110  — decodifica tipo de ítem (9-bit)
//   Stats_CalcBase        @ 0x0047d410  — calcula ataque base por clase
//   Mount_UpdateSlot      @ 0x00451b20  — actualiza estado de montura
//   Entity_SetWalkAnimation @ 0x004430c0 — ajusta walk anim tras cambio de equipo
//   Item_CalcAttackBonus  @ 0x0047d330  — bonus de ataque de un ítem
//   Stats_ApplyBonus      @ 0x0047cef0  — aplica bonus de stat (opcodes 0x3C/0x49)
//   Stats_ApplyBonus2     @ 0x0047cf40  — aplica bonus con divisor (opcode 0x4A)
//   Stats_CalcDefense     @ 0x0047dae0
//   Stats_CalcHP_Max      @ 0x0047dd50
//   Stats_CalcMP_Max      @ 0x0047dd80
//   Stats_CalcAttackSpeed @ 0x0047dfe0
//   Stats_CalcCritBase    @ 0x0047e160
//   Stats_CalcDodgeBase   @ 0x0047e2e0
//   Stats_CalcFinal       @ 0x0047e310
//   ClearInventorySlot    @ 0x004cc530
//   Item_ValidateSlot     @ 0x0047b910

#include "stdafx.h"
#include "UI/UI.h"
