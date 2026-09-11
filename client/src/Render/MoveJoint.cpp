// MoveJoint.cpp
// FUN_00470030 @ 0x00470030  [Kayito: MoveJoint]
//
// Per-frame movement/animation tick for "joint" effect slots.
// Joint slots are chained lightning/beam/special-link effects.
// Called by MoveJoints (FUN_004736e0) once per active joint slot per frame.
//
// param_1 = undefined1*  — pointer to joint slot (stride ~0x9d4, base near DAT_07abf5d0)
// param_2 = uint         — frame counter / slot index used for trig oscillation seeds
//
// Joint slot layout (byte offsets):
//   [+0x00] char    — active flag (0 = dead)
//   [+0x04] int     — joint type id (switch key)
//   [+0x08] int     — sub-mode
//   [+0x0c] float   — alpha/scale
//   [+0x10] float   — world pos X (current)
//   [+0x14] float   — world pos Y
//   [+0x18] float   — world pos Z
//   [+0x24] float   — facing angle copy (for anim-state change detection)
//   [+0x28] float[3]— euler angles for matrix
//   [+0x34] float   — color R
//   [+0x38] float   — color G
//   [+0x3c] float   — color B
//   [+0x40] int/ptr — linked entity pointer (or 0)
//   [+0x44] float   — anchor pos X
//   [+0x48] float   — anchor pos Y
//   [+0x4c] float   — anchor pos Z
//   [+0x50] int     — segment count
//   [+0x54] int     — segment max
//   [+0x5c] float[] — segment positions (stride 0x30 per segment, 4 sub-points x 3 floats)
//   [+0x78] BYTE    — entity flags (bit 3 checked for type 0x10a mode 4)
//   [+0x9b8] int    — lifetime counter (counts down to 0)
//   [+0x9c0] float  — speed / approach-distance accumulator
//   [+0x9c4] float  — oscillation value (written each frame)
//   [+0x9c8] float  — oscillation anchor (used by type 0x10a mode 3)
//   [+0x9cc] float  — vertical bob accumulator (type 0x498)
//   [+0x9d2] char   — flags byte (0x01 = write-back to linked entity)
//
// Returns: char* — cast of lifetime counter after decrement (or dead ptr)

#include "stdafx.h"

// Helper: store a float bit-pattern in an int-sized local (Ghidra undefined4* = float storage)
// Used for Ghidra's pattern:  local_eX = (undefined4 *)(float)val
static inline int float_as_int(float f) {
    int i;
    memcpy(&i, &f, 4);
    return i;
}
static inline float int_as_float(int i) {
    float f;
    memcpy(&f, &i, 4);
    return f;
}


// -----------------------------------------------------------------------------
// 2026-09-03 - COLA GENERICA de MoveJoint (IDA 0x00470030 L1922-2217)
//
// En el binario, todo joint cuyo TIPO no matchea ningun bloque del dispatch cae
// en esta cola.  Es la que CONSTRUYE la estela: recorre `segMax` pasos y en cada
// uno mueve el joint con `MoveHumming` hacia su objetivo y empuja un segmento
// con `sub_46FE90` -- o sea el rayo entero se dibuja en UN tick.
//
// El port no la tenia, y por eso faltaban tres cosas que parecian no
// relacionadas: el rayo del Lightning (1254 sub 0), los rayos del Twister
// (1253 sub 0, ocho joints) y los rayos de fondo de Icarus (1254 sub 6, que
// ademas es el unico subtipo que el epilogo LABEL_487 excluye del tick de
// segmentos justamente porque los construye aca).
//
// Offsets: +8 SubType - +12 Scale - +16 Position - +40 Angle - +52 Light
//          +64 Owner - +68 TargetPosition - +84 segMax - +2488 lifetime
//          +2496 Velocity - +2500/2504/2508 jitter de angulo
static void MoveJoint_GenericTail(unsigned char *o)
{
    float *const pos = (float *)(o + 16);
    float *const tgt = (float *)(o + 68);
    float Position[3], Angle[3], Light[3], in1[3], angles[3];
    float v304[12], v305[12];

    Position[0] = Position[1] = Position[2] = 0.0f;

    const int sub0 = *(int *)(o + 8);

    // -- IDA L1922-1996: ajuste previo por subtipo ----------------------------
    if ((sub0 != 6 || *(int *)(o + 2488) <= 4) && sub0 != 8) {
        bool toLabel126 = false;
        if (sub0 == 4 || sub0 == 5) {
            const float tx = tgt[0], ty = tgt[1], tz = tgt[2];
            pos[0] = tx; pos[1] = ty; pos[2] = tz;
            Position[0] = tx; Position[1] = ty; Position[2] = tz + 30.0f;
            const float rot = (float)(rand() % 360);
            const float sc  = (float)(rand() % 8 + 8) * 0.2f;
            FUN_004795c0(1231, Position, sc, (float *)(o + 52), 0, rot, 0);
            const int life = *(int *)(o + 2488);
            if (life > 10) {
                Light[0] = 0.5f; Light[1] = 0.5f; Light[2] = 0.5f;
                if ((life & 1) == 0) {
                    Angle[0] = (float)(rand() % 60 - 30) + *(float *)(o + 40);
                    Angle[1] = 0.0f                      + *(float *)(o + 44);
                    Angle[2] = (float)(rand() % 30 + 90) + *(float *)(o + 48);
                    Joint_Create(1254, Position, Position, (float *)(o + 40), 3, 0, 10.0f, 10, 0xA);
                    Joint_Create(1254, Position, Position, Angle,              8, 0, 10.0f, -1, 0);
                }
                in1[0] = 0.0f; in1[1] = 120.0f; in1[2] = 0.0f;
                angles[0] = (float)(rand() % 360);
                angles[1] = 0.0f;
                angles[2] = (float)(rand() % 360);
                Matrix_BuildFromEuler(angles, v304);
                Vector_Rotate(in1, v304, Position);
                Position[0] += pos[0]; Position[1] += pos[1]; Position[2] += pos[2];
                Joint_Create(1254, Position, pos, angles, 7, 0, 10.0f, 10, 0);
            }
        } else if (sub0 == 6 || sub0 == 7) {
            toLabel126 = true;
        } else if (sub0 == 9) {
            Position[0] = (float)(rand() % 200) + tgt[0] - 100.0f;
            Position[1] = (float)(rand() % 200) + tgt[1] - 100.0f;
            Position[2] = tgt[2];
            toLabel126 = true;
        }
        if (toLabel126) {   // IDA LABEL_126
            pos[0] = *(float *)(o + 28);
            pos[1] = *(float *)(o + 32);
            pos[2] = *(float *)(o + 36);
        }
    }

    // -- IDA L1996-2210: el bucle que construye la estela ---------------------
    const int segMax = *(int *)(o + 84);
    if (segMax <= 0) return;

    float v295 = 0.0f;          // distancia devuelta por MoveHumming
    int   step = 0;
    for (;;) {
        const int sub = *(int *)(o + 8);

        switch (sub) {
        case 0: case 1: case 2: {
            const int owner = *(int *)(o + 64);
            if (owner) {
                tgt[0] = *(float *)(owner + 16);
                tgt[1] = *(float *)(owner + 20);
                tgt[2] = *(float *)(owner + 24) + 80.0f;
            }
            v295 = FUN_0043e4a0(pos, (float *)(o + 40), tgt, 25.0f);   // LABEL_131
            break;
        }
        case 3:
            v295 = FUN_0043e4a0(pos, (float *)(o + 40), tgt, 50.0f);
            break;
        case 4: case 5: {
            Position[0] = tgt[0]; Position[1] = tgt[1]; Position[2] = tgt[2] - 300.0f;
            v295 = FUN_0043e4a0(pos, (float *)(o + 40), Position, -10.0f);
            break;
        }
        case 6: {
            const int owner = *(int *)(o + 64);
            if (owner) {
                tgt[0] = *(float *)(owner + 16);
                tgt[1] = *(float *)(owner + 20);
                tgt[2] = *(float *)(owner + 24);
            }
            tgt[0] += (float)(rand() % 200) + 2050.0f;
            tgt[1] += (float)(rand() % 200) + 2050.0f;
            tgt[2] -= 10000.0f;
            v295 = FUN_0043e4a0(pos, (float *)(o + 40), tgt, (float)(rand() % 100 + 50));
            break;
        }
        case 7:
            v295 = FUN_0043e4a0(pos, (float *)(o + 40), tgt, (float)(rand() % 100 + 50));
            break;
        case 9:
            v295 = FUN_0043e4a0(pos, (float *)(o + 40), Position, (float)(rand() % 80 + 60));
            break;
        case 0xC: {
            // IDA entra directo a LABEL_133: no recalcula la distancia.
            const int owner = *(int *)(o + 64);
            if (owner) {
                const short mdl = *(short *)(owner + 2);
                float out[3] = { 0.0f, -100.0f, 0.0f };
                BMD_TransformPosition((void *)((uintptr_t)DAT_05828d58 + 188 * mdl),
                                      (float *)&DAT_07abf474, out, pos, '\x01');
            }
            break;
        }
        default:
            v295 = FUN_0043e4a0(pos, (float *)(o + 40), tgt, 25.0f);    // LABEL_131
            break;
        }

        // -- LABEL_133: jitter del angulo por subtipo -------------------------
        const int sub2 = *(int *)(o + 8);
        bool toLabel151 = false;
        float v92 = 0.0f;
        if (sub2 == 1) {
            *(float *)(o + 2500) = (float)(rand() % 256 - 128);
            v92 = (float)(rand() % 256 - 128);
        } else if (sub2 == 4) {
            *(float *)(o + 2500) = (float)(rand() % 64 - 32);
            toLabel151 = true;
        } else if (sub2 == 6 || sub2 == 7) {
            *(float *)(o + 2500) = (float)(rand() % 100 + 20);
            v92 = (float)(rand() % 100 + 20);
        } else if (sub2 == 11) {
            const float sc = *(float *)(o + 12);
            *(float *)(o + 2500) = (float)((double)(rand() % 1024 - 512) * 0.69999999) / sc;
            v92 = (float)((double)(rand() % 1024 - 512) * 0.69999999) / sc;
        } else {
            const float sc = *(float *)(o + 12);
            *(float *)(o + 2500) = (float)(rand() % 1024 - 512) / sc;
            v92 = (float)(rand() % 1024 - 512) / sc;
        }
        if (!toLabel151) *(float *)(o + 2508) = v92;

        // -- LABEL_151: empujar un segmento -----------------------------------
        Angle[0] = *(float *)(o + 40) + *(float *)(o + 2500);
        Angle[1] = *(float *)(o + 44) + *(float *)(o + 2504);
        Angle[2] = *(float *)(o + 48) + *(float *)(o + 2508);
        Matrix_BuildFromEuler(Angle, v305);
        FUN_0046fe90((int)(uintptr_t)o, v305);

        const int sub3 = *(int *)(o + 8);
        if (sub3 == 3) {
            if (v295 > 150.0f) *(int *)(o + 2488) = 0;
        } else if (*(float *)(o + 2496) * 1.5f > v295 &&
                   sub3 != 6 && sub3 != 9 && sub3 != 7) {
            if (*(int *)(o + 12) == 1112014848) {           // Scale == 50.0f
                Particle_Spawn(1180, pos, (float *)(o + 40), (float *)(o + 52), 0, 1.0f, 0);
                if ((rand() % 8) == 0) {
                    angles[0] = (float)(rand() % 64 - 32) + tgt[0];
                    angles[1] = (float)(rand() % 64 - 32) + tgt[1];
                    angles[2] = (float)(rand() % 64 - 32) + tgt[2];
                    Particle_Spawn(1220, pos, (float *)(o + 40), (float *)(o + 52), 0, 1.0f, 0);
                }
                if (*(int *)(o + 8) == 0 && ((long long)DAT_05826e08) % 1000 < 500) {
                    if ((rand() % 16) == 0) {
                        angles[0] = (float)(rand() % 100 - 50) + tgt[0];
                        angles[1] = (float)(rand() % 100 - 50) + tgt[1];
                        angles[2] = (float)(rand() % 120 - 60) + tgt[2];
                        Joint_Create(1254, angles, tgt, (float *)(o + 40), 1,
                                     *(int *)(o + 64), (float)(rand() % 8) + 6.0f, -1, 0);
                    }
                }
            }
            break;                                          // IDA: goto LABEL_182
        }

        if (*(float *)(o + 12) >= 50.0f && sub3 != 4 && sub3 != 7 &&
            (sub3 == 0 || sub3 == 11 || sub3 == 2)) {
            const float v99 = (float)(rand() % 4 + 4) * 0.079999998f;
            float v100;
            if (sub3 == 0 || sub3 == 11) {
                Light[0] = v99 * 0.1f;
                Light[1] = Light[0];
                v100 = v99 * 0.5f;
            } else {
                Light[0] = v99 * 0.40000001f;
                v100 = v99 * 0.1f;
                Light[1] = v100;
            }
            Light[2] = v100;
            AddTerrainLight(pos[0], pos[1], Light, 2, PrimaryTerrainLight[0]);
        }

        Position[0] = 0.0f;
        Position[1] = -*(float *)(o + 2496);
        Position[2] = 0.0f;
        Vector_Rotate(Position, v305, in1);
        pos[0] += in1[0];
        pos[1] += in1[1];
        pos[2] += in1[2];
        if (++step >= segMax) break;
    }

    // -- LABEL_182 ------------------------------------------------------------
    if (*(int *)(o + 8) == 7) {
        pos[0] = tgt[0]; pos[1] = tgt[1]; pos[2] = tgt[2];
    }
}

char * __cdecl FUN_00470030(undefined1 *param_1, uint param_2)
{
    float   fVar2;
    float   fVar3;
    void   *pvVar5;
    uint    uVar7;
    char   *pcVar11;
    char   *pcVar12;
    int     iVar13;
    float  *pfVar14;
    float  *pfVar15;
    int     iVar16;
    bool    bVar21;
    double  fVar22;
    double  fVar23;
    double  fVar24;
    // Ghidra "undefined4*" locals used as float storage via bit-casts
    float   local_e8_f;   // maps to local_e8
    float   local_e4_f;   // maps to local_e4
    float   local_e0_f;   // maps to local_e0
    float   local_dc_f;   // maps to local_dc (delta X, used as float seed)
    // 2026-08-10 FIX (haces oscuros): estos "locales" que produjo Ghidra son en
    // realidad UN bloque contiguo del frame original (ebp-0xD8 .. ebp), y el
    // código de abajo depende de esa contigüidad — MSVC no la garantiza:
    //   L99  Vector_Rotate(&local_b4, local_30, local_d8 + 6)  → escribe el vec3
    //        de salida en local_d8[6],[7],[8]; L102 lee out[2] como `local_b8`,
    //        o sea `local_d8[8] == local_b8` (¡y local_d8 estaba dimensionado a
    //        8, así que era además una escritura fuera de rango en el stack!).
    //   L138 Vector_Rotate(local_d8 + 6, local_a8 + 6, &local_b4) → L140/L141
    //        leen out[1] y out[2] como `local_b0` y `local_ac`, o sea
    //        `(&local_b4)[1] == local_b0` y `(&local_b4)[2] == local_ac`.
    // Con locales sueltos, la entrada de Vector_Rotate traía basura en y/z y la
    // salida se perdía → la Position del joint (+0x14/+0x18) quedaba con valores
    // de ~1e9 y el render dibujaba quads que cruzaban toda la pantalla.
    // Mapeo por offset de frame: -0xD8=[0] … -0xB8=[8] … -0xA8=[12] …
    // -0x60=[30] … -0x30=[42], total 0xD8/4 = 54 floats.
    float   __frame[54] = {0};
    float *const local_d8 = &__frame[0];    // ebp-0xD8 (8 floats + el [8] de abajo)
    float &local_b8 = __frame[8];           // ebp-0xB8  == local_d8[8]
    float &local_b4 = __frame[9];           // ebp-0xB4
    float &local_b0 = __frame[10];          // ebp-0xB0  == (&local_b4)[1]
    float &local_ac = __frame[11];          // ebp-0xAC  == (&local_b4)[2]
    float *const local_a8 = &__frame[12];   // ebp-0xA8 (18 floats)
    float *const local_60 = &__frame[30];   // ebp-0x60 (12 floats)
    float *const local_30 = &__frame[42];   // ebp-0x30 (12 floats)

    // Compute delta from current pos to anchor pos (X component used as oscillation seed)
    local_dc_f = *(float *)(param_1 + 0x10) - *(float *)(param_1 + 0x44);
    pfVar15    = (float *)(param_1 + 0x10);
    pfVar14    = (float *)(param_1 + 0x44);
    local_e4_f = *(float *)(param_1 + 0x14) - *(float *)(param_1 + 0x48);

    // Default movement: rotate velocity by euler matrix and apply to position.
    // Type 0x4ee overrides this entirely (has its own path below).
    if (*(int *)(param_1 + 4) != 0x4ee) {
        Matrix_BuildFromEuler((float *)(param_1 + 0x28), local_30);
        local_b0 = -*(float *)(param_1 + 0x9c0);
        local_b4 = 0.0f;
        local_ac = 0.0f;
        Vector_Rotate(&local_b4, local_30, local_d8 + 6);
        *pfVar15                        = local_d8[6] + *pfVar15;
        *(float *)(param_1 + 0x14)      = local_d8[7] + *(float *)(param_1 + 0x14);
        *(float *)(param_1 + 0x18)      = local_b8   + *(float *)(param_1 + 0x18);
    }

    iVar16 = *(int *)(param_1 + 4);

    // ─────────────────────────────────────────────────────────────────
    // TYPE DISPATCH (switch on joint type id at +0x04)
    // ─────────────────────────────────────────────────────────────────

    // ── Type 0x4e8: Dragon/Smoke approach ────────────────────────────
    if (iVar16 == 0x4e8) {
        // IDA case 1256 (BITMAP_JOINT_FIRE en el source de MU 5.2,
        // ZzzEffectJoint.cpp:4243):
        //     Distance = MoveHumming(Position, Angle, TargetPosition, 0.f);
        //     ... if (Distance <= o->Velocity) { o->Live = false; ... }
        // IDA lo muestra como `sub_43E4A0(...); v295 = v4;` — `v4` es el retorno
        // FPU que Hex-Rays no tipa. Antes se aproximaba con `local_e4_f`
        // (distancia recalculada a mano); ahora se usa el valor real.
        const float dist_4e8 =
            FUN_0043e4a0(pfVar15, (float *)(param_1 + 0x28), pfVar14, 0.0f);
        float matrix_4e8[12];
        Matrix_BuildFromEuler((float *)(param_1 + 0x28), matrix_4e8);
        FUN_0046fe90((int)param_1, matrix_4e8);
        if (dist_4e8 > *(float *)(param_1 + 0x9c0)) {
            const float brightness = (float)(rand() % 4 + 4) * 0.1f;
            float light_4e8[3] = { brightness, brightness * 0.6f, brightness * 0.2f };
            FUN_004f76c0(*pfVar15, *(float *)(param_1 + 0x14), (int)light_4e8, 4, (int)&DAT_081cb608[0]);
            float step_4e8[3] = { 0.0f, -*(float *)(param_1 + 0x9c0), 0.0f };
            float rotated_4e8[3];
            Vector_Rotate(step_4e8, matrix_4e8, rotated_4e8);
            *pfVar15 += rotated_4e8[0];
            *(float *)(param_1 + 0x14) += rotated_4e8[1];
            *(float *)(param_1 + 0x18) += rotated_4e8[2];
        } else {
            *param_1 = 0;
            Particle_Spawn(1215, pfVar15, (float *)(param_1 + 0x28),
                         (float *)(param_1 + 0x34), 0, 1.0f, 0);
        }
#if 0 // Former approximation, retained only as source history.
        float *pfVar26 = (float *)(param_1 + 0x28);
        // FUN_0043e4a0 computes angle/distance and returns result via pfOut;
        // Ghidra shows its return used as float — call with a temp output.
        float arc_out = 0.0f;
        FUN_0043e4a0(pfVar15, pfVar26, pfVar14, 0.0f);  // result in pfVar14[2] area; use arc_out
        // In the decompile: local_e4 = (undefined4*)(float)fVar24 where fVar24 = FUN_0043e4a0(...)
        // The function signature is void in functions.h so we approximate the distance check
        // using *(float*)(param_1+0x9c0) vs the 3D distance already computed above.
        local_e4_f = sqrtf(local_dc_f * local_dc_f + local_e4_f * local_e4_f);
        Matrix_BuildFromEuler(pfVar26, local_a8 + 6);
        FUN_0046fe90((int)param_1, local_a8 + 6);
        if (*(float *)(param_1 + 0x9c0) < local_e4_f) {
            uVar7 = _rand();
            uVar7 = uVar7 & 0x80000003;
            if ((int)uVar7 < 0) {
                uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
            }
            local_e8_f = (float)(int)(uVar7 + 4);
            local_d8[3] = local_e8_f * _DAT_005524f4;
            local_d8[4] = local_d8[3] * _DAT_00552534;
            local_d8[5] = local_d8[3] * _DAT_005526e4;
            FUN_004f76c0(*pfVar15, *(float *)(param_1 + 0x14), (int)(local_d8 + 3), 4, (int)&DAT_081cb608[0]);
            local_d8[7] = -*(float *)(param_1 + 0x9c0);
            local_d8[6] = 0.0f;
            local_b8    = 0.0f;
            Vector_Rotate(local_d8 + 6, local_a8 + 6, &local_b4);
            *pfVar15                   = local_b4 + *pfVar15;
            *(float *)(param_1 + 0x14) = local_b0 + *(float *)(param_1 + 0x14);
            *(float *)(param_1 + 0x18) = local_ac + *(float *)(param_1 + 0x18);
        }
        else {
            *param_1 = 0;
            Particle_Spawn(0x4bf, pfVar15, pfVar26, (float *)(param_1 + 0x34), 0, 1.0f, 0);
        }
#endif
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4e2: Chain/rope follower ──────────────────────────────
    if (iVar16 == 0x4e2) {
        // IDA case 1250: three interpolated anchor steps, each offset by the
        // rotating vector stored at +0x9c4.  Keep the joint rotation matrix
        // separate from the entity angle matrix, as in the original.
        char *owner_4e2 = *(char **)(param_1 + 0x40);
        if (*owner_4e2 == '\0') {
            *(int *)(param_1 + 0x9b8) = -1;
            *pfVar15 = *(float *)(param_1 + 0x44);
            *(float *)(param_1 + 0x14) = *(float *)(param_1 + 0x48);
            *(float *)(param_1 + 0x18) = *(float *)(param_1 + 0x4c) + 50.0f;
        } else {
            float delta_4e2[3] = {
                *(float *)(owner_4e2 + 0x10) - *(float *)(owner_4e2 + 0x170),
                *(float *)(owner_4e2 + 0x14) - *(float *)(owner_4e2 + 0x174),
                *(float *)(owner_4e2 + 0x18) - *(float *)(owner_4e2 + 0x178)
            };
            *(float *)(param_1 + 0x44) = *(float *)(owner_4e2 + 0x170);
            *(float *)(param_1 + 0x48) = *(float *)(owner_4e2 + 0x174);
            *(float *)(param_1 + 0x4c) = *(float *)(owner_4e2 + 0x178);
            float entityAngle_4e2[3] = { *(float *)(owner_4e2 + 0x28), *(float *)(owner_4e2 + 0x2c), *(float *)(owner_4e2 + 0x30) };
            float jointMatrix_4e2[12];
            Matrix_BuildFromEuler((float *)(param_1 + 0x28), jointMatrix_4e2);
            delta_4e2[0] *= 1.0f / 3.0f;
            delta_4e2[1] *= 1.0f / 3.0f;
            delta_4e2[2] *= 1.0f / 3.0f;
            entityAngle_4e2[1] += *(float *)(param_1 + 0x9c0) - 90.0f;
            for (int segment_4e2 = 0; segment_4e2 < 3; ++segment_4e2) {
                entityAngle_4e2[1] += 30.0f;
                *(float *)(param_1 + 0x44) += delta_4e2[0];
                *(float *)(param_1 + 0x48) += delta_4e2[1];
                *(float *)(param_1 + 0x4c) += delta_4e2[2];
                float entityMatrix_4e2[12], rotated_4e2[3];
                Matrix_BuildFromEuler(entityAngle_4e2, entityMatrix_4e2);
                Vector_Rotate((float *)(param_1 + 0x9c4), entityMatrix_4e2, rotated_4e2);
                *pfVar15 = rotated_4e2[0] + *(float *)(param_1 + 0x44);
                *(float *)(param_1 + 0x14) = rotated_4e2[1] + *(float *)(param_1 + 0x48);
                *(float *)(param_1 + 0x18) = rotated_4e2[2] + *(float *)(param_1 + 0x4c);
                if (*(int *)(param_1 + 0x50) < *(int *)(param_1 + 0x54) - 1 || *(unsigned char *)(param_1 + 0x9d2) != 0)
                    FUN_0046fe90((int)param_1, jointMatrix_4e2);
            }
            if (*(int *)(param_1 + 0x9b8) < 15) {
                *(float *)(param_1 + 0x34) *= 2.0f / 3.0f;
                *(float *)(param_1 + 0x38) *= 2.0f / 3.0f;
                *(float *)(param_1 + 0x3c) *= 2.0f / 3.0f;
            }
        }
#if 0 // Former non-IDA reconstruction.
        pcVar11 = *(char **)(param_1 + 0x40);
        if (*pcVar11 == '\0') {
            // Linked entity is dead — snap to anchor + height offset, kill slot
            float fVar28 = *(float *)(param_1 + 0x4c) + _DAT_00552598;
            *(unsigned int *)(param_1 + 0x9b8) = 0xffffffff;
            *pfVar15                            = *pfVar14;
            *(undefined4 *)(param_1 + 0x14)    = *(undefined4 *)(param_1 + 0x48);
            *(float *)(param_1 + 0x18)          = fVar28;
            goto switchD_caseD_4ef;
        }
        // Compute target velocity delta (current_pos - prev_pos from entity +0x10/0x170)
        local_d8[0] = *(float *)(pcVar11 + 0x10)  - *(float *)(pcVar11 + 0x170);
        local_d8[1] = *(float *)(pcVar11 + 0x14)  - *(float *)(pcVar11 + 0x174);
        local_d8[2] = *(float *)(pcVar11 + 0x18)  - *(float *)(pcVar11 + 0x178);
        *pfVar14                         = *(float *)(pcVar11 + 0x170);
        *(undefined4 *)(param_1 + 0x48) = *(undefined4 *)(pcVar11 + 0x174);
        *(undefined4 *)(param_1 + 0x4c) = *(undefined4 *)(pcVar11 + 0x178);
        local_a8[3] = *(float *)(pcVar11 + 0x28);
        local_a8[4] = *(float *)(pcVar11 + 0x2c);
        local_a8[5] = *(float *)(pcVar11 + 0x30);
        Matrix_BuildFromEuler((float *)(param_1 + 0x28), local_60);
        local_d8[0] *= _DAT_005529d4;
        iVar16       = 3;
        local_d8[1] *= _DAT_005529d4;
        local_d8[2] *= _DAT_005529d4;
        local_a8[4] = (*(float *)(param_1 + 0x9c0) - _DAT_00552848) + local_a8[4];
        do {
            local_a8[4] += _DAT_0055284c;
            *(float *)(param_1 + 0x44) += local_d8[0];
            *(float *)(param_1 + 0x48) += local_d8[1];
            *(float *)(param_1 + 0x4c) += local_d8[2];
            Matrix_BuildFromEuler(local_a8 + 3, local_a8 + 6);
            Vector_Rotate((float *)(param_1 + 0x9c4), local_a8 + 6, local_a8);
            *pfVar15                        = local_a8[0] + *(float *)(param_1 + 0x44);
            *(float *)(param_1 + 0x14)      = local_a8[1] + *(float *)(param_1 + 0x48);
            *(float *)(param_1 + 0x18)      = local_a8[2] + *(float *)(param_1 + 0x4c);
            if ((*(int *)(param_1 + 0x50) < *(int *)(param_1 + 0x54) + -1) ||
                (*(char *)(param_1 + 0x9d2) != '\0'))
            {
                FUN_0046fe90((int)param_1, local_60);
            }
            iVar16--;
        } while (iVar16 != 0);
        if (0xe < *(int *)(param_1 + 0x9b8)) goto switchD_caseD_4ef;
        *(float *)(param_1 + 0x34) *= _DAT_00552a8c;
        *(float *)(param_1 + 0x38) *= _DAT_00552a8c;
        {
            float fVar28 = *(float *)(param_1 + 0x3c) * _DAT_00552a8c;
            *(float *)(param_1 + 0x3c) = fVar28;
        }
#endif
        goto switchD_caseD_4ef;
    }

    // ── Type 0x498: Quick fade-out ────────────────────────────────────
    if (iVar16 == 0x498) {
        if (*(int *)(param_1 + 8) == 0) {
            float fVar28 = *(float *)(param_1 + 0x9cc) + _DAT_00552660;
            *pfVar15                         = *pfVar14;
            *(undefined4 *)(param_1 + 0x14)  = *(undefined4 *)(param_1 + 0x48);
            *(float *)(param_1 + 0x9cc)      = fVar28;
            *(float *)(param_1 + 0x18)       = fVar28 + *(float *)(param_1 + 0x18);
        }
        if (4 < *(int *)(param_1 + 0x9b8)) goto switchD_caseD_4ef;
        *(float *)(param_1 + 0x34) *= _DAT_00552a90;
        *(float *)(param_1 + 0x38) *= _DAT_00552a90;
        {
            float fVar28 = *(float *)(param_1 + 0x3c) * _DAT_00552a90;
            goto LAB_00473578_498;
LAB_00473578_498:
            *(float *)(param_1 + 0x3c) = fVar28;
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x10a: Wispy follower ────────────────────────────────────
    if (iVar16 == 0x10a) {
        iVar16 = *(int *)(param_1 + 8);

        if (iVar16 == 2) {
            // Mode 2: interpolate towards bone target position
            if ((*(char **)(param_1 + 0x40) == (char *)0x0) ||
                (**(char **)(param_1 + 0x40) == '\0'))
            {
                *(undefined4 *)(param_1 + 0x40) = 0;
            }
            else {
                *(float *)(param_1 + 0x0c) =
                    (float)*(int *)(param_1 + 0x9b8) * _DAT_00552540;
                float t_raw = (float)(*(int *)(param_1 + 0x9b8) + -10) * _DAT_00552a00;
                float t;
                if ((_DAT_0055256c <= t_raw) || (_DAT_00552580 <= t_raw)) {
                    t = (_DAT_0055256c <= t_raw) ? 1.0f : 0.0f;
                }
                else {
                    t = 0.0f;
                }
                float inv_t = _DAT_0055256c - t;
                FUN_00473d90(param_2 * 0x4539, local_d8, 1.4f);
                iVar16 = *(int *)(param_1 + 0x40);
                local_d8[0] = local_d8[0] * _DAT_00552900 + *pfVar14;
                local_d8[1] = local_d8[1] * _DAT_00552900 + *(float *)(param_1 + 0x48);
                local_d8[2] = local_d8[2] * _DAT_00552900 + *(float *)(param_1 + 0x4c);
                local_a8[0] = *(float *)(iVar16 + 0x188);
                local_a8[1] = *(float *)(iVar16 + 0x18c);
                local_a8[2] = *(float *)(iVar16 + 400);
                // Blend: out[i] = t * rand_pos[i] + inv_t * bone_pos[i]
                pfVar15[0] = t * local_d8[0] + inv_t * local_a8[0];
                pfVar15[1] = t * local_d8[1] + inv_t * local_a8[1];
                pfVar15[2] = t * local_d8[2] + inv_t * local_a8[2];
            }
            goto switchD_caseD_4ef;
        }

        if (iVar16 == 3) {
            // Mode 3: sinusoidal oscillation, kill if linked entity dead
            if (**(char **)(param_1 + 0x40) == '\0') {
                goto LAB_0047036e;
            }
            int rand_roll = _rand();
            int new_lifetime;
            if (rand_roll % 5 == 0) {
                new_lifetime = *(int *)(param_1 + 0x9b8) + -5;
LAB_004702ce:
                *(int *)(param_1 + 0x9b8) = new_lifetime;
            }
            else {
                uVar7 = _rand();
                uVar7 = uVar7 & 0x80000001;
                bVar21 = (uVar7 == 0);
                if ((int)uVar7 < 0) {
                    bVar21 = ((uVar7 - 1 | 0xfffffffe) == 0xffffffff);
                }
                if (bVar21) {
                    new_lifetime = *(int *)(param_1 + 0x9b8) + 1;
                    goto LAB_004702ce;
                }
            }
            iVar16 = *(int *)(param_1 + 0x40);
            fVar22 = fsin(((double)*(int *)(param_1 + 0x9b8) -
                           (double)*(float *)(param_1 + 0x9c8)) * (double)_DAT_00552874);
            fVar24 = (double)_DAT_00552540;
            *(float *)(param_1 + 0x9c4) = (float)(fVar22 * fVar24);
            *pfVar15 = (float)(fVar22 * fVar24 + (double)*pfVar15);
            if (iVar16 == 0) goto switchD_caseD_4ef;
            {
                int hp_val = *(int *)(iVar16 + 0x60);
                float fVar28;
                if (hp_val < 0x33) {
                    fVar28 = (float)hp_val * _DAT_00552a00;
                }
                else {
                    fVar28 = (float)(100 - hp_val) * _DAT_00552aa4;
                }
                if (_DAT_0055256c < fVar28) {
                    fVar28 = _DAT_0055256c;
                }
                *(float *)(param_1 + 0x0c) = fVar28;
            }
            goto switchD_caseD_4ef;
        }

        // Modes 0, 1, 4: orbit / spiral with trig
        if (**(char **)(param_1 + 0x40) == '\0') {
LAB_0047036e:
            *(undefined4 *)(param_1 + 0x9b8) = 0;
            *param_1 = 0;
            goto switchD_caseD_4ef;
        }
        if (iVar16 == 4) {
            if (((*(unsigned char *)(*(int *)(param_1 + 0x40) + 0x78)) & 8) != 8)
                goto LAB_0047036e;
            *(undefined4 *)(param_1 + 0x9b8) = 100;
        }

        // 2026-09-03 -- DESVIACION DOCUMENTADA, no encontrada en IDA.
        // Estos dos bucles des-trasladan y vuelven a trasladar TODA la historia
        // de segmentos por el delta del ancla, de modo que la estela acompana al
        // owner cuando se mueve.  Busque su origen en el binario y NO esta:
        //   - cuerpo del tipo 266 de MoveJoint (0x00470030 L890-1240): los unicos
        //     bucles son el `for (j<3)` del subtipo 2 y el del subtipo 9;
        //   - MoveJoints (0x004736E0) es solo el walker del pool;
        //   - el renderer (0x00473710) emite los vertices crudos, sin sumar ancla.
        // Pero SIN ellos el aura se deshace en tiras sueltas al caminar, y en el
        // cliente original el anillo queda pegado al personaje.  O sea el binario
        // re-ancla en algun punto que todavia no ubique.  Se conservan hasta
        // encontrarlo; si aparece el sitio real, esto se reemplaza por el port fiel.
        if ((iVar16 == 0) || (iVar16 == 4)) {
            // Subtract anchor from segment positions (un-translate)
            // 2026-09-03 FIX (la banda de Icarus): el bucle cubria las filas
            // 0..segCount-1, pero el renderer (0x00473710) dibuja segCount
            // quads leyendo las filas segIdx y segIdx+1, o sea llega hasta la
            // fila **segCount**; y `sub_46FE90` tambien escribe esa fila.  La
            // fila de mas quedaba fuera del par restar/sumar y derivaba: la
            // sonda JSPAN la cazo con un quad de 1572 unidades entre
            // (1648,1571,299) y (1638,0,285) -- una raya cruzando el mapa.
            // Ahora se cubren segCount+1 filas, acotado a segMax.
            int seg_rows = *(int *)(param_1 + 0x50) + 1;
            {
                const int seg_max = *(int *)(param_1 + 0x54);
                if (seg_rows > seg_max) seg_rows = seg_max;
            }
            if (seg_rows > 0) {
                float *pfVar26 = (float *)(param_1 + (seg_rows + -1) * 0x30 + 0x5c);
                do {
                    iVar13 = 4;
                    float *pf = pfVar26;
                    do {
                        iVar13--;
                        pf[-1] -= *pfVar14;
                        *pf    -= *(float *)(param_1 + 0x48);
                        pf[1]  -= *(float *)(param_1 + 0x4c);
                        pf += 3;
                    } while (iVar13 != 0);
                    pfVar26  += -0xc;
                    seg_rows--;
                } while (seg_rows != 0);
            }
        }

        // Update anchor from linked entity current position
        iVar16   = *(int *)(param_1 + 0x40);
        *pfVar14 = *(float *)(iVar16 + 0x10);
        *(undefined4 *)(param_1 + 0x48) = *(undefined4 *)(iVar16 + 0x14);
        *(float *)(param_1 + 0x4c)      = *(float *)(iVar16 + 0x18) + _DAT_00552488;

        iVar16 = *(int *)(param_1 + 8);

        if ((iVar16 == 0) || (iVar16 == 4)) {
            // Add anchor back to segment positions (re-translate)
            // 2026-09-03 FIX (la banda de Icarus): el bucle cubria las filas
            // 0..segCount-1, pero el renderer (0x00473710) dibuja segCount
            // quads leyendo las filas segIdx y segIdx+1, o sea llega hasta la
            // fila **segCount**; y `sub_46FE90` tambien escribe esa fila.  La
            // fila de mas quedaba fuera del par restar/sumar y derivaba: la
            // sonda JSPAN la cazo con un quad de 1572 unidades entre
            // (1648,1571,299) y (1638,0,285) -- una raya cruzando el mapa.
            // Ahora se cubren segCount+1 filas, acotado a segMax.
            int seg_rows = *(int *)(param_1 + 0x50) + 1;
            {
                const int seg_max = *(int *)(param_1 + 0x54);
                if (seg_rows > seg_max) seg_rows = seg_max;
            }
            if (seg_rows > 0) {
                float *pfVar26 = (float *)(param_1 + (seg_rows + -1) * 0x30 + 0x5c);
                do {
                    iVar13 = 4;
                    float *pf = pfVar26;
                    do {
                        iVar13--;
                        pf[-1] += *pfVar14;
                        *pf    += *(float *)(param_1 + 0x48);
                        pf[1]  += *(float *)(param_1 + 0x4c);
                        pf += 3;
                    } while (iVar13 != 0);
                    pfVar26  += -0xc;
                    seg_rows--;
                } while (seg_rows != 0);
            }
        }

        /* HashTable obfuscation block skipped */
        // (ref-count insert/remove on DAT_055c9bc8/bcc/bd0/bd4 — not game logic)

        // Harmonic oscillation seed for wispy movement
        uVar7  = param_2 & 0x80000001;
        bVar21 = (uVar7 == 0);
        if ((int)uVar7 < 0) {
            bVar21 = ((uVar7 - 1 | 0xfffffffe) == 0xffffffff);
        }
        // 2026-08-24 FIX (Soul Barrier: el efecto se deformaba y cubria al pj en
        // vez de orbitar): la semilla era `local_dc_f`, que es el DELTA X
        // (Position.x - Target.x) — un valor geometrico. IDA (0x470030 L1035-1039)
        // usa MoveSceneFrame, el contador de frames:
        //     v49 = LODWORD(v297);                      // v297 = MoveSceneFrame
        //     if ( !(iIndex % 2) ) v49 = -LODWORD(v297);
        //     v52 = iIndex + v49 + 53730 * iIndex;
        // Con el delta X la posicion del joint se calcula a partir de si misma:
        // realimentacion -> la orbita se abre en cada frame hasta cubrir al pj.
        // Hex-Rays reusa el slot `v297` varias veces en esta funcion y el port
        // tomo el valor de otro tramo.
        //
        // `LODWORD` sobre un float toma sus BITS: MoveSceneFrame es un contador
        // entero (DAT_083a7c00, DWORD) que Hex-Rays tipeo float, asi que el valor
        // correcto es el entero directo — no `float_as_int` de un float negado,
        // que solo invierte el bit de signo y da otra cosa.
        //
        // Ojo el sentido: IDA niega cuando el indice es PAR (`if (!(iIndex % 2))`),
        // al reves de lo que hacia el port.
        const int frameSeed = (int)DAT_083a7c00;          // MoveSceneFrame
        const int dc_signed = bVar21 ? -frameSeed : frameSeed;
        iVar13 = dc_signed + (int)(param_2 * 0xd1e3);
        local_a8[2] = 0.1113f;
        iVar16 = *(int *)(param_1 + 8);
        float fFreq  = _DAT_00552a9c;
        float fScale = _DAT_00552aa0;
        if (iVar16 == 1) {
            local_a8[2] = 0.05565f;
            fFreq  = _DAT_00552a94;
            fScale = _DAT_00552a98;
        }
        int seed1 = iVar13 + 0x2b67;
        float ang0 = (float)iVar13 * fFreq;
        fVar24 = (double)fScale * (double)(iVar13 + 0xd903);
        fVar22 = fsin(fVar24);
        fVar23 = fcos((double)ang0);
        local_a8[3] = (float)(fVar23 * fVar22);
        fVar23 = fsin((double)ang0);
        local_a8[4] = (float)(fVar23 * fVar22);
        fVar24 = fcos(fVar24);
        fVar22 = fsin((double)seed1 * (double)local_a8[2]);
        float sin_s1 = (float)fVar22;
        fVar22 = fcos((double)seed1 * (double)local_a8[2]);
        local_a8[1] = (float)(sin_s1 * (double)local_a8[4] + fVar22 * fVar24);
        fVar24 = fVar22 * (double)local_a8[4] - (double)sin_s1 * fVar24;

        if (iVar16 == 0 || iVar16 == 4) {
            *(float *)(param_1 + 0x10) =
                (float)(fVar24 * (double)_DAT_00552878 + (double)*pfVar14);
            *(float *)(param_1 + 0x14) =
                local_a8[1] * _DAT_00552878 + *(float *)(param_1 + 0x48);
            *(float *)(param_1 + 0x18) =
                local_a8[3] * _DAT_00552908 + *(float *)(param_1 + 0x4c) + _DAT_005528fc;
            goto switchD_caseD_4ef;
        }
        if (iVar16 == 1) {
            *(unsigned int *)(param_1 + 0x34) = 0x3e4ccccd;  // 0.2f
            *(unsigned int *)(param_1 + 0x38) = 0x3e4ccccd;
            *(float *)(param_1 + 0x10) =
                (float)(fVar24 * (double)_DAT_00552938 + (double)*pfVar14);
            *(float *)(param_1 + 0x14) =
                local_a8[1] * _DAT_00552938 + *(float *)(param_1 + 0x48);
            *(float *)(param_1 + 0x18) =
                local_a8[3] * _DAT_00552958 + *(float *)(param_1 + 0x4c);
            {
                float fVar28 = sin_s1 * _DAT_005526e4 + _DAT_005528b4;
                *(float *)(param_1 + 0x3c) = fVar28;
            }
            goto switchD_caseD_4ef;
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x48e → redirect to 0x4ed ───────────────────────────────
    // ── Type 0x4e1 → redirect to 0x4fd ───────────────────────────────
    if (iVar16 == 0x4e1) {
        goto switchD_caseD_4fd;
    }

    // ── Type 0x4e5: Pet/butterfly orbit ──────────────────────────────
    if (iVar16 == 0x4e5) {
        // IDA 00470030, case 1253 / subtypes 0 and 5.
        const int subtype_4e5 = *(int *)(param_1 + 8);
        if (subtype_4e5 == 1) {
            float light_4e5_1[3] = { 0.8f, 0.4f, 1.0f };
            FUN_004795c0(1150, pfVar15, 4.0f, light_4e5_1,
                          *(int *)(param_1 + 0x40), (float)(rand() % 360), 0);
            goto switchD_caseD_4ef;
        }
        if (subtype_4e5 == 3) {
            float orbit_4e5_3[3];
            float light_4e5_3[3] = { 1.0f, 0.5f, 0.1f };
            FUN_00473d90((int)param_2, orbit_4e5_3, 1.0f);
            *pfVar15 += orbit_4e5_3[0] * 50.0f;
            *(float *)(param_1 + 0x14) += orbit_4e5_3[1] * 50.0f;
            *(float *)(param_1 + 0x18) += orbit_4e5_3[2] * 50.0f;
            FUN_004795c0(1150, pfVar15, 3.0f, light_4e5_3,
                          *(int *)(param_1 + 0x40), (float)(rand() % 360), 0);
            FUN_004795c0(1231, pfVar15, 1.5f, light_4e5_3,
                          *(int *)(param_1 + 0x40), (float)(rand() % 360), 0);
            goto switchD_caseD_4ef;
        }
        if (subtype_4e5 == 2) {
            const int lifetime_4e5_2 = *(int *)(param_1 + 0x9b8);
            float light_4e5_2[3] = { 1.0f, 0.5f, 0.1f };
            if (lifetime_4e5_2 >= 10) {
                if (lifetime_4e5_2 > 18 && lifetime_4e5_2 < 20) {
                    float rotation_4e5_2[3] = { 0.0f, 0.0f, (float)(rand() % 360) };
                    float position_4e5_2[3] = {
                        (float)(rand() % 200) + *(float *)(param_1 + 0x28) - 100.0f,
                        (float)(rand() % 200) + *(float *)(param_1 + 0x2c) - 100.0f,
                        *(float *)(param_1 + 0x30) - 200.0f
                    };
                    Joint_Create(1249, position_4e5_2, position_4e5_2, rotation_4e5_2,
                                  2, 0, 40.0f, -1, 0);
                }
            } else {
                const float fade_4e5_2 = *(float *)(param_1 + 0x34) * (5.0f / 6.0f);
                *(float *)(param_1 + 0x34) = fade_4e5_2;
                *(float *)(param_1 + 0x38) = fade_4e5_2;
                *(float *)(param_1 + 0x3c) = fade_4e5_2;
                light_4e5_2[0] = fade_4e5_2;
                light_4e5_2[1] = fade_4e5_2;
                light_4e5_2[2] = fade_4e5_2;
            }
            const float scale_4e5_2 = (float)((20 - lifetime_4e5_2) / 5) + 4.0f;
            FUN_004795c0(1150, pfVar15, scale_4e5_2, light_4e5_2,
                          *(int *)(param_1 + 0x40), (float)(rand() % 360), 0);
            *(float *)(param_1 + 0x9c0) += 5.0f;
            goto switchD_caseD_4ef;
        }
        if (subtype_4e5 == 0 || subtype_4e5 == 5) {
            if (*(int *)(param_1 + 0x0c) == 1117782016) {
                Effect_Create(205, pfVar15, (float *)(param_1 + 0x28), (float *)(param_1 + 0x34),
                             (float *)(subtype_4e5 == 5 ? 3 : 0), 0, (float *)-1, 0, 0);
                if ((*(int *)(param_1 + 0x9b8) % 15) == 0 && *(int *)(param_1 + 0x40) == (int)Hero) {
                    FUN_0045fec0(*(unsigned char *)(param_1 + 0x9d2), pfVar15, 150.0f,
                                  *(unsigned char *)(param_1 + 0x9d3), *(unsigned short *)(param_1 + 0x9d0));
                }
            }

            const int owner_4e5 = *(int *)(param_1 + 0x40);
            *(float *)(param_1 + 0x44) = *(float *)(owner_4e5 + 0x10);
            *(float *)(param_1 + 0x48) = *(float *)(owner_4e5 + 0x14);
            *(float *)(param_1 + 0x4c) = *(float *)(owner_4e5 + 0x18) + 80.0f;
            const float horizontalDistance_4e5 = sqrtf(local_e4_f * local_e4_f + local_dc_f * local_dc_f);
            FUN_0043e4a0(pfVar15, (float *)(param_1 + 0x28), (float *)(param_1 + 0x44),
                          subtype_4e5 == 5 ? 2.0f : 10.0f);
            if (*(unsigned char *)(param_1 + 0x9bc) == 0 && *(float *)(param_1 + 0x9c0) * 2.0f >= horizontalDistance_4e5)
                *(unsigned char *)(param_1 + 0x9bc) = 1;

            if (subtype_4e5 != 5) {
                *(float *)(param_1 + 0x9c4) += (float)(rand() % 32 - 16) * 0.2f;
                const float angleZ = (float)(rand() % 32 - 16) * 0.8f + *(float *)(param_1 + 0x9cc);
                *(float *)(param_1 + 0x28) += *(float *)(param_1 + 0x9c4);
                *(float *)(param_1 + 0x30) += angleZ;
                *(float *)(param_1 + 0x9c4) *= 0.6f;
                *(float *)(param_1 + 0x9cc) = angleZ * 0.8f;
            }

            const float terrain_4e5 = FUN_004f7500(*pfVar15, *(float *)(param_1 + 0x14));
            if (terrain_4e5 + 100.0f > *(float *)(param_1 + 0x18)) {
                *(float *)(param_1 + 0x9c4) = 0.0f;
                *(float *)(param_1 + 0x28) = -5.0f;
            }
            if (terrain_4e5 + 400.0f < *(float *)(param_1 + 0x18)) {
                *(float *)(param_1 + 0x9c4) = 0.0f;
                *(float *)(param_1 + 0x28) = 5.0f;
            }
            const float color_4e5 = (float)*(int *)(param_1 + 0x9b8) * 0.1f;
            *(float *)(param_1 + 0x34) = color_4e5;
            *(float *)(param_1 + 0x38) = color_4e5;
            *(float *)(param_1 + 0x3c) = color_4e5;
            const float darkness_4e5 = (float)(rand() % 4 + 4) * -0.01f;
            float light_4e5[3] = { darkness_4e5, darkness_4e5, darkness_4e5 };
            FUN_004f76c0(*pfVar15, *(float *)(param_1 + 0x14), (int)light_4e5, 4, (int)&DAT_081cb608[0]);
            goto switchD_caseD_4ef;
        }

        // No default subtype branch exists in IDA for type 1253.
        goto switchD_caseD_4ef;
    }

    // 2026-09-02 (Lightning sin rayo): el tipo 1254 NO tiene bloque propio en
    // IDA MoveJoint (0x00470030) -- su unica mencion en toda la funcion es la
    // exclusion del epilogo (L2226).  El bloque que habia aca era invencion del
    // port y hacia DOS cosas daninas:
    //   1. Llamaba `sub_46FE90` una segunda vez por frame (el epilogo ya la
    //      llama), o sea DOBLE scroll de la historia de segmentos: la estela
    //      perdia la mitad de su largo y duplicaba la cabeza.
    //   2. Aplicaba el fade `if (lifetime < 5) color *= 0.76923078`, que en
    //      IDA pertenece al tipo **1176** (L785-791), no al 1254.
    // Con lifetime 2 (CreateJoint case 1254 sub 0 -> LABEL_57), las dos cosas
    // juntas dejaban el rayo del Lightning practicamente invisible.
    if (iVar16 == 0x4e6) {
        MoveJoint_GenericTail(param_1);   // IDA: 1254 no tiene case -> cae en la cola generica
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4e7: Scatter effect ────────────────────────────────────
    if (iVar16 == 0x4e7) {
        if (*(int *)(param_1 + 0x9b8) > 15) {
            float target_4e7[3] = {
                (float)(rand() % 200) + *(float *)(param_1 + 0x44) - 100.0f,
                (float)(rand() % 100) + *(float *)(param_1 + 0x48) - 50.0f,
                *(float *)(param_1 + 0x4c)
            };
            // 2026-09-03 (haz que cruzaba Icarus): IDA (0x00470030 L1880-1886)
            //     *v2 = *(float *)(o + 28);
            //     *(_DWORD *)(o + 20) = *(_DWORD *)(o + 32);
            //     *(_DWORD *)(o + 24) = *(_DWORD *)(o + 36);
            // Esos 28/32/36 son DECIMALES = 0x1C/0x20/0x24, donde `CreateJoint`
            // guarda el ORIGEN del rayo (LABEL_61: `*((float *)v11 + 7) = *v12`).
            // El port los leyo como HEX y reseteaba desde 0x28/0x2c/0x30, que es
            // el vector Angle (~0,0,0 para estos joints): el rayo renacia en la
            // esquina del mapa en cada rebuild y la estela lo unia con el cielo
            // -- el haz azul que cruzaba Icarus.  Medido con la sonda JBEAM:
            // `type=1255 sub=9 P=(3167,928,58) v0=(1251,54,139)`.
            *pfVar15                   = *(float *)(param_1 + 0x1c);
            *(float *)(param_1 + 0x14) = *(float *)(param_1 + 0x20);
            *(float *)(param_1 + 0x18) = *(float *)(param_1 + 0x24);
            const int segmentLimit_4e7 = *(int *)(param_1 + 0x54);
            for (int segment_4e7 = 0; segment_4e7 < segmentLimit_4e7; ++segment_4e7) {
                const float speed_4e7 = (float)(rand() % 80) + 60.0f;
                FUN_0043e4a0(pfVar15, (float *)(param_1 + 0x28), target_4e7, speed_4e7);
                *(float *)(param_1 + 0x9c4) = (float)(rand() % 1400 - 700) / *(float *)(param_1 + 0x0c);
                *(float *)(param_1 + 0x9cc) = (float)(rand() % 1400 - 700) / *(float *)(param_1 + 0x0c);
                float angle_4e7[3] = {
                    *(float *)(param_1 + 0x28) + *(float *)(param_1 + 0x9c4),
                    *(float *)(param_1 + 0x2c) + *(float *)(param_1 + 0x9c8),
                    *(float *)(param_1 + 0x30) + *(float *)(param_1 + 0x9cc)
                };
                float matrix_4e7[12], step_4e7[3] = { 0.0f, -speed_4e7, 0.0f }, rotated_4e7[3];
                Matrix_BuildFromEuler(angle_4e7, matrix_4e7);
                FUN_0046fe90((int)param_1, matrix_4e7);
                Vector_Rotate(step_4e7, matrix_4e7, rotated_4e7);
                *pfVar15 += rotated_4e7[0];
                *(float *)(param_1 + 0x14) += rotated_4e7[1];
                *(float *)(param_1 + 0x18) += rotated_4e7[2];
            }
        } else {
            *(float *)(param_1 + 0x3c) -= 10.12f;
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4e9: Glow ring ────────────────────────────────────────
    if (iVar16 == 0x4e9) {
        // IDA case 1257: only subtype 1 changes state; all other subtypes
        // proceed directly to the common epilogue.
        if (*(int *)(param_1 + 8) == 1) {
            *(float *)(param_1 + 0x9c0) += 0.3f;
            const float fade_4e9 = *(float *)(param_1 + 0x34) * (5.0f / 7.0f);
            *(float *)(param_1 + 0x34) = fade_4e9;
            *(float *)(param_1 + 0x38) = fade_4e9;
            *(float *)(param_1 + 0x3c) = fade_4e9;
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4ea: Crawling creature ────────────────────────────────
    if (iVar16 == 0x4ea) {
        const int subtype_4ea = *(int *)(param_1 + 8);
        if (subtype_4ea == 0 || subtype_4ea == 1 || subtype_4ea == 6 || subtype_4ea == 9) {
            if (*(int *)(param_1 + 0x9b8) < 100) {
                *(float *)(param_1 + 0x9c0) += 5.0f;
                if (*(float *)(param_1 + 0x9c0) >= 30.0f)
                    *(float *)(param_1 + 0x9c0) = 30.0f;
                const float originalAngleZ_4ea = *(float *)(param_1 + 0x30);
                float target_4ea[3];
                if (subtype_4ea == 6 || subtype_4ea == 9) {
                    target_4ea[0] = *(float *)(param_1 + 0x44);
                    target_4ea[1] = *(float *)(param_1 + 0x48);
                    target_4ea[2] = *(float *)(param_1 + 0x4c);
                } else {
                    const int owner_4ea = *(int *)(param_1 + 0x40);
                    target_4ea[0] = *(float *)(owner_4ea + 0x10);
                    target_4ea[1] = *(float *)(owner_4ea + 0x14);
                    target_4ea[2] = *(float *)(owner_4ea + 0x18) + 120.0f;
                    *(float *)(param_1 + 0x44) = target_4ea[0];
                    *(float *)(param_1 + 0x48) = target_4ea[1];
                    *(float *)(param_1 + 0x4c) = target_4ea[2];
                }
                // 2026-08-16: `Distance` es el RETORNO de MoveHumming, no la Z
                // del target. Hex-Rays tipaba FUN_0043e4a0 como void (retorno en
                // st0) y este port comparaba `target[2]` = ownerZ + 120, que en
                // cualquier mapa es >> 35 → las esferas de EXP nunca llegaban a
                // absorberse y orbitaban al pj acumulandose. Confirmado contra el
                // source de MU 5.2 (ZzzEffectJoint.cpp:3368).
                const float dist_4ea =
                    FUN_0043e4a0(pfVar15, (float *)(param_1 + 0x28), target_4ea,
                                 *(float *)(param_1 + 0x9c0));
                if (dist_4ea > 35.0f) {
                    if (dist_4ea <= 70.0f && fabsf(originalAngleZ_4ea - *(float *)(param_1 + 0x30)) > 20.0f &&
                        *(float *)(param_1 + 0x9c0) >= 20.0f)
                        *(float *)(param_1 + 0x9c0) -= 10.0f;
                } else {
                    *param_1 = 0;
                    Particle_Spawn(1191, pfVar15, (float *)(param_1 + 0x28), (float *)(param_1 + 0x34), 1, 1.0f, 0);
                    PlayBuffer(35, 0, 0);
                }
            } else {
                if (subtype_4ea != 0) {
                    *(float *)(param_1 + 0x30) -= 10.0f;
                    *(float *)(param_1 + 0x18) += 5.0f;
                } else {
                    *(float *)(param_1 + 0x30) += 10.0f;
                    *(float *)(param_1 + 0x18) += 6.0f;
                }
            }
            const float brightness_4ea = (float)(rand() % 4 + 8) * 0.03f;
            float terrainLight_4ea[3] = { brightness_4ea * 0.4f, brightness_4ea, brightness_4ea * 0.8f };
            FUN_004f76c0(*pfVar15, *(float *)(param_1 + 0x14), (int)terrainLight_4ea, 2, (int)&DAT_081cb608[0]);
            if (subtype_4ea == 6 || subtype_4ea == 9)
                Particle_Spawn(1191, pfVar15, (float *)(param_1 + 0x28), (float *)(param_1 + 0x34), 3, 0.05f, 0);
            else
                Particle_Spawn(1191, pfVar15, (float *)(param_1 + 0x28), (float *)(param_1 + 0x34), 0, 1.0f, 0);
            goto switchD_caseD_4ef;
        }
        if (subtype_4ea == 2 || subtype_4ea == 3 || subtype_4ea == 4 || subtype_4ea == 5 || subtype_4ea == 8) {
            const int owner_4ea = *(int *)(param_1 + 0x40);
            if (*(unsigned char *)owner_4ea == 0) {
                *param_1 = 0;
                return 0;
            }
            if (subtype_4ea == 3) {
                *pfVar15 = *(float *)(owner_4ea + 0x4c);
                *(float *)(param_1 + 0x14) = *(float *)(owner_4ea + 0x50);
                *(float *)(param_1 + 0x18) = *(float *)(owner_4ea + 0x54);
            } else {
                *pfVar15 = *(float *)(owner_4ea + 0x40);
                *(float *)(param_1 + 0x14) = *(float *)(owner_4ea + 0x44);
                *(float *)(param_1 + 0x18) = *(float *)(owner_4ea + 0x48);
                if (subtype_4ea == 8)
                    *(float *)(param_1 + 0x0c) += 10.1f;
            }
            if (subtype_4ea == 2 || subtype_4ea == 3) {
                const float pulse_4ea = sinf((float)WorldTime * 0.002f) * 0.3f + 0.8f;
                float light_4ea[3] = { pulse_4ea * 0.5f, pulse_4ea * 0.1f, pulse_4ea };
                Particle_Spawn(1191, pfVar15, (float *)(param_1 + 0x28), light_4ea, 0, 1.0f, 0);
            }
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4eb: Balloon/fireball ─────────────────────────────────
    if (iVar16 == 0x4eb) {
        const int subtype_4eb = *(int *)(param_1 + 8);
        const int lifetime_4eb = *(int *)(param_1 + 0x9b8);
        if (subtype_4eb == 4) {
            const int owner_4eb = *(int *)(param_1 + 0x40);
            const float anchorX_4eb = *(float *)(param_1 + 0x44);
            *(float *)(param_1 + 0x10) = *(float *)(owner_4eb + 0x10) + anchorX_4eb;
            *(float *)(param_1 + 0x14) = *(float *)(owner_4eb + 0x14) + *(float *)(param_1 + 0x48);
            *(float *)(param_1 + 0x18) = *(float *)(owner_4eb + 0x18) + *(float *)(param_1 + 0x4c) + *(float *)(param_1 + 0x9c0);
            *(float *)(param_1 + 0x9c0) += 10.0f;
            const float fade_4eb = (float)(12 - lifetime_4eb) * 0.1f;
            *(float *)(param_1 + 0x34) = fade_4eb * 0.4f;
            *(float *)(param_1 + 0x38) = fade_4eb * 0.6f;
            *(float *)(param_1 + 0x3c) = fade_4eb;
            goto switchD_caseD_4ef;
        }
        if (subtype_4eb == 5) {
            const float fade_4eb = (float)(12 - lifetime_4eb) * 0.1f;
            *(float *)(param_1 + 0x34) = fade_4eb * 0.4f;
            *(float *)(param_1 + 0x38) = fade_4eb * 0.6f;
            *(float *)(param_1 + 0x3c) = fade_4eb;
            goto switchD_caseD_4ef;
        }

        *(float *)(param_1 + 0x9c0) += (subtype_4eb == 6 || subtype_4eb == 7) ? 2.0f : 4.0f;
        float target_4eb[3];
        if (subtype_4eb == 6 || subtype_4eb == 7) {
            target_4eb[0] = *(float *)(param_1 + 0x44);
            target_4eb[1] = *(float *)(param_1 + 0x48);
            target_4eb[2] = *(float *)(param_1 + 0x4c);
        } else {
            const int owner_4eb = *(int *)(param_1 + 0x40);
            target_4eb[0] = *(float *)(owner_4eb + 0x10);
            target_4eb[1] = *(float *)(owner_4eb + 0x14);
            target_4eb[2] = *(float *)(owner_4eb + 0x18) + 120.0f;
            FUN_0043e4a0(pfVar15, (float *)(param_1 + 0x28), target_4eb, 10.0f);
        }
        const float fade_4eb = (float)(12 - lifetime_4eb) * 0.1f;
        if (subtype_4eb == 1) {
            *(float *)(param_1 + 0x34) = fade_4eb * 0.4f;
            *(float *)(param_1 + 0x38) = fade_4eb * 0.6f;
            *(float *)(param_1 + 0x3c) = fade_4eb;
        } else if (subtype_4eb == 2) {
            *(float *)(param_1 + 0x34) = fade_4eb * 0.4f;
            *(float *)(param_1 + 0x38) = fade_4eb;
            *(float *)(param_1 + 0x3c) = fade_4eb * 0.6f;
        } else if (subtype_4eb == 3) {
            *(float *)(param_1 + 0x34) = fade_4eb;
            *(float *)(param_1 + 0x38) = fade_4eb * 0.6f;
            *(float *)(param_1 + 0x3c) = fade_4eb * 0.4f;
        }
        if (subtype_4eb == 6 && lifetime_4eb <= 10) {
            const float pulse_4eb = (float)(6 - abs(lifetime_4eb - 6)) * 0.15f;
            float light_4eb[3] = { pulse_4eb, pulse_4eb, pulse_4eb };
            FUN_004795c0(1231, target_4eb, (float)(rand() % 8 + 8) * 0.05f,
                          light_4eb, *(int *)(param_1 + 0x40), (float)(rand() % 360), 0);
        } else if (subtype_4eb != 7 && lifetime_4eb == 1) {
            FUN_004795c0(1231, target_4eb, (float)(rand() % 8 + 8) * 0.2f,
                          (float *)(param_1 + 0x34), *(int *)(param_1 + 0x40), (float)(rand() % 360), 0);
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4ec: Seeker ───────────────────────────────────────────
    if (iVar16 == 0x4ec) {
        const int owner = *(int *)(param_1 + 0x40);
        *(float *)(param_1 + 0x44) = *(float *)(owner + 0x10);
        *(float *)(param_1 + 0x48) = *(float *)(owner + 0x14);
        *(float *)(param_1 + 0x4c) = *(float *)(owner + 0x18) + 130.0f;

        const float horizontalDistance = sqrtf(local_e4_f * local_e4_f + local_dc_f * local_dc_f);
        const float turnStep = 3000.0f / horizontalDistance;
        FUN_0043e4a0(pfVar15, (float *)(param_1 + 0x28), (float *)(param_1 + 0x44), turnStep);

        if (*(unsigned char *)(param_1 + 0x9bc) == 0 &&
            *(float *)(param_1 + 0x9c0) * 2.0f >= horizontalDistance) {
            *(unsigned char *)(param_1 + 0x9bc) = 1;
            *(int *)(param_1 + 0x9b8) = 5;
        }
        const float color = (float)*(int *)(param_1 + 0x9b8) * 0.1f;
        *(float *)(param_1 + 0x34) = color;
        *(float *)(param_1 + 0x38) = color;
        *(float *)(param_1 + 0x3c) = color;

        const float terrainLight = (float)(rand() % 4 + 4) * -0.01f;
        float light[3] = { terrainLight, terrainLight, terrainLight };
        FUN_004f76c0(*pfVar15, *(float *)(param_1 + 0x14), (int)light, 4, (int)&DAT_081cb608[0]);
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4ed: Firefly random wander ────────────────────────────
    // IDA 00470030 LABEL_286: types 0x4ED (1261) and 0x48E (1166)
    // share this guided chain.  0x4ED is not a random-wander branch.
    if (iVar16 == 0x4ed || iVar16 == 0x48e) {
        const int type = iVar16;
        const int segmentLimit = *(int *)(param_1 + 0x54);
        float targetDistance = local_e4_f;

        for (int segment = 0; segment < segmentLimit; ++segment) {
            if (*(int *)(param_1 + 8) != 2) {
                const int owner = *(int *)(param_1 + 0x40);
                *(float *)(param_1 + 0x44) = *(float *)(owner + 0x10);
                *(float *)(param_1 + 0x48) = *(float *)(owner + 0x14);
                targetDistance = *(float *)(owner + 0x18) + 80.0f;
                *(float *)(param_1 + 0x4c) = targetDistance;
            }

            FUN_0043e4a0(pfVar15, (float *)(param_1 + 0x28), (float *)(param_1 + 0x44), 25.0f);
            *(float *)(param_1 + 0x9c4) = ((float)(rand() % 256 - 128) / *(float *)(param_1 + 0x0c) + *(float *)(param_1 + 0x9c4)) * 0.8f;
            *(float *)(param_1 + 0x9c8) *= 0.8f;
            *(float *)(param_1 + 0x9cc) = ((float)(rand() % 256 - 128) / *(float *)(param_1 + 0x0c) + *(float *)(param_1 + 0x9cc)) * 0.8f;

            float angle[3] = { *(float *)(param_1 + 0x28) + *(float *)(param_1 + 0x9c4),
                               *(float *)(param_1 + 0x2c) + *(float *)(param_1 + 0x9c8),
                               *(float *)(param_1 + 0x30) + *(float *)(param_1 + 0x9cc) };
            float matrix[12], rotated[3];
            Matrix_BuildFromEuler(angle, matrix);
            FUN_0046fe90((int)param_1, matrix);

            if (*(float *)(param_1 + 0x9c0) * 2.0f >= targetDistance) {
                if ((rand() & 1) == 0) {
                    (void)rand(); (void)rand(); (void)rand();
                    Particle_Spawn(1195, pfVar15, (float *)(param_1 + 0x28), (float *)(param_1 + 0x34), 0, 1.0f, 0);
                }
                break;
            }

            const float intensity = (float)(rand() % 4 + 4) * 0.05f;
            float light[3];
            if (type == 0x4ed) {
                light[0] = intensity;
                if (*(int *)(param_1 + 8) == 1) {
                    light[1] = intensity * 0.1f;
                    light[2] = light[1];
                } else {
                    light[1] = intensity * 0.6f;
                    light[2] = intensity * 0.3f;
                }
            } else {
                light[0] = 0.0f;
                light[1] = intensity * 0.1f;
                light[2] = intensity * 0.2f;
            }
            FUN_004f76c0(*pfVar15, *(float *)(param_1 + 0x14), (int)light, 2, (int)&DAT_081cb608[0]);

            float step[3] = { 0.0f, -*(float *)(param_1 + 0x9c0), 0.0f };
            Vector_Rotate(step, matrix, rotated);
            *pfVar15 += rotated[0];
            *(float *)(param_1 + 0x14) += rotated[1];
            *(float *)(param_1 + 0x18) += rotated[2];
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4ee: Cannon/torpedo — 8-iter chain (no extra tick) ────
    if (iVar16 == 0x4ee) {
        // IDA 00470030, case 1262: rebuild eight linked segments.  The
        // previous position is intentionally retained as the source of both
        // 1254 joints before the anchor-relative position is updated.
        float in1[3] = { 0.0f, -180.0f, 0.0f };
        for (int i = 0; i < 8; ++i) {
            if (*(int *)(param_1 + 0x50) < *(int *)(param_1 + 0x54) - 1) {
                float previousPosition[3] = {
                    *(float *)(param_1 + 0x10), *(float *)(param_1 + 0x14), *(float *)(param_1 + 0x18)
                };
                float matrix[12];
                float rotated[3];

                Matrix_BuildFromEuler((float *)(param_1 + 0x9c4), matrix);
                Vector_Rotate(in1, matrix, rotated);
                *(float *)(param_1 + 0x10) = rotated[0] + *(float *)(param_1 + 0x44);
                *(float *)(param_1 + 0x14) = rotated[1] + *(float *)(param_1 + 0x48);
                *(float *)(param_1 + 0x18) = rotated[2] + *(float *)(param_1 + 0x4c);

                Matrix_BuildFromEuler((float *)(param_1 + 0x28), matrix);
                FUN_0046fe90((int)param_1, matrix);
                *(float *)(param_1 + 0x9cc) -= 11.0f;
                if ((rand() & 1) == 0) {
                    Particle_Spawn(1195, (float *)(param_1 + 0x10),
                                 (float *)(param_1 + 0x28), (float *)(param_1 + 0x34), 0, 1.0f, 0);
                }
                Joint_Create(1254, previousPosition, (float *)(param_1 + 0x10),
                              (float *)(param_1 + 0x28), 3, 0, (float)(rand() % 10) + 5.0f, 5, 10);
                Joint_Create(1254, previousPosition, (float *)(param_1 + 0x10),
                              (float *)(param_1 + 0x28), 3, 0, (float)(rand() % 8) + 4.0f, 5, 10);
            }
            if (*(int *)(param_1 + 0x40) == (int)Hero && *(int *)(param_1 + 0x9b8) > 18 && (i % 5) == 0) {
                FUN_0045fec0(*(unsigned char *)(param_1 + 0x9d2), (float *)(param_1 + 0x10), 150.0f,
                              *(unsigned char *)(param_1 + 0x9d3), *(unsigned short *)(param_1 + 0x9d0));
            }
        }
        const int lifetime = *(int *)(param_1 + 0x9b8);
        const float fade = lifetime >= 10 ? (float)lifetime * (1.0f / 30.0f)
                                          : *(float *)(param_1 + 0x34) * (2.0f / 3.0f);
        *(float *)(param_1 + 0x34) = fade;
        *(float *)(param_1 + 0x38) = fade;
        *(float *)(param_1 + 0x3c) = fade;
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4fd: Orbital ring ─────────────────────────────────────
    if (iVar16 == 0x4fd) {
        goto switchD_caseD_4fd;
    }

    // ─────────────────────────────────────────────────────────────────
    // LABEL TARGETS for redirect cases
    // ─────────────────────────────────────────────────────────────────

#if 0 // Superseded: 0x4ED/0x48E are handled by IDA LABEL_286 above.
    // Firefly: random wander — nudge X and Y randomly each frame
    {
        uVar7 = _rand();
        uVar7 = uVar7 & 0x80000001;
        bVar21 = (uVar7 == 0);
        if ((int)uVar7 < 0) {
            bVar21 = ((uVar7 - 1 | 0xfffffffe) == 0xffffffff);
        }
        if (!bVar21) {
            *(float *)(param_1 + 0x10) += _DAT_005524f0;
        }
        else {
            *(float *)(param_1 + 0x10) -= _DAT_005524f0;
        }
        uVar7 = _rand();
        uVar7 = uVar7 & 0x80000001;
        bVar21 = (uVar7 == 0);
        if ((int)uVar7 < 0) {
            bVar21 = ((uVar7 - 1 | 0xfffffffe) == 0xffffffff);
        }
        if (!bVar21) {
            *(float *)(param_1 + 0x14) += _DAT_005524f0;
        }
        else {
            *(float *)(param_1 + 0x14) -= _DAT_005524f0;
        }
        FUN_0046fe90((int)param_1, local_30);
        goto switchD_caseD_4ef;
    }

#endif
switchD_caseD_4fd:
    // ── IDA LABEL_301 — movimiento compartido por los tipos 1249 (0x4E1) y
    // 1277 (0x4FD). Es un switch sobre el SubType (+0x08).
    //
    // 2026-08-10 FIX (destello dorado errático del set +11): el port mandaba
    // TODOS los subtipos a la órbita pseudo-aleatoria de más abajo, sembrada con
    // el índice de slot — de ahí que el efecto saltara de un lado a otro en
    // cualquier altura. El subtipo 0 (que es el halo del set +11, spawneado por
    // `Entity_UpdateRender` sección 6) en el original es un **círculo que sube**:
    //     a = (lifetime + phase) * 0.1        (PKKey == -1)
    //     Position.x = cos(a) * 40 + TargetPos.x
    //     Position.y = TargetPos.y - sin(a) * 40
    //     Position.z += riseSpeed
    //
    // Se portan las ramas verificables 1:1 contra IDA (0, 10, 2, 3, 5, 13). Los
    // subtipos 4/6/7/8/9/11/12 quedan en el cuerpo viejo de abajo: su decompile
    // está entrelazado con ruido de hash-table (LABEL_438/439) y no se puede
    // leer limpio todavía. NO se tocan para no cambiar comportamiento que el
    // usuario no reportó. TODO: portarlos cuando se pueda aislar ese bloque.
    {
        const int  jsub  = *(int *)(param_1 + 8);
        float     *jposX = (float *)(param_1 + 0x10);   // Position.x  (IDA v2)
        const float tgtX = *(float *)(param_1 + 0x44);  // TargetPos.x (IDA v3)
        const float tgtY = *(float *)(param_1 + 0x48);  // TargetPos.y (o + 72)

        if (jsub == 0 || jsub == 10) {
            short pk  = *(short *)(param_1 + 0x9d0);                 // PKKey
            double t  = (double)*(int *)(param_1 + 0x9b8)            // lifetime
                      + (double)*(float *)(param_1 + 0x9c8);         // fase random
            double a  = (pk == -1) ? (t * 0.1) : (t / (double)pk);
            *jposX                     = (float)(fcos(a) * 40.0 + (double)tgtX);
            *(float *)(param_1 + 0x14) = (float)((double)tgtY - fsin(a) * 40.0);
            float rise = *(float *)(param_1 + 0x9cc);
            *(float *)(param_1 + 0x18) = (jsub != 0)
                ? (*(float *)(param_1 + 0x18) - rise)
                : (rise + *(float *)(param_1 + 0x18));
            goto switchD_caseD_4ef;
        }
        if (jsub == 14) {
            // 00470030 LABEL_301/case 14.
            int owner = *(int *)(param_1 + 0x40);
            if (owner != 0) {
                *(DWORD *)(param_1 + 0x1c) = *(DWORD *)(owner + 392);
                *(DWORD *)(param_1 + 0x20) = *(DWORD *)(owner + 396);
            }
            const int life = *(int *)(param_1 + 0x9b8);
            const short pk = *(short *)(param_1 + 0x9d0);
            const double t = (double)life + (double)*(float *)(param_1 + 0x9c8);
            const double a = (pk == -1) ? t * 0.1 : t / (double)pk;
            const float approach = (float)(100 - life) * 0.011111111f;
            *jposX = (float)(fcos(a) * 15.0) +
                      approach * tgtX + *(float *)(param_1 + 0x1c);
            *(float *)(param_1 + 0x14) = *(float *)(param_1 + 0x20) -
                                          (float)(fsin(a) * 15.0) + approach * tgtY;
            *(float *)(param_1 + 0x18) += *(float *)(param_1 + 0x9cc);
            if (*(float *)(param_1 + 0x18) >= 500.0f) {
                *(int *)(param_1 + 8) = 1;
                *(int *)(param_1 + 0x9b8) = (life > 10) ? 10 : life;
            }
            goto switchD_caseD_4ef;
        }
        if (jsub == 2 || jsub == 13) {
            if (*(int *)(param_1 + 0x9b8) > 25) goto switchD_caseD_4ef;
            float step = *(float *)(param_1 + 0x9cc) + (jsub == 2 ? 5.0f : 1.0f);
            *jposX                     = tgtX;
            *(float *)(param_1 + 0x14) = tgtY;
            *(float *)(param_1 + 0x9cc) = step;
            *(float *)(param_1 + 0x18) = step + *(float *)(param_1 + 0x18);
            goto switchD_caseD_4ef;
        }
        if (jsub == 5) {
            float step = *(float *)(param_1 + 0x9cc) - 60.0f;
            *jposX                      = tgtX;
            *(float *)(param_1 + 0x14)  = tgtY;
            *(float *)(param_1 + 0x9cc) = step;
            *(float *)(param_1 + 0x18)  = step + *(float *)(param_1 + 0x18);
            *(float *)(param_1 + 0x0c)  = (float)(_rand() % 4) + 6.0f;
            goto switchD_caseD_4ef;
        }
        if (jsub == 6 || jsub == 12) {
            // 00470030 LABEL_301 cases 6 and 12, converging at LABEL_438/439.
            const int life = *(int *)(param_1 + 0x9b8);
            const float phase = *(float *)(param_1 + 0x9c4);
            float lateral;
            float vertical;
            if (jsub == 6) {
                const float radius = (float)((life <= 1) ? 1 : life);
                const double a = (double)life * 0.5 + (double)phase;
                lateral = (float)(-fcos(a) * ((double)radius + (double)radius));
                vertical = (float)(fsin(a) * ((double)radius + (double)radius));
            } else {
                const double a = ((double)life + (double)phase) * 0.1;
                lateral = (float)(fcos(a) * -26.0);
                vertical = (float)(fsin(a) * 26.0 - (double)(90 - life) * 0.3);
            }
            *(float *)(param_1 + 0x44) += *(float *)(param_1 + 0x9c8);
            *(float *)(param_1 + 0x48) += *(float *)(param_1 + 0x9cc);
            const float heading = (90.0f - *(float *)(param_1 + 0x30)) * 0.017453292f;
            *jposX = fsin(heading) * lateral + *(float *)(param_1 + 0x44);
            *(float *)(param_1 + 0x14) = fcos(heading) * lateral + *(float *)(param_1 + 0x48);
            *(float *)(param_1 + 0x18) = vertical + *(float *)(param_1 + 0x4c);
            goto switchD_caseD_4ef;
        }
        // 2026-09-01 FIX — los subtipos 8 y 9 PISABAN `local_30`, que es el
        // `in2` de IDA: la matriz que arma el prologo con
        // `AngleMatrix((float *)(o + 40), in2)` (L319) y que el epilogo
        // LABEL_487 (L2228) le pasa a `sub_46FE90` para construir las 4
        // esquinas del segmento nuevo.  IDA usa una matriz APARTE (`v304`,
        // declarada en L307) en estos dos cases; reusar `in2` dejaba el
        // scroll de segmentos del epilogo con la matriz equivocada.
        float jointMatrix_v304[12];               // IDA: v304[3][4]
        if (jsub == 8) {
            // 00470030 LABEL_301/case 8: rotate the fixed vertical offset
            // by the raw +0x44 vector, anchor it at +0x1c, then advance +0x4c.
            float offset[3] = { 0.0f, -50.0f, 0.0f };   // IDA: in1
            float rotated[3];                           // IDA: Position
            Matrix_BuildFromEuler((float *)(param_1 + 0x44), jointMatrix_v304);
            Vector_Rotate(offset, jointMatrix_v304, rotated);
            *jposX                     = rotated[0] + *(float *)(param_1 + 0x1c);
            *(float *)(param_1 + 0x14) = rotated[1] + *(float *)(param_1 + 0x20);
            *(float *)(param_1 + 0x18) = rotated[2] + *(float *)(param_1 + 0x24);
            Matrix_BuildFromEuler((float *)(param_1 + 0x40), jointMatrix_v304);
            *(float *)(param_1 + 0x4c) += 10.0f;
            goto switchD_caseD_4ef;
        }
        if (jsub == 9) {
            // 00470030 LABEL_301/case 9.
            Matrix_BuildFromEuler((float *)(param_1 + 0x40), jointMatrix_v304);
            const int segments = *(int *)(param_1 + 0x54);   // IDA: *(int *)(o + 84)
            for (int i = 0; i < segments; ++i) {             // IDA: v180 / iIndex
                *jposX                     += *(float *)(param_1 + 0x1c);
                *(float *)(param_1 + 0x14) += *(float *)(param_1 + 0x20);
                *(float *)(param_1 + 0x18) += *(float *)(param_1 + 0x24);
                FUN_0046fe90((int)param_1, jointMatrix_v304);
            }
            goto switchD_caseD_4ef;
        }
        if (jsub == 3) {
            goto switchD_caseD_4ef;   // IDA: case 3 → LABEL_447 (sin movimiento)
        }

        // 2026-09-04 -- PORTADO: subtipos 7 y 11 (IDA 0x00470030 L1302-1682).
        //
        // Estos dos NO caen en la helice generica de mas abajo.  Rehice la traza
        // de anidamiento de LABEL_301 contando llaves: `if (v168 != 7)` cierra en
        // L1301 y la ejecucion sigue en L1302, dentro del bloque de
        // `if (v168 != 12)`, que llega hasta L1683 y termina en `goto LABEL_447`.
        // Solo los subtipos 4 y 12 alcanzan la helice.
        //
        // Es un PROYECTIL que converge sobre el owner:
        //   - la posicion sale de una espiral alrededor de TargetPosition, con
        //     radio que crece de 0 a 150 a medida que baja la vida;
        //   - un bucle de tres pasos (x, y, z) interpola esa posicion hacia el
        //     owner con peso `w`, que decrece con la vida -> se va pegando al
        //     objetivo;
        //   - y emite tres CreateSprite por tick (1231 + dos 1150), que es lo
        //     unico visible del efecto.
        //
        // Es el disparo del ataque de Alquamos: AttackEffect case 0x45 crea ocho
        // 1249/sub7 con el heroe como owner.  Nuestro port los mandaba a la
        // helice, que ni converge ni emite sprites -- de ahi que primero se
        // vieran lineas hacia cualquier lado y, una vez limpia la velocidad
        // heredada del slot, no se viera nada.
        if (jsub == 7 || jsub == 11) {
            const int iIndex = (int)param_2;                 // indice de slot
            const int msf    = (int)DAT_083a7c00;            // MoveSceneFrame
            const int v203   = (iIndex % 2) ? msf : -msf;
            float a1 = 0.061299998f, a2 = 0.1113f, v204 = 0.048f;
            if (jsub == 11) { v204 = 0.071999997f; a1 = 0.091949999f; a2 = 0.16695f; }
            const int    v205 = iIndex + v203 + 53730 * iIndex;
            const double v207 = (double)v204 * (double)(v205 + 55555);
            const int    life = *(int *)(param_1 + 0x9b8);
            const double v209 = fsin(v207);
            const double v210 = (double)v205 * (double)a1;
            const float  out0 = (float)(fcos(v210) * v209);
            const float  out1 = (float)(fsin(v210) * v209);
            const double v211 = fcos(v207);
            const double v212 = (double)(v205 + 11111) * (double)a2;
            const double s212 = fsin(v212), c212 = fcos(v212);
            const float  A1 = (float)(c212 * v211 + s212 * (double)out1);
            const float  A0 = (float)(c212 * (double)out1 - s212 * v211);

            const double v295 = (double)life * 1.3333334;
            // IDA elige aca entre dos formulas segun un flag que Hex-Rays
            // perdio (v216/v219 quedan sin definir: son el resultado de una
            // comparacion x87 que el decompilador no reconstruyo):
            //     sub 11:  v217 = v215 * 4.0   ...o...  v215 + 60.0
            //     resto:   v217 = v215 * 7.0   ...o...  v215 + 60.0
            // El umbral se deduce de que las dos ramas tienen que empalmar sin
            // salto: `v215*7 == v215+60` en v215 == 10, y `v215*4 == v215+60`
            // en v215 == 20.  O sea es una funcion continua a trozos y la
            // condicion perdida es `v215 < 10` (resp. `< 20`).
            //
            // Importa para el gameplay: `v217` define el peso `w` con el que la
            // posicion se interpola hacia el owner, y con solo la rama `+60` el
            // peso nunca baja de ~47, asi que el proyectil se quedaba en ~97%
            // del camino y reventaba justo antes de tocar al jugador.  Con la
            // rama corta, en los ultimos ~7 ticks el peso cae a ~9 y el disparo
            // termina de cerrar sobre el objetivo.
            const double v217 = (jsub == 11)
                              ? ((v295 < 20.0) ? (v295 * 4.0) : (v295 + 60.0))
                              : ((v295 < 10.0) ? (v295 * 7.0) : (v295 + 60.0));
            const int    v294 = *(int *)(param_1 + 0x9d4) + 30;
            const float  w    = (float)(v217 / (double)v294 * 30.0);
            if (life == 30) PlayBuffer(46, 0, 0);

            float v222;
            if (jsub == 11) {
                double v220 = v295 - 10.0;
                const double v221 = (v220 >= 0.0) ? v220 : 0.0;
                if (v221 * 5.0 < 150.0) { if (v220 < 0.0) v220 = 0.0; v222 = (float)(v220 * 5.0); }
                else                    { v222 = 150.0f; }
            } else {
                double v223 = 40.0 - v295;
                const double v224 = (v223 >= 0.0) ? v223 : 0.0;
                if (v224 * 15.0 < 150.0) { if (v223 < 0.0) v223 = 0.0; v222 = (float)(v223 * 15.0); }
                else                     { v222 = 150.0f; }
            }
            *(float *)(param_1 + 0x10) = v222 * A0   + *(float *)(param_1 + 0x44);
            *(float *)(param_1 + 0x14) = v222 * A1   + *(float *)(param_1 + 0x48);
            *(float *)(param_1 + 0x18) = v222 * out0 + *(float *)(param_1 + 0x4c);

            const int jowner = *(int *)(param_1 + 0x40);
            if (jowner) {
                int v226 = 0;
                // 2026-09-04 FIX (la flecha del arco colapsaba en un destello):
                // el subtipo 11 NO converge hacia `owner + 16/20/24` sino hacia
                // `owner + 368/372/376`.  IDA lo escribe ofuscado --
                // `LODWORD(v295) = 352 - o;` y luego
                // `*(float *)(LODWORD(v295) + *(_DWORD *)(o + 64) + v228)` --
                // pero 352 + {16,20,24} = {368,372,376}, que es la posicion
                // DERIVADA del efecto de la flecha: la que fija CreateEffect
                // case 243 (`i+92..94`) 100 unidades por delante y la que usa su
                // propio AddTerrainLight.
                //
                // Con `owner + 16` la cosa se realimenta: el joint con
                // SkillIndex==1 escribe su posicion EN `owner + 16`, asi que
                // convergia hacia si mismo y los cuatro joints se apelotonaban
                // en el punto de salida -- el destello sin estela.
                const int srcBase = (jsub == 11) ? 352 : 0;
                for (int v228 = 16; v228 < 28; v228 += 4) {
                    const int    k    = iIndex + v226 + 51230 * iIndex + msf / 10;
                    const double v229 = fcos((double)k * 0.0099999998) * 25.0
                                      + (double)*(float *)(jowner + srcBase + v228);
                    float *dst = (float *)(param_1 + v228);
                    *dst = (float)(((double)w * (double)*dst
                                    + v229 * (100.0 - (double)w)) * 0.0099999998);
                    v226 += 3711;
                }
            }
            if (jsub != 11) *(float *)(param_1 + 0x18) += 100.0f;

            float jangles[3] = { 0.5f, 0.5f, 1.0f };
            if (jsub == 11) {
                jangles[0] = 0.30000001f; jangles[1] = 0.30000001f; jangles[2] = 0.5f;
                if (*(unsigned char *)(param_1 + 0x9d2) == 1 && jowner) {
                    *(int *)(jowner + 16) = *(int *)(param_1 + 0x10);
                    *(int *)(jowner + 20) = *(int *)(param_1 + 0x14);
                    *(int *)(jowner + 24) = *(int *)(param_1 + 0x18);
                    FUN_00466440(jowner);
                }
            }
            {
                float *ppos = (float *)(param_1 + 0x10);
                FUN_004795c0(1231, ppos, (float)(_rand() % 2 + 8) * 0.1f,
                             jangles, jowner, (float)(_rand() % 360), 0);
                FUN_004795c0(1150, ppos, (float)(_rand() % 2 + 8) * 0.18000001f,
                             jangles, jowner, (float)(_rand() % 360), 0);
                FUN_004795c0(1150, ppos, (float)(_rand() % 2 + 8) * 0.18000001f,
                             jangles, jowner, (float)(_rand() % 360), 0);
                if (jsub != 11 && *(int *)(param_1 + 0x9b8) == 1) {
                    float jzero[3]  = { 0.0f, 0.0f, 0.0f };
                    float jwhite[3] = { 1.0f, 1.0f, 1.0f };
                    Particle_Spawn(1215, ppos, jzero, jwhite, 1, 0.60000002f, 0);
                }
            }
            goto _skipLabel182;   // IDA: LABEL_447 -> LABEL_487, saltea LABEL_182
        }
    }

    // 2026-09-01 — PORTADA la cola real de LABEL_301 (IDA L1706-1724 ->
    // LABEL_438 -> LABEL_439).  Aca habia una "orbita pseudo-aleatoria"
    // INVENTADA (sembrada con el indice de slot y unos globals de ruido) que
    // ademas llamaba `FUN_0046fe90` de mas: el epilogo LABEL_487 ya lo llama,
    // asi que se scrolleaba la historia de segmentos DOS veces por tick — el
    // anillo avanzaba al doble y quedaban segmentos duplicados/entrelazados.
    //
    // Lo que hace el binario para los subtipos que no matchean ningun case
    // (4, 7, 11 y el resto) es la MISMA helice que 6 y 12, con otra formula de
    // radio:
    //     v237 = *(_DWORD *)(o + 2488);                       // life
    //     v238 = ((double)v237 + *(float *)(o + 2500)) * 0.1;  // angulo
    //     v240 = v237 + 40;  v296 = (v240 <= 10) ? 10 : v240;  // radio
    //     v234 = -(cos(v238) * (v296 * 0.64999998));
    //     v236 =   v296 * 0.64999998;  v235 = sin(v238);
    //   LABEL_438: v239 = v235 * v236;
    //   LABEL_439: <ancla + heading + escritura de Pos>
    {
        const int life = *(int *)(param_1 + 0x9b8);              // IDA: v237
        const float phase = *(float *)(param_1 + 0x9c4);         // IDA: o + 2500
        const double a = ((double)life + (double)phase) * 0.1;   // IDA: v238
        int radius = life + 40;                                  // IDA: v240
        if (radius <= 10) radius = 10;                           // IDA: v296 = 10
        const double r65 = (double)radius * 0.64999998;
        const float lateral  = (float)(-(fcos(a) * r65));        // IDA: v234
        const float vertical = (float)(fsin(a) * r65);           // IDA: v239 = v235 * v236

        // IDA LABEL_439 (compartido con los subtipos 6 y 12).
        *(float *)(param_1 + 0x44) += *(float *)(param_1 + 0x9c8);
        *(float *)(param_1 + 0x48) += *(float *)(param_1 + 0x9cc);
        const float heading = (90.0f - *(float *)(param_1 + 0x30)) * 0.017453292f;
        *(float *)(param_1 + 0x10) = fsin(heading) * lateral + *(float *)(param_1 + 0x44);
        *(float *)(param_1 + 0x14) = fcos(heading) * lateral + *(float *)(param_1 + 0x48);
        *(float *)(param_1 + 0x18) = vertical + *(float *)(param_1 + 0x4c);
        goto switchD_caseD_4ef;
    }

    // ─────────────────────────────────────────────────────────────────
    // COMMON EPILOGUE
    // ─────────────────────────────────────────────────────────────────

switchD_caseD_4ef:
    // Segment tick — excluded for types 0x4e6+submode6, 0x4ee, 0x4e7(lifetime>=10)
    // IDA 00470030 LABEL_182 (comun a TODOS los tipos, justo antes del
    // decremento de vida): los joints de SubType 7 re-anclan su Position a la
    // TargetPosition guardada en o+68/72/76.
    //
    //     if ( *(_DWORD *)(o + 8) == 7 ) {
    //         v109 = *(_DWORD *)(o + 72);  v110 = *(_DWORD *)(o + 76);
    //         *v2 = *(float *)(o + 68);            // v2 = (float *)(o + 16)
    //         *(_DWORD *)(o + 20) = v109;  *(_DWORD *)(o + 24) = v110;
    //     }
    //
    // Faltaba entero. La cadena de 13 joints 1254 de Queen Rainer
    // (MoveCharacterVisual case 0x141) se crea con SubType 7, asi que sus
    // segmentos nacian y despues no seguian al hueso: las lineas quedaban
    // sueltas en el aire en vez de acompanar al monstruo.
    if (*(int *)(param_1 + 8) == 7) {
        *(float *)(param_1 + 0x10) = *(float *)(param_1 + 0x44);
        *(float *)(param_1 + 0x14) = *(float *)(param_1 + 0x48);
        *(float *)(param_1 + 0x18) = *(float *)(param_1 + 0x4c);
    }
_skipLabel182:;

    // 2026-09-04 -- ORDEN CORREGIDO.  En IDA el epilogo es
    //     LABEL_182 (re-ancla SubType 7)  ->  LABEL_487 (scroll de segmentos)
    // y aca estaban al reves.  Con el orden invertido, el `goto _skipLabel182`
    // que usa el bloque de los subtipos 7/11 -- que en IDA salta LABEL_182 pero
    // SI pasa por LABEL_487 -- terminaba salteando tambien el scroll, asi que
    // esos joints nunca acumulaban segmentos y su cinta no se dibujaba.  Es la
    // estela de la flecha del arco (cuatro 1249/sub11 que crea CreateEffect
    // case 243) y la cadena de Queen Rainer.
    iVar16 = *(int *)(param_1 + 4);
    if (((iVar16 != 0x4e6) || (*(int *)(param_1 + 8) != 6)) &&
        ((iVar16 != 0x4ee) &&
         ((iVar16 != 0x4e7) || (10 < *(int *)(param_1 + 0x9b8)))))
    {
        FUN_0046fe90((int)param_1, local_30);
    }

    // Decrement lifetime counter
    const int remainingLife = *(int *)(param_1 + 0x9b8) - 1;
    *(int *)(param_1 + 0x9b8) = remainingLife;
    if (remainingLife >= 0) {
        const int type = *(int *)(param_1 + 4);
        const int subtype = *(int *)(param_1 + 8);
        if ((remainingLife % 12) != 0 && (type == 1249 || type == 1277) &&
            (subtype == 4 || subtype == 12)) {
            return FUN_00470030(param_1, param_2);
        }
        return (char *)(intptr_t)remainingLife;
    }
    *param_1 = 0;
    return (char *)(intptr_t)remainingLife;
#if 0 // Former decompiler-artifact formulation of the same epilogue.
    pcVar11 = (char *)(*(int *)(param_1 + 0x9b8) + -1);
    *(int *)(param_1 + 0x9b8) = (int)(intptr_t)pcVar11;

    if (-1 < (int)(intptr_t)pcVar11) {
        // Recursive self-call on types 0x4e1/0x4fd at sub-frame boundaries (modes 4 or 0xc)
        pcVar12 = (char *)((int)(intptr_t)pcVar11 / 0xc);
        if ((((int)(intptr_t)pcVar11 % 0xc != 0) &&
             ((*(int *)(param_1 + 4) == 0x4e1) ||
              ((pcVar12 = (char *)(*(int *)(param_1 + 4) + -0x4fd),
                pcVar12 == (char *)0x0)))) &&
            ((pcVar12 = *(char **)(param_1 + 8),
              (pcVar12 == (char *)0x4) || (pcVar12 == (char *)0xc))))
        {
            pcVar12 = FUN_00470030(param_1, param_2);
        }
        return pcVar12;
    }

    // Lifetime expired — deactivate slot
    *param_1 = 0;
    return pcVar11;
#endif
}

// end of FUN_00470030
