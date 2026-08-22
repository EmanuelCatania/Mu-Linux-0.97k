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

// ── New helpers needed by SecondPassword implementations ─────────────────────

// GetItemCount @ 0x00482FF0 — count of inventory items of given type+level
// IDA-ported: walks the 8×8 inventory grid (base DAT_07EA9328..DAT_07EA9504,
// 17 ints per row, 136 ints per column) and counts matches.
// iType == -1: match any; iLevel == -1: match any.
int __cdecl GetItemCount(int siType, int iLevel) {
    int result = 0;
    int *v3 = (int *)&DAT_07ea9504;
    do {
        int *v4 = v3;
        int v5 = 8;
        do {
            if ( *((short *)v4 - 28) == siType
              && (siType == -1 || *v4 > 0)
              && (iLevel == -1 || ((*(v4 - 13) >> 3) & 0xF) == iLevel) )
            {
                ++result;
            }
            v4 -= 136;
            --v5;
        } while ( v5 );
        v3 -= 17;
    } while ( (int)v3 >= (int)&DAT_07ea9328 );
    return result;
}

// GetItemSlot @ 0x00482D70 — get inventory slot index for given type+level
// IDA-ported: same 8×8 inventory grid walk as GetItemCount, returns first
// matching slot index (0..63) or -1. iLevel == -1 matches any level.
int __cdecl GetItemSlot(int siType, int iLevel) {
    int v2 = 7;
    int *v3 = (int *)&DAT_07ea9504;
LABEL_2: {
        int v4 = 7;
        int result = v2 + 56;
        int *v6 = v3;
        while ( *((short *)v6 - 28) != siType || *v6 <= 0
                || (iLevel != -1 && ((*(v6 - 13) >> 3) & 0xF) != iLevel) )
        {
            --v4;
            v6 -= 136;
            result -= 8;
            if ( v4 < 0 )
            {
                v3 -= 17;
                --v2;
                if ( (int)v3 >= (int)&DAT_07ea9328 )
                    goto LABEL_2;
                return -1;
            }
        }
        return result;
    }
}

// FUN_0051d780 @ 0x0051D780 — CreateDialogInterface(textId, flag)
// Ported from IDA: builds a custom MessageBox-style dialog from GlobalText[textId].
//   - Wraps text into up to 7 lines × 38 chars via SeparateTextIntoLines
//   - Stores a 0x14-byte (5-int) button rect descriptor at DAT_083a42f8
//     {1, 71, 140, 70, 21} = (id=1, x=71, y=140, w=70, h=21) for the OK button
//   - Sets ErrorMessage=141 (or NextErrorMessage if one is already showing)
//
// Globals used:
//   DAT_083a7c04 = dialog text id (a1)
//   DAT_083a7c09 = flag byte (a2)
//   DAT_083a7c08 = state (cleared to 0)
//   ErrorMessage  = DAT_083a7c24 (via SetErrorMessage helper)
//   NextErrorMessage = DAT_083a7c28
void __cdecl FUN_0051d780(int a1, int a2)
{
    DAT_083a7c04 = (DWORD)a1;
    DAT_083a7c09 = (char)a2;
    DAT_083a7c08 = 0;

    g_iNumLineMessageBoxCustom = SeparateTextIntoLines(GlobalText[a1],
        g_lpszMessageBoxCustom[0], 7, 38);

    // Clear 0x28 (40 bytes = 10 ints) starting at DAT_083a42f8, then write 5 ints
    memset((void*)&DAT_083a42f8[0], 0, 0x28u);
    DAT_083a42f8[0] = 1;
    DAT_083a42f8[1] = 71;
    DAT_083a42f8[2] = 140;
    DAT_083a42f8[3] = 70;
    DAT_083a42f8[4] = 21;

    if (DAT_083a7c24 != 0) {
        DAT_083a7c28 = 141;
    } else {
        DAT_083a7c24 = 141;
    }
}

// FUN_004c45c0 @ 0x004C45C0 — CalcMaxDurability(ITEM*, ITEM_ATTRIBUTE*, Level) → WORD
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
unsigned int __cdecl FUN_004c45c0(void* item, int attrBase, int Level)
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

// FUN_0047c690 @ 0x0047C690 — ItemValue(ITEM* item, int sellMode) → int gold
//
// IDA-ported 2026-04-26 (audit #3): el stub anterior devolvía "número de dígitos"
// en vez del valor real. Calcula precio gold del item considerando type/level/
// durabilidad/options. Si sellMode!=0 aplica reducción ×1/3 y penalty por durab.
//
// Caso especial #135 = Wings stage 1 / #143 = Wings stage 2 (jewel pricing).
// Cases 461/462/464/470/430/431/419/432-434/465-467/469/457/468 = jewels y sets.
// Constantes derivadas del binario original (`&unk_xxxxxx` en IDA = direcciones
// usadas como valores enteros — comprobado: 0x895440 = 9_000_000, etc.).
int __cdecl FUN_0047c690(void* item_v, int a2)
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
        unsigned int maxDur = FUN_004c45c0((void*)a1, (int)((unsigned short)v25) * 0x40 + (int)DAT_07d78068, Level) & 0xffff;
        double penalty = (1.0 - (double)*(unsigned char*)(a1 + 26) / (double)maxDur)
                       * (double)v34 * -0.60000002;
        v5 += (int)(__int64)penalty;
    }
    if (v5 >= 1000) return 100 * (v5 / 100);
    if (v5 >= 100)  return 10  * (v5 / 10);
    return v5;
}

// FUN_004c3ef0 @ 0x004C3EF0 — ConvertRepairGold(Gold, Durability, MaxDurability, Type, Text)
//
// IDA-ported 2026-04-26 (audit #3): el stub anterior solo escribía "%u / %u"
// pero el real calcula gold de reparación: sqrt(sqrt(Gold)) * sqrt(Gold) * 3 *
// (1 - dur/maxDur) + 1, con bonus 1.4× si rota, +5% si RepairEnable, redondeo
// a múltiplos de 100/10, y formato "1,234,567" en Text. Devuelve gold final.
unsigned int __cdecl FUN_004c3ef0(int Gold, int Durability, int MaxDurability, short Type, char* Text)
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

// FUN_004c4080 @ 0x004C4080 — CharData_RecalcDurability
// Scans equipped items (+0x218, stride 0x44, 12 slots) and extra items (DAT_07ea8410, 8 slots).
// For each occupied slot with curDur < maxDur, calls CalcMaxDurability + AppendDurabilityLine.
// Accumulates repair-count into DAT_07eaa0f8.
void __cdecl FUN_004c4080(void)
{
    // (HashTable obfuscation unlock block skipped)
    DAT_07eaa0f8 = 0;

    // Iterate 12 equipped item slots (base + 0x218, stride 0x44, total 0x330 bytes)
    for (int iVar7 = 0; iVar7 < 0x330; iVar7 += 0x44) {
        short *psVar12 = (short *)((int)DAT_07cf1ffc + 0x218 + iVar7);
        short itemType = *psVar12;
        if (itemType != -1 && *(int *)((char *)psVar12 + 0x38) != 0) {
            unsigned int maxDur = FUN_004c45c0(psVar12, (int)itemType * 0x40 + (int)DAT_07d78068,
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
                int gold = FUN_0047c690((void*)psVar12, 2);
                unsigned int repairGold = FUN_004c3ef0(gold, (int)curDur, (int)maxDur, itemType, local_68);
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
            unsigned int maxDur = FUN_004c45c0(psVar12, (int)itemType * 0x40 + (int)DAT_07d78068,
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
                int gold = FUN_0047c690((void*)psVar12, 2);
                unsigned int repairGold = FUN_004c3ef0(gold, (int)curDur, (int)maxDur, itemType, local_68);
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


// FUN_00409e20 — implemented in src/Net/Crypto.cpp
// FUN_00423760 — implemented in src/Net/Crypto.cpp

// FUN_004414d0 @ 0x004414D0 — BMD_DrawBoneSlot_Anim
// Renders polygons for a mesh slot using bone-animated vertex positions.
// unaff_EBX in original = mesh entry = model.Actions[frame], same as iVar7 computed below.
void __cdecl FUN_004414d0(void *model, char a, int b, float frame, int flags,
                           float f3, float f4, float f5, float f6, float f7, unsigned int rgba)
{
    if (!model) return;

    // Mesh entry = Actions[frame] at stride 0x28
    int meshEntry = *(int *)((int)model + 0x28) + (int)frame * 0x28;

    // Texture index from bone index lookup
    unsigned int texIdx = (unsigned int)*(short *)(*(int *)((int)model + 0x38) +
                           *(short *)(meshEntry + 2) * 2);
    if (texIdx == 300) return;  // BITMAP_HIDE
    if (*(short *)(meshEntry + 10) == 0) return;  // no polygons

    // Override texture with rgba param if not 0xffffffff
    if (rgba != 0xffffffff) texIdx = rgba;

    int bVar3 = (int)(unsigned char)((unsigned int)flags & 0xFF);

    // GL state setup
    if ((bVar3 & 1) == 1) {
        if ((bVar3 & 0x40) == 0x40)      FUN_00511710();
        else if ((bVar3 & 0x80) == 0x80) FUN_00511790();
        else                             FUN_00511600();
        FUN_00511590('\0');
        glColor3fv((float *)((int)model + 0x48));
    } else if ((bVar3 & 2) == 2) {
        FUN_00511480(texIdx);
        if ((bVar3 & 0x40) == 0x40)      FUN_00511710();
        else if ((bVar3 & 0x80) == 0x80) FUN_00511790();
        else                             FUN_00511600();
    } else if ((bVar3 & 0x40) == 0x40) {
        if (texIdx == 4) return;  // (&DAT_083a7cc8)[local_24 * 0x38] == 4 early-out
        FUN_00511710();
        FUN_00511590('\0');
        FUN_00511530();
    }
    // else param_6 = 2.8026e-45 — no extra state

    // (HashTable obfuscation block skipped — pure ref-count noise)

    glBegin(GL_TRIANGLES);

    int polyCount = *(short *)(meshEntry + 10);
    int param_5_i = 0;
    for (int local_20 = 0; local_20 < polyCount; local_20++) {
        char *pcVar10 = (char *)(param_5_i + *(int *)(meshEntry + 0x1c));
        if (*pcVar10 > 0) {
            int deformFlag = b & 1;
            short *psVar15 = (short *)(pcVar10 + 10);
            for (int vi = 0; vi < (int)*pcVar10; vi++, psVar15++) {
                int iVar7 = (int)psVar15[-4];

                if ((int)flags == 2) {
                    // Textured: UV from UV array
                    float *uvPtr = (float *)(*(int *)(meshEntry + 0x18) + (int)psVar15[4] * 8);
                    float uCoord, vCoord;
                    if (f5 == 0.0f) {
                        uCoord = *uvPtr;
                        vCoord = uvPtr[1];
                    } else {
                        uCoord = f5 + *uvPtr;
                        vCoord = f6 + uvPtr[1];
                    }
                    glTexCoord2f(uCoord, vCoord);
                    if (a != '\0') {
                        int iVar13 = ((int)*psVar15 + (int)frame * 15000) * 0xc;
                        if (f3 < _DAT_00552544) {
                            glColor4f(*(float *)(&DAT_060db65c + iVar13),
                                      *(float *)(&DAT_060db65c + iVar13 + 4),
                                      *(float *)(&DAT_060db65c + iVar13 + 8), f3);
                        } else {
                            glColor3fv((float *)(&DAT_060db65c + iVar13));
                        }
                    }
                } else if ((int)flags == 4) {
                    // Chrome UV
                    if (f3 < _DAT_00552544) {
                        glColor4f(*(float *)((int)model + 0x48), *(float *)((int)model + 0x4c),
                                  *(float *)((int)model + 0x50), f3);
                    } else {
                        glColor3fv((float *)((int)model + 0x48));
                    }
                    // BUG-FIX 2026-07-15: el V leía `&DAT_05828d5c + (idx*2+1)*4`
                    // (stride ×4, Ghidra float→byte mis-decompile) → out-of-bounds /
                    // UV degenerado. La tabla es {U,V} contigua: V = índice idx*2+1.
                    glTexCoord2f((&DAT_05828d5c)[*psVar15 * 2],
                                 (&DAT_05828d5c)[*psVar15 * 2 + 1]);
                }

                // Vertex position
                float *pfVar5;
                float afStack_14[3];
                if (deformFlag) {
                    // Sin-wave deformation
                    int iVar13 = iVar7 * 0x3a3 + (int)frame;
                    float sinVal = (float)fsin((double)iVar13 * (double)_DAT_005528c4);
                    float *pfVar12 = (float *)((char*)&DAT_0584621c + ((int)frame * 15000 + iVar7) * 3 * 4);
                    int normBase = ((int)*psVar15 + (int)frame * 15000) * 0xc;
                    for (int k = 0; k < 3; k++) {
                        afStack_14[k] = sinVal * *(float *)(&DAT_06f433bc + normBase + k * 4)
                                        * _DAT_00552644 + pfVar12[k];
                    }
                    pfVar5 = afStack_14;
                } else {
                    pfVar5 = (float *)((char*)&DAT_0584621c + (iVar7 + (int)frame * 15000) * 3 * 4);
                }
                glVertex3fv(pfVar5);
            }
        }
        param_5_i += 0x24;
    }

    glEnd();
    (void)f4; (void)f7;
}

// FUN_004e13a0 @ 0x004E13A0 — RenderObjectScreen
// Renders a 3D item/object at world position param_4[0..2].
// Sets rotation globals per type, builds a stack entity, calls BMD_Animation + Entity_DrawAt.
static bool ApplyInventoryExactPoseLate(int param_1, int level, float* outPos)
{
    switch (param_1) {
    case MODEL_SWORD + 0:
        outPos[0] -= 0.02f; outPos[1] += 0.03f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_SPEAR + 0:
        outPos[1] += 0.05f;
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 20.0f; return true;
    case MODEL_BOW + 7:
    case MODEL_BOW + 15:
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 17:
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 20:
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 21:
        outPos[1] += 0.12f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_BOW + 22:
    case MODEL_BOW + 23:
        outPos[0] -= 0.10f; outPos[1] += 0.08f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_SPEAR + 10:
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 20.0f; return true;
    case MODEL_HELM + 30:
        outPos[0] -= 0.03f; outPos[1] += 0.07f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELM + 31:
        outPos[0] += 0.03f; outPos[1] -= 0.06f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELM + 35:
        outPos[0] -= 0.02f; outPos[1] += 0.05f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 5:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 180.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 6:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 7:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 10:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 11:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = -20.0f; return true;
    case MODEL_EVENT + 12:
        _DAT_07ea952c = 250.0f; _DAT_07ea9530 = 140.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 14:
        _DAT_07ea952c = 255.0f; _DAT_07ea9530 = 160.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 15:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 16:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_EVENT + 18:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 3:
    case MODEL_HELPER + 4:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 5:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = -35.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 16:
    case MODEL_HELPER + 17:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 18:
        _DAT_07ea952c = 290.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 21:
    case MODEL_HELPER + 22:
    case MODEL_HELPER + 23:
    case MODEL_HELPER + 24:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 160.0f; _DAT_07ea9534 = 20.0f; return true;
    case MODEL_HELPER + 29:
        _DAT_07ea952c = 290.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_HELPER + 30:
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 12:
        if (level == 0) { _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; }
        else if (level == 1) { _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f; }
        else if (level == 2) { _DAT_07ea952c = 90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; }
        return true;
    case MODEL_STAFF + 7:
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 205.0f; return true;
    case MODEL_STAFF + 12:
        outPos[0] += 0.025f; outPos[1] -= 0.10f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 8.0f; return true;
    case MODEL_STAFF + 13:
        outPos[0] += 0.02f; outPos[1] += 0.02f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 8.0f; return true;
    case MODEL_POTION + 20:
    case MODEL_POTION + 27:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 13:
    case MODEL_POTION + 14:
    case MODEL_POTION + 22:
        outPos[0] += 0.005f; outPos[1] += 0.015f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 21:
        outPos[0] += 0.005f; outPos[1] -= 0.005f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 41:
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 42:
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_POTION + 43:
    case MODEL_POTION + 44:
        outPos[0] -= 0.04f; outPos[1] += 0.02f; outPos[2] += 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = -45.0f; return true;
    case MODEL_POTION + 63:
        outPos[1] += 0.08f;
        _DAT_07ea952c = -50.0f; _DAT_07ea9530 = -60.0f; _DAT_07ea9534 = 0.0f; return true;
    case MODEL_SWORD + 26:
        outPos[0] -= 0.02f; outPos[1] += 0.04f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_SWORD + 27:
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_SWORD + 28:
        outPos[1] += 0.02f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_MACE + 16:
        outPos[0] -= 0.02f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_MACE + 17:
        outPos[0] -= 0.02f; outPos[1] += 0.04f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 270.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_MACE + 18:
        outPos[0] -= 0.03f; outPos[1] += 0.06f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 2.0f; return true;
    case MODEL_MACE + 14:
        outPos[0] -= 0.01f; outPos[1] += 0.10f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 13.0f; return true;
    case MODEL_MACE + 15:
        outPos[1] += 0.05f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 13.0f; return true;
    case MODEL_SPEAR + 11:
        outPos[1] += 0.02f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_STAFF + 30:
    case MODEL_STAFF + 31:
    case MODEL_STAFF + 32:
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_STAFF + 33:
        outPos[0] += 0.02f; outPos[1] -= 0.06f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_STAFF + 34:
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 10.0f; return true;
    case MODEL_BOW + 24:
        outPos[0] -= 0.07f; outPos[1] += 0.07f;
        _DAT_07ea952c = 180.0f; _DAT_07ea9530 = -90.0f; _DAT_07ea9534 = 15.0f; return true;
    case MODEL_HELPER + 39:
    case MODEL_HELPER + 40:
    case MODEL_HELPER + 41:
    case MODEL_HELPER + 42:
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    default:
        if (param_1 >= MODEL_HELPER + 12 && param_1 < MODEL_HELPER + 512 &&
            param_1 != MODEL_HELPER + 12 && param_1 != MODEL_HELPER + 13 &&
            param_1 != MODEL_HELPER + 14 && param_1 != MODEL_HELPER + 15) {
            _DAT_07ea952c = 360.0f;
            _DAT_07ea9530 = 0.0f;
            _DAT_07ea9534 = 0.0f;
            return true;
        }
        if (param_1 == MODEL_ARMOR + 29) {
            outPos[1] += 0.07f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 30) {
            outPos[1] += 0.10f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 34) {
            outPos[1] += 0.03f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 35) {
            outPos[1] += 0.05f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_ARMOR + 36 || param_1 == MODEL_ARMOR + 37) {
            outPos[1] -= 0.05f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_HELM + 39 && param_1 <= MODEL_HELM + 44) {
            outPos[1] -= 0.05f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 25.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_ARMOR + 38 && param_1 <= MODEL_ARMOR + 44) {
            outPos[1] -= 0.08f;
            _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_STAFF + 21 && param_1 <= MODEL_STAFF + 29) {
            _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_POTION + 130 && param_1 <= MODEL_POTION + 132) {
            outPos[1] += 0.06f;
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 133) {
            outPos[0] += 0.01f;
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_POTION + 134 && param_1 <= MODEL_POTION + 139) {
            outPos[1] += 0.05f;
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 140) {
            outPos[1] += 0.09f;
            _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 52) {
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -25.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 63) {
            outPos[1] += 0.08f;
            _DAT_07ea952c = -50.0f; _DAT_07ea9530 = -60.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 == MODEL_POTION + 160 || param_1 == MODEL_POTION + 161) {
            outPos[1] += 0.05f;
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
        if (param_1 >= MODEL_POTION + 145 && param_1 <= MODEL_POTION + 150) {
            outPos[0] += 0.01f; outPos[1] += 0.04f;
            _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
        }
    return false;
}

}

#if 0
void __cdecl FUN_004e13a0(int param_1, unsigned int param_2, unsigned char param_3, unsigned char param_4, float *param_5, int param_6, char param_7)
{
    // 2026-05-08: per-call recovery. Esta función se llama MUCHAS veces por
    // frame (una por cada item 3D del inventario). El watchdog en Render_GameFrame
    // sólo recupera 1 vez por frame; si la corrupción de DAT_07d78068 ocurre
    // entre dos calls del mismo frame, las posteriores crashean. Recuperar
    // inline antes del read.
    {
        unsigned int p = (unsigned int)DAT_07d78068;
        if (p < 0x100000u || p >= 0x80000000u) {
            if (g_ItemAttribute_Backup >= 0x100000u && g_ItemAttribute_Backup < 0x80000000u) {
                DAT_07d78068 = (int)g_ItemAttribute_Backup;
            } else {
                return;   // can't recover — bail
        }
}

#endif
static bool ApplyInventoryExactPoseTail(int param_1, float* outPos)
{
    if (param_1 == MODEL_POTION + 96) {
        outPos[0] += 0.003f; outPos[1] -= 0.013f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_POTION + 99) {
        outPos[0] += 0.02f; outPos[1] -= 0.03f;
        _DAT_07ea952c = 290.0f; _DAT_07ea9530 = -40.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_POTION + 100) {
        outPos[0] += 0.01f; outPos[1] -= 0.05f;
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 76) {
        outPos[1] -= 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 80 || param_1 == MODEL_HELPER + 123) {
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 40.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 81 || param_1 == MODEL_HELPER + 82) {
        outPos[0] += 0.005f; outPos[1] += 0.035f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 93 || param_1 == MODEL_HELPER + 94) {
        outPos[0] += 0.005f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 97 || param_1 == MODEL_HELPER + 98) {
        outPos[0] += 0.002f; outPos[1] -= 0.04f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 99) {
        outPos[0] += 0.002f; outPos[1] += 0.025f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 180.0f; _DAT_07ea9534 = 45.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 103) {
        outPos[0] += 0.01f; outPos[1] += 0.01f;
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 104 || param_1 == MODEL_HELPER + 105) {
        outPos[0] += 0.01f; outPos[1] -= 0.03f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 106) {
        outPos[0] += 0.01f; outPos[1] -= 0.05f;
        _DAT_07ea952c = 255.0f; _DAT_07ea9530 = 45.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 107) {
        _DAT_07ea952c = 90.0f; _DAT_07ea9530 = 225.0f; _DAT_07ea9534 = 45.0f; return true;
    }
    if (param_1 >= MODEL_HELPER + 109 && param_1 <= MODEL_HELPER + 112) {
        outPos[0] += 0.025f; outPos[1] -= 0.035f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 25.0f; _DAT_07ea9534 = 25.0f; return true;
    }
    if (param_1 >= MODEL_HELPER + 113 && param_1 <= MODEL_HELPER + 115) {
        outPos[0] += 0.005f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 116) {
        outPos[0] += 0.005f; outPos[1] -= 0.03f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 121) {
        outPos[1] -= 0.04f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 122) {
        outPos[0] += 0.01f; outPos[1] -= 0.035f;
        _DAT_07ea952c = 290.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 124) {
        outPos[1] -= 0.04f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_HELPER + 125 && param_1 <= MODEL_HELPER + 127) {
        outPos[0] += 0.007f; outPos[1] -= 0.035f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 128 || param_1 == MODEL_HELPER + 131 || param_1 == MODEL_HELPER + 133) {
        outPos[0] += 0.017f; outPos[1] -= 0.053f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 129) {
        outPos[0] += 0.012f; outPos[1] -= 0.045f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 130) {
        outPos[0] += 0.007f; outPos[1] += 0.005f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 132) {
        outPos[0] += 0.007f; outPos[1] += 0.045f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 134) {
        outPos[0] += 0.005f; outPos[1] -= 0.033f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -20.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_HELPER + 135 && param_1 <= MODEL_HELPER + 145) {
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_WING + 30 || param_1 == MODEL_WING + 31 ||
        (param_1 >= MODEL_WING + 136 && param_1 <= MODEL_WING + 143)) {
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f;
        if (param_1 == MODEL_WING + 142 || param_1 == MODEL_WING + 143) _DAT_07ea9534 = -45.0f;
        else if (param_1 == MODEL_WING + 136 || param_1 == MODEL_WING + 137) outPos[1] -= 0.05f;
        else if (param_1 == MODEL_WING + 139) { outPos[1] -= 0.05f; _DAT_07ea9530 = 90.0f; }
        return true;
    }
    if (param_1 >= MODEL_WING + 60 && param_1 <= MODEL_WING + 65) {
        _DAT_07ea952c = 10.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 10.0f; return true;
    }
    if (param_1 >= MODEL_WING + 70 && param_1 <= MODEL_WING + 74) {
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_WING + 100 && param_1 <= MODEL_WING + 129) {
        _DAT_07ea952c = 0.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_WING + 49) {
        outPos[0] += 0.015f; outPos[1] += 0.01f;
        _DAT_07ea952c = -90.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_WING + 50) {
        outPos[1] += 0.15f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_WING + 135) {
        outPos[0] += 0.005f; outPos[1] += 0.05f; return true;
    }
    if (param_1 == MODEL_POTION + 52) {
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -25.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_ETC + 19 && param_1 <= MODEL_ETC + 27) {
        outPos[0] += 0.03f; outPos[1] += 0.03f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_HELPER + 38) {
        outPos[1] += 0.02f;
        _DAT_07ea952c = -198.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_POTION + 63) {
        outPos[1] += 0.08f;
        _DAT_07ea952c = -50.0f; _DAT_07ea9530 = -60.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_WING + 7) {
        outPos[0] += 0.005f; outPos[1] -= 0.015f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_ETC + 30 && param_1 <= MODEL_ETC + 36) {
        outPos[0] += 0.03f; outPos[1] += 0.03f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_POTION + 110) {
        outPos[0] += 0.005f; outPos[1] -= 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_POTION + 111) {
        outPos[0] += 0.01f; outPos[1] -= 0.02f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_POTION + 112 && param_1 <= MODEL_POTION + 113) {
        outPos[0] += 0.05f; outPos[1] += 0.009f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 180.0f; _DAT_07ea9534 = 45.0f; return true;
    }
    if (param_1 >= MODEL_POTION + 114 && param_1 <= MODEL_POTION + 119) {
        outPos[1] += 0.06f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 == MODEL_POTION + 120) {
        outPos[0] += 0.01f; outPos[1] += 0.05f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    if (param_1 >= MODEL_POTION + 126 && param_1 <= MODEL_POTION + 129) {
        outPos[1] += 0.06f;
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f; return true;
    }
    return false;
}
void __cdecl FUN_004e13a0(int param_1, unsigned int param_2, unsigned char param_3, unsigned char param_4, float *param_5, int param_6, char param_7)
{
    // 2026-05-08: per-call recovery. Esta función se llama muchas veces por frame.
    {
        unsigned int p = (unsigned int)DAT_07d78068;
        if (p < 0x100000u || p >= 0x80000000u) {
            if (g_ItemAttribute_Backup >= 0x100000u && g_ItemAttribute_Backup < 0x80000000u) {
                DAT_07d78068 = (int)g_ItemAttribute_Backup;
            } else {
                return;
            }
        }
    }
    float direction[3];
    direction[0] = param_5[0] - _DAT_083a4284;
    direction[1] = param_5[1] - _DAT_083a4288;
    direction[2] = param_5[2] - _DAT_083a428c;

    float outPos[3];
    float camPos[3] = { _DAT_083a4284, _DAT_083a4288, _DAT_083a428c };
    FUN_004f9ce0(camPos, param_7 ? 0.07f : 0.1f, direction, outPos);

    // ── ITEM3D: ver el bloque SCALE3D más abajo (después de resolver local_3bc).
    // ── (round-trip ya verificado: proj_pos == proj_tgt, cadena consistente)
#if 0
    // ── ITEM3D (temporal): prueba de ida y vuelta del descentrado de items.
    // `Projection` (0x5113F0) es el inverso exacto de `CreateScreenVector`, así
    // que re-proyectar a pantalla debe devolver las MISMAS coords que entraron.
    //   · Target y outPos deben proyectar al MISMO punto: el lerp va a lo largo
    //     del rayo de visión, así que sólo conserva la posición en pantalla si
    //     `MousePosition` (DAT_083a4284) es de verdad el ojo de la cámara.
    //   · Si difieren, el corrimiento medido ES el bug y su magnitud dice cuánto.
    {
        static DWORD s_lastI = 0;
        DWORD nowI = GetTickCount();
        if (nowI - s_lastI > 1000) {
            s_lastI = nowI;
            int tx = 0, ty = 0, px = 0, py = 0;
            FUN_005113f0(param_5, &tx, &ty);   // Target  → pantalla
            FUN_005113f0(outPos,  &px, &py);   // Position→ pantalla
            char ib[240];
            _snprintf_s(ib, sizeof(ib), _TRUNCATE,
                "ITEM3D type=%d cam=(%.1f,%.1f,%.1f) tgt=(%.1f,%.1f,%.1f) "
                "pos=(%.1f,%.1f,%.1f) proj_tgt=(%d,%d) proj_pos=(%d,%d) d=(%d,%d)",
                (int)param_1, camPos[0], camPos[1], camPos[2],
                param_5[0], param_5[1], param_5[2],
                outPos[0], outPos[1], outPos[2],
                tx, ty, px, py, px - tx, py - ty);
            DbgLogPublic(ib);
        }
    }
#endif

    int level = ((int)param_2 >> 3) & 0xf;
    short modelType = (short)param_1;

    // Default rotation values (overridden below)
    _DAT_07ea952c = 0.0f;
    _DAT_07ea9530 = 0.0f;
    _DAT_07ea9534 = 0.0f;

    DWORD local_3bc = 0;  // model scale (raw IEEE 754 float bits)
    bool exactPose = false;

    if (param_1 == 0x190) { // MODEL_SWORD+0
        outPos[0] -= 0.02f;
        outPos[1] += 0.03f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 270.0f;
        _DAT_07ea9534 = 15.0f;
        exactPose = true;
    } else if (param_1 == 0x1f0) { // MODEL_SPEAR+0
        outPos[1] += 0.05f;
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 20.0f;
        exactPose = true;
    } else if (param_1 == 0x224) { // MODEL_BOW+20
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 15.0f;
        exactPose = true;
    } else if (param_1 == 0x225) { // MODEL_BOW+21
        outPos[1] += 0.12f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 15.0f;
        exactPose = true;
    } else if (param_1 == 0x226 || param_1 == 0x227) { // MODEL_BOW+22/+23
        outPos[0] -= 0.10f;
        outPos[1] += 0.08f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 15.0f;
        exactPose = true;
    } else if (param_1 == 0x23c) { // MODEL_STAFF+12
        outPos[1] -= 0.10f;
        outPos[0] += 0.025f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 8.0f;
        exactPose = true;
    } else if (param_1 == 0x23d) { // MODEL_STAFF+13
        outPos[0] += 0.02f;
        outPos[1] += 0.02f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 8.0f;
        exactPose = true;
    } else if (param_1 >= 0x245 && param_1 <= 0x24d) { // MODEL_STAFF+21..29
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x1de) { // MODEL_MACE+14
        outPos[1] += 0.10f;
        outPos[0] -= 0.01f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 13.0f;
        exactPose = true;
    } else if (param_1 == 0x1df) { // MODEL_MACE+15
        outPos[1] += 0.05f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 13.0f;
        exactPose = true;
    } else if (param_1 == 0x28e) { // MODEL_HELM+30
        outPos[1] += 0.07f;
        outPos[0] -= 0.03f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x28f) { // MODEL_HELM+31
        outPos[1] -= 0.06f;
        outPos[0] += 0.03f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x2ad) { // MODEL_ARMOR+29
        outPos[1] += 0.07f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x2ae) { // MODEL_ARMOR+30
        outPos[1] += 0.10f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x293) { // MODEL_HELM+35
        outPos[0] -= 0.02f;
        outPos[1] += 0.05f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x297 && param_1 <= 0x29c) { // MODEL_HELM+39..44
        outPos[1] -= 0.05f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 25.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x2b2) { // MODEL_ARMOR+34
        outPos[1] += 0.03f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x2b3) { // MODEL_ARMOR+35
        outPos[1] += 0.05f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x2b4 || param_1 == 0x2b5) { // MODEL_ARMOR+36/+37
        outPos[1] -= 0.05f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x2b6 && param_1 <= 0x2bc) { // MODEL_ARMOR+38..44
        outPos[1] -= 0.08f;
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x1a8) { // MODEL_SWORD+24
        outPos[0] -= 0.02f;
        outPos[1] += 0.03f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 15.0f;
        exactPose = true;
    } else if (param_1 == 0x24b) { // MODEL_EVENT+10
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x24c) { // MODEL_EVENT+11
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = -20.0f;
        _DAT_07ea9534 = -20.0f;
        exactPose = true;
    } else if (param_1 == 0x24d) { // MODEL_EVENT+12
        _DAT_07ea952c = 250.0f;
        _DAT_07ea9530 = 140.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x24f) { // MODEL_EVENT+14
        _DAT_07ea952c = 255.0f;
        _DAT_07ea9530 = 160.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x250) { // MODEL_EVENT+15
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x251) { // MODEL_EVENT+16
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_STAFF + 7) {
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 205.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 0) {
        outPos[0] += 0.002f;
        outPos[1] += 0.010f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 1 || param_1 == MODEL_HELPER + 2) {
        outPos[0] += 0.002f;
        outPos[1] += 0.008f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 12 || param_1 == MODEL_HELPER + 13) {
        outPos[0] += 0.002f;
        outPos[1] += 0.010f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if ((param_1 >= MODEL_POTION + 1 && param_1 <= MODEL_POTION + 10) ||
               param_1 == MODEL_POTION + 13 || param_1 == MODEL_POTION + 14 || param_1 == MODEL_POTION + 22) {
        outPos[0] += 0.005f;
        outPos[1] += 0.015f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_POTION + 21) {
        outPos[0] += 0.005f;
        outPos[1] -= 0.005f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -10.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x34e) { // MODEL_HELPER+30
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x34f) { // MODEL_HELPER+31
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x350) { // MODEL_HELPER+32
        outPos[0] += 0.01f;
        outPos[1] -= 0.03f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x351) { // MODEL_HELPER+33
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x352 || param_1 == 0x353) { // MODEL_HELPER+34/+35
        outPos[0] += 0.01f;
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x354) { // MODEL_HELPER+36
        outPos[0] += 0.01f;
        outPos[1] += 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x355) { // MODEL_HELPER+37
        outPos[0] += 0.01f;
        outPos[1] += 0.04f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x35b || param_1 == 0x35d) { // raw ids verified from asset table
        outPos[1] += (param_1 == 0x35b) ? -0.027f : -0.02f;
        outPos[0] += 0.005f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x35e || param_1 == 0x35f || param_1 == 0x360 || param_1 == 0x361 || param_1 == 0x362 || param_1 == 0x363) { // MODEL_HELPER+46..51
        outPos[1] += (param_1 == 0x361) ? -0.04f : (param_1 == 0x362 ? -0.03f : (param_1 == 0x363 ? -0.02f : -0.04f));
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x364) { // MODEL_HELPER+52
        outPos[1] += 0.045f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x365) { // MODEL_HELPER+53
        outPos[1] += 0.04f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 120.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x366 && param_1 <= 0x36a) { // MODEL_HELPER+54..58
        outPos[1] -= 0.02f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x36b) { // MODEL_HELPER+59
        outPos[0] += 0.01f;
        outPos[1] += 0.02f;
        _DAT_07ea952c = 90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x36c) { // MODEL_HELPER+60
        outPos[1] -= 0.06f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x36d) { // MODEL_HELPER+61
        outPos[1] -= 0.04f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x36e) { // MODEL_HELPER+62
        outPos[0] += 0.01f;
        outPos[1] -= 0.03f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x36f) { // MODEL_HELPER+63
        outPos[0] += 0.01f;
        outPos[1] += 0.082f;
        _DAT_07ea952c = 90.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x385) { // MODEL_POTION+53
        outPos[1] += 0.042f;
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x386) { // MODEL_POTION+54
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x38a) { // MODEL_POTION+58
        outPos[1] += 0.07f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x38b && param_1 <= 0x38e) { // MODEL_POTION+59..62
        outPos[1] += 0.06f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x390) { // MODEL_POTION+64
        outPos[1] += 0.02f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x396 && param_1 <= 0x397) { // MODEL_POTION+70..71
        outPos[0] += 0.01f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x398 && param_1 <= 0x39d) { // MODEL_POTION+72..77
        outPos[1] += 0.08f;
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x39e && param_1 <= 0x3a2) { // MODEL_POTION+78..82
        outPos[1] += 0.01f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x3a3) { // MODEL_POTION+83
        outPos[1] += 0.06f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x3a4 && param_1 <= 0x3aa) { // MODEL_POTION+84..90
        if (param_1 == 0x3a4 || param_1 == 0x3a6 || param_1 == 0x3a7) outPos[1] += 0.01f;
        else if (param_1 == 0x3a5) outPos[1] -= 0.01f;
        else outPos[1] += 0.015f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 >= 0x3ab && param_1 <= 0x3af) { // MODEL_POTION+91..95
        if (param_1 == 0x3ae) outPos[0] += 0.01f; // +94
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == 0x3b1 || param_1 == 0x3b2) { // MODEL_POTION+97..98
        outPos[1] += 0.09f;
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_WING + 37 || param_1 == MODEL_WING + 38 || param_1 == MODEL_WING + 40) {
        outPos[1] += 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -10.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_WING + 39) {
        outPos[1] += 0.08f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -10.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_WING + 42) {
        outPos[1] += 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 2.0f;
        exactPose = true;
    } else if (param_1 == MODEL_WING + 44 || param_1 == MODEL_WING + 45 || param_1 == MODEL_WING + 46 || param_1 == MODEL_WING + 47) {
        outPos[0] += 0.005f;
        outPos[1] -= 0.015f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 64) {
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -10.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 65) {
        outPos[1] -= 0.02f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -10.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 66) {
        outPos[0] += 0.01f;
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 67) {
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 40.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 68) {
        outPos[0] += 0.02f;
        outPos[1] -= 0.02f;
        _DAT_07ea952c = 300.0f;
        _DAT_07ea9530 = 10.0f;
        _DAT_07ea9534 = 20.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 69) {
        outPos[0] += 0.005f;
        outPos[1] -= 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = -30.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_HELPER + 70) {
        outPos[0] += 0.04f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 70.0f;
        exactPose = true;
    } else if (param_1 >= MODEL_HELPER + 71 && param_1 <= MODEL_HELPER + 75) {
        outPos[1] += 0.07f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = (param_6 == 1) ? 180.0f : (DAT_05826e08 * 0.2f);
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_ARMOR + 10 || param_1 == MODEL_ARMOR + 11) {
        outPos[1] -= 0.10f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_PANTS + 10 || param_1 == MODEL_PANTS + 11) {
        outPos[1] -= 0.08f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_POTION + 65) {
        outPos[1] += 0.05f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (param_1 == MODEL_POTION + 66 || param_1 == MODEL_POTION + 67) {
        outPos[1] += 0.11f;
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    }

    if (!exactPose)
        exactPose = ApplyInventoryExactPoseTail(param_1, outPos);

    if (!exactPose && param_1 == MODEL_EVENT + 5) {
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 180.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (!exactPose && param_1 == MODEL_EVENT + 6) {
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (!exactPose && param_1 == MODEL_EVENT + 7) {
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    }

    if (!exactPose && (param_1 >= MODEL_POTION + 32 && param_1 <= MODEL_POTION + 34)) {
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = (float)DAT_05826e08 * _DAT_00552c00;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    } else if (!exactPose && (param_1 >= MODEL_EVENT + 21 && param_1 <= MODEL_EVENT + 23)) {
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = (float)DAT_05826e08 * _DAT_00552c00;
        _DAT_07ea9534 = 0.0f;
        exactPose = true;
    }

    if (!exactPose)
        exactPose = ApplyInventoryExactPoseLate(param_1, (int)param_2, outPos);

    // ── BUG-FIX (2026-04-20) ─────────────────────────────────────────────────
    // Las líneas siguientes antes asignaban `_DAT_07ea952c = 0x42b40000` etc.
    // Como esos globals están tipados `float` en globals.h/cpp, C hace conversión
    // int→float: 0x42b40000 == 1'119'748'096, no 90.0f. El patrón es idéntico al
    // bug que tuvimos en _DAT_005597c8 — producía rotaciones locas (~1e9°).
    if (exactPose) {
    } else if (param_1 == 0x217 || param_1 == 0x21f) {
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 270.0f;
        _DAT_07ea9534 = 15.0f;
    } else if (param_1 == 0x221) {
        _DAT_07ea952c = 0.0f;
        _DAT_07ea9530 = 90.0f;
        _DAT_07ea9534 = 15.0f;
    } else if (param_1 >= 0x218 && param_1 <= 0x22f) {
        _DAT_07ea952c = 90.0f;
        _DAT_07ea9530 = 180.0f;
        _DAT_07ea9534 = 20.0f;
    } else if (param_1 == 0x1fa) {
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 270.0f;
        _DAT_07ea9534 = 20.0f;
    } else if (param_1 >= 0x250 && param_1 <= 0x26f) {
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = 270.0f;
        _DAT_07ea9534 = 0.0f;
    } else if (param_1 >= 0x190 && param_1 < 0x250) {
        switch (param_1) {
        case 0x23e: outPos[1] += 0.04f; break;                 // MODEL_STAFF+14
        case 0x241: outPos[0] += 0.02f; outPos[1] += 0.03f; break; // MODEL_STAFF+17
        case 0x242: outPos[0] += 0.02f; break;                 // MODEL_STAFF+18
        case 0x243: outPos[0] -= 0.02f; outPos[1] -= 0.02f; break; // MODEL_STAFF+19
        case 0x244: outPos[0] += 0.01f; outPos[1] -= 0.01f; break; // MODEL_STAFF+20
        default: break;
        }
        _DAT_07ea952c = 180.0f;
        _DAT_07ea9530 = 270.0f;
        if (*(char *)(param_1 * 0x40 + -0x63e2 + DAT_07d78068) != '\0')
            _DAT_07ea9534 = 25.0f;
        else
            _DAT_07ea9534 = 15.0f;
    } else if (param_1 == 0x333) {
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = -90.0f;
        _DAT_07ea9534 = 0.0f;
    } else if (param_1 == 0x342) {
        _DAT_07ea952c = 290.0f;
        _DAT_07ea9530 = 0.0f;
        _DAT_07ea9534 = 0.0f;
    } else if (param_1 == 0x3be) {
        _DAT_07ea952c = -90.0f;
        _DAT_07ea9530 = -20.0f;
        _DAT_07ea9534 = -20.0f;    // 0xc1a00000
    } else if (param_1 == 0x35c) {
        if (level == 0) { _DAT_07ea952c = 180.0f; _DAT_07ea9530 = 0.0f;   _DAT_07ea9534 = 0.0f; }
        else if (level == 1) { _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f; }
        else if (level == 2) { _DAT_07ea952c =  90.0f; _DAT_07ea9530 = 0.0f;  _DAT_07ea9534 = 0.0f; }
    } else if (param_1 == 0x3b8 || param_1 == 0x3ba || param_1 == 0x364) {
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f;
    } else if (param_1 == 0x3b9) {
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = 90.0f; _DAT_07ea9534 = 0.0f;
    } else if ((param_1 >= 0x33c && param_1 <= 0x34f) &&
               param_1 != 0x33e && param_1 != 0x33f) {
        _DAT_07ea952c = 360.0f; _DAT_07ea9530 = 0.0f; _DAT_07ea9534 = 0.0f;
    } else {
        // FIX 2026-05-01: IDA fall-through (verified disasm @0x4E16FC/1728):
        // Angle=270, Y=-10, Z=0. Antes Y=0 → items rendered upside-down.
        _DAT_07ea952c = 270.0f; _DAT_07ea9530 = -10.0f; _DAT_07ea9534 = 0.0f;
    }

    if (param_6 == 1)
        _DAT_07ea9530 = (float)DAT_05826e08 * _DAT_00552c00;

    _DAT_07ea9512 = modelType;
    if (modelType >= 0x270 && modelType < 0x310) {
        modelType = 390;
        _DAT_07ea9512 = 390;
    } else if (modelType == 0x35c) {
        if (level == 0) {
            param_1 = 947;
            modelType = 947;
            _DAT_07ea9512 = 947;
        } else if (level == 2) {
            param_1 = 948;
            modelType = 948;
            _DAT_07ea9512 = 948;
        }
    }

    // Set model height offset based on type range
    void *modelThis = (void *)(DAT_05828d58 + (int)modelType * 0xbc);

    // FIX 2026-05-02: faltaba `*(BYTE*)(modelThis + 0xa0) = 0`. IDA hace esto
    // al inicio para resetear el action index. Sin esto, el player pose usa el
    // action index del último frame del mundo (e.g. running) → body parts
    // posan con bones en pose de correr en lugar de idle.
    *(unsigned char *)((char *)modelThis + 0xa0) = 0;

    // BodyHeight (model + 0x84) — desplazamiento vertical por rango de modelo.
    // Valores exactos de IDA `RenderObjectScreen` (0x4E13A0), decodificados de
    // los literales del decompile:
    //   [624,656) HELM   → -1021313024 = 0xC31C0000 = -156.0
    //   [656,688) ARMOR  → -1027080192 = 0xC2C00000 =  -96.0
    //   [688,720) PANTS  → -1035468800 = 0xC2400000 =  -48.0
    //   [720,752) GLOVES → -1031012352 = 0xC2900000 =  -72.0
    //   resto (incl. BOOTS [752,784)) → 0
    //
    // 2026-08-11 FIX: esta copia (la VIVA) tenía DOS errores encadenados —
    //   (a) los cuatro valores eran los viejos -160/-100/-50/-70, y
    //   (b) PANTS y GLOVES estaban CRUZADOS (MODEL_PANTS=688, MODEL_GLOVES=720;
    //       el código le daba a GLOVES el valor de pants y viceversa).
    // Combinados: pants recibía -70 en vez de -48 (22 unidades DE MÁS hacia
    // abajo) y guantes -50 en vez de -72 (22 hacia arriba) — exactamente los
    // dos síntomas opuestos reportados. El comentario original ya listaba los
    // rangos de IDA bien y el código los contradecía.
    if (param_1 >= MODEL_HELM && param_1 < MODEL_HELM + 32)
        *(unsigned int *)((char *)modelThis + 0x84) = 0xC31C0000;   // -156.0
    else if (param_1 >= MODEL_ARMOR && param_1 < MODEL_ARMOR + 32)
        *(unsigned int *)((char *)modelThis + 0x84) = 0xC2C00000;   //  -96.0
    else if (param_1 >= MODEL_PANTS && param_1 < MODEL_PANTS + 32)
        *(unsigned int *)((char *)modelThis + 0x84) = 0xC2400000;   //  -48.0
    else if (param_1 >= MODEL_GLOVES && param_1 < MODEL_GLOVES + 32)
        *(unsigned int *)((char *)modelThis + 0x84) = 0xC2900000;   //  -72.0
    else
        *(unsigned int *)((char *)modelThis + 0x84) = 0;

    if (param_1 == MODEL_HELM + 65 || param_1 == MODEL_HELM + 70)
        outPos[0] += 0.04f;

    // Resolve per-type model scale (local_3bc)
    auto rawf = [](float f) -> DWORD {
        DWORD d;
        memcpy(&d, &f, sizeof(d));
        return d;
    };

    if (param_1 >= MODEL_HELM && param_1 < MODEL_HELM + 32) local_3bc = rawf(0.0039f);
    else if (param_1 >= MODEL_ARMOR && param_1 < MODEL_ARMOR + 32) local_3bc = rawf(0.0039f);
    else if (param_1 >= MODEL_GLOVES && param_1 < MODEL_GLOVES + 32) local_3bc = rawf(0.0038f);
    else if (param_1 >= MODEL_PANTS && param_1 < MODEL_PANTS + 32) local_3bc = rawf(0.0033f);
    else if (param_1 >= MODEL_BOOTS && param_1 < MODEL_BOOTS + 32) local_3bc = rawf(0.0032f);
    if (param_1 == MODEL_POTION + 45 || param_1 == MODEL_POTION + 49) local_3bc = rawf(0.003f);
    else if (param_1 >= MODEL_POTION + 46 && param_1 <= MODEL_POTION + 48) local_3bc = rawf(0.0025f);
    else if (param_1 == MODEL_POTION + 50) local_3bc = rawf(0.001f);
        else if (param_1 >= MODEL_POTION + 32 && param_1 <= MODEL_POTION + 34) {
            outPos[1] += 0.05f;
            local_3bc = rawf(0.002f);
        }
        else if (param_1 >= MODEL_EVENT + 21 && param_1 <= MODEL_EVENT + 23) {
            outPos[1] += (param_1 == MODEL_EVENT + 21) ? 0.08f : 0.06f;
            local_3bc = rawf(0.002f);
        }
    else if (param_1 == MODEL_POTION + 21) local_3bc = rawf(0.002f);
    else if (param_1 == MODEL_EVENT + 11) local_3bc = rawf(0.0015f);
    else if (param_1 == MODEL_HELPER + 4) local_3bc = rawf(0.0015f);
    else if (param_1 == MODEL_HELPER + 5) local_3bc = rawf(0.005f);
    else if (param_1 == MODEL_HELPER + 30 || param_1 == MODEL_EVENT + 16 || param_1 == MODEL_HELPER + 16) local_3bc = rawf(0.002f);
    else if (param_1 == MODEL_HELPER + 17 || param_1 == MODEL_HELPER + 18) local_3bc = rawf(0.0018f);
    else if (param_1 >= MODEL_HELPER + 43 && param_1 <= MODEL_HELPER + 45) local_3bc = rawf(0.0021f);
    else if (param_1 >= MODEL_HELPER + 46 && param_1 <= MODEL_HELPER + 48) local_3bc = rawf(0.0018f);
    else if (param_1 == MODEL_POTION + 53) local_3bc = rawf(0.00078f);
    else if (param_1 == MODEL_POTION + 54) local_3bc = rawf(0.0024f);
    else if (param_1 == MODEL_POTION + 58) local_3bc = rawf(0.0012f);
    else if (param_1 == MODEL_POTION + 59 || param_1 == MODEL_POTION + 60) local_3bc = rawf(0.0010f);
    else if (param_1 == MODEL_POTION + 61 || param_1 == MODEL_POTION + 62) local_3bc = rawf(0.0009f);
    else if (param_1 >= MODEL_POTION + 70 && param_1 <= MODEL_POTION + 71) local_3bc = rawf(0.0028f);
    else if (param_1 >= MODEL_POTION + 72 && param_1 <= MODEL_POTION + 77) local_3bc = rawf(0.0025f);
    else if (param_1 == MODEL_HELPER + 59) local_3bc = rawf(0.0008f);
    else if (param_1 >= MODEL_HELPER + 54 && param_1 <= MODEL_HELPER + 58) local_3bc = rawf(0.004f);
    else if (param_1 >= MODEL_POTION + 78 && param_1 <= MODEL_POTION + 82) local_3bc = rawf(0.0025f);
    else if (param_1 == MODEL_HELPER + 60) local_3bc = rawf(0.005f);
    else if (param_1 == MODEL_HELPER + 61) local_3bc = rawf(0.0018f);
    else if (param_1 == MODEL_POTION + 83) local_3bc = rawf(0.0009f);
    else if (param_1 == MODEL_POTION + 91) local_3bc = rawf(0.0034f);
    else if (param_1 == MODEL_POTION + 92 || param_1 == MODEL_POTION + 93 || param_1 == MODEL_POTION + 95) local_3bc = rawf(0.0024f);
    else if (param_1 == MODEL_POTION + 94) local_3bc = rawf(0.0022f);
    else if (param_1 == MODEL_POTION + 84) local_3bc = rawf(0.0031f);
    else if (param_1 == MODEL_POTION + 85) local_3bc = rawf(0.0044f);
    else if (param_1 == MODEL_POTION + 86) local_3bc = rawf(0.0031f);
    else if (param_1 == MODEL_POTION + 87) local_3bc = rawf(0.0061f);
    else if (param_1 == MODEL_POTION + 88 || param_1 == MODEL_POTION + 89 || param_1 == MODEL_POTION + 90) local_3bc = rawf(0.0035f);
    else if (param_1 == MODEL_HELPER + 62 || param_1 == MODEL_HELPER + 63) local_3bc = rawf(0.002f);
    else if (param_1 == MODEL_POTION + 97 || param_1 == MODEL_POTION + 98) local_3bc = rawf(0.003f);
    else if (param_1 == MODEL_POTION + 96) local_3bc = rawf(0.0028f);
    else if (param_1 == MODEL_HELPER + 0) local_3bc = rawf(0.0022f);
    else if (param_1 == MODEL_HELPER + 1 || param_1 == MODEL_HELPER + 2) local_3bc = rawf(0.0020f);
    else if (param_1 == MODEL_HELPER + 12 || param_1 == MODEL_HELPER + 13) local_3bc = rawf(0.0014f);
    else if (param_1 == MODEL_HELPER + 64) local_3bc = rawf(0.0005f);
    else if (param_1 == MODEL_HELPER + 65) local_3bc = rawf(0.0016f);
    else if (param_1 == MODEL_HELPER + 67) local_3bc = rawf(0.0015f);
    else if (param_1 == MODEL_HELPER + 80) local_3bc = rawf(0.0020f);
    else if (param_1 == MODEL_HELPER + 68 || param_1 == MODEL_HELPER + 76) local_3bc = rawf(0.0026f);
    else if (param_1 == MODEL_HELPER + 69) local_3bc = rawf(0.0023f);
    else if (param_1 == MODEL_HELPER + 70) local_3bc = rawf(0.0018f);
    else if (param_1 >= MODEL_HELPER + 71 && param_1 <= MODEL_HELPER + 75) local_3bc = rawf(0.0019f);
    else if (param_1 == MODEL_HELPER + 81 || param_1 == MODEL_HELPER + 82) local_3bc = rawf(0.0012f);
    else if (param_1 == MODEL_HELPER + 93 || param_1 == MODEL_HELPER + 94) local_3bc = rawf(0.0021f);

    if (local_3bc == 0) {
        if (param_1 == MODEL_HELPER + 97 || param_1 == MODEL_HELPER + 98 || param_1 == MODEL_POTION + 91) local_3bc = rawf(0.0028f);
        else if (param_1 == MODEL_HELPER + 99) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_POTION + 101) local_3bc = rawf(0.004f);
        else if (param_1 == MODEL_POTION + 102) local_3bc = rawf(0.005f);
        else if (param_1 >= MODEL_POTION + 103 && param_1 <= MODEL_POTION + 108) local_3bc = rawf(0.004f);
        else if (param_1 == MODEL_POTION + 109) local_3bc = rawf(0.003f);
        else if (param_1 == MODEL_POTION + 110 || param_1 == MODEL_POTION + 111) local_3bc = rawf(0.004f);
        else if (param_1 == MODEL_HELPER + 105) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_HELPER + 106) local_3bc = rawf(0.0015f);
        else if (param_1 == MODEL_HELPER + 107) local_3bc = rawf(0.0034f);
        else if (param_1 >= MODEL_HELPER + 109 && param_1 <= MODEL_HELPER + 112) local_3bc = rawf(0.0045f);
        else if (param_1 >= MODEL_HELPER + 113 && param_1 <= MODEL_HELPER + 115) local_3bc = rawf(0.0018f);
        else if (param_1 >= MODEL_POTION + 112 && param_1 <= MODEL_POTION + 113) local_3bc = rawf(0.0032f);
        else if (param_1 == MODEL_HELPER + 116) local_3bc = rawf(0.0021f);
        else if (param_1 >= MODEL_POTION + 114 && param_1 <= MODEL_POTION + 119) local_3bc = rawf(0.0038f);
        else if (param_1 == MODEL_POTION + 120) local_3bc = rawf(0.0038f);
        else if (param_1 == MODEL_HELPER + 121) local_3bc = rawf(0.0018f);
        else if (param_1 == MODEL_HELPER + 122) local_3bc = rawf(0.0033f);
        else if (param_1 == MODEL_HELPER + 123) local_3bc = rawf(0.0009f);
        else if (param_1 == MODEL_HELPER + 124) local_3bc = rawf(0.0018f);
        else if (param_1 >= MODEL_HELPER + 125 && param_1 <= MODEL_HELPER + 127) local_3bc = rawf(0.0013f);
        else if (param_1 == MODEL_HELPER + 128 || param_1 == MODEL_HELPER + 129) local_3bc = rawf(0.0035f);
        else if (param_1 == MODEL_HELPER + 130) local_3bc = rawf(0.0032f);
        else if (param_1 == MODEL_HELPER + 131) local_3bc = rawf(0.0033f);
        else if (param_1 == MODEL_HELPER + 132) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_HELPER + 133 || param_1 == MODEL_HELPER + 134) local_3bc = rawf(0.0033f);
        else if (param_1 >= MODEL_POTION + 126 && param_1 <= MODEL_POTION + 129) local_3bc = rawf(0.0038f);
        else if (param_1 >= MODEL_POTION + 130 && param_1 <= MODEL_POTION + 132) local_3bc = rawf(0.0038f);
        else if (param_1 == MODEL_POTION + 133) local_3bc = rawf(0.0030f);
        else if (param_1 >= MODEL_POTION + 134 && param_1 <= MODEL_POTION + 139) local_3bc = rawf(0.0050f);
        else if (param_1 == MODEL_POTION + 140) local_3bc = rawf(0.0026f);
        else if (param_1 >= MODEL_POTION + 145 && param_1 <= MODEL_POTION + 150) local_3bc = rawf(0.0018f);
        else if (param_1 >= MODEL_HELPER + 135 && param_1 <= MODEL_HELPER + 145) local_3bc = rawf(0.0010f);
        else if (param_1 == MODEL_SWORD + 19) { if ((int)param_2 >= 0) local_3bc = rawf(0.0025f); else { local_3bc = rawf(0.001f); param_2 = 0; } }
        else if (param_1 == MODEL_STAFF + 10) { if ((int)param_2 >= 0) local_3bc = rawf(0.0019f); else { local_3bc = rawf(0.001f); param_2 = 0; } }
        else if (param_1 == MODEL_BOW + 18) { if ((int)param_2 >= 0) local_3bc = rawf(0.0025f); else { local_3bc = rawf(0.0015f); param_2 = 0; } }
        else if (param_1 >= MODEL_MACE + 8 && param_1 <= MODEL_MACE + 11) local_3bc = rawf(0.003f);
        else if (param_1 == MODEL_MACE + 12) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_MACE + 18) local_3bc = rawf(0.0024f);
        else if (param_1 == MODEL_EVENT + 12) local_3bc = rawf(0.0012f);
        else if (param_1 == MODEL_EVENT + 13) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_EVENT + 14) local_3bc = rawf(0.0028f);
        else if (param_1 == MODEL_EVENT + 15) local_3bc = rawf(0.0023f);
        else if (param_1 >= MODEL_POTION + 22 && param_1 < MODEL_POTION + 25) local_3bc = rawf(0.0025f);
        else if (param_1 >= MODEL_POTION + 25 && param_1 < MODEL_POTION + 27) local_3bc = rawf(0.0028f);
        else if (param_1 == MODEL_POTION + 63) local_3bc = rawf(0.007f);
        else if (param_1 == MODEL_POTION + 99) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_POTION + 52) local_3bc = rawf(0.0014f);
        else if (param_1 == MODEL_BOW + 19) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_POTION + 41) local_3bc = rawf(0.0035f);
        else if (param_1 == MODEL_POTION + 42) local_3bc = rawf(0.005f);
        else if (param_1 == MODEL_POTION + 43) {
            outPos[1] -= 0.005f;
            local_3bc = rawf(0.0035f);
        }
        else if (param_1 == MODEL_POTION + 44) {
            outPos[1] -= 0.005f;
            local_3bc = rawf(0.004f);
        }
        else if (param_1 == MODEL_POTION + 7 || param_1 == MODEL_HELPER + 7 || param_1 == MODEL_HELPER + 11) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_EVENT + 18) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_HELPER + 38) local_3bc = rawf(0.0025f);
        else if (param_1 == MODEL_HELPER + 32) local_3bc = rawf(0.0019f);
        else if (param_1 == MODEL_HELPER + 33 || param_1 == MODEL_HELPER + 34 || param_1 == MODEL_HELPER + 35) local_3bc = rawf(0.004f);
        else if (param_1 == MODEL_HELPER + 36) local_3bc = rawf(0.007f);
        else if (param_1 == MODEL_HELPER + 37) local_3bc = rawf(0.005f);
        else if (param_1 == MODEL_BOW + 21) local_3bc = rawf(0.0022f);
        else if (param_1 == MODEL_BOW + 24) local_3bc = rawf(0.0023f);
        else if (param_1 == MODEL_HELPER + 49) local_3bc = rawf(0.0013f);
        else if (param_1 == MODEL_HELPER + 50 || param_1 == MODEL_HELPER + 51 || param_1 == MODEL_POTION + 64 || param_1 == MODEL_POTION + 65) local_3bc = rawf(0.003f);
        else if (param_1 == MODEL_POTION + 66 || param_1 == MODEL_POTION + 67) local_3bc = rawf(0.0035f);
        else if (param_1 == MODEL_POTION + 68) local_3bc = rawf(0.003f);
        else if (param_1 == MODEL_HELPER + 52 || param_1 == MODEL_HELPER + 53) local_3bc = rawf(0.005f);
        else if (param_1 == MODEL_HELPER + 66) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_SWORD + 24) local_3bc = rawf(0.0028f);
        else if (param_1 == MODEL_BOW + 22) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_BOW + 23) local_3bc = rawf(0.0032f);
        else if (param_1 == MODEL_HELPER + 14 || param_1 == MODEL_HELPER + 15) local_3bc = rawf(0.003f);
        else if (param_1 == MODEL_POTION + 100) local_3bc = rawf(0.0040f);
        else if (param_1 == MODEL_EVENT + 10) local_3bc = rawf(0.0010f);
        else if (param_1 >= MODEL_ETC + 19 && param_1 <= MODEL_ETC + 27) local_3bc = rawf(0.0023f);
        else if (param_1 == MODEL_WING + 6) local_3bc = rawf(0.0015f);
        else if (param_1 >= MODEL_WING + 32 && param_1 <= MODEL_WING + 34) {
            outPos[1] -= 0.05f;
            local_3bc = rawf(0.0010f);
        }
        else if (param_1 >= MODEL_WING + 60 && param_1 <= MODEL_WING + 65) local_3bc = rawf(0.0022f);
        else if (param_1 >= MODEL_WING + 70 && param_1 <= MODEL_WING + 74) local_3bc = rawf(0.0017f);
        else if (param_1 >= MODEL_WING + 100 && param_1 <= MODEL_WING + 129) local_3bc = rawf(0.0017f);
        else if (param_1 == MODEL_WING + 49 || param_1 == MODEL_WING + 50) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_WING + 130) local_3bc = rawf(0.0012f);
        else if (param_1 == MODEL_WING + 135) local_3bc = rawf(0.0012f);
        else if (param_1 == MODEL_WING + 30 || param_1 == MODEL_WING + 31 ||
                 (param_1 >= MODEL_WING + 136 && param_1 <= MODEL_WING + 143)) {
            local_3bc = rawf(0.0040f);
            if (param_1 == MODEL_WING + 142) {
                outPos[0] -= 0.05f;
                local_3bc = rawf(0.0030f);
            } else if (param_1 == MODEL_WING + 143) {
                outPos[0] -= 0.05f;
                local_3bc = rawf(0.0040f);
            } else if (param_1 == MODEL_WING + 137) {
                outPos[1] += 0.05f;
                local_3bc = rawf(0.0025f);
            } else if (param_1 == MODEL_WING + 141) {
                outPos[1] += 0.025f;
                local_3bc = rawf(0.0020f);
            } else if (param_1 == MODEL_WING + 138) {
                outPos[1] += 0.05f;
                local_3bc = rawf(0.0036f);
            } else if (param_1 == MODEL_WING + 136) {
                outPos[1] += 0.025f;
                local_3bc = rawf(0.0035f);
            } else if (param_1 == MODEL_WING + 139) {
                outPos[1] += 0.05f;
                local_3bc = rawf(0.0035f);
            } else if (param_1 == MODEL_WING + 140) {
                local_3bc = rawf(0.0050f);
            }
        }
        else if (param_1 >= MODEL_WING && param_1 < MODEL_WING + 6) local_3bc = rawf(0.0025f);
        else if (param_1 >= MODEL_WING && param_1 < MODEL_WING + 512) local_3bc = rawf(0.0020f);
        else if (param_1 == MODEL_POTION + 160 || param_1 == MODEL_POTION + 161) local_3bc = rawf(0.0010f);
        else if (param_1 >= MODEL_POTION && param_1 < MODEL_POTION + 512) local_3bc = rawf(0.0035f);
        else if (param_1 >= MODEL_SPEAR && param_1 < MODEL_SPEAR + 512) {
            if (param_1 == MODEL_SPEAR + 10) local_3bc = rawf(0.0018f);
            else if (param_1 == MODEL_SPEAR + 11) local_3bc = rawf(0.0025f);
            else local_3bc = rawf(0.0021f);
        }
        else if (param_1 >= MODEL_STAFF && param_1 < MODEL_STAFF + 512) {
            if (param_1 >= MODEL_STAFF + 14 && param_1 <= MODEL_STAFF + 20) local_3bc = rawf(0.0028f);
            else if (param_1 >= MODEL_STAFF + 21 && param_1 <= MODEL_STAFF + 29) local_3bc = rawf(0.004f);
            else if (param_1 == MODEL_STAFF + 33 || param_1 == MODEL_STAFF + 34) local_3bc = rawf(0.0028f);
            else local_3bc = rawf(0.0022f);
        }
        else if (param_1 == MODEL_BOW + 15) local_3bc = rawf(0.0011f);
        else if (param_1 == MODEL_BOW + 7) local_3bc = rawf(0.0012f);
        else if (param_1 == MODEL_EVENT + 6) local_3bc = rawf(0.0039f);
        else if (param_1 == MODEL_EVENT + 8) local_3bc = rawf(0.0015f);
        else if (param_1 == MODEL_EVENT + 9) local_3bc = rawf(0.0019f);
    }

    if (local_3bc == 0 && (param_1 == MODEL_HELM + 62 || param_1 == MODEL_HELM + 63 || param_1 == MODEL_HELM + 65 || param_1 == MODEL_HELM + 70)) local_3bc = rawf(0.0010f);
    else if (local_3bc == 0 && param_1 == MODEL_HELM + 31) local_3bc = rawf(0.0070f);
    else if (local_3bc == 0 && param_1 >= MODEL_HELM + 39 && param_1 <= MODEL_HELM + 44) local_3bc = rawf(0.0070f);
    else if (local_3bc == 0 && (param_1 == MODEL_ARMOR + 30 || param_1 == MODEL_ARMOR + 32)) local_3bc = rawf(0.0035f);
    else if (local_3bc == 0 && param_1 == MODEL_ARMOR + 29) local_3bc = rawf(0.0033f);
    else if (local_3bc == 0 && (param_1 == MODEL_ARMOR + 34 || param_1 == MODEL_ARMOR + 35 || param_1 == MODEL_GLOVES + 38)) local_3bc = rawf(0.0032f);
    else if (local_3bc == 0 && param_1 >= 0x270 && param_1 < 0x310) {
        if (param_1 < 0x290)       local_3bc = rawf(0.0039f);
        else if (param_1 < 0x2b0)  local_3bc = rawf(0.0039f);
        else if (param_1 < 0x2d0)  local_3bc = rawf(0.0033f);
        else if (param_1 < 0x2f0)  local_3bc = rawf(0.0038f);
        else                        local_3bc = rawf(0.0032f);
    } else if (local_3bc == 0 && param_1 == 0x316)  local_3bc = 0x3ac49ba6;
    else if (param_1 >= 0x310 && param_1 < 0x330) local_3bc = 0x3b03126f;
    else if (param_1 == 0x340)    local_3bc = 0x3b03126f;
    else if (param_1 == 0x341 || param_1 == 0x342) local_3bc = 0x3aebedfa;
    else if (param_1 == 0x365)    local_3bc = 0x3b03126f;
    else if (param_1 == 0x3be)    local_3bc = 0x3ac49ba6;
    else if (param_1 == 0x1a3)    { if ((int)param_2 >= 0) local_3bc = 0x3b23d70a; else { local_3bc = 0x3a83126f; param_2 = 0; } }
    else if (param_1 == 0x23a)    { if ((int)param_2 >= 0) local_3bc = 0x3af9096c; else { local_3bc = 0x3a83126f; param_2 = 0; } }
    else if (param_1 == 0x222)  {
        if ((int)param_2 < 0) {
            local_3bc = 0x3ac49ba6;
            param_2 = 0;
        } else {
            local_3bc = 0x3b378034;
        }
    }
    else if (param_1 >= 0x366 && param_1 <= 0x368) local_3bc = 0x3b23d70a;
    else if (param_1 == 0x369 || param_1 == 0x36a) local_3bc = 0x3b378034;
    else if (param_1 == 0x33e || param_1 == 0x33f) local_3bc = 0x3b449ba6;
    else if (param_1 >= 0x350 && param_1 < 0x370)  local_3bc = 0x3b656042;
    else if (param_1 >= 0x1f0 && param_1 < 0x210)  local_3bc = 0x3aebedfa;
    else if (param_1 >= 0x230 && param_1 < 0x250)  local_3bc = 0x3b102de0;
    else if (param_1 == 0x21f) local_3bc = 0x3a902de0;
    else if (param_1 == 0x217) local_3bc = 0x3a9d4952;
    else if (param_1 == 0x3b9) local_3bc = 0x3b7f9724;
    else if (param_1 == 0x3bb) local_3bc = 0x3ac49ba6;
    else if (param_1 == 0x3bc) local_3bc = 0x3af9096c;
    else if (param_1 == 0x3bd) local_3bc = 0x3a83126f;
    // 2026-08-11 FIX (items 3D chicos y descentrados): este `else` NO estaba
    // gateado, así que pisaba la escala ya calculada. Las líneas ~1881-1885
    // asignan bien la escala de los body-parts ([624,784): casco/armadura 0.0039,
    // guantes 0.0038, pants 0.0033, botas 0.0032), pero después vienen dos
    // cascadas más cuyas ramas llevan `local_3bc == 0 &&`; al no matchear
    // ninguna, la ejecución caía en este `else` final y clavaba TODO en el
    // default 0.0025. Medido en runtime (probe SCALE3D): los modelos 642/674/
    // 706/738/770 salían todos con 0.0025 en vez de su valor → items ~1.56×
    // más chicos y, como el ancla queda por debajo del centro de la casilla,
    // visualmente corridos. En IDA no puede pasar: la rama de [624,784) termina
    // con `goto LABEL_142`, saltándose el resto de la lógica de escala.
    else if (local_3bc == 0) local_3bc = 0x3b23d70a;

    // OpenMu/IDA 0x4E13A0 treats Book01..Book16 (models 880..895) as plain
    // item models: fixed pose (270,-10,0), 0.0025 scale, and Y rotation only
    // while the item is selected.  The reconstructed generic item table above
    // reuses this numeric range for potion-specific poses, which overwrote the
    // native Book state after it had been established.
    if (param_1 >= 880 && param_1 <= 895) {
        _DAT_07ea952c = 270.0f;
        _DAT_07ea9530 = param_6 == 1 ? (float)DAT_05826e08 * 0.45f : -10.0f;
        _DAT_07ea9534 = 0.0f;
        local_3bc = 0x3b23d70a; // 0.0025f
    }

    // Reset render state globals
    _DAT_07ea9618 = 0;
    _DAT_07ea961c = 0;
    DAT_07ea9616 = 0;

    // Pose model with BMD_Animation
    float angleArr[3] = { _DAT_07ea952c, _DAT_07ea9530, _DAT_07ea9534 };
    float headAngle[3] = { _DAT_07ea9538, 0.0f, 0.0f };
    FUN_00440060(modelThis, (int)&DAT_06970a9c, 0.0f, 0, 0, (unsigned int *)angleArr, headAngle, '\0', '\0');

    // Build stack entity and draw
    // entity_type at [+2], scale at [+0x0c], world_pos at [+0x10..+0x18]
    char ent[0x200];
    memset(ent, 0, sizeof(ent));
    *(short *)(ent + 2) = (short)param_1;
    *(DWORD *)(ent + 0x0c) = local_3bc;  // write raw float bits into entity+0x0c
    *(unsigned char *)(ent + 0x3d) = param_4; // preserve raw ExtOption for later render passes

    FUN_00502ba0((int)ent);

    // FIX 2026-05-01 (BUG REAL): ItemObjectAttribute (FUN_00502ba0) sobreescribe
    // ent[+0xC] con un valor default (0x3F4CCCCD = 0.8f para items en mundo).
    // En IDA, después de ItemObjectAttribute hay un `v16 = v11;` que reasigna el
    // scale (v16 = ent+0xC). Sin esa reasignación, RenderPartObject lee scale=0.8
    // (80%) en vez de los valores correctos (0.0025 para items, etc.) → modelos
    // renderizados a tamaño gigante.
    //
    // 2026-08-08 FIX (items 3D descentrados / más bajos que su casilla): el
    // mismo patrón, pero con la POSICIÓN. El port escribía outPos en
    // ent+0x10/0x14/0x18 ANTES de ItemObjectAttribute y no los reescribía.
    // En IDA (0x4E13A0 LABEL_142) el orden es:
    //     ItemObjectAttribute(&o);
    //     v16 = v11;              // o + 12 = Scale
    //     v17 = Position[0];      // o + 16
    //     v18 = Position[1];      // o + 20
    //     v19 = Position[2];      // o + 24
    //     v20 = 0;                // o + 0xDC
    //     v22 = 2;                // o + 0x1BC
    // o sea Position (y los otros dos campos) se escriben DESPUÉS, porque
    // ItemObjectAttribute también los pisa — igual que hace con el scale.
    // Verificado que el resto de la cadena (anchor, CreateScreenVector, lerp
    // 0.1, tabla de escala, viewport/perspectiva) ya era fiel, y que el DLL de
    // inyección usa los mismos multiplicadores con su tabla de ajuste VACÍA:
    // era este orden.
    *(DWORD *)(ent + 0x0c) = local_3bc;
    *(float *)(ent + 0x10) = outPos[0];
    *(float *)(ent + 0x14) = outPos[1];
    *(float *)(ent + 0x18) = outPos[2];
    *(int   *)(ent + 0xdc) = 0;
    *(unsigned char *)(ent + 0x1bc) = 2;


    float light[3] = { 1.0f, 1.0f, 1.0f };
    // FIX 2026-05-01: param_5 era 0.0f → fallaba el gate
    // `if (_DAT_005524f8 < param_5)` (threshold = 0.01) y RenderPartObject
    // entero se saltaba → items del inventario no se veían. Pasamos 1.0f
    // (distancia "siempre visible") como hace el path de mundo (que pasa
    // entity scale/distance).

    // FIX confirmado 2026-05-01: el bug de "items rendering huge" venía de
    // ItemObjectAttribute(FUN_00502ba0) sobreescribiendo ent[+0xc] con 0.8f.
    // La reasignación post-ItemObjectAttribute arreglo el problema.

    FUN_00505a10((int)ent, param_1, 0, light, 1.0f, param_2, param_3, '\x01', 1, '\x01', 0, 2);
    (void)param_6;
}



// FUN_0050e2c0 @ 0x0050E2C0 — Parse_NextToken
// Text/config file tokenizer. Reads from DAT_083a40fc; output to DAT_083a3ff4.
// Returns: 0=string/identifier, 1=numeric, 2=EOF, or single-char code for {,};#.
// Skips whitespace and '//' line comments. Stores token type in _DAT_083a40f4,
// numeric value in _DAT_083a40f8.
int __cdecl FUN_0050e2c0(void) {
    DAT_083a3ff4[0] = '\0';
    int c = fgetc(DAT_083a40fc);
    // Skip whitespace and '//' line comments
    while (c != -1) {
        if (c == '/') {
            c = fgetc(DAT_083a40fc);
            if (c == '/') {
                do { c = fgetc(DAT_083a40fc); } while (c != '\n' && c != -1);
                if (c != -1) c = fgetc(DAT_083a40fc);
                continue;
            }
            // single '/' — not a comment, handle below
            break;
        }
        if (!isspace(c)) break;
        c = fgetc(DAT_083a40fc);
    }
    if (c == -1) return 2;

    switch (c) {
    case '"': {
        char *p = DAT_083a3ff4;
        while ((c = fgetc(DAT_083a40fc)) != -1 && c != '"')
            *p++ = (char)c;
        *p = '\0';
        _DAT_083a40f4 = 0;
        return 0;
    }
    case '#': _DAT_083a40f4 = '#'; return '#';
    case ',': _DAT_083a40f4 = ','; return ',';
    case ';': _DAT_083a40f4 = ';'; return ';';
    case '{': _DAT_083a40f4 = '{'; return '{';
    case '}': _DAT_083a40f4 = '}'; return '}';
    }
    // Numeric: digit, '-', or '.'
    if (isdigit(c) || c == '-' || c == '.') {
        char buf[100];
        char *p = buf;
        *p++ = (char)c;
        int nc;
        while ((nc = fgetc(DAT_083a40fc)) != -1 && (isdigit(nc) || nc == '.' || nc == '-'))
            *p++ = (char)nc;
        *p = '\0';
        if (nc != -1) ungetc(nc, DAT_083a40fc);
        _DAT_083a40f8 = (float)atof(buf);
        _DAT_083a40f4 = 1;
        return 1;
    }
    // Alpha or underscore → identifier
    if (!isalpha(c)) {
        _DAT_083a40f4 = '<';
        return '<';
    }
    char *p = DAT_083a3ff4;
    *p++ = (char)c;
    int nc;
    while ((nc = fgetc(DAT_083a40fc)) != -1 && (isalnum(nc) || nc == '.' || nc == '_'))
        *p++ = (char)nc;
    if (nc != -1) ungetc(nc, DAT_083a40fc);
    *p = '\0';
    _DAT_083a40f4 = 0;
    return 0;
}

// FUN_00456590 @ 0x00456590 — Entity_SpawnBoneEffect(entity, effectType, scale, bone, x, flags, yOff)
// Transforms an offset vector through the entity's bone matrix, then spawns a particle
// effect at the resulting world position with a pulsing light color.
void* __cdecl FUN_00456590(int entity, int effectType, float scale, int bone, float x, int flags, float yOff)
{
    // offset vector at bone position + x/yOff
    // BUG-FIX 2026-08-18 (A): el vector de offset se pasaba desde `&offset[3]`,
    // o sea leia offset[3],[4],[5] - dos floats FUERA del array. IDA sub_456590
    // arma v9[0]=a5, v9[1]=a6, v9[2]=a7 y pasa v9, el indice 0.
    float offset[3];
    offset[0] = x;
    offset[1] = (float)flags;   // param_6 (undefined4 packed as float here)
    offset[2] = yOff;

    float outPos[3];
    void *modelPtr = (void *)(DAT_05828d58 + *(short *)(entity + 2) * 0xbc);
    float *boneMat = (float *)(*(int *)(entity + 0x114) + bone * 0x30);
    FUN_004409a0(modelPtr, boneMat, offset, outPos, '\x01');

    // Pulsing light: sin(animTick * period) * amp + base
    float sinVal = (float)fsin((double)DAT_05826e08 * (double)_DAT_005528e0);
    float light[3];
    light[0] = sinVal * _DAT_005528b8 + _DAT_00552928;
    light[1] = light[0] * _DAT_00552534;
    light[2] = light[0] * _DAT_005528b4;

    // BUG-FIX 2026-08-18 (B): el 3er argumento de CreateSprite es la ESCALA y se
    // pasaba (float)effectType - o sea escala 1191 para el tipo 1191. IDA
    // sub_456590: CreateSprite(Type, Position, Scale, Light, Owner, 0.0, 0).
    // Con eso el quad media 152448 unidades y, pintado con Light=(v, v*0.6,
    // v*0.4) = salmon, tapaba la pantalla entera en Atlans.
    FUN_004795c0((unsigned short)effectType, outPos, scale, light, entity, 0.0f, 0);
    return (void *)entity;
}

// FUN_00456650 @ 0x00456650 — Entity_SpawnBoneRangeEffect(entity, bone1, bone2, scale)
// Spawns beam effects between two bones of the entity, and writes bone2 world pos to entity+0x40.
void* __cdecl FUN_00456650(int entity, int bone1, int bone2, float scale)
{
    float sinVal = (float)fsin((double)DAT_05826e08 * (double)_DAT_005528e0);
    void *modelPtr = (void *)(DAT_05828d58 + *(short *)(entity + 2) * 0xbc);

    float light0 = sinVal * _DAT_005528b8 + _DAT_00552530;
    float light[3] = { light0, light0, light0 };

    // Bone1: offset {5, 0, 0} → spawn at world pos
    float vec1[4] = { 5.0f, 0.0f, 0.0f, 0.0f };
    float outPos1[4];
    FUN_004409a0(modelPtr, (float *)(bone1 * 0x30 + *(int *)(entity + 0x114)), vec1, outPos1, '\x01');
    FUN_004795c0(0x4d1, outPos1, scale, light, 0, 0.0f, 0);

    // Bone2: offset {-5, 0, 0} → spawn at world pos, write result to entity+0x40
    vec1[0] = -5.0f;
    float outPos2[4];
    FUN_004409a0(modelPtr, (float *)(bone2 * 0x30 + *(int *)(entity + 0x114)), vec1, outPos2, '\x01');
    FUN_004795c0(0x4d1, outPos2, scale, light, 0, 0.0f, 0);

    *(float *)(entity + 0x40) = outPos2[0];
    *(float *)(entity + 0x44) = outPos2[1];
    *(float *)(entity + 0x48) = outPos2[2];
    return (void *)entity;
}


// FUN_005403a0 @ 0x005403a0 — Pipe_WriteFile
// Writes a formatted message to the named pipe handle (lpTargetHandle_00563b58).
void __cdecl FUN_005403a0(LPCVOID param_1, int param_2, CHAR *param_3)
{
    if (lpTargetHandle_00563b58 == NULL || lpTargetHandle_00563b58 == INVALID_HANDLE_VALUE)
        return;
    int len = crt_sprintf(lpBuffer_083bbb60, "%p:%d:%s", param_1, param_2,
                          param_3 ? param_3 : "");
    if (len <= 0) return;
    DWORD dwWritten = 0;
    WriteFile(lpTargetHandle_00563b58, lpBuffer_083bbb60, (DWORD)(len + 1), &dwWritten, NULL);
    DAT_083bbb64 = dwWritten;
}


// FUN_005404a0 @ 0x005404a0 — Pipe_Write (thin wrapper)
void __cdecl FUN_005404a0(LPCVOID param_1, int param_2, CHAR *param_3)
{
    FUN_005403a0(param_1, param_2, param_3);
}


// FUN_0053ed30 @ 0x0053ed30 — Pipe_SetTarget
// Copies name into manager+0x2f0, then sends opcode 0x613 via Pipe_Write.
void __cdecl FUN_0053ed30(void *mgr, CHAR *name)
{
    if (mgr == NULL || name == NULL) return;
    char *dst = (char *)mgr + 0x2f0;
    int len = (int)strlen(name);
    int words = len / 2;
    for (int i = 0; i < words; i++)
        ((short *)dst)[i] = ((short *)name)[i];
    if (len & 1)
        dst[len - 1] = name[len - 1];
    FUN_005404a0((LPCVOID)0x8b1, 0x613, name);
}


