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

static void Warrior_SendSkill19(BYTE skill, WORD targetKey)
{
    BYTE packet[6] = { 0xC1, 6, 0x19, skill,
                       (BYTE)(targetKey >> 8), (BYTE)targetKey };
    Net_SendSmallPacket(packet, sizeof(packet));
}

// IDA: UseSkillWarrior 0x485780, L682-908 -- el sitio que el DLL de inyeccion
// hookea como `SendContinueDeathStab` (Patchs.cpp, 0x00486136).
// `packedOffset` es IDA v316.
//
// 2026-09-03 -- DESVIACION DE PROTOCOLO (servidor MuEmu), la misma que ya
// aplican `Combat_SendDuration1E_97k` y `SendSkillPacket1E_Local`:
// el 0.97k vanilla arma 9 bytes y NO manda la key del objetivo, pero
// PMSG_DURATION_SKILL_ATTACK_RECV (GameServer/SkillManager.h:96) son 11 y el
// server lee `index[]` SIEMPRE:
//     short bIndex = MAKE_NUMBERW(lpMsg->index[0], lpMsg->index[1]);
//     this->UseDurationSkillAttack(..., bIndex, ...);   // SkillManager.cpp:2045
// Con 9 bytes lee esos dos bytes FUERA del paquete: `bIndex` sale basura y
// `MultiSkillAttack -> BasicSkillAttack(aIndex, bIndex, ...)` le pega a otra
// entidad o a ninguna.  Este era el UNICO de los tres emisores de C3:1E que
// habia quedado en la forma vanilla -- descartaba `targetKey` con un
// `(void)targetKey` explicito.
static void Warrior_SendSkill1E(BYTE skill, BYTE x, BYTE y, BYTE direction,
                                BYTE packedOffset, BYTE angle, WORD targetKey)
{
    BYTE packet[11] = { 0xC1, 11, 0x1E, skill, x, y, direction, packedOffset, angle,
                        (BYTE)((targetKey >> 8) & 0xFF), (BYTE)(targetKey & 0xFF) };
    Net_SendSmallPacket(packet, sizeof(packet));
}

// IDA: sub_45FDB0 @ 0x45FDB0.  The warrior's 43 branch starts with the
// selected target and appends nearby monsters or allied player objects,
// without duplicate keys.  `centre` corresponds to IDA a1; `relationKey` is
// IDA a3; `keys`/`count` are IDA a4/a5.  The native key list has *DWORD*
// stride (despite every entry holding only a signed 16-bit object key).
static void Warrior_CollectMultiTargets(const float* centre, float radius,
                                        WORD relationKey, int* keys, int* count, int maximum)
{
    if (!centre || !keys || !count || *count >= maximum || DAT_07abf5d0 == 0)
        return;

    const float radiusSquared = radius * radius;
    BYTE* characters = (BYTE*)(uintptr_t)DAT_07abf5d0;
    for (int index = 0; index < 400 && *count < maximum; ++index) {
        BYTE* candidate = characters + index * 0x394;
        if (!candidate[0] || !candidate[0x160] || candidate == (BYTE*)(uintptr_t)DAT_07abf5d8 || candidate[0x2FD])
            continue;

        const int key = *(short*)(candidate + 0x1DC); // IDA: *(__int16 *)(v9 + 456)
        const BYTE type = candidate[0x84]; // IDA: v10 = *(_BYTE *)(v9 + 112)
        if (type != 2 && (type != 1 || key != relationKey))
            continue;

        const float dx = *(float*)(candidate + 0x10) - centre[0];
        const float dy = *(float*)(candidate + 0x14) - centre[1];
        // IDA v16/v17 use the X/Y plane only; Z is intentionally ignored.
        if (dx * dx + dy * dy > radiusSquared)
            continue;

        bool duplicate = false;
        for (int item = 0; item < *count; ++item) {
            if (keys[item] == key) { duplicate = true; break; }
        }
        if (!duplicate)
            keys[(*count)++] = key;
    }
}

static void Warrior_SendMultiSkill(BYTE skill, const float* centre, BYTE serial,
                                   const int* keys, int count)
{
    if (!centre || !keys || count <= 0) return;
    if (count > 6) count = 6;
    BYTE packet[5 + 2 + 2 + 1 + 1 + 12] = {};
    const BYTE tileX = (BYTE)(int)(centre[0] * 0.01f);
    const BYTE tileY = (BYTE)(int)(centre[1] * 0.01f);
    const int length = 8 + count * 2;
    packet[0] = 0xC1; packet[1] = (BYTE)length; packet[2] = 0x1D;
    packet[3] = skill; packet[4] = tileX; packet[5] = tileY;
    packet[6] = serial; packet[7] = (BYTE)count;
    for (int item = 0; item < count; ++item) {
        packet[8 + item * 2] = (BYTE)(keys[item] >> 8);
        packet[9 + item * 2] = (BYTE)keys[item];
    }
    Net_SendSmallPacket(packet, length);
}

// IDA: UseSkillWarrior @ 0x00485780 (2225 lineas de decompile)
//   void __cdecl UseSkillWarrior(int c, int o)   — c = CHARACTER*, o = OBJECT*
// Correspondencia de nombres con el decompile:
//   skillType    = v317   (id de skill resuelto en L366-373)
//   selectedSlot = dword_7D7809C (0x07D7809C, el slot que siembra Attack)
//   targetIdx    = MovementSkillTarget (0x07D780A0)
//   targetEntity = CharactersClient + 916 * MovementSkillTarget
//   targetKey    = v285   (*(__int16 *)(targetEntity + 476))
//   centre       = Angle[3] (L624-642, el centro desplazado 120u del multiataque)
//   keys/count   = &v285 / SkillIndex (el par que llena sub_45FDB0)
//   serial       = *(BYTE *)(CharacterMachine + 1408)  ->  o+136
// Client-side handler for all Warrior skill activations (1989 lines in Ghidra).
// 1. Resolves skill ID from DAT_07d78098/DAT_07d7809c (+ CharacterAttribute skill table).
// 2. Sends the compact C1:06:10 facing packet built locally at 0x485815.
// 3. Sets attack animation based on model type (0x186=special warrior anims).
// 4. Spawns caster sparkle effect 0x4D0, plays random sword sound.
// 5. Computes facing angle toward target via CreateAngle.
// 6. Per-skill dispatch: 0x2B=TeleportSlash, 0x2F=DownStab, 0x31=Whirlwind, else=generic.
// 7. Sends opcode 0x19 skill packet for each specific skill type.
// 8. Final: sends opcode 0x11 if tile is walkable.
// Anti-tamper hash table ops and XOR encryption blocks are skipped per project policy.
void __cdecl Combat_UseWarriorSkill(int c /* IDA: c */, int o /* IDA: o */)
{
    if (!c || !o) return;
    BYTE* character = (BYTE*)(uintptr_t)c; // IDA: c / CHARACTER state buffer
    BYTE* object = (BYTE*)(uintptr_t)o;    // IDA: o / visual OBJECT
    BYTE* attributes = (BYTE*)(uintptr_t)DAT_07cf1ff4; // IDA: CharacterAttribute @ 0x07CF1FF4
    const BYTE selectedSlot = (BYTE)DAT_07d7809c; // IDA: dword_7D7809C
    const BYTE skillType = (DAT_07d78098 && attributes) // IDA: v317
        ? attributes[selectedSlot + 87] : selectedSlot;

    // 0x485815: this is a compact facing packet, not Send_MovePacket.
    BYTE movePacket[6] = { 0xC1, 6, 0x10,
        *(BYTE*)(character + 904), *(BYTE*)(character + 908),
        (BYTE)(16 * (((int)((*(float*)(object + 36) + 22.5f) * 0.022222223f + 1.0f)) & 7)) };
    Net_SendC1Packet(movePacket, sizeof(movePacket));
    *(BYTE*)(c + 0x2EC) = 0;

    if (*(short*)(o + 2) == 390) {
        FUN_00443e70(); // SetAttackSpeed
        if (skillType == 43) {
            FUN_0043e820(o, 67);
        } else if (skillType == 47) {
            FUN_0043e820(o, 66);
        } else if (skillType == 49) {
            if (g_GameSubState == 8 || g_GameSubState == 10) {
                FUN_0043e820(o, 65);
            } else {
                FUN_0043e820(o, 64);
            }
        } else {
            // IDA intentionally uses the selected slot here, not v317.
            FUN_0043e820(o, selectedSlot + 37);
        }
    } else {
        FUN_00444410(c, 0, 0, 0);
    }

    // IDA L597-601:
    //   Light[0] = Light[1] = Light[2] = 1.0;
    //   Particle_Spawn(1232, (float *)(o + 16), (float *)(o + 28), Light, 0, 0.0, o);
    // El port llamaba `Effect_Create(0x4D0, ...)` (= CreateEffect @0x00460DC0),
    // que es OTRA funcion, y ademas pasaba `o+0xE8` como luz en vez del {1,1,1}
    // local.  1232 = 0x4D0 es un tipo de PARTICULA, no de efecto.
    float Light[3] = { 1.0f, 1.0f, 1.0f };            // IDA: Light[3]
    Particle_Spawn(1232, (float*)(o + 16), (float*)(o + 28), Light, 0, 0.0f, o);

    // Play random sword sound (0x28 or 0x29)
    PlayBuffer((_rand() & 1) + 0x28, 0, 0);

    int targetIdx = (int)DAT_07d780a0; // IDA: MovementSkillTarget @ 0x07D780A0
    // Native code indexes CharactersClient directly at 0x4859A9.  Attack and
    // Action establish this index before entering this helper; adding a local
    // rejection here changes the original tail (state + C1:11 confirmation).
    char* targetEntity = (char*)(uintptr_t)DAT_07abf5d0 + targetIdx * 0x394; // IDA: v285 source
    *(float*)(c + 0x314) = *(float*)(targetEntity + 0x10); // TargetPosition.x
    *(float*)(c + 0x318) = *(float*)(targetEntity + 0x14); // TargetPosition.y
    *(float*)(c + 0x31C) = *(float*)(targetEntity + 0x18); // TargetPosition.z

    *(float*)(o + 36) = FUN_0043e050(
        *(float*)(o + 0x10), *(float*)(o + 0x14),
        *(float*)(c + 0x314), *(float*)(c + 0x318));
    const WORD targetKey = *(WORD*)(targetEntity + 476); // IDA: v285
    // There is deliberately no key-validity early return here.  The native
    // CFG at 0x485A2B continues through every family and serializes v285 as
    // read, including the sentinel value when the selected slot is stale.
    if (skillType == 43) {
            DAT_07e11d84 = GetTickCount();
            float step[3] = { *(float*)(c + 788) - *(float*)(o + 16),
                              *(float*)(c + 792) - *(float*)(o + 20),
                              *(float*)(c + 796) - *(float*)(o + 24) }; // IDA: v320/v321/v322
            float length = sqrtf(step[0] * step[0] + step[1] * step[1] + step[2] * step[2]);
            if (length < 1.0f) length = 1.0f;
            step[0] *= 120.0f / length;
            step[1] *= 120.0f / length;
            step[2] *= 120.0f / length;
            float centre[3] = { *(float*)(o + 16) + step[0],
                                *(float*)(o + 20) + step[1],
                                *(float*)(o + 24) + step[2] }; // IDA: Angle after first loop pass
            DAT_05826d10 = 43;                        // IDA L680: dword_5826D10 = 0x2B
            // IDA L687-690 / L738 / L780: los dos bytes de grilla salen de
            // `c + 904` y `c + 908` (la grilla del HEROE), no de la posicion de
            // mundo del objetivo (c+788/792) que usaba el port.
            const BYTE tileX = (BYTE)*(DWORD*)(c + 904);   // IDA: *(_DWORD *)v39
            const BYTE tileY = (BYTE)*(DWORD*)(c + 908);   // IDA: LOBYTE(x2), v38
            // IDA L818: v50 = (__int64)(*(float *)(o + 36) * 0.71111113) — el mismo
            // paso de 256/360 que usan el resto de los constructores C3:1E del port.
            // El port aplicaba aca el empaquetado de 8 direcciones (16*(a&7)), que
            // en el binario solo se usa para el paquete de movimiento 0x10.
            const BYTE direction = (BYTE)(int)(*(float*)(o + 36) * (256.0f / 360.0f));
            // Exact odd-looking native packing at 0x48608B..0x4860A6:
            // both nibbles derive from targetX - heroGridX (IDA: v316).
            const BYTE targetTileX = (BYTE)(int)(*(float*)(c + 788) * 0.01f); // IDA: LOBYTE(x2)
            const BYTE deltaX = (BYTE)(targetTileX - tileX);
            const BYTE packedOffset = (BYTE)(((deltaX + 8) << 4) | ((deltaX - 8) & 0x0F));
            Warrior_SendSkill1E(skillType, tileX, tileY, direction, packedOffset, 0, targetKey);

            int keys[6] = { (short)targetKey }; // IDA: int v285, DWORD stride
            int count = 1;
            Warrior_CollectMultiTargets(centre, 100.0f, *(WORD*)(o + 134), keys, &count, 6);
            // UseSkillWarrior executes this loop twice (IDA 0x486C17): the
            // second probe is another 120 units forward, and that final
            // Angle is also the position serialized into C1:1D.
            centre[0] += step[0];
            centre[1] += step[1];
            centre[2] += step[2];
            Warrior_CollectMultiTargets(centre, 100.0f, *(WORD*)(o + 134), keys, &count, 6);
            const BYTE serial = *(BYTE*)(character + 1408);
            *(BYTE*)(character + 1408) = serial + 1;
            *(BYTE*)(o + 136) = serial;
            Warrior_SendMultiSkill(attributes ? attributes[*(BYTE*)(character + 913) + 87] : skillType,
                                   centre, *(BYTE*)(o + 136), keys, count);
    } else if (skillType == 56) {
            float angle[3] = { *(float*)(o + 28), *(float*)(o + 32), *(float*)(o + 36) - 40.0f };
            int skillIndex = 0;
            if (attributes) for (; skillIndex < 20 && attributes[skillIndex + 87] != 56; ++skillIndex) {}
            for (int part = 0; part < 5; ++part) {
                Effect_Create(203, (float*)(o + 16), angle, (float*)(o + 232),
                              (float*)(intptr_t)2, (float*)o,
                              (float*)(intptr_t)*(WORD*)(o + 134),
                              (float*)(intptr_t)skillIndex, 0);
                angle[2] += 20.0f;
            }
    } else if ((DWORD)(GetTickCount() - DAT_05826cf4) > 300) {
            DAT_05826cf4 = GetTickCount();
            Warrior_SendSkill19(skillType, targetKey);
    }

    *(BYTE*)(c + 757) = 1;
    if ((*(DWORD*)(o + 120) & 0x20) == 0) {
        int tileX = (int)(*(float*)(c + 788) * 0.01f);
        int tileY = (int)(*(float*)(c + 792) * 0.01f);
        if (World >= 11 && World <= 16) {
            switch (abs((int)(*(float*)(o + 36) * 0.022222223f)) & 7) {
            case 0: ++tileY; break; case 1: --tileX; ++tileY; break;
            case 2: --tileX; break; case 3: --tileX; --tileY; break;
            case 4: --tileY; break; case 5: ++tileX; --tileY; break;
            case 6: ++tileX; break; case 7: ++tileX; ++tileY; break;
            }
        }
        const BYTE terrain = TerrainWall[((tileY & 0xFF) << 8) | (tileX & 0xFF)];
        if ((terrain & 0x0C) == 0 && skillType != 47 && skillType != 43 && skillType != 49) {
            BYTE confirm[5] = { 0xC1, 5, 0x11, (BYTE)tileX, (BYTE)tileY };
            Net_SendC1Packet(confirm, sizeof(confirm));
        }
    }
}

// ── Entity action stubs (Skills.cpp / Combat.cpp externs) ────────────────────

// Legacy helper only; no canonical FUN mapping retained here. Previous `FUN_004742B0`
// label was incorrect: IDA FUN_004742B0 is documented as CreateTeleportBegin in Entity_LegacyTeleport.cpp.
// Hit reaction: Entity_SetAnimation(0x57), clear +0x164, set +0x7C=1,
// spawn effect 0x498 at entity pos, play sound 0x58.
void __cdecl Entity_WeaponHit(int param_1) {
    FUN_00443e70();
    FUN_0043e820(param_1, 0x57);
    *(DWORD *)(param_1 + 0x164) = 0;
    *(BYTE  *)(param_1 + 0x7c)  = 1;
    Effect_Create(0x498,
        (float*)(param_1+0x10), (float*)(param_1+0x1c), (float*)(param_1+0xe8),
        (float*)0, (float*)0, (float*)0xffffffff, (float*)0, 0);
    FUN_00404bc0(0x58, 0, 0);
}

// IDA: FUN_0042BC00 is named SetPlayerBow in the reference notes; `Entity_ResetToWalk`
// remains this port's descriptive legacy alias.
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

// IDA: SetPlayerMagic @ 0x00444A80 (el nombre real del binario; el alias
// `Entity_SelectTarget_Player` es solo del port y se conserva por los callers).
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

// Legacy helper only; no canonical FUN mapping retained here. `FUN_00444D90` is
// documented as SetPlayerDie in Net/SecondPassword.cpp, so the old address label was misleading.
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
        Effect_Create(0xd2, (float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                     (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
        for (int i = 0; i < 10; i++)
            Effect_Create(0xd3,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                         (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
    } else if (sVar2 == 0x127) {
        *puVar3 = 0;
        for (int i = 0; i < 8; i++) {
            Effect_Create(0xe2,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                         (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
            Effect_Create(0xe3,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                         (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
        }
    } else if (sVar2 == 300) {
        *puVar3 = 0;
        Effect_Create(0xd2,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
                     (float*)0,(float*)0,(float*)0xffffffff,(float*)0, 0);
        for (int i = 0; i < 10; i++)
            Effect_Create(0xd3,(float*)(puVar3+0x10),(float*)(puVar3+0x1c),(float*)(puVar3+0xe8),
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

// Legacy helper only; no canonical FUN mapping retained here. `FUN_004792C0` is
// documented as CreatePoint in Entity_LegacyTeleport.cpp, so the old address label was misleading.
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

// Legacy helper only; no canonical FUN mapping retained here. `FUN_00474310` is
// documented as CreateTeleportEnd in Entity_LegacyTeleport.cpp, so the old address label was misleading.
// Melee attack initiation: anim 0x57, set speed=5.0f (+0x108), mode=3 (+0x7C),
// scale=1.0f (+0x164), spawn effect 0x498, play sound 0x58.
void __cdecl Entity_MeleeAttackStart(int param_1) {
    FUN_00443e70();
    FUN_0043e820(param_1, 0x57);
    *(float*)(param_1 + 0x108) = 5.0f;   // 0x40A00000
    *(BYTE *)(param_1 + 0x7c)  = 3;      // melee attack mode
    *(float*)(param_1 + 0x164) = 1.0f;   // 0x3F800000
    Effect_Create(0x498,
        (float*)(param_1+0x10), (float*)(param_1+0x1c), (float*)(param_1+0xe8),
        (float*)0, (float*)0, (float*)0xffffffff, (float*)0, 0);
    FUN_00404bc0(0x58, 0, 0);
}

// FUN_00479330 @ 0x00479330 — RenderPoints (damage popup renderer)
