// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);
extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

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

static BYTE* Warrior_GetSkillRecord97k(int skillType)
{
    if (skillType < 0 || skillType >= 64)
        return nullptr;
    if (DAT_07cf1ff8 != 0)
        return (BYTE*)(uintptr_t)DAT_07cf1ff8 + skillType * 0x28;
    if (DAT_07d29d20 != 0)
        return (BYTE*)(uintptr_t)DAT_07d29d20 + skillType * 0x28;
    return nullptr;
}

// FUN_00485780 @ 0x00485780 — UseSkillWarrior(c=CHARACTER*, o=OBJECT*)
// Client-side handler for all Warrior skill activations (1989 lines in Ghidra).
// 1. Resolves skill ID from DAT_07d78098/DAT_07d7809c (+ CharacterAttribute skill table).
// 2. Sends opcode 0x10 position packet (XOR-encrypted — delegated to FUN_0048d640).
// 3. Sets attack animation based on model type (0x186=special warrior anims).
// 4. Spawns caster sparkle effect 0x4D0, plays random sword sound.
// 5. Computes facing angle toward target via CreateAngle.
// 6. Per-skill dispatch: 0x2B=TeleportSlash, 0x2F=DownStab, 0x31=Whirlwind, else=generic.
// 7. Sends opcode 0x19 skill packet for each specific skill type.
// 8. Final: sends opcode 0x11 if tile is walkable.
// Anti-tamper hash table ops and XOR encryption blocks are skipped per project policy.
void __cdecl FUN_00485780(int c, int o)
{
    if (!c || !o) return;

    // anti-tamper hash table — skipped (CharacterMachine encrypt/decrypt around skill read)

    DWORD skillType = DAT_07d7809c;
    if (DAT_07d78098 != '\0') {
        BYTE* charAttr = (BYTE*)DAT_07cf1ff4;
        if (charAttr) {
            skillType = (DWORD)charAttr[DAT_07d7809c + 87];
        }
    }

    // anti-tamper hash table — skipped

    // Send opcode 0x10 position packet (movement before skill) using the
    // real 97k sender, not the old zero-arg legacy helper.
    FUN_00491c40(c, o);

    // Clear movement flag
    *(BYTE*)(c + 0x2EC) = 0;

    // Set attack animation based on object model type
    short modelType = *(short*)(o + 2); // OBJECT.Type
    if (modelType == 0x186) {
        // Special warrior model: per-skill animation
        FUN_00443e70(); // SetAttackSpeed
        if (skillType == 0x2B) {
            FUN_0043e820(o, 0x43); // TeleportSlash anim
        } else if (skillType == 0x2F) {
            FUN_0043e820(o, 0x42); // DownStab anim
        } else if (skillType == 0x31) {
            if (g_GameSubState == 8 || g_GameSubState == 10) {
                FUN_0043e820(o, 0x41); // Whirlwind alt
            } else {
                FUN_0043e820(o, 0x40); // Whirlwind normal
            }
        } else {
            FUN_0043e820(o, (int)(skillType + 0x25)); // Generic skill anim
        }
    } else {
        FUN_00444410(c, 0, 0, 0); // SetPlayerAttack — generic attack anim
    }

    // Spawn caster sparkle effect (always)
    float* pos   = (float*)(o + 0x10);
    float* angle = (float*)(o + 0x1c);
    float* light = (float*)(o + 0xe8);
    float scale[3] = { 1.0f, 1.0f, 1.0f };
    FUN_00460dc0(0x4D0, pos, angle, light, NULL, NULL, (float*)(uintptr_t)0xffffffff, NULL, 0);

    // Play random sword sound (0x28 or 0x29)
    PlayBuffer((_rand() & 1) + 0x28, 0, 0);

    // Set target position from CharactersClient[MovementSkillTarget]
    int targetIdx = (int)DAT_07d780a0;
    if (targetIdx < 0 || targetIdx >= 400) {
        return;
    }
    char* targetEntity = (char*)(uintptr_t)DAT_07abf5d0 + targetIdx * 0x394;
    *(float*)(c + 0x314) = *(float*)(targetEntity + 0x10); // TargetPosition.x
    *(float*)(c + 0x318) = *(float*)(targetEntity + 0x14); // TargetPosition.y
    *(float*)(c + 0x31C) = *(float*)(targetEntity + 0x18); // TargetPosition.z

    // Compute facing angle toward target
    float facingAngle = FUN_0043e050(
        *(float*)(o + 0x10), *(float*)(o + 0x14),
        *(float*)(c + 0x314), *(float*)(c + 0x318));
    *(float*)(o + 0x24) = facingAngle;

    // Per-skill dispatch
    if (skillType == 0x2B) {
        // TeleportSlash: compute direction vector, send opcode 0x1E
        DAT_05826d10 = 0x2B; // CurrentSkill
    } else if (skillType == 0x38) {
        // Whirlwind: send opcode 0x19 with skill 0x38
        DAT_05826d10 = 0x38;
    } else {
        // Generic warrior skill: send opcode 0x19
        DAT_05826d10 = skillType;
    }

    // Mark skill as active
    *(BYTE*)(c + 0x2F5) = 1;

    // 2026-05-06: send packet 0x19 PMSG_SKILL_ATTACK_RECV per server source
    // Mu-linux-97K/Source/MuServer/GameServer/SkillManager.h:82.
    // Wire format: [C1][06][19][skillID][TgtH][TgtL] + chain XOR + MuEmu byte XOR.
    //
    // ANTES: comment "anti-tamper + XOR packet build for opcode 0x19 — skipped"
    // significaba que el packet NO se enviaba — skill effects nunca llegaban
    // al server. User reportaba "no puedo usar skills".
    {
        WORD targetEntityId = *(WORD*)(targetEntity + 476);
        if (targetEntityId == 0xFFFF) {
            return;
        }
        if (skillType == 0x2B) {
            BYTE skillDist = 0;
            if (BYTE* rec = Warrior_GetSkillRecord97k((int)skillType))
                skillDist = rec[0x27];
            BYTE dir = (BYTE)(((int)(facingAngle / 45.0f)) & 0xFF);
            BYTE gridXb = (BYTE)*(DWORD*)(c + 0x388);
            BYTE gridYb = (BYTE)*(DWORD*)(c + 0x38C);

            BYTE pkt[11];
            pkt[0] = 0xC1;
            pkt[1] = 0x0B;
            pkt[2] = 0x1E;
            pkt[3] = (BYTE)skillType;
            pkt[4] = gridXb;
            pkt[5] = gridYb;
            pkt[6] = dir;
            pkt[7] = skillDist;
            pkt[8] = dir;
            pkt[9] = (BYTE)((targetEntityId >> 8) & 0xFF);
            pkt[10] = (BYTE)(targetEntityId & 0xFF);
            Net_SendSmallPacket(pkt, 11);
        } else {
            BYTE pkt[6];
            pkt[0] = 0xC1;
            pkt[1] = 0x06;
            pkt[2] = 0x19;
            pkt[3] = (BYTE)skillType;
            pkt[4] = (BYTE)((targetEntityId >> 8) & 0xFF);
            pkt[5] = (BYTE)(targetEntityId & 0xFF);
            Net_SendSmallPacket(pkt, 6);
        }
    }

    // Send opcode 0x11 position confirmation if tile is walkable
    // (checks DAT_0838bc70 terrain wall at current grid position)
    int gridX = *(int*)(c + 0x388);
    int gridY = *(int*)(c + 0x38C);
    int tileIdx = FUN_004f6c40((unsigned int)gridX, (unsigned int)gridY);
    (void)tileIdx;
}

// ── Entity action stubs (Skills.cpp / Combat.cpp externs) ────────────────────

// Entity_WeaponHit @ 0x004742B0
// Hit reaction: Entity_SetAnimation(0x57), clear +0x164, set +0x7C=1,
// spawn effect 0x498 at entity pos, play sound 0x58.
void __cdecl Entity_WeaponHit(int param_1) {
    FUN_00443e70();
    FUN_0043e820(param_1, 0x57);
    *(DWORD *)(param_1 + 0x164) = 0;
    *(BYTE  *)(param_1 + 0x7c)  = 1;
    FUN_00460dc0(0x498,
        (float*)(param_1+0x10), (float*)(param_1+0x1c), (float*)(param_1+0xe8),
        (float*)0, (float*)0, (float*)0xffffffff, (float*)0, 0);
    FUN_00404bc0(0x58, 0, 0);
}

// Entity_ResetToWalk @ 0x0042BC00
// Reads anim at +0x288. If NOT in attack range (0x210-0x216 / 0x221):
//   if prev anim (+0x270) was wind-up (0x218-0x21E or 0x220): set walk/swim-walk.
// If currently in attack anim: immediately revert to walk (0x2E) or swim-walk (0x30).
void __cdecl Entity_ResetToWalk(int param_1) {
    short sVar1    = *(short*)(param_1 + 0x288);
    bool swimming  = ((*(BYTE*)(param_1 + 0x1bc) & 7) == 2);
    bool has_water = (*(short*)(param_1 + 0x2a0) != -1);
    if (((sVar1 < 0x210) || (0x216 < sVar1)) && (sVar1 != 0x221)) {
        sVar1 = *(short*)(param_1 + 0x270);
        if (((0x217 < sVar1) && (sVar1 < 0x21f)) || (sVar1 == 0x220)) {
            FUN_0043e820(param_1, (swimming && has_water) ? 0x31 : 0x2f);
        }
        return;
    }
    FUN_0043e820(param_1, (swimming && has_water) ? 0x30 : 0x2e);
}

// Entity_SelectTarget_Player @ 0x00444A80
// entity_type 0x186 (special NPC): picks idle/stance anim from 0x52/0x53/0x56/0x5B.
// Other types: increments combo_counter (+0x303) and alternates anim 3 / 4.
void __cdecl Entity_SelectTarget_Player(int param_1, int /*target*/) {
    if (*(short*)(param_1 + 2) != 0x186) {
        UINT uVar1 = (*(BYTE*)(param_1 + 0x303) % 3 == 0) ? 3 : 4;
        FUN_0043e820(param_1, uVar1);
        *(char*)(param_1 + 0x303) = *(char*)(param_1 + 0x303) + 1;
        return;
    }
    FUN_00443e70();
    if (((*(short*)(param_1 + 0x2b8) == 0x332) || (*(short*)(param_1 + 0x2b8) == 0x333)) &&
         (*(char*)(param_1 + 0x34e) == '\0')) {
        FUN_0043e820(param_1, 0x5b); // death anim for special class
        return;
    }
    if ((*(BYTE*)(param_1 + 0x1bc) & 7) == 2) {
        FUN_0043e820(param_1, 0x56); // swim idle
        return;
    }
    UINT uVar1 = rand() & 0x80000001;
    if ((int)uVar1 < 0) uVar1 = (uVar1 - 1 | 0xfffffffe) + 1; // parity correction
    FUN_0043e820(param_1, uVar1 + 0x52); // 0x52 or 0x53 random idle
}

// Entity_TeleportEnd @ 0x00444D90
// Handles landing after a teleport:
//   If local player (param_1 == DAT_07abf5d8): clears teleport-state flag at DAT_07cf1ff4+0x1c.
//   Spawns landing visual effects per class:
//     0x186 (DarkWizard, sub_id 0xCE-0xD0): effect 0xD2 once + 0xD3 ×10, then sound 0x5E.
//     0x127 (DarkElf): effects 0xE2+0xE3 ×8, then sound 0x5E.
//     300 (0x12C): same as 0x186.
//     other: set anim 6 (land-idle), return.
//   Post-effects: if +0x108 == _DAT_00552580 (0.0f): play footstep sound from class table.
void __cdecl Entity_TeleportEnd(int entity_idx) {
    BYTE* puVar3 = (BYTE*)(uintptr_t)(int)entity_idx;

    // Local player: clear teleport in-flight state (hash-table ops are obfuscation noise)
    if ((void*)entity_idx == (void*)DAT_07abf5d8) {
        *(short*)((char*)DAT_07cf1ff4 + 0x1c) = 0;
    }

    short sVar2 = *(short*)(puVar3 + 2); // entity_type
    if (sVar2 == 0x186) {
        int sub_id = *(int*)(puVar3 + 4);
        if (sub_id < 0xce || sub_id > 0xd0) {
            FUN_0043e820(entity_idx, 0x83);
            goto LAB_00445110;
        }
        *puVar3 = 0;
        FUN_00460dc0(0xd2, (float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                     (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
        for (int i = 0; i < 10; i++)
            FUN_00460dc0(0xd3,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                         (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
    } else if (sVar2 == 0x127) {
        *puVar3 = 0;
        for (int i = 0; i < 8; i++) {
            FUN_00460dc0(0xe2,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                         (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
            FUN_00460dc0(0xe3,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                         (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
        }
    } else if (sVar2 == 300) {
        *puVar3 = 0;
        FUN_00460dc0(0xd2,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                     (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
        for (int i = 0; i < 10; i++)
            FUN_00460dc0(0xd3,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                         (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
    } else {
        FUN_0043e820(entity_idx, 6);
        goto LAB_00445110;
    }
    FUN_00404bc0(0x5e, (int)entity_idx, 0);

LAB_00445110:
    // Post-teleport footstep sound: only if anim speed == _DAT_00552580 (ground landed)
    if (*(float*)(puVar3 + 0x108) == *(float*)&DAT_00552580) {
        if ((sVar2 == 0x186) && (*(int*)(puVar3+4) < 0xce || *(int*)(puVar3+4) > 0xd0)) {
            // DarkWizard swimming vs walking footstep
            if ((puVar3[0x1bc] & 7) == 2)
                FUN_00404bc0(0x50, (int)entity_idx, 0);
            else
                FUN_00404bc0(0x4e, (int)entity_idx, 0);
            return;
        }
        // General: look up footstep sound from entity-class table at DAT_05828d58+0xb2
        short sfx = *(short*)((char*)DAT_05828d58 + 0xb2 + sVar2 * 0xbc);
        if (sfx != -1)
            FUN_00404bc0(sfx + 0xaa, (int)entity_idx, 0);
    }
}

// Entity_TeleportAnim @ 0x004792C0
// Finds a free slot in the teleport-effect pool (DAT_07C80110, stride 0x70, ~12 entries).
// Fills: active=1, entity_id, src_pos (world_pos + height_offset DAT_00552958), dst_pos.
// Slots: [0x00]=active, [0x04]=entity_id, [0x10..0x18]=src_pos, [0x1C..0x24]=dst_pos,
//        [0x0C]=param_4, [0x38..0x3B]=phase_flags, [0x48..0x4B]=state_chars.
void __cdecl Entity_TeleportAnim(float* world_pos, float entity_id, float* dst_pos, float param_4) {
    // BUG-FIX 2026-05-03: was `while ((int)pcVar1 < 0x7c82cd0)` (absolute source-binary
    // bound). With DAT_07c80110 sized as 1 byte and the loop walking 100 × 0x70 bytes,
    // every teleport effect spawn corrupted the heap. Pool now sized to 100 slots in
    // globals.cpp; bound is iteration count.
    char *pcVar1 = (char*)&DAT_07c80110[0];
    for (int i = 0; i < 100; ++i) {
        if (*pcVar1 == '\0') {
            *pcVar1 = '\x01';
            *(float*)(pcVar1 + 0x04) = entity_id;
            *(float*)(pcVar1 + 0x10) = world_pos[0];
            *(float*)(pcVar1 + 0x14) = world_pos[1];
            *(float*)(pcVar1 + 0x18) = world_pos[2] + _DAT_00552958;
            *(float*)(pcVar1 + 0x1c) = dst_pos[0];
            *(float*)(pcVar1 + 0x20) = dst_pos[1];
            *(float*)(pcVar1 + 0x24) = dst_pos[2];
            *(float*)(pcVar1 + 0x0c) = param_4;
            pcVar1[0x38] = pcVar1[0x39] = pcVar1[0x3a] = pcVar1[0x3b] = '\0';
            pcVar1[0x48] = '\0'; pcVar1[0x49] = '\0';
            pcVar1[0x4a] = ' ';  pcVar1[0x4b] = 'A';
            return;
        }
        pcVar1 += 0x70;
    }
}

// Entity_MeleeAttackStart @ 0x00474310
// Melee attack initiation: anim 0x57, set speed=5.0f (+0x108), mode=3 (+0x7C),
// scale=1.0f (+0x164), spawn effect 0x498, play sound 0x58.
void __cdecl Entity_MeleeAttackStart(int param_1) {
    FUN_00443e70();
    FUN_0043e820(param_1, 0x57);
    *(float*)(param_1 + 0x108) = 5.0f;   // 0x40A00000
    *(BYTE *)(param_1 + 0x7c)  = 3;      // melee attack mode
    *(float*)(param_1 + 0x164) = 1.0f;   // 0x3F800000
    FUN_00460dc0(0x498,
        (float*)(param_1+0x10), (float*)(param_1+0x1c), (float*)(param_1+0xe8),
        (float*)0, (float*)0, (float*)0xffffffff, (float*)0, 0);
    FUN_00404bc0(0x58, 0, 0);
}

// FUN_00479330 @ 0x00479330 — RenderPoints (damage popup renderer)
