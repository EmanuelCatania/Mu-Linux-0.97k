// Combat_Projectiles.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// IDA: FUN_00474370 @ 0x00474370 — CreateArrow (~658 lines), creates one arrow/bolt projectile.
// Decompiled from Ghidra. Anti-tamper hash table ops skipped.
// Phantom params (unaff_EBX/ESI/EDI/EBP/retaddr) are obfuscation artifacts.
// Reads weapon type from character's equipment slots, determines projectile
// visual effect, plays sound, sets CurrentSkill, then dispatches through
// two switch tables: first by weapon-type for the projectile, second by
// adjusted weapon-type (+400) for the trail effect.
void __cdecl CreateArrow_stub(DWORD c, DWORD o, DWORD to, WORD SkillIndex, WORD Skill, WORD SKKey) {
    // Read weapon types from character equipment slots
    int weaponType0 = (int)*(short*)(c + 0x270);  // Weapon[0].Type
    int weaponType1 = (int)*(short*)(c + 0x288);  // Weapon[1].Type

    bool isHero = (c == (DWORD)DAT_07abf5d8);

    if (isHero) {
        // anti-tamper hash table — skipped (CharacterMachine encrypt/decrypt/lookup)
        // Read weapon types from CharacterMachine instead
        DWORD cm = (DWORD)DAT_07cf1ffc;
        weaponType0 = (int)*(short*)(cm + 0x218);
        weaponType1 = (int)*(short*)(cm + 0x25C);
    }

    // Adjust: add ITEM_BASE_MODEL (400) if not -1
    int adjType0 = weaponType0;
    if (weaponType0 != -1) {
        adjType0 = weaponType0 + 400;
    }
    int adjType1 = weaponType1;
    if (weaponType1 != -1) {
        adjType1 = weaponType1 + 400;
    }

    // Play sound based on SKKey
    DWORD effectFlags = 0;
    if ((char)SKKey == '4') {
        effectFlags = 2;
        PlayBuffer(0x41, (DWORD)o, 0);
    }

    // Set CurrentSkill
    DAT_05826d10 = (DWORD)(SKKey & 0xFF);

    if ((char)SKKey == '3') {
        PlayBuffer(0x65, (DWORD)o, 0);
    }

    // Pointers for CreateEffect calls
    float* oPos   = (float*)(o + 0x10);
    float* oAngle = (float*)(o + 0x1C);
    float* oLight = (float*)(o + 0xE8);
    WORD   oPKKey = *(WORD*)(o + 0x86);

    // First switch: by raw weapon type — select projectile effect
    // weaponType0 - 0x218 offset means actual item type values:
    //   0 = type 0 (sword-class bow)  -> effect 0xD9
    //   1 = type 1 (crossbow)         -> effect 0xD9
    //   2 = type 2 (siege crossbow)   -> effect 0xDD
    //   3 = type 3 (spirit bow)       -> effect 0xDB
    //   4 = type 4 (rapid bow)        -> effect 0xDA
    //   5 = type 5 (elven bow)        -> effect 0xE0
    //   6 = type 6 (crystal bow)      -> effect 0xDF
    //   8 = type 8 (divine bow)       -> effect 0xF2
    //   10 = type 10 (legend bow)     -> effect 0xF2
    switch (weaponType0) {
    case 0:
        Effect_Create(0xD9, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 1:
        Effect_Create(0xD9, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 2:
        Effect_Create(0xDD, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 3:
        Effect_Create(0xDB, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 4:
        Effect_Create(0xDA, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 5:
        Effect_Create(0xE0, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 6:
        Effect_Create(0xDF, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 8:
        Effect_Create(0xF2, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    case 10:
        Effect_Create(0xF2, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        break;
    default:
        break;
    }

    // Second switch: by adjusted weapon type (+400 = model index)
    // Determines trail/secondary effect
    switch (adjType1) {
    case 0x210: // 528 (crossbow model range)
    case 0x211:
    case 0x213:
    case 0x214:
    case 0x215:
        Effect_Create(0xD8, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        return;

    case 0x212:
        Effect_Create(0xDC, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        return;

    case 0x216:
        Effect_Create(0xDE, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        return;

    case 0x221:
        Effect_Create(0xF3, oPos, oAngle, oLight,
                     (float*)(intptr_t)effectFlags, (float*)o,
                     (float*)(intptr_t)(int)oPKKey, (float*)(intptr_t)(int)SkillIndex,
                     (BYTE)Skill);
        return;

    default:
        break;
    }
}

// IDA: FUN_00474BD0 @ 0x00474BD0 — CreateArrows (~144 lines), multi-arrow skill handler.
// SKKey '3'/'4': single arrow + counter increment.
// Skill 1: triple shot (center + 2 angle offsets).
// Skill 2: five-way shot with 4 additional angle offsets.
void __cdecl CreateArrows_stub(DWORD c, DWORD o, DWORD to, WORD SkillIndex, WORD Skill, WORD SKKey) {
    // 0x00474BD0 — Multi-arrow skill handler.
    // SKKey '3' or '4': single arrow + increment arrow counter in CharacterMachine.
    // Skill 1: triple shot (center + 2 angle offsets via _DAT_00552834 / _DAT_0055284c).
    // Skill 2: five-way shot with 4 additional angle offsets via _DAT_00552ab0 / _DAT_00552584.
    // Anti-tamper hash table operations surround the arrow counter increment — skipped.

    // o->Angle[2] is at offset +0x1C in OBJECT struct (Angle[0]=+0x14, Angle[1]=+0x18, Angle[2]=+0x1C)
    // Actually from Ghidra: o->Angle[2] direct field access. OBJECT is opaque DWORD here.
    // Angle[2] offset in OBJECT: from struct layout, Angle starts at +0x14 (3 floats after Position[3] at +0x08)
    // But in Ghidra the struct has Position[0..2] at +0x08 and Angle[0..2] at +0x14.
    // o->Angle[2] = *(float*)(o + 0x1C)

    float* pAngleZ = (float*)(o + 0x1C);  // o->Angle[2]

    if ((char)SKKey == '4' || (char)SKKey == '3') {
        // Single arrow + counter increment
        CreateArrow_stub(c, o, to, SkillIndex, Skill, SKKey);

        // anti-tamper hash table — skipped (encrypt CharacterMachine)
        // Increment arrow counter at CharacterMachine + 0x160
        // CharacterMachine = DAT_07cf1ffc
        char* cm = (char*)(DWORD)DAT_07cf1ffc;
        cm[0x160] = cm[0x160] + 1;
        // anti-tamper hash table — skipped (decrypt CharacterMachine)
    }
    else {
        // Multi-arrow based on Skill level
        CreateArrow_stub(c, o, to, SkillIndex, Skill, SKKey);

        if ((char)Skill == 1) {
            // Triple shot: center already created, now +offset and -offset
            *pAngleZ += _DAT_00552834;
            CreateArrow_stub(c, o, to, SkillIndex, 1, SKKey);
            *pAngleZ -= _DAT_0055284c;  // goes to negative offset from original
            CreateArrow_stub(c, o, to, SkillIndex, 1, SKKey);
            *pAngleZ += _DAT_00552834;  // restore to original angle

            // anti-tamper hash table — skipped (encrypt CharacterMachine)
            char* cm = (char*)(DWORD)DAT_07cf1ffc;
            cm[0x160] = cm[0x160] + 1;
            // anti-tamper hash table — skipped (decrypt CharacterMachine)
        }
        else if ((char)Skill == 2) {
            // Five-way shot: center already created, now 4 additional angles
            *pAngleZ += _DAT_00552ab0;
            CreateArrow_stub(c, o, to, SkillIndex, 2, SKKey);
            *pAngleZ += _DAT_00552ab0;
            CreateArrow_stub(c, o, to, SkillIndex, 2, SKKey);
            *pAngleZ -= _DAT_00552584;   // jump to negative side
            CreateArrow_stub(c, o, to, SkillIndex, 2, SKKey);
            *pAngleZ -= _DAT_00552ab0;
            CreateArrow_stub(c, o, to, SkillIndex, 2, SKKey);
            *pAngleZ += _DAT_00552834;   // restore to original angle

            // anti-tamper hash table — skipped (encrypt CharacterMachine)
            char* cm = (char*)(DWORD)DAT_07cf1ffc;
            cm[0x160] = cm[0x160] + 1;
            // anti-tamper hash table — skipped (decrypt CharacterMachine)
        }
    }
}
