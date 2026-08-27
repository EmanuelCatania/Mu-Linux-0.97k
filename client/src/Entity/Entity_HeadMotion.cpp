// Entity_HeadMotion.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// IDA: FUN_0043E890 @ 0x0043E890 — LookAtTarget (~38 lines), rotates entity head toward target character.
// Computes angle between entity and target, sets HeadTargetAngle if within threshold.
void __cdecl LookAtTarget_stub(DWORD o, DWORD TargetCharacter) {
    // 0x0043E890 — Rotates entity head toward target character
    // o = OBJECT* (entity), TargetCharacter = CHARACTER* (target, OBJECT at offset 0)
    // OBJECT struct (from Ghidra, size 0x1BC):
    //   +0x10 = Position[3] (float), +0x1C = Angle[3] (float, [2]=facing)
    //   +0x34 = HeadTargetAngle[3] (float)
    // _DAT_00552848 = angle threshold (~80.0f)
    // _DAT_00552598 = height offset, _DAT_005526e4 = vertical angle scale
    // _DAT_00552580 = -180.0f guard, _DAT_0055286c = 360.0f wrap

    if (!TargetCharacter) return;

    float oX = *(float*)(o + 0x10);      // o->Position[0]
    float oY = *(float*)(o + 0x14);      // o->Position[1]
    float oZ = *(float*)(o + 0x18);      // o->Position[2]
    float oFacing = *(float*)(o + 0x24); // o->Angle[2]  — offset 0x1C + 8 = 0x24

    float tX = *(float*)(TargetCharacter + 0x10);  // target->Object.Position[0]
    float tY = *(float*)(TargetCharacter + 0x14);  // target->Object.Position[1]
    float tZ = *(float*)(TargetCharacter + 0x18);  // target->Object.Position[2]

    // CreateAngle @ 0x0043e050: declared as FUN_0043e050 with wrong sig; cast to correct prototype
    typedef float (__cdecl *CreateAngleFn)(float, float, float, float);
    float angle = ((CreateAngleFn)&FUN_0043e050)(oX, oY, tX, tY);

    // FarAngle(oFacing, angle, 1) — angular distance
    double deltaAngle = (double)FUN_0043e370(oFacing, angle, '\x01');

    float* headTarget = (float*)(o + 0x34);  // o->HeadTargetAngle[3]

    if (deltaAngle < (double)_DAT_00552848) {
        // Within threshold: compute head rotation
        headTarget[0] = oFacing - angle;
        headTarget[1] = (tZ - (oZ + _DAT_00552598)) * _DAT_005526e4;

        // Wrap angles if < -180.0f
        for (int i = 0; i < 2; i++) {
            if (headTarget[i] < _DAT_00552580) {
                headTarget[i] = headTarget[i] + _DAT_0055286c;
            }
        }
    } else {
        // Outside threshold: reset head to neutral
        headTarget[0] = 0.0f;
        headTarget[1] = 0.0f;
    }
}

// IDA: FUN_0043E940 @ 0x0043E940 — MoveHead (~56 lines), random/tracked head movement for entities.
void __cdecl MoveHead_stub(int param_1) {
    char anim = *(char*)(param_1 + 0x105);
    if (anim == '\x06') return; // dead — no head movement

    if (anim == '\0') {
        // Idle: 1/32 chance random head look
        unsigned int r = rand();
        unsigned int masked = r & 0x8000001f;
        bool isZero = (masked == 0);
        if ((int)masked < 0) {
            isZero = ((masked - 1 | 0xffffffe0) == 0xffffffff);
        }
        if (isZero) {
            float* pfVar4 = (float*)(param_1 + 0x34);
            *pfVar4 = (float)(rand() % 128 - 64);          // 0043E940: -64..+63
            *(float*)(param_1 + 0x38) = (float)(rand() % 48 - 16); // -16..+31
            // Clamp both head angles toward 0 by _DAT_0055286c if below _DAT_00552580
            int iVar5 = 2;
            do {
                if (*pfVar4 < _DAT_00552580) {
                    *pfVar4 = *pfVar4 + _DAT_0055286c;
                }
                pfVar4 = pfVar4 + 1;
                iVar5 = iVar5 - 1;
            } while (iVar5 != 0);
            return;
        }
    }
    else if ((anim == '\x02') && (*(short*)(param_1 + 0x310) != -1)) {
        // Combat: look at attack target
        LookAtTarget_stub((DWORD)param_1, CharactersClient + *(short*)(param_1 + 0x310));
        return;
    }
    else {
        // Default: reset head angles to 0
        *(DWORD*)(param_1 + 0x34) = 0;
        *(DWORD*)(param_1 + 0x38) = 0;
    }
}
