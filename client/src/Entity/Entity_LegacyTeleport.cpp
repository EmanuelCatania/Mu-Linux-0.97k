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


// (61 bytes) — walks Joints pool DAT_07b27150 (500 × 0x9D8) and zeroes any
// active slot whose Type/Target/SubType match.
//
// Slot layout per IDA `CreateJoint` (raw/0046D840_CreateJoint.c:178-195):
//   slot+0   byte  active flag
//   slot+4   int   Type
//   slot+8   int   SubType
//   slot+64  DWORD Target  (= entity ptr / owner)
// Stride 0x9D8 = 2520 bytes per slot.
//
// 2026-05-08: previously a no-op (mc_DeleteJoint in SecondPassword.cpp) "until
// joint pool wired". Pool IS sized in globals.cpp (200×0x9D8); now functional.
extern "C" void __cdecl DeleteJoint(int Type, DWORD Target, int SubType)
{
    BYTE* base = (BYTE*)&DAT_07b27150[0];
    const int kJointSlots = (int)(sizeof(DAT_07b27150) / 0x9D8);   // 500
    for (int i = 0; i < kJointSlots; ++i) {
        BYTE* slot = base + i * 0x9D8;
        if (slot[0] != 0 &&
            *(int*)(slot + 4) == Type &&
            *(DWORD*)(slot + 64) == Target &&
            (SubType == -1 || *(int*)(slot + 8) == SubType))
        {
            slot[0] = 0;
        }
    }
}
void __cdecl FUN_0046fe00(int Type, DWORD Target, int SubType)
{ DeleteJoint(Type, Target, SubType); }

// FUN_004792c0 @ 0x004792C0 — CreatePoint(float Position[3], int Value,
//   float Color[3], float scale)  (101 bytes)
//
// Spawns a damage popup / floating text in the point pool DAT_07c80110
// (100 × 0x70 bytes). Per IDA decompile (raw/004792C0_CreatePoint.c).
//
// 2026-05-08: this address is the same function as the existing
// `Entity_TeleportAnim` in this file — the alias was misnamed because
// `Skills.cpp:349` calls it expecting a teleport-spawner, but the actual IDA
// is `CreatePoint` (damage-popup spawner). The Net_Process case 0x15 path
// uses it correctly as damage popup. We add the IDA name as the canonical
// entry point and the existing `Entity_TeleportAnim` impl provides the body.
//
// Skill teleport effect is actually `CreateTeleportBegin` (FUN_004742b0) /
// `CreateTeleportEnd` (FUN_00474310) — Skills.cpp:349 is a misrouted call
// (left as-is for now; spawning a degenerate damage popup is harmless, and
// fixing the routing belongs to Skills.cpp).
//
// `Entity_TeleportAnim` body (lower in this file, ~line 1070) is the actual
// implementation; this wrapper just exposes the IDA name.
extern void __cdecl Entity_TeleportAnim(float* world_pos,
    float entity_id, float* dst_pos, float param_4);
extern "C" void __cdecl CreatePoint(float Position[3], int Value,
                                    float Color[3], float scale)
{
    // bit-cast Value → float for the existing impl's "entity_id" slot
    // (matches IDA: stores Value as int at +4 / float at +4 alias).
    float val_as_float;
    *(int*)&val_as_float = Value;
    Entity_TeleportAnim(Position, val_as_float, Color, scale);
}
void __cdecl FUN_004792c0(float* Position, int Value, float* Color, float scale)
{ CreatePoint(Position, Value, Color, scale); }

// FUN_004742b0 @ 0x004742B0 — CreateTeleportBegin(DWORD o)  (83 bytes)
// Per IDA decompile (raw/004742B0_CreateTeleportBegin.c). Begins teleport
// animation: anim 87, alpha=0 (fade out), state byte 1, sparkle effect 1176.
// Sound 88 (whoosh).
//
// 2026-05-08: previously aliased as `Entity_WeaponHit` in CLAUDE.md and our
// Combat.cpp comments, but IDA confirms this is the teleport-begin function.
// FUN_00443e70 / FUN_0043e820 / FUN_00460dc0 / FUN_00404bc0 decls in functions.h.
extern "C" void __cdecl CreateTeleportBegin(unsigned int o)
{
    if (!o) return;
    FUN_00443e70();                                  // SetAttackSpeed
    (void)FUN_0043e820((int)o, 87);                  // SetAction(o, 87)
    *(unsigned int*)(o + 356) = 0;                   // alpha = 0 (fade-out)
    *(BYTE*)(o + 124) = 1;                           // state byte = 1 (begin)
    (void)FUN_00460dc0(1176, (float*)(o + 16), (float*)(o + 28),
                       (float*)(o + 232), nullptr, nullptr,
                       (float*)(uintptr_t)0xFFFFFFFFu, nullptr, 0);
    FUN_00404bc0(88, 0, 0);                          // PlayBuffer(88) whoosh
}

// FUN_00474310 @ 0x00474310 — CreateTeleportEnd(DWORD o)  (93 bytes)
// Per IDA decompile (raw/00474310_CreateTeleportEnd.c). Completes teleport:
// anim 87, anim_speed +0x108=4.5f (0x40A00000), state byte 3, alpha=1.0f
// (fade-in), sparkle effect 1176, sound 88.
extern "C" void __cdecl CreateTeleportEnd(unsigned int o)
{
    if (!o) return;
    FUN_00443e70();
    (void)FUN_0043e820((int)o, 87);
    *(unsigned int*)(o + 264) = 0x40A00000u;        // anim_speed = 5.0f
    *(BYTE*)(o + 124) = 3;                           // state byte = 3 (end)
    *(unsigned int*)(o + 356) = 0x3F800000u;        // alpha = 1.0f (fade-in)
    (void)FUN_00460dc0(1176, (float*)(o + 16), (float*)(o + 28),
                       (float*)(o + 232), nullptr, nullptr,
                       (float*)(uintptr_t)0xFFFFFFFFu, nullptr, 0);
    FUN_00404bc0(88, 0, 0);
}

// FUN_00485780 @ 0x00485780 — UseSkillWarrior(c=CHARACTER*, o=OBJECT*)
