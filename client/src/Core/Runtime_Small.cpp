// Runtime_Small.cpp
//
// Extracted from stubs_bulk_small.cpp (B3: stubs.cpp lines 14828-16129).
//
// BATCH: Small unmapped functions (1-60 bytes), sorted by size.
// Decompiled from Ghidra in bulk — closes ~3KB of the binary gap.
// ~80 functions implemented (1-52 bytes range).
//
// These are mostly:
//   - CRT/STL/runtime stubs (constructors, destructors, accessors)
//   - GameGuard / anti-tamper trampolines
//   - Tiny hash table / refcount helpers
//   - One-line getters/setters

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl Xor_ConvertBlock(BYTE *lpBuffer, int iSize, int iKey);

// IDA `qmemcpy` is just memcpy with size_t hint.
#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif

// ═══════════════════════════════════════════════════════════════════════════════
// BATCH: Small unmapped functions (1-60 bytes), sorted by size
// Decompiled from Ghidra in bulk — closes ~3KB of the binary gap
// ═══════════════════════════════════════════════════════════════════════════════

// Forward declarations for functions defined later in this batch
static void __cdecl FUN_00405320_impl(int param_1);
// (FUN_00405290_impl_fwd: forward decl muerto removido — la def real es
//  FUN_00405290_impl @ ~line 13553, __cdecl, sin necesidad de fwd separado)

// ── 1-byte: nop ──────────────────────────────────────────────────────────────

// FUN_0053ad80 — see proper definition at ~line 15177 (codec noop start_pass)

// ── 3-byte: stub returning 0 ────────────────────────────────────────────────

// FillBuffer @ 0x00404A00 (3 bytes) — always returns S_OK (unused sound fill)
HRESULT __cdecl FillBuffer(int Buffer, int MaxChannel, BOOL Enable) { return 0; }



// ── 7-byte: vtable setters ──────────────────────────────────────────────────

// FUN_00403310 @ 0x00403310 (7 bytes)
void __fastcall FUN_00403310(int *param_1) { *param_1 = (int)&PTR_LAB_00552460; }

// FUN_00407970 @ 0x00407970 (7 bytes)
// FUN_00407970 (IDA-activated, was Ghidra stub)
void __cdecl FUN_00407970(void *_this)
{
  extern void *g_ClothNodeVTable[1];
  *(DWORD *)_this = (DWORD)g_ClothNodeVTable;
}


// FUN_00407de0 @ 0x00407DE0 (7 bytes)
// FUN_00407de0 (IDA-activated, was Ghidra stub)
void __cdecl FUN_00407de0(DWORD *_this)
{
  *_this = (DWORD)&DAT_00552508;
}


// FUN_0040f690 @ 0x0040F690 (7 bytes)
void __fastcall FUN_0040f690(void *This) { *(int *)This = (int)&PTR_LAB_005527e4; }

// FUN_0040f680 @ 0x0040F680 (9 bytes) — same vtable as 0040f690 (duplicate entry)
void __fastcall FUN_0040f680(void *This) { *(int *)This = (int)&PTR_LAB_005527e4; }














// FUN_00407ec0 @ 0x00407EC0 (11 bytes)
// FUN_00407ec0 (IDA-activated, was Ghidra stub)
void __cdecl FUN_00407ec0(DWORD *_this)
{
  extern void *g_ClothAnchorVTable[3];
  *_this = (DWORD)g_ClothAnchorVTable;
  FUN_00407de0(_this);
}



// FUN_0040a6e0 @ 0x0040A6E0 (11 bytes)
void __fastcall FUN_0040a6e0(void *This) {
    *(int *)This = (int)&PTR_LAB_005524e8;
    FUN_00409f10(This);
}









// ── 13-byte: named game functions ───────────────────────────────────────────

// InitPartyList @ 0x004E54F0 (13 bytes) — reinicia el estado de party
void __cdecl InitPartyList(void) {
    // IDA: FUN_004E54F0. Los globales de scripts NPC en 07E11E50/54 no están
    // relacionados; se reinicia el estado activo de party usado por paquetes/UI/combate.
    PartyNumber = 0;
    PartyKey = 0;
}

// CenterMouseX @ 0x005110D0 (13 bytes)
void __cdecl CenterMouseX(void) { MouseX = (unsigned int)WindowWidth >> 1; }

// CenterMouseY @ 0x005110F0 (13 bytes)
void __cdecl CenterMouseY(void) { MouseY = (unsigned int)WindowHeight >> 1; }


// ── 15-byte ─────────────────────────────────────────────────────────────────

// CErrorReport::AddSeparator @ 0x00405580 (15 bytes)
void __fastcall CErrorReport__AddSeparator(DWORD This) {
    CErrorReport__Write(This, (char *)"---------------------------------------");
}





// ClearNotice @ 0x0047FAC0 (17 bytes) — zero-fill notice text array
void __cdecl ClearNotice(void) { memset(DAT_083a2370 + 0x10, 0, 0x630); }

// ── 18-byte ─────────────────────────────────────────────────────────────────

// FUN_00406de0 @ 0x00406DE0 (18 bytes) — HashTable init (set vtable + zero fields)
void __fastcall FUN_00406de0_impl(int *param_1) {
    *param_1 = (int)&PTR_FUN_005524d8;
    param_1[3] = 0;
    param_1[1] = 0;
    param_1[2] = 0;
}


// FUN_0053d580 @ 0x0053D580 (18 bytes) — GameGuard query wrapper
int __stdcall FUN_0053d580(void) {
    if (lpParameter == NULL) return 0;
    return FUN_0053ea90(lpParameter);
}



// ── 19-byte ─────────────────────────────────────────────────────────────────

// FUN_00405290 @ 0x00405290 (19 bytes) — CErrorReport field init
static void __cdecl FUN_00405290_impl(int param_1) {
    *(int *)(param_1 + 4) = -1;  // 0xffffffff
    *(BYTE *)(param_1 + 8) = 0;
    *(int *)(param_1 + 0x10c) = 0;
}

// FUN_0047cfe0 @ 0x0047CFE0 (19 bytes) — item field accessor (word at +0x14)
short __cdecl FUN_0047cfe0(short *param_1) {
    if (*param_1 == -1) return 0;
    return param_1[10];  // offset +0x14
}

// FUN_0047d000 @ 0x0047D000 (19 bytes) — item field accessor (word at +0x18)
short __cdecl FUN_0047d000(short *param_1) {
    if (*param_1 == -1) return 0;
    return param_1[0xc];  // offset +0x18
}

// PlusSpecial @ 0x0047CEF0 (68 bytes) — port FIEL desde IDA (2026-05-02).
// Iterates an ITEM's special properties array. For each special with ID
// matching `Special`, adds its value (at Item[+45+i]) to *Value (only if
// Item[+26] = Option1 byte is set).
//
// ITEM layout used:
//   Item[+0]  WORD  Type (0xFFFF = empty)
//   Item[+26] BYTE  Option1 (0 = no special active)
//   Item[+36] BYTE  numSpecials (count of properties)
//   Item[+37+i] BYTE specialID[i]
//   Item[+45+i] BYTE specialValue[i]
extern "C" void __cdecl PlusSpecial(unsigned short *Value, int Special, DWORD Item)
{
    if (*(unsigned short*)Item == 0xFFFF) return;
    int count = *(unsigned char*)(Item + 36);
    for (int i = 0; i < count; ++i) {
        if (*(unsigned char*)(Item + i + 37) == (unsigned int)Special) {
            if (*(unsigned char*)(Item + 26)) {
                *Value += *(unsigned char*)(i + Item + 45);
            }
        }
    }
}



// CalcDurabilityPercent @ 0x0047D330 (159 bytes) — port FIEL desde IDA.
// Returns durability degradation factor for stat reduction:
//   1.0 - (dur / adjusted_max_dur)
//   Where adjusted_max_dur = maxDur + level_bonus + option_bonus.
// Returns:
//   > 80% damaged: 0.50
//   > 70% damaged: 0.30
//   > 50% damaged: 0.20
//   else: 0.00
extern "C" float __cdecl CalcDurabilityPercent(BYTE dur, BYTE maxDur, int Level, int option) {
    int maxDurability = maxDur;
    int levelBonus = (Level >> 3) & 0xF;
    for (int i = 0; i < levelBonus; ++i) {
        if (i < 4) ++maxDurability;
        else        maxDurability += 2;
    }
    if ((option & 0x3F) != 0) maxDurability += 15;
    double durP = 1.0 - (double)dur / (double)maxDurability;
    if (durP > 0.8)  return 0.5f;
    if (durP > 0.7)  return 0.3f;
    if (durP <= 0.5) return 0.0f;
    return 0.2f;
}

// sub_47CF40 @ 0x0047CF40 (104 bytes) — Stats_ApplyBonus2 (percentage).
// Like PlusSpecial but multiplies: *a1 += a4 * (*a1) / 100 if any special
// matches `a2`. Used for percentage-based stat bonuses (defense %, etc).
int __cdecl FUN_0047cf40(short *a1, int a2, int a3, unsigned short a4) {
    int result = a3;
    if (*(unsigned short*)a3 == 0xFFFF) return result;
    int count = *(unsigned char*)(a3 + 36);
    for (int i = 0; i < count; ++i) {
        if (*(unsigned char*)(a3 + i + 37) == (unsigned int)a2) {
            *(unsigned short*)a1 += (unsigned short)(a4 * (unsigned short)*a1 / 100);
        }
    }
    return result;
}

// sub_47D410 @ 0x0047D410 (1739 bytes) — Stats_CalcBase: attack damage min/max.
// Computes two-hand attack ranges at this[+60..+66]:
//   [+60] AttackMin (left/main)
//   [+62] AttackMax (left/main)
//   [+64] AttackMin (right/off)
//   [+66] AttackMax (right/off)
// Class-based base formula from Str/Vit/Energy at this[+20..+26]:
//   class 2 (Wiz): base = (Str+Dex)/7, max = (Str+Dex)/4
//   class 1 (DK):  base = Str/6,        max = Str/4
//   class 3 (MG):  base = Str/6 + Vit/12, max = Vit/8 + Str/4
//   default:       base = Str>>3,       max = Str>>2
// Weapon-type override (bow/staff/wand types 128..134, 136..142, 145, 144..159):
//   base = Vit/7, max = Vit/4
// Then per-slot bonuses (Wings/WeaponL/WeaponR/Ring1) with durability scaling.
// Bow+arrows + crossbow+bolts synergy: special level boost based on arrow type.
// dword_7E91388/pPickedItem preview path skipped (hover-time UI).
int __fastcall FUN_0047d410(int a1) {
    // GUARDA 2026-07-19 (CRASH 0xC0000005 @ +0xA3): se validaba `ca`
    // (CharacterAttribute) pero NO `a1` (CharacterMachine). Abajo se hace
    // `*(short*)(a1 + 536)` / `(a1 + 604)` (slots de arma) sin chequear, así que
    // con a1 nulo o basura reventaba. Se disparaba al abrir el inventario /
    // mostrar el tooltip de un item, que fuerza un recálculo de stats.
    if (a1 == 0 || (uintptr_t)a1 < 0x100000 || (uintptr_t)a1 >= 0x80000000) return 0;

    DWORD ca = (DWORD)DAT_07cf1ff4;
    if (ca == 0) return 0;
    if ((uintptr_t)ca < 0x100000 || (uintptr_t)ca >= 0x80000000) return 0;
    char* charAttr = (char*)(uintptr_t)ca;

    // GUARDA 2026-07-20 (CRASH 0xC0000005 read @ 0x612A = 388*0x40 + 42):
    // esta funcion indexaba `ItemAttribute[tipo]` leyendo DAT_07d78068 CRUDO,
    // ignorando el helper ItemAttribute_Base() de globals.h que existe
    // justamente porque ese puntero se pisa a 0x1 en runtime (ver la nota de
    // 2026-05-08 sobre tooltip / RenderBrokenItem). Con la tabla en 0, el
    // indexado daba una direccion chica y reventaba.
    // Se disparaba al cerrar el inventario con un tooltip de item abierto,
    // porque ese camino fuerza un recalculo de stats.
    ITEM_ATTRIBUTE* ITEM_ATTR = (ITEM_ATTRIBUTE*)(uintptr_t)ItemAttribute_Base();
    if (ITEM_ATTR == nullptr) return 0;

    DWORD wp_L = (DWORD)(uintptr_t)(a1 + 536);
    DWORD wp_R = (DWORD)(uintptr_t)(a1 + 604);
    DWORD ring1 = (DWORD)(uintptr_t)(a1 + 1148);

    // Skip hover-preview override (dword_7E91388 path)

    unsigned short *atkMinL = (unsigned short*)(a1 + 60);
    unsigned short *atkMaxL = (unsigned short*)(a1 + 62);
    unsigned short *atkMinR = (unsigned short*)(a1 + 64);
    unsigned short *atkMaxR = (unsigned short*)(a1 + 66);

    short tL = *(short*)wp_L;
    short tR = *(short*)wp_R;
    bool useBowFormula = false;
    if ((tL >= 136 && tL < 143 && *(unsigned char*)(wp_L + 26))
        || (((tR >= 128 && tR < 135) || tR == 145) && *(unsigned char*)(wp_R + 26))
        || (tL >= 144 && tL < 160 && *(unsigned char*)(wp_L + 26))) {
        useBowFormula = true;
    }
    if (useBowFormula) {
        unsigned short vit = *(unsigned short*)(a1 + 22);
        *atkMinL = vit / 7;
        *atkMaxL = vit >> 2;
        *atkMinR = vit / 7;
        *atkMaxR = vit >> 2;
    } else {
        int charClass = *(unsigned char*)(charAttr + 11) & 7;
        unsigned short str = *(unsigned short*)(a1 + 20);
        unsigned short vit = *(unsigned short*)(a1 + 22);
        unsigned short eng = *(unsigned short*)(a1 + 26);
        unsigned short baseMin, baseMax;
        if (charClass == 2) {
            unsigned short total = str + vit;
            baseMin = total / 7;
            baseMax = total / 4;
        } else if (charClass == 1) {
            baseMin = str / 6;
            baseMax = str / 4;
        } else if (charClass == 3) {
            baseMin = (unsigned short)(str / 6 + eng / 12);
            baseMax = (unsigned short)(eng / 8 + str / 4);
        } else {
            baseMin = (unsigned short)(str >> 3);
            baseMax = (unsigned short)(str >> 2);
        }
        *atkMinL = baseMin;  *atkMaxL = baseMax;
        *atkMinR = baseMin;  *atkMaxR = baseMax;
    }

    // Wings (this+1012) attack bonus, special 60, durability scaled
    DWORD wings = (DWORD)(uintptr_t)(a1 + 1012);
    {
        // OJO: el binario original NO chequea 0xFFFF en el slot de alas (si lo
        // hace en los 3 slots de abajo).  Alla es inofensivo porque
        // ItemAttribute[-1] cae dentro de su propio segmento de datos; aca no.
        short wType = *(short*)(a1 + 1012);
        if (wType < 0 || wType >= 1024) goto skip_wings;
        {
        ITEM_ATTRIBUTE* attr = &ITEM_ATTR[wType];
        float durP = CalcDurabilityPercent(*(unsigned char*)(a1 + 1038),
                                           attr->Durability,
                                           *(int*)(a1 + 1016), 0);
        unsigned short minB = 0, maxB = 0;
        PlusSpecial(&minB, 60, wings);
        PlusSpecial(&maxB, 60, wings);
        unsigned short minDed = (unsigned short)((double)minB * (double)durP);
        unsigned short maxDed = (unsigned short)((double)maxB * (double)durP);
        unsigned short minNet = minB - minDed;
        unsigned short maxNet = maxB - maxDed;
        *atkMaxL += maxNet;  *atkMaxR += maxNet;
        *atkMinL += minNet;  *atkMinR += minNet;
        }
skip_wings: ;
    }

    // WeaponL (slot 0)
    if (*(unsigned short*)wp_L != 0xFFFF) {
        unsigned char dur = *(unsigned char*)(wp_L + 26);
        if (dur != 0) {
            short sType = *(short*)wp_L;
            if (sType < 0 || sType >= 1024) sType = 0;
            ITEM_ATTRIBUTE* attr = &ITEM_ATTR[sType];
            float durP = CalcDurabilityPercent(dur, attr->Durability,
                                               *(int*)(wp_L + 4),
                                               *(unsigned char*)(wp_L + 27));
            unsigned short minB = *(unsigned short*)(wp_L + 12);
            unsigned short maxB = *(unsigned short*)(wp_L + 14);
            PlusSpecial(&minB, 60, wp_L);
            PlusSpecial(&maxB, 60, wp_L);
            unsigned short minNet = minB - (unsigned short)((double)minB * (double)durP);
            unsigned short maxNet = maxB - (unsigned short)((double)maxB * (double)durP);
            short t = *(short*)wp_L;
            // Two-hand weapons (160..192): half the bonus to L hand
            if (t >= 160 && t <= 192) {
                *atkMinL += minNet >> 1;
                *atkMaxL += maxNet >> 1;
            } else {
                *atkMinL += minNet;
                *atkMaxL += maxNet;
            }
            PlusSpecial(atkMinL, 73, wp_L);
            PlusSpecial(atkMaxL, 73, wp_L);
            FUN_0047cf40((short*)atkMinL, 74, (int)wp_L, 2);
            FUN_0047cf40((short*)atkMaxL, 74, (int)wp_L, 2);
        }
    }

    // WeaponR (slot 1)
    if (*(unsigned short*)wp_R != 0xFFFF) {
        unsigned char dur = *(unsigned char*)(wp_R + 26);
        if (dur != 0) {
            short sType = *(short*)wp_R;
            if (sType < 0 || sType >= 1024) sType = 0;
            ITEM_ATTRIBUTE* attr = &ITEM_ATTR[sType];
            float durP = CalcDurabilityPercent(dur, attr->Durability,
                                               *(int*)(wp_R + 4),
                                               *(unsigned char*)(wp_R + 27));
            unsigned short minB = *(unsigned short*)(wp_R + 12);
            unsigned short maxB = *(unsigned short*)(wp_R + 14);
            PlusSpecial(&minB, 60, wp_R);
            PlusSpecial(&maxB, 60, wp_R);
            unsigned short minNet = minB - (unsigned short)((double)minB * (double)durP);
            unsigned short maxNet = maxB - (unsigned short)((double)maxB * (double)durP);
            short t = *(short*)wp_R;
            if (t >= 160 && t <= 192) {
                *atkMinR += minNet >> 1;
                *atkMaxR += maxNet >> 1;
            } else {
                *atkMinR += minNet;
                *atkMaxR += maxNet;
            }
            PlusSpecial(atkMinR, 73, wp_R);
            PlusSpecial(atkMaxR, 73, wp_R);
            FUN_0047cf40((short*)atkMinR, 74, (int)wp_R, 2);
            FUN_0047cf40((short*)atkMaxR, 74, (int)wp_R, 2);
        }
    }

    // Ring1 (this+1148) — applies to both hands
    if (*(unsigned short*)ring1 != 0xFFFF && *(unsigned char*)(ring1 + 26)) {
        PlusSpecial(atkMinL, 73, ring1);
        PlusSpecial(atkMaxL, 73, ring1);
        FUN_0047cf40((short*)atkMinL, 74, (int)ring1, 2);
        FUN_0047cf40((short*)atkMaxL, 74, (int)ring1, 2);
        PlusSpecial(atkMinR, 73, ring1);
        PlusSpecial(atkMaxR, 73, ring1);
        FUN_0047cf40((short*)atkMinR, 74, (int)ring1, 2);
        FUN_0047cf40((short*)atkMaxR, 74, (int)ring1, 2);
    }

    // Bow + arrows synergy (Type 135 with arrows option) / Crossbow + bolts (Type 143)
    short tWL = *(short*)wp_L;
    short tWR = *(short*)wp_R;
    if (tWL >= 128 && tWL < 160 && tWR >= 128 && tWR < 160) {
        int levelL = (*(int*)(wp_L + 4) >> 3) & 0xF;
        int levelR = (*(int*)(wp_R + 4) >> 3) & 0xF;
        if (tWL == 135 && levelR != 0) {
            // Bow + Arrows: boost L damage by (2*levelR + 1)% + 1
            float boost = (2.0f * levelR + 1.0f) * 0.01f;
            *atkMinL += (unsigned short)((double)*atkMinL * (double)boost + 1.0);
            *atkMaxL += (unsigned short)((double)*atkMaxL * (double)boost + 1.0);
        } else if (tWR == 143 && levelL >= 1) {
            // Crossbow + Bolts: boost R damage similarly
            float boost = (2.0f * levelL + 1.0f) * 0.01f;
            *atkMinR += (unsigned short)((double)*atkMinR * (double)boost + 1.0);
            *atkMaxR += (unsigned short)((double)*atkMaxR * (double)boost + 1.0);
        }
    }
    return *atkMinL;
}






// ClearWhisperID @ 0x004804D0 (19 bytes) — zero 28 entries × 4 bytes = 0x6e bytes + 2
void __cdecl ClearWhisperID(void) { memset(WhisperRegistID, 0, 0x6e); }

// FUN_0040f650 @ 0x0040F650 (20 bytes) — thiscall: call vtable method on sub-object
void __fastcall FUN_0040f650(int ecx, int /*edx*/, int param_1) {
    if (param_1 != 0) {
        // Call release method: (*(this+4))->vtable[2](param_1)
        typedef void (__cdecl *ReleaseFunc)(int);
        int *pObj = *(int **)(ecx + 4);
        ReleaseFunc fn = (ReleaseFunc)(*(int *)(*pObj + 8));
        fn(param_1);
    }
}

// ── 22-byte ─────────────────────────────────────────────────────────────────

// FUN_00405320 @ 0x00405320 (22 bytes) — close handle + field init
static void __cdecl FUN_00405320_impl(int param_1) {
    CloseHandle(*(HANDLE *)(param_1 + 4));
    FUN_00405290_impl(param_1);
}

// FUN_00406d20 @ 0x00406D20 (22 bytes) — initialize hash table fields
void __fastcall FUN_00406d20(int param_1) {
    *(int *)(param_1 + 0x1c) = 0x83;
    *(int *)(param_1 + 0xc) = 0;
    *(int *)(param_1 + 4) = 0;
    *(int *)(param_1 + 8) = 0;
    *(int *)(param_1 + 0x18) = 0;
}







// FUN_004cbdd0 @ 0x004CBDD0 (29 bytes) — clear one of the item-slot tables.
//
// BUG-FIX 2026-05-03: original port used absolute source-binary addresses
// (`0x07e11fb0` literal start, `0x7e12830` literal end) — in our build the
// linker places `&DAT_07e11fb0` somewhere completely different so the
// dereference and bound were both garbage.
//
// In the source binary, the IDA decompile reads as:
//   result = dword_7E11FB0;
//   *(WORD*)((char*)result - 56) = -1;   // type field at -56 from result
//   *result = 0;                          // active flag at +0
//   result += 17 (=68 bytes);
// The "-56" offset reaches into a SEPARATE global `DAT_07e11f78` (Net_Packet
// Session.cpp:228-235 confirms this is a paired layout: type-field array at
// _07e11f78, active-flag array at _07e11fb0, both 0x880 bytes / 32 slots ×
// 68-byte stride). We use the symbol bases here so the linker's placement
// is irrelevant.
void __cdecl FUN_004cbdd0(void) {
    BYTE* typeBase   = (BYTE*)&DAT_07e11f78;
    BYTE* activeBase = (BYTE*)&DAT_07e11fb0;
    for (int i = 0; i < 32; ++i) {
        int off = i * 0x44;
        *(short*)(typeBase   + off) = (short)0xffff;
        *(int*  )(activeBase + off) = 0;
    }
}

// ReleaseMainData @ 0x005110A0 (29 bytes) — release all game data
void __cdecl ReleaseMainData(void) {
    DeleteObjects();
    DeleteNpcs();
    DeleteMonsters();
    ClearItems();
    ClearCharacters(-1);
}



// FUN_00404e60 @ 0x00404E60 — CWaveFile deinit (set vtable, close MMIO)


// FUN_00406cb0 @ 0x00406CB0 — HashWidget ~dtor
void __fastcall FUN_00406cb0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00406cd0((void *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// FUN_00407dc0 @ 0x00407DC0 — CWaveFile base ~dtor
void __fastcall FUN_00407dc0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00407de0((DWORD *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// FUN_00407ea0 @ 0x00407EA0 — CWaveFile derived ~dtor
void __fastcall FUN_00407ea0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_00407ec0((DWORD *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}



// FUN_0040a6c0 @ 0x0040A6C0 — WidgetC derived ~dtor
void __fastcall FUN_0040a6c0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_0040a6e0((void *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// FUN_0040db60 @ 0x0040DB60 — DSDevice ~dtor
void __fastcall FUN_0040db60(int ecx, int /*edx*/, BYTE param_1) {
    FUN_0040d550((void *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// FUN_0040db80 @ 0x0040DB80 — DSBuffer ~dtor
void __fastcall FUN_0040db80(int ecx, int /*edx*/, BYTE param_1) {
    FUN_0040dba0((void *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// FUN_0040eac0 @ 0x0040EAC0 — DSBufferB ~dtor
void __fastcall FUN_0040eac0(int ecx, int /*edx*/, BYTE param_1) {
    FUN_0040eae0((void *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// FUN_0040f520 @ 0x0040F520 — Stream deinit ~dtor
void __fastcall FUN_0040f520(int ecx, int /*edx*/, BYTE param_1) {
    FUN_0040f540((void *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}

// FUN_0040f840 @ 0x0040F840 — StreamB ~dtor
void __fastcall FUN_0040f840(int ecx, int /*edx*/, BYTE param_1) {
    FUN_0040f950((void *)ecx);
    if (param_1 & 1) operator_delete((void *)ecx);
}





// ── 30-byte: other ──────────────────────────────────────────────────────────

// FUN_00405240 @ 0x00405240 (30 bytes) — ErrorReport init (vtable + open log)
// NOTE: FUN_004052b0 opens the log file — stub it as nop for now
void __fastcall FUN_00405240_init(void *This) {
    *(int *)This = (int)&PTR_FUN_005524c4;
    FUN_00405290_impl((int)This);
    // FUN_004052b0(This, "MuError.log"); — opens MuError.log for writing
}

// ── 32-byte ─────────────────────────────────────────────────────────────────

// CSQuest::setQuestList @ 0x004011B0 (32 bytes)
void __fastcall CSQuest__setQuestList(int ecx, int /*edx*/, int index, int result) {
    *(char *)(ecx + 0x1c87a) = (char)index;
    *(BYTE *)(((index & 0xff) >> 2) + 0x1c848 + ecx) = (BYTE)result;
}

// DecryptCheckSumKey @ 0x00412D30 (32 bytes)
WORD __cdecl DecryptCheckSumKey(WORD wSource) {
    WORD uVar1 = wSource ^ 0xb479;
    return (uVar1 >> 6 ^ uVar1) & 0xf ^ uVar1 >> 6;
}

// StopMusic @ 0x00513420 (32 bytes) — corta los 6 BGM de la tabla.
//
// IDA:  v0 = g_lpszMp3;  do { StopMp3(*v0++, 0); } while ((int)v0 < (int)&g_iCurrentDialogScript);
// El bound `< 0x5615DC` es &g_lpszMp3[6], asi que son 6 iteraciones. Antes esto
// era un StopMp3 unico "best-effort" porque g_lpszMp3 era un DWORD = 0 y no
// habia tabla; ahora la tabla existe (ver globals.cpp) y el loop es el de IDA.
void __cdecl StopMusic(void) {
    for (int i = 0; i < 6; i++)
        StopMp3(g_lpszMp3[i], 0);
}

// ── 33-byte ─────────────────────────────────────────────────────────────────

// FUN_00406e60 @ 0x00406E60 (33 bytes) — free two allocations + zero count
void __fastcall FUN_00406e60(int param_1) {
    operator_delete(*(void **)(param_1 + 8));
    operator_delete(*(void **)(param_1 + 4));
    *(int *)(param_1 + 0xc) = 0;
}


// ── 34-byte ─────────────────────────────────────────────────────────────────

// AllStopSound @ 0x00404CA0 (34 bytes) — stop all 420 sound buffers
void __cdecl AllStopSound(void) {
    if (g_EnableSound) {
        for (int i = 0; i < 420; i++) StopBuffer(i, 1);
    }
}

// ── 35-byte ─────────────────────────────────────────────────────────────────

// EndOpengl @ 0x00511BC0 (35 bytes) — pop GL matrices
void __cdecl EndOpengl(void) {
    glPopMatrix();
    glMatrixMode(0x1701);  // GL_PROJECTION
    glPopMatrix();
    glMatrixMode(0x1700);  // GL_MODELVIEW
}

// ── 36-byte ─────────────────────────────────────────────────────────────────

// GetHandOfWeapon @ 0x00448900 (36 bytes) — returns 1 if entity type 390 and action 37/38
int __cdecl GetHandOfWeapon(int obj) {
    int hand = 0;
    if (*(short *)(obj + 2) == 390) {
        BYTE act = *(BYTE *)(obj + 0x105);
        if (act == 37 || act == 38) hand = 1;
    }
    return hand;
}

// ── 37-byte ─────────────────────────────────────────────────────────────────

// FUN_0040a830 @ 0x0040A830 (37 bytes) — free two optional allocations
void __fastcall FUN_0040a830(int param_1) {
    if (*(void **)(param_1 + 0x1c) != NULL) operator_delete(*(void **)(param_1 + 0x1c));
    if (*(void **)(param_1 + 8) != NULL) operator_delete(*(void **)(param_1 + 8));
}

// getEqualMonster @ 0x0047EBD1 (38 bytes) — DevilSquare kill check
bool __cdecl getEqualMonster(int addV) {
    return m_iKillMonster <= m_iMaxKillMonster + addV;
}

// DeleteBug @ 0x004FFFA0 (38 bytes) — disable butterfly objects owned by Owner
void __cdecl DeleteBug(int Owner) {
    char *o = (char *)&Butterfles;
    char *end = (char *)&Butterfles + 0x1158;  // 10 entries × 0x1BC stride
    do {
        if ((*(BYTE *)o != 0) && (*(int *)(o + 0xFC) == Owner)) {
            *(BYTE *)o = 0;
        }
        o += 0x1BC;  // Butterfles OBJECT stride
    } while (o < end);
}

// ── 42-byte ─────────────────────────────────────────────────────────────────

// FreeDirectSound @ 0x00404B80 (42 bytes) — release DirectSound device.
// Also releases per-slot buffer arrays and the 3D listener added by our
// DirectSound8 rewrite, so a clean shutdown doesn't leak COM refs.
void __cdecl FreeDirectSound(void) {
    if (!g_EnableSound) return;

    for (int i = 0; i < 420; ++i) {
        for (int c = 0; c < 4; ++c) {
            if (g_lpDS3DBuffer[i][c]) { g_lpDS3DBuffer[i][c]->Release(); g_lpDS3DBuffer[i][c] = NULL; }
            if (g_lpDSBuffer  [i][c]) { g_lpDSBuffer  [i][c]->Release(); g_lpDSBuffer  [i][c] = NULL; }
        }
        MaxBufferChannel[i] = 0;
        BufferChannel[i]    = 0;
        Enable3DSound[i]    = false;
    }
    if (g_lpDS3DListener) { g_lpDS3DListener->Release(); g_lpDS3DListener = NULL; }
    if (g_lpDS)           { g_lpDS->Release();           g_lpDS = NULL; }

    g_EnableSound = FALSE;
}

// ── 43-byte ─────────────────────────────────────────────────────────────────

// FUN_00406ec0 @ 0x00406EC0 (43 bytes) — hash table destructor (zero fields + set vtable + conditional delete)
void __fastcall FUN_00406ec0(int ecx, int /*edx*/, BYTE param_1) {
    *(int *)(ecx + 0xc) = 0;
    *(int *)(ecx + 4) = 0;
    *(int *)(ecx + 8) = 0;
    *(int *)ecx = (int)&PTR_FUN_005524d8;
    if (param_1 & 1) operator_delete((void *)ecx);
}

// ── 45-byte ─────────────────────────────────────────────────────────────────

// FUN_0040f540 @ 0x0040F540 (45 bytes) — Stream deinit: set vtable + release sub-object
void __fastcall FUN_0040f540(void *This) {
    int *piVar1 = (int *)((int *)This)[1];
    *(int *)This = (int)&PTR_FUN_005527e0;
    if (piVar1 != NULL) {
        typedef void (__cdecl *RelFunc)(int);
        RelFunc fn = (RelFunc)(*(int *)(*piVar1 + 0x14));
        fn(1);
    }
}

// FUN_0053cc00 @ 0x0053CC00 (45 bytes) — GameGuard class: zero 12 fields

// ── 48-byte ─────────────────────────────────────────────────────────────────

// FUN_00406db0 @ 0x00406DB0 (48 bytes) — thiscall: set fields + random hash prime
void __fastcall FUN_00406db0(int ecx, int /*edx*/, int param_1, int param_2) {
    *(int *)(ecx + 0x10) = param_1;
    *(int *)(ecx + 0x14) = param_2;
    int r = rand();
    *(int *)(ecx + 0x1c) = ((r % 0xf0) / 2) * 2 + 0x6f;
}

// MoveBlurs @ 0x0046C3B0 (48 bytes) — tick blur pool, decrement counters.
//
// 2026-05-03: AUTO-SKIP removed. The blur/joint/trail shared pool is now
// allocated as `g_RenderPool_07c608a8[100 * 0x2f0]` in globals.cpp.
// DAT_07c608b8 is the +16 anchor (= +4 from DAT_07c608b4). The walker reads
// fields at piVar2[-4..-2..0] which map to slot offsets 0..4..8..16 — all
// inside the 100-slot pool. Iteration count is 100 (matches IDA bound
// `< 0x7c72e78` = base + 100*0x2f0).
void __cdecl MoveBlurs(void) {
    int *piVar2 = &DAT_07c608b8;
    for (int slotIdx = 0; slotIdx < 100; ++slotIdx, piVar2 = (int*)((int)piVar2 + 0x2f0)) {
        if (*(char *)(piVar2 - 4) != '\0') {
            int iVar1 = piVar2[-2];
            piVar2[-2] = iVar1 - 1;
            if (iVar1 - 1 < 1) {
                iVar1 = *piVar2;
                *piVar2 = iVar1 - 1;
                if (iVar1 - 1 < 1) {
                    *(BYTE *)(piVar2 - 4) = 0;
                }
            }
        }
    }
}

// ── 49-byte ─────────────────────────────────────────────────────────────────

// SetMatchInfo @ 0x0047EBA0 (49 bytes) — set DevilSquare match parameters
void __cdecl SetMatchInfo(BYTE byType, int iMaxTime, int iTime, int iMaxMonster, int iKillMonster_p) {
    m_byMatchType = byType;
    m_iMatchTimeMax = iMaxTime;
    m_iMatchTimeMax = iTime;  // NOTE: Ghidra shows double-assign (original code bug)
    m_iMaxKillMonster = iMaxMonster;
    m_iKillMonster = iKillMonster_p;
}

// ── 50-byte ─────────────────────────────────────────────────────────────────

// getMonsterName @ 0x0047D200 — lookup name by type en la tabla MonsterScript.
// RE-ACTIVADO 2026-07-24: la tabla ahora esta bien dimensionada (512 × 0x36) y
// la carga NPCName_Load con Type[0]/Name[1].  IDA: `mov dl,[eax]` (Type es un
// BYTE en [0]), stride 0x36, hasta GateAttribute.  Aca acotamos por el contador
// real (DAT_07d78078 = EditMonsterNumber) en vez del literal 0x7cf5600.
char *__cdecl getMonsterName(int type) {
    int n = DAT_07d78078;
    if (n > 512) n = 512;
    for (int i = 0; i < n; ++i) {
        BYTE *m = &MonsterScript[i * 0x36];
        if (m[0] == (BYTE)type)
            return (char *)(m + 1);   // Name en [1]
    }
    return NULL;
}


// ═══════════════════════════════════════════════════════════════════════════════
// END BATCH: ~80 functions implemented (1-52 bytes range)
