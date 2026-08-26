// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD DAT_07eaa128;
extern void __cdecl FUN_0054158c(void* ptr);
extern void FUN_004fa5a0(void);
#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y) (0)
#endif
#ifndef LODWORD
#define LODWORD(x) (*((DWORD*)&(x)))
#define HIDWORD(x) (*(((DWORD*)&(x))+1))
#define SLOBYTE(x) (*((char*)&(x)))
#define SLOWORD(x) (*((short*)&(x)))
#define SLODWORD(x) (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x) (*((unsigned char*)&(x)))
#define HIBYTE(x) (*(((unsigned char*)&(x))+1))
#define LOWORD(x) (*((unsigned short*)&(x)))
#define HIWORD(x) (*(((unsigned short*)&(x))+1))
#endif
#define ITEM_SPECIAL_SKILL_OPTION 0
#define ITEM_SPECIAL_LUCK_OPTION 1
#define ITEM_OPTION_ADD_PHYSI_DAMAGE_CODE 60
#define ITEM_OPTION_ADD_MAGIC_DAMAGE_CODE 61
#define ITEM_OPTION_ADD_DEFENSE_RATE_CODE 62
#define ITEM_OPTION_ADD_DEFENSE_CODE 63
#define ITEM_OPTION_ADD_EXCELLENT_DAMAGE_CODE 72
// AngleMatrix @ 0x004F9DB0 (50 lines) — Build 3x4 rotation matrix from Euler angles
// Standard Quake/Half-Life convention:
//   angles[0] = PITCH (rotation around Y)
//   angles[1] = YAW   (rotation around Z)
//   angles[2] = ROLL  (rotation around X)
// BUG-FIX (CRÍTICO, 2026-04-20):
//   El port previo intercambiaba las etiquetas: calculaba las entradas con
//   sp=sin(angles[0]), sy=sin(angles[1]), sr=sin(angles[2]) PERO las
//   combinaba como si fueran de un orden distinto (fórmulas no-Quake). El
//   resultado: para rot=(0,0,180) (ships, chars login) producía Rx(180)
//   (patas arriba) en vez de Rz(180) (mirando al revés en pie) → todos los
//   modelos volteados. Re-verificado byte-exact contra Ghidra decompile de
//   0x004F9DB0. Mapeo correcto sP→A[0], sY→A[1], sR→A[2].
void __cdecl AngleMatrix(float *angles, float (*matrix)[4]) {
    float deg2rad = 0.017453292f; // pi/180 = DAT_00552ce8
    float sP = sinf(angles[0] * deg2rad), cP = cosf(angles[0] * deg2rad); // pitch
    float sY = sinf(angles[1] * deg2rad), cY = cosf(angles[1] * deg2rad); // yaw
    float sR = sinf(angles[2] * deg2rad), cR = cosf(angles[2] * deg2rad); // roll

    matrix[0][0] = cY * cR;
    matrix[0][1] = sP * sY * cR - cP * sR;
    matrix[0][2] = sP * sR + cP * sY * cR;
    matrix[0][3] = 0.0f;
    matrix[1][0] = cY * sR;
    matrix[1][1] = cP * cR + sP * sY * sR;
    matrix[1][2] = cP * sY * sR - sP * cR;
    matrix[1][3] = 0.0f;
    matrix[2][0] = -sY;
    matrix[2][1] = sP * cY;
    matrix[2][2] = cP * cY;
    matrix[2][3] = 0.0f;
}

// VectorIRotate @ 0x004FA110 (20 lines) — Inverse-rotate vector by matrix (transpose multiply)
// out = M^T * in1 (rotation only, ignores translation column)
void __cdecl VectorIRotate(float *in1, float (*matrix)[4], float *out) {
    out[0] = in1[0] * matrix[0][0] + in1[1] * matrix[1][0] + in1[2] * matrix[2][0];
    out[1] = in1[0] * matrix[0][1] + in1[1] * matrix[1][1] + in1[2] * matrix[2][1];
    out[2] = in1[0] * matrix[0][2] + in1[1] * matrix[1][2] + in1[2] * matrix[2][2];
}

// BMD__TransformPosition @ 0x004409A0 (67 lines) — Transform position through bone matrix
// If Translate: result = scale * (Matrix * Pos) + origin
// If !Translate: result = Matrix * Pos (direct transform)
void __fastcall BMD__TransformPosition(void *This, float (*BoneMatrix)[4], float *Pos, float *WorldPos, bool Translate) {
    // VectorTransform: WorldPos = BoneMatrix * Pos
    float temp[3];
    float *dst = Translate ? temp : WorldPos;
    dst[0] = Pos[0] * BoneMatrix[0][0] + Pos[1] * BoneMatrix[0][1] + Pos[2] * BoneMatrix[0][2] + BoneMatrix[0][3];
    dst[1] = Pos[0] * BoneMatrix[1][0] + Pos[1] * BoneMatrix[1][1] + Pos[2] * BoneMatrix[1][2] + BoneMatrix[1][3];
    dst[2] = Pos[0] * BoneMatrix[2][0] + Pos[1] * BoneMatrix[2][1] + Pos[2] * BoneMatrix[2][2] + BoneMatrix[2][3];

    if (Translate) {
        // Scale by BMD scale factor and add origin
        float scale = *(float *)((int)This + 0x68);
        float *origin = (float *)((int)This + 0x6c);
        WorldPos[0] = scale * temp[0] + origin[0];
        WorldPos[1] = scale * temp[1] + origin[1];
        WorldPos[2] = scale * temp[2] + origin[2];
    }
}


void __cdecl FaceNormalize(float v[3], float out[3], float v2[3], float normal[3]) {
    // 0x00440A60 approx — Compute face normal from 3 vertices
    // normal = normalize(cross(v1-v0, v2-v0))
    (void)v; (void)out; (void)v2; (void)normal;
}

bool __cdecl CollisionDetectLineToFace(float pos[3], float target[3], int normalIdx,
          float localC[3], float* posZ, float* v3, float* v4, float normal[3], char flag) {
    // 0x00440C90 approx — Test line segment against a triangle face
    (void)pos; (void)target; (void)normalIdx; (void)localC;
    (void)posZ; (void)v3; (void)v4; (void)normal; (void)flag;
    return false;
}
