// Entity_LegacyTeleport.cpp — DeleteJoint, CreatePoint y CreateTeleportBegin/End.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern void __cdecl operator_delete(void* ptr);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) operator_delete((unsigned char*)(p))
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


// Apaga los joints activos con ese Type, Target y SubType (-1 = cualquiera).
extern "C" void __cdecl DeleteJoint(int Type, DWORD Target, int SubType)
{
    JOINT* joints = AsJoint(DAT_07b27150);
    const int count = (int)(sizeof(DAT_07b27150) / sizeof(JOINT));   // 500
    for (int i = 0; i < count; ++i) {
        JOINT* j = &joints[i];
        if (j->Live != 0 &&
            j->Type == Type &&
            (DWORD)(uintptr_t)j->Target == Target &&
            (SubType == -1 || j->SubType == SubType))
        {
            j->Live = 0;
        }
    }
}

// IDA: CreatePoint (0x004792C0)
// CreatePoint(float Position[3], int Value,
//   float Color[3], float scale)  (101 bytes)
//
// Spawns a damage popup / floating text in the point pool DAT_07c80110
// (100 × 0x70 bytes). La usa Net_Process para los numeros de dano.
//
// El cuerpo vive en `Entity_TeleportAnim` (Combat/Skills_WarriorLegacy.cpp), un
// nombre heredado que no corresponde: no es un efecto de teleport (esos son
// CreateTeleportBegin/End, mas abajo). Este wrapper expone el nombre de IDA.
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
// FUN_004742b0 @ 0x004742B0 — CreateTeleportBegin(DWORD o)  (83 bytes)
// Per IDA decompile. Begins teleport animation: anim 87, alpha=0 (fade out),
// state byte 1, sparkle effect 1176. Sound 88 (whoosh).
// SetAttackSpeed / SetAction / CreateEffect / PlayBuffer decls in functions.h.
extern "C" void __cdecl CreateTeleportBegin(OBJECT* o)
{
    if (!o) return;
    SetAttackSpeed();                                  // SetAttackSpeed
    (void)SetAction(o, 87);
    o->AlphaTarget = 0.0f;
    o->Teleport = 1;
    (void)CreateEffect(1176, o->Position, o->Angle,
                       o->Light, nullptr, nullptr,
                       (float*)(uintptr_t)0xFFFFFFFFu, nullptr, 0);
    PlayBuffer(88, 0, 0);                          // PlayBuffer(88) whoosh
}

// FUN_00474310 @ 0x00474310 — CreateTeleportEnd(DWORD o)  (93 bytes)
// Per IDA decompile. Completes teleport:
// anim 87, anim_speed +0x108=5.0f (0x40A00000), state byte 3, alpha=1.0f
// (fade-in), sparkle effect 1176, sound 88.
extern "C" void __cdecl CreateTeleportEnd(OBJECT* o)
{
    if (!o) return;
    SetAttackSpeed();
    (void)SetAction(o, 87);
    o->AnimationFrame = 5.0f;
    o->Teleport = 3;
    o->AlphaTarget = 1.0f;
    (void)CreateEffect(1176, o->Position, o->Angle,
                       o->Light, nullptr, nullptr,
                       (float*)(uintptr_t)0xFFFFFFFFu, nullptr, 0);
    PlayBuffer(88, 0, 0);
}

// Combat_UseWarriorSkill @ 0x00485780 — UseSkillWarrior(c=CHARACTER*, o=OBJECT*)
