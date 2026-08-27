// Weather.cpp
// FUN_00500e80 @ 0x00500E80
//
// Weather_Update — per-frame weather particle system update (846 lines).
//
// Manages 40 weather object slots at DAT_0839bcb0 (stride 0x1bc = 444 bytes).
// Also uses HashTable anti-tamper pattern around DAT_083a7c00.
//
// Weather types (short @ slot+2):
//   0xae  = falling cloud / puff (LoginScene)
//   0xaf  = snow flake
//   0xb0  = rain drop
//   0xb6  = snow variant A (g_GameSubState==7)
//   0xb7  = snow variant B
//   0xb8  = tree leaves / debris
//   0x10a = firefly / butterfly (circular orbit with sin/cos)
//   0x12d = thunder cloud (DAT_083a3ff0 == rain-mode)
//   0x1f  = thunder sound (played via FUN_005098c0)
//
// Slot layout (all offsets relative to iVar14, stride=0x1bc):
//   pcVar3         = &DAT_0839bcb0 + iVar14*0x1bc  — slot base
//   pcVar3[0]      — active flag (char)
//   iVar2          = iVar14 * 0x1bc                — byte offset
//   (&DAT_0839bcb2)[iVar14*0xde] — type (short, stride=0xde*2=0x1bc/1)
//   (&DAT_0839bcb4)[iVar14*0x6f] — mode flag (char)
//   (&DAT_0839bcbc)[iVar14*0x6f] — angular speed (float)
//   (&DAT_0839bcc0)[iVar14*0x6f] — pos_x (float)
//   (&DAT_0839bcc4)[iVar14*0x6f] — pos_y (float)
//   (&DAT_0839bcc8)[iVar14*0x6f] — pos_z / altitude (float)
//   (&DAT_0839bccc)[iVar14*0x6f] — vel_x (float)
//   (&DAT_0839bcd0)[iVar14*0x6f] — vel_y (float)
//   (&DAT_0839bcd4)[iVar14*0x6f] — phase angle (float)
//   (&DAT_0839bd08)[iVar14*0x6f] — color R or -1
//   (&DAT_0839bd14)[iVar14*0x6f] — color G or -1
//   (&DAT_0839bd30)[iVar14*0x6f] — rotation angle (float)
//   (&DAT_0839bd40+iVar2)        — direction vector (float[3])
//   (&DAT_0839bd70+iVar2)        — cached head pos X (float)
//   (&DAT_0839bd74+iVar2)        — cached head pos Y (float)
//   (&DAT_0839bd78+iVar2)        — velocity Z (float)
//   (&DAT_0839bd7c)[iVar14*0x6f] — scale (float)
//   (&DAT_0839bd84+iVar2)        — size (float)
//   (&DAT_0839bd88+iVar2)        — altitude limit (float)
//   (&DAT_0839bd8c)[iVar14*0x6f] — render flag (char)
//   (&DAT_0839bd98+iVar2)        — color R (float)
//   (&DAT_0839bd9c+iVar2)        — color G (float)
//   (&DAT_0839bda0+iVar2)        — color B (float)
//   (&DAT_0839bdb4)[iVar14*1]    — state byte (cloud life phase 0..3)
//   (&DAT_0839bdb5)[iVar14*1]    — blend mode
//   (&DAT_0839bdb6+iVar2)        — ???
//   (&DAT_0839bdb8+iVar2)        — rotation float
//   (&DAT_0839bdbc+iVar2)        — ???
//   (&DAT_0839be11)[iVar2]       — something_byte
//   (&DAT_0839be14)[iVar14*0x6f] — alpha fade
//   (&DAT_0839be18)[iVar14*0x6f] — alpha start
//
// Cloud life phases (state byte at bdb4):
//   0 = rising:   check distance, set altitude
//   1 = hovering: bob up/down via altitude offset
//   2 = descending: wait for player anim then transition
//   3 = falling away: reduce scale until gone
//
// Firefly/butterfly (0x10a) orbit logic:
//   pos_x += sin(phase) * scale
//   pos_y -= cos(phase) * scale
//   phase += cos(sin(...) * angle_increment)
//   Kill if > max_dist from player or random 1/5120 chance
//
// Globals:
//   DAT_0839bcb0   — weather slot table base (40 slots × 0x1bc bytes)
//   DAT_05828d58   — particle model table base (stride 0xbc, indexed by type)
//   DAT_07abf5d8   — local player entity ptr
//   DAT_083a3ff0   — weather mode (0=sun/clear, 1=rain, 2=snow, 3=storm)
//   DAT_0838bc70   — per-tile terrain attribute array
//   DAT_0055a7ac   — g_GameSubState
//   DAT_055c9bc8   — HashTable context (anti-tamper)
//   DAT_083a7c00   — anti-tamper key
//   _DAT_005524f0  — world grid scale
//   _DAT_005524f4  — PI/180
//   _DAT_005524f8  — grid unit
//   _DAT_00552464  — particle speed scale
//   _DAT_005526e4  — frame delta time
//   _DAT_00552874  — angle increment
//   _DAT_00552900  — altitude_max
//   _DAT_00552914  — scale step
//   _DAT_00552940  — scale decay
//   _DAT_0055285c  — min altitude
//   _DAT_00552850  — max altitude
//   _DAT_00552ab4  — near distance (sound trigger)
//   _DAT_00552d24  — far distance (despawn)
//   _DAT_00552d28  — orbit angle step
//   FUN_0043e5c0   — commit particle / advance state
//   FUN_0043e680   — particle path update
//   FUN_0043e820   — set particle animation
//   FUN_00440aa0   — BMD_Animation tick
//   FUN_004f76c0   — spawn ground ripple
//   Matrix_BuildFromEuler   — Vec3_Normalize or direction update
//   Vector_Rotate   — Vec3_Scale (multiply direction by speed)
//   FUN_005098c0   — play thunder sound
//   FUN_005129f0   — terrain height at angle

#include "stdafx.h"
// Decompilation artifacts: suppress truncation/conversion/uninitialized warnings
#pragma warning(disable: 4244 4305 4309 4700)

uint __cdecl FUN_00500e80(void)
{
    float  *pfVar1;
    int     iVar2;
    char   *pcVar3;
    char    cVar4;
    short   sVar5;
    float   fVar7;
    uint    uVar8;
    void   *pvVar9;
    DWORD  *puVar10;
    int     iVar11, iVar12;
    uint    uVar13;
    int     iVar14;
    DWORD  *puVar16;
    unsigned char *pbVar17, *pbVar18;
    char   *pcVar19;
    bool    bVar20, bVar21;
    float10 fVar22, fVar23;
    long long lVar24;
    float   fVar25;
    uint    uStack_48;
    DWORD *puStack_44;
    DWORD *local_40;
    int     iStack_3c;
    float   fStack_38, fStack_34, fStack_30;
    float   fStack_2c, fStack_28, fStack_24;
    float   fStack_20, fStack_1c, fStack_18;
    float   fStack_14, fStack_10, fStack_c;

    // ── Anti-tamper: HashTable reference-count increment for DAT_083a7c00 ─────
    local_40 = &DAT_083a7c00;
    uVar8 = (**(unsigned int (**)(void *))((char *)DAT_055c9bc8 + 0xc))(&DAT_083a7c00);
    uVar13 = 0;
    local_40 = (DWORD *)0x0;
    if (DAT_055c9bd4 != 0) {
        do {
            bVar20 = false; iVar14 = 0; bVar21 = true;
            pbVar18 = (unsigned char *)(DAT_055c9bd0 + uVar8 * 4);
            iVar12 = 4; DWORD **ppuVar15 = &local_40; pbVar17 = pbVar18;
            do {
                if (iVar12 == 0) break;
                iVar12--; bVar20 = *(unsigned char *)ppuVar15 < *pbVar17;
                bVar21 = *(unsigned char *)ppuVar15 == *pbVar17;
                ppuVar15 = (DWORD **)((int)ppuVar15 + 1); pbVar17++;
            } while (bVar21);
            if (!bVar21) iVar14 = (1 - (uint)bVar20) - (uint)(bVar20 != 0);
            if (iVar14 == 0) goto LAB_00500f16;
            iVar12 = 4; bVar20 = false; iVar14 = 0; bVar21 = true;
            puVar16 = (DWORD*)&uStack_48;
            do {
                if (iVar12 == 0) break;
                iVar12--; bVar20 = (unsigned char)*puVar16 < *pbVar18;
                bVar21 = (unsigned char)*puVar16 == *pbVar18;
                puVar16 = (DWORD*)((int)puVar16 + 1); pbVar18++;
            } while (bVar21);
            if (!bVar21) iVar14 = (1 - (uint)bVar20) - (uint)(bVar20 != 0);
            if (iVar14 == 0) {
                if (uVar8 == 0xffffffff) goto LAB_00500f16;
                uVar8 = HashTable_GetIndex(&DAT_055c9bc8, &DAT_083a7c00);
                puVar10 = (uVar8 == 0xffffffff) ? (DWORD *)0x0
                                                 : *(DWORD **)(DAT_055c9bcc + uVar8 * 4);
                cVar4 = *(char *)(puVar10 + 1);
                *(unsigned char *)(puVar10 + 1) = cVar4 + 1U;
                if ((unsigned char)(cVar4 + 1U) < 2)
                    FUN_00409e20(&DAT_083a7c00, puVar10);
                goto LAB_00500f34;
            }
            uVar8 = (uVar8 + 1) % DAT_055c9bd4;
            uVar13++;
        } while (uVar13 < DAT_055c9bd4);
    }
    FUN_00405540(&DAT_055c9bf0, s_Hash_table_full______GetIndex_00558108);
LAB_00500f16:
    pvVar9 = operator_new(5);
    *(unsigned char *)((int)pvVar9 + 4) = 1;
    FUN_00403f80(&DAT_055c9bc8, pvVar9, &DAT_083a7c00);
LAB_00500f34:
    // Anti-tamper: HashTable reference-count decrement
    fStack_38 = DAT_083a7c00;
    puStack_44 = &DAT_083a7c00;
    uVar8 = (**(unsigned int (**)(void *))((char *)DAT_055c9bc8 + 0xc))(&DAT_083a7c00);
    uVar13 = 0;
    puStack_44 = (DWORD *)0x0;
    if (DAT_055c9bd4 != 0) {
        do {
            iVar12 = 4; bVar20 = false; iVar14 = 0; bVar21 = true;
            pbVar18 = (unsigned char *)(DAT_055c9bd0 + uVar8 * 4);
            DWORD **ppuVar15b = &puStack_44; pbVar17 = pbVar18;
            do {
                if (iVar12 == 0) break;
                iVar12--; bVar20 = *(unsigned char *)ppuVar15b < *pbVar17;
                bVar21 = *(unsigned char *)ppuVar15b == *pbVar17;
                ppuVar15b = (DWORD **)((int)ppuVar15b + 1); pbVar17++;
            } while (bVar21);
            if (!bVar21) iVar14 = (1 - (uint)bVar20) - (uint)(bVar20 != 0);
            if (iVar14 == 0) goto LAB_00501064;
            iVar12 = 4; bVar20 = false; iVar14 = 0; bVar21 = true;
            puVar16 = (DWORD*)&uStack_48;
            do {
                if (iVar12 == 0) break;
                iVar12--; bVar20 = (unsigned char)*puVar16 < *pbVar18;
                bVar21 = (unsigned char)*puVar16 == *pbVar18;
                puVar16 = (DWORD *)((int)puVar16 + 1); pbVar18++;
            } while (bVar21);
            if (!bVar21) iVar14 = (1 - (uint)bVar20) - (uint)(bVar20 != 0);
            if (iVar14 == 0) {
                if (uVar8 != 0xffffffff) {
                    uVar8 = HashTable_GetIndex(&DAT_055c9bc8, &DAT_083a7c00);
                    puVar10 = (uVar8 == 0xffffffff) ? (DWORD *)0x0
                                                     : *(DWORD **)(DAT_055c9bcc + uVar8 * 4);
                    cVar4 = *(char *)(puVar10 + 1);
                    *(char *)(puVar10 + 1) = cVar4 - 1;
                    if ((char)(cVar4 - 1) == '\0')
                        FUN_00423760(puVar10, &DAT_083a7c00);
                }
                goto LAB_00501064;
            }
            uVar8 = (uVar8 + 1) % DAT_055c9bd4;
            uVar13++;
        } while (uVar13 < DAT_055c9bd4);
    }
    FUN_00405540(&DAT_055c9bf0, s_Hash_table_full______GetIndex_00558108);
LAB_00501064:

    // ── Weather-mode: random rain effects (DAT_083a3ff0 != 0) ─────────────────
    uVar8 = (uint)(uintptr_t)DAT_07abf5d8;
    if (DAT_083a3ff0 != 0) {
        // Spawn occasional rain splash particle (1/40 chance)
        iVar12 = _rand();
        if (iVar12 % 0x28 == 0) {
            iVar12 = _rand();
            puStack_44 = (DWORD *)(iVar12 % 600 + -200);
            fStack_20 = (float)(int)puStack_44 + *(float *)(DAT_07abf5d8 + 0x10);
            iVar12 = _rand();
            puStack_44 = (DWORD *)(iVar12 % 400 + 200);
            fStack_1c = (float)(int)puStack_44 + *(float *)(DAT_07abf5d8 + 0x14);
            fStack_18 = *(float *)(DAT_07abf5d8 + 0x18) + _DAT_00552900;
            fStack_2c = 0.0f; fStack_28 = 0.0f; fStack_24 = 0.0f;
            fStack_38 = 1.0f; fStack_34 = 1.0f; fStack_30 = 1.0f;
            Effect_Create(0xbf, &fStack_20, &fStack_2c, &fStack_38,
                         (float *)0x3, (float *)0x0, (float *)0xffffffff, (float *)0x0, 0);
            FUN_00404bc0(0x2e, 0, 0);
        }
        // Ground ripple effect
        fStack_38 = -0.3f; fStack_34 = -0.3f; fStack_30 = -0.2f;
        FUN_004f76c0(*(float *)(uVar8 + 0x10), *(float *)(uVar8 + 0x14), (int)&fStack_38, 0x10, (int)&DAT_081cb608[0]);
    }

    // ── g_GameSubState == 10: spawn distant lightning clouds ─────────────────
    if ((DAT_0055a7ac == 10) && (_rand() % 10 == 0)) {
        iVar12 = _rand();
        puStack_44 = (DWORD *)(iVar12 % 5000 + -0x9c4);
        fStack_38 = (float)(int)puStack_44 + *(float *)(DAT_07abf5d8 + 0x10);
        iVar12 = _rand();
        puStack_44 = (DWORD *)(iVar12 % 5000 + -0x9c4);
        fStack_34 = (float)(int)puStack_44 + *(float *)(DAT_07abf5d8 + 0x14);
        fStack_30 = *(float *)(DAT_07abf5d8 + 0x18) - _DAT_00552ab8;
        fStack_20 = 1.0f; fStack_1c = 1.0f; fStack_18 = 1.0f;
        fStack_2c = 0.0f; fStack_28 = 0.0f; fStack_24 = 0.0f;
        Effect_Create(0x47e, &fStack_38, &fStack_2c, &fStack_20,
                     (float *)0x0, (float *)0x0, (float *)0xffffffff, (float *)0x0, 0);
    }

    // Get terrain tile at player position
    lVar24 = (long long)(unsigned int)__ftol();
    uVar8  = (uint)lVar24;
    lVar24 = (long long)(unsigned int)__ftol();
    uVar8  = FUN_004f6c40((uint)lVar24, uVar8);
    iVar14 = 0;
    iVar12 = DAT_0055a7ac;
    uStack_48 = uVar8;

    // ── Main weather slot loop (40 slots) ─────────────────────────────────────
    do {
        // Limit active slots by game state
        if (iVar12 != 7) {
            if (iVar12 == 10) {
                if (0xc < iVar14) return uVar8;
            } else if (4 < iVar14) return uVar8;
        }

        iVar2    = iVar14 * 0x1bc;
        pcVar3   = &DAT_0839bcb0 + iVar2;

        if ((&DAT_0839bcb0)[iVar14 * 0x1bc] == '\0') {
            // ── Free slot: try to spawn new weather object ───────────────────
            if (DAT_083a3ff0 != 0) {
                // Thunder/storm mode: random thunder cloud (1/300 chance)
                iVar11 = _rand();
                uVar8  = iVar11 / 300;
                iVar12 = DAT_0055a7ac;
                if (iVar11 % 300 == 0) {
                    *pcVar3 = '\x01';
                    FUN_005098c0(0x1f);   // play thunder sound
                    (&DAT_0839bcb2)[iVar14 * 0xde] = 0x12d;  // type = lightning
                    iVar12 = _rand();
                    (*(unsigned int*)&g_WeatherSlotPool[0x168 + iVar14 * 0x1bc]) = 0x3f800000;
                    (&DAT_0839be14)[iVar14 * 0x6f] = 0x3f800000;
                    (&DAT_0839bd7c)[iVar14 * 0x6f] = 0x3f000000;
                    (&DAT_0839bd8c)[iVar2] = 1;
                    (&DAT_0839be11)[iVar2] = 0;
                    (&DAT_0839bcb4)[iVar14 * 0x6f] = 0;
                    (&DAT_0839bd08)[iVar14 * 0x6f] = 0xffffffff;
                    (&DAT_0839bd14)[iVar14 * 0x6f] = 0xffffffff;
                    (&DAT_0839bcbc)[iVar14 * 0x6f] = (float)(iVar12 % 3 + 6) * _DAT_005524f4;
                    uVar8 = _rand(); uVar8 &= 0x8000007f;
                    if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffff80) + 1;
                    (&DAT_0839bd10)[iVar14 * 0x6f] = uVar8 + 0x80;
                    iVar11 = _rand();
                    (&DAT_0839bccc)[iVar14 * 0x6f] = 0;
                    iVar12 = DAT_083a3ff0;
                    (&DAT_0839bcd0)[iVar14 * 0x6f] = 0;
                    (&DAT_0839bcd4)[iVar14 * 0x6f] = 0xc2b40000;
                    (&DAT_0839bd30)[iVar14 * 0x6f] = (float)(iVar11 % 10) * _DAT_005524f4;
                    if (iVar12 == 3)
                        (&DAT_0839bcb4)[iVar14 * 0x6f] = 1;
                    iVar12 = _rand();
                    (&DAT_0839bcc0)[iVar14 * 0x6f] = (float)(iVar12 % 600 + -100) + *(float *)(DAT_07abf5d8 + 0x10);
                    iVar11 = _rand();
                    uVar8 = (uint)(uintptr_t)DAT_07abf5d8;
                    iVar12 = DAT_0055a7ac;
                    (&DAT_0839bcc4)[iVar14 * 0x6f] = (float)(iVar11 % 400 + 200) + *(float *)(DAT_07abf5d8 + 0x14);
                    (&DAT_0839bcc8)[iVar14 * 0x6f] = *(float *)(uVar8 + 0x18) + _DAT_00552900;
                }
                goto LAB_0050172b;
            }

            // Non-storm: spawn by game state
            if (((iVar12 == 0) || (iVar12 == 1)) || (iVar12 == 3) ||
                (iVar12 == 4) || (iVar12 == 10))
            {
                // Clear all 0x1bc bytes of slot, then initialize
                char *pcVar19b = pcVar3;
                for (int n = 0x6f; n != 0; n--) {
                    pcVar19b[0] = '\0'; pcVar19b[1] = '\0';
                    pcVar19b[2] = '\0'; pcVar19b[3] = '\0';
                    pcVar19b += 4;
                }
                *pcVar3 = '\x01';
                (&DAT_0839bd7c)[iVar14 * 0x6f] = 0x3f800000;   // scale = 1.0
                (&DAT_0839bd8c)[iVar2] = 1;
                (&DAT_0839bd10)[iVar14 * 0x6f] = 0;
                (&DAT_0839bcb4)[iVar14 * 0x6f] = 0;
                *(unsigned int *)(&DAT_0839bd98 + iVar2) = 0x3f000000;
                *(unsigned int *)(&DAT_0839bd9c + iVar2) = 0x3f000000;
                *(unsigned int *)(&DAT_0839bda0 + iVar2) = 0x3f000000;
                (*(unsigned int*)&g_WeatherSlotPool[0x168 + iVar14 * 0x1bc]) = 0;
                (&DAT_0839be14)[iVar14 * 0x6f] = 0x3f800000;
                *(unsigned int *)(&DAT_0839bd88 + iVar2) = 0x41500000;
                (&DAT_0839bdb4)[iVar2] = 0;
                (&DAT_0839bdb5)[iVar2] = 0;

                // Select type by game sub-state
                if (iVar12 == 0) {
                    (&DAT_0839bcb2)[iVar14 * 0xde] = 0xae;   // cloud (connecting)
                } else if ((iVar12 == 1) || (iVar12 == 4)) {
                    (&DAT_0839bcb2)[iVar14 * 0xde] = 0xb0;   // rain (entering)
                } else if (iVar12 == 3) {
                    (&DAT_0839bcb2)[iVar14 * 0xde] = 0xaf;   // snow
                    (&DAT_0839bd7c)[iVar14 * 0x6f] = 0x3e99999a;
                    (&DAT_0839bd8c)[iVar2] = 0;
                } else if (iVar12 == 10) {
                    if (iVar14 < 3) {
                        // Lightning slots 0-2 in loading state
                        *pcVar3 = '\x01';
                        FUN_005098c0(0x1f);
                        (&DAT_0839bcb2)[iVar14 * 0xde] = 0x12d;
                        // (detailed init same as storm mode above)
                        goto LAB_0050172b;
                    }
                    (&DAT_0839bcb2)[iVar14 * 0xde] = 0x10a;  // firefly
                    (&DAT_0839bd7c)[iVar14 * 0x6f] = 0x400ccccd;
                    (&DAT_0839bd8c)[iVar2] = 0;
                    (&DAT_0839bd10)[iVar14 * 0x6f] = 0x2580;
                    (&DAT_0839bdb4)[iVar2] = 2;
                }

                *(unsigned int *)(&DAT_0839bd98 + iVar2) = 0x3f800000;
                *(unsigned int *)(&DAT_0839bd9c + iVar2) = 0x3f800000;
                *(unsigned int *)(&DAT_0839bda0 + iVar2) = 0x3f800000;

                (&DAT_0839be11)[iVar2] = 1;
                (&DAT_0839bcbc)[iVar14 * 0x6f] = 0x3f4ccccd;
                *(unsigned int *)(&DAT_0839bd84 + iVar2) = 0x41200000;
                (&DAT_0839bd08)[iVar14 * 0x6f] = 0xffffffff;
                (&DAT_0839bd14)[iVar14 * 0x6f] = 0xffffffff;
                iVar12 = _rand();
                pfVar1 = (float *)(&DAT_0839bcc0 + iVar14 * 0x6f);
                (&DAT_0839bd30)[iVar14 * 0x6f] = (float)(iVar12 % 0x13a) * _DAT_005524f8;
                uVar8 = _rand(); uVar8 &= 0x800003ff;
                if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffc00) + 1;
                *pfVar1 = (float)(int)(uVar8 - 0x200) + *(float *)(DAT_07abf5d8 + 0x10);
                uVar13 = _rand(); uVar8 = (uint)(uintptr_t)DAT_07abf5d8;
                uVar13 &= 0x800003ff;
                if ((int)uVar13 < 0) uVar13 = (uVar13 - 1 | 0xfffffc00) + 1;
                sVar5 = (&DAT_0839bcb2)[iVar14 * 0xde];
                (&DAT_0839bcc4)[iVar14 * 0x6f] = (float)(int)(uVar13 - 0x200) + *(float *)(uVar8 + 0x14);
                (&DAT_0839bcc8)[iVar14 * 0x6f] = *(unsigned int *)(uVar8 + 0x18);
                iVar12 = DAT_0055a7ac;

                // Firefly: init joint chain
                if (sVar5 == 0x10a) {
                    (&DAT_0839bcc8)[iVar14 * 0x6f] = *(unsigned int *)(uVar8 + 0x18);
                    if ((iVar12 < 0xb) || (0x10 < iVar12)) {
                        uVar8 = (uint)(uintptr_t)Joint_Create(0x10a, pfVar1, pfVar1,
                                             (float *)(&DAT_0839bccc + iVar14 * 0x6f), 1,
                                             (int)pcVar3, 25.0f, -1, 0);
                    } else {
                        uVar8 = (uint)(uintptr_t)Joint_Create(0x10a, pfVar1, pfVar1,
                                             (float *)(&DAT_0839bccc + iVar14 * 0x6f), 3,
                                             (int)pcVar3, 25.0f, -1, 0);
                    }
                } else {
                    fVar22 = FUN_004f7500((float)(&DAT_0839bcc0)[iVar14*0x6f], (float)(&DAT_0839bcc4)[iVar14*0x6f]);
                    (&DAT_0839bcc8)[iVar14 * 0x6f] = (float)(iVar12 % 200 + 0x96) + (float)fVar22;
                }

                // Initialize velocity
                (&DAT_0839bccc)[iVar14 * 0x6f] = 0;
                (&DAT_0839bcd0)[iVar14 * 0x6f] = 0;
                (&DAT_0839bcd4)[iVar14 * 0x6f] = 0;
                if (sVar5 == 0x10a) {
                    iVar12 = _rand();
                    (&DAT_0839bcd4)[iVar14 * 0x6f] = (float)(iVar12 % 0x168);
                }
            } else if (iVar12 == 7) {
                // Snow: check terrain
                unsigned char cVar4b = (unsigned char)DAT_0838bc70[uStack_48];
                if ((cVar4b == '\0') || (cVar4b == '\x02')) {
                    // Same as state 0..4 spawn
                }
            } else if ((10 < iVar12) && (iVar12 < 0x11)) {
                // Map loading states: use same generic spawn
            }
        }

LAB_0050172b:
        if (*pcVar3 == '\0') goto LAB_00501734;

LAB_00501734:
        if (*pcVar3 != '\0') {
            sVar5  = (&DAT_0839bcb2)[iVar14 * 0xde];
            fVar25 = 1.0f;
            pvVar9 = (void *)(DAT_05828d58 + sVar5 * 0xbc);
            if (sVar5 == 0x12d) fVar25 = 0.5f;

            if ((DAT_083a3ff0 == 0) || (sVar5 != 0x12d)) {
                pfVar1 = (float *)(&DAT_0839bcc0 + iVar14 * 0x6f);
                *(unsigned char *)((int)pvVar9 + 0xa0) = (&DAT_0839bdb5)[iVar2];
                FUN_00440aa0(pvVar9,
                             (float *)(&DAT_0839bdb8 + iVar2),
                             (float *)(&DAT_0839bdbc + iVar2),
                             &DAT_0839bdb6 + iVar2, fVar25);
                uVar8 = (uint)(uintptr_t)DAT_07abf5d8;
                sVar5 = (&DAT_0839bcb2)[iVar14 * 0xde];

                // ── Update by type ────────────────────────────────────────────
                if ((sVar5 == 0xae) || (sVar5 == 0xb8)) {
                    // Cloud life-cycle: rising(0) → hovering(1) → descending(2) → fading(3)
                    if ((&DAT_0839bdb4)[iVar2] == '\0') {
                        // Phase 0: move upward, check distance
                        lVar24 = (long long)(unsigned int)__ftol();
                        uVar8  = (uint)lVar24 & 0x80001fff;
                        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xffffe000) + 1;
                        if (((int)uVar8 < 0x800) &&
                            (fVar25 = *pfVar1 - *(float *)(DAT_07abf5d8 + 0x10),
                             fVar7  = (float)(&DAT_0839bcc4)[iVar14 * 0x6f] - *(float *)(DAT_07abf5d8 + 0x14),
                             fVar25 = SQRT(fVar25*fVar25 + fVar7*fVar7),
                             _DAT_0055285c <= fVar25) && (fVar25 <= _DAT_00552850)) {
                            (&DAT_0839bdb4)[iVar2] = 1;
                        }
                        (&DAT_0839bd7c)[iVar14 * 0x6f] = 0x3f800000;
                        uVar8 = _rand(); uVar8 &= 0x8000000f;
                        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
                        fVar25 = (float)(int)(uVar8 - 8) + (float)(&DAT_0839bcc8)[iVar14 * 0x6f];
                        (&DAT_0839bcc8)[iVar14 * 0x6f] = (unsigned int)(*(int *)&fVar25);
                        if (_DAT_0055285c <= fVar25) {
                            if (_DAT_00552ab4 < fVar25)
                                *(unsigned int *)(&DAT_0839bd78 + iVar2) = 0xc1200000;
                        } else {
                            *(unsigned int *)(&DAT_0839bd78 + iVar2) = 0x41200000;
                        }
                    }
                    if ((&DAT_0839bdb4)[iVar2] == '\x01') {
                        // Phase 1: hover at altitude, check terrain height
                        *(unsigned int *)(&DAT_0839bd78 + iVar2) = 0xc1a00000;
                        fVar22 = FUN_004f7500((float)(&DAT_0839bcc0)[iVar14*0x6f], (float)(&DAT_0839bcc4)[iVar14*0x6f]);
                        if ((float10)(float)(&DAT_0839bcc8)[iVar14 * 0x6f] < fVar22) {
                            (&DAT_0839bdb4)[iVar2] = 3;
                            (&DAT_0839bd7c)[iVar14 * 0x6f] = 0x3f8ccccd;
                            *(unsigned int *)(&DAT_0839bd78 + iVar2) = 0x41a00000;
                            (&DAT_0839bdb5)[iVar2] = 0;
                        }
                    }
                    if ((&DAT_0839bdb4)[iVar2] == '\x02') {
                        // Phase 2: descend when player animation is idle
                        if (*(unsigned char *)(DAT_07abf5d8 + 0x105) < 0xd) {
                            uVar8 = _rand(); uVar8 &= 0x800000ff;
                            bVar20 = (uVar8 == 0);
                            if ((int)uVar8 < 0) bVar20 = ((uVar8-1|0xffffff00)==0xffffffff);
                            if (!bVar20) goto LAB_00501cb5;
                        }
                        (&DAT_0839bdb4)[iVar2] = 3;
                        (&DAT_0839bd7c)[iVar14 * 0x6f] = 0x3f8ccccd;
                        *(unsigned int *)(&DAT_0839bd78 + iVar2) = 0x41a00000;
                        (&DAT_0839bdb5)[iVar2] = 0;
                    }
LAB_00501cb5:
                    if ((&DAT_0839bdb4)[iVar2] == '\x03') {
                        // Phase 3: fade out / scale down
                        uVar8 = _rand(); uVar8 &= 0x8000000f;
                        if ((int)uVar8 < 0) uVar8 = (uVar8 - 1 | 0xfffffff0) + 1;
                        (&DAT_0839bcc8)[iVar14 * 0x6f] = (unsigned int)
                            ((float)(int)(uVar8-8) + (float)(&DAT_0839bcc8)[iVar14*0x6f]);
                        fVar25 = (float)(&DAT_0839bd7c)[iVar14 * 0x6f] - _DAT_00552940;
                        (&DAT_0839bd7c)[iVar14 * 0x6f] = (unsigned int)(*(int *)&fVar25);
                        if (fVar25 <= _DAT_0055256c)
                            (&DAT_0839bdb4)[iVar2] = 0;
                    }
                } else if (sVar5 == 0xb0) {
                    // Rain: follow terrain height via sine
                    fVar22 = FUN_004f7500((float)(&DAT_0839bcc0)[iVar14*0x6f], (float)(&DAT_0839bcc4)[iVar14*0x6f]);
                    (&DAT_0839bcc8)[iVar14 * 0x6f] = (unsigned int)(float)fVar22;
                    fVar22 = (float10)fsin((float10)(float)(&DAT_0839bd30)[iVar14*0x6f]);
                    fVar22 = FUN_005129f0((float)fVar22);
                    (&DAT_0839bcc8)[iVar14 * 0x6f] = (unsigned int)(float)
                        ((float10)(float)(&DAT_0839bcc8)[iVar14*0x6f] - fVar22*(float10)_DAT_0055297c
                         + (float10)_DAT_00552ca4);
                    (&DAT_0839bd30)[iVar14 * 0x6f] = (unsigned int)
                        ((float)(&DAT_0839bd30)[iVar14*0x6f] + _DAT_005526e4);
                } else if (sVar5 == 0xaf) {
                    // Snow: random drift
                    uVar8 = _rand(); uVar8 &= 0x8000001f;
                    bVar20 = (uVar8 == 0);
                    if ((int)uVar8 < 0) bVar20 = ((uVar8-1|0xffffffe0)==0xffffffff);
                    if (bVar20) {
                        iVar12 = _rand();
                        (&DAT_0839bcd4)[iVar14*0x6f] = (float)(iVar12 % 0x168);
                        iVar12 = _rand();
                        *(float *)(&DAT_0839bd78 + iVar2) = (float)(iVar12 % 0xf - 7);
                    }
                    iVar12 = _rand();
                    *(float *)(&DAT_0839bd78 + iVar2) =
                        (float)(iVar12 % 0xf - 7) * _DAT_005526e4 + *(float *)(&DAT_0839bd78 + iVar2);
                    fVar22 = FUN_004f7500((float)(&DAT_0839bcc0)[iVar14*0x6f], (float)(&DAT_0839bcc4)[iVar14*0x6f]);
                    if ((float10)(float)(&DAT_0839bcc8)[iVar14*0x6f] < fVar22 + (float10)_DAT_00552598) {
                        *(float *)(&DAT_0839bd78 + iVar2) =
                            *(float *)(&DAT_0839bd78+iVar2) * _DAT_00552530 + _DAT_0055256c;
                    }
                    if (fVar22 + (float10)_DAT_00552900 < (float10)(float)(&DAT_0839bcc8)[iVar14*0x6f]) {
                        *(float *)(&DAT_0839bd78 + iVar2) =
                            *(float *)(&DAT_0839bd78+iVar2) * _DAT_00552530 - _DAT_0055256c;
                    }
                    iVar12 = _rand();
                    (&DAT_0839bcc8)[iVar14*0x6f] = (unsigned int)
                        ((float)(iVar12%0xf - 7) * _DAT_005528b8 + (float)(&DAT_0839bcc8)[iVar14*0x6f]);
                } else if (sVar5 == 0x10a) {
                    // Firefly/butterfly: circular orbit
                    fVar22 = (float10)fsin((float10)(float)(&DAT_0839bcd4)[iVar14*0x6f]);
                    *pfVar1 = (float)(fVar22 * (float10)(float)(&DAT_0839bd7c)[iVar14*0x6f] + (float10)*pfVar1);
                    fVar22 = (float10)fcos((float10)(float)(&DAT_0839bcd4)[iVar14*0x6f]);
                    (&DAT_0839bcc4)[iVar14*0x6f] = (unsigned int)(float)
                        ((float10)(float)(&DAT_0839bcc4)[iVar14*0x6f] - fVar22*(float10)(float)(&DAT_0839bd7c)[iVar14*0x6f]);
                    fVar22 = (float10)fsin((float10)(iStack_3c + iVar14*0x1429 + 0x4544) * (float10)_DAT_00552d28);
                    fVar23 = (float10)fcos((float10)(iStack_3c + iVar14*0xa139 + 0x870b) * (float10)_DAT_00552d28);
                    (&DAT_0839bcd4)[iVar14*0x6f] = (unsigned int)(float)
                        (fVar23 * fVar22 * (float10)_DAT_005524f8 + (float10)(float)(&DAT_0839bcd4)[iVar14*0x6f]);
                    fVar25 = *pfVar1 - *(float *)(uVar8 + 0x10);
                    fVar7  = (float)(&DAT_0839bcc4)[iVar14*0x6f] - *(float *)(uVar8 + 0x14);
                    if (_DAT_00552d24 <= SQRT(fVar25*fVar25 + fVar7*fVar7)) *pcVar3 = '\0';
                    if (_rand() % 0x1400 == 0) *pcVar3 = '\0';
                    if (((10 < DAT_0055a7ac) && (DAT_0055a7ac < 0x11)) &&
                        ((int)(&DAT_0839bd10)[iVar14*0x6f] < 1)) *pcVar3 = '\0';
                }

                // ── Movement: apply direction vector ─────────────────────────
                if ((&DAT_0839bdb4)[iVar2] != '\x02') {
                    if ((&DAT_0839bcb2)[iVar14*0xde] != 0xaf) {
                        // BUG-FIX 2026-05-04: era literal `0x839bcb0` (dirección absoluta del binario
                        // fuente). En nuestro build DAT_0839bcb0 vive en otra dirección; pasar el
                        // literal hacía que FUN_0043e680 leyera memoria random → AV at 0x004BF712 al
                        // entrar al mundo (param0=0 read, param1=0x0839BCB0).
                        FUN_0043e680((int)pcVar3, iVar14, (int)(uintptr_t)&DAT_0839bcb0, 0x28);
                    }
                    Matrix_BuildFromEuler((float *)(&DAT_0839bccc + iVar14*0x6f), (float *)(&DAT_0839bd40 + iVar2));

                    // Scale velocity by sub-state
                    if (DAT_0055a7ac == 7) {
                        if (_DAT_00552660 <= (float)(&DAT_0839bd30)[iVar14*0x6f]) {
                            uVar8 = _rand(); uVar8 &= 0x8000001f;
                            if ((int)uVar8 < 0) uVar8 = (uVar8-1|0xffffffe0)+1;
                            fStack_30 = *(float *)(&DAT_0839bd78 + iVar2);
                            *(unsigned int *)(&DAT_0839bd88 + iVar2) = 0x40a00000;
                            fStack_38 = (float)(int)(uVar8+0x20) * (float)(&DAT_0839bd7c)[iVar14*0x6f];
                        } else if (iVar14 < 0x23) {
                            uVar8 = _rand(); uVar8 &= 0x8000000f;
                            if ((int)uVar8 < 0) uVar8 = (uVar8-1|0xfffffff0)+1;
                            fStack_30 = *(float *)(&DAT_0839bd78 + iVar2);
                            *(unsigned int *)(&DAT_0839bd88 + iVar2) = 0x41700000;
                            fStack_38 = (float)(int)(uVar8+8) * (float)(&DAT_0839bd7c)[iVar14*0x6f];
                        } else {
                            uVar8 = _rand(); uVar8 &= 0x8000000f;
                            if ((int)uVar8 < 0) uVar8 = (uVar8-1|0xfffffff0)+1;
                            fStack_30 = *(float *)(&DAT_0839bd78 + iVar2);
                            *(unsigned int *)(&DAT_0839bd88 + iVar2) = 0x41700000;
                            fStack_38 = (float)(int)(uVar8+0x10) * (float)(&DAT_0839bd7c)[iVar14*0x6f];
                        }
                        fVar25 = (float)(&DAT_0839bd30)[iVar14*0x6f] + _DAT_005524f4;
                        (&DAT_0839bd30)[iVar14*0x6f] = (unsigned int)(*(int *)&fVar25);
                        if (_DAT_00552488 <= fVar25)
                            (&DAT_0839bd30)[iVar14*0x6f] = 0;
                    } else {
                        fStack_38 = (float)(&DAT_0839bd7c)[iVar14*0x6f] * _DAT_00552464;
                        fStack_30 = *(float *)(&DAT_0839bd78 + iVar2);
                    }
                    fStack_34 = 0.0f;
                    Vector_Rotate(&fStack_38, (float *)(&DAT_0839bd40 + iVar2), &fStack_2c);
                    fVar7  = fStack_2c + *pfVar1;
                    sVar5  = (&DAT_0839bcb2)[iVar14*0xde];
                    puStack_44 = (DWORD *)0x44bb8000;
                    *pfVar1 = fVar7;
                    fVar25 = fStack_28 + (float)(&DAT_0839bcc4)[iVar14*0x6f];
                    (&DAT_0839bcc4)[iVar14*0x6f] = (unsigned int)(*(int *)&fVar25);
                    uVar8 = (uint)(uintptr_t)DAT_07abf5d8;
                    (&DAT_0839bcc8)[iVar14*0x6f] = (unsigned int)(fStack_24 + (float)(&DAT_0839bcc8)[iVar14*0x6f]);
                    *(float *)(&DAT_0839bd70 + iVar2) = fStack_2c * _DAT_00552540 + fVar7;
                    *(float *)(&DAT_0839bd74 + iVar2) = fStack_28 * _DAT_00552540 + fVar25;
                    fVar7  = fVar7 - *(float *)(uVar8 + 0x10);
                    fVar25 = fVar25 - *(float *)(uVar8 + 0x14);
                    { float f_dist = SQRT(fVar7*fVar7 + fVar25*fVar25);
                      float f_maxdist;
                      if (sVar5 == 0x12d) {
                          DWORD dw_maxdist = 0x457a0000;
                          f_maxdist = *(float*)&dw_maxdist;  // float 1000.0f
                      } else {
                          DWORD dw_maxdist = 0x44bb8000;
                          f_maxdist = *(float*)&dw_maxdist;  // float 1500.0f
                          uVar8 = _rand(); uVar8 &= 0x800001ff;
                          bVar20 = (uVar8 == 0);
                          if ((int)uVar8 < 0) bVar20 = ((uVar8-1|0xfffffe00)==0xffffffff);
                          if (bVar20) *pcVar3 = '\0';
                      }
                      if (f_maxdist <= f_dist) *pcVar3 = '\0';
                    }
                }

                // Loop-kill conditions (state 7, terrain type = grass)
                if ((((int)(&DAT_0839bd10)[iVar14*0x6f] < 1) && (DAT_0055a7ac == 7)) &&
                    ((unsigned char)DAT_0838bc70[uStack_48] == 0x01)) {
                    fVar25 = (float)(&DAT_0839bcd4)[iVar14*0x6f] + _DAT_005524ec;
                    bVar20 = (_DAT_0055286c <= fVar25);
                    (&DAT_0839bcd4)[iVar14*0x6f] = (unsigned int)(*(int *)&fVar25);
                    if (bVar20) (&DAT_0839bcd4)[iVar14*0x6f] = (unsigned int)(fVar25 - _DAT_0055286c);
                    (&DAT_0839bd10)[iVar14*0x6f] = 10;
                    (&DAT_0839bcb4)[iVar14*0x6f] = (int)(&DAT_0839bcb4)[iVar14*0x6f] + 1;
                }
                if (1 < (int)(&DAT_0839bcb4)[iVar14*0x6f]) *pcVar3 = '\0';
                (&DAT_0839bd10)[iVar14*0x6f] = (int)(&DAT_0839bd10)[iVar14*0x6f] - 1;

                // Proximity effects (cloud type: chance to play sound/effect)
                fVar25 = *pfVar1 - *(float *)(DAT_07abf5d8 + 0x10);
                fVar7  = (float)(&DAT_0839bcc4)[iVar14*0x6f] - *(float *)(DAT_07abf5d8 + 0x14);
                if (SQRT(fVar25*fVar25 + fVar7*fVar7) < _DAT_00552ab4) {
                    sVar5 = (&DAT_0839bcb2)[iVar14*0xde];
                    pcVar19 = pcVar3;
                    if (sVar5 == 0xae) {
                        uVar8 = _rand(); uVar8 &= 0x800001ff;
                        bVar20 = (uVar8 == 0);
                        if ((int)uVar8 < 0) bVar20 = ((uVar8-1|0xfffffe00)==0xffffffff);
                        if (bVar20) FUN_00404bc0(0xc, (int)pcVar3, 0);
                        uVar8 = _rand(); uVar8 &= 0x800001ff;
                        bVar20 = (uVar8 == 0);
                        if ((int)uVar8 < 0) bVar20 = ((uVar8-1|0xfffffe00)==0xffffffff);
                        if (bVar20) FUN_00404bc0(0xd, (int)pcVar19, 0);
                    } else if (sVar5 == 0xb0) {
                        uVar8 = _rand(); uVar8 &= 0x800000ff;
                        bVar20 = (uVar8 == 0);
                        if ((int)uVar8 < 0) bVar20 = ((uVar8-1|0xffffff00)==0xffffffff);
                        if (bVar20) FUN_00404bc0(0xe, (int)pcVar19, 0);
                    } else if ((sVar5 == 0xb8) && ((unsigned char)DAT_0838bc70[uStack_48] == 0x01)) {
                        uVar8 = _rand(); uVar8 &= 0x8000007f;
                        bVar20 = (uVar8 == 0);
                        if ((int)uVar8 < 0) bVar20 = ((uVar8-1|0xffffff80)==0xffffffff);
                        if (bVar20) FUN_00404bc0(0x6d, (int)pcVar19, 0);
                    }
                }
            } else {
                // DAT_083a3ff0 != 0 && type == 0x12d: thunder cloud — full animation
                FUN_0043e820((int)pcVar3, 7);
                *(unsigned char *)((int)pvVar9 + 0xa0) = (&DAT_0839bdb5)[iVar2];
                FUN_00440aa0(pvVar9,
                             (float *)(&DAT_0839bdb8 + iVar2),
                             (float *)(&DAT_0839bdbc + iVar2),
                             &DAT_0839bdb6 + iVar2, fVar25);
                Matrix_BuildFromEuler((float *)(&DAT_0839bccc + iVar14*0x6f), (float *)(&DAT_0839bd40 + iVar2));
                fStack_20 = (float)(&DAT_0839bcbc)[iVar14*0x6f] * _DAT_005528e4;
                fStack_1c = 0.0f; fStack_18 = 0.0f;
                Vector_Rotate(&fStack_20, (float *)(&DAT_0839bd40 + iVar2), &fStack_14);
                (&DAT_0839bcc0)[iVar14*0x6f] = (unsigned int)(fStack_14 + (float)(&DAT_0839bcc0)[iVar14*0x6f]);
                (&DAT_0839bcc4)[iVar14*0x6f] = (unsigned int)(fStack_10 + (float)(&DAT_0839bcc4)[iVar14*0x6f]);
                (&DAT_0839bcc8)[iVar14*0x6f] = (unsigned int)(fStack_c  + (float)(&DAT_0839bcc8)[iVar14*0x6f]);
                fVar22 = FUN_004f7500((float)(&DAT_0839bcc0)[iVar14*0x6f], (float)(&DAT_0839bcc4)[iVar14*0x6f]);
                (&DAT_0839bcc8)[iVar14*0x6f] = (unsigned int)(float)(fVar22 + (float10)_DAT_00552900);
                fVar22 = (float10)fsin((float10)(float)(&DAT_0839bd30)[iVar14*0x6f]);
                fVar23 = FUN_005129f0((float)fVar22);
                fVar22 = (float10)_DAT_005524f0;
                iVar12 = (&DAT_0839bd10)[iVar14*0x6f];
                (&DAT_0839bd10)[iVar14*0x6f] = iVar12 - 1;
                (&DAT_0839bcc8)[iVar14*0x6f] = (unsigned int)(float)
                    ((float10)(float)(&DAT_0839bcc8)[iVar14*0x6f] - fVar23*fVar22 + fVar22);
                (&DAT_0839bd30)[iVar14*0x6f] = (unsigned int)
                    ((float)(&DAT_0839bcbc)[iVar14*0x6f] * _DAT_00552874 + (float)(&DAT_0839bd30)[iVar14*0x6f]);
                if (iVar12 - 1 < 1) *pcVar3 = '\0';
                uVar8 = _rand(); uVar8 &= 0x8000007f;
                bVar20 = (uVar8 == 0);
                if ((int)uVar8 < 0) bVar20 = ((uVar8-1|0xffffff80)==0xffffffff);
                if (bVar20) FUN_00404bc0(0x126, 0, 0);
            }
        }

        FUN_0043e5c0((int)pcVar3);
        uVar8 = 0;  // return value unused; FUN_0043e5c0 returns void
        iVar12 = DAT_0055a7ac;

        iVar14++;
        if (0x27 < iVar14) return uVar8;
    } while (true);
}
