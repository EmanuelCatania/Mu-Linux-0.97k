// UI_SkillHotkeys.cpp

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FindHotKey @ 0x004B1170 (~202 lines)
// Looks up a skill ID in the CharacterMachine hotkey table via MAIN_HASH_CLASS.
// Returns hotkey slot index (0..19), or 0 if not found (igual que IDA).
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
// OJO: no devolver -1 en el camino de no-encontrado.  El retorno termina en
// CreateEffect (MoveCharacter -> CreateArrows(c, o, 0, FindHotKey(skill), ...)
// -> CreateArrow -> `i[133] = (BYTE)SkillIndex`) y sub_466440 (MoveEffect, en
// cada tick del proyectil) lee `CharacterAttribute[ i[133] + 87 ]`: con 0xFF
// sale del array de 20 skills (87..106) y puede disparar
// `CreateJoint(1249, ...)` (la espiral de Penetration) en cada flecha.
// Ningun caller del arbol compara el retorno contra -1 ni contra < 0.
// IDA: FindHotKey (0x004B1170)
int __stdcall FindHotKey(int Skill) {
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
// IDA: sub_4BB940 (0x004BB940)
void __cdecl RenderSkillIcon(int iIndex, float x, float y, float width, float height) {
    // 0x004BB940 — Renders a skill icon bitmap at (x,y) with given dimensions.
    // ~80% of Ghidra output is anti-tamper hash table operations wrapping reads to
    // CharacterAttribute->Skill and ->HotKey arrays. Only real logic implemented.

    // anti-tamper hash table — skipped (encrypt CharacterMachine before read)

    // Skill ID = CharacterAttribute->Skill[iIndex] (CharacterAttribute = DAT_07cf1ff4,
    // el array Skill arranca en +0x57; Ghidra lo muestra como Skill[unaff_retaddr + 4]).
    // Se acota iIndex (puede venir de Hero[913] con basura): sin eso
    // CA[0x57+iIndex] se sale del buffer y el skillId basura indexa
    // SkillAttribute fuera de rango.
    if (iIndex < 0 || iIndex >= 60) return;
    char* charAttr = (char*)DAT_07cf1ff4;
    if (!charAttr) return;
    unsigned int skillId = (unsigned char)*(charAttr + 0x57 + iIndex);

    // Skip if skillId is invalid (0 = empty, >= 64 = OOB on SkillAttribute table).
    if (skillId == 0 || skillId >= 64) {
        return;
    }

    // Skill 0x2f (47, se usa montado): IDA sub_4BB940 L91-97 tiñe el icono
    // rojizo si el heroe no tiene Uniria (818) ni Dinorant (819) en el slot de
    // helper (Hero + 696 = c+0x2B8).
    if (skillId == 0x2f) {
        const BYTE* hero = (const BYTE*)(uintptr_t)DAT_07abf5d8;
        const short helperType = hero ? *(const short*)(hero + 696) : -1;
        if (helperType != 818 && helperType != 819)
            glColor3f(1.0f, 0.5f, 0.5f);
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
        RenderNumber2D(x + _DAT_00552650, y + _DAT_00552a4c, hotkey, 9.0f, 10.0f);
    }
    DAT_00559c6c = (char)hotkey;
}
