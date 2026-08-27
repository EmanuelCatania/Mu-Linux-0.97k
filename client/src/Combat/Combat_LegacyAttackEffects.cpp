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


// FUN_00445230 @ 0x00445230 — AttackEffect(CHARACTER* c)
// Spawns attack-hit effects and plays sounds based on the attacker's entity type.
// Signature in original binary: __stdcall AttackEffect(CHARACTER *c) — 1 param.
// Declared in functions.h as 3 ints; callers must use that convention.
//
// Main switch on *(short*)(entity+0x02) (Object.Type / MonsterIndex):
//   0x23 '#' DK melee    → CreateEffect(0xBF, pos) + PlayBuffer(0x2E)
//   0x26 '&' DK sword    → CreateEffect(0xC8/0xC9, pos) + PlayBuffer(0x59)
//   0x2A '*' Elf bow     → BMD_TransformPosition(bone 0xB, pos)
//   0x2D '-' Elf arrow   → BMD_TransformPosition(bone 2, pos)
//   0x35 '5' DW          → CreateEffect(0x238, pos)×18 + PlayBuffer
//   0x3D '=' Summoner    → CreateEffect(0xF1, pos) + CreateEffect(0xF0/0x238, pos)
//   0x42 'B'             → CreateEffect(0xF1, pos)
//   0x46 'F' AoE         → CreateEffect(0x4F7, targetPos)×20
//   0x48 'H' Lightning   → CreateJoint(0x4E5, ...)×36
//   0x4D 'M' Lightning   → CreateJoint(0x4E5, ...)×40
//   (most others)        → CreateEffect(0xBF, pos) + PlayBuffer(0x2E)
// Second section: target effects for skill category 0x11 (direct) and 0x03 (magic).
// 2026-08-16: version PARCIAL (374 lineas). Desactivada a favor del port fiel
// de IDA en stubs_IDA_ports.cpp (2043 lineas), que ahora se activa con
// IDA_PORT_00445230 en globals.h.
#if !defined(IDA_PORT_00445230)
void __cdecl FUN_00445230(int entity)
{
    if (!entity) return;
    const BYTE attackEffectKind = *(BYTE*)(entity + 747);
    float* pos   = (float*)(entity + 0x10);  // world position
    float* angle = (float*)(entity + 0x1c);  // world angle
    float* light = (float*)(entity + 0xe8);  // light/color

    // 00445230 reads the queued skill through FUN_0045fae0. +770 is a
    // hash-table protected byte in the original client; comparing its storage
    // directly made this path depend on the encoded value instead of the skill.
    const BYTE queuedSkill = FUN_0045fae0((DWORD)&DAT_055c9bc8,
                                          (BYTE*)(entity + 770));

    // First switch of 00445230 (lines 293..1689): this selector is +747,
    // not the model type. `frame` is +757 and `action` is +261.
    const BYTE frame = *(BYTE*)(entity + 757);
    const BYTE action = *(BYTE*)(entity + 261);
    const DWORD boneMatricesTop = *(DWORD*)(entity + 276);
    const int modelTypeTop = *(short*)(entity + 2);
    void* modelTop = (modelTypeTop >= 0 && DAT_05828d58)
        ? (void*)(DAT_05828d58 + modelTypeTop * 188) : 0;
    float zero[3] = { 0.0f, 0.0f, 0.0f };
    float world[3] = { 0.0f, 0.0f, 0.0f };
    float effectAngle[3] = { 0.0f, 0.0f, 0.0f };
    float whiteTop[3] = { 1.0f, 1.0f, 1.0f };

    switch (attackEffectKind) {
    case 35:
        if (queuedSkill == 50 && frame == 1) {
            for (int n = 0; n < 18; ++n) {
                effectAngle[0] = 0.0f; effectAngle[1] = 0.0f; effectAngle[2] = n * 20.0f;
                Effect_Create(191, pos, effectAngle, light, (float*)1, (float*)entity, (float*)-1, 0, 0);
            }
            FUN_00404bc0(46, 0, 0);
        }
        break;
    case 38:
    case 67:
        if (queuedSkill == 50) {
            if (frame == 1) {
                Effect_Create(200, pos, angle, light, 0, 0, (float*)-1, 0, 0);
                Effect_Create(201, pos, angle, light, 0, 0, (float*)-1, 0, 0);
                FUN_00404bc0(89, 0, 0);
            }
            world[0] = pos[0] + (float)(rand() % 1024) - 512.0f;
            world[1] = pos[1] + (float)(rand() % 1024) - 512.0f;
            world[2] = pos[2];
            Effect_Create(191, world, angle, light, 0, 0, (float*)-1, 0, 0);
            FUN_00404bc0(46, 0, 0);
        }
        break;
    case 42:
        if (queuedSkill == 50 && frame == 1 && modelTop && boneMatricesTop) {
            BMD_TransformPosition(modelTop, (float*)(boneMatricesTop + 528), zero, world, 1);
            effectAngle[0] = angle[0] - 20.0f; effectAngle[1] = angle[1]; effectAngle[2] = angle[2] - 30.0f;
            Effect_Create(191, world, effectAngle, light, (float*)2, 0, (float*)-1, 0, 0);
            effectAngle[0] = angle[0] - 30.0f; effectAngle[2] = angle[2];
            Effect_Create(191, world, effectAngle, light, (float*)2, 0, (float*)-1, 0, 0);
            effectAngle[0] = angle[0] - 20.0f; effectAngle[2] = angle[2] + 30.0f;
            Effect_Create(191, world, effectAngle, light, (float*)2, 0, (float*)-1, 0, 0);
            FUN_00404bc0(46, 0, 0);
        }
        break;
    case 45:
        if (modelTop && boneMatricesTop) for (int n = 0; n < 4; ++n) {
            zero[0] = (float)(rand() % 32 - 16); zero[1] = (float)(rand() % 32 - 16); zero[2] = (float)(rand() % 32 - 16);
            BMD_TransformPosition(modelTop, (float*)(boneMatricesTop + 96), zero, world, 1);
            Particle_Spawn(1241, world, angle, whiteTop, 0, 1.0f, 0);
            Particle_Spawn(1206, world, angle, whiteTop, 0, 1.0f, 0);
        }
        break;
    case 49:
        if (frame % 5 == 1 && modelTop && boneMatricesTop) {
            zero[0] = zero[1] = zero[2] = 0.0f;
            BMD_TransformPosition(modelTop, (float*)(boneMatricesTop + 3024), zero, world, 1);
            Effect_Create(1211, world, angle, light, 0, 0, (float*)-1, 0, 0);
        }
        if (queuedSkill == 50 && frame == 1) {
            float boltAngle[3] = { angle[0], angle[1], angle[2] + 20.0f };
            float boltPosition[3] = { pos[0], pos[1], pos[2] + 50.0f };
            float boltLight[3] = { 0.42000002f, 0.84000003f, 1.4f };
            for (int n = 0; n < 9; ++n) {
                boltAngle[2] += 40.0f;
                Effect_Create(1210, boltPosition, boltAngle, boltLight, 0, 0, (float*)-1, 0, 0);
            }
        }
        break;
    case 53: case 58: case 59:
        if (frame == 1) Effect_SpawnBombRing(pos);
        if (attackEffectKind == 59 && queuedSkill == 50 && frame == 14) {
            for (int n = 0; n < 18; ++n) {
                effectAngle[0] = angle[0]; effectAngle[1] = angle[1]; effectAngle[2] = angle[2] + n * 20.0f;
                Effect_Create(568, pos, effectAngle, light, 0, 0, (float*)-1, 0, 0);
            }
        }
        break;
    case 54: case 57: case 151:
        if (frame == 1) {
            Effect_Create(223, pos, angle, light, 0, (float*)entity, (float*)-1, 0, 0);
            if (attackEffectKind == 57) {
                effectAngle[0] = angle[0]; effectAngle[1] = angle[1]; effectAngle[2] = angle[2] + 20.0f;
                Effect_Create(223, pos, effectAngle, light, 0, (float*)entity, (float*)-1, 0, 0);
                effectAngle[2] -= 40.0f;
                Effect_Create(223, pos, effectAngle, light, 0, (float*)entity, (float*)-1, 0, 0);
            }
        }
        break;
    case 61:
        if (frame == 1) Effect_Create(241, pos, angle, light, 0, 0, (float*)-1, 0, 0);
        break;
    case 63:
        if (frame == 1) {
            Effect_SpawnBombRing(pos);
            Effect_Create(241, pos, angle, light, 0, 0, (float*)-1, 0, 0);
        }
        if (queuedSkill == 50) {
            world[0] = pos[0] + (float)(rand() % 800) - 400.0f;
            world[1] = pos[1] + (float)(rand() % 800) - 400.0f; world[2] = pos[2];
            Effect_Create(240, world, angle, light, 0, 0, (float*)-1, 0, 0);
            if (frame == 14) for (int n = 0; n < 18; ++n) {
                effectAngle[0] = angle[0]; effectAngle[1] = angle[1]; effectAngle[2] = angle[2] + n * 20.0f;
                Effect_Create(568, pos, effectAngle, light, 0, 0, (float*)-1, 0, 0);
            }
        }
        break;
    case 66:
        if (queuedSkill == 50 && frame == 1)
            Effect_Create(241, pos, angle, light, (float*)1, 0, (float*)-1, 0, 0);
        break;
    case 70:
        if (frame == 5 && *(short*)(entity + 784) >= 0 && *(short*)(entity + 784) < 400) {
            BYTE* targetTop = (BYTE*)(uintptr_t)CharactersClient + *(short*)(entity + 784) * 916;
            for (int n = 0; n < 20; ++n) Effect_Create(1271, (float*)(targetTop + 16), (float*)(targetTop + 28), whiteTop, 0, 0, (float*)-1, 0, 0);
        }
        break;
    case 71: case 74:
        if ((action == 3 || action == 4) && frame == 5) {
            Effect_SpawnBombRing(pos); Effect_Create(241, pos, angle, light, 0, 0, (float*)-1, 0, 0);
        }
        break;
    case 72:
        if (queuedSkill == 50 && frame == 14) for (int n = 0; n < 36; ++n) {
            effectAngle[0] = (float)(rand() % 360); effectAngle[1] = (float)(rand() % 360); effectAngle[2] = (float)(rand() % 360);
            world[0] = pos[0]; world[1] = pos[1]; world[2] = pos[2] + 100.0f;
            Joint_Create(1253, world, world, effectAngle, 1, 0, 60.0f, 0, 0);
        }
        break;
    case 73: case 75:
        if (action == 3 && frame == 11) {
            effectAngle[0] = angle[0] + 45.0f; effectAngle[1] = angle[1]; effectAngle[2] = angle[2];
            float orange[3] = { 1.0f, 0.5f, 0.0f };
            for (int n = 0; n < 5; ++n) {
                world[0] = pos[0] + (float)(rand() % 1001 - 500); world[1] = pos[1] + (float)(rand() % 1001 - 500); world[2] = pos[2] + 500.0f;
                *(float*)(entity + 368) = world[0]; *(float*)(entity + 372) = world[1]; *(float*)(entity + 376) = world[2];
                Effect_Create(256, world, effectAngle, orange, (float*)1, (float*)entity, (float*)-1, 0, 0);
            }
        }
        break;
    case 77:
        if (queuedSkill == 50 && (frame == 2 || frame == 6)) for (int n = 0; n < 40; ++n) {
            effectAngle[0] = (float)(rand() % 360); effectAngle[1] = (float)(rand() % 360); effectAngle[2] = (float)(rand() % 360);
            world[0] = pos[0]; world[1] = pos[1]; world[2] = pos[2] + 100.0f;
            Joint_Create(1253, world, world, effectAngle, 3, 0, 50.0f, 0, 0);
        }
        break;
    case 89: case 95: case 112: case 118: case 124: case 130: case 136:
        if (queuedSkill == 50) {
            if (rand() & 1) {
                world[0] = pos[0] + (float)(rand() % 1024) - 512.0f;
                world[1] = pos[1] + (float)(rand() % 1024) - 512.0f; world[2] = pos[2];
                Effect_Create(191, world, angle, light, 0, 0, (float*)-1, 0, 0);
                FUN_00404bc0(46, 0, 0);
            } else if (frame == 1) {
                Effect_Create(241, pos, angle, light, (float*)1, 0, (float*)-1, 0, 0);
            }
        }
        break;
    case 103:
        if (queuedSkill == 50) {
            world[0] = pos[0] + (float)(rand() % 1024) - 512.0f;
            world[1] = pos[1] + (float)(rand() % 1024) - 512.0f; world[2] = pos[2];
            Effect_Create(191, world, angle, light, 0, 0, (float*)-1, 0, 0);
            FUN_00404bc0(46, 0, 0);
        }
        break;
    default: break;
    }

    if (queuedSkill != 3 && queuedSkill != 17) return;
    const short targetSlot = *(short*)(entity + 784);
    if (targetSlot < 0 || targetSlot >= 400 || !CharactersClient) return;

    BYTE* target = (BYTE*)(uintptr_t)CharactersClient + targetSlot * 916;
    const int modelType = *(short*)(entity + 2);
    if (modelType < 0 || !DAT_05828d58 || !*(DWORD*)(entity + 276)) return;

    // The following switch is the skill-3 switch at 00445230:1898.  The
    // apparent `TargetPosition` in IDA is the transformed hand-bone position;
    // it is deliberately not the target's world position.
    float local[3] = { 0.0f, 0.0f, 0.0f };
    float source[3] = { 0.0f, 0.0f, 0.0f };
    float jointAngle[3] = { 0.0f, 0.0f, 0.0f };
    float white[3] = { 1.0f, 1.0f, 1.0f };
    void* model = (void*)(DAT_05828d58 + modelType * 188);
    const DWORD boneMatrices = *(DWORD*)(entity + 276);
    const auto handPosition = [&](int handIndex, float offsetY, int secondHandAt = 2) {
        local[0] = 0.0f;
        local[1] = offsetY;
        local[2] = 0.0f;
        const BYTE bone = *(BYTE*)(entity + 628 + (handIndex >= secondHandAt ? 24 : 0));
        BMD_TransformPosition(model, (float*)(boneMatrices + 48 * bone), local, source, 1);
    };

    // 00445230:1696.  Skill 17 has a distinct, preceding switch; it is not a
    // variant of the skill-3 default.  Keep the two dispatches separated just
    // as they are in IDA so their stage guards and joint actions stay intact.
    if (queuedSkill == 17) {
        switch (attackEffectKind) {
        case 0x25:
            if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(87, 0, 0);
            for (int n = 0; n < 4; ++n) {
                handPosition(n, 0.0f);
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
                Joint_Create(1261, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
                Particle_Spawn(1195, source, (float*)(entity + 28), light, 0, 1.0f, 0);
            }
            break;
        case 0x2E:
            if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(87, 0, 0);
            for (int n = 0; n < 4; ++n) {
                handPosition(n, 0.0f);
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = 0.0f;
                Joint_Create(1166, source, (float*)(target + 16), jointAngle, 1, (int)target, 50.0f, -1, 0);
                Joint_Create(1166, source, (float*)(target + 16), jointAngle, 1, (int)target, 10.0f, -1, 0);
            }
            break;
        case 0x3D:
            for (int n = 0; n < 6; ++n) {
                handPosition(n, 0.0f, 3);
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
                Joint_Create(1254, source, (float*)(target + 16), jointAngle, 2, (int)target, 50.0f, -1, 0);
                Joint_Create(1254, source, (float*)(target + 16), jointAngle, 2, (int)target, 10.0f, -1, 0);
            }
            if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(87, 0, 0);
            for (int n = 0; n < 4; ++n) {
                handPosition(n, 0.0f);
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
                Joint_Create(1261, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
                Particle_Spawn(1195, source, (float*)(entity + 28), light, 0, 1.0f, 0);
            }
            break;
        case 0x42: {
            if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(60, 0, 0);
            float spin = 225.0f - (float)(((long long)DAT_05826e08 / 10 +
                                           3 * *(BYTE*)(entity + 757)) % 90);
            for (int n = 0; n < 4; ++n, spin += 270.0f) {
                handPosition(n % 2, 0.0f);
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = spin;
                Joint_Create(1261, source, (float*)(target + 16), jointAngle, 1, (int)target, 50.0f, -1, 0);
                Particle_Spawn(1195, source, (float*)(entity + 28), light, 0, 1.0f, 0);
            }
            break;
        }
        case 0x49:
        case 0x4B:
            if (*(BYTE*)(entity + 261) == 4 && *(BYTE*)(entity + 757) == 13) {
                float offset[3] = { -50.0f, 100.0f, 0.0f };
                float effectAngle[3] = { *(float*)(entity + 28) + 45.0f,
                                          *(float*)(entity + 32), *(float*)(entity + 36) };
                float orange[3] = { 1.0f, 0.5f, 0.0f };
                BMD_TransformPosition(model, (float*)(boneMatrices + 528), offset, source, 1);
                Effect_Create(256, source, effectAngle, orange, (float*)1, 0, (float*)-1, 0, 0);
                Joint_Create(1254, source, (float*)(target + 16), effectAngle, 2, (int)target, 50.0f, -1, 0);
            }
            break;
        case 0x4D:
            if (*(BYTE*)(entity + 757) == 14) {
                float zero[3] = { 0.0f, 0.0f, 0.0f };
                float effectAngle[3] = { *(float*)(entity + 28), *(float*)(entity + 32), *(float*)(entity + 36) };
                BMD_TransformPosition(model, (float*)g_AttackEffectMatrix_04D, zero, source, 1);
                Effect_Create(256, source, effectAngle, white, (float*)1, 0, (float*)-1, 0, 0);
                Joint_Create(1254, source, source, effectAngle, 2, (int)target, 50.0f, -1, 0);
            }
            break;
        case 0x57: case 0x5D: case 0x63: case 0x74: case 0x7A: case 0x80: case 0x86:
            if (*(BYTE*)(entity + 757) == 13) {
                float offset[3] = { 60.0f, 30.0f, 0.0f };
                float effectAngle[3] = { *(float*)(entity + 28), *(float*)(entity + 32), *(float*)(entity + 36) };
                BMD_TransformPosition(model, (float*)(boneMatrices + 288), offset, source, 1);
                Effect_Create(191, source, effectAngle, light, (float*)5, 0, (float*)-1, 0, 0);
            }
            break;
        case 0x59: case 0x5F: case 0x70: case 0x76: case 0x7C: case 0x82: case 0x88:
            if (*(BYTE*)(entity + 757) == 14) {
                float zero[3] = { 0.0f, 0.0f, 0.0f };
                float effectAngle[3] = { *(float*)(entity + 28), *(float*)(entity + 32), *(float*)(entity + 36) };
                BMD_TransformPosition(model, (float*)(boneMatrices + 1584), zero, source, 1);
                Effect_Create(256, source, effectAngle, white, (float*)1, 0, (float*)-1, 0, 0);
                Joint_Create(1254, source, source, effectAngle, 2, (int)target, 50.0f, -1, 0);
            }
            break;
        // 0x45 uses IDA's uninitialised/reused scratch position.  It remains
        // deferred instead of inventing a source point.
        default:
            break;
        }
        return;
    }

    switch (attackEffectKind) {
    case 0x22:
        for (int n = 0; n < 4; ++n) {
            handPosition(n, 0.0f);
            jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
            Joint_Create(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
            Joint_Create(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 10.0f, -1, 0);
            Particle_Spawn(1180, source, (float*)(entity + 28), white, 0, 1.0f, 0);
        }
        break;
    case 0x25:
        if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(87, 0, 0);
        for (int n = 0; n < 4; ++n) {
            handPosition(n, 0.0f);
            jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
            Joint_Create(1261, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
            Particle_Spawn(1195, source, (float*)(entity + 28), light, 0, 1.0f, 0);
        }
        break;
    case 0x27:
        handPosition(0, *(short*)(entity + 2) == 390 ? 0.0f : -130.0f);
        jointAngle[0] = -60.0f; jointAngle[1] = 0.0f; jointAngle[2] = *(float*)(entity + 36);
        Joint_Create(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
        Joint_Create(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 10.0f, -1, 0);
        Particle_Spawn(1180, source, (float*)(entity + 28), white, 0, 1.0f, 0);
        break;
    case 0x30:
        for (int n = 0; n < 6; ++n) {
            handPosition(n, 0.0f, 3);
            jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
            Joint_Create(1254, source, (float*)(target + 16), jointAngle, 2, (int)target, 50.0f, -1, 0);
            Joint_Create(1254, source, (float*)(target + 16), jointAngle, 2, (int)target, 10.0f, -1, 0);
        }
        break;
    case 0x59: case 0x5F: case 0x70: case 0x76: case 0x7C: case 0x82: case 0x88: {
        if (*(BYTE*)(entity + 757) == 1) FUN_00404bc0(60, 0, 0);
        float spin = 225.0f - (float)(((long long)DAT_05826e08 / 10 +
                                       3 * *(BYTE*)(entity + 757)) % 90);
        for (int n = 0; n < 4; ++n, spin += 270.0f) {
            handPosition(n % 2, 0.0f);
            jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = spin;
            Joint_Create(1261, source, (float*)(target + 16), jointAngle, 1, (int)target, 50.0f, -1, 0);
            Particle_Spawn(1195, source, (float*)(entity + 28), light, 0, 1.0f, 0);
        }
        break;
    }
    case 0x4D:
        if (*(BYTE*)(entity + 757) >= 8) {
            float zero[3] = { 0.0f, 0.0f, 0.0f };
            BMD_TransformPosition(model, (float*)g_AttackEffectMatrix_04D_Alt, zero, source, 1);
            for (int n = 0; n < 4; ++n) {
                jointAngle[0] = 0.0f; jointAngle[1] = 0.0f; jointAngle[2] = (float)(rand() % 360);
                Joint_Create(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 80.0f, -1, 0);
            }
        }
        break;
    default:
        handPosition(0, *(short*)(entity + 2) == 390 ? 0.0f : -130.0f);
        jointAngle[0] = -60.0f; jointAngle[1] = 0.0f; jointAngle[2] = *(float*)(entity + 36);
        Joint_Create(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 50.0f, -1, 0);
        Joint_Create(1254, source, (float*)(target + 16), jointAngle, 0, (int)target, 10.0f, -1, 0);
        Particle_Spawn(1180, source, (float*)(entity + 28), white, 0, 1.0f, 0);
        break;
    }
}
#endif  // !IDA_PORT_00445230


// ─────────────────────────────────────────────────────────────────────────────
// 2026-05-08: Companion-DLL Offsets.h cross-reference — port small functions
// that were truly missing in our build. Sizes per IDA decompile.
// ─────────────────────────────────────────────────────────────────────────────
