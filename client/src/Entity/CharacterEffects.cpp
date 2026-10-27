#include "stdafx.h"
#include "Entity/CharacterEffects.h"
#include "Render/EquipmentVisuals.h"
#include "Item/ContentCatalog.h"
#include "Config/UserSettings.h"
#include "Game/MapManager.h"

#define local_60 (scratch.light[0])
#define local_5c (scratch.light[1])
#define local_58 (scratch.light[2])
#define local_48 (scratch.offset[0])
#define local_44 (scratch.offset[1])
#define local_40 (scratch.offset[2])
#define local_54 (scratch.position[0])
#define local_50 (scratch.position[1])
#define local_4c (scratch.position[2])
#define local_30 (scratch.previous[0])
#define local_2c (scratch.previous[1])
#define local_28 (scratch.previous[2])
#define local_3c (scratch.extra[0])
#define local_38 (scratch.extra[1])
#define local_34 (scratch.extra[2])

// IDA: emisores iniciales de RenderCharacter (0x00456770).
// DESVIACION: azar y emisiones persistentes se evalúan sólo en el paso SIM.
void CharacterEffects_TickEarly(int entity, CharacterEffectScratch &scratch) {
    int *param_1 = (int *)entity, *puVar13 = (int *)entity;
    short sVar2 = AsObject(entity)->Type;
    int entity_type = sVar2;
    BYTE bVar7 = AsCharacter(entity)->MonsterIndex;
    void *model = (void *)(DAT_05828d58 + entity_type * 0xbc);
    const CatalogMonster *monster = entity_type != 390 ? gContentCatalog.GetMonster(bVar7) : nullptr;
    if (monster && monster->Golden) return;
    if (bVar7 == 0x45) {
        // 9-bone glitter + 3 random-bone sparks
        const float sparkle = (float)(rand() % 30 + 70) * 0.01f;
        local_60 = sparkle * 0.8f;
        local_5c = sparkle * 0.9f;
        local_58 = sparkle;
        BYTE *boneIdxTable = (BYTE *)&DAT_0055984c;
        for (int i = 0; i < 9; i++) {
            BMD_TransformPosition(model,
                (float *)(puVar13[0x45] + (int)(UINT)boneIdxTable[i] * 0x30),
                &local_48, &local_54, '\x01');
            Equipment_AddSprite(0x47e, &local_54, 0.6f, &local_60, (int)puVar13, 0.0f, 0);
        }
        // IDA changes only the random-particle tint after the nine sprites.
        local_60 = sparkle * 0.6f;
        local_5c = sparkle * 0.7f;
        local_58 = sparkle * 0.8f;
        int nBones = *(short *)((char *)model + 0x22);
        for (int i = 0; i < 3; i++) {
            // IDA: v242[0..2] = rand() % 20 - 10 antes de cada particula.
            // Sin esto las tres chispas nacen clavadas en el hueso.
            local_48 = (float)(rand() % 20 - 10);
            local_44 = (float)(rand() % 20 - 10);
            local_40 = (float)(rand() % 20 - 10);
            int r = rand();
            BMD_TransformPosition(model,
                (float *)(puVar13[0x45] + (r % nBones) * 0x30),
                &local_48, &local_54, '\x01');
            Particle_Spawn(0x498, &local_54, (float *)(puVar13 + 7),
                         &local_60, 3, 1.0f, 0);
        }
    }
    else if (bVar7 == 0x46) {
        BMD_TransformPosition(model, (float *)(puVar13[0x45] + 0x3c0),
                     &local_48, &local_54, '\x01');
        Equipment_AddSprite(0x47e, &local_54, 0.8f, &local_60, (int)puVar13, 0.0f, 0);
    }
    else if (bVar7 == 0x49) {
        // Drakan (MonsterID 73). IDA 00456770 `case 'I'` con 747 == 73.
        // Sprites 1150 sobre dos tramos de huesos, mas una cadena de joints
        // 1254 entre huesos consecutivos.
        //
        // IDA fija el color ANTES de la cadena: Light = (0.1, 0.1, 1.0), o sea
        // AZUL. El port no lo hacia y heredaba el (1,1,1) que queda seteado
        // antes del switch (raw L290-292), asi que la cadena salia blanca.
        local_60 = 0.1f;   // Light[0]
        local_5c = 0.1f;   // Light[1]
        local_58 = 1.0f;   // Light[2]
        for (int off = 0x270; off < 0x510; off += 0x30) {
            BMD_TransformPosition(model, (float *)(puVar13[0x45] + off),
                         &local_48, &local_54, '\x01');
            Equipment_AddSprite(0x47e, &local_54, 0.8f, &local_60, (int)puVar13, 0.0f, 0);
            // IDA: `if (v29 >= 672 && v29 <= 768 || v29 == 1104)`.
            // Al port le faltaba el `|| off == 0x450` (hueso 23), o sea un
            // tramo de la cadena no se dibujaba.
            if ((off >= 0x2a0 && off <= 0x300) || off == 0x450) {
                Joint_Create(0x4e6, &local_30, &local_54,
                             (float *)(puVar13 + 7), 7, 0, 20.0f, -1, 0);
            }
            local_30 = local_54; local_2c = local_50; local_28 = local_4c;
        }
        for (int off = 0x9c0; off < 0xb10; off += 0x30) {
            BMD_TransformPosition(model, (float *)(puVar13[0x45] + off),
                         &local_48, &local_54, '\x01');
            Equipment_AddSprite(0x47e, &local_54, 0.8f, &local_60, (int)puVar13, 0.0f, 0);
        }
    }
    else if (bVar7 == 0x4b) {
        BMD_TransformPosition(model, (float *)(puVar13[0x45] + 0x360),
                     &local_48, &local_54, '\x01');
        Particle_Spawn(0x4ab, &local_54, (float *)(puVar13 + 7),
                     &local_60, 0, 0.3f, 0);
    }
}

// IDA: switch final por modelo de RenderCharacter (0x00456770).
void *CharacterEffects_TickLate(int entity, CharacterEffectScratch &scratch) {
    int *param_1 = (int *)entity, *puVar13 = (int *)entity;
    short sVar2 = AsObject(entity)->Type;
    int entity_type = sVar2;
    BYTE bVar7 = AsCharacter(entity)->MonsterIndex;
    void *model = (void *)(DAT_05828d58 + entity_type * 0xbc);
    void *pvVar23 = model;
    RequestTerrainLight(AsObject(entity)->Position[0], AsObject(entity)->Position[1], scratch.light);
    // ── 8. Death / PvP color tint ────────────────────────────────────────────
    // [hash-table obfuscation blocks skipped — anti-tamper, not game logic]
    if ((*(BYTE *)((char *)DAT_07cf1ff4 + 0x28) & 1) != 0) {
        local_44 = 0.9f; local_40 = 0.5f; local_3c = 0.5f;
    }
    if ((*(BYTE *)((char *)DAT_07cf1ff4 + 0x28) & 2) != 0) {
        local_44 *= _DAT_00552504;
        local_40 *= _DAT_005526e8;
        local_3c *= _DAT_00552504;
    }

    // ── 9. Entity-type outer switch — NPC / monster special effects ──────────
    // Se conserva el modelo de la pose SIM para los emisores del cuerpo.
    pvVar23 = model;

    switch (sVar2) {

    case 0x10e:  // Boat-like entity type 270
    case 300:
        // If weapon anim state == 1 (armed standing) or type == 300: sail effect
        if (((sVar2 == 0x10e) && (*(short *)((int)param_1 + 0x1be) == 1)) ||
            (sVar2 == 300)) {
            puVar13 = (int *)Entity_SpawnBoneRangeEffect((int)puVar13, 0x16, 0x17, 1.0f);
            return puVar13;
        }
        break;

    case 0x12a:  // Entity 298 — multi-bone particle rain (two tints based on flag)
    {
        int iVar9  = 0;
        int iVar18 = 0;
        local_48 = 0.0f; local_44 = 0.0f; local_40 = 0.0f;
        bool bAlt = (*(short *)((int)param_1 + 0x1be) != 0);
        if (bAlt) {
            local_60 = 0.2f; local_5c = 0.7f; local_58 = 0.1f;
        } else {
            local_60 = 1.0f; local_5c = 1.0f; local_58 = 1.0f;
        }
        int nBones = *(short *)((char *)pvVar23 + 0x22);
        while (iVar18 < nBones) {
            // Skip bones in exclusion ranges (model-specific blank bones)
            if ((*(char *)(*(int *)((int)pvVar23 + 0x2c) + 0x22 + iVar9) == '\0') &&
                ((iVar9 < 0x834 || iVar9 > 0xaf0)) &&
                ((iVar9 < 0xec4 || iVar9 > 0x1180))) {
                BMD_TransformPosition(pvVar23, (float *)(puVar13[0x45] + iVar18 * 0x30),
                             &local_48, &local_54, '\x01');
                UINT uType = bAlt ? 0x4f0 : 0x4cf;
                float fSc  = bAlt ? 1.3f : 2.5f;
                Equipment_AddSprite((int)uType, &local_54, fSc, &local_60, (int)puVar13, 0, (int)!bAlt);
                // Random 1/4 chance: add spark if walking
                BYTE bAnimState = *(BYTE *)((int)puVar13 + 0x105);
                if ((rand() & 3) == 0 && bAnimState >= 3 && bAnimState <= 4)
                    Particle_Spawn(0x49c, &local_54, (float *)(puVar13 + 7),
                                 &local_60, 0, 1.0f, 0);
            }
            iVar9  += 0x8c;
            iVar18 += 1;
        }
        break;
    }

    case 0x12f:  // Entity 303 — dual smoke/fire column
        Entity_SpawnBoneEffect((int)puVar13, 0x497, 4.0f, 9, 0.0f, 0, 5.0f);
        puVar13 = (int *)Entity_SpawnBoneEffect((int)puVar13, 0x4d0, 3.0f, 9, 0.0f, 0, 5.0f);
        return puVar13;

    case 0x130:  // Entity 304 — six-point smoke/fire/magic
        Entity_SpawnBoneEffect((int)puVar13, 0x4a7, 0.5f, 0x1e, 0.0f, 0, -5.0f);
        Entity_SpawnBoneEffect((int)puVar13, 0x4a7, 0.5f, 0x27, 0.0f, 0, -5.0f);
        Entity_SpawnBoneEffect((int)puVar13, 0x497, 4.0f, 0x1e, 0.0f, 0, -5.0f);
        Entity_SpawnBoneEffect((int)puVar13, 0x497, 4.0f, 0x27, 0.0f, 0, -5.0f);
        Entity_SpawnBoneEffect((int)puVar13, 0x4d0, 2.0f, 0x1e, 0.0f, 0, -5.0f);
        puVar13 = (int *)Entity_SpawnBoneEffect((int)puVar13, 0x4d0, 2.0f, 0x27, 0.0f, 0, -5.0f);
        return puVar13;

    case 0x132:  // Entity 306 — claw/spider: beam + 8 effect bones
        Entity_SpawnBoneRangeEffect((int)puVar13, 0x2a, 0x2b, 1.0f);
        Entity_SpawnBoneEffect((int)puVar13, 0x497, 2.0f, 0x1a, 0.0f, 0, 0.0f);
        Entity_SpawnBoneEffect((int)puVar13, 0x497, 2.0f, 0x1f, 0.0f, 0, 0.0f);
        Entity_SpawnBoneEffect((int)puVar13, 0x497, 2.0f, 0x24, 0.0f, 0, 0.0f);
        Entity_SpawnBoneEffect((int)puVar13, 0x497, 2.0f, 0x29, 0.0f, 0, 0.0f);
        Entity_SpawnBoneEffect((int)puVar13, 0x4d0, 1.0f, 0x1a, 0.0f, 0, 0.0f);
        Entity_SpawnBoneEffect((int)puVar13, 0x4d0, 1.0f, 0x1f, 0.0f, 0, 0.0f);
        Entity_SpawnBoneEffect((int)puVar13, 0x4d0, 1.0f, 0x24, 0.0f, 0, 0.0f);
        puVar13 = (int *)Entity_SpawnBoneEffect((int)puVar13, 0x4d0, 1.0f, 0x29, 0.0f, 0, 0.0f);
        return puVar13;

    case 0x133:  // Entity 307 — large fire/smoke at bone 0x3f
        Entity_SpawnBoneEffect((int)puVar13, 0x4a7, 1.0f, 0x3f, 0.0f, 0, 20.0f);
        puVar13 = (int *)Entity_SpawnBoneEffect((int)puVar13, 0x4d0, 4.0f, 0x3f, 0.0f, 0, 20.0f);
        return puVar13;

    case 0x142:  // Entity 322 — dual beam bones 0x1a/0x1b  scale=2
        puVar13 = (int *)Entity_SpawnBoneRangeEffect((int)puVar13, 0x1a, 0x1b, 2.0f);
        return puVar13;

    case 0x15c:  // Entity 348 — soft sparkle at bone 0x20
        puVar13 = (int *)Entity_SpawnBoneEffect((int)puVar13, 0x47e, 1.5f, 0x20, 0.0f, 0, 0.0f);
        return puVar13;

    case 0x15d:  // Entity 349 — sin-wave orb cloud + occasional lightning joint
    {
        float fSin = (float)(double)fsin((double)(WorldTime * _DAT_005528e0));
        local_48 = 3.5f; local_44 = -12.0f; local_40 = 10.0f;
        float fCol = fSin * _DAT_0055294c + _DAT_00552948;
        local_60 = fCol; local_5c = fCol; local_58 = fCol;
        BMD_TransformPosition(pvVar23, (float *)(puVar13[0x45] + 0x3c0),
                     &local_48, &local_54, '\x01');
        Equipment_AddSprite(0x4a7, &local_54, 0.3f, &local_60, (int)puVar13,
                     (float)WorldTime * _DAT_00552944, 0);
        Equipment_AddSprite(0x4a7, &local_54, 0.3f, &local_60, (int)puVar13,
                     -(float)WorldTime * _DAT_00552944, 0);
        if (rand() % 0x1e == 0) {
            // Scatter lightning from random offsets around bone position
            local_48 = (float)(rand() % 100) + local_54 - _DAT_00552598;
            local_44 = (float)(rand() % 100) + local_50 - _DAT_00552598;
            local_40 = (float)(rand() % 100) + local_4c - _DAT_00552598;
            puVar13 = (int *)Joint_Create(0x4ea, &local_48, &local_54,
                                          (float *)(puVar13 + 7), 6, 0, 20.0f, -1, 0);
            return puVar13;
        }
        break;
    }

    case 0x176:  // Entity 374 — sparkle at bone 6  scale=2
        puVar13 = (int *)Entity_SpawnBoneEffect((int)puVar13, 0x47e, 2.0f, 6, 0.0f, 0, 0.0f);
        return puVar13;

    case 0x186:  // Entity 390 (Player) — port completo IDA case 390 (líneas 2111-2269)
    {
        // CheckFullSet detecta si las 5 piezas de armadura forman un set
        // matcheado con level >= 9 → setea `EquipmentLevelSet` global y devuelve
        // true si el char tiene 5 piezas (aunque no matcheen, para v230=true).
        bool v230 = CheckFullSet(reinterpret_cast<uintptr_t>(param_1));
        local_48 = 0.0f; local_44 = 0.0f; local_40 = 0.0f;

        if (SceneFlag == 2) {
            return puVar13;
        }

        // ── 1. Glove sparkle: class byte (cls&7)==0 + (cls_skin&0xF8)==8 ───
        // (DK 2nd-tier with specific skin range): sprite + sin halo at bone 19
        if (((*(BYTE *)((int)param_1 + 0x1bc) & 7) == 0) &&
            ((*(char *)((int)param_1 + 0x1bd) & (char)~7) == '\b')) {
            local_48 = -4.0f; local_44 = 11.0f; local_40 = 0.0f;
            local_60 = 1.0f; local_5c = 1.0f; local_58 = 1.0f;
            BMD_TransformPosition(pvVar23, (float *)(puVar13[0x45] + 0x390),
                         &local_48, &local_54, '\x01');
            Equipment_AddSprite(0x498, &local_54, 0.6f, &local_60, 0, 0, 0);
            float fS = (float)(double)fsin((double)(WorldTime * _DAT_00552500));
            Equipment_AddSprite(0x4cf, &local_54, (float)(fS * _DAT_005528b4),
                         &local_60, 0, 0, 0);
        }

        // ── 2. PLAYER_SPELL anim (88 = 'X') — orbit sparkles (3 sprites) ──
        // IDA línea 2133-2152. Bone idx en c+628 (= LinkBone Weapon[0]),
        // tres Equipment_AddSprite(1191) con rotación basada en WorldTime.
        if (*(char *)((int)puVar13 + 0x105) == 'X') {
            BYTE bone = *(BYTE *)(param_1 + 0x9d);  // = (param_1+0x9d*4-1)? no: int-idx 0x9d * 4 = byte 0x274 = c+628 LinkBone[0]
            // Wait int-arithmetic: param_1 (int*) + 0x9d ints == byte+0x274 == c+628 ✓
            BMD_TransformPosition(pvVar23,
                (float *)(puVar13[0x45] + (UINT)bone * 0x30),
                &local_48, &local_54, '\x01');
            float Scalep = *(float *)((int)puVar13 + 0x108) * 0.1f;  // anim frame * 0.1
            local_60 = 0.1f; local_5c = 0.1f; local_58 = 1.0f;
            Equipment_AddSprite(0x4a7, &local_54, Scalep * 0.30000001f,
                         &local_60, (int)puVar13, 0, 0);
            Equipment_AddSprite(0x4a7, &local_54, Scalep, (float*)(puVar13 + 0x3a),
                         (int)puVar13, -WorldTime * 0.1f, 0);
            Equipment_AddSprite(0x4a7, &local_54, Scalep * 2.5f, (float*)(puVar13 + 0x3a),
                         (int)puVar13, WorldTime * 0.1f, 0);
        }

        // ── 3. Dragon mode (entity flags bit 2) — fire spray from hand bones ──
        if ((*(BYTE *)(puVar13 + 0x1e) & 4) == 4) {
            BYTE *pbVar20 = (BYTE *)(param_1 + 0x9d);  // c+628 = first weapon LinkBone
            for (int iSlot = 2; iSlot > 0; iSlot--, pbVar20 += 0x18) {
                int iRnd = rand();
                local_60 = (float)(iRnd % 0x1e + 0x46) * _DAT_005524f8;
                local_5c = (float)local_60 * _DAT_005528b8;
                local_58 = (float)local_60 * _DAT_005526e4;
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + (UINT)*pbVar20 * 0x30),
                    &local_48, &local_54, '\x01');
                Equipment_AddSprite(0x4cf, &local_54, 1.5f, &local_60, (int)puVar13, 0, 0);
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbVar20 - 6) * 0x30),
                    &local_48, &local_54, '\x01');
                Equipment_AddSprite(0x4cf, &local_54, 1.5f, &local_60, (int)puVar13, 0, 0);
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbVar20 - 7) * 0x30),
                    &local_48, &local_54, '\x01');
                Equipment_AddSprite(0x4cf, &local_54, 1.5f, &local_60, (int)puVar13, 0, 0);
            }
        }

        // ── 4. Sin full set (v230==false) → no glow, exit ─────────────────
        // DESVIACION (antilag propio): sin el brillo del set completo.
        if (!v230 || gUserSettings.GetAntilag(ANTILAG_GLOW)) {
            return puVar13;
        }

        // ── 5. Full set body glow: PartObjectColor + 6 sparkles (3 bones × 2) ──
        // IDA línea 2191: PartObjectColor(c->Boot.Type, alpha, 0.5, Light, 0)
        // → escribe en Light[3] el RGB del glow basado en el tipo de bota.
        PartObjectColor(*(short*)((int)param_1 + 600),
                     *(float *)((int)puVar13 + 0x168),
                     0.5f, &local_60, '\0');

        // Light = local_60..local_58 (RGB output del PartObjectColor)
        // Spawn 3 sprites por weapon slot (Weapon[0] y Weapon[1]) en bones
        // [bone, bone-6, bone-7] — esos son los huesos cercanos a la mano:
        //   bone (≈19/20)   = mano principal
        //   bone-6 (≈13/14) = brazo
        //   bone-7 (≈12/13) = hombro
        {
            BYTE *pbV193 = (BYTE *)(param_1 + 0x9d);  // c+628 = LinkBone[Weapon0]
            for (int v194 = 2; v194 > 0; v194--, pbV193 += 0x18) {
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + (UINT)*pbV193 * 0x30),
                    &local_48, &local_54, '\x01');
                Equipment_AddSprite(0x47e, &local_54, 1.3f, &local_60, (int)puVar13, 0, 0);  // CreateSprite 1150
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbV193 - 6) * 0x30),
                    &local_48, &local_54, '\x01');
                Equipment_AddSprite(0x47e, &local_54, 1.3f, &local_60, (int)puVar13, 0, 0);
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbV193 - 7) * 0x30),
                    &local_48, &local_54, '\x01');
                Equipment_AddSprite(0x47e, &local_54, 1.3f, &local_60, (int)puVar13, 0, 0);
            }
        }

        // ── 5b. WATERFALL particles at bone 0 — REVERTIDO ───────────────────
        // Port de 5.2 quitado: 0.97k NO tiene esto, y el spawn rate provocaba
        // particle whiteout (todos accumulating sin morir). El +9 glow del
        // 0.97k es solo los 6 weapon-bone sprites + lightning crackle 1/20.

        // ── 6. EquipmentLevelSet >= 10 — extra lightning trail (1/20 chance) ──
        if (EquipmentLevelSet <= 9 || (rand() % 20)) {
            return puVar13;
        }
        // Save entity light, set to white temporarily for the lightning render
        float fSaveX = *(float *)(puVar13 + 0x3a);
        float fSaveY = *(float *)(puVar13 + 0x3b);
        float fSaveZ = *(float *)(puVar13 + 0x3c);
        *(float *)(puVar13 + 0x3a) = 1.0f;
        *(float *)(puVar13 + 0x3b) = 1.0f;
        *(float *)(puVar13 + 0x3c) = 1.0f;
        float *pfVar14 = (float *)(puVar13 + 0x3a);

        if (EquipmentLevelSet == 10) {
            // Single lightning particle around the entity
            Particle_Spawn(0x4e1, (float *)(puVar13 + 4),
                         (float *)(puVar13 + 7), pfVar14, 0, 0.19f, (int)puVar13);
        } else if (EquipmentLevelSet == 11) {
            // 1/8: lightning joint, 7/8: particle
            int v198 = rand() & 0x80000007;
            bool v197 = (v198 == 0);
            if (v198 < 0) {
                v197 = (((BYTE)v198 - 1) | 0xFFFFFFF8) == 0xFFu;
            }
            if (v197) {
                Joint_Create(0x4e1, (float *)(puVar13 + 4),
                             (float *)(puVar13 + 4), (float *)(puVar13 + 7),
                             0, (int)puVar13, 10.0f, -1, 0);
            } else {
                Particle_Spawn(0x4e1, (float *)(puVar13 + 4),
                             (float *)(puVar13 + 7), pfVar14, 0, 0.19f, (int)puVar13);
            }
        }
        // Restore entity light
        *(float *)(puVar13 + 0x3a) = fSaveX;
        *(float *)(puVar13 + 0x3b) = fSaveY;
        *(float *)(puVar13 + 0x3c) = fSaveZ;
        return puVar13;
    }

    } // end switch(sVar2)

    return puVar13;
}
