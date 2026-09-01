// Weather_Particles.cpp — IDA: FUN_0046cc80 @ 0x0046cc80 — WeatherParticles_Update
// Per-frame weather particle pool tick.
//
// Iterates a particle pool at DAT_07c5ab5c (pointer to base), stride 0x70 bytes
// (pfVar10 advances by 0x1c floats = 0x70 bytes per entry).
// Active count = iVar9, computed from g_GameSubState.
//
// Particle entry layout (relative to pfVar10 at offset +0x44 in entry):
//   pfVar10[-0xb]  char   active flag (0=free, nonzero=alive)
//   pfVar10[-0xa]  float  type_id    (1.4013e-43 = type0/normal, 1.41531e-43 = type2b,
//                                     1.42932e-43 = snow/rain_fall)
//   pfVar10[-9]    float  (unused in visible logic)
//   pfVar10[-8]    float  drop_weight (5.0 normal, 10.0 heavy rain)
//   pfVar10[-7]    float  pos_x
//   pfVar10[-6]    float  pos_y
//   pfVar10[-5]    float  pos_z
//   pfVar10[-4]    float  vel_x (angle/dir for snow)
//   pfVar10[-3]    float  vel_y
//   pfVar10[-2]    float  vel_z
//   pfVar10[-1]    float  color_r
//   pfVar10[0]     float  color_g
//   pfVar10[1]     float  color_b
//   pfVar10[8..10] float  velocity xyz (integrated each frame)
//   pfVar10[0xb..d] float angular velocity xyz (butterfly/bubble)
//   pfVar10[0xe..10] float origin xyz (butterfly/bubble start pos)
//
// g_GameSubState values (DAT_0055a7ac):
//   0  = ServerSelect — butterfly/bubble floating around origin, deactivated at max dist
//   2  = rain (light)  — falling drops, 1/10 chance heavy (drop_weight=10)
//   3,7 = InGame map    — butterfly/bubble (same as 0)
//   9  = logout/loading — snow: rotated velocity via Matrix_BuildFromEuler/4fa0b0,
//                          splash effect (Particle_Spawn type 0x67/0x68) on ground hit
//   10 = rain (dense)   — limited active count = (DAT_07c74ae4 * 200) / 100
//
// Wind/environment per frame:
//   _DAT_00559b9c = (int)(fsin(tick * _DAT_00552500) + 3) * 10     — wind X speed
//   DAT_07c74ae8  = (int)(fsin(tick * _DAT_00552a40 + offset) * 20) — rain Y offset
//   DAT_07c74aec  = (DAT_07c74aec + 0x14) % 2000                   — rain Z counter
//
// Related: DAT_07c74ae0 = intensity target (set to 100 when substate→10)
//          DAT_07c74ae4 = current intensity (interpolated toward ae0 each frame)

#include "stdafx.h"
#pragma warning(disable: 4244 4101)
#include "globals.h"
#include "functions.h"
#include <math.h>

// fsin/__ftol are defined as inline functions in stdafx.h (Ghidra intrinsic stubs)
// _rand is a macro defined as rand() in stdafx.h

// ─────────────────────────────────────────────────────────────────────────────

void WeatherParticles_Update(void)
{
    int iVar7 = DAT_0055a7ac;   // g_GameSubState

    // ── Intensity ramp for sub-state 10 (dense rain) ─────────────────────────
    if (iVar7 == 10)
        DAT_07c74ae0 = 100;
    if (DAT_07c74ae4 < (int)DAT_07c74ae0)      DAT_07c74ae4++;
    else if ((int)DAT_07c74ae4 > (int)DAT_07c74ae0) DAT_07c74ae4--;

    // ── Per-frame wind / oscillation globals ─────────────────────────────────
    // BUG-FIX 2026-07-19: acá había un bloque con `__ftol()` sin argumentos
    // (artefacto de Ghidra) y debajo una "aproximación" que OMITÍA el sin()
    // por completo — o sea RainSpeed/RainAngle crecían monótonamente con
    // WorldTime en vez de oscilar. IDA MoveLeaves (0x46CC80) es explícito:
    //   RainSpeed = 10 * ((__int64)sin(WorldTime * 0.001) + 3);
    //   RainAngle = 20 * (__int64)sin(WorldTime * 0.00050000002 + 50.0);
    // (_DAT_00559b9c = RainSpeed, DAT_07c74ae8 = RainAngle.)
    _DAT_00559b9c = (int)(10 * ((longlong)sin((double)DAT_05826e08 * 0.001) + 3));
    DAT_07c74ae8  = (int)(20 * (longlong)sin((double)DAT_05826e08 * 0.00050000002 + 50.0));

    int iVar1 = DAT_07c74aec + 0x14;
    DAT_07c74aec  = iVar1 % 2000;

    // ── Compute active particle count ─────────────────────────────────────────
    // sub-state 9 (snow/logout): 200; others: 0x88 (136) unless sub-state is 9
    // IDA: iMaxLeaves = World != 9 ? 80 : 200;  (el port tenia 0x88 = 136)
    int iVar9 = (iVar7 != 9) ? 80 : 200;
    if (iVar9 < 1) return;

    // ── Main particle loop ────────────────────────────────────────────────────
    // IDA: for ( i = (float *)&unk_7C5AB5C; ; i += 28 )
    // El port sumaba 0x44 bytes de mas sobre un alias que ya estaba corrido 12,
    // asi que escribia el flag de activo 56 bytes fuera de donde lo lee
    // SkillEffect_Render: el pool quedaba siempre vacio (medido: active=0).
    float *pfVar10 = (float *)&DAT_07c5ab5c;
    int local_d4 = 0;

    do {
        if (*(char *)(pfVar10 - 0xb) == '\0')
        {
            // ── Spawn new particle ────────────────────────────────────────────
            pfVar10[-1] = 1.0f; pfVar10[0] = 1.0f; pfVar10[1] = 1.0f;
            *(char *)(pfVar10 - 0xb) = 1;

            if (iVar7 == 9)
            {
                // Snow: rotated falling velocity via euler matrix
                pfVar10[-10] = 1.42932e-43f;
                if ((int)pfVar10 < 0x7c62e9c) {
                    pfVar10[-7] = (float)(_rand() % 0x640 - 800) + *(float *)(DAT_07abf5d8 + 0x10);
                    pfVar10[-6] = (float)(_rand() % 0x578 - 500) + *(float *)(DAT_07abf5d8 + 0x14);
                } else {
                    pfVar10[-7] = (float)(_rand() % 0x640 - 800) + *(float *)(DAT_07abf5d8 + 0x10);
                    pfVar10[-6] = ((float)(_rand() % 300 + 1000) + *(float *)(DAT_07abf5d8 + 0x14))
                                  - (float)DAT_07c74aec;
                }
                pfVar10[-5] = (float)(_rand() % 200 + 300) + *(float *)(DAT_07abf5d8 + 0x18);

                // random horizontal direction: even=left, odd=left+wind
                unsigned int r = (unsigned int)_rand() & 0x80000001u;
                if ((int)r < 0) r = (r - 1u | 0xfffffffeu) + 1u;
                int vx;
                if (r == 0) vx = -0x14 - _rand() % 0x14;
                else        vx = (-0x1e  - _rand() % 0x14) - (int)DAT_07c74ae8;

                pfVar10[-4] = (float)vx;
                pfVar10[-3] = 0.0f;
                pfVar10[-2] = 0.0f;

                float local_b4[3] = { 0.0f, 0.0f, -(float)(_rand() % 0x28 + _DAT_00559b9c) };
                float mat[12], out[3];
                Matrix_BuildFromEuler(pfVar10 - 4, mat);
                Vector_Rotate(local_b4, mat, pfVar10 + 8);
            }
            else if (iVar7 == 0)
            {
                // Butterfly/bubble
                pfVar10[-10] = 1.4013e-43f;
                float fx = (float)(_rand() % 0x640 - 800) + *(float *)(DAT_07abf5d8 + 0x10);
                float fy = (float)(_rand() % 0x578 - 500) + *(float *)(DAT_07abf5d8 + 0x14);
                float fz = (float)(_rand() % 300 + 0x32)  + *(float *)(DAT_07abf5d8 + 0x18);
                pfVar10[-7] = fx; pfVar10[-6] = fy; pfVar10[-5] = fz;
                pfVar10[0xe] = fx; pfVar10[0xf] = fy; pfVar10[0x10] = fz;

                // Random rotation angle
                unsigned int ur = (unsigned int)_rand() & 0x8000003fu;
                if ((int)ur < 0) ur = (ur - 1u | 0xffffffc0u) + 1u;
                float ang = (float)(int)(ur + 0x40) * _DAT_005529c8;
                pfVar10[8] = ang;
                if (fy < *(float *)&DAT_083a42d8 + _DAT_00552850)
                    pfVar10[8] = _DAT_00552a3c - ang;

                ur = (unsigned int)_rand() & 0x8000001fu; if ((int)ur < 0) ur = (ur-1u|0xffffffe0u)+1u;
                pfVar10[9]  = (float)(int)(ur - 0x10) * _DAT_005524f4;
                ur = (unsigned int)_rand() & 0x8000001fu; if ((int)ur < 0) ur = (ur-1u|0xffffffe0u)+1u;
                pfVar10[10] = (float)(int)(ur - 0x10) * _DAT_005524f4;
                ur = (unsigned int)_rand() & 0x8000000fu; if ((int)ur < 0) ur = (ur-1u|0xfffffff0u)+1u;
                pfVar10[0xb] = (float)(int)(ur - 8) * _DAT_005524f4;
                ur = (unsigned int)_rand() & 0x8000003fu; if ((int)ur < 0) ur = (ur-1u|0xffffffc0u)+1u;
                pfVar10[0xc] = (float)(int)(ur - 0x20) * _DAT_005524f4;
                ur = (unsigned int)_rand() & 0x8000000fu; if ((int)ur < 0) ur = (ur-1u|0xfffffff0u)+1u;
                pfVar10[0xd] = (float)(int)(ur - 8) * _DAT_005524f4;
            }
            else if (iVar7 == 10)
            {
                // Dense rain: limited active count
                if (local_d4 < ((int)DAT_07c74ae4 * 200) / 100) {
                    pfVar10[-10] = 1.42932e-43f;
                    pfVar10[-7] = (float)(_rand() % 0x640 - 800) + *(float *)(DAT_07abf5d8 + 0x10);
                    pfVar10[-6] = (float)(_rand() % 0x578 - 500) + *(float *)(DAT_07abf5d8 + 0x14);
                    pfVar10[-4] = -30.0f; pfVar10[-3] = 0.0f; pfVar10[-2] = 0.0f;
                    float fz2 = *(float *)(DAT_07abf5d8 + 0x18);
                    pfVar10[-5] = (float)(_rand() % 200 + 200) + fz2;
                    float loc[3] = { 0.0f, 0.0f, -(float)(_rand() % 0x18 + 0x14) };
                    float mat[12];
                    Matrix_BuildFromEuler(pfVar10 - 4, mat);
                    Vector_Rotate(loc, mat, pfVar10 + 8);
                }
            }
            else if (iVar7 == 2)
            {
                // Light rain (1/10 chance heavy)
                pfVar10[-10] = 1.4013e-43f;
                pfVar10[-8]  = 5.0f;
                if (_rand() % 10 == 0) { pfVar10[-10] = 1.41531e-43f; pfVar10[-8] = 10.0f; }
                pfVar10[-7] = (float)(_rand() % 0x640 - 800) + *(float *)(DAT_07abf5d8 + 0x10);
                pfVar10[-6] = (float)(_rand() % 0x578 - 500) + *(float *)(DAT_07abf5d8 + 0x14);
                float fz2   = *(float *)(DAT_07abf5d8 + 0x18);
                pfVar10[-4] = -30.0f; pfVar10[-3] = 0.0f; pfVar10[-2] = 0.0f;
                pfVar10[-5] = (float)(_rand() % 200 + 200) + fz2;
                unsigned int ur = (unsigned int)_rand() & 0x8000000fu;
                if ((int)ur < 0) ur = (ur - 1u | 0xfffffff0u) + 1u;
                float loc[3] = { 0.0f, 0.0f, -(float)(int)(ur + 8) };
                float mat[12];
                Matrix_BuildFromEuler(pfVar10 - 4, mat);
                Vector_Rotate(loc, mat, pfVar10 + 8);
            }
            else if (iVar7 == 3 || iVar7 == 7)
            {
                // Butterfly/bubble (in-game map states) — same as state 0
                pfVar10[-10] = 1.4013e-43f;
                float fx = (float)(_rand() % 0x640 - 800) + *(float *)(DAT_07abf5d8 + 0x10);
                float fy = (float)(_rand() % 0x578 - 500) + *(float *)(DAT_07abf5d8 + 0x14);
                float fz = (float)(_rand() % 300 + 0x32)  + *(float *)(DAT_07abf5d8 + 0x18);
                pfVar10[-7] = fx; pfVar10[-6] = fy; pfVar10[-5] = fz;
                pfVar10[0xe] = fx; pfVar10[0xf] = fy; pfVar10[0x10] = fz;
                unsigned int ur;
                ur = (unsigned int)_rand() & 0x8000003fu; if ((int)ur<0) ur=(ur-1u|0xffffffc0u)+1u;
                float ang = (float)(int)(ur + 0x40) * _DAT_005529c8;
                pfVar10[8] = ang;
                if (fy < *(float *)&DAT_083a42d8 + _DAT_00552850) pfVar10[8] = _DAT_00552a3c - ang;
                ur=(unsigned int)_rand()&0x8000001fu; if((int)ur<0)ur=(ur-1u|0xffffffe0u)+1u; pfVar10[9]  =(float)(int)(ur-0x10)*_DAT_005524f4;
                ur=(unsigned int)_rand()&0x8000001fu; if((int)ur<0)ur=(ur-1u|0xffffffe0u)+1u; pfVar10[10] =(float)(int)(ur-0x10)*_DAT_005524f4;
                ur=(unsigned int)_rand()&0x8000000fu; if((int)ur<0)ur=(ur-1u|0xfffffff0u)+1u; pfVar10[0xb]=(float)(int)(ur-8)*_DAT_005524f4;
                ur=(unsigned int)_rand()&0x8000003fu; if((int)ur<0)ur=(ur-1u|0xffffffc0u)+1u; pfVar10[0xc]=(float)(int)(ur-0x20)*_DAT_005524f4;
                ur=(unsigned int)_rand()&0x8000000fu; if((int)ur<0)ur=(ur-1u|0xfffffff0u)+1u; pfVar10[0xd]=(float)(int)(ur-8)*_DAT_005524f4;
            }
        }
        else
        {
            // ── Update active particle ────────────────────────────────────────
            if (iVar7 == 9)
            {
                // Snow: integrate velocity
                pfVar10[-7] += pfVar10[8];
                pfVar10[-6] += pfVar10[9];
                pfVar10[-5] += pfVar10[10];
                float terZ = FUN_004f7500(pfVar10[-7], pfVar10[-6]);
                if (pfVar10[-5] < terZ) {
                    *(char *)(pfVar10 - 0xb) = 0;
                    pfVar10[-5] = terZ + (float)_DAT_00552488;
                    unsigned int r = (unsigned int)_rand() & 0x80000003u;
                    if ((int)r < 0) r = (r - 1u | 0xfffffffcu) + 1u;
                    if (r == 0)
                        Particle_Spawn(0x67, pfVar10 - 7, pfVar10 - 4, pfVar10 - 1, 0, 1.0f, 0);
                    else
                        Particle_Spawn(0x68, pfVar10 - 7, pfVar10 - 4, pfVar10 - 1, 0, 1.0f, 0);
                }
            }

            if (iVar7 == 0 || iVar7 == 10)
            {
                if (pfVar10[-10] == 1.42932e-43f)
                {
                    // Rain/snow falling: integrate
                    pfVar10[-7] += pfVar10[8];
                    pfVar10[-6] += pfVar10[9];
                    pfVar10[-5] += pfVar10[10];
                    float terZ = FUN_004f7500(pfVar10[-7], pfVar10[-6]);
                    if (pfVar10[-5] < terZ) {
                        *(char *)(pfVar10 - 0xb) = 0;
                        pfVar10[-5] = terZ + (float)_DAT_00552488;
                        if (iVar7 != 10)
                            Particle_Spawn(0x67, pfVar10 - 7, pfVar10 - 4, pfVar10 - 1, 0, 1.0f, 0);
                    }
                }
                else
                {
                    // Butterfly: random drift + distance limit
                    unsigned int ur;
                    ur=(unsigned int)_rand()&0x8000000fu; if((int)ur<0)ur=(ur-1u|0xfffffff0u)+1u;
                    pfVar10[8]  += (float)(int)(ur-8) * _DAT_005524f4;
                    ur=(unsigned int)_rand()&0x8000000fu; if((int)ur<0)ur=(ur-1u|0xfffffff0u)+1u;
                    pfVar10[9]  += (float)(int)(ur-8) * _DAT_005524f4;
                    ur=(unsigned int)_rand()&0x8000000fu; if((int)ur<0)ur=(ur-1u|0xfffffff0u)+1u;
                    float vz = (float)(int)(ur-8)*_DAT_005524f4 + pfVar10[10];
                    pfVar10[10] = vz;
                    pfVar10[-7] += pfVar10[8];
                    pfVar10[-6] += pfVar10[9];
                    pfVar10[-5] += vz;

                    ur=(unsigned int)_rand()&0x80000007u; if((int)ur<0)ur=(ur-1u|0xfffffff8u)+1u;
                    pfVar10[0xb] += (float)(int)(ur-4)*_DAT_00552914;
                    ur=(unsigned int)_rand()&0x8000000fu; if((int)ur<0)ur=(ur-1u|0xfffffff0u)+1u;
                    pfVar10[0xc] += (float)(int)(ur-8)*_DAT_00552914;
                    ur=(unsigned int)_rand()&0x80000007u; if((int)ur<0)ur=(ur-1u|0xfffffff8u)+1u;
                    float az = (float)(int)(ur-4)*_DAT_00552914 + pfVar10[0xd];
                    pfVar10[0xd] = az;
                    pfVar10[-4] += pfVar10[0xb];
                    pfVar10[-3] += pfVar10[0xc];
                    pfVar10[-2] += az;

                    // Deactivate if too far from origin
                    float dx = pfVar10[0xe]-pfVar10[-7], dy = pfVar10[0xf]-pfVar10[-6], dz = pfVar10[0x10]-pfVar10[-5];
                    if (_DAT_00552a38 <= dx*dx + dy*dy + dz*dz)
                        *(char *)(pfVar10 - 0xb) = 0;
                }
            }
            else
            {
                // Other states: fall if above terrain; fade alpha when on ground
                float terZ = FUN_004f7500(pfVar10[-7], pfVar10[-6]);
                if (terZ < pfVar10[-5]) {
                    // still falling
                    unsigned int ur;
                    ur=(unsigned int)_rand()&0x8000000fu; if((int)ur<0)ur=(ur-1u|0xfffffff0u)+1u;
                    pfVar10[8]  += (float)(int)(ur-8)*_DAT_005524f4;
                    ur=(unsigned int)_rand()&0x8000000fu; if((int)ur<0)ur=(ur-1u|0xfffffff0u)+1u;
                    pfVar10[9]  += (float)(int)(ur-8)*_DAT_005524f4;
                    ur=(unsigned int)_rand()&0x8000000fu; if((int)ur<0)ur=(ur-1u|0xfffffff0u)+1u;
                    float vz = (float)(int)(ur-8)*_DAT_005524f4 + pfVar10[10];
                    pfVar10[10] = vz;
                    pfVar10[-7] += pfVar10[8];
                    pfVar10[-6] += pfVar10[9];
                    pfVar10[-5] += vz;
                } else {
                    // on ground: fade RGB and deactivate
                    pfVar10[-5] = terZ;
                    pfVar10[-1] -= _DAT_00552874;
                    pfVar10[0]  -= _DAT_00552874;
                    pfVar10[1]  -= _DAT_00552874;
                    if (pfVar10[-1] <= _DAT_00552580)
                        *(char *)(pfVar10 - 0xb) = 0;
                }
            }
        }

        pfVar10 += 0x1c;
        local_d4++;
        iVar7 = DAT_0055a7ac;
    } while (local_d4 < iVar9);
}
