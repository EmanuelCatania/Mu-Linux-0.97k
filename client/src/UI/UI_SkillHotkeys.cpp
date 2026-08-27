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
int __stdcall FindHotKey_stub(int Skill) {
    // anti-tamper hash table — skipped (encrypt CharacterMachine before read)

    char* charAttr = (char*)DAT_07cf1ff4;  // CharacterAttribute

    // Scan 20 hotkey slots (indices 0..19)
    int iVar5 = 0;
    do {
        if ((unsigned char)*(charAttr + 0x57 + iVar5) == (unsigned int)Skill) break;
        iVar5++;
    } while (iVar5 < 0x14);

    // anti-tamper hash table — skipped (decrypt CharacterMachine after read)

    // If iVar5 < 0x14, we found a match; otherwise not found
    if (iVar5 >= 0x14) {
        // Not found — check WhisperRegistID table for the skill name
        // Ghidra: iterates WhisperRegistID (stride 10, up to 0x7db9373)
        // and calls AddText if no match found. This path is rarely hit.
        // For now, return -1 (not found).
        return -1;
    }
    return iVar5;
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
