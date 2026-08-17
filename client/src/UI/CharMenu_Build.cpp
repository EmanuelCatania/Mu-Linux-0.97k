// CharMenu_Build.cpp — FUN_004c3530 @ 0x004c3530
// Character info / stats menu builder.  Dispatches on DAT_07e11d20 (mode 1/2/3).
//
// Populates a string list buffer (lpString_07e90798, 100 bytes/entry, ~30 slots)
// with formatted text for the character menu, then calls FUN_004c2420 to display it.
//
// ── Mode 1: class-list type A ─────────────────────────────────────────────────
//   Writes header (DAT_0055a408), subheader (DAT_0055a40c).
//   Iterates DAT_07d32af0 (stride 300, limit 0x7d34134) — class info list A.
//   Writes footer (DAT_0055a410).
//   Calls FUN_004c2420(1,1,count,0,2,1).
//
// ── Mode 2: class-list type B ─────────────────────────────────────────────────
//   Same structure with DAT_07d34260 (limit 0x7d358a4) and strings DAT_0055a414/418/41c.
//
// ── Mode 3: class stats detail ────────────────────────────────────────────────
//   Layout parameters by resolution (DAT_0056156c):
//     0x280(640)  → col_w=0x5a, pad=0x34
//     800         → col_w=0x5a, pad=0x2f
//     0x400(1024) → col_w=0x67, pad=0x28
//     0x500(1280) → col_w=0x7b, pad=0x20
//
//   Class-id ranges in DAT_07e11d24:
//     0x000..0x09F → type 1 (max 0x870 xp)
//     0x0A0..0x0BF → type 2 (max 900)
//     0x0C0..0x0DF → type 3 (max 0x708)
//     0x0E0..0x17F → type 4 (max 3000)
//     0x1E0..0x1FF → type 5 (no xp bar)
//
//   Computes local_1c = max_xp / col_width (horizontal scale for progress bar).
//   Calls FUN_004c2e20(class_id) to prepare class data.
//   Builds string slots: class name, subtype header, padding rows, then calls
//   FUN_004c2880(class_data_ptr) for the detail block.
//   Draws stat rows via FUN_004c2d50 / FUN_004c2c10 conditionally on stat flags
//   (DAT_07e91530/534/53c/540) and class-id range.
//
// After building: sets DAT_07e11d6e = 1 (dirty flag → triggers re-render).

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// String table aliases for readability
#define s_ChMenu_HdrA  DAT_0055a408
#define s_ChMenu_SubA  DAT_0055a40c
#define s_ChMenu_FtrA  DAT_0055a410
#define s_ChMenu_HdrB  DAT_0055a414
#define s_ChMenu_SubB  DAT_0055a418
#define s_ChMenu_FtrB  DAT_0055a41c
#define s_ChMenu_HdrC  DAT_0055a420
#define s_ChMenu_SubC1 DAT_0055a424
#define s_ChMenu_SubC2 DAT_0055a428
#define s_ChMenu_SubC3 DAT_0055a42c
#define s_ChMenu_SubC4 DAT_0055a430
#define s_ChMenu_FtrC  DAT_0055a434

// ─────────────────────────────────────────────────────────────────────────────

// Helper: strcpy-length into lpString_07e90798[slot] using manual word-copy loop.
// (Ghidra emits length + word-copy idiom for all string copies here.)
static void slot_strcpy(int slot, const char *src)
{
    char *dst = lpString_07e90798 + slot * 100;
    // determine source length
    int len = 0; while (src[len]) len++; len++;   // include NUL
    const char *s = src;
    char *d = dst;
    for (unsigned u = (unsigned)len >> 2; u; u--, s+=4, d+=4)
        *(unsigned int *)d = *(unsigned int *)s;
    for (unsigned u = (unsigned)len & 3; u; u--)
        *d++ = *s++;
}

// ─────────────────────────────────────────────────────────────────────────────

void FUN_004c3530(void)
{
    // ── Mode 1: build class-list A ────────────────────────────────────────────
    if (DAT_07e11d20 == 1)
    {
        FUN_00511600();
        DAT_07eaa154 = 0;

        // Slot 0: header line
        crt_sprintf(lpString_07e90798, s_ChMenu_HdrA);
        int iVar4 = DAT_07eaa154 + 1;
        DAT_07e91708[iVar4] = 1;
        (&DAT_07ea7b10)[iVar4] = 1;
        slot_strcpy(iVar4, &DAT_07d329c4);
        DAT_07eaa154 = iVar4 + 1;   // +2

        // Slot 2 (iVar4): subheader
        crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_SubA);
        DAT_07eaa154++;

        // Class entries from list A (stride 300, limit 0x7d34134)
        char *pcVar5 = &DAT_07d32af0;
        char *local_c = lpString_07e90798 + DAT_07eaa154 * 100;
        while (pcVar5 != nullptr && (int)pcVar5 < 0x7d34134) {
            DAT_07e91708[DAT_07eaa154] = 0;
            (&DAT_07ea7b10)[DAT_07eaa154] = 0;
            slot_strcpy(DAT_07eaa154, pcVar5);
            DAT_07eaa154++;
            pcVar5 += 300;
            local_c += 100;
        }

        // Footer
        crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_FtrA);
        DAT_07eaa154++;
        DAT_07e11d6e = 1;
        FUN_004c2420(1, 1, DAT_07eaa154, 0, 2, 1);
        return;
    }

    // ── Mode 2: build class-list B ────────────────────────────────────────────
    if (DAT_07e11d20 == 2)
    {
        FUN_00511600();
        DAT_07eaa154 = 0;

        crt_sprintf(lpString_07e90798, s_ChMenu_HdrB);
        int iVar4 = DAT_07eaa154 + 1;
        DAT_07e91708[iVar4] = 1;
        (&DAT_07ea7b10)[iVar4] = 1;
        slot_strcpy(iVar4, &DAT_07d34134);
        DAT_07eaa154 = iVar4 + 1;   // +2

        crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_SubB);
        DAT_07eaa154++;

        char *pcVar5 = &DAT_07d34260;
        char *local_c = lpString_07e90798 + DAT_07eaa154 * 100;
        while (pcVar5 != nullptr && (int)pcVar5 < 0x7d358a4) {
            DAT_07e91708[DAT_07eaa154] = 0;
            (&DAT_07ea7b10)[DAT_07eaa154] = 0;
            slot_strcpy(DAT_07eaa154, pcVar5);
            DAT_07eaa154++;
            pcVar5 += 300;
            local_c += 100;
        }

        crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_FtrB);
        DAT_07eaa154++;
        DAT_07e11d6e = 1;
        FUN_004c2420(1, 1, DAT_07eaa154, 0, 2, 1);
        return;
    }

    // ── Mode 3: class stats detail ────────────────────────────────────────────
    if (DAT_07e11d20 != 3) return;

    FUN_00511600();

    // Layout parameters by screen resolution
    int col_w  = 0;    // column width (iVar4 / iVar7 in Ghidra)
    int pad    = 0;    // padding (local_14)
    int col_w2 = 0;    // second col width (local_18)
    int pad2   = 0;    // second pad (local_c as int)

    if (DAT_0056156c < 0x401) {
        if      (DAT_0056156c == 0x400) { col_w=0x67; pad=0x1c; col_w2=0x67; pad2=0x28; }
        else if (DAT_0056156c == 0x280) { col_w=0x5a; pad=0x26; col_w2=0x5a; pad2=0x34; }
        else if (DAT_0056156c == 800  ) { col_w=0x5a; pad=0x21; col_w2=0x5a; pad2=0x2f; }
    } else if (DAT_0056156c == 0x500)   { col_w=0x7b; pad=0x16; col_w2=0x7b; pad2=0x20; }

    // Determine class type from DAT_07e11d24 (class ID)
    int iVar7  = 0;      // class type (1..5)
    unsigned int uVar2 = 0;   // xp max constant
    int local_10 = 0;

    int cls = (int)DAT_07e11d24;
    if      (cls < 0xa0)                        { iVar7=1; uVar2=0x870;  local_10=1; }
    else if (cls < 0xc0)                        { iVar7=2; uVar2=900;    local_10=2; }
    else if (cls < 0xe0)                        { iVar7=3; uVar2=0x708;  local_10=3; }
    else if (cls < 0x180)                       { iVar7=4; uVar2=3000;   local_10=4; }
    else if (cls >= 0x1e0 && cls <= 0x1ff)     { iVar7=5; uVar2=0x1734; local_10=5;
        if (DAT_0056156c == 800 || DAT_0056156c == 0x400) uVar2 = 0x1450; }
    else { DAT_07e11d20 = 0; return; }

    int local_1c = (col_w > 0) ? (int)((unsigned long long)uVar2 / (unsigned long long)(long long)col_w) : 0;

    // Extra slot count for class type 5
    int local_8 = (iVar7 == 5) ? 0 : 0xb;

    // Pointer to class data: DAT_07d78068[class_id * 0x40]
    int iVar4 = (int)DAT_07e11d24 * 0x40 + DAT_07d78068;

    FUN_004c2e20(DAT_07e11d24);
    DAT_07eaa154 = 0;

    // Header slot
    crt_sprintf(lpString_07e90798, s_ChMenu_HdrC);
    int iVar7b = DAT_07eaa154 + 2;
    DAT_07e91708[DAT_07eaa154 + 1] = 1;
    (&DAT_07ea7b10)[DAT_07eaa154 + 1] = 1;
    slot_strcpy(DAT_07eaa154 + 1, &DAT_07d358a4);
    DAT_07e91708[iVar7b] = 1;
    (&DAT_07ea7b10)[iVar7b] = 1;

    crt_sprintf(lpString_07e90798 + iVar7b * 100, s_ChMenu_SubC1);
    DAT_07e91708[DAT_07eaa154] = 0;
    (&DAT_07ea7b10)[DAT_07eaa154] = 1;
    DAT_07eaa154 = iVar7b + 1;

    crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_SubC2);
    DAT_07eaa154++;
    crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_SubC3);
    DAT_07eaa154++;
    crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_SubC4);
    int iVar7c = DAT_07eaa154 + 1;
    DAT_07e11d6e = 1;

    // Padding row (spaces, width adjusted for resolution)
    unsigned int uVar3 = (DAT_0056156c > 800) ? 0x2e + 5 : 0x2e;
    char *pPad = lpString_07e90798 + iVar7c * 100;
    unsigned int *pu = (unsigned int *)pPad;
    for (unsigned u = uVar3 >> 2; u; u--) *pu++ = 0x20202020;
    DAT_07e91708[iVar7c] = 0;
    for (unsigned u = uVar3 & 3; u; u--) *(unsigned char *)pu++ = 0x20;
    (&DAT_07ea7b10)[iVar7c] = 0;
    DAT_07eaa154 += 2;
    pPad[uVar3] = 0;

    // Extra blank rows for type ≠ 5
    if (local_8 > 0) {
        unsigned char *pu10 = (unsigned char *)(&DAT_07ea7b10 + DAT_07eaa154);
        for (char *p2 = (char *)local_8; p2; p2--) { *pu10 = 0; pu10 += 4; }
        char *pb = &lpString_07e90798[0] + DAT_07eaa154 * 100 + 1;
        while (local_8 > 0) {
            pb[-1] = 0x20;
            pb[0]  = 0;
            DAT_07eaa154++;
            pb += 100;
            local_8--;
            DAT_07e91708[DAT_07eaa154] = 0;
        }
    }

    // Class detail block
    FUN_004c2880(iVar4);

    // Footer
    crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_FtrC);
    DAT_07eaa154++;
    FUN_004c2420(1, 1, DAT_07eaa154, col_w2, 2, 1);
    FUN_00511680('\x01');

    // Stat rows
    DAT_07eaa154 = 0;
    FUN_004c2d50(0, local_1c, pad);
    FUN_004c2c10(0, (unsigned char *)0x0055a440, &local_1c,
                 (const char *)0x0055a438, pad2, local_10);

    if (DAT_07e91530 > 0 && !(cls >= 0x1e0 && cls <= 0x1ff)) {
        FUN_004c2c10(2, (unsigned char *)0x0055a448, &local_1c,
                     (const char *)0x0055a444, pad2, 0);
        FUN_004c2d50(2, local_1c, pad);
        FUN_004c2c10(0, (unsigned char *)0x0055a450, &local_1c,
                     (const char *)0x0055a44c, pad2, 0);
    }
    if (DAT_07e91534 > 0 && !(cls >= 0x1e0 && cls <= 0x1ff)) {
        FUN_004c2c10(3, (unsigned char *)0x0055a45c, &local_1c,
                     (const char *)0x0055a454, pad2, 0);
    }
    if (cls >= 0xa0 && cls < 0xc0) {
        FUN_004c2d50(4, local_1c, pad);
        FUN_004c2c10(4, (unsigned char *)0x0055a468, &local_1c,
                     (const char *)0x0055a460, pad2, 0);
    }
    if (DAT_07e9153c > 0) {
        FUN_004c2d50(5, local_1c, pad);
        FUN_004c2c10(5, (unsigned char *)0x0055a478, &local_1c,
                     (const char *)0x0055a470, pad2, 0);
    }
    if (DAT_07e91540 > 0) {
        FUN_004c2d50(6, local_1c, pad);
        FUN_004c2c10(6, (unsigned char *)0x0055a484, &local_1c,
                     (const char *)0x0055a47c, pad2, 0);
    }
    if (!(cls >= 0x1e0 && cls <= 0x1ff)) {
        FUN_004c2d50(7, local_1c, pad);
        FUN_004c2c10(7, (unsigned char *)0x0055a494, &local_1c,
                     (const char *)0x0055a48c, pad2, 0);
    }
    if (!(cls >= 0x1e0 && cls <= 0x1ff)) {
        FUN_004c2d50(8, local_1c, pad);
        FUN_004c2c10(8, (unsigned char *)0x0055a4a0, &local_1c,
                     (const char *)0x0055a498, pad2, 0);
    }
    if (cls >= 0x1e0 && cls <= 0x1ff) {
        FUN_004c2d50(9, local_1c, pad);
        FUN_004c2c10(9, (unsigned char *)0x0055a4ac, &local_1c,
                     (const char *)0x0055a4a4, pad2, local_10);
    }
    FUN_00511600();
}


// ─────────────────────────────────────────────────────────────────────────────
// FUN_004c2420 @ 0x004c2420 — CharMenu_RenderTextList
//
// Renders the text-list buffer (lpString_07e90798) into the char-menu panel.
// param_1 = mode (1 = with shadow border, else plain)
// param_2 = start index, param_3 = count
// param_4 = x position, param_5 = layout type, param_6 = border flag
//
// Selects font via DAT_07ea7b10 per entry, measures width with GetTextExtentPointA,
// colors from DAT_07e91708 color table (types 0-6).
// FUN_0040f610 — text render; FUN_005124c0 — filled rect (shadow box).

void __cdecl FUN_004c2420(int param_1, int param_2, int param_3,
                            int param_4, int param_5, int param_6)
{
    TEXTMETRICA tm;
    SIZE        sz;
    HDC         hdc = (HDC)DAT_055c9fec;
    int         x0, y0, entry_h;
    int         iVar1;

    // guard: nothing to draw
    if (param_3 <= 0) return;

    // Row height from TEXTMETRIC
    GetTextMetricsA(hdc, &tm);
    entry_h = tm.tmHeight + tm.tmExternalLeading;

    x0 = param_4;
    y0 = param_2 * entry_h;     // start Y

    for (int i = 0; i < param_3; i++) {
        char *pText = lpString_07e90798 + i * 100;
        int   colorType = DAT_07e91708[i];
        int   fontIdx   = (&DAT_07ea7b10)[i];

        // Select font
        if (fontIdx != 0)
            FUN_00511600();

        // Measure text
        GetTextExtentPointA(hdc, pText, (int)strlen(pText), &sz);

        if (param_6 == 1) {
            // Draw shadow/border rect first
            FUN_005124c0((float)(x0 - 2), (float)(y0 + i * entry_h - 1), (float)(sz.cx + 4), (float)(entry_h + 2));
        }

        // Color from table (0=white,1=yellow,2=green,3=red,4=cyan,5=gray,6=orange)
        static const DWORD colorTable[7] = {
            0xFFFFFFFF, 0xFFFFFF00, 0xFF00FF00,
            0xFFFF0000, 0xFF00FFFF, 0xFF808080, 0xFFFF8000
        };
        iVar1 = (colorType < 7) ? colorType : 0;

        FUN_0040f610(hdc, x0, y0 + i * entry_h, pText, colorTable[iVar1]);
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// FUN_004c2880 @ 0x004c2880 — CharMenu_AppendSkillReq
//
// Reads weapon skill slots at param_1+0x38 (4 slots, each 4 bytes).
// Checks hero's class (DAT_07abf5d8+0x1bc) vs required class per slot.
// Formats each slot into lpString_07e90798 using crt_sprintf with format
// strings from DAT_0055a400/DAT_0055a404.
// Increments DAT_07eaa154 per entry.

void __cdecl FUN_004c2880(int param_1)
{
    int  iVar1;
    char buf[256];
    int  heroClass = *(int*)(&DAT_07abf5d8 + 0x1bc);

    for (int s = 0; s < 4; s++) {
        int slot = *(int*)(param_1 + 0x38 + s * 4);
        if (slot == 0) continue;

        // Check class requirement
        int required = slot & 0xff;
        if (required != 0 && heroClass != required) {
            // Use "not met" format
            crt_sprintf(buf, DAT_0055a404, slot >> 8, required);
            DAT_07e91708[DAT_07eaa154] = 3;  // red
        } else {
            crt_sprintf(buf, DAT_0055a400, slot >> 8);
            DAT_07e91708[DAT_07eaa154] = 2;  // green
        }

        (&DAT_07ea7b10)[DAT_07eaa154] = 0;
        slot_strcpy(DAT_07eaa154, buf);
        DAT_07eaa154++;
        DAT_07eaa158++;
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// FUN_004c2c10 @ 0x004c2c10 — CharMenu_AppendStatRows
//
// Loops 0 to 0xb (skips if param_6==5), reads from DAT_07e9152c.
// Calls FUN_004c2420 for display.
// Uses GetTextExtentPointA for width measurement.

void __cdecl FUN_004c2c10(int row, unsigned char *color, int *value,
                            const char *label, int x, int flags)
{
    char   buf[256];
    int    iVar1;
    SIZE   sz;
    HDC    hdc = (HDC)DAT_055c9fec;

    if (flags == 5) return;

    for (int i = 0; i <= 0xb; i++) {
        int val = *(int*)(&DAT_07e9152c + i * 4);
        if (val == 0) continue;

        crt_sprintf(buf, label, val);
        GetTextExtentPointA(hdc, buf, (int)strlen(buf), &sz);

        DAT_07e91708[DAT_07eaa154] = (color ? color[i] : 0);
        (&DAT_07ea7b10)[DAT_07eaa154] = 0;
        slot_strcpy(DAT_07eaa154, buf);
        DAT_07eaa154++;
    }

    FUN_004c2420(row, x, DAT_07eaa154, *value, 2, 1);
}


// ─────────────────────────────────────────────────────────────────────────────
// FUN_004c2d50 @ 0x004c2d50 — CharMenu_AppendSkillDesc
//
// Switches on param_1 (0-9) to select description string from
// DAT_07d359d0...DAT_07d36204. Formats into text buffer, calls FUN_004c2420.

void __cdecl FUN_004c2d50(int param_1, int param_2, int param_3)
{
    // BUG-FIX 2026-05-03: original `descTable` held literal source-binary addresses
    // (0x07d359d0 onwards) that map to text-pool data in the source binary but are
    // unmapped memory in our build. Reading `(const char*)descTable[param_1]`
    // would AV on `if (!*desc)`. Until the description text-pool is parsed
    // (Data\Local\Text.bmd handler chain), this function is a no-op.
    (void)param_1; (void)param_2; (void)param_3;
}


// ─────────────────────────────────────────────────────────────────────────────
// FUN_004c2e20 @ 0x004c2e20 — CharMenu_BuildStatRequirements
//
// Guard: if DAT_00559fe0 == param_1, return (already built for this class).
// Reads char data from DAT_07d78068 + param_1 * 0x40.
// Calculates level-scaled ATT/DEF/MANA reqs, fills DAT_07e91528-07e91550 array.
// Contains HashTable operations with XOR encryption (key at DAT_00559050,
// 0x584 bytes, ref-count at +0x161). Loops 0-11 per stat level.

void __cdecl FUN_004c2e20(int param_1)
{
    int   iVar1;
    int   iVar2;
    int   iVar3;
    int  *piDst;

    // Already computed?
    if (DAT_00559fe0 == param_1) return;
    DAT_00559fe0 = param_1;

    // Base class data pointer
    int classBase = DAT_07d78068 + param_1 * 0x40;

    // Clear stat requirement array
    piDst = DAT_07e91528;
    for (int i = 0; i <= 11; i++) piDst[i] = 0;

    // Build per-level scaled requirements
    for (int i = 0; i <= 11; i++) {
        int base_att  = *(int*)(classBase + 0x00);
        int base_def  = *(int*)(classBase + 0x04);
        int base_mana = *(int*)(classBase + 0x08);
        int level_req = *(int*)(classBase + 0x0c + i * 4);

        if (level_req <= 0) continue;

        // Scale by level
        iVar1 = base_att  + (level_req * *(int*)(classBase + 0x30));
        iVar2 = base_def  + (level_req * *(int*)(classBase + 0x34));
        iVar3 = base_mana + (level_req * *(int*)(classBase + 0x38));

        DAT_07e91528[i] = iVar1;
        *(int*)(&DAT_07e91530 + i * 4) = iVar2;
        *(int*)(&DAT_07e9153c + i * 4) = iVar3;
    }

    // HashTable XOR-obfuscation (ref-count at DAT_00559050+0x161)
    unsigned char *pHT = (unsigned char*)&DAT_00559050;
    int refCount = *(int*)(pHT + 0x161);
    // (no game-logic side-effects here; this is the compiler/obfuscation artifact)
    (void)refCount;
}
