// UI_SkillHotkeys.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FindHotKey @ 0x004B1170 (~202 lines)
// Looks up a skill ID in the CharacterMachine hotkey table via MAIN_HASH_CLASS.
// Returns hotkey slot index (0..19), or -1 if not found.
// Original wraps access in anti-tamper encrypt/decrypt; we skip that.
// Ghidra: CharacterAttribute->Skill[iVar5+4] == unaff_retaddr (phantom param = Skill)
// Real access: *(BYTE*)(DAT_07cf1ff4 + 0x57 + iVar5) == Skill
// IDA: FindHotKey @ 0x004B1170 — int __cdecl FindHotKey(int Skill)
//   v17 = 0;                                        // <- valor por defecto
//   v4  = 0;
//   while ( *(unsigned __int8 *)(CharacterAttribute + v4 + 87) != Skill )
//     if ( ++v4 >= 20 ) goto LABEL_22;
//   v17 = v4;
// LABEL_22:
//   return v17;
//
// 2026-09-01 FIX — devolvia **-1** cuando el skill no esta en los 20 slots;
// IDA devuelve **0** (el inicializador de v17, que el camino de no-encontrado
// nunca pisa).  Consecuencia real medida en el path de flechas:
//   MoveCharacter (6 sitios) -> CreateArrows(c, o, 0, FindHotKey(skill), ...)
//   -> CreateArrow -> CreateEffect(..., SkillIndex, Skill)
//   -> CreateEffect prologo: `i[133] = (BYTE)SkillIndex`  (= 0xFF con -1)
//   -> sub_466440 (0x00466440, llamado por MoveEffect en cada tick del
//      proyectil) hace `CharacterAttribute[ i[133] + 87 ]`, o sea
//      CharacterAttribute[342] — FUERA del array de 20 skills (87..106).
// Ese byte basura se compara contra 51/52 y, cuando cae en 52, dispara
// `CreateJoint(1249, ..., SubType 6, ...)` (la espiral de Penetration) en CADA
// flecha, de cualquier skill de Elf.  Tambien envenena
// `sub_45FEC0(i[133], ...)`.  Con 0 el indice vuelve a caer dentro del array.
// Ningun caller del arbol distingue -1 (verificado): nadie compara el retorno
// contra -1 ni contra < 0.
int __stdcall FindHotKey_stub(int Skill) {
    // anti-tamper hash table — skipped (encrypt CharacterMachine before read)

    char* charAttr = (char*)DAT_07cf1ff4;  // IDA: CharacterAttribute
    if (!charAttr)
        return 0;

    int slot = 0;                          // IDA: v4
    while ((unsigned char)charAttr[0x57 + slot] != (unsigned int)Skill) {
        if (++slot >= 0x14)
            return 0;                      // IDA: goto LABEL_22 con v17 = 0
    }

    // anti-tamper hash table — skipped (decrypt CharacterMachine after read)
    return slot;                           // IDA: v17 = v4
}

// RenderSkillIcon @ 0x004BB940 (~250 lines) — SUMMARY STUB
// Renders a skill icon bitmap at (x,y) with given dimensions.
// Reads CharacterAttribute->Skill[iIndex] via encrypted hash table access.
// If skill ID is 0, renders empty slot. Otherwise renders skill texture.
void __cdecl RenderSkillIcon_stub(int iIndex, float x, float y, float width, float height) {
    // 0x004BB940 — Renders a skill icon bitmap at (x,y) with given dimensions.
    // ~80% of Ghidra output is anti-tamper hash table operations wrapping reads to
    // CharacterAttribute->Skill and ->HotKey arrays. Only real logic implemented.

    // anti-tamper hash table — skipped (encrypt CharacterMachine before read)

    // Read skill ID from CharacterAttribute->Skill[iIndex + 4]
    // CharacterAttribute = DAT_07cf1ff4, Skill array starts at offset +0x57
    // Actually the Ghidra accesses Skill[unaff_retaddr + 4] where unaff_retaddr = iIndex
    // 2026-05-05: bounds check on iIndex (passed by caller, can be Hero[913]
    // garbage). Without this, reading CA[0x57+iIndex] overflows CA buffer
    // → garbage skillId → OOB on subsequent SkillAttribute reads → crash.
    if (iIndex < 0 || iIndex >= 60) return;
    char* charAttr = (char*)DAT_07cf1ff4;
    if (!charAttr) return;
    unsigned int skillId = (unsigned char)*(charAttr + 0x57 + iIndex);

    // Skip if skillId is invalid (0 = empty, >= 64 = OOB on SkillAttribute table).
    if (skillId == 0 || skillId >= 64) {
        return;
    }

    // If skill is 0x2f (Helper summon) and helper type is not Dark Horse (0x332) or Dark Spirit (0x333),
    // tint the icon reddish
    if (skillId == 0x2f) {
        // Hero->Helper.Type is at entity struct offset — approximate via DAT_07abf5d8
        // (Hero->Object).Position is at +0x10; Helper.Type offset approximated from Ghidra
        // In Ghidra: (Hero->Helper).Type  — skipping exact struct access, use raw offset
        // short helperType = *(short*)((char*)(DWORD)Hero + helperTypeOffset);
        // For now, skip the helper type check — the tint is cosmetic only
        // glColor3f(1.0f, 0.5f, 0.5f);
    }

    float fWidth = (float)(int)width;
    float fHeight = (float)(int)height;
    float fX = (float)(int)x;
    float fY = (float)(int)y;

    // 004BB940: skill icons are eight columns wide.  The original advances U
    // with the icon width and V with the icon height, then trims one pixel from
    // the V extent to avoid sampling the next atlas row.
    const int atlasIndex = (int)skillId - 1;
    const float atlasU = (float)((atlasIndex % 8) * (int)width);
    const float atlasV = (float)((atlasIndex / 8) * (int)height);

    // RenderBitmap(298, x, y, width, height, u, v, uWidth, vHeight, 1, 1)
    GL_DrawTexture(0x12a, fX, fY, fWidth, fHeight,
                 atlasU * _DAT_00552b7c,
                 atlasV * _DAT_00552b7c,
                 fWidth * _DAT_00552b7c,
                 (fHeight - _DAT_0055256c) * _DAT_00552b7c,
                 1, 1);

    // anti-tamper hash table — skipped (encrypt CharacterMachine before read)

    // Read hotkey assignment from CharacterAttribute->HotKey[SelectedHero][iIndex + 4]
    // Confirmed via Ghidra CHARACTER_ATTRIBUTE layout:
    //   0x53 Skill[64]      (iIndex+4 indexed here earlier)
    //   0x93 SkillLevel[64]
    //   0xD3 HotKey[4][64]  — 4 characters × 64 slots, stride 64 bytes
    // HotKey[SelectedHero][iIndex+4] = charAttr + 0xD3 + SelectedHero*64 + (iIndex + 4)
    unsigned int selectedHero = (unsigned int)DAT_005616ac;  // SelectedHero index (0..3)
    if (selectedHero > 3) selectedHero = 0;  // safety clamp
    int hotkey = (unsigned char)*(charAttr + 0xD3 + selectedHero * 64 + iIndex + 4);
    if (hotkey == 0) hotkey = 0xFF;  // empty slot → no-hotkey sentinel

    // anti-tamper hash table — skipped (decrypt CharacterMachine after read)

    if (hotkey != 0xFF) {
        RenderNumber2D_stub(x + _DAT_00552650, y + _DAT_00552a4c, hotkey, 9.0f, 10.0f);
    }
    DAT_00559c6c = (char)hotkey;
}
