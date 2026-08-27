// Item_Durability.cpp
//
// Extracted from stubs_helpers.cpp; original IDA comments and DAT_* provenance retained.

// stubs_helpers.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 12638-13754 (1117 lines).
//
// Originally tagged "New helpers needed by SecondPassword implementations" but
// content is mixed: item/inventory helpers (GetItemCount/GetItemSlot/
// CalcMaxDurability/ConvertItemType/ItemValue/ConvertGold), render helpers
// (CreateOkMessageBox/BMD::Animation/RenderObjectScreen), math helpers
// (VectorMA/VectorNormalize/RandomXY), effect helpers (SpawnEffectAtBone/
// JointBetweenBones), Pipe helpers (Pipe_Send/Recv/SetTarget), CSQuest helpers.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" DWORD g_ItemAttribute_Backup;
extern void __cdecl FUN_0054158c(void* ptr);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

// IDA Hex-Rays intrinsic shims.
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


// IDA: FUN_004C45C0
// Port FIEL del IDA decompile (raw 0x4C45C0):
//   1. base = p->Durability  (BYTE @ +41 dentro de ITEM_ATTRIBUTE)
//   2. Si type 160..191, base = p->MagicDurability (BYTE @ +42)
//   3. Level bonus: +1 si Level<4, +2 si <9, +3 si =9, +4 si >=10 (per-level loop)
//   4. Si Option1 & 0x3F != 0 (excellent options), excluyendo types 19/146/170/387-390:
//      result += 15
//
// BUG-FIX 2026-05-01: stub anterior leía *(uint*)(attrBase+0x8) que cae en
// Name[8..11] del struct ITEM_ATTRIBUTE. Para "Light Saber", Name[8..11] = "ber"
// = 0x00726562 = 25954 — exact valor visto en RenderBrokenItem ("0/25954").
// Corregido a leer p->Durability (offset +41 = +0x29).
unsigned int __cdecl Item_CalculateMaxDurability(void* item, int attrBase, int Level)
{
    // 2026-05-08: bug-fix — antes solo chequeaba `attrBase == 0`, pero callers
    // pasan `Type * 0x40 + DAT_07d78068` y si DAT_07d78068 == 0 entonces
    // attrBase = Type*0x40 (un valor pequeño tipo 0x2A00 para Type=168).
    // Eso pasa el `!= 0` check pero defereferenciar p->MagicDurability (offset
    // 42) crashea con AV en addr 0x2A2A. Validamos que attrBase sea un puntero
    // razonable de heap (>= 0x100000) y que `item` también sea válido.
    if ((uintptr_t)attrBase < 0x100000 || (uintptr_t)attrBase >= 0x80000000)
        return 255;
    if (item == nullptr || (uintptr_t)item < 0x100000) return 255;
    ITEM* ip          = (ITEM*)item;
    ITEM_ATTRIBUTE* p = (ITEM_ATTRIBUTE*)(uintptr_t)attrBase;

    // Base durability per type (Magic items 160..191 use MagicDurability)
    unsigned int result = (unsigned int)(unsigned char)p->Durability;
    short Type = ip->Type;
    if (Type >= 160 && Type < 192) {
        result = (unsigned int)(unsigned char)p->MagicDurability;
    }

    // Level bonus per upgrade tier
    for (int i = 0; i < Level; ++i) {
        if (i < 4)        result += 1;
        else if (i < 9)   result += 2;
        else if (i < 10)  result += 3;
        else              result += 4;
    }

    // Excellent option bonus (+15) — exclude wings/special types.
    BYTE Option1 = (BYTE)ip->Option1;
    if ((Option1 & 0x3F) != 0 &&
        (Type < 387 || Type > 390) &&
        Type != 19 && Type != 146 && Type != 170)
    {
        result += 15;
    }

    return result & 0xFFFF;
}

// IDA: FUN_0047C690
//
// IDA-ported 2026-04-26 (audit #3): el stub anterior devolvía "número de dígitos"
// en vez del valor real. Calcula precio gold del item considerando type/level/
// durabilidad/options. Si sellMode!=0 aplica reducción ×1/3 y penalty por durab.
//
// Caso especial #135 = Wings stage 1 / #143 = Wings stage 2 (jewel pricing).
// Cases 461/462/464/470/430/431/419/432-434/465-467/469/457/468 = jewels y sets.
// Constantes derivadas del binario original (`&unk_xxxxxx` en IDA = direcciones
// usadas como valores enteros — comprobado: 0x895440 = 9_000_000, etc.).
int __cdecl Item_CalculateValue(void* item_v, int a2)
{
    // 2026-05-08: defensive — same problem as CalcMaxDurability/RenderItemInfo:
    // si DAT_07d78068 está en 0 (table base no inicializada), el cómputo
    // `(int)DAT_07d78068 + type * 0x40` da un valor pequeño y crashea al
    // dereferenciar p->Money / p->Level. Bail con 0 en ese caso.
    if (!item_v || (uintptr_t)item_v < 0x100000) return 0;
    if ((uintptr_t)DAT_07d78068 < 0x100000 || (uintptr_t)DAT_07d78068 >= 0x80000000)
        return 0;

    int a1 = (int)item_v;
    int v5 = 0;
    int v34 = 0;

    short v3 = *(short*)a1;
    if ((unsigned short)v3 == 0xFFFF) return 0;
    // Bound type to ItemAttribute table size (1024).
    if (v3 < 0 || (unsigned short)v3 >= 1024) return 0;

    ITEM_ATTRIBUTE* p = (ITEM_ATTRIBUTE*)((int)DAT_07d78068 + (unsigned short)v3 * 0x40);
    int Money = p->Money;
    if (Money) {
        if (a2) Money /= 3;
        if (Money >= 1000) return 100 * (Money / 100);
        if (Money >= 100)  return 10  * (Money / 10);
        return Money;
    }

    int Level = (*(int*)(a1 + 4) >> 3) & 0xF;
    char v7 = 0;
    int v8 = (int)v3 / 32;
    int v26 = *(unsigned char*)(a1 + 36);

    if (v26) {
        for (int v9 = 0; v9 < v26; v9++) {
            unsigned char v10 = *(unsigned char*)(a1 + v9 + 37);
            if (v10 >= 0x42 && v10 <= 0x4F) v7 = 1;
        }
    }

    int v11 = 3 * Level + p->Level;
    if (v7) v11 += 25;

    short v12 = *(short*)a1;
    int v13 = 0;
    BYTE Durability = 0;

    if (v12 == 135) {
        if (Level == 0)      { v13 = 100;  Durability = p->Durability; }
        else if (Level == 1) { v13 = 1400; Durability = p->Durability; }
        else if (Level == 2) { v13 = 2200; Durability = p->Durability; }
        else                 { v13 = a2;   Durability = p->Durability; }
        goto LABEL_36;
    }

    if (v12 != 143) {
        bool handled = true;
        switch (v12) {
            case 461: v5 = 9000000;  goto LABEL_147;   // 0x895440
            case 462: v5 = 6000000;  goto LABEL_147;   // 0x5B8D80
            case 399: v5 = 810000;   goto LABEL_147;
            case 464: v5 = 45000000; goto LABEL_147;   // 0x2AEA540
            case 470: v5 = 36000000; goto LABEL_147;   // 0x2255100
            case 430: v5 = 180000;   goto LABEL_147;
            case 419: {
                v5 = 960000;
                v34 = 960000;
                if (v26) {
                    for (int v15 = 0; v15 < v26; v15++) {
                        unsigned char v16 = *(unsigned char*)(a1 + v15 + 37);
                        if (v16 == 77 || (v16 > 0x52 && v16 <= 0x54)) v5 += 300000;
                    }
                    goto LABEL_147;
                }
                goto LABEL_148;
            }
            case 431: v5 = 33000000; goto LABEL_147;   // 0x1F78A40
            case 432: case 433: {
                int v17;
                switch (Level) {
                    case 1: v17 = 5000;  break;
                    case 2: v17 = 7000;  break;
                    case 3: v17 = 10000; break;
                    case 4: v17 = 13000; break;
                    case 5: v17 = 16000; break;
                    case 6: v17 = 20000; break;
                    default: v17 = 0;    break;
                }
                v5 = 3 * v17;
                goto LABEL_147;
            }
            case 434: {
                int v17 = 4000 * (5 * Level + 45);
                if (Level == 1) v17 = 50000;
                v5 = 3 * v17;
                goto LABEL_147;
            }
            case 469:
                if (Level == 1) { v5 = 9000; goto LABEL_147; }
                goto LABEL_148;
            case 465: case 466: {
                int tab[5] = { 30000, 15000, 30000, 21000, 45000 };
                int idx = Level < 0 ? 0 : (Level >= 4 ? 4 : Level);
                v5 = tab[idx];
                goto LABEL_147;
            }
            case 467: {
                int tab[5] = { 120000, 60000, 120000, 84000, 180000 };
                int idx = Level < 0 ? 0 : (Level >= 4 ? 4 : Level);
                v5 = tab[idx];
                goto LABEL_147;
            }
            default: handled = false; break;
        }

        if (!handled) {
            if (v12 == 457 && Level == 1) { v5 = 1000; goto LABEL_147; }
            if (v12 == 468)               { v5 = 900;  goto LABEL_147; }

            if (p->Value) {
                v5 = 10 * (int)p->Value * (int)p->Value / 12;
                v34 = v5;
                if (v3 < 448 || v3 > 456) goto LABEL_148;
                v5 *= *(unsigned char*)(a1 + 26);   // durability multiplier (Excellent ancient)
                goto LABEL_147;
            }
        }

        if (v8 == 12) {
            if (v3 <= 390) goto LABEL_94;
        } else if (v8 != 13 && v8 != 15) {
            goto LABEL_94;
        }

        // Otherwise fall through to shield/armor pricing — but Ghidra's IDA
        // has nothing else here. Default to LABEL_148 (final scale).
        goto LABEL_148;

LABEL_94:
        switch (Level) {
            case 5:  v11 += 4;   break;
            case 6:  v11 += 10;  break;
            case 7:  v11 += 25;  break;
            case 8:  v11 += 45;  break;
            case 9:  v11 += 65;  break;
            case 10: v11 += 95;  break;
            case 11: v11 += 135; break;
            default: break;
        }

        if (v8 == 12 && v3 <= 390) {
            // Staff/Magic-class: (constant 40_000_000 base) + 11 * v11^2 * (v11+40)
            v5 = 40000000 + 11 * v11 * v11 * (v11 + 40);
        } else {
            v5 = v11 * v11 * (v11 + 40) / 8 + 100;
            v34 = v5;
            if (v8 < 0 || v8 > 6 || p->TwoHand) {
                goto LABEL_110;
            }
            v5 = 80 * v5 / 100;
        }
        v34 = v5;

LABEL_110:
        if (v26) {
            int v21 = 0;
            do {
                unsigned char opt = *(unsigned char*)(a1 + v21 + 37);
                double v22 = 0.0;
                bool didMul = false;
                bool didAdd = false;

                switch (opt) {
                case 0x12: case 0x13: case 0x14: case 0x15:
                case 0x16: case 0x17: case 0x18: case 0x38:
                    v22 = (double)v34 * 1.5;
                    didAdd = true;
                    break;

                case 0x3C: case 0x3D: case 0x3F: case 0x41:
                    if (v8 == 12 && v3 <= 390) {
                        int v23 = *(unsigned char*)(v21 + a1 + 45);
                        if (opt == 0x41) v23 *= 4;
                        switch (v23) {
                            case 4:  v22 = (double)v34 * 0.30000001; didAdd = true; break;
                            case 8:  v22 = (double)v34 * 0.60000002; didAdd = true; break;
                            case 12: v22 = (double)v34;              didAdd = true; break;
                            case 16: v22 = (double)v34 + (double)v34;didAdd = true; break;
                            default: break;
                        }
                    } else {
                        switch (*(unsigned char*)(v21 + a1 + 45)) {
                            case 4:  v22 = (double)v34 * 0.60000002; didAdd = true; break;
                            case 8:  v22 = (double)v34 * 1.4;        didAdd = true; break;
                            case 12: v22 = (double)v34 * 2.8;        didAdd = true; break;
                            case 16: v22 = (double)v34 * 5.5999999;  didAdd = true; break;
                            default: break;
                        }
                    }
                    break;

                case 0x3E:
                    switch (*(unsigned char*)(v21 + a1 + 45)) {
                        case 5:  v22 = (double)v34 * 0.60000002; didAdd = true; break;
                        case 10: v22 = (double)v34 * 1.4;        didAdd = true; break;
                        case 15: v22 = (double)v34 * 2.8;        didAdd = true; break;
                        case 20: v22 = (double)v34 * 5.5999999;  didAdd = true; break;
                        default: break;
                    }
                    break;

                case 0x40: case 0x50: case 0x51: case 0x52:
                case 0x53: case 0x54:
                    v22 = (double)v34 * 0.25;
                    didAdd = true;
                    break;

                case 0x42: case 0x43: case 0x44: case 0x45:
                case 0x46: case 0x47: case 0x48: case 0x49:
                case 0x4A: case 0x4B: case 0x4C: case 0x4D:
                case 0x4E: case 0x4F: case 0x5A:
                    v5 *= 2;
                    v34 = v5;
                    didMul = true;
                    break;
                default: break;
                }
                if (didAdd) {
                    v5 += (int)(__int64)v22;
                    v34 = v5;
                }
                (void)didMul;
                v21++;
            } while (v21 < v26);
        }
        goto LABEL_148;
    }

    // v12 == 143 path
    if (Level == 0)        v13 = 70;
    else if (Level == 1)   v13 = 1200;
    else if (Level == 2)   v13 = 2000;
    else                   v13 = a2;
    Durability = p->Durability;

LABEL_36:
    if (Durability) {
        v5 = v13 * (int)*(unsigned char*)(a1 + 26) / Durability;
LABEL_147:
        v34 = v5;
    }

LABEL_148:
    if (a2) {
        v5 /= 3;
        v34 = v5;
    }

    short v25 = *(short*)a1;
    if ((v3 < 416 || v25 > 419)
        && v25 != 426
        && v25 != 135
        && v25 != 143
        && v25 < 448
        && (v25 < 391 || v25 > 403)
        && (v25 < 430 || v25 > 435)
        && a2 == 1)
    {
        unsigned int maxDur = Item_CalculateMaxDurability((void*)a1, (int)((unsigned short)v25) * 0x40 + (int)DAT_07d78068, Level) & 0xffff;
        double penalty = (1.0 - (double)*(unsigned char*)(a1 + 26) / (double)maxDur)
                       * (double)v34 * -0.60000002;
        v5 += (int)(__int64)penalty;
    }
    if (v5 >= 1000) return 100 * (v5 / 100);
    if (v5 >= 100)  return 10  * (v5 / 10);
    return v5;
}

// IDA: FUN_004C3EF0
//
// IDA-ported 2026-04-26 (audit #3): el stub anterior solo escribía "%u / %u"
// pero el real calcula gold de reparación: sqrt(sqrt(Gold)) * sqrt(Gold) * 3 *
// (1 - dur/maxDur) + 1, con bonus 1.4× si rota, +5% si RepairEnable, redondeo
// a múltiplos de 100/10, y formato "1,234,567" en Text. Devuelve gold final.
unsigned int __cdecl Item_CalculateRepairCost(int Gold, int Durability, int MaxDurability, short Type, char* Text)
{
    (void)Type;
    if (!Text) {
        Text = (char*)"";
    }
    double dGold = (double)Gold;
    double v6 = 1.0 - (double)Durability / (double)MaxDurability;
    float Golda_f = (float)v6;

    double v8;
    if (v6 <= 0.0) {
        v8 = 0.0;
    } else {
        double v7 = sqrt(dGold);
        v8 = sqrt(v7) * v7 * 3.0 * (double)Golda_f + 1.0;
        if (Durability <= 0) v8 *= 1.4;
    }

    // RepairEnable byte (treated as 0/1 multiplier). El símbolo no está
    // mapeado en mu97k-src todavía → asumimos 0 (no extra tax).
    double withTax = 0.0 * v8 * 0.050000001 + v8;
    int v9 = (int)(__int64)withTax;
    int v10 = v9;
    if (v9 < 1000) {
        if (v9 >= 100) v10 = 10 * (v9 / 10);
    } else {
        v10 = 100 * (v9 / 100);
    }

    if (v10 < 1000000000) {
        if (v10 < 1000000) {
            int v12 = v10 % 1000;
            if (v10 < 1000) {
                crt_sprintf(Text, "%d", v12);
            } else {
                crt_sprintf(Text, "%d,%03d", v10 % 1000000 / 1000, v12);
            }
        } else {
            crt_sprintf(Text, "%d,%03d,%03d",
                        v10 % 1000000000 / 1000000,
                        v10 % 1000000 / 1000,
                        v10 % 1000);
        }
    } else {
        crt_sprintf(Text, "%d,%03d,%03d,%03d",
                    v10 / 1000000000,
                    v10 % 1000000000 / 1000000,
                    v10 % 1000000 / 1000,
                    v10 % 1000);
    }
    return (unsigned int)v10;
}

// IDA: FUN_004C4080
// Scans equipped items (+0x218, stride 0x44, 12 slots) and extra items (DAT_07ea8410, 8 slots).
// For each occupied slot with curDur < maxDur, calls CalcMaxDurability + AppendDurabilityLine.
// Accumulates repair-count into DAT_07eaa0f8.
void __cdecl Item_RecalculateRepairCost(void)
{
    // (HashTable obfuscation unlock block skipped)
    DAT_07eaa0f8 = 0;

    // Iterate 12 equipped item slots (base + 0x218, stride 0x44, total 0x330 bytes)
    for (int iVar7 = 0; iVar7 < 0x330; iVar7 += 0x44) {
        short *psVar12 = (short *)((int)DAT_07cf1ffc + 0x218 + iVar7);
        short itemType = *psVar12;
        if (itemType != -1 && *(int *)((char *)psVar12 + 0x38) != 0) {
            unsigned int maxDur = Item_CalculateMaxDurability(psVar12, (int)itemType * 0x40 + (int)DAT_07d78068,
                                               (*(int *)(psVar12 + 2) >> 3) & 0xf);
            unsigned int curDur = (unsigned int)*(unsigned char *)((char *)psVar12 + 0x1a);
            unsigned int uVar8  = (unsigned int)itemType;
            maxDur &= 0xffff;
            // Skip ring/wingtype/etc equipment IDs that don't degrade
            // BUG-FIX 2026-04-26 (audit #3): IDA real:
            //   gold = ItemValue(item, 2);
            //   DAT_07eaa0f8 += ConvertRepairGold(gold, dur, maxDur, type, buf);
            // El stub anterior pasaba (curDur, type, item, 2) → desordenado.
            if ((itemType < 416 || itemType > 419) &&
                itemType != 426 && itemType != 135 && itemType != 143 &&
                itemType < 448 &&
                (itemType < 391 || itemType > 403) &&
                (itemType < 430 || itemType > 435) &&
                curDur < maxDur) {
                char local_68[104]; // buffer for text
                int gold = Item_CalculateValue((void*)psVar12, 2);
                unsigned int repairGold = Item_CalculateRepairCost(gold, (int)curDur, (int)maxDur, itemType, local_68);
                DAT_07eaa0f8 += (int)repairGold;
                (void)uVar8;
            }
        }
    }

    // Iterate 8 extra item slots (DAT_07ea8410, stride 0x44, short* step = 0x22)
    short *psVar12 = (short *)&DAT_07ea8410;
    for (int i = 0; i < 8; i++, psVar12 += 0x22) {
        if (*(int *)((char *)psVar12 + 0x38) != 0) {
            short itemType = *psVar12;
            unsigned int maxDur = Item_CalculateMaxDurability(psVar12, (int)itemType * 0x40 + (int)DAT_07d78068,
                                               (*(int *)(psVar12 + 2) >> 3) & 0xf);
            unsigned int curDur = (unsigned int)*(unsigned char *)((char *)psVar12 + 0x1a);
            unsigned int uVar8  = (unsigned int)itemType;
            maxDur &= 0xffff;
            if ((itemType < 416 || itemType > 419) &&
                itemType != 426 && itemType != 135 && itemType != 143 &&
                itemType < 448 &&
                (itemType < 391 || itemType > 403) &&
                (itemType < 430 || itemType > 435) &&
                curDur < maxDur) {
                char local_68[104];
                int gold = Item_CalculateValue((void*)psVar12, 2);
                unsigned int repairGold = Item_CalculateRepairCost(gold, (int)curDur, (int)maxDur, itemType, local_68);
                DAT_07eaa0f8 += (int)repairGold;
                (void)uVar8;
            }
        }
    }
    // (Second HashTable ref-decrement + unaff_EBP block skipped — anti-tamper)
}

// FUN_004233e0 @ 0x004233E0 — HashTable_Unlock (2-arg, release read lock)
// STUB: HashTable obfuscation helper.
void __cdecl FUN_004233e0(int a, int b) { (void)a; (void)b; }
