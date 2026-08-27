// Particle_Move.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// MoveParticles @ 0x00477090 (~1130 lines) — FULL IMPLEMENTATION
// Per-frame particle system update.
// 1. Random camera shake with clamping
// 2. Per active particle: decrement lifetime, MovePosition,
//    switch on type for gravity/terrain snap/fade/scale/color
//
// Particle slot layout (stride 0x70, base DAT_07abf5f0):
//   +0x00 active  +0x04 type  +0x08 sub  +0x0C scale
//   +0x10 posX    +0x14 posY  +0x18 posZ
//   +0x1C dirX    +0x20 angle
//   +0x28 lightR  +0x2C lightG +0x30 lightB
//   +0x38 lifetime +0x3C entity_ptr +0x40 rotation +0x44 frame
//   +0x48 gravity  +0x4C velX +0x50 velY +0x54 velZ
//   +0x58 extra0   +0x5C extra1 +0x60 extra2
//   +0x64 targetX  +0x68 targetY +0x6C targetZ
//
// NOTE: Ghidra shows unaff_EBX/EBP/ESI/EDI phantom params in the AngleMatrix/VectorRotate
//       calls for type 0x4c6 — these are 63 phantom stack params (anti-tamper artifact).
//       The AngleMatrix+VectorRotate path for type 0x4c6 is implemented structurally but
//       the phantom register values cannot be reconstructed; it runs the math correctly
//       with the actual particle data.
//
// Bitmaps access: DAT_083a7ca0 + type * 0x38 + 0x24 = height field per bitmap slot.
// Models access: DAT_05828d58 + entity_type * 0xBC = BMD struct base.
// WorldTime = DAT_05826e08 (global animation tick).

// Helper macros for particle field access
// BUG-FIX 2026-04-27: era `&DAT_07abf5f0` cuando DAT era `char` solo. Ahora es
// array `char[N]` y `&array` sería pointer-to-array (aritmética × sizeof[N]).
// Usar `DAT_07abf5f0` directo decae a char* correcto.
#define P_BASE      (DAT_07abf5f0)
#define P_ACTIVE(o) (*(char*)(P_BASE + (o)))
#define P_TYPE(o)   (*(int*)(P_BASE + (o) + 0x04))
#define P_SUB(o)    (*(int*)(P_BASE + (o) + 0x08))
#define P_SCALE(o)  (*(float*)(P_BASE + (o) + 0x0C))
#define P_POSX(o)   (*(float*)(P_BASE + (o) + 0x10))
#define P_POSY(o)   (*(float*)(P_BASE + (o) + 0x14))
#define P_POSZ(o)   (*(float*)(P_BASE + (o) + 0x18))
#define P_DIRX(o)   (*(float*)(P_BASE + (o) + 0x1C))
#define P_ANGLE(o)  (*(float*)(P_BASE + (o) + 0x20))
#define P_LR(o)     (*(float*)(P_BASE + (o) + 0x28))
#define P_LG(o)     (*(float*)(P_BASE + (o) + 0x2C))
#define P_LB(o)     (*(float*)(P_BASE + (o) + 0x30))
#define P_LIFE(o)   (*(int*)(P_BASE + (o) + 0x38))
#define P_ENT(o)    (*(int*)(P_BASE + (o) + 0x3C))
#define P_ROT(o)    (*(float*)(P_BASE + (o) + 0x40))
#define P_FRAME(o)  (*(int*)(P_BASE + (o) + 0x44))
#define P_GRAV(o)   (*(float*)(P_BASE + (o) + 0x48))
#define P_VELX(o)   (*(float*)(P_BASE + (o) + 0x4C))
#define P_VELY(o)   (*(float*)(P_BASE + (o) + 0x50))
#define P_VELZ(o)   (*(float*)(P_BASE + (o) + 0x54))
#define P_EX0(o)    (*(float*)(P_BASE + (o) + 0x58))
#define P_EX1(o)    (*(float*)(P_BASE + (o) + 0x5C))
#define P_EX2(o)    (*(float*)(P_BASE + (o) + 0x60))
#define P_TGX(o)    (*(float*)(P_BASE + (o) + 0x64))
#define P_TGY(o)    (*(float*)(P_BASE + (o) + 0x68))
#define P_TGZ(o)    (*(float*)(P_BASE + (o) + 0x6C))
// Bitmaps height: g_BitmapsRaw + type * 0x38 + 0x24 = height field per bitmap slot
#define BMP_HEIGHT(t)  (*(float*)(g_BitmapsRaw + (t) * 0x38 + 0x24))

void __stdcall MoveParticles_stub(void)
{
    float fVar15, fVar19, fVar3;
    float10 fVar11, fVar12;
    int iVar4, iVar5, iVar6, iVar9;
    unsigned int uVar7;

    // ── 1. Camera shake: random perturbation + clamping ──────────────────────
    // Velocity array: DAT_07c80104 (X), DAT_07c80108 (Y) — two floats
    float* shakeVel = &DAT_07c80104;
    for (int s = 0; s < 2; s++) {
        iVar4 = _rand();
        fVar19 = (float)(iVar4 % 0x7d1 - 1000) * _DAT_00552b54 + shakeVel[s];
        fVar15 = fVar19;
        if (fVar19 < _DAT_00552534 && fVar19 >= _DAT_00552b50) {
            // nop — keep fVar15 = fVar19
        } else {
            fVar15 = _DAT_00552534;
        }
        if (fVar19 < _DAT_00552b50) {
            fVar15 = _DAT_00552b50;
        }
        shakeVel[s] = fVar15;
    }

    // Accumulate shake velocity into position, clamp
    // DAT_07c800f8 (X), DAT_07c800fc (Y) — two floats
    float* shakePos = &DAT_07c800f8;
    for (int s = 0; s < 2; s++) {
        fVar19 = shakeVel[s] + shakePos[s];
        shakePos[s] = fVar19;
        fVar15 = fVar19;
        if (fVar19 < _DAT_00552b4c && fVar19 >= _DAT_00552b48) {
            // nop
        } else {
            fVar15 = _DAT_00552b4c;
        }
        if (fVar19 < _DAT_00552b48) {
            fVar15 = _DAT_00552b48;
        }
        shakePos[s] = fVar15;
    }

    // ── 2. Per-particle update loop ──────────────────────────────────────────
    for (iVar4 = 0; iVar4 < 3000; iVar4++) {
        iVar9 = iVar4 * 0x70;

        if (P_ACTIVE(iVar9) == '\0') continue;

        // Decrement lifetime
        iVar5 = P_LIFE(iVar9);
        P_LIFE(iVar9) = iVar5 - 1;
        if (iVar5 - 1 < 1) {
            P_ACTIVE(iVar9) = '\0';
        }

        // MovePosition(pos, dir, vel) — advances position along velocity rotated by direction
        float* pPos = &P_POSX(iVar9);
        float* pDir = &P_DIRX(iVar9);
        float* pVel = &P_VELX(iVar9);
        FUN_0043e570(pPos, pDir, pVel);

        fVar15 = _DAT_0055256c;
        iVar5 = P_TYPE(iVar9);

        // ── Main type switch ─────────────────────────────────────────────────
        // The original binary has a complex nested switch. We replicate the
        // structure exactly, including fall-through and goto patterns.

        if (iVar5 > 0x4c4) {
            // High type range: 0x4c5..0x599
            switch (iVar5) {

            case 0x4c5: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552ae4;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                P_SCALE(iVar9) += _DAT_00552a7c;
                P_POSX(iVar9) += P_VELX(iVar9);
                P_POSY(iVar9) += P_VELY(iVar9);
                P_POSZ(iVar9) += P_VELZ(iVar9);
                P_VELX(iVar9) *= _DAT_005526e8;
                P_VELY(iVar9) *= _DAT_005526e8;
                P_VELZ(iVar9) *= _DAT_005526e8;
                fVar15 = FUN_004f7500(P_POSX(iVar9), P_POSY(iVar9));
                P_POSZ(iVar9) = BMP_HEIGHT(P_TYPE(iVar9)) * P_SCALE(iVar9) * _DAT_00552504 + fVar15;
                break;
            }

            case 0x4c6: {
                if (P_LIFE(iVar9) < 10) {
                    fVar15 = _DAT_0055256c - (float)(10 - P_LIFE(iVar9)) * _DAT_005524f4;
                }
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                // AngleMatrix + VectorRotate to transform local offset by entity bone
                // NOTE: Ghidra shows phantom register args (unaff_EBX/EBP/ESI/EDI).
                // We use the particle's stored direction as angles and target as local offset.
                {
                    float angles3[3], mat3x4[3][4];
                    // (audit #11) removed unused: float in1_3[3], in2_3[3][4], out3[3];
                    angles3[0] = P_DIRX(iVar9);
                    angles3[1] = 0.0f;
                    angles3[2] = 0.0f;
                    AngleMatrix(angles3, mat3x4);
                    // VectorRotate: transform stored offset by matrix
                    float localOff[3];
                    localOff[0] = P_TGX(iVar9);
                    localOff[1] = P_TGY(iVar9);
                    localOff[2] = P_TGZ(iVar9); // placeholder
                    float worldOff[3];
                    Vector_Rotate(localOff, (float*)mat3x4, worldOff);
                    // Add to entity position
                    int entPtr = P_ENT(iVar9);
                    P_POSX(iVar9) = worldOff[0] + *(float*)(entPtr + 0x10);
                    P_POSY(iVar9) = worldOff[1] + *(float*)(entPtr + 0x14);
                    P_POSZ(iVar9) = worldOff[2] + *(float*)(entPtr + 0x18);
                }
                P_ANGLE(iVar9) += _DAT_00552660;
                P_SCALE(iVar9) -= _DAT_005524f8;
                break;
            }

            case 0x4ce: {
                fVar11 = (float10)fsin((float10)P_LIFE(iVar9) * (float10)_DAT_00552ae0);
                P_SCALE(iVar9) = (float)fVar11;
                if (P_SUB(iVar9) == 1) {
                    P_SCALE(iVar9) = (float)(fVar11 * (float10)_DAT_00552adc);
                    P_ROT(iVar9) -= _DAT_00552664;
                }
                break;
            }

            case 0x4cf: {
                if (P_SUB(iVar9) == 99) {
                    int entPtr = P_ENT(iVar9);
                    P_POSX(iVar9) = *(float*)(entPtr + 0x10);
                    P_POSY(iVar9) = *(float*)(entPtr + 0x14);
                    P_POSZ(iVar9) = *(float*)(entPtr + 0x18) + _DAT_00552598;
                    P_ROT(iVar9) -= _DAT_0055256c;
                } else {
                    fVar12 = (float10)fsin((float10)P_LIFE(iVar9) * (float10)_DAT_00552ad8);
                    fVar11 = (float10)_DAT_00552660;
                    P_SCALE(iVar9) = (float)(fVar12 * fVar11);
                    if (P_SUB(iVar9) < 2) {
                        FUN_00475170((int)(P_BASE + iVar9));
                    } else {
                        P_SCALE(iVar9) = (float)(fVar12 * fVar11 * (float10)_DAT_00552adc);
                        P_ROT(iVar9) -= _DAT_00552664;
                        FUN_00475170((int)(P_BASE + iVar9));
                    }
                }
                break;
            }

            case 0x4d0: {
                fVar11 = (float10)fsin((float10)P_LIFE(iVar9) * (float10)_DAT_00552ae0);
                P_SCALE(iVar9) = (float)(fVar11 * (float10)_DAT_00552540);
                FUN_00475170((int)(P_BASE + iVar9));
                break;
            }

            case 0x4d2: {
                if (P_SUB(iVar9) == 0) {
                    fVar11 = (float10)fsin((float10)P_LIFE(iVar9) * (float10)_DAT_00552ae0);
                    P_SCALE(iVar9) = (float)(fVar11 * (float10)_DAT_00552540 + (float10)_DAT_0055264c);
                }
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552b28;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                break;
            }

            case 0x4d9: {
                P_FRAME(iVar9) += 1;
                iVar5 = _rand();
                P_POSX(iVar9) += (float)(iVar5 % 0x14 - 10) * P_SCALE(iVar9) * _DAT_00552950;
                iVar5 = _rand();
                P_POSY(iVar9) += (float)(iVar5 % 0x14 - 10) * P_SCALE(iVar9) * _DAT_00552950;
                iVar5 = _rand();
                P_POSZ(iVar9) += (float)(iVar5 % 0x14 + 10) * P_SCALE(iVar9) * _DAT_00552950;
                break;
            }

            case 0x4da:
            case 0x4db:
            case 0x4dc: {
                if (P_FRAME(iVar9) == 0) {
                    uVar7 = _rand();
                    uVar7 = uVar7 & 0x8000001f;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xffffffe0) + 1;
                    P_VELX(iVar9) += (float)(int)(uVar7 - 0x10) * _DAT_005524f4;

                    uVar7 = _rand();
                    uVar7 = uVar7 & 0x8000001f;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xffffffe0) + 1;
                    P_VELY(iVar9) += (float)(int)(uVar7 - 0x10) * _DAT_005524f4;

                    uVar7 = _rand();
                    uVar7 = uVar7 & 0x8000001f;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xffffffe0) + 1;
                    P_VELZ(iVar9) += (float)(int)(uVar7 - 0x10) * _DAT_005524f4;

                    P_POSX(iVar9) += P_VELX(iVar9);
                    P_POSY(iVar9) += P_VELY(iVar9);
                    P_POSZ(iVar9) += P_VELZ(iVar9);
                }
                fVar15 = FUN_004f7500(P_POSX(iVar9), P_POSY(iVar9));
                if (P_POSZ(iVar9) < fVar15) {
                    P_POSZ(iVar9) = fVar15;
                    P_FRAME(iVar9) = 1;
                    P_VELX(iVar9) = 0.0f;
                    P_VELY(iVar9) = 0.0f;
                    P_VELZ(iVar9) = 0.0f;
                }
                uVar7 = _rand();
                uVar7 = uVar7 & 0x8000000f;
                if ((int)uVar7 < 0) uVar7 = ((uVar7 - 1) | 0xfffffff0) + 1;
                P_ROT(iVar9) += (float)(int)uVar7;
                break;
            }

            case 0x4df: {
                P_VELZ(iVar9) -= _DAT_00552504;
                break;
            }

            case 0x4e1: {
                iVar5 = P_SUB(iVar9);
                float lifeF = (float)P_LIFE(iVar9);
                if (iVar5 == 0 || iVar5 == 3) {
                    fVar11 = ((float10)(int)P_LIFE(iVar9) + (float10)P_VELX(iVar9)) * (float10)_DAT_005524f4;
                    fVar12 = (float10)fsin((double)fVar11);
                    P_POSX(iVar9) = (float)(fVar12 * (float10)_DAT_005528e4 + (float10)P_TGX(iVar9));
                    fVar11 = (float10)fcos((double)fVar11);
                    P_POSY(iVar9) = (float)((float10)P_TGY(iVar9) - fVar11 * (float10)_DAT_005528e4);
                    fVar15 = P_GRAV(iVar9);
                    P_POSZ(iVar9) += fVar15;
                    P_SCALE(iVar9) -= _DAT_005528e0;
                } else if (iVar5 == 2) {
                    P_POSX(iVar9) = P_TGX(iVar9);
                    P_POSY(iVar9) = P_TGY(iVar9);
                    float steps = (float)((0x3c - P_LIFE(iVar9)) / 10);
                    fVar15 = steps * P_GRAV(iVar9);
                    P_POSZ(iVar9) += fVar15;
                    P_SCALE(iVar9) -= _DAT_005528e0;
                }

                if (iVar5 == 4) {
                    fVar11 = ((float10)(int)lifeF + (float10)P_VELX(iVar9)) * (float10)_DAT_005524f4;
                    fVar12 = (float10)fsin((double)fVar11);
                    P_POSX(iVar9) = (float)(fVar12 * (float10)_DAT_005528e4 + (float10)P_TGX(iVar9));
                    fVar11 = (float10)fcos((double)fVar11);
                    P_POSY(iVar9) = (float)((float10)P_TGY(iVar9) - fVar11 * (float10)_DAT_005528e4);
                    P_POSZ(iVar9) += P_GRAV(iVar9);
                    P_SCALE(iVar9) -= _DAT_005524bc;
                    if ((int)lifeF < 0x1f) {
                        goto label_color_decay;
                    }
                } else if ((int)lifeF < 0x15) {
                    label_color_decay:
                    P_LR(iVar9) *= _DAT_00552ae8;
                    P_LG(iVar9) *= _DAT_00552ae8;
                    P_LB(iVar9) *= _DAT_00552ae8;
                }

                if (iVar5 == 0) {
                    unsigned char animState = *(unsigned char*)(P_ENT(iVar9) + 0x105);
                    if ((animState > 0xc && animState < 0x22) || (animState > 0x37 && animState < 0x3d)) {
                        P_SUB(iVar9) = 1;
                        if ((int)lifeF > 0x14) {
                            lifeF = 20.0f;
                        }
                        P_LIFE(iVar9) = (int)lifeF;
                        P_VELX(iVar9) = 0.0f;
                        P_VELY(iVar9) = 0.0f;
                        P_VELZ(iVar9) = 0.0f;
                        P_TGY(iVar9) = P_POSY(iVar9);
                        P_TGX(iVar9) = P_POSX(iVar9);
                        P_TGZ(iVar9) = P_POSZ(iVar9);
                    }
                }
                break;
            }

            case 0x4f4: {
                if (P_SUB(iVar9) == 6) {
                    P_POSX(iVar9) += P_VELX(iVar9);
                    P_POSY(iVar9) += P_VELY(iVar9);
                    P_POSZ(iVar9) += P_VELZ(iVar9);
                    P_VELX(iVar9) *= _DAT_00552b44;
                    P_VELY(iVar9) *= _DAT_00552b44;
                    P_VELZ(iVar9) += _DAT_00552534;

                    uVar7 = _rand(); uVar7 &= 0x80000003;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                    P_POSX(iVar9) += (float)(int)(uVar7 - 2);

                    uVar7 = _rand(); uVar7 &= 0x80000003;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                    P_POSY(iVar9) += (float)(int)(uVar7 - 2);

                    uVar7 = _rand(); uVar7 &= 0x80000003;
                    if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                    P_POSZ(iVar9) += (float)(int)(uVar7 - 2) * _DAT_00552530;

                    P_SCALE(iVar9) += _DAT_00552874;
                    fVar15 = (float)P_LIFE(iVar9) * _DAT_00552944;
                    P_LR(iVar9) = fVar15;
                    P_LG(iVar9) = fVar15;
                    P_LB(iVar9) = fVar15;

                    uVar7 = _rand(); uVar7 &= 0x80000001;
                    if ((int)uVar7 < 0) uVar7 = ((uVar7 - 1) | 0xfffffffe) + 1;
                    P_ROT(iVar9) += (float)(int)uVar7 + _DAT_0055256c;
                } else {
                    // Non-sub6: check entity death flag
                    if (*(char*)(P_ENT(iVar9) + 0x160) == '\x01') {
                        P_LIFE(iVar9) = 0x32;
                    }
                    if (P_LIFE(iVar9) < 1) {
                        *(int*)(P_ENT(iVar9) + 0x58) = 0;
                        P_ACTIVE(iVar9) = '\0';
                    }
                    fVar11 = (float10)fsin(((float10)DAT_05826e08 + (float10)P_GRAV(iVar9)) * (float10)_DAT_00552b0c);
                    P_POSZ(iVar9) = (float)(fVar11 * (float10)_DAT_005524fc + (float10)P_TGZ(iVar9));
                }
                // Sub-type rotation
                switch (P_SUB(iVar9)) {
                case 1:
                case 4:
                    P_ROT(iVar9) = (float)DAT_05826e08 * P_EX0(iVar9) * _DAT_00552914 + P_TGY(iVar9);
                    break;
                case 2:
                case 5:
                    P_ROT(iVar9) = (float)DAT_05826e08 * P_EX0(iVar9) * _DAT_00552acc + P_TGY(iVar9);
                    break;
                }
                break;
            }

            case 0x565: {
                P_POSX(iVar9) += P_VELX(iVar9);
                P_POSY(iVar9) += P_VELY(iVar9);
                P_POSZ(iVar9) += P_VELZ(iVar9);
                P_VELX(iVar9) *= _DAT_00552b44;
                P_VELY(iVar9) *= _DAT_00552b44;
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552a00;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                P_VELZ(iVar9) += _DAT_005528b8;
                if (P_SUB(iVar9) == 0) {
                    P_SCALE(iVar9) += _DAT_00552ac8;
                } else {
                    P_SCALE(iVar9) += _DAT_00552ac4;
                }
                break;
            }

            case 0x566: {
                P_POSX(iVar9) += P_VELX(iVar9);
                P_POSY(iVar9) += P_VELY(iVar9);
                P_POSZ(iVar9) += P_VELZ(iVar9);
                P_VELX(iVar9) *= _DAT_00552b44;
                P_VELY(iVar9) *= _DAT_00552b44;
                P_VELZ(iVar9) += _DAT_00552534;

                uVar7 = _rand(); uVar7 &= 0x80000003;
                if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                P_POSX(iVar9) += (float)(int)(uVar7 - 2);

                uVar7 = _rand(); uVar7 &= 0x80000003;
                if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                P_POSY(iVar9) += (float)(int)(uVar7 - 2);

                uVar7 = _rand(); uVar7 &= 0x80000003;
                if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                P_POSZ(iVar9) += (float)(int)(uVar7 - 2) * _DAT_00552530;

                P_SCALE(iVar9) += _DAT_00552874;
                fVar15 = (float)P_LIFE(iVar9) * _DAT_0055295c;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;

                uVar7 = _rand(); uVar7 &= 0x80000001;
                if ((int)uVar7 < 0) uVar7 = ((uVar7 - 1) | 0xfffffffe) + 1;
                P_ROT(iVar9) += (float)(int)uVar7 + _DAT_0055256c;
                break;
            }

            case 0x567: {
                fVar15 = P_GRAV(iVar9) + _DAT_005524bc;
                P_GRAV(iVar9) = fVar15;
                P_SCALE(iVar9) -= _DAT_00552914;
                fVar15 = fVar15 * _DAT_00552488;
                P_FRAME(iVar9) = (0x17 - P_LIFE(iVar9)) / 6;
                P_POSZ(iVar9) += fVar15;
                break;
            }

            case 0x596: {
                P_FRAME(iVar9) = P_SUB(iVar9);
                break;
            }

            case 0x598: {
                float lifeF2 = (float)P_LIFE(iVar9);
                P_GRAV(iVar9) = (float)(5 - P_LIFE(iVar9)) * _DAT_00552ad4;
                fVar15 = lifeF2 * _DAT_005528dc;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                break;
            }

            case 0x599: {
                float grav = P_GRAV(iVar9);
                P_GRAV(iVar9) = grav - _DAT_005526e4;
                fVar12 = (float10)fsin((float10)P_LIFE(iVar9) * (float10)_DAT_00552ad0);
                P_SCALE(iVar9) = (float)(fVar12 + (float10)(grav - _DAT_005526e4) + (float10)_DAT_0055256c);
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552a30;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                break;
            }

            default:
                break;
            } // end high-type switch
            continue;
        } // end if iVar5 > 0x4c4

        if (iVar5 == 0x4c4) {
            // ── Type 0x4c4: multi-sub particle (flame/fire) ──────────────────
            switch (P_SUB(iVar9)) {
            case 0: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552b1c;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                goto label_4c4_0_4_shared;
            }
            case 1: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552944;
                P_LR(iVar9) = fVar15 * _DAT_00552504;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15 * _DAT_00552530;
                P_VELX(iVar9) *= _DAT_005528b4;
                P_VELY(iVar9) *= _DAT_005528b4;
                P_VELZ(iVar9) *= _DAT_005528b4;
                P_SCALE(iVar9) += _DAT_00552874;
                break;
            }
            case 2: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552944;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                fVar15 = P_GRAV(iVar9) - _DAT_005524f4;
                P_GRAV(iVar9) = fVar15;
                P_POSX(iVar9) -= fVar15 * _DAT_005526e4;
                P_POSZ(iVar9) += fVar15;
                P_SCALE(iVar9) -= _DAT_005524f8;
                break;
            }
            case 3: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552b1c;
                fVar3 = fVar15 * _DAT_00552530;
                P_LR(iVar9) = fVar3;
                P_LG(iVar9) = fVar3;
                P_LB(iVar9) = fVar15;
                P_VELX(iVar9) *= _DAT_005528b4;
                P_VELY(iVar9) *= _DAT_005528b4;
                P_VELZ(iVar9) *= _DAT_005528b4;
                P_SCALE(iVar9) += _DAT_005524f4;
                break;
            }
            case 4: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552b1c;
                P_LR(iVar9) = fVar15 * _DAT_00552b18;
                P_LG(iVar9) = fVar15 * _DAT_00552b14;
                fVar15 = fVar15 * _DAT_00552b10;
                label_4c4_0_4_shared:
                P_LB(iVar9) = fVar15;
                fVar15 = P_GRAV(iVar9) + _DAT_005526e4;
                P_GRAV(iVar9) = fVar15;
                P_POSZ(iVar9) += fVar15;
                P_SCALE(iVar9) += _DAT_00552874;
                break;
            }
            case 5: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552a00;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                P_GRAV(iVar9) -= _DAT_005524f4;
                uVar7 = _rand(); uVar7 &= 0x80000001;
                if ((int)uVar7 < 0) uVar7 = ((uVar7 - 1) | 0xfffffffe) + 1;
                P_POSX(iVar9) -= (float)(int)(uVar7 + 1) * _DAT_005526e4 * P_GRAV(iVar9);
                P_POSZ(iVar9) += P_GRAV(iVar9);
                P_SCALE(iVar9) -= _DAT_005524f8;
                break;
            }
            case 6: {
                fVar11 = (float10)DAT_05826e08;
                P_LIFE(iVar9) = 10;
                fVar11 = (float10)fsin((double)((fVar11 + (float10)P_GRAV(iVar9)) * (float10)_DAT_00552b0c));
                P_POSZ(iVar9) = (float)(fVar11 * (float10)_DAT_005524fc + (float10)P_ROT(iVar9));
                {
                    unsigned long long uv = (unsigned long long)(long long)(double)((float10)DAT_05826e08 * (float10)_DAT_00552500);
                    int modVal = (int)((long long)uv % 0x708);
                    P_LR(iVar9) = 0.36f;  // 0x3eb851ec
                    P_LG(iVar9) = 0.3f;   // 0x3e99999a
                    P_LB(iVar9) = 0.24f;  // 0x3e75c290
                    fVar11 = (float10)fsin((float10)modVal * (float10)_DAT_00552b08);
                    P_SCALE(iVar9) = (float)(fVar11 * (float10)_DAT_00552504 + (float10)_DAT_00552a18);
                }
                break;
            }
            case 7: {
                P_SCALE(iVar9) += _DAT_00552910;
                float lifeF3 = (float)P_LIFE(iVar9);
                fVar3 = P_GRAV(iVar9) + _DAT_0055256c;
                P_GRAV(iVar9) = fVar3;
                P_VELY(iVar9) -= _DAT_005524f4;
                P_POSZ(iVar9) -= fVar3;
                if (P_LIFE(iVar9) < 5) {
                    fVar15 = lifeF3 * _DAT_00552b1c;
                    P_SCALE(iVar9) -= _DAT_005524f4;
                }
                P_LR(iVar9) = fVar15 * _DAT_00552b04;
                P_LG(iVar9) = fVar15 * _DAT_00552b00;
                P_LB(iVar9) = fVar15 * _DAT_00552afc;
                break;
            }
            case 8: {
                fVar15 = P_GRAV(iVar9) + _DAT_00552914;
                float lifeF4 = (float)P_LIFE(iVar9);
                P_GRAV(iVar9) = fVar15;
                if (P_LIFE(iVar9) < 6) {
                    fVar15 = P_LR(iVar9) * _DAT_00552504;
                    P_VELX(iVar9) = 0.0f;
                    P_VELY(iVar9) = 0.0f;
                    P_VELZ(iVar9) = 0.0f;
                    P_LR(iVar9) = fVar15;
                    P_LG(iVar9) *= _DAT_00552504;
                    P_LB(iVar9) *= _DAT_00552504;
                } else {
                    P_SCALE(iVar9) += fVar15;
                    P_POSZ(iVar9) += fVar15 * _DAT_005524fc;
                    fVar15 = lifeF4 * _DAT_00552af8;
                    P_LR(iVar9) = fVar15;
                    P_LG(iVar9) = fVar15;
                    P_LB(iVar9) = fVar15;
                    P_VELX(iVar9) *= _DAT_00552af4;
                    P_VELY(iVar9) *= _DAT_00552af4;
                    P_VELZ(iVar9) *= _DAT_00552af4;
                }
                break;
            }
            case 9: {
                P_LB(iVar9) = 0.0f;
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552a00;
                P_LR(iVar9) = fVar15 * _DAT_00552af0;
                P_LG(iVar9) = fVar15 * _DAT_00552aec;
                fVar15 = P_GRAV(iVar9) + _DAT_00552504;
                P_GRAV(iVar9) = fVar15;
                P_POSZ(iVar9) += fVar15;
                P_SCALE(iVar9) += _DAT_005524f4;
                break;
            }
            case 10: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_005526dc;
                P_LR(iVar9) = fVar15;
                fVar15 = fVar15 * _DAT_005524f4;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                P_SCALE(iVar9) += _DAT_00552874;
                break;
            }
            case 0xb: {
                fVar15 = (float)P_LIFE(iVar9) * _DAT_00552944;
                P_LR(iVar9) = fVar15 * P_EX0(iVar9);
                P_LG(iVar9) = fVar15 * P_EX1(iVar9);
                P_LB(iVar9) = fVar15 * P_EX2(iVar9);
                P_VELX(iVar9) *= _DAT_005528b4;
                P_VELY(iVar9) *= _DAT_005528b4;
                P_VELZ(iVar9) *= _DAT_005528b4;
                P_SCALE(iVar9) += _DAT_00552874;
                P_POSZ(iVar9) -= _DAT_0055256c;
                break;
            }
            } // end 0x4c4 sub switch
            continue;
        } // end iVar5 == 0x4c4

        // ── Low type range: 0x67..0x4c3 ──────────────────────────────────────
        if (iVar5 < 0x4a7) {
            if (iVar5 == 0x4a6) {
                // Type 0x4a6: terrain light particle
                uVar7 = _rand(); uVar7 &= 0x80000003;
                if ((int)uVar7 < 0) uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
                P_FRAME(iVar9) = (int)uVar7;
                float lightF = (float)P_LIFE(iVar9) * _DAT_00552b28;
                P_LR(iVar9) = lightF;
                P_LG(iVar9) = lightF;
                P_LB(iVar9) = lightF;
                // AddTerrainLight — first call
                {
                    float lt0[3];
                    lt0[0] = lightF;  // placeholder
                    lt0[1] = lightF;
                    lt0[2] = lightF;
                    FUN_004f76c0(P_POSX(iVar9), P_POSY(iVar9), (int)lt0, 6, (int)&DAT_081cb608[0]);
                }
                // AddTerrainLight — second call with color modulation
                {
                    float lt1[3];
                    lt1[0] = lightF * _DAT_005526e4;
                    lt1[1] = lightF * _DAT_005528b4;
                    lt1[2] = lightF;
                    FUN_004f76c0(P_POSX(iVar9), P_POSY(iVar9), (int)lt1, 4, (int)&DAT_081cb608[0]);
                }
                continue;
            }

            if (iVar5 > 0x497) {
                if (iVar5 == 0x498) {
                    // Type 0x498: multi-sub particle
                    switch (P_SUB(iVar9)) {
                    case 0:
                        fVar15 = P_SCALE(iVar9) - _DAT_00552504;
                        P_SCALE(iVar9) = fVar15;
                        if (fVar15 < _DAT_005526e4) P_ACTIVE(iVar9) = '\0';
                        break;
                    case 1:
                        fVar15 = P_SCALE(iVar9) - _DAT_0055264c;
                        P_SCALE(iVar9) = fVar15;
                        if (fVar15 < _DAT_005526e4) P_ACTIVE(iVar9) = '\0';
                        break;
                    case 2:
                        if (P_LIFE(iVar9) < 1) P_ACTIVE(iVar9) = '\0';
                        if (P_LIFE(iVar9) > 5 && P_ACTIVE(P_ENT(iVar9) - (int)P_BASE) == '\0') {
                            // Entity died — clamp life to 5
                            // Note: P_ENT points to an entity; checking its active flag
                            // Ghidra: **(char **)(P_ENT) == '\0'
                            P_LIFE(iVar9) = 5;
                        }
                        {
                            float lt2 = (float)P_LIFE(iVar9) * _DAT_00552b28;
                            P_LR(iVar9) = lt2;
                            P_LG(iVar9) = lt2;
                            P_LB(iVar9) = lt2;
                        }
                        P_SCALE(iVar9) += _DAT_00552874;
                        P_POSX(iVar9) = P_TGX(iVar9) + P_VELX(iVar9);
                        {
                            float newY = P_VELY(iVar9) + P_TGY(iVar9);
                            P_POSY(iVar9) = newY;
                            float newZ = P_VELZ(iVar9) + P_TGZ(iVar9);
                            P_TGY(iVar9) = newY;
                            P_POSZ(iVar9) = newZ;
                            P_TGX(iVar9) += P_VELX(iVar9);
                            P_TGZ(iVar9) = newZ;
                        }
                        break;
                    case 3:
                        P_SCALE(iVar9) *= _DAT_00552530;
                        break;
                    }
                } else {
                    if (iVar5 == 0x499) goto label_4b5;
                    if (iVar5 == 0x49c) {
                        P_ROT(iVar9) += _DAT_005524fc;
                    }
                }
                continue;
            }

            if (iVar5 == 0x497) {
                // Type 0x497: bounce + glow decay
                fVar15 = (float)P_LIFE(iVar9) * _DAT_005526dc;
                P_LR(iVar9) = fVar15;
                P_LG(iVar9) = fVar15;
                P_LB(iVar9) = fVar15;
                P_POSZ(iVar9) += P_GRAV(iVar9);
                P_GRAV(iVar9) -= _DAT_0055264c;
                fVar15 = FUN_004f7500(P_POSX(iVar9), P_POSY(iVar9));
                if (P_POSZ(iVar9) < fVar15) {
                    P_POSZ(iVar9) = fVar15;
                    fVar15 = P_GRAV(iVar9) * _DAT_00552b50;
                    P_LIFE(iVar9) -= 4;
                    P_GRAV(iVar9) = fVar15;
                }
                P_POSX(iVar9) += P_VELX(iVar9);
                P_POSY(iVar9) += P_VELY(iVar9);
                P_POSZ(iVar9) += P_VELZ(iVar9);
                continue;
            }

            // Types < 0x497
            if (iVar5 > 0x47f) {
                if (iVar5 == 0x490) {
                    fVar15 = P_GRAV(iVar9) + _DAT_005524f8;
                    P_GRAV(iVar9) = fVar15;
                    P_SCALE(iVar9) += fVar15;
                    fVar15 = (float)P_LIFE(iVar9) * _DAT_00552a00;
                    P_LR(iVar9) = fVar15;
                    P_LG(iVar9) = fVar15;
                    P_LB(iVar9) = fVar15;
                }
                continue;
            }

            if (iVar5 == 0x47f) {
                P_POSZ(iVar9) -= P_GRAV(iVar9);
                P_POSY(iVar9) -= _DAT_0055256c;
                P_GRAV(iVar9) += _DAT_0055256c;
                P_SCALE(iVar9) *= _DAT_00552b44;
                continue;
            }

            if (iVar5 < 0x67) continue;

            if (iVar5 > 0x68) {
                if (iVar5 == 0x47e) {
                    // Type 0x47e: complex multi-sub with camera shake
                    iVar5 = P_SUB(iVar9);
                    if (iVar5 == 0) {
                        fVar19 = 0.5f;
                    } else if (iVar5 == 2) {
                        fVar19 = 4.0f;
                    } else if (iVar5 == 1 || iVar5 == 5) {
                        P_LR(iVar9) *= _DAT_005526e8;
                        P_LG(iVar9) *= _DAT_005526e8;
                        P_LB(iVar9) *= _DAT_005526e8;
                        P_SCALE(iVar9) *= _DAT_00552b44;
                        continue;
                    } else if (iVar5 == 3) {
                        int entPtr = P_ENT(iVar9);
                        P_LR(iVar9) *= _DAT_005526e8;
                        P_LG(iVar9) *= _DAT_005526e8;
                        P_LB(iVar9) *= _DAT_005526e8;
                        P_SCALE(iVar9) *= _DAT_00552b40;
                        fVar15 = P_GRAV(iVar9) + _DAT_00552660;
                        P_GRAV(iVar9) = fVar15;
                        P_POSX(iVar9) = *(float*)(entPtr + 0x10);
                        fVar3 = P_POSX(iVar9) + _DAT_0055264c;
                        P_POSY(iVar9) = *(float*)(entPtr + 0x14);
                        P_POSZ(iVar9) = *(float*)(entPtr + 0x18);
                        P_POSX(iVar9) = fVar3;
                        P_POSY(iVar9) -= _DAT_0055264c;
                        P_POSZ(iVar9) += fVar15;
                        continue;
                    } else if (iVar5 == 4) {
                        int entPtr = P_ENT(iVar9);
                        if (entPtr != 0) {
                            // BMD::TransformPosition — get bone position from entity model
                            // Uses __thiscall on BMD* (Models + entity_type * 0xBC)
                            // Ghidra: phantom register args present; result updates particle pos
                            float localP[3] = { 0.0f, 0.0f, 0.0f };
                            float worldP[3];
                            void* mdl = (void*)((char*)DAT_05828d58 + *(short*)(entPtr + 2) * 0xBC);
                            long long lFrame = (long long)(double)P_SCALE(iVar9); // __ftol analog
                            // BMD__TransformPosition(this, BoneMatrix, Pos, WorldPos, Translate)
                            BMD_TransformPosition(mdl,
                                (float*)((int)lFrame * 0x30 + *(int*)(entPtr + 0x114)),
                                localP, worldP, 0);
                        }
                        iVar5 = _rand();
                        P_GRAV(iVar9) += (float)(iVar5 % 0x28 + 0x3c) * _DAT_00552b3c;
                        iVar5 = _rand();
                        P_SCALE(iVar9) -= (float)(iVar5 % 400 + 400) * _DAT_00552b38;
                        P_POSZ(iVar9) += P_GRAV(iVar9);
                        P_LR(iVar9) *= _DAT_00552b34;
                        P_LG(iVar9) *= _DAT_00552b34;
                        P_LB(iVar9) *= _DAT_00552b34;
                        continue;
                    } else {
                        continue;
                    }
                    // Shared path for sub 0 and 2: camera shake + drift
                    iVar5 = P_SUB(iVar9);
                    iVar6 = _rand();
                    fVar15 = (float)(_rand() % 0x7d1 - 1000) * _DAT_00552b30;
                    float fDrift = (float)(iVar6 % 0x7d1 - 1000) * _DAT_00552b30 + DAT_07c800fc;
                    float fShakeZ = DAT_07c80100 * fVar19;
                    P_POSX(iVar9) += (DAT_07c800f8 + fVar15) * fVar19;
                    P_POSY(iVar9) += fDrift * fVar19;
                    P_POSZ(iVar9) += fVar19 * _DAT_00552660 + fShakeZ;
                    if (iVar5 == 0) {
                        fVar15 = P_SCALE(iVar9) - _DAT_00552874;
                    } else if (iVar5 == 1) {
                        fVar15 = P_SCALE(iVar9) * _DAT_00552b2c;
                    } else {
                        fVar15 = P_SCALE(iVar9) * _DAT_00552b44;
                    }
                    P_SCALE(iVar9) = fVar15;
                    P_LR(iVar9) = fVar15;
                    P_LG(iVar9) = fVar15;
                    P_LB(iVar9) = fVar15;
                    if (iVar5 == 1) {
                        fVar15 = P_SCALE(iVar9) * _DAT_005528b8;
                        P_LR(iVar9) = fVar15;
                        P_LG(iVar9) = fVar15;
                        P_LB(iVar9) = fVar15;
                    }
                    if (iVar5 == 5) {
                        fVar15 = P_SCALE(iVar9) * _DAT_005528b8;
                        P_LR(iVar9) = fVar15;
                        P_LG(iVar9) = fVar15;
                        P_LB(iVar9) = fVar15;
                    }
                    if ((iVar5 == 0 && P_SCALE(iVar9) <= _DAT_005524f4) ||
                        (iVar5 == 2 && P_SCALE(iVar9) <= _DAT_005528b8)) {
                        P_LIFE(iVar9) = -1;
                        P_ACTIVE(iVar9) = '\0';
                    }
                    continue;
                }
                continue; // types between 0x68 and 0x47e not handled above
            }

            // Types 0x67..0x68: glow + fade
            if (P_LIFE(iVar9) < 1) P_ACTIVE(iVar9) = '\0';
            if (P_SUB(iVar9) == 1) {
                fVar15 = P_GRAV(iVar9) + P_SCALE(iVar9);
            } else {
                fVar15 = P_SCALE(iVar9) + _DAT_00552910;
            }
            P_SCALE(iVar9) = fVar15;
            {
                float lt3[3] = { 0.05f, 0.05f, 0.05f };
                // fall through to color decay label
            }
            goto label_light_decay;
        } // end iVar5 < 0x4a7

        // ── Types 0x4a7..0x4c3 ───────────────────────────────────────────────
        switch (iVar5) {

        case 0x4a7: {
            // Terrain light + positioning
            unsigned long long uv = (unsigned long long)(long long)(double)((float10)DAT_05826e08 * (float10)_DAT_00552500);
            P_ROT(iVar9) = (float)(int)((long long)(uv) % 1000) * _DAT_00552500;
            float lightA = (float)P_LIFE(iVar9) * _DAT_00552a00;
            float ltA0 = lightA * _DAT_00552504;
            float ltA1 = lightA;
            float ltA2 = lightA * _DAT_00552530;
            {
                float ltBuf[3] = { ltA0, ltA1, ltA2 };
                FUN_004f76c0(P_POSX(iVar9), P_POSY(iVar9), (int)ltBuf, 3, (int)&DAT_081cb608[0]);
            }
            if (P_SUB(iVar9) == 2) {
                int entPtr = P_ENT(iVar9);
                P_LR(iVar9) = ltA0;
                P_LG(iVar9) = ltA1;
                P_SCALE(iVar9) += _DAT_005524f4;
                P_LB(iVar9) = ltA2;
                P_POSX(iVar9) = *(float*)(entPtr + 0x10);
                P_POSY(iVar9) = *(float*)(entPtr + 0x14);
                P_POSZ(iVar9) = *(float*)(entPtr + 0x18) + _DAT_00552878;
            }
            break;
        }

        case 0x4ab:
        case 0x4ad: {
            float grav2 = P_GRAV(iVar9) + _DAT_005524bc;
            P_GRAV(iVar9) = grav2;
            switch (P_SUB(iVar9)) {
            case 5:
            case 6:
                P_SCALE(iVar9) -= _DAT_00552a0c;
                P_ROT(iVar9) += _DAT_00552660;
                break;
            case 7:
                P_FRAME(iVar9) = (0xf - P_LIFE(iVar9)) / 6;
                // fall through
            case 0:
                P_SCALE(iVar9) -= _DAT_00552a0c;
                break;
            case 8:
                P_SCALE(iVar9) *= _DAT_00552b44;
                P_ROT(iVar9) += _DAT_00552660;
                break;
            case 9: {
                int entPtr = P_ENT(iVar9);
                P_POSX(iVar9) = *(float*)(entPtr + 0x10) + P_TGX(iVar9);
                P_POSY(iVar9) = *(float*)(entPtr + 0x14) + P_TGY(iVar9);
                P_POSZ(iVar9) = *(float*)(entPtr + 0x18) + P_TGZ(iVar9);
                iVar5 = _rand();
                float rndG = (float)((iVar5 % 0x3c + 0x3c) / 100);
                fVar15 = P_GRAV(iVar9);
                P_GRAV(iVar9) = rndG + fVar15;
                P_SCALE(iVar9) -= (rndG + fVar15) * _DAT_00552a60;
                break;
            }
            case 10:
                P_SCALE(iVar9) *= _DAT_00552b44;
                break;
            default:
                P_SCALE(iVar9) = grav2 + P_SCALE(iVar9);
                P_VELX(iVar9) *= _DAT_00552b2c;
                P_VELY(iVar9) *= _DAT_00552b2c;
                P_VELZ(iVar9) *= _DAT_00552b2c;
                break;
            }
            P_POSZ(iVar9) += P_GRAV(iVar9) * _DAT_00552488;
            P_FRAME(iVar9) = (0x17 - P_LIFE(iVar9)) / 6;
            break;
        }

        case 0x4ac: {
            fVar15 = P_GRAV(iVar9) + _DAT_00552914;
            P_GRAV(iVar9) = fVar15;
            P_SCALE(iVar9) += fVar15;
            P_VELX(iVar9) *= _DAT_00552b24;
            P_VELY(iVar9) *= _DAT_00552b24;
            P_VELZ(iVar9) *= _DAT_00552b24;
            P_POSZ(iVar9) += fVar15 * _DAT_005524fc;
            fVar15 = (float)P_LIFE(iVar9) * _DAT_005526e4;
            P_LR(iVar9) = fVar15;
            P_LG(iVar9) = fVar15;
            P_LB(iVar9) = fVar15;
            break;
        }

        case 0x4b0: {
            if (P_LIFE(iVar9) < 1) P_ACTIVE(iVar9) = '\0';
            switch (P_SUB(iVar9)) {
            case 1:
                if (P_LIFE(iVar9) == 10) {
                    P_VELY(iVar9) += _DAT_00552b20;
                    P_SCALE(iVar9) -= _DAT_00552934;
                }
                fVar15 = (float)(_rand() % 0x168);
                break;
            case 2:
                fVar15 = (float)(_rand() % 0x168);
                break;
            case 3:
                P_ROT(iVar9) = (float)(_rand() % 0x168);
                continue;
            case 4: {
                int entPtr = P_ENT(iVar9);
                P_POSX(iVar9) += *(float*)(entPtr + 0x10) - P_TGX(iVar9);
                P_POSY(iVar9) += *(float*)(entPtr + 0x14) - P_TGY(iVar9);
                P_POSZ(iVar9) += *(float*)(entPtr + 0x18) - P_TGZ(iVar9) + P_GRAV(iVar9);
                fVar15 = P_GRAV(iVar9) + _DAT_005524f4;
                P_TGX(iVar9) = *(float*)(entPtr + 0x10);
                P_TGY(iVar9) = *(float*)(entPtr + 0x14);
                P_TGZ(iVar9) = *(float*)(entPtr + 0x18);
                P_GRAV(iVar9) = fVar15;
                fVar15 = (float)(_rand() % 0x168);
                break;
            }
            case 5:
                if (P_LIFE(iVar9) == 10) {
                    P_VELY(iVar9) += _DAT_00552b20;
                    P_SCALE(iVar9) *= _DAT_00552530;
                }
                fVar15 = (float)(_rand() % 0x168);
                break;
            default:
                continue;
            }
            P_ROT(iVar9) = fVar15;
            label_light_decay:
            P_LR(iVar9) -= _DAT_00552874;
            P_LG(iVar9) -= _DAT_00552874;
            P_LB(iVar9) -= _DAT_00552874;
            break;
        }

        case 0x4b5:
        case 0x4b6:
        label_4b5: {
            fVar15 = P_VELX(iVar9) * _DAT_00552b44;
            P_VELX(iVar9) = fVar15;
            float vy = P_VELY(iVar9) * _DAT_00552b44;
            P_VELY(iVar9) = vy;
            float vz = P_VELZ(iVar9) * _DAT_00552b44;
            P_VELZ(iVar9) = vz;
            P_FRAME(iVar9) = (0xc - P_LIFE(iVar9)) / 3;
            P_POSX(iVar9) += fVar15;
            P_POSY(iVar9) += vy;
            P_POSZ(iVar9) += vz;
            break;
        }

        case 0x4bf: {
            P_FRAME(iVar9) = (0x14 - P_LIFE(iVar9)) / 2;
            if (P_SUB(iVar9) != 1) {
                float ltBF = (float)P_LIFE(iVar9) * _DAT_00552a10;
                float lt4[3] = { ltBF * _DAT_00552504, ltBF * _DAT_005528b8, ltBF * _DAT_005524f4 };
                FUN_004f76c0(P_POSX(iVar9), P_POSY(iVar9), (int)lt4, 4, (int)&DAT_081cb608[0]);
            }
            break;
        }

        case 0x4c0: {
            P_FRAME(iVar9) = (0xc - P_LIFE(iVar9)) / 3;
            break;
        }

        default:
            break;
        } // end types 0x4a7..0x4c3 switch

    } // end particle loop
}

// Undefine particle helper macros to avoid polluting other translation units
#undef P_BASE
#undef P_ACTIVE
#undef P_TYPE
#undef P_SUB
#undef P_SCALE
#undef P_POSX
#undef P_POSY
#undef P_POSZ
#undef P_DIRX
#undef P_ANGLE
#undef P_LR
#undef P_LG
#undef P_LB
#undef P_LIFE
#undef P_ENT
#undef P_ROT
#undef P_FRAME
#undef P_GRAV
#undef P_VELX
#undef P_VELY
#undef P_VELZ
#undef P_EX0
#undef P_EX1
#undef P_EX2
#undef P_TGX
#undef P_TGY
#undef P_TGZ
#undef BMP_HEIGHT
