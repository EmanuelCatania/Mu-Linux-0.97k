#include "stdafx.h"
#include "Item/ContentCatalog.h"
#include "Config/UserSettings.h"
#include "Game/MapManager.h"

// GridSpring_Create guarda su 2do arg con `*(float*)(thiz+4) = entity` y después lo
// RELEE como puntero (`*(int*)(thiz+4)`). Convertir el puntero a float lo
// destruye (float tiene 24 bits de mantisa), así que hay que pasar los BITS.
// IDA aloca los widgets de tela con el prefijo de count del `eh vector
// constructor iterator`: `raw = operator_new(0x58); *raw = 1; obj = raw + 1;`
// El objeto vive en **+4** y el count en +0. El dtor (`sub_45AAA0` con flags&2)
// lee `*((int*)obj - 1)` y libera `obj - 4`, así que sin el prefijo leía el
// header del heap y liberaba un puntero inválido → AV dentro de operator_delete.
static inline void *ClothNew(void) {
    int *raw = (int *)operator_new(0x58);
    if (!raw) return nullptr;
    raw[0] = 1;                       // count = 1 elemento
    return Widget_CtorBase(raw + 1);     // el ctor recibe el objeto, no el bloque
}

static inline float PtrAsFloatBits(const void *p) {
    float f; int v = (int)(uintptr_t)p; memcpy(&f, &v, 4); return f;
}

#pragma warning(disable: 4244 4305 4701 4702 4700)
// Entity_UpdateRender.cpp  —  RenderCharacter @ 0x00456770  (2195 lines in Ghidra)
//
// Per-frame visual update for a single entity.  Called from Entity_RenderAll_3D
// for every visible entity.  Drives:
//   - Skill-channel widget objects  (channeling beams / barriers)
//   - Entity_PrepareRender          (bone + AABB compute)
//   - Per-skill / per-anim-state particle effects on entity bones
//   - Weapon-slot rendering         (RenderLinkObject)
//   - Per-entity-type NPC / monster special effects (large outer switch)
//
// param_1  — player / local entity  (int*, stride 0x394, base DAT_07abf5d0[0])
// param_2  — entity being rendered  (undefined4* / puVar13 in Ghidra)
// param_3  — zone-id or context param (treated as int for zone-scale calc)
//
// Anti-tamper: ~30 HashTable_GetIndex / HashTable_Insert / XOR-encode blocks are
// interspersed throughout; those are pure obfuscation and are omitted.

// All FUN_* prototypes and DAT_* globals come from stdafx.h → functions.h / globals.h


// ── RenderCharacter  Entity_UpdateRender ──────────────────────────────────────
extern "C" {
    // From Render_PlayerEquipment.cpp
    void Render_PlayerHelper(int c, int o);
    void Render_PlayerWeaponLoop(int c, int o);
    // Back-weapon render decision. Returns 1 if weapon was rendered
    // on back (LinkBone 47). When 1, Render_PlayerWeaponLoop should be skipped.
    int  RenderCharacterBackItem(int c, int o);
    // Watchdog por frame que restaura wings/weapons/pendant del
    // hero si fueron reseteados a -1 después de F3/03.
    void HeroEquipWatchdog(int c);
}

// IDA: RenderGuildMarkOnShield — dibuja la textura 34 ya compuesta por CreateGuildMark
// sobre el hueso 26 del modelo de jugador. El segundo parámetro es el escudo
// equipado; sólo modifica el desplazamiento vertical del emblema.
void __cdecl RenderGuildMarkOnShield(int entity, int shield_id)
{
    BYTE* object = (BYTE*)(uintptr_t)entity;

    EnableAlphaTest(true);
    GL_EnableCullFace();
    glColor3f(1.0f, 1.0f, 1.0f);
    GL_BindTextureSlot(34);
    glPushMatrix();

    float angles[3] = {
        *(float*)(object + 28) + 80.0f,
        *(float*)(object + 32) + 45.0f,
        *(float*)(object + 36) + 135.0f,
    };
    float localMatrix[12] = {};
    AngleMatrix(angles, (float (*)[4])localMatrix);
    localMatrix[3] = 20.0f;
    localMatrix[7] = -5.0f;
    localMatrix[11] = (shield_id == 676) ? -18.0f : -10.0f;

    // `object+276` es el buffer de matrices animadas; 26 * 48 = 1248.
    float* bone26 = (float*)((BYTE*)(uintptr_t)*(DWORD*)(object + 276) + 1248);
    R_ConcatTransforms(bone26, localMatrix, &DAT_06989c9c);

    glTranslatef(*(float*)(object + 16), *(float*)(object + 20), *(float*)(object + 24));
    GL_DrawBillboard(5.0f, 7.0f, &DAT_06989c9c);
    glPopMatrix();
    GL_DisableCullFace();
}

void* __cdecl RenderCharacter(void *param_1_, void *param_2_, void *param_3)
{
    int *param_1  = (int *)param_1_;
    int *puVar13  = (int *)param_2_;   // Ghidra alias for param_2
    bool bDeferWing = false;           // 0.97.20: ala del catálogo, va después del cuerpo

    // ── 1. Setup ─────────────────────────────────────────────────────────────
    short sVar2    = *(short *)((int)puVar13 + 2);        // entity_type

    int  entity_type = (int)sVar2;
    void *model = (void *)(DAT_05828d58 + entity_type * 0xbc);



    // Early-out: no animation data in this model slot
    if (*(short *)((char *)model + 0x26) == 0)
        return (void *)entity_type;

    // Arrays contiguos: `&local_X` se pasa a funciones que leen/escriben 3 floats
    // consecutivos (CreateSprite, BMD_TransformPosition, etc.).
    float local_60_buf[3] = { 1.0f, 1.0f, 1.0f }; // RGB color tint
    float local_48_buf[3] = { 0.0f, 0.0f, 0.0f }; // position offset
    float local_54_buf[3] = { 0.0f, 0.0f, 0.0f }; // transformed world pos
    float local_30_buf[3] = { 0.0f, 0.0f, 0.0f }; // prev bone pos
    float local_3c_buf[3] = { 0.0f, 0.0f, 0.0f }; // scratch
    #define local_60 (local_60_buf[0])
    #define local_5c (local_60_buf[1])
    #define local_58 (local_60_buf[2])
    #define local_48 (local_48_buf[0])
    #define local_44 (local_48_buf[1])
    #define local_40 (local_48_buf[2])
    #define local_54 (local_54_buf[0])
    #define local_50 (local_54_buf[1])
    #define local_4c (local_54_buf[2])
    #define local_30 (local_30_buf[0])
    #define local_2c (local_30_buf[1])
    #define local_28 (local_30_buf[2])
    #define local_3c (local_3c_buf[0])
    #define local_38 (local_3c_buf[1])
    #define local_34 (local_3c_buf[2])
    void *pvVar23 = NULL;   // model/BMD object (resolved via HashTable — see note)
    void *local_78 = NULL;  // same as pvVar23 in outer switch
    void *local_74 = NULL;  // dead/anim flag

    // ── 2. Switch por TIPO DE MONSTRUO (+0x2EB) ──────────────────────────────
    char cVar6 = *(char *)((int)param_1 + 0x2eb);  // tipo de monstruo
    switch (cVar6) {
    case 'Y': case '_': case 'p': case 'v': case '|':
    case (char)-0x7e: case (char)-0x78:
    {
        // Skill channel active — beam/barrier widget path
        unsigned int uVar11 = (unsigned int)(size_t)Calc_RenderObject((int)puVar13, '\x01', (int)param_3);
        if (param_1[0x61] == 0) {
            // IDA 0x456770 L308-323:
            //     v9 = (float *)(block + 4);
            //     *(_DWORD *)block = 1;                  // prefijo de count
            //     eh_vector_ctor(block + 4, 0x60, 1, sub_4093A0, sub_4093C0);
            //     sub_4093E0(v9, ...); sub_409250(v9, ...); sub_409250(v9, ...);
            //     *(_DWORD *)(c + 388) = v9;             // guarda el OBJETO
            //
            // En c+388 va el objeto (`block + 4`), no `block`: el tick de la tela
            // (`sub_408CB0`) arranca con una llamada por vtable --
            // `(*(void(**)(_DWORD*))(*a1 + 8))(a1)` -- y leeria el prefijo de count como
            // si fuera la vtable.
            //
            // Los tipos de monstruo de este case (+0x2EB: 89, 95, 112, 118, 124,
            // 130, 136) incluyen el 130 = "Magic Skeleton", que es el que aparece
            // al caer la puerta del evento de Blood Castle.
            void *puVar8 = operator_new(100);
            *(int *)puVar8 = 1;                    // count del eh vector ctor
            void *clothObj = (char *)puVar8 + 4;   // el objeto vive en +4
            L_YGXPAXIHP6EX0_Z1_Z(clothObj, 0x60, 1, (void *)Widget_Ctor);
            SpringMesh_Create(clothObj, (int)param_1, (short *)2, 0x12, 0x400, -1);
            VerletNode_AddToSystem(clothObj, 0.0f,   0.0f, 0.0f, 50.0f, 18);
            VerletNode_AddToSystem(clothObj, 0.0f, -20.0f, 0.0f, 30.0f, 18);
            param_1[0x61] = (int)clothObj;
            *(char *)(param_1 + 0x60) = 1;
        }
        int *piVar16 = (int *)param_1[0x61];
        if (piVar16) {
            int iVar9 = (int)(size_t)Widget_CheckState(piVar16, 0x3ba3d70a, 5);
            if (iVar9 == 0)
                DeleteCloth((int)param_1, (int)puVar13, 0);
            else
                FUN_00408ff0((void *)piVar16);
        }
        if ((BYTE)uVar11 != 0)
            Draw_RenderObject(puVar13, 1, (int)param_3, '\0');
        break;
    }
    default:
        // Normal state — prepare entity render (bone / AABB)
        cVar6 = ((cVar6 == 'C') || (cVar6 == 'J') || (cVar6 == 'K')) ? '\x01' : '\0';
        Entity_PrepareRender((unsigned char *)puVar13, 1, (int)param_3, cVar6);
        break;
    }
    // IDA RenderCharacter 0x456770: draw the projected shadow immediately
    // after Entity_PrepareRender and BEFORE monster glow/equipment passes.
    // o+140 selects the shadow branch in RenderPartObjectEffect (0x504B50).
    // A late opaque body pass here would cover Silver Valkyrie's glow.
    if (sVar2 != 0x186 && *(BYTE *)((char *)puVar13 + 0x84) != 8) {
        BYTE v11 = *(BYTE *)((char *)param_1 + 747);   // *(c + 747)
        // Use the global World; a same-name local here was self-initialized.
        float alpha = *(float *)((char *)puVar13 + 0x168);   // o + 360

        if (v11 != 25 && v11 != 22 && v11 != 42 && v11 != (BYTE)-14 &&
            v11 != 59 && v11 != 63 &&
            gMapManager.GetCurrentMap() != 10 && alpha >= 0.3f)
        {
            // Blood Castle (11..16): clamp Z to terrain height when
            // entity is dead+action-start (Blood Castle special case).
            if (gMapManager.GetCurrentMap() >= 11 && gMapManager.GetCurrentMap() <= 16) {
                if (*(BYTE *)((char *)puVar13 + 0x195) != 0 &&     // o+405 m_bActionStart
                    *(BYTE *)((char *)param_1 + 0x2FD) != 0) {     // c+765 Dead>0 byte
                    float wx = *(float *)((char *)puVar13 + 0x10);
                    float wy = *(float *)((char *)puVar13 + 0x14);
                    float th = RequestTerrainHeight(wx, wy);
                    if (th < *(float *)((char *)puVar13 + 0x18)) {
                        *(float *)((char *)puVar13 + 0x18) = th;
                    }
                }
            }

            // Special status -24/-23 (=232/233 unsigned) → set HiddenMesh=2
            if (v11 == (BYTE)-24 || v11 == (BYTE)-23) {
                *(int *)((char *)puVar13 + 0x58) = 2;
            }

            short v15 = sVar2;
            if (v15 != 330 && v15 != 331) {
                *(BYTE *)((char *)puVar13 + 0x8C) = 1;       // EnableShadow = 1
                RenderPartObject((int)param_1,
                             (int)v15,
                             0,
                             (float *)(param_1 + 200),       // c+800 Light
                             alpha,
                             0, 0, '\0', 0, '\x01',
                             0, 2);
                *(BYTE *)((char *)puVar13 + 0x8C) = 0;       // EnableShadow = 0
            }

            // Status -24/-23 → render alpha bitmap on terrain (water reflection)
            if (v11 == (BYTE)-24 || v11 == (BYTE)-23) {
                EnableAlphaBlend();
                float wx = *(float *)((char *)puVar13 + 0x10);
                float wy = *(float *)((char *)puVar13 + 0x14);
                double t  = (double)DAT_05826e08 * 0.0015;
                float lum = (float)(sin(t) * 0.30000001 + 0.80000001);
                float Light[3] = { lum * 0.5f, lum * 0.5f, lum };
                float Rotation = -*(float *)((char *)puVar13 + 0x24);
                RenderTerrainAlphaBitmap(1264, wx, wy, 2.7f, 2.7f, Light,
                                         Rotation, 1.0f);
                *(int *)((char *)puVar13 + 0x58) = -1;   // HiddenMesh = -1
            }
        }
    }

    // ── 4. Skill-state / anim-state particle effects ─────────────────────────
    BYTE bVar7 = *(BYTE *)((int)param_1 + 0x2eb);   // tipo de monstruo

    // IDA 00456770 groups these MonsterIDs before the individual Icarus
    // branches.  43 and 78..83 need the first 0x48 pass; 67 uses 0x144 for
    // its second pass; 59 is the only half-bright member of the group.
    // DESVIACION (DLL CustomMonsterGolden, ahora catálogo 0.97.20): un
    // monstruo marcado golden en el server se dibuja como los Golden 78..83.
    const CatalogMonster* catalogMonster = (entity_type != 390) ? gContentCatalog.GetMonster(bVar7) : nullptr;
    const bool catalogGolden = catalogMonster && catalogMonster->Golden;
    if (bVar7 == 38 || bVar7 == 43 || bVar7 == 52 || bVar7 == 59 ||
        bVar7 == 67 || (bVar7 >= 78 && bVar7 <= 83) || catalogGolden) {
        const float bodyBright = (bVar7 == 59) ? 0.5f : 1.0f;
        if (bVar7 == 43 || (bVar7 >= 78 && bVar7 <= 83) || catalogGolden) {
            RenderPartObjectBodyColor(model, (int)puVar13, entity_type,
                         *(float *)(puVar13 + 0x5a), 0x48, bodyBright, 0xffffffff);
        }
        const int secondFlags = (bVar7 == 67) ? 0x144 : 0x44;
        RenderPartObjectBodyColor(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), secondFlags, bodyBright, 0xffffffff);
    }
    else if (bVar7 == 0x45) {
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
            CreateSprite(0x47e, &local_54, 0.6f, &local_60, (int)puVar13, 0.0f, 0);
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
        CreateSprite(0x47e, &local_54, 0.8f, &local_60, (int)puVar13, 0.0f, 0);
    }
    else if ((bVar7 == 0x47) || (bVar7 == 0x4a)) {
        // Glow bar widget for special skill (0x1ed = Meteor / 0x1ef = another)
        if (param_1[0x61] == 0) {
            int iType = (bVar7 == 0x47) ? 0x1ed : 0x1ef;
            void *puVar8 = ClothNew();
            // IDA L493: sub_408130(v33, c, 19, 10.0, 0, 5, 15, 30.0, 300.0, tex, tex, 0x1100)
            // (los enteros del decompile: 1106247680 = 0x41F00000 = 30.0;
            // 1133903872 = 0x43960000 = 300.0).
            GridSpring_Create(puVar8, PtrAsFloatBits(param_1), 0x13, 10.0f, 0.0f,
                         5, 0xf, 30.0f, 300.0f, iType, iType, 0x1100);
            param_1[0x61] = (int)puVar8;
            *(char *)(param_1 + 0x60) = 1;
        }
        int *piVar16 = (int *)param_1[0x61];
        if (piVar16) {
            int iVar9 = (int)(size_t)Widget_CheckState(piVar16, 0x3ba3d70a, 5);
            if (iVar9 == 0) DeleteCloth((int)param_1, (int)puVar13, 0);
            else {
                // vtable+0xC = sub_408FF0 (ver nota en el case 0x186).
                FUN_00408ff0((void *)piVar16);
            }
        }
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
            CreateSprite(0x47e, &local_54, 0.8f, &local_60, (int)puVar13, 0.0f, 0);
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
            CreateSprite(0x47e, &local_54, 0.8f, &local_60, (int)puVar13, 0.0f, 0);
        }
        // Drakan (73): IDA emite DOS pases distintos, no un 0x344 combinado.
        // El segundo es RenderPartObjectBodyColorAlt (sub_504AC0, flags 592).
        RenderPartObjectBodyColor(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x44, 1.0f, 0xffffffff);
        Entity_SetModelColorAlt(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x250, 1.0f, 0xffffffff);
    }
    else if (bVar7 == 0x4b) {
        BMD_TransformPosition(model, (float *)(puVar13[0x45] + 0x360),
                     &local_48, &local_54, '\x01');
        Particle_Spawn(0x4ab, &local_54, (float *)(puVar13 + 7),
                     &local_60, 0, 0.3f, 0);
        // Giant Drakan (75): a diferencia de Drakan (73), IDA hace UN solo
        // pase con flags (0x100 | 0x44) = 0x144 y sin sub_504AC0.
        RenderPartObjectBodyColor(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x144, 1.0f, 0xffffffff);
    }
    else if (bVar7 == 0x4d) {
        // Phoenix of Darkness (MonsterID 77): IDA 00456770 case 'M'.
        // Its body pass pulses and then prepares three pose variants before
        // caching the attack-effect bones and ticking the cloth widget.
        const float targetLight = (sinf(fmodf(DAT_05826e08, 10000.0f) * 0.001f) + 1.0f) * 0.5f;
        const float bodyBright = targetLight * 0.7f + 0.3f;
        RenderPartObjectBodyColor(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x44, bodyBright, 0xffffffff);

        // IDA temporarily changes the render pose components, prepares slots
        // 2 and 3, restores X, increments ModelID for the Select pose, then
        // restores it after the cloth/effect pass.
        *(int *)((BYTE *)puVar13 + 100) = 0;
        *(float *)((BYTE *)puVar13 + 104) = (2.0f - targetLight) * 0.3f;
        Entity_PrepareRender((unsigned char *)puVar13, 1, 2, 0);
        Entity_PrepareRender((unsigned char *)puVar13, 1, 3, 0);

        const DWORD actionBones = *(DWORD*)((BYTE*)puVar13 + 276);
        if (actionBones) {
            memcpy(g_AttackEffectMatrix_04D,     (const void*)(actionBones + 1152), sizeof(g_AttackEffectMatrix_04D));
        }
        *(int *)((BYTE *)puVar13 + 100) = -1;
        ++*(short *)((BYTE *)puVar13 + 2);
        Entity_PrepareRender((unsigned char *)puVar13, 1, (int)param_3, 0);
        if (actionBones) {
            memcpy(g_AttackEffectMatrix_04D_Alt, (const void*)(actionBones + 1104), sizeof(g_AttackEffectMatrix_04D_Alt));
            memcpy(g_AttackEffectMatrix_04D_Aux, (const void*)(actionBones + 672),  sizeof(g_AttackEffectMatrix_04D_Aux));
        }

        if (param_1[0x61] == 0) {
            void *puVar8 = ClothNew();
            // IDA L598: sub_408130(v27, o, 10, -10.0, 0, 5, 12, 15.0, 240.0, 1275, 1275, 0x1100)
            // El ancho era 60.0; 1097859072 = 0x41700000 = 15.0.
            GridSpring_Create(puVar8, PtrAsFloatBits(puVar13), 10, -10.0f, 0.0f,
                         5, 0xc, 15.0f, 240.0f, 0x4fb, 0x4fb, 0x1100);
            VerletNode_AddToSystem(puVar8, 0.0f, 0.0f, 40.0f, 30.0f, 10);
            param_1[0x61] = (int)puVar8;
            *(char *)(param_1 + 0x60) = 1;
        }
        int *piVar16 = (int *)param_1[0x61];
        if (piVar16) {
            int iVar9 = (int)(size_t)Widget_CheckState(piVar16, 0x3ba3d70a, 5);
            if (iVar9 == 0) DeleteCloth((int)param_1, (int)puVar13, 0);
            else {
                // IDA invokes vtable slot 3 as `this->Render(0)`.  The port
                // was invoking the function pointer as cdecl with literal 0,
                // so ECX no longer contained the cloth object.  00408FF0
                // consumes the object in ECX and ignores its second argument.
                FUN_00408ff0((void *)piVar16);
            }
        }
        --*(short *)((BYTE *)puVar13 + 2);
    }

    // Taikan (53) y Soldier (54). IDA 00456770 L616-627: un pase de cuerpo
    // EXTRA, despues del switch por MonsterID, con flags 72 (0x48) y — a
    // diferencia del resto — una textura FIJA (1231) en vez de -1.
    // Faltaba entero en el port.
    if (bVar7 == 53 || bVar7 == 54) {
        RenderPartObjectBodyColor(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x48, 1.0f, 1231);
    }

    // ── 5. Dual-wield / shield-glow weapon cases ─────────────────────────────
    if (bVar7 == 0x2a) {
        // Dual-axe: right weapon at Z=-40, left at Y=-40 angle=45
        short *psVar1 = (short *)(param_1 + 0xa8);
        *psVar1 = 0xea;
        *(char *)(param_1 + 0xa9) = 9;
        param_1[0xac] = 0x3e4ccccd; // 0.2f alpha
        RenderLinkObject(0.0f, 0.0f, -40.0f, (int)param_1, (int)psVar1,
                     0xea, '\0', 0, '\0', '\x01', 0);
        *psVar1 = 0xeb;
        *(char *)(param_1 + 0xa9) = 0x3d;
        RenderLinkObject(0.0f, -40.0f, 45.0f, (int)param_1, (int)psVar1,
                     0xeb, '\0', 0, '\0', '\x01', 0);
    }
    else if ((bVar7 > 0x83) && (bVar7 < 0x87)) {
        // Special class shield / weapon glow
        short *psVar1 = (short *)(param_1 + 0xa8);
        *(char *)(param_1 + 0xa9) = 1;
        param_1[0xac] = 0x3e4ccccd;
        if (bVar7 == 0x84) *psVar1 = 0x23a;
        if (bVar7 == 0x85) *psVar1 = 0x1a3;
        if (bVar7 == 0x86) { *psVar1 = 0x222; *(int *)(puVar13 + 3) = 0x3f666666; }
        RenderLinkObject(0.0f, 0.0f, 0.0f, (int)param_1, (int)psVar1,
                     (int)*psVar1, '\0', 0, '\x01', '\x01', 0);
    }

    // ── 5-bis. Sombra del jugador (RenderPartObject con el modelo 391) ───────
    // IDA RenderCharacter L750-772.  Faltaba entera: los jugadores se dibujaban
    // sin sombra.  El gate salteando la sombra cuando se va montado en Uniria
    // (818) o Dinorant (819) fuera de zona segura es del binario, no una
    // simplificacion nuestra.
    {
        char *o = (char *)puVar13;
        if (*(float *)(o + 360) >= 0.5f && gMapManager.GetCurrentMap() != 10 &&
            *(short *)(o + 2) == 390)
        {
            const unsigned short helper = *(unsigned short *)((char *)param_1 + 696);
            if (helper < 818 || helper > 819 || *((char *)param_1 + 846) != 0) {
                // Blood Castle (11..16): si esta muerto sobre el puente, la
                // sombra se pega al terreno en vez de quedar flotando.
                if (gMapManager.GetCurrentMap() >= 11 && gMapManager.GetCurrentMap() <= 16 &&
                    *(BYTE *)(o + 405) != 0 && *((BYTE *)param_1 + 765) != 0)
                {
                    float th = RequestTerrainHeight(*(float *)(o + 16), *(float *)(o + 20));
                    if (th < *(float *)(o + 24)) *(float *)(o + 24) = th;
                }
                const float shadowAlpha = *(float *)(o + 360);
                *(BYTE *)(o + 140) = 1;              // EnableShadow
                RenderPartObject((int)param_1, 391, 0,
                             (float *)((char *)param_1 + 800), shadowAlpha,
                             0, 0, '\0', 0, '\x01', 0, 2);
                *(BYTE *)(o + 140) = 0;
            }
        }
    }

    // ── 6. Scale / color from zone param + entity sub-state ──────────────────
    float fVar32 = (float)(int)param_3 * _DAT_005524f8;

    // IDA RenderCharacter: siempre arma c+800/804/808 desde
    // RequestTerrainLight(o.x, o.y, Light) + o.ColorOffset[232..240].
    {
        float terrainLight[3] = { 0.0f, 0.0f, 0.0f };
        float wx = *(float*)((char*)puVar13 + 16);
        float wy = *(float*)((char*)puVar13 + 20);
        RequestTerrainLight(wx, wy, terrainLight);
        local_60 = terrainLight[0];
        local_5c = terrainLight[1];
        local_58 = terrainLight[2];
    }

    // Luz base del cuerpo, desde el terreno + el ColorOffset del objeto.
    // IDA RenderCharacter L775-779.
    //
    // El binario escribe la luz del cuerpo en DOS momentos distintos:
    //
    //   L775-779   c+800 = Light[] + ColorOffset      <- luz base (aca)
    //   L786       if (c == Hero) -> (1,1,1) x bebida
    //   L1020-1057 RenderPartObject ...................  el CUERPO usa esa luz
    //   L2310-2317 PK: si >= 6 -> (1.0, 0.1, 0.1)     <- segunda escritura
    //   L2342      RenderLinkObject(c + 672) .........  las ALAS usan el rojo
    //
    // O sea el rojo del PK se escribe DESPUES de dibujar el cuerpo, asi que en
    // el original solo alcanza a lo que se dibuja despues: las alas.  Por eso
    // aca va solo la luz base; la segunda escritura (PK) vive mas abajo, justo
    // antes del bloque de armas y alas.
    *(float *)(param_1 + 200)  = local_60 + *(float *)(puVar13 + 0x3a);
    *(float *)(param_1 + 0xc9) = local_5c + *(float *)(puVar13 + 0x3b);
    *(float *)(param_1 + 0xca) = local_58 + *(float *)(puVar13 + 0x3c);

    // IDA RenderCharacter: ModelID 325 (Phoenix of Darkness) overrides the
    // final terrain-derived BodyLight before the body and attachment passes.
    if (sVar2 == 325) {
        *(float *)(param_1 + 200) = 0.6f;
        *(float *)(param_1 + 0xc9) = 0.3f;
        *(float *)(param_1 + 0xca) = 0.3f;
    }

    // ── BodyLight del heroe + tinte de las bebidas (IDA RenderCharacter
    //    L786-L1009, `if (c == Hero)`) ──────────────────────────────────────
    // El heroe NO usa la luz del terreno que se acaba de calcular: la pisa con
    // (1,1,1) y despues le aplica el tinte segun los dos bits de bebida activa
    // de `CharacterAttribute + 40`, que escribe el handler del 0x29
    // (ReceiveHelperItem / PMSG_ITEM_SPECIAL_TIME_SEND) y limpia el timer.
    //   bit 0 -> Ale             (0.9, 0.5, 0.5) = rojizo
    //   bit 1 -> Remedy of Love  multiplica (0.5, 0.9, 0.5)
    // Todo lo que IDA tiene entre el gate y estas tres escrituras es el ruido
    // de hash-table que descifra CharacterMachine para leer el byte (omitido
    // por ser anti-tamper).
    //
    // Va ANTES del render del cuerpo y ANTES de la segunda escritura de luz
    // (la del PK, mas abajo), igual que en IDA: L786 el heroe, L2310 el PK.
    if ((void *)param_1 == DAT_07abf5d8) {
        float L0 = 1.0f, L1 = 1.0f, L2 = 1.0f;
        const unsigned char drink = CharacterAttribute
            ? *((unsigned char *)(uintptr_t)CharacterAttribute + 40) : 0;
        if (drink & 1) { L0 = 0.9f;   L1 = 0.5f;   L2 = 0.5f;   }
        if (drink & 2) { L0 *= 0.5f;  L1 *= 0.9f;  L2 *= 0.5f;  }
        *(float *)(param_1 + 200)  = L0;
        *(float *)(param_1 + 0xc9) = L1;
        *(float *)(param_1 + 0xca) = L2;
    }

    // ── 7. Entity type 0x186 weapon-slot arm render ──────────────────────────
    // (Local_74 = `Bind` de IDA RenderCharacter: arma a la espalda.)
    // +0x34E es SafeZone (NO dead_flag — el dead real es +0x2FD). Sólo se
    // renombró la variable; la lógica queda tal cual el decompile.
    {
        bool bSafeZone = *(char *)((int)param_1 + 0x34e) != '\0';
        BYTE bAnim = *(BYTE *)((int)puVar13 + 0x105);
        local_74 = (void *)((bSafeZone || (bAnim >= 0x5d && bAnim <= 0x7c)) ? 0 : 1);
        if (gMapManager.IsSwimmable(gMapManager.GetCurrentMap()) && (bAnim == 0x15 || bAnim == 0x1d))   // IDA: World == 7
            local_74 = (void *)1;
        if (gMapManager.GetCurrentMap() > 10 && gMapManager.GetCurrentMap() < 0x11)
            local_74 = (void *)0;
    }

    // ── Re-aplicar el stash de equipo si está reseteado ─────────────────────
    if (SceneFlag == 5 && param_1_ == DAT_07abf5d8) {
        HeroEquipWatchdog((int)(uintptr_t)param_1_);


        // +0x34E es SafeZone (no dead_flag): vale 1 con el héroe vivo en el pueblo.
        // No forzarlo a 0: mataría la música de pueblo, el bind del arma a la
        // espalda y el gate de "no atacar en zona segura".
    }


    // ── Tinte de PK / de zona (IDA RenderCharacter L2310-2317) ──────────────
    //
    // SEGUNDA escritura de la luz del cuerpo.  Va aca a proposito: el cuerpo y
    // sus partes ya se dibujaron mas arriba con la luz base, asi que esto solo
    // afecta a lo que se renderiza DESPUES -- el bloque de armas y alas que
    // sigue (RenderLinkObject sobre c + 672).  De ahi que un PK se vea con el
    // cuerpo normal y las ALAS rojas, y no rojo entero.
    //
    // El `goto` del decompile (que saltaba de la rama de zona al cuerpo del
    // else) se reescribe con un flag: las dos ramas terminan poniendo 0.1 en
    // verde y azul, solo cambia el rojo.
    {
        const BYTE pkLevel = *(BYTE *)((int)param_1 + 0x2ea);
        bool bTint = false;
        if (pkLevel < 6) {
            // IDA L1137-1139: la rama `< 6` REESCRIBE la luz base.  No es un
            // duplicado de la escritura de mas arriba: es el reset que separa
            // lo ya dibujado de lo que viene.  En el binario borra aca el
            // tinte de las bebidas que dejo el bloque del heroe, de modo que
            // el CUERPO (L1020, antes) lo tiene y las ALAS (L1239, despues) no.
            // Sacarlo hace que el Ale tina tambien las alas.
            *(float *)(param_1 + 200)  = local_60 + *(float *)(puVar13 + 0x3a);
            *(float *)(param_1 + 0xc9) = local_5c + *(float *)(puVar13 + 0x3b);
            *(float *)(param_1 + 0xca) = local_58 + *(float *)(puVar13 + 0x3c);

            // Override de escala por zona (World - 9 en [1..7])
            const int iSub = (int)gMapManager.GetCurrentMap() - 9;
            const BYTE bv2 = *(BYTE *)((int)param_1 + 0x2eb);
            const bool bInRange =
                (0x55 < bv2 && bv2 < 0x5a) || (0x5b < bv2 && bv2 < 0x60) ||
                (0x72 < bv2 && bv2 < 0x77) || (0x78 < bv2 && bv2 < 0x7d) ||
                (0x7e < bv2 && bv2 < 0x83) || (0x84 < bv2 && bv2 < 0x89);
            if ((0 < iSub) && (iSub < 8) && bInRange && ((iSub / 3) != 0)) {
                *(float *)(param_1 + 200) = (float)(iSub / 3) * _DAT_00552504;
                bTint = true;
            }
        } else {
            *(float *)(param_1 + 200) = 1.0f;   // PKLVL_KILLER: rojo
            bTint = true;
        }
        if (bTint) {
            *(float *)(param_1 + 0xc9) = 0.1f;
            *(float *)(param_1 + 0xca) = 0.1f;
        }
    }

    bool bSkipWeaponLoop = false;
    if (sVar2 == 0x186) {
        // Equipped back items are rendered once by RenderCharacterBackItem below.
        // IDA 0x4582BC..0x45830A: Bind = SafeZone || greeting (93..124) ||
        // (World == 7 && swimming (21/29)), then cleared in Blood Castle.
        // Icarus (World == 10) does NOT set Bind. The duplicate block here used
        // World 10..16 + !SafeZone, drawing the elf's bow on bone 47 as well
        // as the normal hand attachment while flying in Icarus.

        // -- Arma del evento de Blood Castle sobre la espalda (EtcPart) -------
        // IDA LABEL_308: `if (World >= 11 && World <= 16 && c->EtcPart)`, con
        //     EtcPart 1 -> 570 (Staff)   2 -> 419 (Sword)   3 -> 546 (Bow)
        // y LinkBone 47.  `c->EtcPart` es el byte +0x2E8 (= param_1[0xba] con
        // param_1 como int*), que escribe el handler del 0x9B con el
        // EventItemLevel que manda el server.
        //
        // No va dentro de `if (Bind)`: en IDA vive en la rama contraria (`!Back ||
        // Type == -1`) y ademas Bind se fuerza a 0 en Blood Castle, asi que
        // siempre se alcanza.
        if ((gMapManager.GetCurrentMap() >= 11) && (gMapManager.GetCurrentMap() <= 16) &&
            (*(char *)(param_1 + 0xba) != 0)) {
            *(BYTE *)(param_1 + 0xa9) = 0x2f;   // LinkBone = 47
            BYTE bAnim = *(BYTE *)((int)puVar13 + 0x105);
            param_1[0xac] = ((bAnim == 0x1e) || (bAnim == 0x1f)) ? 0x3f800000 : 0x3e800000;
            char cType = *(char *)(param_1 + 0xba);
            int iSecType = (cType == 1) ? 0x23a :   // 570 Staff
                           (cType == 2) ? 0x1a3 :   // 419 Sword
                           (cType == 3) ? 0x222 : 0; // 546 Bow
            if (iSecType != 0) {
                RenderLinkObject(0.0f, 0.0f, 15.0f, (int)param_1, (int)(param_1 + 0xa8),
                             iSecType, 0, 0, 1, 1, 0);
            }
        }

        // Alas (c + 0x2A0).  DESVIACION (DLL WeaponView.cpp): se pueden ocultar.
        // DESVIACION (0.97.20): un ala con modelo del catálogo se dibuja después
        // del cuerpo, como en el 5.2 (RenderParts y después el ala): sus mallas
        // aditivas no escriben profundidad y el cuerpo la taparía siempre.
        if (*(short *)(param_1 + 0xa8) != -1 && !gUserSettings.GetAntilag(ANTILAG_WINGS) &&
            gContentCatalog.EntityDrawModel(param_1, *(short *)(param_1 + 0xa8)) >= MODEL_MAX_VANILLA) {
            bDeferWing = true;
        } else if (*(short *)(param_1 + 0xa8) != -1 && !gUserSettings.GetAntilag(ANTILAG_WINGS)) {
            *(BYTE *)(param_1 + 0xa9) = 0x2f;
            BYTE bAnim = *(BYTE *)((int)puVar13 + 0x105);
            param_1[0xac] = ((bAnim == 0x1e) || (bAnim == 0x1f)) ? 0x3f800000 : 0x3e800000;
            RenderLinkObject(0.0f, 0.0f, 15.0f, (int)param_1, (int)(param_1 + 0xa8),
                         (int)*(short *)(param_1 + 0xa8),
                         *(char *)((int)param_1 + 0x2a2),
                         *(BYTE *)((int)param_1 + 0x2a3), '\0', '\x01', 0);
        }

        // Wings / shield bone (slot 0xae == 0x331)
        if (*(short *)(param_1 + 0xae) == 0x331) {
            *(BYTE *)(param_1 + 0xaf) = 0x22;
            param_1[0xb2] = 0x3f000000; // 0.5f
            RenderLinkObject(20.0f, 0.0f, 0.0f, (int)param_1, (int)(param_1 + 0xae),
                         0x331,
                         *(char *)((int)param_1 + 0x2ba),
                         (UINT)(size_t)local_74, '\0', '\x01', 0);
            // Trail spawn at transformed bone position
            float fOff[3] = { 20.0f, 0.0f, 15.0f };
            float fWorldPos[3];
            BMD_TransformPosition((void *)(DAT_05828d58 + (int)*(short *)((int)puVar13 + 2) * 0xbc),
                         (float *)((UINT)*(BYTE *)(param_1 + 0xaf) * 0x30 + puVar13[0x45]),
                         fOff, fWorldPos, '\x01');
            float fColor2[3] = { fVar32 * _DAT_00552504, 0.0f, 0.0f };
            CreateSprite(0x47e, fWorldPos, 1.5f, fColor2, (int)puVar13, 0, 0);
        }

        // ── Helper render (IDA 1267-1288) ──────────────────────────────────
        // Pet helper (Type 817) attached at bone 34 of player.
        Render_PlayerHelper((int)param_1, (int)puVar13);

        // ── Back-render decision (port WeaponView.cpp:49) ───────────────────
        // Si bBindBack=1, renderiza armas en la espalda (LinkBone
        // 47) y se SALTA Render_PlayerWeaponLoop. Caso típico: safe-zone.
        int bBindBack = RenderCharacterBackItem((int)param_1, (int)puVar13);

        bSkipWeaponLoop = bBindBack != 0;
    }

    // IDA LABEL_330 is reached by every entity. Only the back-item/wing/helper
    // block above is player-specific; ordinary monster weapons continue through
    // the shared two-slot renderer.
    if (!bSkipWeaponLoop)
        Render_PlayerWeaponLoop((int)param_1, (int)puVar13);

    // ── Restaurar la luz base antes de las partes del cuerpo ────────────────
    //
    // DESVIACION FORZADA por el orden de este port.  En el binario las partes
    // del cuerpo se dibujan ANTES que las alas (L1020 vs L1239/L2342), asi que
    // el tinte de PK -- que se escribe entre medio, en L2310 -- alcanza solo a
    // las alas.  Aca el orden esta invertido: alas arriba, cuerpo abajo, con
    // lo cual una sola escritura no puede dejar las alas rojas y el cuerpo
    // normal: lo que tinta las alas tinta tambien el cuerpo.
    //
    // Se vuelve a poner la luz base justo antes del loop de partes.  El efecto
    // final es el del binario (alas rojas, cuerpo con su luz normal) sin tener
    // que reordenar los dos bloques de render, que estan muy anidados.
    *(float *)(param_1 + 200)  = local_60 + *(float *)(puVar13 + 0x3a);
    *(float *)(param_1 + 0xc9) = local_5c + *(float *)(puVar13 + 0x3b);
    *(float *)(param_1 + 0xca) = local_58 + *(float *)(puVar13 + 0x3c);
    if ((void *)param_1 == DAT_07abf5d8) {
        // El heroe no usa la luz del terreno (ver el bloque de mas arriba).
        float L0 = 1.0f, L1 = 1.0f, L2 = 1.0f;
        const unsigned char drink = CharacterAttribute
            ? *((unsigned char *)(uintptr_t)CharacterAttribute + 40) : 0;
        if (drink & 1) { L0 = 0.9f;   L1 = 0.5f;   L2 = 0.5f;   }
        if (drink & 2) { L0 *= 0.5f;  L1 *= 0.9f;  L2 *= 0.5f;  }
        *(float *)(param_1 + 200)  = L0;
        *(float *)(param_1 + 0xc9) = L1;
        *(float *)(param_1 + 0xca) = L2;
    }

    // ── 7b. Body-part render loop (Ghidra RenderCharacter lines 1622-1700) ──────
    // Player.bmd is skeleton-only (numMesh=0); body geometry lives in separate
    // BMD models (HelmClass##/ArmorClass##/PantClass##/GloveClass##/BootClass##)
    // referenced by entity equipment slots and rendered here via RenderPartObject
    // (Entity_DrawAt) using the parent entity's animated bones.
    //
    // Equipment slot layout (6 entries, stride 0x18 bytes):
    //   [0] +0x1e0  BodyPart[0] (extra / unused)
    //   [1] +0x1f8  BodyPart[1]  — Helm    (HelmClass## 0x390-0x393)
    //   [2] +0x210  BodyPart[2]  — Armor   (ArmorClass## 0x397-0x39a)
    //   [3] +0x228  BodyPart[3]  — Pant    (PantClass## 0x39e-0x3a1)
    //   [4] +0x240  BodyPart[4]  — Glove   (GloveClass## 0x3a5-0x3a8)
    //   [5] +0x258  BodyPart[5]  — Boot    (BootClass## 0x3ac-0x3af)
    // Per slot layout:
    //   +0x00 short  model_idx (-1 if empty)
    //   +0x02 byte   level
    //   +0x03 byte   option
    // Gate: *(param_1 + 0x34f) == 0 (not hide-equipment state).
    // ── 7a-bis. NPC con modelo de jugador + SubType propio (IDA L1012-1042) ──
    // Caso del Golden Archer / esqueleto de Lorencia. CreateMonster case 236
    // (0x45CCF0 L803) crea la entidad como
    //     OpenNpc(390); c = CreateCharacter(Key, 390, ...);
    //     o->SubType (o+4) = 207;  o->Kind (o+132) = 4;  c+446 = 8;
    // o sea Type = 390 = MODEL_PLAYER. Con Type 390 el render cae en el loop de
    // body-parts de abajo, pero este NPC no tiene NINGUNA parte equipada
    // (+0x1e0…+0x258 todos -1) → no se dibujaría nada.
    //
    // El original tiene una rama previa: si Type == 390 y SubType está en
    // [206, 208] (MODEL_SKELETON1..3 — Data\Skill\Bones_Warrior / Bone_A /
    // Bone_C, cargados por OpenSkills 0x50B710), la entidad se dibuja como UN
    // solo modelo con RenderPartObject(SubType) y se SALTEA el loop de partes.
    // Hay dos variantes: NPC (Kind==4) en Lorencia (World==0) pasa
    // `8 * *(WORD*)(c+446)` como flags de render; el resto pasa 0.
    bool bSubTypeNpcRendered = false;
    if (sVar2 == 0x186) {
        int subType = *(int *)((char *)puVar13 + 4);
        if (subType >= 206 && subType <= 208) {
            float alphaST = *(float *)((char *)puVar13 + 0x168);
            int   flags   = 0;
            if (*(BYTE *)((char *)puVar13 + 0x84) == 4 && (int)gMapManager.GetCurrentMap() == 0) {
                flags = 8 * (int)*(unsigned short *)((char *)param_1 + 446);
            }
            RenderPartObject((int)param_1, subType, 0,
                         (float *)(param_1 + 200), alphaST,
                         flags, 0, '\0', 0, '\x01', 0, 2);
            bSubTypeNpcRendered = true;
        }
    }

    if (!bSubTypeNpcRendered && *(char *)((int)param_1 + 0x34f) == '\0') {
        int *piVar16 = param_1 + 0x7d;   // int*-index: byte offset +0x1f4
        for (int nSlot = 6; nSlot > 0; nSlot--) {
            short sSlot = (short)piVar16[-5];   // slot model_idx at -20 bytes
            if (sSlot != (short)-1) {
                int iVar9 = (int)sSlot;
                // Class byte @ entity+0x1BC (IDA sub_456770: *(BYTE*)(c + 444))
                // Original Ghidra port had `(int)param_1 + 0x6f` — 0x6f*4 = 0x1bc,
                // so the decomp was int-indexed (param_1[0x6f]) not byte-offset.
                // Byte-offset 0x6f is an unrelated field; reading it here made all
                // login demo characters render as DW (class=0). Fixed to 0x1bc.
                // 0.97.20: modelo propio de la pieza (catálogo).
                iVar9 = gContentCatalog.EntityDrawModel(param_1, iVar9);
                BYTE bClassByte = *(BYTE *)((int)param_1 + 0x1bc);
                *(BYTE *)(DAT_05828d58 + iVar9 * 0xbc + 0x98) =
                    (BYTE)(((bClassByte & 7) << 1) | (bClassByte >> 3));
                // ── DIAG: log per-call to RenderPartObject (Entity_DrawAt) for char-select
                // dump scale, model addr, animCount@26, and DAT_005524f8 (cull thresh).
                if (SceneFlag == 4) {
                    int csSlot = (int)(((uintptr_t)param_1_ - (uintptr_t)DAT_07abf5d0) / 0x394);
                    if (csSlot >= 0 && csSlot < 5) {
                        static DWORD s_lastDA[5] = {0,0,0,0,0};
                        DWORD now = GetTickCount();
                        if (now - s_lastDA[csSlot] > 1000) {
                            // log only the FIRST body-part draw of this slot per second
                            s_lastDA[csSlot] = now;
                            void* bm = (void*)(DAT_05828d58 + iVar9 * 0xbc);
                            float fScale = *(float *)(puVar13 + 0x5a);
                        }
                    }
                }
                {
                    BYTE rawLvl = *(BYTE *)((char *)piVar16 - 0x12);
                    BYTE rawOpt = *(BYTE *)((char *)piVar16 - 0x11);
                    // c+482 es el nivel RAW (0-15) del equipo; el shift <<3 lo
                    // encoda para que RenderPartObjectEffect lo decodifique con
                    // (val>>3)&0xF. FIEL a IDA (8 * *(BYTE*)(v76-18)).
                    UINT shiftedLvl = (UINT)rawLvl << 3;
                    RenderPartObject((int)param_1,
                                 iVar9,
                                 (int)(piVar16 - 5),
                                 (float *)(param_1 + 200),
                                 *(float *)(puVar13 + 0x5a),
                                 shiftedLvl,
                                 rawOpt,
                                 '\0', 0, '\x01',
                                 (int)(size_t)param_3, 2);
                }
                // IDA RenderCharacter (0x456770 L1070-1104): la tela de los
                // pants Grand Soul (modelo 706 = MODEL_PANTS + 18).  Cada pieza
                // guarda su tela en part+0x14 (el DWORD al que apunta piVar16).
                // El port lo salteaba con la etiqueta "guild-mark related",
                // que era incorrecta: la capita de atras se movia pegada al
                // cuerpo en vez de simularse.
                {
                    const bool isGrandSoulPants = (iVar9 == 706);
                    if (*piVar16 == 0 && isGrandSoulPants) {
                        // IDA: operator_new(0x54) + sub_407FE0, SIN el prefijo de
                        // count del `eh vector` (a diferencia de la capa del MG):
                        // DeleteCloth la libera con el dtor en modo 1.
                        void *cloth = Widget_CtorBase(operator_new(0x54));
                        // sub_408130(obj, c, 2, 10.0, 10.0, 5, 15, 45.0, 85.0,
                        //            1276, 1276, 0x1400)
                        GridSpring_Create(cloth, PtrAsFloatBits(param_1), 2, 10.0f, 10.0f,
                                     5, 15, 45.0f, 85.0f, 1276, 1276, 0x1400);
                        // sub_409250(obj, 0, -15.0, -20.0, 30.0, 2)
                        VerletNode_AddToSystem(cloth, 0.0f, -15.0f, -20.0f, 30.0f, 2);
                        *piVar16 = (int)cloth;
                    }
                    if (*piVar16) {
                        if (!isGrandSoulPants) {
                            DeleteCloth((int)param_1, 0, 0);          // DeleteCloth(c, 0, 0)
                        } else if (Widget_CheckState((int *)*piVar16, 0x3ba3d70a, 5)) {
                            if (*(float *)(puVar13 + 0x5a) > 0.01f)
                                FUN_00408ff0((void *)*piVar16);        // vtable[3]
                        } else {
                            DeleteCloth((int)param_1, (int)puVar13, 0);  // DeleteCloth(c, o, 0)
                        }
                    }
                }
            }
            piVar16 += 6;   // advance to next slot (+0x18 bytes)
        }
        // IDA: RenderCharacter llama CreateGuildMark/RenderGuildMarkOnShield después de
        // renderizar las seis piezas, sólo para modelos de jugador visibles.
        const short guildMarkIndex = *(short*)((BYTE*)param_1 + 474);
        if (guildMarkIndex >= 0 &&
            *(short*)((BYTE*)puVar13 + 2) == 390 &&
            *(float*)((BYTE*)puVar13 + 360) != 0.0f) {
            CreateGuildMark(guildMarkIndex, true);
            RenderGuildMarkOnShield((int)(uintptr_t)puVar13,
                          *(short*)((BYTE*)param_1 + 528));
        }
    }

    if (bDeferWing) {
        *(BYTE *)(param_1 + 0xa9) = 0x2f;
        BYTE bAnim = *(BYTE *)((int)puVar13 + 0x105);
        param_1[0xac] = ((bAnim == 0x1e) || (bAnim == 0x1f)) ? 0x3f800000 : 0x3e800000;
        RenderLinkObject(0.0f, 0.0f, 15.0f, (int)param_1, (int)(param_1 + 0xa8),
                     (int)*(short *)(param_1 + 0xa8),
                     *(char *)((int)param_1 + 0x2a2),
                     *(BYTE *)((int)param_1 + 0x2a3), '\0', '\x01', 0);
    }

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
    // pvVar23 / local_78 is resolved from the model object in the hash-table
    // section above (skipped).  In the port we use el model lookup directo
    // (DAT_05828d58 + entity_type * 0xbc) que ya teníamos calculado al inicio.
    pvVar23 = model;
    local_78 = model;

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
                CreateSprite((int)uType, &local_54, fSc, &local_60, (int)puVar13, 0, (int)!bAlt);
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
        float fSin = (float)(double)fsin((double)(DAT_05826e08 * _DAT_005528e0));
        local_48 = 3.5f; local_44 = -12.0f; local_40 = 10.0f;
        float fCol = fSin * _DAT_0055294c + _DAT_00552948;
        local_60 = fCol; local_5c = fCol; local_58 = fCol;
        BMD_TransformPosition(pvVar23, (float *)(puVar13[0x45] + 0x3c0),
                     &local_48, &local_54, '\x01');
        CreateSprite(0x4a7, &local_54, 0.3f, &local_60, (int)puVar13,
                     (float)DAT_05826e08 * _DAT_00552944, 0);
        CreateSprite(0x4a7, &local_54, 0.3f, &local_60, (int)puVar13,
                     -(float)DAT_05826e08 * _DAT_00552944, 0);
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
        bool v230 = CheckFullSet((int)param_1);
        local_48 = 0.0f; local_44 = 0.0f; local_40 = 0.0f;

        // ── Capa del Magic Gladiator (IDA RenderCharacter L667-743) ───────────
        // La capa NO es una malla del modelo: es un objeto de **tela**
        // (cloth/verlet) construido con `sub_408130`, texturizado con
        // Robe01.jpg/Robe02.jpg = slots 490/491 (los carga `OpenPlayerTextures`
        // 0x507610, en Model/Model_Monsters.cpp).
        //
        //   Targetd = 0;
        //   if ((c[444] & 7) == 3 && o->Type == 390) Targetd = 1;   // clase 3 = MG
        //   if (c[747] == 55)                        Targetd = 1;   // evento
        //   if (EnableSoccer && guild == hero-guild/rival-guild) Targetd = 1;
        //   if (SoccerObserver && guild == either observer team) Targetd = 1;
        //   if (Targetd && !c->Cloth) { crear + anclar; }
        //   luego: si sub_408900(0.005,5) → tick por vtable+0xC, si no DeleteCloth
        //
        // Slot: `o + 384` = flag creada, `o + 388` = puntero al sistema de tela
        // (mismo par que usan los otros dos cloth ya portados en este archivo).
        {
            BYTE* cb = (BYTE*)param_1_;
            bool bCape = false;
            if (((cb[444] & 7) == 3) && (*(short*)((char*)puVar13 + 2) == 390))
                bCape = true;                    // Magic Gladiator
            if (cb[747] == 55)
                bCape = true;

            // IDA RenderCharacter: Soccer gives the standard cloth cape to
            // both participating guilds.  The observer branch is also kept
            // because it is part of the same original predicate, even though
            // the current MuEmu build does not emit F3/23 observer state.
            const short guildIndex = *(short*)(cb + 474);
            if (guildIndex != -1) {
                const char* guildName = DAT_07e919bc + 80 * guildIndex;
                if (EnableSoccer && DAT_07abf5d8) {
                    const short heroGuild = *(short*)((BYTE*)DAT_07abf5d8 + 474);
                    if (heroGuild != -1 &&
                        (strcmp(DAT_07e919bc + 80 * heroGuild, guildName) == 0 ||
                         strcmp(GuildWarName, guildName) == 0)) {
                        bCape = true;
                    }
                }
                if (DAT_05826d33 &&
                    (strcmp(SoccerTeamName[0], guildName) == 0 ||
                     strcmp(SoccerTeamName[1], guildName) == 0)) {
                    bCape = true;
                }
            }

            if (bCape) {
                if (param_1[0x61] == 0) {
                    void* cloth = ClothNew();
                    if (cb[747] == 55) {
                        GridSpring_Create(cloth, PtrAsFloatBits(puVar13), 19, 10.0f, 0.0f,
                                     10, 10, 55.0f, 140.0f, 492, 492, 4097);
                        VerletNode_AddToSystem(cloth, -10.0f, -10.0f, -10.0f, 35.0f, 17);
                        VerletNode_AddToSystem(cloth,  10.0f, -10.0f, -10.0f, 35.0f, 17);
                        VerletNode_AddToSystem(cloth,   0.0f, -10.0f, -20.0f, 50.0f, 19);
                    } else {
                        GridSpring_Create(cloth, PtrAsFloatBits(puVar13), 19, 10.0f, 0.0f,
                                     10, 10, 75.0f, 120.0f, 490, 491, 1029);
                        VerletNode_AddToSystem(cloth, -10.0f, -10.0f, -10.0f, 25.0f, 17);
                        VerletNode_AddToSystem(cloth,  10.0f, -10.0f, -10.0f, 25.0f, 17);
                        VerletNode_AddToSystem(cloth, -10.0f, -10.0f,  20.0f, 27.0f, 17);
                        VerletNode_AddToSystem(cloth,  10.0f, -10.0f,  20.0f, 27.0f, 17);
                    }
                    param_1[0x61] = (int)cloth;
                    *(char *)(param_1 + 0x60) = 1;
                }
                int *pCloth = (int *)param_1[0x61];
                if (pCloth) {
                    int alive = (int)(size_t)Widget_CheckState(pCloth, 0x3ba3d70a, 5);
                    if (alive == 0) {
                        DeleteCloth((int)param_1, (int)puVar13, 0);
                    } else {
                        // IDA: `(*(void (__thiscall **)(int,_DWORD))(*(_DWORD *)v49 + 12))(v49, 0);`
                        // La vtable `off_552520` no está portada (Widget_CtorBase
                        // tiene el vtable-set saltado), así que se llama directo al destino real.
                        // Vtable leída del binario original (`Cliente armado/main.exe`, MD5 eb95ac…):
                        //     off_552520 = { 0x0045AAA0, 0x00408780,
                        //                    0x004089B0, 0x00408FF0 }
                        // o sea **+0xC = sub_408FF0**.
                        FUN_00408ff0((void *)pCloth);
                    }
                }
            }
        }
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
            CreateSprite(0x498, &local_54, 0.6f, &local_60, 0, 0, 0);
            float fS = (float)(double)fsin((double)(DAT_05826e08 * _DAT_00552500));
            CreateSprite(0x4cf, &local_54, (float)(fS * _DAT_005528b4),
                         &local_60, 0, 0, 0);
        }

        // ── 2. PLAYER_SPELL anim (88 = 'X') — orbit sparkles (3 sprites) ──
        // IDA línea 2133-2152. Bone idx en c+628 (= LinkBone Weapon[0]),
        // tres CreateSprite(1191) con rotación basada en WorldTime.
        if (*(char *)((int)puVar13 + 0x105) == 'X') {
            BYTE bone = *(BYTE *)(param_1 + 0x9d);  // = (param_1+0x9d*4-1)? no: int-idx 0x9d * 4 = byte 0x274 = c+628 LinkBone[0]
            // Wait int-arithmetic: param_1 (int*) + 0x9d ints == byte+0x274 == c+628 ✓
            BMD_TransformPosition(pvVar23,
                (float *)(puVar13[0x45] + (UINT)bone * 0x30),
                &local_48, &local_54, '\x01');
            float Scalep = *(float *)((int)puVar13 + 0x108) * 0.1f;  // anim frame * 0.1
            local_60 = 0.1f; local_5c = 0.1f; local_58 = 1.0f;
            CreateSprite(0x4a7, &local_54, Scalep * 0.30000001f,
                         &local_60, (int)puVar13, 0, 0);
            float WorldTime = (float)DAT_05826e08;
            CreateSprite(0x4a7, &local_54, Scalep, (float*)(puVar13 + 0x3a),
                         (int)puVar13, -WorldTime * 0.1f, 0);
            CreateSprite(0x4a7, &local_54, Scalep * 2.5f, (float*)(puVar13 + 0x3a),
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
                CreateSprite(0x4cf, &local_54, 1.5f, &local_60, (int)puVar13, 0, 0);
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbVar20 - 6) * 0x30),
                    &local_48, &local_54, '\x01');
                CreateSprite(0x4cf, &local_54, 1.5f, &local_60, (int)puVar13, 0, 0);
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbVar20 - 7) * 0x30),
                    &local_48, &local_54, '\x01');
                CreateSprite(0x4cf, &local_54, 1.5f, &local_60, (int)puVar13, 0, 0);
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
                CreateSprite(0x47e, &local_54, 1.3f, &local_60, (int)puVar13, 0, 0);  // CreateSprite 1150
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbV193 - 6) * 0x30),
                    &local_48, &local_54, '\x01');
                CreateSprite(0x47e, &local_54, 1.3f, &local_60, (int)puVar13, 0, 0);
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbV193 - 7) * 0x30),
                    &local_48, &local_54, '\x01');
                CreateSprite(0x47e, &local_54, 1.3f, &local_60, (int)puVar13, 0, 0);
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
