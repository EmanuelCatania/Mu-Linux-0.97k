// MoveJoint.cpp
// MoveJoint @ 0x00470030  [Kayito: MoveJoint]
//
// Per-frame movement/animation tick for "joint" effect slots.
// Joint slots are chained lightning/beam/special-link effects.
// Called by MoveJoints (MoveJoints) once per active joint slot per frame.
//
// param_1 = JOINT* (Render/JointTypes.h), como puntero a byte
// param_2 = uint     — índice del joint en el pool; semilla de las oscilaciones
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
// COLA GENERICA de MoveJoint (IDA 0x00470030 L1922-2217)
//
// En el binario, todo joint cuyo TIPO no matchea ningun bloque del dispatch cae
// en esta cola.  Es la que CONSTRUYE la estela: recorre `segMax` pasos y en cada
// uno mueve el joint con `MoveHumming` hacia su objetivo y empuja un segmento
// con `sub_46FE90` -- o sea el rayo entero se dibuja en UN tick.
//
// La usan el rayo del Lightning (1254 sub 0), los rayos del Twister (1253 sub 0,
// ocho joints) y los rayos de fondo de Icarus (1254 sub 6, que ademas es el
// unico subtipo que el epilogo LABEL_487 excluye del tick de segmentos
// justamente porque los construye aca).
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

    const int sub0 = AsJoint(o)->SubType;

    // -- IDA L1922-1996: ajuste previo por subtipo ----------------------------
    if ((sub0 != 6 || AsJoint(o)->LifeTime <= 4) && sub0 != 8) {
        bool toLabel126 = false;
        if (sub0 == 4 || sub0 == 5) {
            const float tx = tgt[0], ty = tgt[1], tz = tgt[2];
            pos[0] = tx; pos[1] = ty; pos[2] = tz;
            Position[0] = tx; Position[1] = ty; Position[2] = tz + 30.0f;
            const float rot = (float)(rand() % 360);
            const float sc  = (float)(rand() % 8 + 8) * 0.2f;
            CreateSprite(1231, Position, sc, (float *)(o + 52), 0, rot, 0);
            const int life = AsJoint(o)->LifeTime;
            if (life > 10) {
                Light[0] = 0.5f; Light[1] = 0.5f; Light[2] = 0.5f;
                if ((life & 1) == 0) {
                    Angle[0] = (float)(rand() % 60 - 30) + AsJoint(o)->Angle[0];
                    Angle[1] = 0.0f                      + AsJoint(o)->Angle[1];
                    Angle[2] = (float)(rand() % 30 + 90) + AsJoint(o)->Angle[2];
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
            pos[0] = AsJoint(o)->StartPosition[0];
            pos[1] = AsJoint(o)->StartPosition[1];
            pos[2] = AsJoint(o)->StartPosition[2];
        }
    }

    // -- IDA L1996-2210: el bucle que construye la estela ---------------------
    const int segMax = AsJoint(o)->MaxTails;
    if (segMax <= 0) return;

    float v295 = 0.0f;          // distancia devuelta por MoveHumming
    int   step = 0;
    for (;;) {
        const int sub = AsJoint(o)->SubType;

        switch (sub) {
        case 0: case 1: case 2: {
            const int owner = *(int *)&AsJoint(o)->Target;
            if (owner) {
                tgt[0] = *(float *)(owner + 16);
                tgt[1] = *(float *)(owner + 20);
                tgt[2] = *(float *)(owner + 24) + 80.0f;
            }
            v295 = MoveHumming(pos, (float *)(o + 40), tgt, 25.0f);   // LABEL_131
            break;
        }
        case 3:
            v295 = MoveHumming(pos, (float *)(o + 40), tgt, 50.0f);
            break;
        case 4: case 5: {
            Position[0] = tgt[0]; Position[1] = tgt[1]; Position[2] = tgt[2] - 300.0f;
            v295 = MoveHumming(pos, (float *)(o + 40), Position, -10.0f);
            break;
        }
        case 6: {
            const int owner = *(int *)&AsJoint(o)->Target;
            if (owner) {
                tgt[0] = *(float *)(owner + 16);
                tgt[1] = *(float *)(owner + 20);
                tgt[2] = *(float *)(owner + 24);
            }
            tgt[0] += (float)(rand() % 200) + 2050.0f;
            tgt[1] += (float)(rand() % 200) + 2050.0f;
            tgt[2] -= 10000.0f;
            v295 = MoveHumming(pos, (float *)(o + 40), tgt, (float)(rand() % 100 + 50));
            break;
        }
        case 7:
            v295 = MoveHumming(pos, (float *)(o + 40), tgt, (float)(rand() % 100 + 50));
            break;
        case 9:
            v295 = MoveHumming(pos, (float *)(o + 40), Position, (float)(rand() % 80 + 60));
            break;
        case 0xC: {
            // IDA entra directo a LABEL_133: no recalcula la distancia.
            const int owner = *(int *)&AsJoint(o)->Target;
            if (owner) {
                const short mdl = *(short *)(owner + 2);
                float out[3] = { 0.0f, -100.0f, 0.0f };
                BMD_TransformPosition((void *)((uintptr_t)DAT_05828d58 + 188 * mdl),
                                      (float *)&DAT_07abf474, out, pos, '\x01');
            }
            break;
        }
        default:
            v295 = MoveHumming(pos, (float *)(o + 40), tgt, 25.0f);    // LABEL_131
            break;
        }

        // -- LABEL_133: jitter del angulo por subtipo -------------------------
        const int sub2 = AsJoint(o)->SubType;
        bool toLabel151 = false;
        float v92 = 0.0f;
        if (sub2 == 1) {
            AsJoint(o)->Direction[0] = (float)(rand() % 256 - 128);
            v92 = (float)(rand() % 256 - 128);
        } else if (sub2 == 4) {
            AsJoint(o)->Direction[0] = (float)(rand() % 64 - 32);
            toLabel151 = true;
        } else if (sub2 == 6 || sub2 == 7) {
            AsJoint(o)->Direction[0] = (float)(rand() % 100 + 20);
            v92 = (float)(rand() % 100 + 20);
        } else if (sub2 == 11) {
            const float sc = AsJoint(o)->Scale;
            AsJoint(o)->Direction[0] = (float)((double)(rand() % 1024 - 512) * 0.69999999) / sc;
            v92 = (float)((double)(rand() % 1024 - 512) * 0.69999999) / sc;
        } else {
            const float sc = AsJoint(o)->Scale;
            AsJoint(o)->Direction[0] = (float)(rand() % 1024 - 512) / sc;
            v92 = (float)(rand() % 1024 - 512) / sc;
        }
        if (!toLabel151) AsJoint(o)->Direction[2] = v92;

        // -- LABEL_151: empujar un segmento -----------------------------------
        Angle[0] = AsJoint(o)->Angle[0] + AsJoint(o)->Direction[0];
        Angle[1] = AsJoint(o)->Angle[1] + AsJoint(o)->Direction[1];
        Angle[2] = AsJoint(o)->Angle[2] + AsJoint(o)->Direction[2];
        Matrix_BuildFromEuler(Angle, v305);
        Joint_SegmentTick((int)(uintptr_t)o, v305);

        const int sub3 = AsJoint(o)->SubType;
        if (sub3 == 3) {
            if (v295 > 150.0f) AsJoint(o)->LifeTime = 0;
        } else if (AsJoint(o)->Velocity * 1.5f > v295 &&
                   sub3 != 6 && sub3 != 9 && sub3 != 7) {
            if (*(int *)&AsJoint(o)->Scale == 1112014848) {           // Scale == 50.0f
                Particle_Spawn(1180, pos, (float *)(o + 40), (float *)(o + 52), 0, 1.0f, 0);
                if ((rand() % 8) == 0) {
                    angles[0] = (float)(rand() % 64 - 32) + tgt[0];
                    angles[1] = (float)(rand() % 64 - 32) + tgt[1];
                    angles[2] = (float)(rand() % 64 - 32) + tgt[2];
                    Particle_Spawn(1220, pos, (float *)(o + 40), (float *)(o + 52), 0, 1.0f, 0);
                }
                if (AsJoint(o)->SubType == 0 && ((long long)WorldTime) % 1000 < 500) {
                    if ((rand() % 16) == 0) {
                        angles[0] = (float)(rand() % 100 - 50) + tgt[0];
                        angles[1] = (float)(rand() % 100 - 50) + tgt[1];
                        angles[2] = (float)(rand() % 120 - 60) + tgt[2];
                        Joint_Create(1254, angles, tgt, (float *)(o + 40), 1,
                                     *(int *)&AsJoint(o)->Target, (float)(rand() % 8) + 6.0f, -1, 0);
                    }
                }
            }
            break;                                          // IDA: goto LABEL_182
        }

        if (AsJoint(o)->Scale >= 50.0f && sub3 != 4 && sub3 != 7 &&
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
            AddTerrainLight(pos[0], pos[1], (float*)Light, 2, (float*)PrimaryTerrainLight[0]);
        }

        Position[0] = 0.0f;
        Position[1] = -AsJoint(o)->Velocity;
        Position[2] = 0.0f;
        Vector_Rotate(Position, v305, in1);
        pos[0] += in1[0];
        pos[1] += in1[1];
        pos[2] += in1[2];
        if (++step >= segMax) break;
    }

    // -- LABEL_182 ------------------------------------------------------------
    if (AsJoint(o)->SubType == 7) {
        pos[0] = tgt[0]; pos[1] = tgt[1]; pos[2] = tgt[2];
    }
}

char * __cdecl MoveJoint(undefined1 *param_1, uint param_2)
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
    // Estos "locales" que produjo Ghidra son en realidad UN bloque contiguo del
    // frame original (ebp-0xD8 .. ebp), y el código de abajo depende de esa
    // contigüidad — MSVC no la garantiza:
    //   L99  Vector_Rotate(&local_b4, local_30, local_d8 + 6)  → escribe el vec3
    //        de salida en local_d8[6],[7],[8]; L102 lee out[2] como `local_b8`,
    //        o sea `local_d8[8] == local_b8`.
    //   L138 Vector_Rotate(local_d8 + 6, local_a8 + 6, &local_b4) → L140/L141
    //        leen out[1] y out[2] como `local_b0` y `local_ac`, o sea
    //        `(&local_b4)[1] == local_b0` y `(&local_b4)[2] == local_ac`.
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
    local_dc_f = AsJoint(param_1)->Position[0] - AsJoint(param_1)->TargetPosition[0];
    pfVar15    = AsJoint(param_1)->Position;
    pfVar14    = AsJoint(param_1)->TargetPosition;
    local_e4_f = AsJoint(param_1)->Position[1] - AsJoint(param_1)->TargetPosition[1];

    // Default movement: rotate velocity by euler matrix and apply to position.
    // Type 0x4ee overrides this entirely (has its own path below).
    if (AsJoint(param_1)->Type != 0x4ee) {
        Matrix_BuildFromEuler(AsJoint(param_1)->Angle, local_30);
        local_b0 = -AsJoint(param_1)->Velocity;
        local_b4 = 0.0f;
        local_ac = 0.0f;
        Vector_Rotate(&local_b4, local_30, local_d8 + 6);
        *pfVar15                        = local_d8[6] + *pfVar15;
        AsJoint(param_1)->Position[1]      = local_d8[7] + AsJoint(param_1)->Position[1];
        AsJoint(param_1)->Position[2]      = local_b8   + AsJoint(param_1)->Position[2];
    }

    iVar16 = AsJoint(param_1)->Type;

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
        // FPU que Hex-Rays no tipa; se usa el valor de retorno de MoveHumming.
        const float dist_4e8 =
            MoveHumming(pfVar15, AsJoint(param_1)->Angle, pfVar14, 0.0f);
        float matrix_4e8[12];
        Matrix_BuildFromEuler(AsJoint(param_1)->Angle, matrix_4e8);
        Joint_SegmentTick((int)param_1, matrix_4e8);
        if (dist_4e8 > AsJoint(param_1)->Velocity) {
            const float brightness = (float)(rand() % 4 + 4) * 0.1f;
            float light_4e8[3] = { brightness, brightness * 0.6f, brightness * 0.2f };
            AddTerrainLight(*pfVar15, AsJoint(param_1)->Position[1], (float*)light_4e8, 4, (float*)&DAT_081cb608[0]);
            float step_4e8[3] = { 0.0f, -AsJoint(param_1)->Velocity, 0.0f };
            float rotated_4e8[3];
            Vector_Rotate(step_4e8, matrix_4e8, rotated_4e8);
            *pfVar15 += rotated_4e8[0];
            AsJoint(param_1)->Position[1] += rotated_4e8[1];
            AsJoint(param_1)->Position[2] += rotated_4e8[2];
        } else {
            *param_1 = 0;
            Particle_Spawn(1215, pfVar15, AsJoint(param_1)->Angle,
                         AsJoint(param_1)->Light, 0, 1.0f, 0);
        }
#if 0 // Former approximation, retained only as source history.
        float *pfVar26 = AsJoint(param_1)->Angle;
        // MoveHumming computes angle/distance and returns result via pfOut;
        // Ghidra shows its return used as float — call with a temp output.
        float arc_out = 0.0f;
        MoveHumming(pfVar15, pfVar26, pfVar14, 0.0f);  // result in pfVar14[2] area; use arc_out
        // In the decompile: local_e4 = (undefined4*)(float)fVar24 where fVar24 = MoveHumming(...)
        // The function signature is void in functions.h so we approximate the distance check
        // using AsJoint(param_1)->Velocity vs the 3D distance already computed above.
        local_e4_f = sqrtf(local_dc_f * local_dc_f + local_e4_f * local_e4_f);
        Matrix_BuildFromEuler(pfVar26, local_a8 + 6);
        Joint_SegmentTick((int)param_1, local_a8 + 6);
        if (AsJoint(param_1)->Velocity < local_e4_f) {
            uVar7 = _rand();
            uVar7 = uVar7 & 0x80000003;
            if ((int)uVar7 < 0) {
                uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
            }
            local_e8_f = (float)(int)(uVar7 + 4);
            local_d8[3] = local_e8_f * _DAT_005524f4;
            local_d8[4] = local_d8[3] * _DAT_00552534;
            local_d8[5] = local_d8[3] * _DAT_005526e4;
            AddTerrainLight(*pfVar15, AsJoint(param_1)->Position[1], (float*)((local_d8 + 3)), 4, (float*)&DAT_081cb608[0]);
            local_d8[7] = -AsJoint(param_1)->Velocity;
            local_d8[6] = 0.0f;
            local_b8    = 0.0f;
            Vector_Rotate(local_d8 + 6, local_a8 + 6, &local_b4);
            *pfVar15                   = local_b4 + *pfVar15;
            AsJoint(param_1)->Position[1] = local_b0 + AsJoint(param_1)->Position[1];
            AsJoint(param_1)->Position[2] = local_ac + AsJoint(param_1)->Position[2];
        }
        else {
            *param_1 = 0;
            Particle_Spawn(0x4bf, pfVar15, pfVar26, AsJoint(param_1)->Light, 0, 1.0f, 0);
        }
#endif
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4e2: Chain/rope follower ──────────────────────────────
    if (iVar16 == 0x4e2) {
        // IDA case 1250: three interpolated anchor steps, each offset by the
        // rotating vector stored at +0x9c4.  Keep the joint rotation matrix
        // separate from the entity angle matrix, as in the original.
        char *owner_4e2 = ((char *)AsJoint(param_1)->Target);
        if (*owner_4e2 == '\0') {
            AsJoint(param_1)->LifeTime = -1;
            *pfVar15 = AsJoint(param_1)->TargetPosition[0];
            AsJoint(param_1)->Position[1] = AsJoint(param_1)->TargetPosition[1];
            AsJoint(param_1)->Position[2] = AsJoint(param_1)->TargetPosition[2] + 50.0f;
        } else {
            float delta_4e2[3] = {
                *(float *)(owner_4e2 + 0x10) - *(float *)(owner_4e2 + 0x170),
                *(float *)(owner_4e2 + 0x14) - *(float *)(owner_4e2 + 0x174),
                *(float *)(owner_4e2 + 0x18) - *(float *)(owner_4e2 + 0x178)
            };
            AsJoint(param_1)->TargetPosition[0] = *(float *)(owner_4e2 + 0x170);
            AsJoint(param_1)->TargetPosition[1] = *(float *)(owner_4e2 + 0x174);
            AsJoint(param_1)->TargetPosition[2] = *(float *)(owner_4e2 + 0x178);
            float entityAngle_4e2[3] = { *(float *)(owner_4e2 + 0x28), *(float *)(owner_4e2 + 0x2c), *(float *)(owner_4e2 + 0x30) };
            float jointMatrix_4e2[12];
            Matrix_BuildFromEuler(AsJoint(param_1)->Angle, jointMatrix_4e2);
            delta_4e2[0] *= 1.0f / 3.0f;
            delta_4e2[1] *= 1.0f / 3.0f;
            delta_4e2[2] *= 1.0f / 3.0f;
            entityAngle_4e2[1] += AsJoint(param_1)->Velocity - 90.0f;
            for (int segment_4e2 = 0; segment_4e2 < 3; ++segment_4e2) {
                entityAngle_4e2[1] += 30.0f;
                AsJoint(param_1)->TargetPosition[0] += delta_4e2[0];
                AsJoint(param_1)->TargetPosition[1] += delta_4e2[1];
                AsJoint(param_1)->TargetPosition[2] += delta_4e2[2];
                float entityMatrix_4e2[12], rotated_4e2[3];
                Matrix_BuildFromEuler(entityAngle_4e2, entityMatrix_4e2);
                Vector_Rotate(AsJoint(param_1)->Direction, entityMatrix_4e2, rotated_4e2);
                *pfVar15 = rotated_4e2[0] + AsJoint(param_1)->TargetPosition[0];
                AsJoint(param_1)->Position[1] = rotated_4e2[1] + AsJoint(param_1)->TargetPosition[1];
                AsJoint(param_1)->Position[2] = rotated_4e2[2] + AsJoint(param_1)->TargetPosition[2];
                if (AsJoint(param_1)->NumTails < AsJoint(param_1)->MaxTails - 1 || AsJoint(param_1)->Skill != 0)
                    Joint_SegmentTick((int)param_1, jointMatrix_4e2);
            }
            if (AsJoint(param_1)->LifeTime < 15) {
                AsJoint(param_1)->Light[0] *= 2.0f / 3.0f;
                AsJoint(param_1)->Light[1] *= 2.0f / 3.0f;
                AsJoint(param_1)->Light[2] *= 2.0f / 3.0f;
            }
        }
#if 0 // Former non-IDA reconstruction.
        pcVar11 = ((char *)AsJoint(param_1)->Target);
        if (*pcVar11 == '\0') {
            // Linked entity is dead — snap to anchor + height offset, kill slot
            float fVar28 = AsJoint(param_1)->TargetPosition[2] + _DAT_00552598;
            *(unsigned int *)&AsJoint(param_1)->LifeTime = 0xffffffff;
            *pfVar15                            = *pfVar14;
            *(undefined4 *)&AsJoint(param_1)->Position[1]    = *(undefined4 *)&AsJoint(param_1)->TargetPosition[1];
            AsJoint(param_1)->Position[2]          = fVar28;
            goto switchD_caseD_4ef;
        }
        // Compute target velocity delta (current_pos - prev_pos from entity +0x10/0x170)
        local_d8[0] = *(float *)(pcVar11 + 0x10)  - *(float *)(pcVar11 + 0x170);
        local_d8[1] = *(float *)(pcVar11 + 0x14)  - *(float *)(pcVar11 + 0x174);
        local_d8[2] = *(float *)(pcVar11 + 0x18)  - *(float *)(pcVar11 + 0x178);
        *pfVar14                         = *(float *)(pcVar11 + 0x170);
        *(undefined4 *)&AsJoint(param_1)->TargetPosition[1] = *(undefined4 *)(pcVar11 + 0x174);
        *(undefined4 *)&AsJoint(param_1)->TargetPosition[2] = *(undefined4 *)(pcVar11 + 0x178);
        local_a8[3] = *(float *)(pcVar11 + 0x28);
        local_a8[4] = *(float *)(pcVar11 + 0x2c);
        local_a8[5] = *(float *)(pcVar11 + 0x30);
        Matrix_BuildFromEuler(AsJoint(param_1)->Angle, local_60);
        local_d8[0] *= _DAT_005529d4;
        iVar16       = 3;
        local_d8[1] *= _DAT_005529d4;
        local_d8[2] *= _DAT_005529d4;
        local_a8[4] = (AsJoint(param_1)->Velocity - _DAT_00552848) + local_a8[4];
        do {
            local_a8[4] += _DAT_0055284c;
            AsJoint(param_1)->TargetPosition[0] += local_d8[0];
            AsJoint(param_1)->TargetPosition[1] += local_d8[1];
            AsJoint(param_1)->TargetPosition[2] += local_d8[2];
            Matrix_BuildFromEuler(local_a8 + 3, local_a8 + 6);
            Vector_Rotate(AsJoint(param_1)->Direction, local_a8 + 6, local_a8);
            *pfVar15                        = local_a8[0] + AsJoint(param_1)->TargetPosition[0];
            AsJoint(param_1)->Position[1]      = local_a8[1] + AsJoint(param_1)->TargetPosition[1];
            AsJoint(param_1)->Position[2]      = local_a8[2] + AsJoint(param_1)->TargetPosition[2];
            if ((AsJoint(param_1)->NumTails < AsJoint(param_1)->MaxTails + -1) ||
                (*(char *)&AsJoint(param_1)->Skill != '\0'))
            {
                Joint_SegmentTick((int)param_1, local_60);
            }
            iVar16--;
        } while (iVar16 != 0);
        if (0xe < AsJoint(param_1)->LifeTime) goto switchD_caseD_4ef;
        AsJoint(param_1)->Light[0] *= _DAT_00552a8c;
        AsJoint(param_1)->Light[1] *= _DAT_00552a8c;
        {
            float fVar28 = AsJoint(param_1)->Light[2] * _DAT_00552a8c;
            AsJoint(param_1)->Light[2] = fVar28;
        }
#endif
        goto switchD_caseD_4ef;
    }

    // ── Type 0x498: Quick fade-out ────────────────────────────────────
    if (iVar16 == 0x498) {
        if (AsJoint(param_1)->SubType == 0) {
            float fVar28 = AsJoint(param_1)->Direction[2] + _DAT_00552660;
            *pfVar15                         = *pfVar14;
            *(undefined4 *)&AsJoint(param_1)->Position[1]  = *(undefined4 *)&AsJoint(param_1)->TargetPosition[1];
            AsJoint(param_1)->Direction[2]      = fVar28;
            AsJoint(param_1)->Position[2]       = fVar28 + AsJoint(param_1)->Position[2];
        }
        if (4 < AsJoint(param_1)->LifeTime) goto switchD_caseD_4ef;
        AsJoint(param_1)->Light[0] *= _DAT_00552a90;
        AsJoint(param_1)->Light[1] *= _DAT_00552a90;
        {
            float fVar28 = AsJoint(param_1)->Light[2] * _DAT_00552a90;
            goto LAB_00473578_498;
LAB_00473578_498:
            AsJoint(param_1)->Light[2] = fVar28;
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x10a: Wispy follower ────────────────────────────────────
    if (iVar16 == 0x10a) {
        iVar16 = AsJoint(param_1)->SubType;

        if (iVar16 == 2) {
            // Mode 2: interpolate towards bone target position
            if ((((char *)AsJoint(param_1)->Target) == (char *)0x0) ||
                (*((char *)AsJoint(param_1)->Target) == '\0'))
            {
                *(undefined4 *)&AsJoint(param_1)->Target = 0;
            }
            else {
                AsJoint(param_1)->Scale =
                    (float)AsJoint(param_1)->LifeTime * _DAT_00552540;
                // IDA L898-913: rampa de 1 a 0 en los últimos 10 ticks de vida;
                // el joint viaja del punto de partida al de reunión.
                float t_raw = (float)(AsJoint(param_1)->LifeTime + -10) * _DAT_00552a00;
                float t;
                if (t_raw >= _DAT_0055256c)      t = _DAT_0055256c;   // 1.0
                else if (t_raw >= _DAT_00552580) t = t_raw;           // 0.0 <= t < 1.0
                else                             t = 0.0f;
                float inv_t = _DAT_0055256c - t;
                Ring_ComputeOrbit(param_2 * 0x4539, local_d8, 1.4f);
                iVar16 = *(int *)&AsJoint(param_1)->Target;
                local_d8[0] = local_d8[0] * _DAT_00552900 + *pfVar14;
                local_d8[1] = local_d8[1] * _DAT_00552900 + AsJoint(param_1)->TargetPosition[1];
                local_d8[2] = local_d8[2] * _DAT_00552900 + AsJoint(param_1)->TargetPosition[2];
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
            if (*((char *)AsJoint(param_1)->Target) == '\0') {
                goto LAB_0047036e;
            }
            int rand_roll = _rand();
            int new_lifetime;
            if (rand_roll % 5 == 0) {
                new_lifetime = AsJoint(param_1)->LifeTime + -5;
LAB_004702ce:
                AsJoint(param_1)->LifeTime = new_lifetime;
            }
            else {
                uVar7 = _rand();
                uVar7 = uVar7 & 0x80000001;
                bVar21 = (uVar7 == 0);
                if ((int)uVar7 < 0) {
                    bVar21 = ((uVar7 - 1 | 0xfffffffe) == 0xffffffff);
                }
                if (bVar21) {
                    new_lifetime = AsJoint(param_1)->LifeTime + 1;
                    goto LAB_004702ce;
                }
            }
            iVar16 = *(int *)&AsJoint(param_1)->Target;
            fVar22 = fsin(((double)AsJoint(param_1)->LifeTime -
                           (double)AsJoint(param_1)->Direction[1]) * (double)_DAT_00552874);
            fVar24 = (double)_DAT_00552540;
            AsJoint(param_1)->Direction[0] = (float)(fVar22 * fVar24);
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
                AsJoint(param_1)->Scale = fVar28;
            }
            goto switchD_caseD_4ef;
        }

        // Modes 0, 1, 4: orbit / spiral with trig
        if (*((char *)AsJoint(param_1)->Target) == '\0') {
LAB_0047036e:
            *(undefined4 *)&AsJoint(param_1)->LifeTime = 0;
            *param_1 = 0;
            goto switchD_caseD_4ef;
        }
        if (iVar16 == 4) {
            if (((*(unsigned char *)(*(int *)&AsJoint(param_1)->Target + 0x78)) & 8) != 8)
                goto LAB_0047036e;
            *(undefined4 *)&AsJoint(param_1)->LifeTime = 100;
        }

        // Re-anclaje de la historia de segmentos por el delta del ancla: la
        // estela acompana al owner cuando se mueve.  ESTA en el binario (el
        // decompile lo pliega dentro de un `if`): resta en 0x004703A6 y suma en
        // 0x0047040C, verificado con el disassembly.  Sin estos bucles el aura
        // se deshace en tiras sueltas al caminar.
        if ((iVar16 == 0) || (iVar16 == 4)) {
            // Subtract anchor from segment positions (un-translate)
            // Cubre segCount+1 filas, acotado a segMax: el renderer (0x00473710) dibuja
            // segCount quads leyendo las filas segIdx y segIdx+1, o sea llega hasta la
            // fila **segCount**, y `sub_46FE90` tambien escribe esa fila.
            int seg_rows = AsJoint(param_1)->NumTails + 1;
            {
                const int seg_max = AsJoint(param_1)->MaxTails;
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
                        *pf    -= AsJoint(param_1)->TargetPosition[1];
                        pf[1]  -= AsJoint(param_1)->TargetPosition[2];
                        pf += 3;
                    } while (iVar13 != 0);
                    pfVar26  += -0xc;
                    seg_rows--;
                } while (seg_rows != 0);
            }
        }

        // Update anchor from linked entity current position
        iVar16   = *(int *)&AsJoint(param_1)->Target;
        *pfVar14 = *(float *)(iVar16 + 0x10);
        *(undefined4 *)&AsJoint(param_1)->TargetPosition[1] = *(undefined4 *)(iVar16 + 0x14);
        AsJoint(param_1)->TargetPosition[2]      = *(float *)(iVar16 + 0x18) + _DAT_00552488;

        iVar16 = AsJoint(param_1)->SubType;

        if ((iVar16 == 0) || (iVar16 == 4)) {
            // Add anchor back to segment positions (re-translate)
            // Cubre segCount+1 filas, acotado a segMax: el renderer (0x00473710) dibuja
            // segCount quads leyendo las filas segIdx y segIdx+1, o sea llega hasta la
            // fila **segCount**, y `sub_46FE90` tambien escribe esa fila.
            int seg_rows = AsJoint(param_1)->NumTails + 1;
            {
                const int seg_max = AsJoint(param_1)->MaxTails;
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
                        *pf    += AsJoint(param_1)->TargetPosition[1];
                        pf[1]  += AsJoint(param_1)->TargetPosition[2];
                        pf += 3;
                    } while (iVar13 != 0);
                    pfVar26  += -0xc;
                    seg_rows--;
                } while (seg_rows != 0);
            }
        }

        /* HashTable obfuscation block skipped */
        // (ref-count insert/remove on MAIN_HASH_CLASS/bcc/bd0/bd4 — not game logic)

        // Harmonic oscillation seed for wispy movement
        uVar7  = param_2 & 0x80000001;
        bVar21 = (uVar7 == 0);
        if ((int)uVar7 < 0) {
            bVar21 = ((uVar7 - 1 | 0xfffffffe) == 0xffffffff);
        }
        // Semilla = MoveSceneFrame, el contador de frames (no el delta X
        // Position.x - Target.x, que realimentaria la orbita).  IDA (0x470030 L1035-1039):
        //     v49 = LODWORD(v297);                      // v297 = MoveSceneFrame
        //     if ( !(iIndex % 2) ) v49 = -LODWORD(v297);
        //     v52 = iIndex + v49 + 53730 * iIndex;
        // Hex-Rays reusa el slot `v297` varias veces en esta funcion.
        //
        // `LODWORD` sobre un float toma sus BITS: MoveSceneFrame es un contador
        // entero (MoveSceneFrame, DWORD) que Hex-Rays tipeo float, asi que el valor
        // correcto es el entero directo — no `float_as_int` de un float negado,
        // que solo invierte el bit de signo y da otra cosa.
        //
        // Ojo el sentido: IDA niega cuando el indice es PAR (`if (!(iIndex % 2))`).
        const int frameSeed = (int)MoveSceneFrame;          // MoveSceneFrame
        const int dc_signed = bVar21 ? -frameSeed : frameSeed;
        iVar13 = dc_signed + (int)(param_2 * 0xd1e3);
        local_a8[2] = 0.1113f;
        iVar16 = AsJoint(param_1)->SubType;
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
            AsJoint(param_1)->Position[0] =
                (float)(fVar24 * (double)_DAT_00552878 + (double)*pfVar14);
            AsJoint(param_1)->Position[1] =
                local_a8[1] * _DAT_00552878 + AsJoint(param_1)->TargetPosition[1];
            AsJoint(param_1)->Position[2] =
                local_a8[3] * _DAT_00552908 + AsJoint(param_1)->TargetPosition[2] + _DAT_005528fc;
            goto switchD_caseD_4ef;
        }
        if (iVar16 == 1) {
            *(unsigned int *)&AsJoint(param_1)->Light[0] = 0x3e4ccccd;  // 0.2f
            *(unsigned int *)&AsJoint(param_1)->Light[1] = 0x3e4ccccd;
            AsJoint(param_1)->Position[0] =
                (float)(fVar24 * (double)_DAT_00552938 + (double)*pfVar14);
            AsJoint(param_1)->Position[1] =
                local_a8[1] * _DAT_00552938 + AsJoint(param_1)->TargetPosition[1];
            AsJoint(param_1)->Position[2] =
                local_a8[3] * _DAT_00552958 + AsJoint(param_1)->TargetPosition[2];
            {
                float fVar28 = sin_s1 * _DAT_005526e4 + _DAT_005528b4;
                AsJoint(param_1)->Light[2] = fVar28;
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
        const int subtype_4e5 = AsJoint(param_1)->SubType;
        if (subtype_4e5 == 1) {
            float light_4e5_1[3] = { 0.8f, 0.4f, 1.0f };
            CreateSprite(1150, pfVar15, 4.0f, light_4e5_1,
                          *(int *)&AsJoint(param_1)->Target, (float)(rand() % 360), 0);
            goto switchD_caseD_4ef;
        }
        if (subtype_4e5 == 3) {
            float orbit_4e5_3[3];
            float light_4e5_3[3] = { 1.0f, 0.5f, 0.1f };
            Ring_ComputeOrbit((int)param_2, orbit_4e5_3, 1.0f);
            *pfVar15 += orbit_4e5_3[0] * 50.0f;
            AsJoint(param_1)->Position[1] += orbit_4e5_3[1] * 50.0f;
            AsJoint(param_1)->Position[2] += orbit_4e5_3[2] * 50.0f;
            CreateSprite(1150, pfVar15, 3.0f, light_4e5_3,
                          *(int *)&AsJoint(param_1)->Target, (float)(rand() % 360), 0);
            CreateSprite(1231, pfVar15, 1.5f, light_4e5_3,
                          *(int *)&AsJoint(param_1)->Target, (float)(rand() % 360), 0);
            goto switchD_caseD_4ef;
        }
        if (subtype_4e5 == 2) {
            const int lifetime_4e5_2 = AsJoint(param_1)->LifeTime;
            float light_4e5_2[3] = { 1.0f, 0.5f, 0.1f };
            if (lifetime_4e5_2 >= 10) {
                if (lifetime_4e5_2 > 18 && lifetime_4e5_2 < 20) {
                    float rotation_4e5_2[3] = { 0.0f, 0.0f, (float)(rand() % 360) };
                    // IDA `o + 28/32/36`: decimal, o sea StartPosition (0x1C), que
                    // CreateJoint carga con la posición inicial. No Angle (0x28).
                    float position_4e5_2[3] = {
                        (float)(rand() % 200) + AsJoint(param_1)->StartPosition[0] - 100.0f,
                        (float)(rand() % 200) + AsJoint(param_1)->StartPosition[1] - 100.0f,
                        AsJoint(param_1)->StartPosition[2] - 200.0f
                    };
                    Joint_Create(1249, position_4e5_2, position_4e5_2, rotation_4e5_2,
                                  2, 0, 40.0f, -1, 0);
                }
            } else {
                const float fade_4e5_2 = AsJoint(param_1)->Light[0] * (5.0f / 6.0f);
                AsJoint(param_1)->Light[0] = fade_4e5_2;
                AsJoint(param_1)->Light[1] = fade_4e5_2;
                AsJoint(param_1)->Light[2] = fade_4e5_2;
                light_4e5_2[0] = fade_4e5_2;
                light_4e5_2[1] = fade_4e5_2;
                light_4e5_2[2] = fade_4e5_2;
            }
            const float scale_4e5_2 = (float)((20 - lifetime_4e5_2) / 5) + 4.0f;
            CreateSprite(1150, pfVar15, scale_4e5_2, light_4e5_2,
                          *(int *)&AsJoint(param_1)->Target, (float)(rand() % 360), 0);
            AsJoint(param_1)->Velocity += 5.0f;
            goto switchD_caseD_4ef;
        }
        if (subtype_4e5 == 0 || subtype_4e5 == 5) {
            if (*(int *)&AsJoint(param_1)->Scale == 1117782016) {
                CreateEffect(205, pfVar15, AsJoint(param_1)->Angle, AsJoint(param_1)->Light,
                             (float *)(subtype_4e5 == 5 ? 3 : 0), 0, (float *)-1, 0, 0);
                if ((AsJoint(param_1)->LifeTime % 15) == 0 && *(int *)&AsJoint(param_1)->Target == (int)Hero) {
                    Entity_FindNearby_SendPacket(AsJoint(param_1)->Skill, pfVar15, 150.0f,
                                  AsJoint(param_1)->Weapon, *(unsigned short *)&AsJoint(param_1)->PKKey);
                }
            }

            const int owner_4e5 = *(int *)&AsJoint(param_1)->Target;
            AsJoint(param_1)->TargetPosition[0] = *(float *)(owner_4e5 + 0x10);
            AsJoint(param_1)->TargetPosition[1] = *(float *)(owner_4e5 + 0x14);
            AsJoint(param_1)->TargetPosition[2] = *(float *)(owner_4e5 + 0x18) + 80.0f;
            const float horizontalDistance_4e5 = sqrtf(local_e4_f * local_e4_f + local_dc_f * local_dc_f);
            MoveHumming(pfVar15, AsJoint(param_1)->Angle, AsJoint(param_1)->TargetPosition,
                          subtype_4e5 == 5 ? 2.0f : 10.0f);
            if (AsJoint(param_1)->Collision == 0 && AsJoint(param_1)->Velocity * 2.0f >= horizontalDistance_4e5)
                AsJoint(param_1)->Collision = 1;

            if (subtype_4e5 != 5) {
                AsJoint(param_1)->Direction[0] += (float)(rand() % 32 - 16) * 0.2f;
                const float angleZ = (float)(rand() % 32 - 16) * 0.8f + AsJoint(param_1)->Direction[2];
                AsJoint(param_1)->Angle[0] += AsJoint(param_1)->Direction[0];
                AsJoint(param_1)->Angle[2] += angleZ;
                AsJoint(param_1)->Direction[0] *= 0.6f;
                AsJoint(param_1)->Direction[2] = angleZ * 0.8f;
            }

            const float terrain_4e5 = RequestTerrainHeight(*pfVar15, AsJoint(param_1)->Position[1]);
            if (terrain_4e5 + 100.0f > AsJoint(param_1)->Position[2]) {
                AsJoint(param_1)->Direction[0] = 0.0f;
                AsJoint(param_1)->Angle[0] = -5.0f;
            }
            if (terrain_4e5 + 400.0f < AsJoint(param_1)->Position[2]) {
                AsJoint(param_1)->Direction[0] = 0.0f;
                AsJoint(param_1)->Angle[0] = 5.0f;
            }
            const float color_4e5 = (float)AsJoint(param_1)->LifeTime * 0.1f;
            AsJoint(param_1)->Light[0] = color_4e5;
            AsJoint(param_1)->Light[1] = color_4e5;
            AsJoint(param_1)->Light[2] = color_4e5;
            const float darkness_4e5 = (float)(rand() % 4 + 4) * -0.01f;
            float light_4e5[3] = { darkness_4e5, darkness_4e5, darkness_4e5 };
            AddTerrainLight(*pfVar15, AsJoint(param_1)->Position[1], (float*)light_4e5, 4, (float*)&DAT_081cb608[0]);
            goto switchD_caseD_4ef;
        }

        // No default subtype branch exists in IDA for type 1253.
        goto switchD_caseD_4ef;
    }

    // El tipo 1254 NO tiene bloque propio en IDA MoveJoint (0x00470030) -- su
    // unica mencion en toda la funcion es la exclusion del epilogo (L2226) -- y
    // cae en la cola generica.  No agregarle un segundo `sub_46FE90` (el epilogo
    // ya lo llama: doble scroll de la estela) ni el fade
    // `if (lifetime < 5) color *= 0.76923078`, que en IDA es del tipo **1176**
    // (L785-791).
    if (iVar16 == 0x4e6) {
        MoveJoint_GenericTail(param_1);   // IDA: 1254 no tiene case -> cae en la cola generica
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4e7: Scatter effect ────────────────────────────────────
    if (iVar16 == 0x4e7) {
        if (AsJoint(param_1)->LifeTime > 15) {
            float target_4e7[3] = {
                (float)(rand() % 200) + AsJoint(param_1)->TargetPosition[0] - 100.0f,
                (float)(rand() % 100) + AsJoint(param_1)->TargetPosition[1] - 50.0f,
                AsJoint(param_1)->TargetPosition[2]
            };
            // IDA (0x00470030 L1880-1886):
            //     *v2 = *(float *)(o + 28);
            //     *(_DWORD *)(o + 20) = *(_DWORD *)(o + 32);
            //     *(_DWORD *)(o + 24) = *(_DWORD *)(o + 36);
            // Esos 28/32/36 son DECIMALES = 0x1C/0x20/0x24, donde `CreateJoint`
            // guarda el ORIGEN del rayo (LABEL_61: `*((float *)v11 + 7) = *v12`).
            // No confundir con 0x28/0x2c/0x30, que es el vector Angle.
            *pfVar15                   = AsJoint(param_1)->StartPosition[0];
            AsJoint(param_1)->Position[1] = AsJoint(param_1)->StartPosition[1];
            AsJoint(param_1)->Position[2] = AsJoint(param_1)->StartPosition[2];
            const int segmentLimit_4e7 = AsJoint(param_1)->MaxTails;
            for (int segment_4e7 = 0; segment_4e7 < segmentLimit_4e7; ++segment_4e7) {
                const float speed_4e7 = (float)(rand() % 80) + 60.0f;
                MoveHumming(pfVar15, AsJoint(param_1)->Angle, target_4e7, speed_4e7);
                AsJoint(param_1)->Direction[0] = (float)(rand() % 1400 - 700) / AsJoint(param_1)->Scale;
                AsJoint(param_1)->Direction[2] = (float)(rand() % 1400 - 700) / AsJoint(param_1)->Scale;
                float angle_4e7[3] = {
                    AsJoint(param_1)->Angle[0] + AsJoint(param_1)->Direction[0],
                    AsJoint(param_1)->Angle[1] + AsJoint(param_1)->Direction[1],
                    AsJoint(param_1)->Angle[2] + AsJoint(param_1)->Direction[2]
                };
                float matrix_4e7[12], step_4e7[3] = { 0.0f, -speed_4e7, 0.0f }, rotated_4e7[3];
                Matrix_BuildFromEuler(angle_4e7, matrix_4e7);
                Joint_SegmentTick((int)param_1, matrix_4e7);
                Vector_Rotate(step_4e7, matrix_4e7, rotated_4e7);
                *pfVar15 += rotated_4e7[0];
                AsJoint(param_1)->Position[1] += rotated_4e7[1];
                AsJoint(param_1)->Position[2] += rotated_4e7[2];
            }
        } else {
            AsJoint(param_1)->Light[2] -= 10.12f;
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4e9: Glow ring ────────────────────────────────────────
    if (iVar16 == 0x4e9) {
        // IDA case 1257: only subtype 1 changes state; all other subtypes
        // proceed directly to the common epilogue.
        if (AsJoint(param_1)->SubType == 1) {
            AsJoint(param_1)->Velocity += 0.3f;
            const float fade_4e9 = AsJoint(param_1)->Light[0] * (5.0f / 7.0f);
            AsJoint(param_1)->Light[0] = fade_4e9;
            AsJoint(param_1)->Light[1] = fade_4e9;
            AsJoint(param_1)->Light[2] = fade_4e9;
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4ea: Crawling creature ────────────────────────────────
    if (iVar16 == 0x4ea) {
        const int subtype_4ea = AsJoint(param_1)->SubType;
        if (subtype_4ea == 0 || subtype_4ea == 1 || subtype_4ea == 6 || subtype_4ea == 9) {
            if (AsJoint(param_1)->LifeTime < 100) {
                AsJoint(param_1)->Velocity += 5.0f;
                if (AsJoint(param_1)->Velocity >= 30.0f)
                    AsJoint(param_1)->Velocity = 30.0f;
                const float originalAngleZ_4ea = AsJoint(param_1)->Angle[2];
                float target_4ea[3];
                if (subtype_4ea == 6 || subtype_4ea == 9) {
                    target_4ea[0] = AsJoint(param_1)->TargetPosition[0];
                    target_4ea[1] = AsJoint(param_1)->TargetPosition[1];
                    target_4ea[2] = AsJoint(param_1)->TargetPosition[2];
                } else {
                    const int owner_4ea = *(int *)&AsJoint(param_1)->Target;
                    target_4ea[0] = *(float *)(owner_4ea + 0x10);
                    target_4ea[1] = *(float *)(owner_4ea + 0x14);
                    target_4ea[2] = *(float *)(owner_4ea + 0x18) + 120.0f;
                    AsJoint(param_1)->TargetPosition[0] = target_4ea[0];
                    AsJoint(param_1)->TargetPosition[1] = target_4ea[1];
                    AsJoint(param_1)->TargetPosition[2] = target_4ea[2];
                }
                // `Distance` es el RETORNO de MoveHumming, no la Z del target (Hex-Rays
                // tipaba MoveHumming como void, retorno en st0).  Confirmado contra el
                // source de MU 5.2 (ZzzEffectJoint.cpp:3368).
                const float dist_4ea =
                    MoveHumming(pfVar15, AsJoint(param_1)->Angle, target_4ea,
                                 AsJoint(param_1)->Velocity);
                if (dist_4ea > 35.0f) {
                    if (dist_4ea <= 70.0f && fabsf(originalAngleZ_4ea - AsJoint(param_1)->Angle[2]) > 20.0f &&
                        AsJoint(param_1)->Velocity >= 20.0f)
                        AsJoint(param_1)->Velocity -= 10.0f;
                } else {
                    *param_1 = 0;
                    Particle_Spawn(1191, pfVar15, AsJoint(param_1)->Angle, AsJoint(param_1)->Light, 1, 1.0f, 0);
                    PlayBuffer(35, 0, 0);
                }
            } else {
                if (subtype_4ea != 0) {
                    AsJoint(param_1)->Angle[2] -= 10.0f;
                    AsJoint(param_1)->Position[2] += 5.0f;
                } else {
                    AsJoint(param_1)->Angle[2] += 10.0f;
                    AsJoint(param_1)->Position[2] += 6.0f;
                }
            }
            const float brightness_4ea = (float)(rand() % 4 + 8) * 0.03f;
            float terrainLight_4ea[3] = { brightness_4ea * 0.4f, brightness_4ea, brightness_4ea * 0.8f };
            AddTerrainLight(*pfVar15, AsJoint(param_1)->Position[1], (float*)terrainLight_4ea, 2, (float*)&DAT_081cb608[0]);
            if (subtype_4ea == 6 || subtype_4ea == 9)
                Particle_Spawn(1191, pfVar15, AsJoint(param_1)->Angle, AsJoint(param_1)->Light, 3, 0.05f, 0);
            else
                Particle_Spawn(1191, pfVar15, AsJoint(param_1)->Angle, AsJoint(param_1)->Light, 0, 1.0f, 0);
            goto switchD_caseD_4ef;
        }
        if (subtype_4ea == 2 || subtype_4ea == 3 || subtype_4ea == 4 || subtype_4ea == 5 || subtype_4ea == 8) {
            const int owner_4ea = *(int *)&AsJoint(param_1)->Target;
            if (*(unsigned char *)owner_4ea == 0) {
                *param_1 = 0;
                return 0;
            }
            if (subtype_4ea == 3) {
                *pfVar15 = *(float *)(owner_4ea + 0x4c);
                AsJoint(param_1)->Position[1] = *(float *)(owner_4ea + 0x50);
                AsJoint(param_1)->Position[2] = *(float *)(owner_4ea + 0x54);
            } else {
                *pfVar15 = *(float *)(owner_4ea + 0x40);
                AsJoint(param_1)->Position[1] = *(float *)(owner_4ea + 0x44);
                AsJoint(param_1)->Position[2] = *(float *)(owner_4ea + 0x48);
                if (subtype_4ea == 8)
                    AsJoint(param_1)->Scale += 10.1f;
            }
            if (subtype_4ea == 2 || subtype_4ea == 3) {
                const float pulse_4ea = sinf((float)WorldTime * 0.002f) * 0.3f + 0.8f;
                float light_4ea[3] = { pulse_4ea * 0.5f, pulse_4ea * 0.1f, pulse_4ea };
                Particle_Spawn(1191, pfVar15, AsJoint(param_1)->Angle, light_4ea, 0, 1.0f, 0);
            }
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4eb: Balloon/fireball ─────────────────────────────────
    if (iVar16 == 0x4eb) {
        const int subtype_4eb = AsJoint(param_1)->SubType;
        const int lifetime_4eb = AsJoint(param_1)->LifeTime;
        if (subtype_4eb == 4) {
            const int owner_4eb = *(int *)&AsJoint(param_1)->Target;
            const float anchorX_4eb = AsJoint(param_1)->TargetPosition[0];
            AsJoint(param_1)->Position[0] = *(float *)(owner_4eb + 0x10) + anchorX_4eb;
            AsJoint(param_1)->Position[1] = *(float *)(owner_4eb + 0x14) + AsJoint(param_1)->TargetPosition[1];
            AsJoint(param_1)->Position[2] = *(float *)(owner_4eb + 0x18) + AsJoint(param_1)->TargetPosition[2] + AsJoint(param_1)->Velocity;
            AsJoint(param_1)->Velocity += 10.0f;
            const float fade_4eb = (float)(12 - lifetime_4eb) * 0.1f;
            AsJoint(param_1)->Light[0] = fade_4eb * 0.4f;
            AsJoint(param_1)->Light[1] = fade_4eb * 0.6f;
            AsJoint(param_1)->Light[2] = fade_4eb;
            goto switchD_caseD_4ef;
        }
        if (subtype_4eb == 5) {
            const float fade_4eb = (float)(12 - lifetime_4eb) * 0.1f;
            AsJoint(param_1)->Light[0] = fade_4eb * 0.4f;
            AsJoint(param_1)->Light[1] = fade_4eb * 0.6f;
            AsJoint(param_1)->Light[2] = fade_4eb;
            goto switchD_caseD_4ef;
        }

        AsJoint(param_1)->Velocity += (subtype_4eb == 6 || subtype_4eb == 7) ? 2.0f : 4.0f;
        float target_4eb[3];
        if (subtype_4eb == 6 || subtype_4eb == 7) {
            target_4eb[0] = AsJoint(param_1)->TargetPosition[0];
            target_4eb[1] = AsJoint(param_1)->TargetPosition[1];
            target_4eb[2] = AsJoint(param_1)->TargetPosition[2];
        } else {
            const int owner_4eb = *(int *)&AsJoint(param_1)->Target;
            target_4eb[0] = *(float *)(owner_4eb + 0x10);
            target_4eb[1] = *(float *)(owner_4eb + 0x14);
            target_4eb[2] = *(float *)(owner_4eb + 0x18) + 120.0f;
            MoveHumming(pfVar15, AsJoint(param_1)->Angle, target_4eb, 10.0f);
        }
        const float fade_4eb = (float)(12 - lifetime_4eb) * 0.1f;
        if (subtype_4eb == 1) {
            AsJoint(param_1)->Light[0] = fade_4eb * 0.4f;
            AsJoint(param_1)->Light[1] = fade_4eb * 0.6f;
            AsJoint(param_1)->Light[2] = fade_4eb;
        } else if (subtype_4eb == 2) {
            AsJoint(param_1)->Light[0] = fade_4eb * 0.4f;
            AsJoint(param_1)->Light[1] = fade_4eb;
            AsJoint(param_1)->Light[2] = fade_4eb * 0.6f;
        } else if (subtype_4eb == 3) {
            AsJoint(param_1)->Light[0] = fade_4eb;
            AsJoint(param_1)->Light[1] = fade_4eb * 0.6f;
            AsJoint(param_1)->Light[2] = fade_4eb * 0.4f;
        }
        if (subtype_4eb == 6 && lifetime_4eb <= 10) {
            const float pulse_4eb = (float)(6 - abs(lifetime_4eb - 6)) * 0.15f;
            float light_4eb[3] = { pulse_4eb, pulse_4eb, pulse_4eb };
            CreateSprite(1231, target_4eb, (float)(rand() % 8 + 8) * 0.05f,
                          light_4eb, *(int *)&AsJoint(param_1)->Target, (float)(rand() % 360), 0);
        } else if (subtype_4eb != 7 && lifetime_4eb == 1) {
            CreateSprite(1231, target_4eb, (float)(rand() % 8 + 8) * 0.2f,
                          AsJoint(param_1)->Light, *(int *)&AsJoint(param_1)->Target, (float)(rand() % 360), 0);
        }
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4ec: Seeker ───────────────────────────────────────────
    if (iVar16 == 0x4ec) {
        const int owner = *(int *)&AsJoint(param_1)->Target;
        AsJoint(param_1)->TargetPosition[0] = *(float *)(owner + 0x10);
        AsJoint(param_1)->TargetPosition[1] = *(float *)(owner + 0x14);
        AsJoint(param_1)->TargetPosition[2] = *(float *)(owner + 0x18) + 130.0f;

        const float horizontalDistance = sqrtf(local_e4_f * local_e4_f + local_dc_f * local_dc_f);
        const float turnStep = 3000.0f / horizontalDistance;
        MoveHumming(pfVar15, AsJoint(param_1)->Angle, AsJoint(param_1)->TargetPosition, turnStep);

        if (AsJoint(param_1)->Collision == 0 &&
            AsJoint(param_1)->Velocity * 2.0f >= horizontalDistance) {
            AsJoint(param_1)->Collision = 1;
            AsJoint(param_1)->LifeTime = 5;
        }
        const float color = (float)AsJoint(param_1)->LifeTime * 0.1f;
        AsJoint(param_1)->Light[0] = color;
        AsJoint(param_1)->Light[1] = color;
        AsJoint(param_1)->Light[2] = color;

        const float terrainLight = (float)(rand() % 4 + 4) * -0.01f;
        float light[3] = { terrainLight, terrainLight, terrainLight };
        AddTerrainLight(*pfVar15, AsJoint(param_1)->Position[1], (float*)light, 4, (float*)&DAT_081cb608[0]);
        goto switchD_caseD_4ef;
    }

    // ── Type 0x4ed: Firefly random wander ────────────────────────────
    // IDA 00470030 LABEL_286: types 0x4ED (1261) and 0x48E (1166)
    // share this guided chain.  0x4ED is not a random-wander branch.
    if (iVar16 == 0x4ed || iVar16 == 0x48e) {
        const int type = iVar16;
        const int segmentLimit = AsJoint(param_1)->MaxTails;
        float targetDistance = local_e4_f;

        for (int segment = 0; segment < segmentLimit; ++segment) {
            if (AsJoint(param_1)->SubType != 2) {
                const int owner = *(int *)&AsJoint(param_1)->Target;
                AsJoint(param_1)->TargetPosition[0] = *(float *)(owner + 0x10);
                AsJoint(param_1)->TargetPosition[1] = *(float *)(owner + 0x14);
                targetDistance = *(float *)(owner + 0x18) + 80.0f;
                AsJoint(param_1)->TargetPosition[2] = targetDistance;
            }

            MoveHumming(pfVar15, AsJoint(param_1)->Angle, AsJoint(param_1)->TargetPosition, 25.0f);
            AsJoint(param_1)->Direction[0] = ((float)(rand() % 256 - 128) / AsJoint(param_1)->Scale + AsJoint(param_1)->Direction[0]) * 0.8f;
            AsJoint(param_1)->Direction[1] *= 0.8f;
            AsJoint(param_1)->Direction[2] = ((float)(rand() % 256 - 128) / AsJoint(param_1)->Scale + AsJoint(param_1)->Direction[2]) * 0.8f;

            float angle[3] = { AsJoint(param_1)->Angle[0] + AsJoint(param_1)->Direction[0],
                               AsJoint(param_1)->Angle[1] + AsJoint(param_1)->Direction[1],
                               AsJoint(param_1)->Angle[2] + AsJoint(param_1)->Direction[2] };
            float matrix[12], rotated[3];
            Matrix_BuildFromEuler(angle, matrix);
            Joint_SegmentTick((int)param_1, matrix);

            if (AsJoint(param_1)->Velocity * 2.0f >= targetDistance) {
                if ((rand() & 1) == 0) {
                    (void)rand(); (void)rand(); (void)rand();
                    Particle_Spawn(1195, pfVar15, AsJoint(param_1)->Angle, AsJoint(param_1)->Light, 0, 1.0f, 0);
                }
                break;
            }

            const float intensity = (float)(rand() % 4 + 4) * 0.05f;
            float light[3];
            if (type == 0x4ed) {
                light[0] = intensity;
                if (AsJoint(param_1)->SubType == 1) {
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
            AddTerrainLight(*pfVar15, AsJoint(param_1)->Position[1], (float*)light, 2, (float*)&DAT_081cb608[0]);

            float step[3] = { 0.0f, -AsJoint(param_1)->Velocity, 0.0f };
            Vector_Rotate(step, matrix, rotated);
            *pfVar15 += rotated[0];
            AsJoint(param_1)->Position[1] += rotated[1];
            AsJoint(param_1)->Position[2] += rotated[2];
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
            if (AsJoint(param_1)->NumTails < AsJoint(param_1)->MaxTails - 1) {
                float previousPosition[3] = {
                    AsJoint(param_1)->Position[0], AsJoint(param_1)->Position[1], AsJoint(param_1)->Position[2]
                };
                float matrix[12];
                float rotated[3];

                Matrix_BuildFromEuler(AsJoint(param_1)->Direction, matrix);
                Vector_Rotate(in1, matrix, rotated);
                AsJoint(param_1)->Position[0] = rotated[0] + AsJoint(param_1)->TargetPosition[0];
                AsJoint(param_1)->Position[1] = rotated[1] + AsJoint(param_1)->TargetPosition[1];
                AsJoint(param_1)->Position[2] = rotated[2] + AsJoint(param_1)->TargetPosition[2];

                Matrix_BuildFromEuler(AsJoint(param_1)->Angle, matrix);
                Joint_SegmentTick((int)param_1, matrix);
                AsJoint(param_1)->Direction[2] -= 11.0f;
                if ((rand() & 1) == 0) {
                    Particle_Spawn(1195, AsJoint(param_1)->Position,
                                 AsJoint(param_1)->Angle, AsJoint(param_1)->Light, 0, 1.0f, 0);
                }
                Joint_Create(1254, previousPosition, AsJoint(param_1)->Position,
                              AsJoint(param_1)->Angle, 3, 0, (float)(rand() % 10) + 5.0f, 5, 10);
                Joint_Create(1254, previousPosition, AsJoint(param_1)->Position,
                              AsJoint(param_1)->Angle, 3, 0, (float)(rand() % 8) + 4.0f, 5, 10);
            }
            if (*(int *)&AsJoint(param_1)->Target == (int)Hero && AsJoint(param_1)->LifeTime > 18 && (i % 5) == 0) {
                Entity_FindNearby_SendPacket(AsJoint(param_1)->Skill, AsJoint(param_1)->Position, 150.0f,
                              AsJoint(param_1)->Weapon, *(unsigned short *)&AsJoint(param_1)->PKKey);
            }
        }
        const int lifetime = AsJoint(param_1)->LifeTime;
        const float fade = lifetime >= 10 ? (float)lifetime * (1.0f / 30.0f)
                                          : AsJoint(param_1)->Light[0] * (2.0f / 3.0f);
        AsJoint(param_1)->Light[0] = fade;
        AsJoint(param_1)->Light[1] = fade;
        AsJoint(param_1)->Light[2] = fade;
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
            AsJoint(param_1)->Position[0] += _DAT_005524f0;
        }
        else {
            AsJoint(param_1)->Position[0] -= _DAT_005524f0;
        }
        uVar7 = _rand();
        uVar7 = uVar7 & 0x80000001;
        bVar21 = (uVar7 == 0);
        if ((int)uVar7 < 0) {
            bVar21 = ((uVar7 - 1 | 0xfffffffe) == 0xffffffff);
        }
        if (!bVar21) {
            AsJoint(param_1)->Position[1] += _DAT_005524f0;
        }
        else {
            AsJoint(param_1)->Position[1] -= _DAT_005524f0;
        }
        Joint_SegmentTick((int)param_1, local_30);
        goto switchD_caseD_4ef;
    }

#endif
switchD_caseD_4fd:
    // ── IDA LABEL_301 — movimiento compartido por los tipos 1249 (0x4E1) y
    // 1277 (0x4FD). Es un switch sobre el SubType (+0x08).
    //
    // El subtipo 0 (el halo del set +11, spawneado por `Entity_UpdateRender`
    // sección 6) es un **círculo que sube**:
    //     a = (lifetime + phase) * 0.1        (PKKey == -1)
    //     Position.x = cos(a) * 40 + TargetPos.x
    //     Position.y = TargetPos.y - sin(a) * 40
    //     Position.z += riseSpeed
    //
    // TODOS los subtipos estan portados y verificados contra IDA.  El switch de
    // IDA (L1085) solo lleva los cases 0/2/3/10/14 y NO tiene default: el resto se
    // resuelve con ifs encadenados desde L1162, asi que no aparecen como `case`.
    // LABEL_438/439 no es anti-tamper sino la cola comun a la que saltan varios
    // subtipos (MoveJoint no tiene ruido de hash-table).
    {
        const int  jsub  = AsJoint(param_1)->SubType;
        float     *jposX = AsJoint(param_1)->Position;   // Position.x  (IDA v2)
        const float tgtX = AsJoint(param_1)->TargetPosition[0];  // TargetPos.x (IDA v3)
        const float tgtY = AsJoint(param_1)->TargetPosition[1];  // TargetPos.y (o + 72)

        if (jsub == 0 || jsub == 10) {
            short pk  = AsJoint(param_1)->PKKey;                 // PKKey
            double t  = (double)AsJoint(param_1)->LifeTime            // lifetime
                      + (double)AsJoint(param_1)->Direction[1];         // fase random
            double a  = (pk == -1) ? (t * 0.1) : (t / (double)pk);
            *jposX                     = (float)(fcos(a) * 40.0 + (double)tgtX);
            AsJoint(param_1)->Position[1] = (float)((double)tgtY - fsin(a) * 40.0);
            float rise = AsJoint(param_1)->Direction[2];
            AsJoint(param_1)->Position[2] = (jsub != 0)
                ? (AsJoint(param_1)->Position[2] - rise)
                : (rise + AsJoint(param_1)->Position[2]);
            goto switchD_caseD_4ef;
        }
        if (jsub == 14) {
            // 00470030 LABEL_301/case 14.
            int owner = *(int *)&AsJoint(param_1)->Target;
            if (owner != 0) {
                *(DWORD *)&AsJoint(param_1)->StartPosition[0] = *(DWORD *)(owner + 392);
                *(DWORD *)&AsJoint(param_1)->StartPosition[1] = *(DWORD *)(owner + 396);
            }
            const int life = AsJoint(param_1)->LifeTime;
            const short pk = AsJoint(param_1)->PKKey;
            const double t = (double)life + (double)AsJoint(param_1)->Direction[1];
            const double a = (pk == -1) ? t * 0.1 : t / (double)pk;
            const float approach = (float)(100 - life) * 0.011111111f;
            *jposX = (float)(fcos(a) * 15.0) +
                      approach * tgtX + AsJoint(param_1)->StartPosition[0];
            AsJoint(param_1)->Position[1] = AsJoint(param_1)->StartPosition[1] -
                                          (float)(fsin(a) * 15.0) + approach * tgtY;
            AsJoint(param_1)->Position[2] += AsJoint(param_1)->Direction[2];
            if (AsJoint(param_1)->Position[2] >= 500.0f) {
                AsJoint(param_1)->SubType = 1;
                AsJoint(param_1)->LifeTime = (life > 10) ? 10 : life;
            }
            goto switchD_caseD_4ef;
        }
        if (jsub == 2 || jsub == 13) {
            if (AsJoint(param_1)->LifeTime > 25) goto switchD_caseD_4ef;
            float step = AsJoint(param_1)->Direction[2] + (jsub == 2 ? 5.0f : 1.0f);
            *jposX                     = tgtX;
            AsJoint(param_1)->Position[1] = tgtY;
            AsJoint(param_1)->Direction[2] = step;
            AsJoint(param_1)->Position[2] = step + AsJoint(param_1)->Position[2];
            goto switchD_caseD_4ef;
        }
        if (jsub == 5) {
            float step = AsJoint(param_1)->Direction[2] - 60.0f;
            *jposX                      = tgtX;
            AsJoint(param_1)->Position[1]  = tgtY;
            AsJoint(param_1)->Direction[2] = step;
            AsJoint(param_1)->Position[2]  = step + AsJoint(param_1)->Position[2];
            AsJoint(param_1)->Scale  = (float)(_rand() % 4) + 6.0f;
            goto switchD_caseD_4ef;
        }
        if (jsub == 4 || jsub == 6 || jsub == 12) {
            // 00470030 LABEL_301 subtipos 4, 6 y 12: los tres convergen en
            // LABEL_438/439 y solo cambian como calculan lateral/vertical.
            //
            // El 4 llega aca por el `if (v168 != 4)` de L1162, que SALTEA todo el bloque
            // de los demas subtipos, asi que cae junto al 12 en L1681
            // (`v16 = v168 == 12`).  Su rama es la del else de L1702.
            const int life = AsJoint(param_1)->LifeTime;
            const float phase = AsJoint(param_1)->Direction[0];
            float lateral;
            float vertical;
            if (jsub == 4) {
                // IDA L1702-1717: radio = clamp(life + 40, min 10) * 0.65
                int c = life + 40;
                if (c <= 10) c = 10;
                const double r = (double)c * 0.64999998;
                const double a = ((double)life + (double)phase) * 0.1;
                lateral  = (float)(-fcos(a) * r);
                vertical = (float)( fsin(a) * r);
            } else if (jsub == 6) {
                const float radius = (float)((life <= 1) ? 1 : life);
                const double a = (double)life * 0.5 + (double)phase;
                lateral = (float)(-fcos(a) * ((double)radius + (double)radius));
                vertical = (float)(fsin(a) * ((double)radius + (double)radius));
            } else {
                const double a = ((double)life + (double)phase) * 0.1;
                lateral = (float)(fcos(a) * -26.0);
                vertical = (float)(fsin(a) * 26.0 - (double)(90 - life) * 0.3);
            }
            AsJoint(param_1)->TargetPosition[0] += AsJoint(param_1)->Direction[1];
            AsJoint(param_1)->TargetPosition[1] += AsJoint(param_1)->Direction[2];
            const float heading = (90.0f - AsJoint(param_1)->Angle[2]) * 0.017453292f;
            *jposX = fsin(heading) * lateral + AsJoint(param_1)->TargetPosition[0];
            AsJoint(param_1)->Position[1] = fcos(heading) * lateral + AsJoint(param_1)->TargetPosition[1];
            AsJoint(param_1)->Position[2] = vertical + AsJoint(param_1)->TargetPosition[2];
            goto switchD_caseD_4ef;
        }
        // Los subtipos 8 y 9 usan una matriz APARTE (`v304`, declarada en L307), no
        // `local_30`, que es el `in2` de IDA: la matriz que arma el prologo con
        // `AngleMatrix((float *)(o + 40), in2)` (L319) y que el epilogo LABEL_487
        // (L2228) le pasa a `sub_46FE90` para construir las 4 esquinas del segmento
        // nuevo.
        float jointMatrix_v304[12];               // IDA: v304[3][4]
        if (jsub == 8) {
            // 00470030 LABEL_301/case 8: rotate the fixed vertical offset
            // by TargetPosition, anchor it at StartPosition, then advance TargetPosition[2].
            float offset[3] = { 0.0f, -50.0f, 0.0f };   // IDA: in1
            float rotated[3];                           // IDA: Position
            Matrix_BuildFromEuler(AsJoint(param_1)->TargetPosition, jointMatrix_v304);
            Vector_Rotate(offset, jointMatrix_v304, rotated);
            *jposX                     = rotated[0] + AsJoint(param_1)->StartPosition[0];
            AsJoint(param_1)->Position[1] = rotated[1] + AsJoint(param_1)->StartPosition[1];
            AsJoint(param_1)->Position[2] = rotated[2] + AsJoint(param_1)->StartPosition[2];
            Matrix_BuildFromEuler(AsJoint(param_1)->Angle, jointMatrix_v304);
            AsJoint(param_1)->TargetPosition[2] += 10.0f;
            goto switchD_caseD_4ef;
        }
        if (jsub == 9) {
            // 00470030 LABEL_301/case 9.
            Matrix_BuildFromEuler(AsJoint(param_1)->Angle, jointMatrix_v304);
            const int segments = AsJoint(param_1)->MaxTails;   // IDA: *(int *)(o + 84)
            for (int i = 0; i < segments; ++i) {             // IDA: v180 / iIndex
                *jposX                     += AsJoint(param_1)->StartPosition[0];
                AsJoint(param_1)->Position[1] += AsJoint(param_1)->StartPosition[1];
                AsJoint(param_1)->Position[2] += AsJoint(param_1)->StartPosition[2];
                Joint_SegmentTick((int)param_1, jointMatrix_v304);
            }
            goto switchD_caseD_4ef;
        }
        if (jsub == 3) {
            goto switchD_caseD_4ef;   // IDA: case 3 → LABEL_447 (sin movimiento)
        }

        // Subtipos 7 y 11 (IDA 0x00470030 L1302-1682).
        //
        // Estos dos NO caen en la helice generica de mas abajo: en LABEL_301
        // `if (v168 != 7)` cierra en L1301 y la ejecucion sigue en L1302, dentro del
        // bloque de `if (v168 != 12)`, que llega hasta L1683 y termina en
        // `goto LABEL_447`.  Solo los subtipos 4 y 12 alcanzan la helice.
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
        // 1249/sub7 con el heroe como owner.
        if (jsub == 7 || jsub == 11) {
            const int iIndex = (int)param_2;                 // indice de slot
            const int msf    = (int)MoveSceneFrame;            // MoveSceneFrame
            const int v203   = (iIndex % 2) ? msf : -msf;
            float a1 = 0.061299998f, a2 = 0.1113f, v204 = 0.048f;
            if (jsub == 11) { v204 = 0.071999997f; a1 = 0.091949999f; a2 = 0.16695f; }
            const int    v205 = iIndex + v203 + 53730 * iIndex;
            const double v207 = (double)v204 * (double)(v205 + 55555);
            const int    life = AsJoint(param_1)->LifeTime;
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
            const int    v294 = AsJoint(param_1)->MultiUse + 30;
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
            AsJoint(param_1)->Position[0] = v222 * A0   + AsJoint(param_1)->TargetPosition[0];
            AsJoint(param_1)->Position[1] = v222 * A1   + AsJoint(param_1)->TargetPosition[1];
            AsJoint(param_1)->Position[2] = v222 * out0 + AsJoint(param_1)->TargetPosition[2];

            const int jowner = *(int *)&AsJoint(param_1)->Target;
            if (jowner) {
                int v226 = 0;
                // El subtipo 11 NO converge hacia `owner + 16/20/24` sino hacia
                // `owner + 368/372/376`.  IDA lo escribe ofuscado --
                // `LODWORD(v295) = 352 - o;` y luego
                // `*(float *)(LODWORD(v295) + *(_DWORD *)(o + 64) + v228)` --
                // pero 352 + {16,20,24} = {368,372,376}, que es la posicion
                // DERIVADA del efecto de la flecha: la que fija CreateEffect
                // case 243 (`i+92..94`) 100 unidades por delante y la que usa su
                // propio AddTerrainLight.
                //
                // Con `owner + 16` la cosa se realimenta: el joint con SkillIndex==1
                // escribe su posicion EN `owner + 16`, asi que convergeria hacia si mismo.
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
            if (jsub != 11) AsJoint(param_1)->Position[2] += 100.0f;

            float jangles[3] = { 0.5f, 0.5f, 1.0f };
            if (jsub == 11) {
                jangles[0] = 0.30000001f; jangles[1] = 0.30000001f; jangles[2] = 0.5f;
                if (AsJoint(param_1)->Skill == 1 && jowner) {
                    *(int *)(jowner + 16) = *(int *)&AsJoint(param_1)->Position[0];
                    *(int *)(jowner + 20) = *(int *)&AsJoint(param_1)->Position[1];
                    *(int *)(jowner + 24) = *(int *)&AsJoint(param_1)->Position[2];
                    Effect_CollisionCheck(jowner);
                }
            }
            {
                float *ppos = AsJoint(param_1)->Position;
                CreateSprite(1231, ppos, (float)(_rand() % 2 + 8) * 0.1f,
                             jangles, jowner, (float)(_rand() % 360), 0);
                CreateSprite(1150, ppos, (float)(_rand() % 2 + 8) * 0.18000001f,
                             jangles, jowner, (float)(_rand() % 360), 0);
                CreateSprite(1150, ppos, (float)(_rand() % 2 + 8) * 0.18000001f,
                             jangles, jowner, (float)(_rand() % 360), 0);
                if (jsub != 11 && AsJoint(param_1)->LifeTime == 1) {
                    float jzero[3]  = { 0.0f, 0.0f, 0.0f };
                    float jwhite[3] = { 1.0f, 1.0f, 1.0f };
                    Particle_Spawn(1215, ppos, jzero, jwhite, 1, 0.60000002f, 0);
                }
            }
            goto _skipLabel182;   // IDA: LABEL_447 -> LABEL_487, saltea LABEL_182
        }
    }

    // Cola real de LABEL_301 (IDA L1706-1724 -> LABEL_438 -> LABEL_439) para los
    // subtipos que no tienen bloque propio arriba (4/6/12 y 7/11 salen antes).
    // No llamar `Joint_SegmentTick` aca: el epilogo LABEL_487 ya lo llama.
    //
    // Es la MISMA helice que 4, 6 y 12, con otra formula de radio:
    //     v237 = *(_DWORD *)(o + 2488);                       // life
    //     v238 = ((double)v237 + *(float *)(o + 2500)) * 0.1;  // angulo
    //     v240 = v237 + 40;  v296 = (v240 <= 10) ? 10 : v240;  // radio
    //     v234 = -(cos(v238) * (v296 * 0.64999998));
    //     v236 =   v296 * 0.64999998;  v235 = sin(v238);
    //   LABEL_438: v239 = v235 * v236;
    //   LABEL_439: <ancla + heading + escritura de Pos>
    {
        const int life = AsJoint(param_1)->LifeTime;              // IDA: v237
        const float phase = AsJoint(param_1)->Direction[0];         // IDA: o + 2500
        const double a = ((double)life + (double)phase) * 0.1;   // IDA: v238
        int radius = life + 40;                                  // IDA: v240
        if (radius <= 10) radius = 10;                           // IDA: v296 = 10
        const double r65 = (double)radius * 0.64999998;
        const float lateral  = (float)(-(fcos(a) * r65));        // IDA: v234
        const float vertical = (float)(fsin(a) * r65);           // IDA: v239 = v235 * v236

        // IDA LABEL_439 (compartido con los subtipos 6 y 12).
        AsJoint(param_1)->TargetPosition[0] += AsJoint(param_1)->Direction[1];
        AsJoint(param_1)->TargetPosition[1] += AsJoint(param_1)->Direction[2];
        const float heading = (90.0f - AsJoint(param_1)->Angle[2]) * 0.017453292f;
        AsJoint(param_1)->Position[0] = fsin(heading) * lateral + AsJoint(param_1)->TargetPosition[0];
        AsJoint(param_1)->Position[1] = fcos(heading) * lateral + AsJoint(param_1)->TargetPosition[1];
        AsJoint(param_1)->Position[2] = vertical + AsJoint(param_1)->TargetPosition[2];
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
    if (AsJoint(param_1)->SubType == 7) {
        AsJoint(param_1)->Position[0] = AsJoint(param_1)->TargetPosition[0];
        AsJoint(param_1)->Position[1] = AsJoint(param_1)->TargetPosition[1];
        AsJoint(param_1)->Position[2] = AsJoint(param_1)->TargetPosition[2];
    }
_skipLabel182:;

    // Orden de IDA en el epilogo:
    //     LABEL_182 (re-ancla SubType 7)  ->  LABEL_487 (scroll de segmentos)
    // El `goto _skipLabel182` que usa el bloque de los subtipos 7/11 salta
    // LABEL_182 pero SI pasa por LABEL_487; si no, esos joints no acumulan
    // segmentos (estela de la flecha del arco -- cuatro 1249/sub11 que crea
    // CreateEffect case 243 -- y la cadena de Queen Rainer).
    iVar16 = AsJoint(param_1)->Type;
    if (((iVar16 != 0x4e6) || (AsJoint(param_1)->SubType != 6)) &&
        ((iVar16 != 0x4ee) &&
         ((iVar16 != 0x4e7) || (10 < AsJoint(param_1)->LifeTime))))
    {
        Joint_SegmentTick((int)param_1, local_30);
    }

    // Decrement lifetime counter
    const int remainingLife = AsJoint(param_1)->LifeTime - 1;
    AsJoint(param_1)->LifeTime = remainingLife;
    if (remainingLife >= 0) {
        const int type = AsJoint(param_1)->Type;
        const int subtype = AsJoint(param_1)->SubType;
        if ((remainingLife % 12) != 0 && (type == 1249 || type == 1277) &&
            (subtype == 4 || subtype == 12)) {
            return MoveJoint(param_1, param_2);
        }
        return (char *)(intptr_t)remainingLife;
    }
    *param_1 = 0;
    return (char *)(intptr_t)remainingLife;
#if 0 // Former decompiler-artifact formulation of the same epilogue.
    pcVar11 = (char *)(AsJoint(param_1)->LifeTime + -1);
    AsJoint(param_1)->LifeTime = (int)(intptr_t)pcVar11;

    if (-1 < (int)(intptr_t)pcVar11) {
        // Recursive self-call on types 0x4e1/0x4fd at sub-frame boundaries (modes 4 or 0xc)
        pcVar12 = (char *)((int)(intptr_t)pcVar11 / 0xc);
        if ((((int)(intptr_t)pcVar11 % 0xc != 0) &&
             ((AsJoint(param_1)->Type == 0x4e1) ||
              ((pcVar12 = (char *)(AsJoint(param_1)->Type + -0x4fd),
                pcVar12 == (char *)0x0)))) &&
            ((pcVar12 = *(char **)(param_1 + 8),
              (pcVar12 == (char *)0x4) || (pcVar12 == (char *)0xc))))
        {
            pcVar12 = MoveJoint(param_1, param_2);
        }
        return pcVar12;
    }

    // Lifetime expired — deactivate slot
    *param_1 = 0;
    return pcVar11;
#endif
}

// end of MoveJoint
