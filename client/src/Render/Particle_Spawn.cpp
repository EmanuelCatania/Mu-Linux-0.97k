// Particle_Spawn.cpp — IDA: FUN_00475220 @ 0x00475220
// Allocates a free slot in the particle pool and initialises it.
// The pool starts at DAT_07abf5f0; each slot is 0x70 bytes.
// Overflow guard: if the slot pointer exceeds 0x7b1166f, returns 0.
//
// Slot layout (offsets within each 0x70 block):
//   +0x00  char     active flag (0=free, 1=alive)
//   +0x04  int      effect_type  (param_1)
//   +0x08  int      sub_type     (param_5)
//   +0x0c  float    scale        (param_6)
//   +0x10  float[3] world_xyz    (param_2)
//   +0x1c  float[3] dir_xyz      (param_3)
//   +0x28  float[3] color_rgb    (param_4)
//   +0x38  int      lifetime     (default 2)
//   +0x3c  int      entity_ptr   (param_7)
//   +0x40  float    rotation
//   +0x44  float    (clear)
//   +0x48  float    extra2
//   +0x4c  float    extra3
//   +0x50  float[3] vel_xyz
//   +0x5c  float[3] color_copy   (set to color_rgb for some types)
//   +0x64  float[3] prev_xyz     (copy of world_xyz for some types)

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgForge(const char* fn, int type, int model, int bmp, int glTex,
                         int mesh, int blend, float wx, float wy, float wz,
                         float r, float g, float b, float a);   // [DIAG FORGE]

// IDA: FUN_00475220
int __cdecl Particle_Spawn(int param_1, float *param_2, float *param_3, float *param_4,
                         int param_5, float param_6, int param_7)
{
    // [DIAG FORGE] entry-point CreateParticle (mesh field = subtype param_5, a = scale)
    if (param_2)
        DbgForge("CreateParticle", param_1, -1, param_1, -1, param_5, -1,
                 param_2[0], param_2[1], param_2[2],
                 param_4 ? param_4[0] : 0.0f, param_4 ? param_4[1] : 0.0f,
                 param_4 ? param_4[2] : 0.0f, param_6);
    float *pfVar1;
    float *pfVar2;
    float *pfVar3;
    float fVar4;
    float fVar5;
    int   iVar6;
    int   iVar7;
    unsigned int uVar8;
    int   iVar9;
    int   iVar10;
    char *pcVar11;
    long double fVar12;
    long double fVar13;
    unsigned long long uVar14;
    float local_3c;
    float local_38;
    float local_34;
    float local_30[12];

    // ── scan pool for free slot ────────────────────────────────────────────────
    // Pool fix 2026-04-27: era end-bound absoluto 0x7b1166f. Con DAT_07abf5f0
    // como 1 byte el primer slot estaba "ocupado" (basura) y el guard
    // retornaba 0 inmediatamente → NUNCA spawneaba particles. Ahora pool real
    // de 3000 slots, iteramos por índice.
    iVar6 = 0;
    pcVar11 = DAT_07abf5f0;
    while (*pcVar11 != '\0') {
        pcVar11 = pcVar11 + 0x70;
        iVar6 = iVar6 + 1;
        if (iVar6 >= 3000)
            return 0;
    }

    // ── base initialisation ────────────────────────────────────────────────────
    *(int *)(pcVar11 + 8)  = param_5;
    *pcVar11               = '\x01';               // mark active
    *(int *)(pcVar11 + 4)  = param_1;              // effect_type
    pfVar1 = (float *)(pcVar11 + 0x10);
    pfVar2 = (float *)(pcVar11 + 0x1c);
    *pfVar1                    = *param_2;
    *(float *)(pcVar11 + 0x14) = param_2[1];
    *(float *)(pcVar11 + 0x18) = param_2[2];
    // prev_xyz = world_xyz
    *(float *)(pcVar11 + 100)  = *param_2;          // +0x64
    *(float *)(pcVar11 + 0x68) = param_2[1];
    *(float *)(pcVar11 + 0x6c) = param_2[2];
    // color
    *(float *)(pcVar11 + 0x28) = *param_4;
    *(float *)(pcVar11 + 0x2c) = param_4[1];
    *(float *)(pcVar11 + 0x30) = param_4[2];
    // scale
    *(float *)(pcVar11 + 0x0c) = param_6;
    // clear extra / rotation fields
    pcVar11[0x48] = '\0'; pcVar11[0x49] = '\0'; pcVar11[0x4a] = '\0'; pcVar11[0x4b] = '\0';
    pcVar11[0x44] = '\0'; pcVar11[0x45] = '\0'; pcVar11[0x46] = '\0'; pcVar11[0x47] = '\0';
    pcVar11[0x40] = '\0'; pcVar11[0x41] = '\0'; pcVar11[0x42] = '\0'; pcVar11[0x43] = '\0';
    // lifetime default = 2
    pcVar11[0x38] = '\x02'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
    *(int *)(pcVar11 + 0x3c) = param_7;            // entity_ptr
    // dir
    *pfVar2                    = *param_3;
    *(float *)(pcVar11 + 0x20) = param_3[1];
    *(float *)(pcVar11 + 0x24) = param_3[2];
    pfVar3 = (float *)(pcVar11 + 0x4c);
    *pfVar3 = 0.0f;
    // clear vel
    pcVar11[0x50] = '\0'; pcVar11[0x51] = '\0'; pcVar11[0x52] = '\0'; pcVar11[0x53] = '\0';
    pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';

    // ── per-type customisation ────────────────────────────────────────────────
    if (param_1 < 0x4c5) {
        if (param_1 == 0x4c4) {
            // 0x4c4 — fire/ember type; sub_type selects variant
            switch (param_5) {
            case 0: case 4: case 9:
                pcVar11[0x38] = '\x10'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000001f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
                *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 0x30) * _DAT_005524f8;
                iVar7 = _rand();
                *pfVar2 = (float)(iVar7 % 0x168);
                // IDA: `*((float *)v8 + 16) = (float)((__int64)WorldTime % 360);`
                // 2026-08-11: acá había un `__ftol()` sin argumentos (el artefacto
                // de Ghidra para la conversión x87 de WorldTime) cuyo resultado
                // alimentaba el shift/OR de abajo — o sea el ángulo inicial de la
                // partícula salía de basura. Es el mismo patrón ya barrido en los
                // otros 47 sitios; éste sobrevivió porque el tipo 1220 (humo)
                // nunca llegaba a spawnearse.
                *(float *)(pcVar11 + 0x40) = (float)(int)((__int64)WorldTime % 360);
                return iVar6;
            case 1:
                pcVar11[0x38] = '2'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000001f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
                *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 0x50) * _DAT_005524f8;
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000003f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
                *pfVar1 = (float)(int)(uVar8 - 0x20) + *pfVar1;
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000003f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
                *(float *)(pcVar11 + 0x14) = (float)(int)(uVar8 - 0x20) + *(float *)(pcVar11 + 0x14);
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000003f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
                *(float *)(pcVar11 + 0x18) = (float)(int)(uVar8 + 0x20) + *(float *)(pcVar11 + 0x18);
                iVar7 = _rand(); *pfVar2 = (float)(iVar7 % 0x5a + -0x2d);
                iVar7 = _rand(); *pfVar3 = 0.0f;
                *(float *)(pcVar11 + 0x24) = (float)(iVar7 % 0x168);
                uVar8 = _rand();
                uVar8 = uVar8 & 0x80000007;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
                pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
                *(float *)(pcVar11 + 0x50) = -(float)(int)(uVar8 + 0x28);
                return iVar6;
            case 2:
                pcVar11[0x38] = '2'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000003f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
                *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 0x40) * _DAT_005524f8;
                iVar7 = _rand();
                *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000001f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
                *(float *)(pcVar11 + 0x48) = (float)(int)(uVar8 + 0x3c) * _DAT_005524f4;
                return iVar6;
            case 3:
                pcVar11[0x38] = '\n'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000001f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
                *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 0x50) * _DAT_005524f8;
                iVar7 = _rand(); *pfVar2 = (float)(iVar7 % 0x5a + -0x2d);
                iVar7 = _rand(); *pfVar3 = 0.0f;
                *(float *)(pcVar11 + 0x24) = (float)(iVar7 % 0x168);
                uVar8 = _rand();
                uVar8 = uVar8 & 0x80000007;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
                pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
                *(float *)(pcVar11 + 0x50) = -(float)(int)(uVar8 + 0x28);
                return iVar6;
            case 5:
                pcVar11[0x38] = '\x14'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000003f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
                *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 0x62) * _DAT_005524f8;
                iVar7 = _rand();
                *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000001f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
                *(float *)(pcVar11 + 0x48) = (float)(int)(uVar8 + 0x3c) * _DAT_005524f4;
                return iVar6;
            case 6:
                pcVar11[0x38] = '\x1e'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                iVar7 = _rand();
                *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 1000);
                iVar7 = _rand();
                *(float *)(pcVar11 + 0x0c) = (float)(iVar7 % 0x14 + 0xb4) * _DAT_005524f8;
                iVar7 = _rand();
                *pfVar1 = (float)(iVar7 % 200 + -100) * param_6 + *pfVar1;
                iVar7 = _rand();
                *(float *)(pcVar11 + 0x14) = (float)(iVar7 % 200 + -100) * param_6 + *(float *)(pcVar11 + 0x14);
                iVar7 = _rand();
                *pfVar3 = 0.0f;
                pcVar11[0x50] = '\0'; pcVar11[0x51] = '\0'; pcVar11[0x52] = '\0'; pcVar11[0x53] = '\0';
                pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
                fVar4 = (float)(iVar7 % 0x14 + 0x14) + *(float *)(pcVar11 + 0x18);
                *(float *)(pcVar11 + 0x18) = fVar4;
                *(float *)(pcVar11 + 0x40) = fVar4;
                return iVar6;
            case 7:
                pcVar11[0x38] = '\x1e'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                *pfVar3 = 0.0f;
                uVar8 = _rand();
                uVar8 = uVar8 & 0x80000003;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
                pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
                *(float *)(pcVar11 + 0x50) = (float)(int)(uVar8 + 6) * *(float *)(pcVar11 + 0x0c);
                iVar7 = _rand();
                *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 200) * _DAT_00552abc;
                iVar7 = _rand();
                *(float *)(pcVar11 + 0x0c) =
                    (float)(iVar7 % 0x14 + 0x78) * *(float *)(pcVar11 + 0x0c) * _DAT_005524f8;
                iVar7 = _rand();
                *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
                return iVar6;
            case 8:
                pcVar11[0x38] = '\x18'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                *pfVar3 = 0.0f;
                uVar8 = _rand();
                uVar8 = uVar8 & 0x80000007;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
                pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
                *(float *)(pcVar11 + 0x50) = (float)(int)(uVar8 + 0x20) * _DAT_005529c0;
                *(float *)(pcVar11 + 0x0c) = *(float *)(pcVar11 + 0x0c) * _DAT_00552530;
                iVar7 = _rand();
                *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
                return iVar6;
            case 10:
                pcVar11[0x38] = '\x10'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                return iVar6;
            case 0xb:
                pcVar11[0x38] = '2'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000003f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
                *pfVar1 = (float)(int)(uVar8 - 0x20) + *pfVar1;
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000003f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
                *(float *)(pcVar11 + 0x14) = (float)(int)(uVar8 - 0x20) + *(float *)(pcVar11 + 0x14);
                uVar8 = _rand();
                uVar8 = uVar8 & 0x8000003f;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
                *(float *)(pcVar11 + 0x18) = (float)(int)(uVar8 + 0x20) + *(float *)(pcVar11 + 0x18);
                iVar7 = _rand(); *pfVar2 = (float)(iVar7 % 0x5a + -0x2d);
                iVar7 = _rand(); *pfVar3 = 0.0f;
                *(unsigned int *)(pcVar11 + 0x5c) = *(unsigned int *)(pcVar11 + 0x2c);
                *(unsigned int *)(pcVar11 + 0x60) = *(unsigned int *)(pcVar11 + 0x30);
                *(unsigned int *)(pcVar11 + 0x58) = *(unsigned int *)(pcVar11 + 0x28);
                *(float *)(pcVar11 + 0x24) = (float)(iVar7 % 0x168);
                uVar8 = _rand();
                uVar8 = uVar8 & 0x80000007;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
                pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
                *(float *)(pcVar11 + 0x50) = -(float)(int)(uVar8 + 0x28);
                return iVar6;
            default:
                return iVar6;
            }
        }

        if (param_1 < 0x4a7) {
            if (param_1 == 0x4a6) {
                // 0x4a6 — up-burst with fixed scale
                pcVar11[0x38] = '\n'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                pcVar11[0x0c] = 'f'; pcVar11[0x0d] = 'f'; pcVar11[0x0e] = (char)-0x1a; pcVar11[0x0f] = '?';
                *pfVar2 = 30.0f;
                uVar8 = _rand();
                uVar8 = uVar8 & 0x80000003;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
                *(float *)(pcVar11 + 0x28) = (float)(int)(uVar8 + 6) * _DAT_005524f4;
                uVar8 = _rand();
                uVar8 = uVar8 & 0x80000003;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
                *(float *)(pcVar11 + 0x2c) = (float)(int)(uVar8 + 6) * _DAT_005524f4;
                uVar8 = _rand();
                uVar8 = uVar8 & 0x80000003;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
                *(float *)(pcVar11 + 0x30) = (float)(int)(uVar8 + 6) * _DAT_005524f4;
                *(float *)(pcVar11 + 0x18) = *(float *)(pcVar11 + 0x18) + _DAT_005529e0;
                return iVar6;
            }
            if (param_1 < 0x498) {
                if (param_1 == 0x497) {
                    // 0x497 — matrix-transformed directional burst
                    uVar8 = _rand();
                    uVar8 = uVar8 & 0x80000003;
                    if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
                    iVar7 = *(int *)(pcVar11 + 8);
                    *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 4) * _DAT_005524f4;
                    if (((iVar7 != 0) && (iVar7 != 2)) && (iVar7 != 3)) {
                        uVar8 = _rand();
                        uVar8 = uVar8 & 0x80000007;
                        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
                        *(unsigned int *)(pcVar11 + 0x38) = uVar8 + 8;
                        return iVar6;
                    }
                    uVar8 = _rand();
                    uVar8 = uVar8 & 0x8000000f;
                    if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
                    *(unsigned int *)(pcVar11 + 0x38) = uVar8 + 0x18;
                    iVar7 = _rand();
                    *(float *)(pcVar11 + 0x24) = (float)(iVar7 % 0x168);
                    Matrix_BuildFromEuler(pfVar2, local_30);
                    uVar8 = _rand();
                    uVar8 = uVar8 & 0x8000000f;
                    if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
                    local_3c = 0.0f;
                    *(float *)(pcVar11 + 0x48) = (float)(int)(uVar8 + 6);
                    iVar7 = _rand();
                    local_38 = (float)(iVar7 % 0x14 + 0x14) * _DAT_005524f4;
                    if (*(int *)(pcVar11 + 8) == 2) {
                        *(float *)(pcVar11 + 0x0c) = *(float *)(pcVar11 + 0x0c) + *(float *)(pcVar11 + 0x0c);
                        local_3c = local_3c * _DAT_00552540;
                        local_38 = local_38 * _DAT_00552540;
                    }
                    local_34 = 0.0f;
                    Vector_Rotate(&local_3c, local_30, pfVar3);
                    return iVar6;
                }
                if (0x47f < param_1) {
                    if (param_1 == 0x490) {
                        // 0x490 — glow with RGBA
                        fVar4 = *(float *)(pcVar11 + 0x24) + _DAT_0055293c;
                        pcVar11[0x38] = '\n'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                        pcVar11[0x48] = (char)-0x33; pcVar11[0x49] = (char)-0x34;
                        pcVar11[0x4a] = (char)-0x34; pcVar11[0x4b] = '=';
                        *(float *)(pcVar11 + 0x40) = fVar4;
                        return iVar6;
                    }
                    return iVar6;
                }
                if (param_1 == 0x47f) {
                    // handled below with 0x4ab
                    goto switchD_4ab;
                }
                // 0x67/0x68 range
                if (param_1 < 0x67) return iVar6;
                if (param_1 < 0x69) {
                    pcVar11[0x38] = '\x14'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                    iVar7 = _rand();
                    *(float *)(pcVar11 + 0x0c) = (float)(iVar7 % 6 + 8) * _DAT_005524f4;
                    if ((*(int *)(pcVar11 + 4) != 0x68) && (*(int *)(pcVar11 + 8) != 1))
                        return iVar6;
                    iVar7 = _rand(); *pfVar1 = (float)(iVar7 % 10 + -5) + *pfVar1;
                    iVar7 = _rand(); *(float *)(pcVar11 + 0x14) = (float)(iVar7 % 10 + -5) + *(float *)(pcVar11 + 0x14);
                    iVar7 = _rand(); *(float *)(pcVar11 + 0x18) = (float)(iVar7 % 10 + -5) + *(float *)(pcVar11 + 0x18);
                    if (*(int *)(pcVar11 + 8) != 1) return iVar6;
                    *(float *)(pcVar11 + 0x0c) = param_6;
                    iVar7 = _rand();
                    pcVar11[0x50] = '\0'; pcVar11[0x51] = '\0'; pcVar11[0x52] = (char)-0x80; pcVar11[0x53] = '?';
                    *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 100) * _DAT_00552890;
                    return iVar6;
                }
                if (param_1 != 0x47e) return iVar6;
                // 0x47e — misc bubble/spark
                if (param_5 == 0) {
                    iVar7 = _rand(); *(int *)(pcVar11 + 0x38) = iVar7 % 10 + 10;
                    iVar7 = _rand(); *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 10) + _DAT_00552488;
                    iVar7 = _rand();
                } else if (param_5 == 1) {
                    pcVar11[0x38] = '2'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                    return iVar6;
                } else if (param_5 == 2) {
                    iVar7 = _rand(); *(int *)(pcVar11 + 0x38) = iVar7 % 10 + 10;
                    iVar7 = _rand(); *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 10) + _DAT_00552488;
                    iVar7 = _rand();
                } else if (param_5 == 3) {
                    pcVar11[0x38] = '\n'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                    return iVar6;
                } else if (param_5 == 4) {
                    pcVar11[0x38] = '\n'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                    pcVar11[0x48] = '\0'; pcVar11[0x49] = '\0'; pcVar11[0x4a] = '\0'; pcVar11[0x4b] = '\0';
                    *(float *)(pcVar11 + 0x40) = param_6;
                    pcVar11[0x0c] = '\0'; pcVar11[0x0d] = '\0'; pcVar11[0x0e] = '\0'; pcVar11[0x0f] = '@';
                    return iVar6;
                } else if (param_5 == 5) {
                    pcVar11[0x38] = '\x19'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                    return iVar6;
                } else {
                    return iVar6;
                }
                *(float *)(pcVar11 + 0x0c) =
                    ((float)(iVar7 % 0x32) + _DAT_00552598) * param_6 * _DAT_00552594;
                return iVar6;
            }
            // 0x47f fallthrough → treated as 0x4ab
            uVar8 = _rand();
            uVar8 = uVar8 & 0x80000007;
            if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
            *(unsigned int *)(pcVar11 + 0x38) = uVar8 + 0x14;
            return iVar6;
        }

        if (param_1 == 0x498) {
            // 0x498 — directional smoke/mist
            if (param_5 == 1) {
                iVar7 = _rand(); *pfVar2 = (float)(iVar7 % 0x168);
                iVar7 = _rand(); *(float *)(pcVar11 + 0x20) = (float)(iVar7 % 0x168);
                iVar7 = _rand(); *(float *)(pcVar11 + 0x24) = (float)(iVar7 % 0x168);
                Matrix_BuildFromEuler(pfVar2, local_30);
                local_3c = 0.0f; local_38 = -50.0f; local_34 = 0.0f;
                Vector_Rotate(&local_3c, local_30, pfVar3);
                return iVar6;
            }
            if (param_5 == 3) {
                pcVar11[0x38] = '\x05'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                return iVar6;
            }
            if (param_5 == 2) {
                pcVar11[0x38] = '\x0f'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
                *pfVar2 = 0.0f;
                pcVar11[0x20] = '\0'; pcVar11[0x21] = '\0'; pcVar11[0x22] = '\0'; pcVar11[0x23] = '\0';
                pcVar11[0x24] = '\0'; pcVar11[0x25] = '\0'; pcVar11[0x26] = '\0'; pcVar11[0x27] = '\0';
                *(unsigned int *)(pcVar11 + 0x68) = *(unsigned int *)(pcVar11 + 0x14);
                *(float *)(pcVar11 + 100) = *pfVar1;
                *(unsigned int *)(pcVar11 + 0x6c) = *(unsigned int *)(pcVar11 + 0x18);
                *pfVar3 = (*(float *)(param_7 + 0x10) - *pfVar1) * _DAT_00552a00;
                *(float *)(pcVar11 + 0x50) =
                    (*(float *)(param_7 + 0x14) - *(float *)(pcVar11 + 0x14)) * _DAT_00552a00;
                *(float *)(pcVar11 + 0x54) =
                    (*(float *)(param_7 + 0x18) - *(float *)(pcVar11 + 0x18)) * _DAT_00552a00;
                return iVar6;
            }
            return iVar6;
        }

        // 0x499 fallthrough → same as 0x4b5/0x4b6
        if (param_1 == 0x499) goto switchD_4b5;
        if (param_1 == 0x49c) {
            uVar8 = _rand();
            uVar8 = uVar8 & 0x80000007;
            if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
            *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 6) * param_6 * _DAT_005524f4;
            iVar7 = _rand();
            *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
            return iVar6;
        }
        // 2026-08-23 FIX: aca habia un `return iVar6;` que se tragaba TODOS los
        // tipos entre 0x49c y 0x4c4 sin case propio en este arbol — o sea 9 tipos
        // de particula que SI tienen su inicializacion en el switch de mas abajo:
        //   0x4a7, 0x4ab (fuego), 0x4ac, 0x4ad, 0x4b0 (Flame01), 0x4b5, 0x4b6,
        //   0x4bf, 0x4c0.
        // Quedaban con el `life` en el default 2 (en vez de 24 para el fuego) y
        // sin escala/rotacion propias: se creaban ~150 por segundo y morian a los
        // 2 frames, asi que en pantalla habia ~10 a la vez y el fuego se veia como
        // un puntito en vez de una masa.
        //
        // El `if (param_1 < 0x4c5)` de arriba NO es un rango real: es el arbol
        // binario de busqueda que genera Ghidra para un switch disperso, y el port
        // lo convirtio en un if/else con `return` que corta el fall-through.
        goto particleSpawn_outerSwitch;

        // 0x4b5/0x4b6 shared path
switchD_4b5:
        pcVar11[0x38] = '\f'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        uVar8 = _rand();
        uVar8 = uVar8 & 0x80000003;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
        *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 8) * _DAT_00552874;
        iVar7 = _rand(); *pfVar2 = (float)(iVar7 % 0x168);
        Matrix_BuildFromEuler(param_3, local_30);
        local_3c = 0.0f;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000000f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
        local_38 = -(float)(int)(uVar8 + 8);
        iVar7 = _rand(); local_34 = (float)(iVar7 % 6 + -3);
        Vector_Rotate(&local_3c, local_30, pfVar3);
        if (*(int *)(pcVar11 + 4) == 0x4b6) {
            pcVar11[0x28] = (char)-0x33; pcVar11[0x29] = (char)-0x34; pcVar11[0x2a] = (char)-0x34; pcVar11[0x2b] = '=';
            pcVar11[0x2c] = '\0'; pcVar11[0x2d] = '\0'; pcVar11[0x2e] = '\0'; pcVar11[0x2f] = '\0';
            pcVar11[0x30] = '\0'; pcVar11[0x31] = '\0'; pcVar11[0x32] = '\0'; pcVar11[0x33] = '\0';
        }
        return iVar6;
    }

    // ── switch por tipo (los `case` de aca cubren tipos a ambos lados de
    //    0x4c5; ver la nota del `goto particleSpawn_outerSwitch` de arriba) ────
particleSpawn_outerSwitch:
    switch (param_1) {
    case 0x4a7: {
        switch (param_5) {
        case 0:
            pcVar11[0x0c] = (char)-0x66; pcVar11[0x0d] = (char)-0x67;
            pcVar11[0x0e] = '\x19';      pcVar11[0x0f] = '>';
            break;
        case 1:
            pcVar11[0x38] = '\n'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            pcVar11[0x0c] = '\0'; pcVar11[0x0d] = '\0'; pcVar11[0x0e] = '\0'; pcVar11[0x0f] = '\0';
            break;
        case 2:
            pcVar11[0x38] = '\x14'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            pcVar11[0x0c] = '\0'; pcVar11[0x0d] = '\0'; pcVar11[0x0e] = (char)-0x80; pcVar11[0x0f] = '?';
            break;
        case 3:
            pcVar11[0x38] = '\x01'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            *(float *)(pcVar11 + 0x0c) = param_6;
            break;
        default:
            break;
        }
        *(float *)(pcVar11 + 0x28) = *param_4    + _DAT_00552504;
        *(float *)(pcVar11 + 0x2c) = param_4[1]  + _DAT_00552504;
        *(float *)(pcVar11 + 0x30) = param_4[2]  + _DAT_00552504;
        return iVar6;
    }

    case 0x4ab:
    case 0x4ad:
switchD_4ab:
        goto switchD_4ab_impl;

    case 0x4ac:
        pcVar11[0x38] = '\f'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        *pfVar3 = 0.0f;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x80000007;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
        pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
        pcVar11[0x0c] = (char)-0x33; pcVar11[0x0d] = (char)-0x34; pcVar11[0x0e] = 'L'; pcVar11[0x0f] = '?';
        *(float *)(pcVar11 + 0x50) = (float)(int)(uVar8 + 0x20) * _DAT_005529c0;
        iVar7 = _rand();
        *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
        return iVar6;

    case 0x4b0: {
        switch (param_5) {
        case 0:
            pcVar11[0x38] = '\x14'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            *pfVar3 = 0.0f;
            pcVar11[0x50] = '\0'; pcVar11[0x51] = '\0'; pcVar11[0x52] = '\0'; pcVar11[0x53] = '\0';
            uVar8 = _rand();
            uVar8 = uVar8 & 0x8000007f;
            if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffff80) + 1;
            *(float *)(pcVar11 + 0x54) = (float)(int)(uVar8 + 0x80) * _DAT_00552934;
            uVar8 = _rand();
            uVar8 = uVar8 & 0x8000003f;
            if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
            *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 0x40) * _DAT_005524f8;
            return iVar6;
        case 1:
            pcVar11[0x38] = '\x0f'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            uVar8 = _rand();
            uVar8 = uVar8 & 0x8000001f;
            if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
            *pfVar3 = 0.0f;
            *(float *)(pcVar11 + 0x0c) =
                (float)(int)(uVar8 + 0x20) * _DAT_005524f8 + *(float *)(pcVar11 + 0x0c);
            uVar8 = _rand();
            uVar8 = uVar8 & 0x80000003;
            if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
            break;
        case 2:
            pcVar11[0x38] = '\n'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            uVar8 = _rand();
            uVar8 = uVar8 & 0x8000001f;
            if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
            *(float *)(pcVar11 + 0x0c) =
                (float)(int)(uVar8 + 0x20) * _DAT_005524f8 + *(float *)(pcVar11 + 0x0c);
            iVar7 = _rand();
            fVar4 = *param_4; *pfVar3 = 0.0f;
            pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
            fVar5 = (float)(iVar7 % 0x50) * fVar4 * _DAT_00552594;
            *(float *)(pcVar11 + 0x50) = fVar5;
            FUN_0043e570(pfVar1, pfVar2, pfVar3);
            fVar4 = *param_4; *pfVar3 = 0.0f;
            fVar4 = (fVar4 - fVar5) * _DAT_00552a48;
            pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
            *(float *)(pcVar11 + 0x50) = fVar4;
            fVar13 = (long double)fsin((long double)DAT_05826e08 * (long double)_DAT_005528e0);
            fVar13 = fVar13 * (long double)_DAT_005528b8 + (long double)_DAT_00552928;
            *(float *)(pcVar11 + 0x28) = (float)fVar13;
            fVar13 = fVar13 * (long double)_DAT_00552504;
            *(float *)(pcVar11 + 0x2c) = (float)fVar13;
            *(float *)(pcVar11 + 0x30) = (float)fVar13;
            return iVar6;
        case 3:
            pcVar11[0x38] = '\n'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            uVar8 = _rand();
            uVar8 = uVar8 & 0x8000001f;
            if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
            *pfVar3 = 0.0f;
            pcVar11[0x50] = '\0'; pcVar11[0x51] = '\0'; pcVar11[0x52] = '\0'; pcVar11[0x53] = '\0';
            fVar4 = (float)(int)(uVar8 + 0x20) * _DAT_005524f8;
            pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
            *(float *)(pcVar11 + 0x0c) = fVar4 + *(float *)(pcVar11 + 0x0c);
            fVar13 = (long double)fsin((long double)DAT_05826e08 * (long double)_DAT_005528e0);
            fVar13 = fVar13 * (long double)_DAT_005528b8 + (long double)_DAT_00552928;
            *(float *)(pcVar11 + 0x28) = (float)fVar13;
            fVar13 = fVar13 * (long double)_DAT_00552504;
            *(float *)(pcVar11 + 0x2c) = (float)fVar13;
            *(float *)(pcVar11 + 0x30) = (float)fVar13;
            return iVar6;
        case 4:
            pcVar11[0x38] = '\x05'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            uVar8 = _rand();
            uVar8 = uVar8 & 0x8000001f;
            if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
            pcVar11[0x48] = '\0'; pcVar11[0x49] = '\0'; pcVar11[0x4a] = ' '; pcVar11[0x4b] = 'A';
            iVar7 = *(int *)(pcVar11 + 0x3c);
            *(float *)(pcVar11 + 0x0c) =
                (float)(int)(uVar8 + 0x20) * _DAT_005524f8 + *(float *)(pcVar11 + 0x0c);
            fVar12 = (long double)DAT_05826e08;
            fVar13 = (long double)_DAT_005528e0;
            *(unsigned int *)(pcVar11 + 100) = *(unsigned int *)(iVar7 + 0x10);
            fVar13 = (long double)fsin(fVar12 * fVar13);
            *(unsigned int *)(pcVar11 + 0x68) = *(unsigned int *)(iVar7 + 0x14);
            *(unsigned int *)(pcVar11 + 0x6c) = *(unsigned int *)(iVar7 + 0x18);
            fVar13 = fVar13 * (long double)_DAT_005528b8 + (long double)_DAT_00552928;
            *(float *)(pcVar11 + 0x28) = (float)fVar13;
            fVar13 = fVar13 * (long double)_DAT_00552504;
            *(float *)(pcVar11 + 0x2c) = (float)fVar13;
            *(float *)(pcVar11 + 0x30) = (float)fVar13;
            return iVar6;
        case 5:
            pcVar11[0x38] = '\x04'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            *pfVar3 = 0.0f;
            uVar8 = _rand();
            uVar8 = uVar8 & 0x80000003;
            if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
            break;
        default:
            return iVar6;
        }
        fVar4 = (float)(int)(uVar8 + 4) * _DAT_00552934;
        pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
        *(float *)(pcVar11 + 0x50) = fVar4;
        return iVar6;
    }

    case 0x4b5:
    case 0x4b6:
        goto switchD_4b5;

    case 0x4bf:
        pcVar11[0x38] = '\x14'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        *(float *)(pcVar11 + 0x0c) = param_6;
        FUN_00404bc0(0x2c, 0, 0);
        return iVar6;

    case 0x4c0:
        pcVar11[0x38] = '\f'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        *(float *)(pcVar11 + 0x0c) = param_6;
        FUN_00404bc0(0x2c, 0, 0);
        return iVar6;

    case 0x4c5:
        pcVar11[0x38] = ' '; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000001f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
        *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 0x20) * _DAT_005524f8;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000000f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
        *pfVar1 = (float)(int)(uVar8 - 8) + *pfVar1;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000000f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
        *(float *)(pcVar11 + 0x14) = (float)(int)(uVar8 - 8) + *(float *)(pcVar11 + 0x14);
        if (param_5 == 0) {
            Matrix_BuildFromEuler(param_3, local_30);
            local_38 = 3.0f;
        } else {
            iVar7 = _rand();
            *(float *)(pcVar11 + 0x24) = (float)(iVar7 % 0x168);
            Matrix_BuildFromEuler(pfVar2, local_30);
            local_38 = 15.0f;
        }
        local_3c = 0.0f; local_34 = 0.0f;
        Vector_Rotate(&local_3c, local_30, pfVar3);
        iVar7 = _rand(); *pfVar2 = (float)(iVar7 % 0x168);
        return iVar6;

    case 0x4c6:
        pcVar11[0x38] = '2'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000001f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
        *pfVar2 = 0.0f;
        pcVar11[0x20] = '\0'; pcVar11[0x21] = '\0'; pcVar11[0x22] = '\0'; pcVar11[0x23] = '\0';
        *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 0x40) * _DAT_005524f8;
        iVar7 = _rand(); *(float *)(pcVar11 + 0x24) = (float)(iVar7 % 0x168);
        Matrix_BuildFromEuler(pfVar2, local_30);
        local_3c = 0.0f;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000003f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
        local_38 = (float)(int)uVar8;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000000f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
        local_34 = (float)(int)(uVar8 + 0x10);
        Vector_Rotate(&local_3c, local_30, (float *)(pcVar11 + 100));
        *pfVar2 = 0.0f;
        pcVar11[0x20] = '\0'; pcVar11[0x21] = '\0'; pcVar11[0x22] = '\0'; pcVar11[0x23] = '\0';
        pcVar11[0x24] = '\0'; pcVar11[0x25] = '\0'; pcVar11[0x26] = '\0'; pcVar11[0x27] = '\0';
        return iVar6;

    case 0x4ce:
        *pfVar2 = 45.0f;
        pcVar11[0x38] = '\x12'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        return iVar6;

    case 0x4cf:
        pcVar11[0x38] = '$'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        *pfVar2 = 45.0f;
        if (param_5 != 99) FUN_00475170((int)pcVar11);
        return iVar6;

    case 0x4d0:
        pcVar11[0x38] = '\x12'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        FUN_00475170((int)pcVar11);
        return iVar6;

    case 0x4d2:
        pcVar11[0x38] = '\x14'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        return iVar6;

    case 0x4d9:
        iVar7 = _rand(); *(int *)(pcVar11 + 0x38) = iVar7 % 10 + 0x1e;
        iVar7 = _rand();
        pcVar11[0x28] = '\0'; pcVar11[0x29] = '\0'; pcVar11[0x2a] = (char)-0x80; pcVar11[0x2b] = '?';
        pcVar11[0x2c] = '\0'; pcVar11[0x2d] = '\0'; pcVar11[0x2e] = (char)-0x80; pcVar11[0x2f] = '?';
        pcVar11[0x30] = '\0'; pcVar11[0x31] = '\0'; pcVar11[0x32] = (char)-0x80; pcVar11[0x33] = '?';
        *(float *)(pcVar11 + 0x0c) = (float)(iVar7 % 6 + 4) * _DAT_00552910;
        return iVar6;

    case 0x4da: case 0x4db: case 0x4dc:
        iVar7 = _rand(); *(int *)(pcVar11 + 0x38) = iVar7 % 10 + 0xf;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x80000007;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff8) + 1;
        pcVar11[0x28] = '\0'; pcVar11[0x29] = '\0'; pcVar11[0x2a] = (char)-0x80; pcVar11[0x2b] = '?';
        pcVar11[0x2c] = '\0'; pcVar11[0x2d] = '\0'; pcVar11[0x2e] = (char)-0x80; pcVar11[0x2f] = '?';
        pcVar11[0x30] = '\0'; pcVar11[0x31] = '\0'; pcVar11[0x32] = (char)-0x80; pcVar11[0x33] = '?';
        *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 4) * _DAT_00552910;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000001f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
        *pfVar3 = (float)(int)(uVar8 - 0x10) * _DAT_005524f4;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000001f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
        *(float *)(pcVar11 + 0x50) = (float)(int)(uVar8 - 0x10) * _DAT_005524f4;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000001f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
        *(float *)(pcVar11 + 0x54) = (float)(int)(uVar8 - 0x20) * _DAT_005524f4;
        return iVar6;

    case 0x4df:
        pcVar11[0x24] = '\0'; pcVar11[0x25] = '\0'; pcVar11[0x26] = '\0'; pcVar11[0x27] = '\0';
        pcVar11[0x20] = '\0'; pcVar11[0x21] = '\0'; pcVar11[0x22] = '\0'; pcVar11[0x23] = '\0';
        *pfVar2 = 0.0f;
        iVar7 = _rand(); *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
        if (param_5 != -1) {
            iVar7 = param_5 + 0xf;
            iVar10 = iVar7 / 2;
            iVar9 = _rand(); *pfVar3                  = (float)(iVar9 % iVar7 - iVar10) * _DAT_00552934;
            iVar9 = _rand(); *(float *)(pcVar11 + 0x50) = (float)(iVar9 % iVar7 - iVar10) * _DAT_00552934;
            iVar9 = _rand();
            pcVar11[0x38] = '2'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            pcVar11[0x0c] = (char)-0x33; pcVar11[0x0d] = (char)-0x34; pcVar11[0x0e] = (char)-0x34; pcVar11[0x0f] = '=';
            *(float *)(pcVar11 + 0x54) = (float)(iVar9 % iVar7 - iVar10) * _DAT_00552934 + _DAT_00552488;
            return iVar6;
        }
        pcVar11[0x38] = '\x02'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        pcVar11[0x0c] = '\0'; pcVar11[0x0d] = '\0'; pcVar11[0x0e] = (char)-0x80; pcVar11[0x0f] = '>';
        return iVar6;

    case 0x4e1: {
        pcVar11[0x38] = '<'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        if ((param_5 == 0) || (param_5 == 3)) {
            iVar7 = _rand();
            fVar4 = (float)(iVar7 % 100) * _DAT_00552594 * _DAT_00552650;
            fVar4 = fVar4 + _DAT_0055256c;
        } else if (param_5 == 4) {
            pcVar11[0x38] = '('; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            iVar7 = _rand();
            fVar4 = (float)(iVar7 % 100) * _DAT_00552594 * _DAT_00552660;
            fVar4 = fVar4 + _DAT_0055256c;
        } else if (param_5 == 2) {
            iVar7 = _rand();
            fVar4 = ((float)(iVar7 % 100) * _DAT_00552594 + _DAT_0055256c) * _DAT_00552660;
        } else {
            goto lab_4e1_skip_size;
        }
        *(float *)(pcVar11 + 0x48) = fVar4;
lab_4e1_skip_size:
        iVar7 = _rand(); *pfVar3 = (float)(iVar7 % 300 + -0x96);
        iVar7 = _rand(); *(float *)(pcVar11 + 0x0c) = (float)(iVar7 % 6) * _DAT_005524f8 + param_6;
        iVar7 = _rand();
        *pfVar2 = 0.0f;
        pcVar11[0x20] = '\0'; pcVar11[0x21] = '\0'; pcVar11[0x22] = '\0'; pcVar11[0x23] = '\0';
        pcVar11[0x24] = '\0'; pcVar11[0x25] = '\0'; pcVar11[0x26] = '\0'; pcVar11[0x27] = '\0';
        *(unsigned int *)(pcVar11 + 0x68) = *(unsigned int *)(pcVar11 + 0x14);
        *(unsigned int *)(pcVar11 + 0x6c) = *(unsigned int *)(pcVar11 + 0x18);
        *(float *)(pcVar11 + 100) = *pfVar1;
        *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
        return iVar6;
    }

    case 0x4f4:
        if (param_5 != 6) {
            pcVar11[0x38] = '\x1e'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
            iVar7 = _rand(); *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 1000);
            iVar7 = _rand(); *pfVar1 = (float)(iVar7 % 500 + -0xfa) + *pfVar1;
            iVar7 = _rand(); *(float *)(pcVar11 + 0x14) = (float)(iVar7 % 500 + -0xfa) + *(float *)(pcVar11 + 0x14);
            iVar7 = _rand(); *(float *)(pcVar11 + 0x18) = (float)(iVar7 % 0x14 + 0x14) + *(float *)(pcVar11 + 0x18);
            iVar7 = _rand();
            *(unsigned int *)(pcVar11 + 0x6c) = *(unsigned int *)(pcVar11 + 0x18);
            *(float *)(pcVar11 + 0x68) = (float)(iVar7 % 100) * _DAT_00552594;
            iVar7 = _rand(); *(float *)(pcVar11 + 0x0c) = (float)(iVar7 % 0x14 + 0xb4) * _DAT_005524f8;
            iVar7 = _rand();
            *pfVar3 = 0.0f;
            pcVar11[0x50] = '\0'; pcVar11[0x51] = '\0'; pcVar11[0x52] = '\0'; pcVar11[0x53] = '\0';
            pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
            *(float *)(pcVar11 + 0x58) = (float)(iVar7 % 0x1e) * _DAT_00552594 + param_6;
            return iVar6;
        }
        iVar7 = _rand(); *(int *)(pcVar11 + 0x38) = iVar7 % 5 + 0x19;
        iVar7 = _rand();
        pcVar11[0x0c] = (char)-0x33; pcVar11[0x0d] = (char)-0x34; pcVar11[0x0e] = 'L'; pcVar11[0x0f] = '>';
        *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
        iVar7 = _rand();
        pcVar11[0x50] = '\0'; pcVar11[0x51] = '\0'; pcVar11[0x52] = '\0'; pcVar11[0x53] = '\0';
        *pfVar3 = (float)(iVar7 % 10 + 5) * _DAT_005528b4;
        iVar7 = _rand(); fVar4 = (float)(iVar7 % 10 + 5);
        break;   // → shared tail: fVar4 * _DAT_005526e4

    case 0x565:
        iVar7 = _rand();
        pcVar11[0x40] = '\0'; pcVar11[0x41] = '\0'; pcVar11[0x42] = '\0'; pcVar11[0x43] = '\0';
        *(int *)(pcVar11 + 0x38) = iVar7 % 5 + 0x14;
        if (*(int *)(pcVar11 + 8) != 0) {
            iVar7 = _rand(); *(float *)(pcVar11 + 0x0c) = (float)(iVar7 % 10) * _DAT_005524f4 + _DAT_0055256c;
            iVar7 = _rand(); *pfVar3                   = (float)(iVar7 % 10 + 5)  * _DAT_005526e4;
            iVar7 = _rand(); *(float *)(pcVar11 + 0x50) = (float)(iVar7 % 10 + -5) * _DAT_005526e4;
            iVar7 = _rand(); *(float *)(pcVar11 + 0x54) = (float)(iVar7 % 10 + 5)  * _DAT_005524f4;
            return iVar6;
        }
        iVar7 = _rand(); *(float *)(pcVar11 + 0x0c) = (float)(iVar7 % 10) * _DAT_00552914 + _DAT_00552504;
        iVar7 = _rand(); *pfVar3                   = (float)(iVar7 % 10 + 5)  * _DAT_005528b4;
        iVar7 = _rand(); *(float *)(pcVar11 + 0x50) = (float)(iVar7 % 10 + -5) * _DAT_005528b4;
        iVar7 = _rand(); fVar4 = (float)(iVar7 % 10 + 5);
        break;   // → shared tail

    case 0x566:
        iVar7 = _rand(); *(int *)(pcVar11 + 0x38) = iVar7 % 5 + 0x19;
        iVar7 = _rand();
        pcVar11[0x0c] = '\0'; pcVar11[0x0d] = '\0'; pcVar11[0x0e] = '\0'; pcVar11[0x0f] = '?';
        *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
        iVar7 = _rand();
        pcVar11[0x50] = '\0'; pcVar11[0x51] = '\0'; pcVar11[0x52] = '\0'; pcVar11[0x53] = '\0';
        *pfVar3 = (float)(iVar7 % 10 + 5) * _DAT_005528b4;
        iVar7 = _rand(); fVar4 = (float)(iVar7 % 10 + 5);
        break;   // → shared tail

    case 0x567:
        pcVar11[0x38] = '\x18'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        *pfVar3 = 0.0f;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000000f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
        pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
        *(float *)(pcVar11 + 0x50) = (float)(int)(uVar8 + 0x20) * _DAT_005529c8;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000007f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffff80) + 1;
        *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 0x80) * _DAT_005524f8;
        iVar7 = _rand();
        pcVar11[0x28] = '\0'; pcVar11[0x29] = '\0'; pcVar11[0x2a] = '\0'; pcVar11[0x2b] = '?';
        pcVar11[0x2c] = '\0'; pcVar11[0x2d] = '\0'; pcVar11[0x2e] = '\0'; pcVar11[0x2f] = '?';
        pcVar11[0x30] = '\0'; pcVar11[0x31] = '\0'; pcVar11[0x32] = '\0'; pcVar11[0x33] = '?';
        *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
        return iVar6;

    case 0x596:
        pcVar11[0x38] = '\x01'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        pcVar11[0x0c] = '\0'; pcVar11[0x0d] = '\0'; pcVar11[0x0e] = (char)-0x80; pcVar11[0x0f] = '?';
        return iVar6;

    case 0x598:
        pcVar11[0x38] = '\x05'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        pcVar11[0x48] = '\0'; pcVar11[0x49] = '\0'; pcVar11[0x4a] = '\0'; pcVar11[0x4b] = '\0';
        pcVar11[0x0c] = '\0'; pcVar11[0x0d] = '\0'; pcVar11[0x0e] = (char)-0x80; pcVar11[0x0f] = '@';
        if (param_5 == 0) {
            pcVar11[0x40] = '\0'; pcVar11[0x41] = '\0'; pcVar11[0x42] = '\0'; pcVar11[0x43] = '\0';
        } else if (param_5 == 1) {
            pcVar11[0x40] = '\0'; pcVar11[0x41] = '\0'; pcVar11[0x42] = '4'; pcVar11[0x43] = 'B';
        } else if (param_5 == 2) {
            pcVar11[0x40] = '\0'; pcVar11[0x41] = (char)-0x80; pcVar11[0x42] = (char)-99; pcVar11[0x43] = 'C';
        }
        return iVar6;

    case 0x599:
        pcVar11[0x38] = '\a'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
        iVar7 = _rand();
        pcVar11[0x0c] = (char)-0x66; pcVar11[0x0d] = (char)-0x67; pcVar11[0x0e] = (char)-0x67; pcVar11[0x0f] = '?';
        *(float *)(pcVar11 + 0x48) = (float)(iVar7 % 0x32 + 0x96) * _DAT_00552594;
        iVar7 = _rand();
        *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
        return iVar6;

    default:
        return iVar6;
    }

    // ── shared tail for 0x4f4/0x565/0x566 break paths ────────────────────────
    *(float *)(pcVar11 + 0x54) = fVar4 * _DAT_005526e4;
    return iVar6;

    // ── 0x4ab/0x4ad/0x47f shared implementation ──────────────────────────────
switchD_4ab_impl:
    pcVar11[0x38] = '\x18'; pcVar11[0x39] = '\0'; pcVar11[0x3a] = '\0'; pcVar11[0x3b] = '\0';
    *pfVar3 = 0.0f;
    uVar8 = _rand();
    uVar8 = uVar8 & 0x8000000f;
    if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
    pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
    iVar7 = *(int *)(pcVar11 + 8);
    *(float *)(pcVar11 + 0x50) = (float)(int)(uVar8 + 0x20) * _DAT_005529c8;
    if ((iVar7 == 0) || (iVar7 == 5)) {
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000003f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
        fVar4 = (float)(int)(uVar8 + 0x80) * _DAT_005524f8;
    } else if (iVar7 == 7) {
        *pfVar3 = 0.0f;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000001f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffe0) + 1;
        pcVar11[0x54] = '\0'; pcVar11[0x55] = '\0'; pcVar11[0x56] = '\0'; pcVar11[0x57] = '\0';
        *(float *)(pcVar11 + 0x50) = (float)(int)(uVar8 - 0x10) * _DAT_005529c8;
        uVar8 = _rand();
        uVar8 = uVar8 & 0x8000003f;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffffc0) + 1;
        fVar4 = (float)(int)(uVar8 + 0x80) * _DAT_00552ac0;
    } else if (iVar7 == 9) {
        iVar7 = _rand();
        int s = iVar7 % 0x3c - 0x1e;
        *(float *)(pcVar11 + 100) = (float)(int)s;
        *(float *)(pcVar11 + 0x68) = (float)(int)s;
        *(float *)(pcVar11 + 0x6c) =
            _DAT_00552974 -
            (float)(int)((s ^ (s >> 0x1f)) - (s >> 0x1f)) * _DAT_005528f0;
        goto lab_4ab_setscale;
    } else {
        goto lab_4ab_setscale;
    }
    *(float *)(pcVar11 + 0x0c) = fVar4;
lab_4ab_setscale:
    if (*(int *)(pcVar11 + 8) == 1) {
        uVar8 = _rand();
        uVar8 = uVar8 & 0x80000003;
        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffffc) + 1;
        *(float *)(pcVar11 + 0x0c) = (float)(int)(uVar8 + 10) * _DAT_005524f8;
    }
    iVar7 = _rand();
    *(float *)(pcVar11 + 0x40) = (float)(iVar7 % 0x168);
    return iVar6;
}
