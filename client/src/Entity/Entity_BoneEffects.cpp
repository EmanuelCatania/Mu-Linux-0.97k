// Entity_BoneEffects.cpp — efectos atados a huesos (Entity_SpawnBoneEffect,
// Entity_SpawnBoneRangeEffect).

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

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

// IDA Hex-Rays intrinsic shims.
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


// Entity_SpawnBoneEffect @ 0x00456590 — Entity_SpawnBoneEffect(entity, effectType, scale, bone, x, flags, yOff)
// Transforms an offset vector through the entity's bone matrix, then spawns a particle
// effect at the resulting world position with a pulsing light color.
void* __cdecl Entity_SpawnBoneEffect(int entity, int effectType, float scale, int bone, float x, int flags, float yOff)
{
    // offset vector at bone position + x/yOff
    // IDA sub_456590 arma v9[0]=a5, v9[1]=a6, v9[2]=a7 y pasa v9 (desde el indice 0).
    float offset[3];
    offset[0] = x;
    offset[1] = (float)flags;   // param_6 (undefined4 packed as float here)
    offset[2] = yOff;

    float outPos[3];
    void *modelPtr = (void *)(DAT_05828d58 + *(short *)(entity + 2) * 0xbc);
    float *boneMat = (float *)(*(int *)(entity + 0x114) + bone * 0x30);
    BMD_TransformPosition(modelPtr, boneMat, offset, outPos, '\x01');

    // Pulsing light: sin(animTick * period) * amp + base
    float sinVal = (float)fsin((double)DAT_05826e08 * (double)_DAT_005528e0);
    float light[3];
    light[0] = sinVal * _DAT_005528b8 + _DAT_00552928;
    light[1] = light[0] * _DAT_00552534;
    light[2] = light[0] * _DAT_005528b4;

    // IDA sub_456590: CreateSprite(Type, Position, Scale, Light, Owner, 0.0, 0) —
    // el 3er argumento es la ESCALA, no el tipo.
    CreateSprite((unsigned short)effectType, outPos, scale, light, entity, 0.0f, 0);
    return (void *)entity;
}

// Entity_SpawnBoneRangeEffect @ 0x00456650 — Entity_SpawnBoneRangeEffect(entity, bone1, bone2, scale)
// Spawns beam effects between two bones of the entity, and writes bone2 world pos to entity+0x40.
void* __cdecl Entity_SpawnBoneRangeEffect(int entity, int bone1, int bone2, float scale)
{
    float sinVal = (float)fsin((double)DAT_05826e08 * (double)_DAT_005528e0);
    void *modelPtr = (void *)(DAT_05828d58 + *(short *)(entity + 2) * 0xbc);

    float light0 = sinVal * _DAT_005528b8 + _DAT_00552530;
    float light[3] = { light0, light0, light0 };

    // Bone1: offset {5, 0, 0} → spawn at world pos
    float vec1[4] = { 5.0f, 0.0f, 0.0f, 0.0f };
    float outPos1[4];
    BMD_TransformPosition(modelPtr, (float *)(bone1 * 0x30 + *(int *)(entity + 0x114)), vec1, outPos1, '\x01');
    CreateSprite(0x4d1, outPos1, scale, light, 0, 0.0f, 0);

    // Bone2: offset {-5, 0, 0} → spawn at world pos, write result to entity+0x40
    vec1[0] = -5.0f;
    float outPos2[4];
    BMD_TransformPosition(modelPtr, (float *)(bone2 * 0x30 + *(int *)(entity + 0x114)), vec1, outPos2, '\x01');
    CreateSprite(0x4d1, outPos2, scale, light, 0, 0.0f, 0);

    *(float *)(entity + 0x40) = outPos2[0];
    *(float *)(entity + 0x44) = outPos2[1];
    *(float *)(entity + 0x48) = outPos2[2];
    return (void *)entity;
}
