// Entity_Render_3D.cpp
// Entity_Render_3D @ 0x004FC070  (667 lines, decompile completo)
//
// Función de renderizado 3D por entidad. Selecciona el tipo de partícula/efecto/spawn
// según g_GameState + entity.type, y llama a los emitters correspondientes.
// Es en realidad un "efecto-spawn" por entidad, no dibujo directo de geometría.
//
// ── SIGNATURA ─────────────────────────────────────────────────────────────────
//
//   float * __cdecl FUN_004fc070(int param_1)
//     param_1 = puntero a entidad
//
// ── SETUP INICIAL ─────────────────────────────────────────────────────────────
//
//   this = DAT_05828d58 + entity[+2] * 0xbc    → model data para la clase de entidad
//   fVar14 = (rand() % 30 + 70) * g_TileScale  → tamaño base aleatorio [70-99] × tile_scale
//
// ── SWITCH PRINCIPAL: g_GameSubState (DAT_0055a7ac) ──────────────────────────
//
//   case 0 (login/connecting):
//     switch entity.type (*(short*)(param_1+2)):
//       0x38 (type 56 — portal/gate):
//         local_24 = fVar14 * 0.804f, local_20 = fVar14 * 0.25f
//         local_1c = fVar14 * 0.36f,  local_30 = fVar14 * 0.5f
//         BMD_TransformPosition(model, &DAT_0697139c, offset, &local_18, '\0')  → Bone_GetTransform
//         FUN_004795c0(0x47e, &local_18, 0.5f, &local_24, entity, 0, 0) → Particle_Spawn(0x47e=fire)
//         BMD_TransformPosition(model, &DAT_0697154c, offset, &local_18, '\0')
//         FUN_004795c0(0x47e, ...)
//
//       0x69 (type 105 — NPC with random behavior):
//         entity[+0x68] = 1.0f (scale)
//         entity[+0x70] = -(ftol() % 1000) * scale  (Y offset cycling)
//         if rand()&1 == 0:
//           Two calls BMD_TransformPosition + Particle_Spawn(0x4c4, ...) — spawn smoke/dust effect
//           (offsets: local_c.y = -20.0 for low, -80.0 for high puff)
//
//       0x67 (type 103 — weapon spark check):
//         Lookup animation frame from model table at model[+0x30]+param_4*0x10+8
//         If current anim frame == expected: if rand()&0x1f==0:
//           FUN_0043e820(entity, 1) or FUN_0043e820(entity, 0)
//           → Entity_SetWeaponEffect(entity, trail_enable)
//
//   case 2 (in-world):
//     switch entity.type:
//       0x64 (type 100 — player character in world):
//         local_24 = fVar14, local_20 = fVar14
//         local_c = {0, 0, 150.0}
//         local_30 = ftol() % 360 (random rotation angle)
//         BMD_TransformPosition(model, &DAT_06970a9c, offset, &local_18, '\0')
//         FUN_004795c0(0x4a7, &local_18, 3.04f, &local_24, entity, angle, 0)  → fire spark
//         FUN_004795c0(0x4a7, &local_18, 3.04f, &local_24, entity, -angle, 0) → mirror spark
//
//   case 3 (char select):
//     switch entity.type:
//       1: 3 bone transforms + FUN_004795c0(0x47e, ..., 0.5f) ×3 — 3 fire jets (gate)
//       9: BMD_TransformPosition + fVar14=1.5 → single fire + Particle_Spawn
//       0x11: 4 bone transforms + FUN_004795c0(0x47e, ..., 1.0f) ×4 + fVar14=1.0
//       0x23: BMD_TransformPosition + FUN_004795c0(0x47e, ..., 1.5f) → 1 fire
//       0x27 (type 39 — special portal with sparks):
//         2× FUN_004795c0(0x4a7) rotating sparks
//         Loop 4 bone nodes: BMD_TransformPosition + FUN_004795c0(0x47e, ..., 1.0f)
//         if rand()&7==0: Particle_Spawn(0x4ce, ...) ×2  → spawn extra effects
//         if rand()&7==0: 8 iterations of Joint_Create(0x4e9, ...) → particle burst
//       ... (667 líneas totales, continúa con más tipos)
//
// ── FUNCIONES IDENTIFICADAS ───────────────────────────────────────────────────
//
//   BMD_TransformPosition  → Bone_GetWorldTransform(model, bone_ptr, offset_xyz, out_mat, flag)
//                   Obtiene la transformación en espacio mundo de un hueso del modelo.
//                   bone_ptr = puntero a datos de hueso (en sección de datos 0x069xxxxx)
//
//   FUN_004795c0  → Particle_Spawn(type, bone_mat, scale, rgb_ptr, entity, angle, flag)
//                   Crea una partícula en la posición del hueso.
//                   type: 0x47e=fuego/llama, 0x4a7=chispa lineal, 0x4ce=humo, 0x4e9=burst
//
//   Particle_Spawn  → Effect_Spawn(type, bone_mat, pos_ptr, size_ptr, flag, alpha, mode)
//                   Crea un efecto visual (no partícula) — humo, resplandor, etc.
//
//   Joint_Create  → Burst_Spawn(type, mat, mat2, angle_ptr, flag1, flag2, speed, dir, mode)
//                   Explosión/burst de partículas en N direcciones.
//
//   FUN_0043e820  → Entity_SetWeaponEffect(entity, enable)
//                   Activa/desactiva el efecto de trail de arma.
//
//   _rand         → MSVC rand() — usado para variación aleatoria de efectos
//   __ftol        → float-to-long (conversión para ángulos discretos)
//
// ── DATOS DE HUESOS (sección 0x069xxxxx) ─────────────────────────────────────
//
//   DAT_06970a9c  — hueso raíz (posición base entidad)
//   DAT_06970acc  — hueso secundario A
//   DAT_06970b2c  — hueso secundario B
//   DAT_06970b5c  — hueso terciario
//   DAT_06970bbc  — hueso cuaternario
//   DAT_06970bec  — hueso D
//   DAT_06970c1c  — hueso fuego A
//   DAT_06970c4c  — hueso fuego B
//   DAT_06970c7c  — hueso fuego C
//   DAT_06970d0c  — hueso fuego D
//   DAT_0697139c  — hueso portal A
//   DAT_0697154c  — hueso portal B
//   DAT_0697157c  — hueso portal C
//   DAT_0697160c..0x69716cd — array de 4 huesos portal (stride 0x30, loop)
//
// ── NOTA ARQUITECTURAL ────────────────────────────────────────────────────────
//
//   Esta función NO dibuja geometría directamente: spawna partículas/efectos
//   asociados a la entidad (fuego de antorchas, chispas de portales, humo, etc.).
//   El nombre Entity_Render_3D es engañoso; en realidad es Entity_SpawnEffects.
//   Los 667 líneas son principalmente switch/case por (g_GameSubState, entity.type).

#include "stdafx.h"
#include <stdio.h>
extern "C" { void DbgLogPublic(const char* msg); }
#include "Render/Entity_Render_3D.h"

// Entity_SpawnEffects @ 0x004FC070
// Spawna partículas/efectos visuales en la posición de los huesos de la entidad.
// Switch principal: g_GameSubState × entity.type.
//
// 2026-05-03: AUTO-SKIP removed. The only absolute-bound loop was the case
// 0x27 portal bone walker (`while (pfVar3 < 0x69716cd)`), already replaced
// with explicit count of 4. Other paths use proper bounds or symbol math.
float * __cdecl FUN_004fc070(int param_1)
{
    void     *pModel;
    float     fVar14;
    int       iVar2, iVar7;
    float    *pfVar3;
    uint      uVar4;
    unsigned short uVar5;
    float    *pfVar6, *pfVar12, *pfVar13, *pfVar15, *pfVar16, *pfVar17;
    bool      bVar8;
    float10   fVar9;
    longlong  lVar10;
    ulonglong uVar11;
    byte      bVar18;
    float     local_30, local_2c_f, local_28;
    // local_24/20/1c son slots CONSECUTIVOS del frame original y varios sitios
    // pasan `&local_24` como el vec3 de Light a CreateSprite / Particle_Spawn.
    // Como escalares sueltos MSVC no garantiza contiguidad y los canales G y B
    // salian de basura (mismo patron que las nubes de Icarus, y que ya mordio en
    // Weather_Update, MoveJoint y CreateJoint).
    float     __lt[3] = { 0.0f, 0.0f, 0.0f };
    float    &local_24 = __lt[0], &local_20 = __lt[1], &local_1c = __lt[2];
    // BUG-FIX: many paths pass &local_18 as if it were a contiguous float[3]
    // to TransformPosition / CreateSprite / Particle_Spawn. Keep the original
    // names as aliases over a real 3-float buffer so world positions are stable.
    float     local_pos_buf[3];
    #define local_18 local_pos_buf[0]
    #define local_14 local_pos_buf[1]
    #define local_10 local_pos_buf[2]
    float     local_c[3];
    float     local_light_rgb[3];

    pModel = (void *)(DAT_05828d58 + *(short *)(param_1 + 2) * 0xbc);
    iVar2  = _rand();
    fVar14 = (float)(iVar2 % 0x1e + 0x46) * _DAT_005524f8;

    // Login scene special-case: g_GameState==2, entity type 0x3c.
    if ((DAT_005615c0 == 2) && (*(short *)(param_1 + 2) == 0x3c)) {
        local_light_rgb[0] = fVar14;
        local_light_rgb[1] = fVar14 * 0.5f;
        local_light_rgb[2] = fVar14 * 0.2f;
        local_c[0] = 0.0f; local_c[1] = 0.0f; local_c[2] = 0.0f;
        // BUG-FIX 2026-07-12: translate=0 (fiel al IDA Entity_SpawnEffects). El
        // barco tiene 2 antorchas (bones c1c/c4c). El translate=1 de la sesión
        // previa se puso a ciegas (flares invisibles por bug texcoords) y sumaba
        // la pos world de más → 1 flare desplazado al lado del barco. La bone
        // matrix ya está en world-space, translate=0 da la pos correcta.
        BMD_TransformPosition(pModel, (float *)&DAT_06970c1c, local_c, &local_18, '\0');
        FUN_004795c0(0x47e, &local_18, 1.0f, local_light_rgb, param_1, 0, 0);
        BMD_TransformPosition(pModel, (float *)&DAT_06970c4c, local_c, &local_18, '\0');
        FUN_004795c0(0x47e, &local_18, 1.0f, local_light_rgb, param_1, 0, 0);
    }

    local_c[0] = 0.0f; local_c[1] = 0.0f; local_c[2] = 0.0f;
    pfVar3 = (float *)DAT_0055a7ac;

    switch (DAT_0055a7ac) {
    // ── case 0: login / connecting ────────────────────────────────────────────
    case 0:
        pfVar3 = (float *)(int)*(short *)(param_1 + 2);
        if (pfVar3 == (float *)0x38) {
            // Type 56 — portal gate: two fire jets from bone data
            local_30 = fVar14 * _DAT_00552660;
            local_24 = fVar14 * _DAT_00552534;
            local_20 = fVar14 * _DAT_005528b8;
            local_1c = fVar14 * _DAT_005524f4;
            BMD_TransformPosition(pModel, (float *)&DAT_0697139c, &local_18, local_c, '\0');
            float fSave = local_30;
            FUN_004795c0(0x47e, local_c, local_30, &local_24, param_1, 0, 0);
            BMD_TransformPosition(pModel, (float *)&DAT_0697154c, &local_18, local_c, '\0');
            pfVar3 = (float *)FUN_004795c0(0x47e, local_c, fSave, &local_24, param_1, 0, 0);
            return pfVar3;
        }
        if (pfVar3 == (float *)0x69) {
            // Type 105 — NPC: Y-offset scroll + random smoke puffs (50% chance)
            *(undefined4 *)(param_1 + 0x68) = 0x3f800000;
            lVar10 = (longlong)DAT_05826e08;   // IDA: (-(__int64)WorldTime % 1000) * 0.001
            *(float *)(param_1 + 0x70) = (float)(-(int)lVar10 % 1000) * _DAT_00552500;
            uVar4 = _rand();
            pfVar3 = (float *)(uVar4 & 0x80000001);
            if ((int)pfVar3 < 0)
                pfVar3 = (float *)(((int)pfVar3 - 1U | 0xfffffffe) + 1);
            if (pfVar3 == (float *)0x0) {
                // Low puff: y=-20
                uVar4 = _rand() & 0x8000001f;
                if ((int)uVar4 < 0) uVar4 = (uVar4 - 1 | 0xffffffe0) + 1;
                local_c[1] = -20.0f;
                local_c[0] = (float)(int)(uVar4 - 0x10);
                uVar4 = _rand() & 0x8000001f;
                if ((int)uVar4 < 0) uVar4 = (uVar4 - 1 | 0xffffffe0) + 1;
                local_c[2] = (float)(int)(uVar4 - 0x10);
                BMD_TransformPosition(pModel, (float *)&DAT_06970acc, local_c, &local_18, '\0');
                Particle_Spawn(0x4c4, &local_18, (float *)(param_1 + 0x1c), (float *)(param_1 + 0xe8), 0, 1.0f, 0);
                // High puff: y=-80
                uVar4 = _rand() & 0x8000001f;
                if ((int)uVar4 < 0) uVar4 = (uVar4 - 1 | 0xffffffe0) + 1;
                local_c[1] = -80.0f;
                local_c[0] = (float)(int)(uVar4 - 0x10);
                uVar4 = _rand() & 0x8000001f;
                if ((int)uVar4 < 0) uVar4 = (uVar4 - 1 | 0xffffffe0) + 1;
                local_c[2] = (float)(int)(uVar4 - 0x10);
                BMD_TransformPosition(pModel, (float *)&DAT_06970b5c, local_c, &local_18, '\0');
                pfVar3 = (float *)Particle_Spawn(0x4c4, &local_18, (float *)(param_1 + 0x1c), (float *)(param_1 + 0xe8), 0, 1.0f, 0);
                return pfVar3;
            }
        }
        break;

    // ── case 2: in-world ──────────────────────────────────────────────────────
    case 2:
        pfVar3 = (float *)(int)*(short *)(param_1 + 2);
        if (pfVar3 == (float *)0x64) {
            // Type 100 — player: rotating spark pair at z=150
            local_24 = fVar14; local_20 = fVar14; local_1c = fVar14;
            uVar11 = (longlong)(DAT_05826e08 * 0.1f);   // IDA: (int)(__int64)(WorldTime*0.1) % 360
            local_c[0] = 0.0f; local_c[1] = 0.0f; local_c[2] = 150.0f;
            local_30 = (float)(int)((longlong)((ulonglong)(uint)((int)uVar11 >> 0x1f) << 0x20
                                               | uVar11 & 0xffffffff) % 0x168);
            BMD_TransformPosition(pModel, (float *)&DAT_06970a9c, local_c, &local_18, '\0');
            FUN_004795c0(0x4a7, &local_18, 2.5f,&local_24, param_1,  local_30, 0);
            pfVar3 = (float *)FUN_004795c0(0x4a7, &local_18, 2.5f,&local_24, param_1, -local_30, 0);
            return pfVar3;
        }
        // Type 103 — weapon spark trigger (once per anim cycle, 1/32 chance)
        if ((pfVar3 == (float *)0x67) &&
            (pfVar3 = (float *)(*(short *)((uint)*(byte *)(param_1 + 0x105) * 0x10 + 8
                                           + *(int *)((int)pModel + 0x30)) + -1),
             (float *)(int)*(short *)((int)pModel + 0xa8) == pfVar3))
        {
            uVar4 = _rand() & 0x8000001f;
            bVar8 = (uVar4 == 0);
            if ((int)uVar4 < 0) bVar8 = ((uVar4 - 1 | 0xffffffe0) == 0xffffffff);
            if (bVar8) { pfVar3 = (float *)FUN_0043e820(param_1, 1); return pfVar3; }
            pfVar3 = (float *)FUN_0043e820(param_1, 0);
            return pfVar3;
        }
        break;

    // ── case 3: char-select scene ─────────────────────────────────────────────
    case 3:
        pfVar3 = (float *)(*(short *)(param_1 + 2) + -1);
        switch (*(short *)(param_1 + 2)) {
        case 1:
            local_24 = fVar14 * _DAT_005528b4;
            local_20 = fVar14 * _DAT_00552928;
            local_1c = fVar14;
            BMD_TransformPosition(pModel, (float *)&DAT_06970afc, local_c, &local_18, '\0');
            FUN_004795c0(0x47e, &local_18, 0.5f,&local_24, param_1, 0, 0);
            BMD_TransformPosition(pModel, (float *)&DAT_06970b5c, local_c, &local_18, '\0');
            FUN_004795c0(0x47e, &local_18, 0.5f,&local_24, param_1, 0, 0);
            BMD_TransformPosition(pModel, (float *)&DAT_06970bbc, local_c, &local_18, '\0');
            fVar14 = 0.5f;
            break;
        case 9:
            local_24 = fVar14 * _DAT_005528b4;
            local_20 = fVar14 * _DAT_00552928;
            local_1c = fVar14;
            BMD_TransformPosition(pModel, (float *)&DAT_06970acc, local_c, &local_18, '\0');
            fVar14 = 1.5f;
            break;
        case 0x11:
            local_24 = fVar14 * _DAT_005528b4;
            local_20 = fVar14 * _DAT_00552928;
            local_1c = fVar14;
            BMD_TransformPosition(pModel, (float *)&DAT_06970b5c, local_c, &local_18, '\0');
            FUN_004795c0(0x47e, &local_18, 1.0f,&local_24, param_1, 0, 0);
            BMD_TransformPosition(pModel, (float *)&DAT_06970bec, local_c, &local_18, '\0');
            FUN_004795c0(0x47e, &local_18, 1.0f,&local_24, param_1, 0, 0);
            BMD_TransformPosition(pModel, (float *)&DAT_06970c7c, local_c, &local_18, '\0');
            FUN_004795c0(0x47e, &local_18, 1.0f,&local_24, param_1, 0, 0);
            BMD_TransformPosition(pModel, (float *)&DAT_06970d0c, local_c, &local_18, '\0');
            pfVar3 = &local_18;
            fVar14 = 1.0f;
            goto LAB_004fc55a;
        case 0x23:
            local_24 = fVar14 * _DAT_005528b4;
            local_20 = fVar14 * _DAT_00552928;
            local_1c = fVar14;
            BMD_TransformPosition(pModel, (float *)&DAT_06970b2c, local_c, &local_18, '\0');
            pfVar3 = (float *)FUN_004795c0(0x47e, &local_18, 1.5f,&local_24, param_1, 0, 0);
            return pfVar3;
        case 0x27:
            // Special portal: 2 rotating sparks + 4-bone fire loop + optional burst
            local_24 = fVar14 * _DAT_005528b4;
            local_20 = fVar14 * _DAT_00552530;
            local_1c = fVar14;
            uVar11 = (longlong)(DAT_05826e08 * 0.1f);   // IDA: (int)(__int64)(WorldTime*0.1) % 360
            local_30 = (float)(int)((longlong)((ulonglong)(uint)((int)uVar11 >> 0x1f) << 0x20
                                               | uVar11 & 0xffffffff) % 0x168);
            BMD_TransformPosition(pModel, (float *)&DAT_0697154c, local_c, &local_18, '\0');
            FUN_004795c0(0x4a7, &local_18, 1.0f,&local_24, param_1,  local_30, 0);
            FUN_004795c0(0x4a7, &local_18, 1.0f,&local_24, param_1, -local_30, 0);
            local_24 = 1.0f; local_20 = 1.0f; local_1c = 1.0f;
            // BUG-FIX 2026-05-03: bound `< 0x69716cd` was an absolute source-binary
            // address (= &DAT_0697160c + 0xC1). DAT_0697160c is a macro into
            // g_BoneScratch in our build; comparing pfVar3 to literal 0x69716cd
            // is junk. The iteration count is 4 ("array de 4 huesos portal,
            // stride 0x30" per the file header).
            pfVar3 = (float *)&DAT_0697160c;
            for (int boneIdx = 0; boneIdx < 4; ++boneIdx, pfVar3 += 0xc) {
                BMD_TransformPosition(pModel, pfVar3, local_c, &local_18, '\0');
                FUN_004795c0(0x47e, &local_18, 1.0f,&local_24, param_1, 0, 0);
                uVar4 = _rand() & 0x8000001f;
                bVar8 = (uVar4 == 0);
                if ((int)uVar4 < 0) bVar8 = ((uVar4 - 1 | 0xffffffe0) == 0xffffffff);
                if (bVar8) {
                    Particle_Spawn(0x4ce, &local_18, (float *)(param_1 + 0x1c), &local_24, 0, 1.0f, 0);
                    Particle_Spawn(0x4ce, &local_18, (float *)(param_1 + 0x1c), &local_24, 1, 1.0f, 0);
                }
            }
            BMD_TransformPosition(pModel, (float *)&DAT_0697157c, local_c, &local_18, '\0');
            uVar4 = _rand() & 0x80000007;
            pfVar3 = (float *)(uVar4);
            if ((int)pfVar3 < 0) pfVar3 = (float *)(((int)pfVar3 - 1U | 0xfffffff8) + 1);
            if (pfVar3 == (float *)0x0) {
                iVar2 = 8;
                do {
                    iVar7 = _rand();
                    float fAngPhi = (float)(iVar7 % 0x3c + 0x3c);
                    iVar7 = _rand();
                    local_28 = (float)(iVar7 % 0x1e);
                    pfVar3 = (float *)Joint_Create(0x4e9, &local_18, &local_18, &fAngPhi, 0, 0, 10.0f, -1, 0);
                    iVar2--;
                } while (iVar2 != 0);
                return pfVar3;
            }
            goto switchD_004fc1a7_caseD_1;
        default:
            goto switchD_004fc1a7_caseD_1;
        }
        pfVar3 = &local_18;
        goto LAB_004fc55a;

    // ── case 4: in-world variant ──────────────────────────────────────────────
    case 4:
        pfVar3 = (float *)(int)*(short *)(param_1 + 2);
        if ((int)pfVar3 > 0x12 && (int)pfVar3 < 0x15) {
            if (*(short *)(param_1 + 2) == 0x13) {
                uVar5 = 0x4f0; local_1c = 0.0f;
                local_24 = fVar14; local_20 = _DAT_005526e4;
            } else {
                local_24 = fVar14 * _DAT_005528b4;
                uVar5 = 0x4a7; local_1c = fVar14; local_20 = _DAT_00552530;
            }
            local_20 = fVar14 * local_20;
            uVar11 = (longlong)(DAT_05826e08 * 0.1f);   // IDA: (int)(__int64)(WorldTime*0.1) % 360
            local_30 = (float)(int)((longlong)((ulonglong)(uint)((int)uVar11 >> 0x1f) << 0x20
                                               | uVar11 & 0xffffffff) % 0x168);
            BMD_TransformPosition(pModel, (float *)&DAT_06970d6c, local_c, &local_18, '\0');
            float fAngle1 = local_30;
            float fAngleN = -local_30;
            FUN_004795c0(uVar5, &local_18, 0.3f,&local_24, param_1, local_30,  0);
            FUN_004795c0(uVar5, &local_18, 0.3f,&local_24, param_1, fAngleN,   0);
            BMD_TransformPosition(pModel, (float *)&DAT_06970e2c, local_c, &local_18, '\0');
            FUN_004795c0(uVar5, &local_18, 0.3f,&local_24, param_1, fAngle1,   0);
            FUN_004795c0(uVar5, &local_18, 0.3f,&local_24, param_1, fAngleN,   0);
            BMD_TransformPosition(pModel, (float *)&DAT_06970e8c, local_c, &local_18, '\0');
            FUN_004795c0(uVar5, &local_18, 1.5f,&local_24, param_1, fAngle1,   0);
            pfVar3 = (float *)FUN_004795c0(uVar5, &local_18, 1.5f,&local_24, param_1, fAngleN, 0);
        } else if (pfVar3 == (float *)0x28) {
            local_24 = fVar14; local_20 = fVar14; local_1c = fVar14;
            uVar11 = (longlong)(DAT_05826e08 * 0.1f);   // IDA: (int)(__int64)(WorldTime*0.1) % 360
            local_c[0] = 0.0f; local_c[1] = 0.0f; local_c[2] = 260.0f;
            local_14 = *(float *)(param_1 + 0x14);
            local_30 = (float)(int)((longlong)((ulonglong)(uint)((int)uVar11 >> 0x1f) << 0x20
                                               | uVar11 & 0xffffffff) % 0x168);
            local_18 = *(float *)(param_1 + 0x10);
            local_10 = *(float *)(param_1 + 0x18) + _DAT_005529e0;
            FUN_004795c0(0x4a7, &local_18, 2.5f,&local_24, param_1,  local_30, 0);
            pfVar3 = (float *)FUN_004795c0(0x4a7, &local_18, 2.5f,&local_24, param_1, -local_30, 0);
        }
        // fallthrough to case 6
    case 6:
        if (*(short *)(param_1 + 2) == 9) {
            local_24 = fVar14 * _DAT_00552534;
            local_20 = fVar14 * _DAT_005528b8;
            local_1c = fVar14 * _DAT_005524f4;
            BMD_TransformPosition(pModel, (float *)&DAT_06970acc, &local_18, local_c, '\0');
            fVar14 = fVar14 * _DAT_00552660;
            pfVar3 = local_c;
LAB_004fc55a:
            pfVar3 = (float *)FUN_004795c0(0x47e, pfVar3, fVar14, &local_24, param_1, 0, 0);
            return pfVar3;
        }
        break;

    // ── case 8: various special types ─────────────────────────────────────────
    case 8:
        pfVar3 = (float *)(int)*(short *)(param_1 + 2);
        switch ((int)pfVar3) {
        case 0:
            fVar9 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_005528e0);
            fVar9 = fVar9 * (float10)_DAT_005528b8 + (float10)_DAT_00552928;
            local_24 = (float)(fVar9 * (float10)_DAT_00552d0c);
            local_20 = (float)fVar9; local_1c = (float)fVar9;
            BMD_TransformPosition(pModel, (float *)&DAT_06970bbc, local_c, &local_18, '\0');
            pfVar3 = (float *)FUN_004795c0(0x498, &local_18, (float)fVar9 * _DAT_00552660, &local_24, param_1, 0, 0);
            return pfVar3;
        case 0x3c:
            if (*(int *)(param_1 + 0x58) != -2) {
                iVar2 = 0x14;
                do {
                    pfVar3 = (float *)Particle_Spawn(0x4c4, (float *)(param_1 + 0x10), (float *)(param_1 + 0x1c), (float *)(param_1 + 0xe8), 6, *(float *)(param_1 + 0xc), 0);
                } while (--iVar2 != 0);
            }
            *(undefined4 *)(param_1 + 0x58) = 0xfffffffe;
            return pfVar3;
        case 0x3f:
            fVar9 = (float10)fsin(((float10)*(float *)(param_1 + 0x24) * (float10)_DAT_00552660
                                   + (float10)DAT_05826e08) * (float10)_DAT_005528e0);
            fVar9 = fVar9 * (float10)_DAT_005528b8 + (float10)_DAT_00552928;
            local_24 = (float)(fVar9 * (float10)_DAT_00552d0c);
            local_20 = (float)fVar9; local_1c = (float)fVar9;
            BMD_TransformPosition(pModel, (float *)&DAT_06970afc, local_c, &local_18, '\0');
            pfVar3 = (float *)FUN_004795c0(0x597, &local_18, (float)fVar9 * _DAT_005528f0, &local_24, param_1, 0, 0);
            return pfVar3;
        case 0x40:
            fVar9 = (float10)fsin(((float10)*(float *)(param_1 + 0x24) * (float10)_DAT_00552660
                                   + (float10)DAT_05826e08) * (float10)_DAT_005528e0);
            fVar14 = (float)(fVar9 * (float10)_DAT_005528b8 + (float10)_DAT_00552928);
            local_20 = fVar14 * _DAT_00552d08;
            local_24 = fVar14; local_1c = local_20;
            BMD_TransformPosition(pModel, (float *)&DAT_06970afc, local_c, &local_18, '\0');
            pfVar3 = (float *)FUN_004795c0(0x597, &local_18, fVar14 * _DAT_005528f0, &local_24, param_1, 0, 0);
            return pfVar3;
        case 0x46:
            *(undefined4 *)(param_1 + 0x58) = 0xfffffffe;
            iVar2 = _rand();
            if (iVar2 % 5 == 0) {
                pfVar3 = (float *)Particle_Spawn(0x4c4, (float *)(param_1 + 0x10), (float *)(param_1 + 0x1c), (float *)(param_1 + 0xe8), 7, *(float *)(param_1 + 0xc), 0);
                return pfVar3;
            }
            break;
        case 0x4c:
            *(undefined4 *)(param_1 + 0x58) = 0xfffffffe;
            uVar11 = (longlong)DAT_05826e08;   // IDA: (__int64)WorldTime % 5000 > 4500
            uVar11 = (ulonglong)(uint)((int)uVar11 >> 0x1f) << 0x20 | uVar11 & 0xffffffff;
            if (0x1194 < (int)((longlong)uVar11 % 5000)) {
                pfVar3 = (float *)Particle_Spawn(0x4c4, (float *)(param_1 + 0x10), (float *)(param_1 + 0x1c), (float *)(param_1 + 0xe8), 4, *(float *)(param_1 + 0xc), 0);
                return pfVar3;
            }
            break;
        case 0x53:
            *(undefined4 *)(param_1 + 0x58) = 0xfffffffe;
            lVar10 = (longlong)(*(float *)(param_1 + 0x24));   // IDA: 10 * (__int64)*(float*)(o+36)
            iVar7 = (int)lVar10 * 10;
            uVar11 = (longlong)DAT_05826e08;   // IDA: (__int64)WorldTime % 10000
            iVar2 = (int)((longlong)((ulonglong)(uint)((int)uVar11 >> 0x1f) << 0x20 | uVar11 & 0xffffffff) % 10000);
            if (((int)(iVar7 + 0xdac) < iVar2) && (iVar2 < iVar7 + 4000)) {
                pfVar13 = (float *)(param_1 + 0xe8);
                pfVar6  = (float *)(param_1 + 0x1c);
                pfVar12 = (float *)(param_1 + 0x10);
                local_24 = 1.0f; local_20 = 1.0f; local_1c = 1.0f;
                Particle_Spawn(0x4c4, pfVar12, pfVar6, pfVar13, 8, *(float *)(param_1 + 0xc), 0);
                iVar2 = _rand();
                if (iVar2 % 3 == 0) {
                    uVar4 = _rand() & 0x8000007f;
                    if ((int)uVar4 < 0) uVar4 = (uVar4 - 1 | 0xffffff80) + 1;
                    local_18 = (float)(int)(uVar4 - 0x40) + *pfVar12;
                    uVar4 = _rand() & 0x8000007f;
                    if ((int)uVar4 < 0) uVar4 = (uVar4 - 1 | 0xffffff80) + 1;
                    local_10 = *(float *)(param_1 + 0x18);
                    local_14 = (float)(int)(uVar4 - 0x40) + *(float *)(param_1 + 0x14);
                    Particle_Spawn(0x4c4, &local_18, pfVar6, pfVar13, 4, *(float *)(param_1 + 0xc) * _DAT_00552504, 0);
                    bVar18 = 0;
                    pfVar17 = (float *)0x0; pfVar16 = (float *)0xffffffff;
                    pfVar15 = (float *)0x0; pfVar3  = (float *)0x0;
                    uVar4 = _rand() & 0x80000001;
                    if ((int)uVar4 < 0) uVar4 = (uVar4 - 1 | 0xfffffffe) + 1;
                    pfVar3 = (float*)Effect_Create(uVar4 + 0xc5, pfVar12, pfVar6, pfVar13, pfVar3, pfVar15, pfVar16, pfVar17, bVar18);
                    return pfVar3;
                }
            }
            break;
        }
        break;

    // ── case 9: sub-type 2 (entity type 2) — slash trails ────────────────────
    case 9:
        pfVar3 = (float *)(*(short *)(param_1 + 2) + -2);
        if (pfVar3 == (float *)0x0) {
            local_24 = 1.0f; local_20 = 1.0f; local_1c = 1.0f;
            local_c[0] = -15.0f; local_c[1] = 0.0f; local_c[2] = 0.0f;
            uVar4 = _rand() & 0x80000003;
            bVar8 = (uVar4 == 0);
            if ((int)uVar4 < 0) bVar8 = ((uVar4 - 1 | 0xfffffffc) == 0xffffffff);
            if (bVar8) {
                BMD_TransformPosition(pModel, (float *)&DAT_06970eec, local_c, &local_18, '\0');
                Particle_Spawn(0x68, &local_18, (float *)(param_1 + 0x1c), &local_24, 0, 1.0f, 0);
            }
            uVar4 = _rand() & 0x80000003;
            bVar8 = (uVar4 == 0);
            if ((int)uVar4 < 0) bVar8 = ((uVar4 - 1 | 0xfffffffc) == 0xffffffff);
            if (bVar8) {
                BMD_TransformPosition(pModel, (float *)&DAT_0697106c, local_c, &local_18, '\0');
                Particle_Spawn(0x68, &local_18, (float *)(param_1 + 0x1c), &local_24, 0, 1.0f, 0);
            }
            local_c[0] = -15.0f; local_c[1] = 0.0f; local_c[2] = 0.0f;
            BMD_TransformPosition(pModel, (float *)&DAT_06970eec, local_c, &local_18, '\0');
            pfVar3 = (float *)Particle_Spawn(0x68, &local_18, (float *)(param_1 + 0x1c), &local_24, 0, 1.0f, 0);
            return pfVar3;
        }
        break;

    // ── case 0xa: skill particle emitters (types 0–5, 0xa) ───────────────────
    case 0xa:
        pfVar3 = (float *)(int)*(short *)(param_1 + 2);
        switch ((int)pfVar3) {
        case 0: case 1: case 2: {
            if (*(int *)(param_1 + 0x58) != -2) {
                // IDA pasa `Angle` (reusado como Light) = (0.1, 0.1, 0.1) a
                // Particle_Spawn. Estaban como tres escalares sueltos y se pasaba
                // &loc30 como vec3: sin contiguidad garantizada los canales G y B
                // salian de basura — las nubes de Icarus se veian verdes y magenta.
                float loc30[3] = { 0.1f, 0.1f, 0.1f };
                int nIter = 0x14;
                do { pfVar3 = (float *)Particle_Spawn(0x4f4, (float *)(param_1 + 0x10), (float *)(param_1 + 0x1c), loc30, (int)*(short *)(param_1 + 2), *(float *)(param_1 + 0xc), param_1); } while (--nIter);
            }
            *(undefined4 *)(param_1 + 0x58) = 0xfffffffe;
            return pfVar3;
        }
        case 3: case 4: case 5: {
            if (*(int *)(param_1 + 0x58) != -2) {
                // IDA pasa `Angle` (reusado como Light) = (0.1, 0.1, 0.1) a
                // Particle_Spawn. Estaban como tres escalares sueltos y se pasaba
                // &loc30 como vec3: sin contiguidad garantizada los canales G y B
                // salian de basura — las nubes de Icarus se veian verdes y magenta.
                float loc30[3] = { 0.1f, 0.1f, 0.1f };
                int nIter = 10;
                do { pfVar3 = (float *)Particle_Spawn(0x4f4, (float *)(param_1 + 0x10), (float *)(param_1 + 0x1c), loc30, (int)*(short *)(param_1 + 2), *(float *)(param_1 + 0xc), param_1); } while (--nIter);
            }
            *(undefined4 *)(param_1 + 0x58) = 0xfffffffe;
            return pfVar3;
        }
        case 0xa:
            local_c[0] = 0.0f; local_c[1] = 0.0f; local_c[2] = 0.0f;
            BMD_TransformPosition(pModel, (float *)&DAT_06970b2c, local_c, &local_18, '\0');
            local_24 = 1.0f; local_20 = 1.0f; local_1c = 1.0f;
            pfVar3 = (float *)Particle_Spawn(0x47e, &local_18, (float *)(param_1 + 0x1c), &local_24, 0, 1.0f, 0);
            return pfVar3;
        }
        break;

    // ── cases 0xb–0x10: misc particle types ──────────────────────────────────
    case 0xb: case 0xc: case 0xd: case 0xe: case 0xf: case 0x10:
        iVar2 = (int)*(short *)(param_1 + 2);
        if (iVar2 == 0xb) {
            // 7-bone wave particles (bone indices from embedded table at local_30)
            iVar7 = 0;
            fVar9 = (float10)fsin(((float10)*(float *)(param_1 + 0x24) * (float10)_DAT_005524fc
                                   + (float10)DAT_05826e08) * (float10)_DAT_00552500);
            fVar9 = (fVar9 + (float10)_DAT_0055256c) * (float10)_DAT_00552504;
            local_24 = (float)fVar9;
            local_20 = (float)(fVar9 * (float10)_DAT_00552504);
            // local_30 area: embedded byte table {0,x,y,...} stride 1
            do {
                local_c[0] = 0.0f; local_c[1] = 0.0f; local_c[2] = 2.0f;
                BMD_TransformPosition(pModel,
                    // byte-level indexing into g_BoneScratch: bone_idx * 0x30 bytes.
                    // (char*) cast is mandatory — without it DWORD* stride multiplies by 4.
                    (float *)((char *)&DAT_06970a9c + ((char *)(&local_30))[iVar7] * 0x30),
                    local_c, &local_18, '\0');
                pfVar3 = (float *)FUN_004795c0(0x47e, &local_18, 0.5f,&local_24, param_1, 0, 0);
                iVar7++;
            } while (iVar7 < 7);
        } else if (iVar2 == 0xd) {
            fVar9 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_00552500);
            local_c[0] = 0.0f; local_c[1] = 0.0f; local_c[2] = 0.0f;
            fVar9 = fVar9 * (float10)_DAT_005528b8 + (float10)_DAT_00552928;
            local_24 = (float)fVar9; local_20 = (float)fVar9; local_1c = (float)fVar9;
            BMD_TransformPosition(pModel, (float *)&DAT_06970b2c, local_c, &local_18, '\0');
            pfVar3 = (float *)FUN_004795c0(0x4e1, &local_18, (float)fVar9 + _DAT_00552504, &local_24, param_1, 0, 0);
            return pfVar3;
        } else if ((iVar2 - 0x25) == 0) {
            // Type 0x25 — occasional lightning streaks
            local_24 = 1.0f; local_20 = 1.0f; local_1c = 1.0f;
            uVar4 = _rand() & 0x80000001;
            bVar8 = (uVar4 == 0);
            if ((int)uVar4 < 0) bVar8 = ((uVar4 - 1 | 0xfffffffe) == 0xffffffff);
            if (bVar8) {
                uVar11 = (longlong)(*(float *)(param_1 + 0x80) + 2.0f);   // IDA: (int)(__int64)(*(float*)(o+128)+2.0) % 4
                uVar4 = (uint)uVar11 & 0x80000003;
                bVar8 = ((uVar11 & 0x80000003) == 0);
                if ((int)uVar4 < 0) bVar8 = ((uVar4 - 1 | 0xfffffffc) == 0xffffffff);
                *(float *)(param_1 + 0x80) = *(float *)(param_1 + 0x80) + _DAT_0055256c;
                if (bVar8) {
                    Particle_Spawn(0x566, (float *)(param_1 + 0x10), (float *)(param_1 + 0x1c), &local_24, 0, 1.0f, 0);
                    Particle_Spawn(0x565, (float *)(param_1 + 0x10), (float *)(param_1 + 0x1c), &local_24, 0, 1.0f, 0);
                }
            }
            uVar4 = _rand() & 0x80000001;
            pfVar3 = (float *)(uVar4);
            if ((int)pfVar3 < 0) pfVar3 = (float *)(((int)pfVar3 - 1U | 0xfffffffe) + 1);
            if (pfVar3 == (float *)0x0) {
                uVar11 = (longlong)(*(float *)(param_1 + 0x80));   // IDA: (__int64)*(float*)(o+128) % 4
                pfVar3 = (float *)((uint)uVar11 & 0x80000003);
                bVar8 = ((uVar11 & 0x80000003) == 0);
                if ((int)pfVar3 < 0) {
                    pfVar3 = (float *)(((int)pfVar3 - 1U | 0xfffffffc) + 1);
                    bVar8 = (pfVar3 == (float *)0x0);
                }
                *(float *)(param_1 + 0x80) = *(float *)(param_1 + 0x80) + _DAT_0055256c;
                if (bVar8) {
                    pfVar3 = (float *)(param_1 + 0x1c);
                    pfVar6 = (float *)(param_1 + 0x10);
                    Particle_Spawn(0x4f4, pfVar6, pfVar3, &local_24, 6, 1.0f, 0);
                    Particle_Spawn(0x565, pfVar6, pfVar3, &local_24, 1, 1.0f, 0);
                    local_24 = 1.0f; local_20 = 0.8f; local_1c = 0.8f;
                    pfVar3 = (float *)Particle_Spawn(0x4e1, pfVar6, pfVar3, &local_24, 4, 0.19f, 0);
                    return pfVar3;
                }
            }
        }
        break;
    }

switchD_004fc1a7_caseD_1:
    return pfVar3;
}
