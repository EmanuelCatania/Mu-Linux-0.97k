// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD DAT_07eaa128;   // Golden Archer panel flag (globals.cpp)
extern void __cdecl FUN_0054158c(void* ptr);
extern void FUN_004fa5a0(void);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif
#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif
#define ITEM_SPECIAL_SKILL_OPTION             0
#define ITEM_SPECIAL_LUCK_OPTION              1
#define ITEM_OPTION_ADD_PHYSI_DAMAGE_CODE     60
#define ITEM_OPTION_ADD_MAGIC_DAMAGE_CODE     61
#define ITEM_OPTION_ADD_DEFENSE_RATE_CODE     62
#define ITEM_OPTION_ADD_DEFENSE_CODE          63
#define ITEM_OPTION_ADD_EXCELLENT_DAMAGE_CODE 72
// CloseInventoryRelatedWindows @ 0x004CBA60 (218 lines) — Close all trade/shop/inventory windows
// Sets all shop/warehouse/trade/chaos/event flags to 0, clears item slots.
extern "C" BYTE Inventory[];
extern "C" BYTE OffsetTradeItems[];
extern "C" BYTE OffsetMixItems[];
void __cdecl CloseInventoryRelatedWindows(void) {
    // PORT FIEL de IDA 0x004CBA60 (2026-07-25). BUG previo: limpiaba
    // DAT_07e11e98 (global EQUIVOCADO) "comentado como ShopOpened", pero
    // ShopOpened real es DAT_07eaa118 → la tienda quedaba abierta al cerrarla.
    // Anti-tamper hash-table (que envuelve el set de ShopOpened/TradeOpened en
    // IDA) omitido per policy — el efecto neto son estos clears.
    ShopOpened                 = 0;   // DAT_07eaa118  ← EL fix del cierre
    DAT_07eaa132               = 0;   // byte_7EAA132
    DAT_07e11d14               = 0;   // RepairEnable
    WarehouseOpened            = 0;   // DAT_07eaa119
    DAT_00559f5f               = 0;   // byte_559F5F
    DAT_07eaa14c               = 0;   // dword_7EAA14C
    ChaosMixOpened             = 0;   // DAT_07eaa11a
    TradeOpened                = 0;
    EventWindowOpened          = 0;   // DAT_07eaa11c
    _g_bEventChipDialogEnable  = 0;   // DAT_07e5ba80
    DAT_07e11e1c               = 0;   // g_shEventChipCount
    g_bServerDivisionEnable    = 0;
    g_bServerDivisionAccept    = 0;
    // 2026-07-27 FIX (Golden Archer bloqueaba la UI): su panel se gatea con
    // DAT_07eaa128 (!=0 && !=3). Tiene un close propio con hit-test de su X,
    // pero si ese rect no pega el panel quedaba abierto para siempre y no se
    // podía cerrar de ninguna forma. Lo sumamos al cierre genérico, que ya usan
    // Escape / I / V / C / G / P / click-al-mundo.
    DAT_07eaa128               = 0;   // Golden Archer panel

    // Limpiar los pools de items de shop/trade/mix (slots a 0xFFFF, key 0).
    for (int i = 0; i < 32; ++i) {
        BYTE* c = Inventory + i * 0x44;
        *(short*)c = (short)0xFFFF; *(DWORD*)(c + 4) = 0;
        BYTE* t = OffsetTradeItems + i * 0x44;
        *(short*)t = (short)0xFFFF; *(DWORD*)(t + 4) = 0;
        BYTE* m = OffsetMixItems + i * 0x44;
        *(short*)m = (short)0xFFFF; *(DWORD*)(m + 4) = 0;
    }

    PlayBuffer(25, 0, 0);   // sonido de cierre
    PlayBuffer(28, 0, 0);
}

// FindTextA @ 0x004977F0 (32 lines) — MBCS-aware substring search
// Searches for needle in haystack. caseSensitive=true -> prefix match only.

// addr: 0x00482BE0  (sub_482BE0 / category -> inventory slot scan)
// ported from IDA raw/00482BE0_sub_482BE0.c
// 91-line item-type category lookup. Given a category (0,1,2 = right/left/crossbow
// equipped, 3 = force Type=454..452, 4 = force Type=451..448), derives the Type
// range [v2..v1] using DAT_00559c60/64/68 (current equipped weapon per hand) and
// scans the 8x8 inventory grid (DAT_07EA9328..DAT_07EA9504, 17 ints/row,
// 136 ints/col) for the first matching slot, returning a packed slot index.
// Returns -1 if Teleport flag is active and the range collapses to 458, or if no
// slot matches. NOTE: `Teleport` global is not declared in this translation unit
// — treated as 0 (never active); this keeps the fast path identical to IDA.
extern "C" BYTE OffsetInventoryItems[];
int __cdecl FUN_00482be0(int a1) {
    int v1;
    int v2;

    if (a1 < 3) {
        v1 = a1;
        v2 = a1;
    } else if (a1 == 3) {
        v1 = 454;
        v2 = 452;
    } else if (a1 == 4) {
        v1 = 451;
        v2 = 448;
    } else {
        v1 = a1;
        v2 = a1;
    }

    if (a1 == 0) {
        v1 = DAT_00559c60;
        if (DAT_00559c60 == 456 || DAT_00559c60 == 457 || DAT_00559c60 == 468) {
            v2 = DAT_00559c60;
        } else if (DAT_00559c60 >= 452 && DAT_00559c60 <= 454) {
            v1 = 454;
            v2 = 452;
        } else {
            v1 = 451;
            v2 = 448;
        }
    } else if (a1 == 1) {
        v1 = DAT_00559c64;
        if (DAT_00559c64 == 456 || DAT_00559c64 == 457 || DAT_00559c64 == 468) {
            v2 = DAT_00559c64;
        } else if (DAT_00559c64 < 448 || DAT_00559c64 > 451) {
            v1 = 454;
            v2 = 452;
        } else {
            v1 = 451;
            v2 = 448;
        }
    } else if (a1 == 2) {
        v1 = DAT_00559c68;
        if (DAT_00559c68 == 456 || DAT_00559c68 == 457 || DAT_00559c68 == 468) {
            v2 = DAT_00559c68;
        } else if (DAT_00559c68 >= 448 && DAT_00559c68 <= 451) {
            v1 = 451;
            v2 = 448;
        } else if (DAT_00559c68 >= 452 && DAT_00559c68 <= 454) {
            v1 = 454;
            v2 = 452;
        } else {
            v1 = 456;
            v2 = 456;
        }
    }

    if (v1 < v2) {
        return -1;
    }

    ITEM* inv = (ITEM*)OffsetInventoryItems;
    for (int wanted = v1; wanted >= v2; --wanted) {
        for (int slot = 0; slot < 64; ++slot) {
            ITEM* it = &inv[slot];
            if (it->Type == wanted && it->Durability > 0) {
                return slot;
            }
        }
    }
    return -1;
}

// ItemConvert @ 0x0047B910 — inventory/equipment item stat + option expansion.
// Ported directly from IDA structure/logic instead of the old minimal stub.
void __cdecl FUN_0047b910(int pItem, int Attribute1, int Attribute2) {
    ITEM* ip = (ITEM*)pItem;
    if (!ip) return;

    short wType = ip->Type;
    if (wType < 0) {
        ip->Part = (BYTE)-1;
        ip->SpecialNum = 0;
        memset(ip->Special, 0, sizeof(ip->Special));
        memset(ip->SpecialValue, 0, sizeof(ip->SpecialValue));
        return;
    }

    ITEM_ATTRIBUTE* table = (ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;
    if (!table) return;

    ITEM_ATTRIBUTE* p = &table[wType];
    int itemLevel = ((BYTE)Attribute1 >> 3) & 0xF;
    int itemExcel = ((BYTE)Attribute2) & 0x3F;
    int itemExt = (int)ip->byColorState;
    int excelAddValue = 0;
    bool bExtOption = ((itemExt % 4) == 1 || (itemExt % 4) == 2);

    // 97k: helper/potion inventory items must not inherit excellent state
    // from the ext-byte path. Letting them do so pushes RequireLevel +20 and
    // contaminates tooltip/render with equipment logic.
    if ((wType >= 416 && wType < 424) || wType >= 448) {
        bExtOption = false;
    }

    ip->Level = (BYTE)Attribute1;
    memset(ip->Special, 0, sizeof(ip->Special));
    memset(ip->SpecialValue, 0, sizeof(ip->SpecialValue));
    ip->SpecialNum = 0;
    ip->Color = 0;

    // AUDITORIA 2026-07-20 — itemExcel ahora es FIEL a IDA ItemConvert (0x47B910).
    // El original hace exactamente esto y nada mas:
    //     iItemExcel = Attribute2 & 63;
    //     if (Type 387..390 || 19 || 146 || 170) iItemExcel = 0;
    // Aca habia DOS lineas de mas que no existen ni en IDA ni en el DLL de
    // inyeccion (verificado en Source/Client/Main/Item.cpp, que reemplaza
    // ItemConvert entero y tampoco las tiene):
    //
    //   1) `if (bExtOption) itemExcel = 1;`  ← la peor: forzaba el flag excellent,
    //      y de ahi `levelAddValue += 25`, inflando RequireStrength/Dexterity/
    //      Energy y los bonus excellent de damage/defense de CUALQUIER item con
    //      el ext-byte puesto.
    //   2) `if (Type 416..423 || >= 448) itemExcel = 0;`  ← ceroeaba de mas.
    //
    // `bExtOption` se conserva: NO alimenta la matematica de stats, pero si el
    // color del item (ip->Color / byColorState), que lo consumen
    // Render_PlayerEquipment y HUD_Pass4.  Ese bloque es otro injerto de origen
    // distinto y se audita aparte.
    if ((wType >= 387 && wType <= 390) || wType == 19 || wType == 146 || wType == 170) {
        itemExcel = 0;
    }

    ip->TwoHand = p->TwoHand;
    ip->WeaponSpeed = p->AttackSpeed;
    ip->DamageMin = p->DamageMin;
    ip->DamageMax = p->DamageMax;
    ip->SuccessfulBlocking = p->DefenseRate;
    ip->Defense = p->Defense;
    ip->MagicDefense = p->MagicDefense;
    ip->WalkSpeed = p->WalkSpeed;

    switch (wType) {
    case 70:  excelAddValue = 15; break;
    case 134: excelAddValue = 30; break;
    case 167: excelAddValue = 25; break;
    default: break;
    }

    auto min9 = [](int v) -> int { return (v <= 9) ? v : 9; };
    auto post9_bonus = [](int lvl) -> int {
        int add = 0;
        for (int i = 0; i < lvl - 9; ++i) add += (i == 0) ? 4 : 5;
        return add;
    };
    auto push_special = [&](BYTE code, BYTE value = 0) {
        if (ip->SpecialNum < MAX_SPECIAL_OPTION) {
            ip->SpecialValue[ip->SpecialNum] = value;
            ip->Special[ip->SpecialNum] = code;
            ++ip->SpecialNum;
        }
    };

    if (p->DamageMin) {
        if (itemExcel > 0 && p->Level) {
            ip->DamageMin = (WORD)(ip->DamageMin + (excelAddValue ? excelAddValue : (25 * p->DamageMin / p->Level + 5)));
        }
        ip->DamageMin = (WORD)(ip->DamageMin + 3 * min9(itemLevel) + post9_bonus(itemLevel));
    }

    if (p->DamageMax) {
        if (itemExcel > 0 && p->Level) {
            ip->DamageMax = (WORD)(ip->DamageMax + (excelAddValue ? excelAddValue : (25 * p->DamageMin / p->Level + 5)));
        }
        ip->DamageMax = (WORD)(ip->DamageMax + 3 * min9(itemLevel) + post9_bonus(itemLevel));
    }

    if (p->DefenseRate) {
        if (itemExcel > 0 && p->Level) {
            ip->SuccessfulBlocking = (BYTE)(ip->SuccessfulBlocking + (25 * p->DefenseRate / p->Level + 5));
        }
        ip->SuccessfulBlocking = (BYTE)(ip->SuccessfulBlocking + 3 * min9(itemLevel) + post9_bonus(itemLevel));
    }

    if (p->Defense) {
        if (wType >= 192 && wType < 224) {
            ip->Defense = (WORD)(ip->Defense + itemLevel);
        } else {
            if (itemExcel > 0 && p->Level) {
                ip->Defense = (WORD)(ip->Defense + (12 * p->Defense / p->Level + p->Level / 5 + 4));
            }
            ip->Defense = (WORD)(ip->Defense + ((wType >= 387 && wType <= 390) ? 2 : 3) * min9(itemLevel) + post9_bonus(itemLevel));
        }
    }

    if (p->MagicDefense) {
        ip->MagicDefense = (WORD)(ip->MagicDefense + 3 * min9(itemLevel) + post9_bonus(itemLevel));
    }

    int levelAddValue = p->Level;
    if (itemExcel) levelAddValue += 25;

    if (p->RequireStrength) {
        unsigned int v = (unsigned int)(((unsigned long long)4123168605ULL * p->RequireStrength * (levelAddValue + 3 * itemLevel)) >> 32) >> 5;
        ip->RequireStrength = (WORD)(v + (v >> 31) + 20);
    } else {
        ip->RequireStrength = 0;
    }

    if (p->RequireAgility) {
        unsigned int v = (unsigned int)(((unsigned long long)4123168605ULL * p->RequireAgility * (levelAddValue + 3 * itemLevel)) >> 32) >> 5;
        ip->RequireDexterity = (WORD)(v + (v >> 31) + 20);
    } else {
        ip->RequireDexterity = 0;
    }

    if (wType == 395) {
        switch (itemLevel) {
        case 0: ip->RequireEnergy = 30; break;
        case 1: ip->RequireEnergy = 60; break;
        case 2: ip->RequireEnergy = 90; break;
        case 3: ip->RequireEnergy = 130; break;
        case 4: ip->RequireEnergy = 170; break;
        case 5: ip->RequireEnergy = 210; break;
        case 6: ip->RequireEnergy = 300; break;
        default: ip->RequireEnergy = (BYTE)Attribute2; break;
        }
    } else if (p->RequireEnergy) {
        ip->RequireEnergy = (WORD)(4 * p->RequireEnergy * (levelAddValue + 3 * itemLevel) / 100 + 20);
    } else {
        ip->RequireEnergy = 0;
    }

    int requireLevelAdd = 4;
    if (wType >= 387 && wType <= 390) requireLevelAdd = 5;
    if (p->RequireLevel) {
        ip->RequireLevel = (WORD)(p->RequireLevel + itemLevel * requireLevelAdd);
    } else {
        ip->RequireLevel = 0;
    }

    if (wType == 426) {
        ip->RequireLevel = (WORD)(50 + ((((BYTE)Attribute1 >> 3) & 0xF) > 2 ? 0 : -30));
    }
    if (itemExcel > 0 && ip->RequireLevel) {
        ip->RequireLevel = (WORD)(ip->RequireLevel + 20);
    }

    if (wType >= 387 && wType <= 390) {
        if (Attribute2 & 1)  push_special(80, (BYTE)(5 * (itemLevel + 10)));
        if (Attribute2 & 2)  push_special(81, (BYTE)(5 * (itemLevel + 10)));
        if (Attribute2 & 4)  push_special(82, 3);
        if (Attribute2 & 8)  push_special(83, 50);
        if (Attribute2 & 0x10) push_special(77, 5);
    }

    if (Attribute1 & 0x80) {
        if (p->RequireClass[1]) {
            if (ip->Type >= 196 && ip->Type < 224) push_special(18);
            if (wType == 4 || wType == 7 || wType == 8) push_special(21);
            if (wType == 3 || wType == 6 || wType == 9 || wType == 11 || (wType >= 97 && wType <= 100)) push_special(20);
            if (wType == 5 || wType == 10 || wType == 13 || wType == 14 || wType == 16 || wType == 96 || (wType >= 103 && wType <= 105)) push_special(22);
            if (wType == 12 || (wType >= 34 && wType < 64) || wType == 65 || wType == 67 || wType == 68) push_special(19);
            if (wType == 15 || wType == 69 || wType == 70 || wType == 17) push_special(23);
            if (wType == 19 || wType == 106) push_special(22);
        }
        if (p->RequireClass[2]) {
            if (wType >= 128 && wType < 160 && wType != 135 && wType != 143) push_special(24, 6);
        }
        if (p->RequireClass[3]) {
            if (wType == 18) push_special(23);
            else if (wType == 31) push_special(56);
        }
        if (wType == 419) push_special(49);
    }

    if (Attribute1 & 4) {
        if ((wType >= 0 && wType < 384 && wType != 135 && wType != 143) || (wType >= 384 && wType <= 390)) {
            push_special(64);
        }
    }

    int option3 = (Attribute1 & 3) + 4 * ((Attribute2 >> 6) & 1);
    if (option3) {
        if (wType == 419) {
            if (option3 & 1) push_special(84, 5);
            if (option3 & 2) push_special(83, 50);
            if (option3 & 4) push_special(77, 5);
        } else {
            if (wType >= 0 && wType < 160 && wType != 135 && wType != 143) {
                push_special(60, (BYTE)(4 * option3));
                ip->RequireStrength = (WORD)(ip->RequireStrength + 5 * option3);
            }
            if (wType >= 160 && wType < 192) {
                push_special(61, (BYTE)(4 * option3));
                ip->RequireStrength = (WORD)(ip->RequireStrength + 5 * option3);
            }
            if (wType >= 192 && wType < 224) {
                push_special(62, (BYTE)(5 * option3));
                ip->RequireStrength = (WORD)(ip->RequireStrength + 5 * option3);
            }
            if (wType >= 224 && wType < 384) {
                push_special(63, (BYTE)(4 * option3));
                ip->RequireStrength = (WORD)(ip->RequireStrength + 5 * option3);
            }
            if (wType >= 424 && wType < 448) {
                push_special(65, (BYTE)option3);
            }
            switch (wType) {
            case 384: push_special(65, (BYTE)option3); break;
            case 385: push_special(61, (BYTE)(4 * option3)); break;
            case 386: push_special(60, (BYTE)(4 * option3)); break;
            case 387:
                if (itemExcel & 0x20) push_special(65, (BYTE)option3);
                else push_special(60, (BYTE)(4 * option3));
                break;
            case 388:
                if (itemExcel & 0x20) push_special(61, (BYTE)(4 * option3));
                else push_special(65, (BYTE)option3);
                break;
            case 389:
                if (itemExcel & 0x20) push_special(60, (BYTE)(4 * option3));
                else push_special(65, (BYTE)option3);
                break;
            case 390:
                if (itemExcel & 0x20) push_special(60, (BYTE)(4 * option3));
                else push_special(61, (BYTE)(4 * option3));
                break;
            default: break;
            }
        }
    }

    if ((wType >= 192 && wType < 384) || (wType >= 424 && wType <= 425)) {
        if (Attribute2 & 0x20) push_special(66);
        if (Attribute2 & 0x10) push_special(67);
        if (Attribute2 & 8)    push_special(68);
        if (Attribute2 & 4)    push_special(69);
        if (Attribute2 & 2)    push_special(70);
        if (Attribute2 & 1)    push_special(71);
    }

    if ((wType >= 0 && wType < 192) || (wType >= 428 && wType <= 429)) {
        if (Attribute2 & 0x20) push_special(72);
        if ((wType >= 160 && wType < 192) || wType == 428) {
            if (Attribute2 & 0x10) push_special(75, (BYTE)(*(unsigned short*)((BYTE*)CharacterAttribute + 14) / 20));
            if (Attribute2 & 8)    push_special(76);
        } else {
            if (Attribute2 & 0x10) push_special(73, (BYTE)(*(unsigned short*)((BYTE*)CharacterAttribute + 14) / 20));
            if (Attribute2 & 8)    push_special(74);
        }
        if (Attribute2 & 4) push_special(77, 7);
        if (Attribute2 & 2) push_special(78);
        if (Attribute2 & 1) push_special(79);
    }

    if ((wType >= 128 && wType < 136) || wType == 145) {
        ip->Part = 1;
        ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0);
        return;
    }
    if (wType >= 0) {
        if (wType < 192) { ip->Part = 0; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 224) { ip->Part = 1; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 256) { ip->Part = 2; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 288) { ip->Part = 3; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 320) { ip->Part = 4; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 352) { ip->Part = 5; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 384) { ip->Part = 6; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 391) { ip->Part = 7; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
    }
    if (wType >= 416 && wType < 424) { ip->Part = 8; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
    if (wType >= 424 && wType < 428) { ip->Part = 10; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
    if (wType >= 428 && wType < 448) { ip->Part = 9; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
    ip->Part = (BYTE)-1;
    ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0);
}

// 2026-05-08: ItemValue delegates to Item_CalculateValue. Previously
// returned 0 unconditionally → all sell-price calculations in Item_Click
// Handler / RenderItemInfo / shop UI yielded zero gold. Delegate to the real
// impl in stubs_helpers.cpp.
int __cdecl ItemValue(ITEM* ip, unsigned int goldType) {
    return Item_CalculateValue((void*)ip, (int)goldType);
}
