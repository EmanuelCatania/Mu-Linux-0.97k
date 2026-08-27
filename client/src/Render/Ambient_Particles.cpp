// Ambient_Particles.cpp
// IDA: FUN_00502320 @ 0x00502320 — AmbientParticles_Update
//
// Ambient_ParticleUpdate — per-frame ambient particle spawner/updater.
//
// Iterates a table of ambient particle slots at DAT_083a2f78 (base), stepping by 0x6f
// int-words (= 0x1bc = 444 bytes) up to 0x83A40CF. Index: local_20 (0..N).
//
// Slot layout (relative to puVar9 = entry pointer):
//   puVar9[-0x3a] (char)   — active flag (0=free, 1=active)
//   puVar9[-0x39] (int?)   — lifetime / counter
//   puVar9[-0x37] (float)  — angular speed
//   puVar9[-0x36] (float)  — pos_x
//   puVar9[-0x35] (float)  — pos_y
//   puVar9[-0x34] (float)  — pos_z (height/altitude)
//   puVar9[-0x33..] (float[3]) — velocity vector
//   puVar9[-0x2a..] (float[3]) — prev_pos (used by type 0xb3 lightning bolt)
//   puVar9[-0x24] (int)    — — (set to -1 = 0xffffffff)
//   puVar9[-0x22] (int)    — lifetime counter (decremented each frame)
//   puVar9[-0x21] (int)    — sprite Y-scale (0 or 1.0)
//   puVar9[-0x20] (float)  — sprite Y-scale value (animated by sine for 0xbc/0xbd)
//   puVar9[-0x1a] (float)  — oscillation phase angle
//   puVar9[-0x16..] (float[3]) — direction/normal vector
//   puVar9[-0x10] (float)  — cached pos_x (head of trail)
//   puVar9[-0x0f] (float)  — cached pos_y (head of trail)
//   puVar9[-7]  (float)    — speed (= life_speed / angular_speed)
//   puVar9[-4]  (float)    — size / scale
//   puVar9[-3]  (char)     — visible flag
//   puVar9[0..2] (float[3]) — color RGB
//   puVar9[0x1f] (float)   — sprite X-scale
//   puVar9[0x20] (int)     — ???
//   puVar9[0x79] (char)    — ???
//   puVar9 + -0xe6 (short) — particle type:
//       0xae = falling cloud / puff
//       0xaf = snow flake
//       0xb0 = rain drop (type 0=Login, type 1/4=CharSelect)
//       0xb1 = leaf / debris
//       0xb2 = flower petal (reverse)
//       0xb3 = lightning bolt
//       0xb5 = bubble
//       0xb8 = tree leaf
//       0xb9..0xbd = water splash variants
//       0xbc/0xbd = wave ring (sine-animated scale)
//       0xbf = rain splash
//       0x10a = firefly / butterfly (circular orbit)
//
// Spawning logic:
//   If slot is free (active==0):
//     - Check terrain type at player position via FUN_004f6c40(row, col)
//     - game state 0: tile type 5 (water) → 0xb5 (bubble)
//     - game state 1: terrain flags → 0xb5 or splash
//     - game state 7/8: snow / rain environment
//     - game state 2 (InWorld): leaf/flower/rain based on terrain
//   When spawning: place near player pos (rand ±512 tiles), set speed, type, lifetime
//
// Update logic:
//   If slot is active:
//     - FUN_00440aa0 = BMD_Animation update on the particle model
//     - Animate type-specific motion (rain falls, petal spins, firefly orbits)
//     - Move: pos += velocity * speed
//     - Kill if out of range from player (_DAT_00552d24 = max dist)
//     - Special: type 0xb1 can spawn sound FUN_00404bc0(0xf, ...) near player
//
// Globals:
//   DAT_083a2f78  — ambient particle table base
//   DAT_05828d58  — particle model table base (stride 0xbc, indexed by type)
//   DAT_07abf5d8  — local player entity ptr
//   DAT_0055a7ac  — g_GameSubState
//   DAT_0838bc70  — per-tile terrain attribute array
//   DAT_080bb2b4  — per-tile terrain type array (type 5 = water)
//   _DAT_005524f4 — PI/180 (deg→rad) or small angle step
//   _DAT_005524f8 — grid_to_world scale
//   _DAT_00552504 — 1.0f
//   _DAT_0055256c — 1.0f (duplicate)
//   _DAT_00552534 — period or speed constant
//   _DAT_00552d24 — despawn distance

#include "stdafx.h"

void __cdecl AmbientParticles_Update(void)
{
    float       *pfVar1;
    void        *this_model;
    unsigned char bVar2;
    float        fVar3, fVar4;
    short        sVar5;
    unsigned int uVar6;
    int          iVar7, iVar8;
    unsigned int *puVar9;
    bool         bVar10;
    float10      fVar11;
    long long    lVar12;
    int          local_20;     // slot index
    float        local_18, local_14, local_10;
    float        local_c[3];

    local_20 = 0;
    puVar9 = (unsigned int *)DAT_083a2f78;   // base of pool (now real array)
    iVar8  = DAT_0055a7ac;

    // BUG-FIX 2026-04-28: las dos comparaciones contra 0x83a34ac y 0x83a40cf
    // eran direcciones absolutas del binario original. Ahora computamos
    // bounds relativos al array real DAT_083a2f78 (10 × 0x1bc).
    // - Active range (slots 0..2) = primeros 3 × 0x1bc = 0x534 bytes desde base
    // - Total range (slots 0..9) = 10 × 0x1bc = 0x1158 bytes desde base
    const uintptr_t arrBase   = (uintptr_t)DAT_083a2f78;
    const uintptr_t arrActive = arrBase + 3 * 0x1bc;     // hasta slot 2 inclusive
    const uintptr_t arrEnd    = arrBase + 10 * 0x1bc;    // hasta slot 9 inclusive

    do {
        // ── Is this slot forced active (game state 7/8) or within range? ──────
        if (((iVar8 == 7) || (iVar8 == 8)) || ((uintptr_t)puVar9 < arrActive)) {

            // ── Slot is free: try to spawn ────────────────────────────────────
            if (*(char *)(puVar9 + -0x3a) == '\0') {
                pfVar1 = (float *)(puVar9 + -0x36);

                // Random position near player (±512 units in X, +200..600 in Y)
                uVar6 = _rand();
                uVar6 &= 0x800003ff;
                if ((int)uVar6 < 0) uVar6 = (uVar6 - 1 | 0xfffffc00) + 1;
                *pfVar1 = (float)(int)(uVar6 - 0x200) + *(float *)(DAT_07abf5d8 + 0x10);

                uVar6 = _rand();
                iVar8  = (int)DAT_07abf5d8;
                uVar6 &= 0x800003ff;
                if ((int)uVar6 < 0) uVar6 = (uVar6 - 1 | 0xfffffc00) + 1;
                puVar9[-0x35] = *(unsigned int *)((char *)&fVar3);  // pos_y = rand + player_y
                (void)fVar3;
                puVar9[-0x35] = (unsigned int)((float)(int)(uVar6 - 0x200) + *(float *)(DAT_07abf5d8 + 0x14));
                puVar9[-0x34] = *(unsigned int *)(iVar8 + 0x18);    // pos_z = player_z

                // Get terrain type at spawn position
                lVar12 = (long long)(unsigned int)__ftol();
                uVar6  = (unsigned int)lVar12;
                lVar12 = (long long)(unsigned int)__ftol();
                iVar7  = FUN_004f6c40((unsigned int)lVar12, uVar6);
                iVar8  = DAT_0055a7ac;

                // ── Spawn logic per game state ────────────────────────────────
                if (DAT_0055a7ac == 0) {
                    // Connecting state: water bubble on water tiles
                    if (*(char *)((int)&DAT_080bb2b4 + iVar7) == '\x05') goto LAB_0050245e;
                    goto switchD_caseD_2;
                }
                if (DAT_0055a7ac == 1) {
                    bVar2 = (unsigned char)DAT_0838bc70[iVar7];
                    goto joined_spawn_check;
                }

                // Fall through to update path for non-spawn states
                goto switchD_caseD_2;

joined_spawn_check:
                if (bVar2 < 8) {
LAB_0050245e:
                    // Initialize new particle slot
                    puVar9[0]    = 0x3f000000; // color R = 0.5
                    puVar9[1]    = 0x3f000000; // color G = 0.5
                    puVar9[2]    = 0x3f000000; // color B = 0.5
                    *(unsigned char *)(puVar9 + -0x3a) = 1;  // active = true
                    puVar9[0x20] = 0;
                    puVar9[0x1f] = 0x3f800000; // x-scale = 1.0
                    puVar9[-0x21] = 0xffffffff;
                    puVar9[-0x33] = 0;  // vel_x = 0
                    puVar9[-0x32] = 0;  // vel_y = 0
                    puVar9[-0x31] = 0;  // vel_z = 0
                    *(unsigned char *)(puVar9 + -3) = 1;
                    *(unsigned char *)((int)puVar9 + 0x79) = 1;
                    puVar9[-0x39] = 0;
                    puVar9[-0x24] = 0xffffffff;

                    uVar6 = _rand();
                    uVar6 &= 0x8000007f;
                    if ((int)uVar6 < 0) uVar6 = (uVar6 - 1 | 0xffffff80) + 1;
                    puVar9[-0x22] = uVar6;  // lifetime = rand & 0x7f

                    uVar6 = _rand();
                    iVar8 = DAT_0055a7ac;
                    uVar6 &= 0x80000003;
                    if ((int)uVar6 < 0) uVar6 = (uVar6 - 1 | 0xfffffffc) + 1;
                    puVar9[-4]    = 0x41500000; // size = 13.0f
                    *(unsigned char *)((int)puVar9 + -0x5e) = 1;

                    fVar3 = (float)(int)(uVar6 + 4) * _DAT_005524f4;
                    puVar9[-0x37] = (unsigned int)(*(int *)&fVar3);  // angular speed

                    // Assign type by game sub-state
                    switch (iVar8) {
                    case 0:
                        *(short *)((int)puVar9 + -0xe6) = 0xb5;  // bubble
                        uVar6 = _rand();
                        uVar6 &= 0x80000001;
                        if ((int)uVar6 < 0) uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
                        puVar9[0x1f] = (unsigned int)((float)(int)(uVar6 + 2) * _DAT_005524f4);
                        puVar9[-7]   = (unsigned int)(_DAT_00552534 / (float)puVar9[-0x37]);
                        iVar8 = DAT_0055a7ac;
                        break;
                    case 1:
                        fVar3 = _DAT_00552534 / fVar3;
                        *(short *)((int)puVar9 + -0xe6) = 0xb1;  // leaf
                        puVar9[-7] = (unsigned int)(*(int *)&fVar3);
                        break;
                    case 6:
                        fVar3 = _DAT_00552534 / fVar3;
                        *(short *)((int)puVar9 + -0xe6) = 0xb2;  // petal
                        puVar9[-7] = (unsigned int)(*(int *)&fVar3);
                        break;
                    case 7:
                        // Snow: y-position check for altitude
                        uVar6 = _rand();
                        iVar8 = (int)DAT_07abf5d8;
                        uVar6 &= 0x80000001;
                        if ((int)uVar6 < 0) uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
                        puVar9[-0x37] = (unsigned int)((float)(int)(uVar6 + 8) * _DAT_005524f4);
                        // More snow-specific initialization follows in binary...
                        iVar8 = DAT_0055a7ac;
                        break;
                    case 8:
                        // Type 0xb3 — lightning bolt
                        *(short *)((int)puVar9 + -0xe6) = 0xb3;
                        uVar6 = _rand();
                        uVar6 &= 0x80000003;
                        if ((int)uVar6 < 0) uVar6 = (uVar6 - 1 | 0xfffffffc) + 1;
                        puVar9[-0x2a] = *(unsigned int *)pfVar1;  // prev_pos_x = pos_x
                        fVar3 = (float)(int)(uVar6 + 8) * _DAT_005524f4;
                        puVar9[-0x37] = (unsigned int)(*(int *)&fVar3);
                        fVar3 = _DAT_00552950 / fVar3;
                        puVar9[-4]    = 0x41100000; // size = 9.0f
                        puVar9[-0x22] = 100;
                        puVar9[-0x29] = puVar9[-0x35]; // prev_pos_y = pos_y
                        puVar9[-0x28] = puVar9[-0x34]; // prev_pos_z = pos_z
                        puVar9[-7]    = (unsigned int)(*(int *)&fVar3);
                        Joint_Create(0x4ea, pfVar1, pfVar1,
                                     (float *)(puVar9 + -0x33), 4,
                                     (int)(puVar9 + -0x3a),
                                     30.0f, -1, 0);
                        iVar8 = DAT_0055a7ac;
                        break;
                    }
                }
            } else {
                // ── Slot is active: select model and update ───────────────────
switchD_caseD_2:
                if (*(char *)(puVar9 + -0x3a) == '\0') goto LAB_00502b38;
            }
        }

        // ── Active slot update ────────────────────────────────────────────────
        if (*(char *)(puVar9 + -0x3a) != '\0') {
            pfVar1    = (float *)(puVar9 + -0x36);
            short modelIdx = *(short *)((int)puVar9 + -0xe6);
            // BUG-FIX 2026-04-28: el modelIdx puede venir garbage (ej. de un
            // slot reciclado de char-select). Models[] válido en 0..~0x4A8.
            if (modelIdx < 0 || modelIdx > 0x4A8) goto LAB_00502b38;
            this_model = (void *)(DAT_05828d58 + modelIdx * 0xbc);

            // Animate model
            *(unsigned char *)((int)this_model + 0xa0) = *(unsigned char *)((int)puVar9 + 0x1d);
            FUN_00440aa0(this_model,
                         (float *)(puVar9 + 8),
                         (float *)(puVar9 + 9),
                         (unsigned char *)((int)puVar9 + 0x1e),
                         (float)puVar9[-7] * _DAT_00552504);

            sVar5 = *(short *)((int)puVar9 + -0xe6);

            // Wave ring: animate scale with sine
            if ((sVar5 == 0xbc) || (sVar5 == 0xbd)) {
                fVar11 = (float10)fsin((float10)(float)puVar9[-0x1a]);
                puVar9[-0x20] = (unsigned int)(float)(fVar11 * (float10)_DAT_005528b4
                                                      + (float10)_DAT_00552504);
                puVar9[-0x1a] = (unsigned int)((float)puVar9[-0x1a] + _DAT_005524f4);
            }

            // Despawn if out-of-type or expired
            if (((sVar5 < 0xb5) || (0xbf < sVar5)) && ((int)puVar9[-0x22] < 1)) {
                FUN_0043e820((int)(puVar9 + -0x3a), 0);
            } else {
                FUN_0043e820((int)(puVar9 + -0x3a), (unsigned int)(sVar5 == 0xb2));
                FUN_0043e680((int)(puVar9 + -0x3a), local_20, (int)(uintptr_t)DAT_083a2e90, 10);
                Matrix_BuildFromEuler((float *)(puVar9 + -0x33), (float *)(puVar9 + -0x16));

                uVar6 = _rand();
                uVar6 &= 0x80000003;
                if ((int)uVar6 < 0) uVar6 = (uVar6 - 1 | 0xfffffffc) + 1;
                local_c[1] = 0.0f;
                local_c[0] = (float)(int)(uVar6 + 6) * (float)puVar9[-7];
                local_c[2] = 0.0f;
                Vector_Rotate(local_c, (float *)(puVar9 + -0x16), &local_18);

                bVar10 = (DAT_0055a7ac != 7);
                *pfVar1 = local_18 + *pfVar1;            // pos_x += vel_x
                puVar9[-0x35] = (unsigned int)(local_14 + (float)puVar9[-0x35]); // pos_y
                puVar9[-0x34] = (unsigned int)(local_10 + (float)puVar9[-0x34]); // pos_z

                if (bVar10) {
                    // BUG-FIX 2026-04-28: pass explicit (xf, yf) — pos at pfVar1[0/1]
                    float __h = FUN_004f7500(*pfVar1, *(float*)(puVar9 - 0x35));
                    puVar9[-0x34] = *(unsigned int*)&__h;
                    fVar11 = (float10)__h;
                }

                // Compute head position (leading edge offset)
                puVar9[-10] = (unsigned int)(local_18 * _DAT_00552540 + *pfVar1);
                puVar9[-9]  = (unsigned int)(local_14 * _DAT_00552540 + (float)puVar9[-0x35]);

                // Terrain type check for special effects
                lVar12 = (long long)(unsigned int)__ftol();
                uVar6  = (unsigned int)lVar12;
                lVar12 = (long long)(unsigned int)__ftol();
                iVar8  = FUN_004f6c40((unsigned int)lVar12, uVar6);
                sVar5  = *(short *)((int)puVar9 + -0xe6);

                // Transition to water or bounce on land
                if (((sVar5 == 0xb5) &&
                     (*(char *)((int)&DAT_080bb2b4 + iVar8) != '\x05')) ||
                    ((sVar5 == 0xb1) && (7 < (unsigned char)(unsigned char)DAT_0838bc70[iVar8])))
                {
                    // Rolling angle
                    fVar3 = (float)puVar9[-0x31] + _DAT_005524ec;
                    puVar9[-0x31] = (unsigned int)(*(int *)&fVar3);
                    if (_DAT_0055286c <= fVar3)
                        puVar9[-0x31] = (unsigned int)(fVar3 - _DAT_0055286c);
                    iVar8 = puVar9[-0x39] + 1;
                    puVar9[-0x39] = iVar8;
                } else {
                    if (DAT_0055a7ac == 7 || DAT_0055a7ac == 8) {
                        bVar2 = (unsigned char)DAT_0838bc70[iVar8];
                        if ((bVar2 == 1) || (7 < bVar2)) {
                            fVar3 = (float)puVar9[-0x31] + _DAT_005524ec;
                            puVar9[-0x31] = (unsigned int)(*(int *)&fVar3);
                            if (_DAT_0055286c <= fVar3)
                                puVar9[-0x31] = (unsigned int)(fVar3 - _DAT_0055286c);
                            iVar8 = puVar9[-0x39] + 1;
                            puVar9[-0x39] = iVar8;
                            goto skip_decrement;
                        }
                    }
                    if (0 < (int)puVar9[-0x39]) {
                        iVar8 = (int)puVar9[-0x39] - 1;
                        puVar9[-0x39] = iVar8;
                    }
                }
skip_decrement:
                if (1 < (int)puVar9[-0x39])
                    *(unsigned char *)(puVar9 + -0x3a) = 0;  // deactivate

                // Type 0xb3 (lightning): save prev pos
                if (sVar5 == 0xb3) {
                    puVar9[-0x2a] = *(unsigned int *)pfVar1;
                    puVar9[-0x29] = puVar9[-0x35];
                    puVar9[-0x28] = puVar9[-0x34];
                }

                // Despawn if too far from player
                fVar3 = *pfVar1 - *(float *)(DAT_07abf5d8 + 0x10);
                fVar4 = (float)puVar9[-0x35] - *(float *)(DAT_07abf5d8 + 0x14);
                fVar3 = SQRT(fVar3 * fVar3 + fVar4 * fVar4);
                if (_DAT_00552d24 <= fVar3)
                    *(unsigned char *)(puVar9 + -0x3a) = 0;

                // Type 0xb1 (leaf): chance to play sound near player
                if ((fVar3 < _DAT_00552ab4) && (sVar5 == 0xb1)) {
                    uVar6 = _rand();
                    uVar6 &= 0x800000ff;
                    bVar10 = (uVar6 == 0);
                    if ((int)uVar6 < 0)
                        bVar10 = ((uVar6 - 1 | 0xffffff00) == 0xffffffff);
                    if (bVar10)
                        FUN_00404bc0(0xf, (int)(puVar9 + -0x3a), 0);
                }
            }

            // Decrement lifetime, possibly kill
            iVar8 = (int)puVar9[-0x22];
            puVar9[-0x22] = (unsigned int)(iVar8 - 1);
            if ((iVar8 - 1 < 1) && (*(short *)((int)puVar9 + -0xe6) != 0xb2)) {
                uVar6 = _rand();
                uVar6 &= 0x8000003f;
                bVar10 = (uVar6 == 0);
                if ((int)uVar6 < 0)
                    bVar10 = ((uVar6 - 1 | 0xffffffc0) == 0xffffffff);
                if (bVar10) {
                    uVar6 = _rand();
                    uVar6 &= 0x8000007f;
                    if ((int)uVar6 < 0) uVar6 = (uVar6 - 1 | 0xffffff80) + 1;
                    puVar9[-0x22] = uVar6;  // re-randomize lifetime
                }
            }

            FUN_0043e5c0((int)(puVar9 + -0x3a));  // commit particle state
            iVar8 = DAT_0055a7ac;
        }

LAB_00502b38:
        puVar9  = puVar9 + 0x6f;   // stride: 0x1bc bytes per slot
        local_20++;
        if ((uintptr_t)puVar9 >= arrEnd) return;
    } while (true);
}
