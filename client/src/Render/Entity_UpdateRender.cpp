#include "stdafx.h"

// FUN_00408130 guarda su 2do arg con `*(float*)(thiz+4) = entity` y después lo
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
    return FUN_00407fe0(raw + 1);     // el ctor recibe el objeto, no el bloque
}

static inline float PtrAsFloatBits(const void *p) {
    float f; int v = (int)(uintptr_t)p; memcpy(&f, &v, 4); return f;
}

extern "C" DWORD g_ItemAttribute_Backup;   // src/globals.cpp
#pragma warning(disable: 4244 4305 4701 4702 4700)
// Entity_UpdateRender.cpp  —  FUN_00456770 @ 0x00456770  (2195 lines in Ghidra)
//
// Per-frame visual update for a single entity.  Called from Entity_RenderAll_3D
// for every visible entity.  Drives:
//   - Skill-channel widget objects  (channeling beams / barriers)
//   - Entity_PrepareRender          (bone + AABB compute)
//   - Per-skill / per-anim-state particle effects on entity bones
//   - Weapon-slot rendering         (FUN_00455430 = RenderLinkObject)
//   - Per-entity-type NPC / monster special effects (large outer switch)
//
// param_1  — player / local entity  (int*, stride 0x394, base DAT_07abf5d0[0])
// param_2  — entity being rendered  (undefined4* / puVar13 in Ghidra)
// param_3  — zone-id or context param (treated as int for zone-scale calc)
//
// Anti-tamper: ~30 HashTable_GetIndex / HashTable_Insert / XOR-encode blocks are
// interspersed throughout; per CLAUDE.md those are pure obfuscation and are omitted.

// All FUN_* prototypes and DAT_* globals come from stdafx.h → functions.h / globals.h


// ── FUN_00456770  Entity_UpdateRender ──────────────────────────────────────
extern "C" {
    void DbgLogPublic(const char*);
    // From Render_PlayerEquipment.cpp
    void Render_PlayerHelper(int c, int o);
    void Render_PlayerWeaponLoop(int c, int o);
    // 2026-05-04: back-weapon render decision. Returns 1 if weapon was rendered
    // on back (LinkBone 47). When 1, Render_PlayerWeaponLoop should be skipped.
    int  RenderCharacterBackItem(int c, int o);
    // 2026-05-04: per-frame watchdog que restaura wings/weapons/pendant del
    // hero si fueron reseteados a -1 después de F3/03.
    void HeroEquipWatchdog(int c);
}

// IDA: FUN_004552C0 — dibuja la textura 34 ya compuesta por CreateGuildMark
// sobre el hueso 26 del modelo de jugador. El segundo parámetro es el escudo
// equipado; sólo modifica el desplazamiento vertical del emblema.
void __cdecl FUN_004552c0(int entity, int shield_id)
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
    FUN_004f9f70(bone26, localMatrix, &DAT_06989c9c);

    glTranslatef(*(float*)(object + 16), *(float*)(object + 20), *(float*)(object + 24));
    GL_DrawBillboard(5.0f, 7.0f, &DAT_06989c9c);
    glPopMatrix();
    GL_DisableCullFace();
}

void* __cdecl FUN_00456770(void *param_1_, void *param_2_, void *param_3)
{
    int *param_1  = (int *)param_1_;
    int *puVar13  = (int *)param_2_;   // Ghidra alias for param_2

    // ── 1. Setup ─────────────────────────────────────────────────────────────
    short sVar2    = *(short *)((int)puVar13 + 2);        // entity_type
    int  entity_type = (int)sVar2;
    void *model = (void *)(DAT_05828d58 + entity_type * 0xbc);

    // ── DIAG: log slot/type/anim-frame guard for char-select frames ─────────
    // Rate-limit PER SLOT (not globally) so all 5 chars emit once a second.
    if (DAT_005615c0 == 4) {
        int slot = (int)(((uintptr_t)param_1_ - (uintptr_t)DAT_07abf5d0) / 0x394);
        if (slot >= 0 && slot < 5) {
            static DWORD s_lastUR[5] = {0,0,0,0,0};
            DWORD now = GetTickCount();
            if (now - s_lastUR[slot] > 1000) {
                s_lastUR[slot] = now;
                char b[200];
                _snprintf_s(b, sizeof(b), _TRUNCATE,
                    "UR slot=%d type=%d cls=%d animCount@26=%d model=%p early_out=%d",
                    slot, entity_type, ((BYTE*)param_1_)[0x1bc],
                    *(short*)((char*)model + 0x26),
                    model,
                    (*(short *)((char *)model + 0x26) == 0) ? 1 : 0);
                DbgLogPublic(b);
            }
        }
    }

    // ── DIAG: log hero in-game render entry/exit ────────────────────────────
    if (DAT_005615c0 == 5 && param_1_ == DAT_07abf5d8) {
        static DWORD s_lastUR5 = 0;
        DWORD now = GetTickCount();
        if (now - s_lastUR5 > 1000) {
            s_lastUR5 = now;
            char b[256];
            short animCount = *(short*)((char*)model + 0x26);
            short numBones  = *(short*)((char*)model + 0x22);
            BYTE  curAct    = *(BYTE*)(((char*)param_1_) + 0x105);
            float frame     = *(float*)(((char*)param_1_) + 0x108);
            int   actBase   = *(int*)((char*)model + 0x30);
            float actSpd    = (actBase && actBase >= 0x100000) ? *(float*)(actBase + curAct*0x10 + 4) : 0.0f;
            short actFrames = (actBase && actBase >= 0x100000) ? *(short*)(actBase + curAct*0x10 + 8) : 0;
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "UR5 hero type=%d cls=%d animCount@26=%d numBones@22=%d model=%p early_out=%d "
                "act=0x%02x frame=%.3f actSpd=%.3f actFrames=%d",
                entity_type, ((BYTE*)param_1_)[0x1bc],
                animCount, numBones, model,
                (animCount == 0) ? 1 : 0,
                (int)curAct, frame, actSpd, (int)actFrames);
            DbgLogPublic(b);
        }
    }

    // Early-out: no animation data in this model slot
    if (*(short *)((char *)model + 0x26) == 0)
        return (void *)entity_type;

    // ── BUG-FIX 2026-04-27: declarar como arrays contiguos para que `&local_X`
    // pasado a funciones que leen/escriben 3 floats consecutivos (FUN_004795c0,
    // BMD_TransformPosition, etc.) no caiga en stack slots aleatorios. Mismo patrón ya
    // arreglado en Sprite/Math_3D/Scene_CharSelect.
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
    // (2026-08-22: se llamaba "magic_channel_flag"; es el tipo que escribe
    //  CreateMonster.  La logica ya comparaba contra tipos, solo mentia el nombre.)
    char cVar6 = *(char *)((int)param_1 + 0x2eb);  // tipo de monstruo
    switch (cVar6) {
    case 'Y': case '_': case 'p': case 'v': case '|':
    case (char)-0x7e: case (char)-0x78:
    {
        // Skill channel active — beam/barrier widget path
        unsigned int uVar11 = (unsigned int)(size_t)FUN_004faa70((int)puVar13, '\x01', (int)param_3);
        if (param_1[0x61] == 0) {
            void *puVar8 = operator_new(100);
            FUN_00541ec1((char *)puVar8 + 4, 0x60, 1, (void *)FUN_004093a0);
            FUN_004093e0((char *)puVar8 + 4, (int)param_1, (short *)2, 0x12, 0x400, -1);
            FUN_00409250((char *)puVar8 + 4, 0.0f,   0.0f, 0.0f, 50.0f, 18);
            FUN_00409250((char *)puVar8 + 4, 0.0f, -20.0f, 0.0f, 30.0f, 18);
            param_1[0x61] = (int)puVar8;
            *(char *)(param_1 + 0x60) = 1;
        }
        int *piVar16 = (int *)param_1[0x61];
        if (piVar16) {
            int iVar9 = (int)(size_t)FUN_00408900(piVar16, 0x3ba3d70a, 5);
            if (iVar9 == 0)
                FUN_00449840((int)param_1, (int)puVar13, 0);
            else
                (*(void (__cdecl **)(int))(*piVar16 + 0xc))(0);
        }
        if ((BYTE)uVar11 != 0)
            FUN_004fae00(puVar13, 1, (int)param_3, '\0');
        break;
    }
    default:
        // Normal state — prepare entity render (bone / AABB)
        cVar6 = ((cVar6 == 'C') || (cVar6 == 'J') || (cVar6 == 'K')) ? '\x01' : '\0';
        FUN_004fc030((unsigned char *)puVar13, 1, (int)param_3, cVar6);
        break;
    }

    // ── 3. LOD / sparkle (distance check) ───────────────────────────────────
    // entity_type != 0x186, not channel state, DAT_0055a7ac != 10,
    // distance (puVar13[0x5a]) >= _DAT_005528b8
    {
        float fDist = *(float *)(puVar13 + 0x5a);
        if ((entity_type != 0x186) && (DAT_0055a7ac != 10) && (fDist >= _DAT_005528b8)) {
            // LOD distance update + distant sparkle
            // (FUN_00505a10 / FUN_004f8bb0 — see Entity_DrawAt.cpp)
            if ((DAT_0055a7ac > 10) && (DAT_0055a7ac < 17) && (fDist < *(float *)(puVar13 + 6)))
                *(float *)(puVar13 + 6) = fDist;
            GL_SetBlendAdditive();
            // Billboard sparkle at bone 0x13 when entering/leaving range
        }
    }

    // ── 4. Skill-state / anim-state particle effects ─────────────────────────
    BYTE bVar7 = *(BYTE *)((int)param_1 + 0x2eb);   // tipo de monstruo

    if ((bVar7 == 0x26) || (bVar7 == 0x34) || (bVar7 == 0x43)) {
        FUN_00504960(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x44, 1.0f, 0xffffffff);
    }
    else if (bVar7 == 0x2b) {
        FUN_00504960(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x44,
                     *(float *)((char *)puVar13 + 8), 0xffffffff);
    }
    else if (bVar7 == 0x3b) {
        FUN_00504960(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x48, 0.5f, 0xffffffff);
    }
    else if ((bVar7 >= 0x4e) && (bVar7 < 0x54)) {
        FUN_00504960(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x44, 1.0f, 0xffffffff);
    }
    else if (bVar7 == 0x45) {
        // 9-bone glitter + 3 random-bone sparks
        BYTE *boneIdxTable = (BYTE *)&DAT_0055984c;
        for (int i = 0; i < 9; i++) {
            BMD_TransformPosition(model,
                (float *)(puVar13[0x45] + (int)(UINT)boneIdxTable[i] * 0x30),
                &local_48, &local_54, '\x01');
            FUN_004795c0(0x47e, &local_54, 0.6f, &local_60, (int)puVar13, 0.0f, 0);
        }
        int nBones = *(short *)((char *)model + 0x22);
        for (int i = 0; i < 3; i++) {
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
        FUN_004795c0(0x47e, &local_54, 0.8f, &local_60, (int)puVar13, 0.0f, 0);
    }
    else if ((bVar7 == 0x47) || (bVar7 == 0x4a)) {
        // Glow bar widget for special skill (0x1ed = Meteor / 0x1ef = another)
        if (param_1[0x61] == 0) {
            int iType = (bVar7 == 0x47) ? 0x1ed : 0x1ef;
            void *puVar8 = ClothNew();
            FUN_00408130(puVar8, PtrAsFloatBits(param_1), 0x13, 10.0f, 0.0f,
                         5, 0xf, 240.0f, 500.0f, iType, iType, 0x1100);
            param_1[0x61] = (int)puVar8;
            *(char *)(param_1 + 0x60) = 1;
        }
        int *piVar16 = (int *)param_1[0x61];
        if (piVar16) {
            int iVar9 = (int)(size_t)FUN_00408900(piVar16, 0x3ba3d70a, 5);
            if (iVar9 == 0) FUN_00449840((int)param_1, (int)puVar13, 0);
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
            FUN_004795c0(0x47e, &local_54, 0.8f, &local_60, (int)puVar13, 0.0f, 0);
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
            FUN_004795c0(0x47e, &local_54, 0.8f, &local_60, (int)puVar13, 0.0f, 0);
        }
        FUN_00504960(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x344, 1.0f, 0xffffffff);
    }
    else if (bVar7 == 0x4b) {
        BMD_TransformPosition(model, (float *)(puVar13[0x45] + 0x360),
                     &local_48, &local_54, '\x01');
        Particle_Spawn(0x4ab, &local_54, (float *)(puVar13 + 7),
                     &local_60, 0, 0.3f, 0);
        FUN_00504960(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x44, 1.0f, 0xffffffff);
        FUN_00504ac0(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x250, 1.0f, 0xffffffff);
    }
    else if (bVar7 == 0x4d) {
        // Scale-flicker + morph widget (Teleport skill)
        FUN_00504960(model, (int)puVar13, entity_type,
                     *(float *)(puVar13 + 0x5a), 0x44, 1.0f, 0xffffffff);
        // 00456770 copies these three matrices immediately after preparing the
        // 0x4D model.  AttackEffect later uses them for the skill-17/skill-3
        // branches of the same action; keeping a hand-bone substitute here
        // changes both the source point and the visual direction.
        const DWORD actionBones = *(DWORD*)((BYTE*)puVar13 + 276);
        if (actionBones) {
            memcpy(g_AttackEffectMatrix_04D,     (const void*)(actionBones + 1152), sizeof(g_AttackEffectMatrix_04D));
            memcpy(g_AttackEffectMatrix_04D_Alt, (const void*)(actionBones + 1104), sizeof(g_AttackEffectMatrix_04D_Alt));
            memcpy(g_AttackEffectMatrix_04D_Aux, (const void*)(actionBones + 672),  sizeof(g_AttackEffectMatrix_04D_Aux));
        }
        // Copy three bone matrix blocks to global scratch buffers
        // (DAT_07abf444 / 3e4 / 414 — morph target cache)
        if (param_1[0x61] == 0) {
            void *puVar8 = ClothNew();
            FUN_00408130(puVar8, PtrAsFloatBits(puVar13), 10, -10.0f, 0.0f,
                         5, 0xc, 60.0f, 240.0f, 0x4fb, 0x4fb, 0x1100);
            FUN_00409250(puVar8, 0.0f, 0.0f, 40.0f, 30.0f, 10);
            param_1[0x61] = (int)puVar8;
            *(char *)(param_1 + 0x60) = 1;
        }
        int *piVar16 = (int *)param_1[0x61];
        if (piVar16) {
            int iVar9 = (int)(size_t)FUN_00408900(piVar16, 0x3ba3d70a, 5);
            if (iVar9 == 0) FUN_00449840((int)param_1, (int)puVar13, 0);
            else (*(void (__cdecl **)(int))(*piVar16 + 0xc))(0);
        }
    }

    // ── 5. Dual-wield / shield-glow weapon cases ─────────────────────────────
    if (bVar7 == 0x2a) {
        // Dual-axe: right weapon at Z=-40, left at Y=-40 angle=45
        short *psVar1 = (short *)(param_1 + 0xa8);
        *psVar1 = 0xea;
        *(char *)(param_1 + 0xa9) = 9;
        param_1[0xac] = 0x3e4ccccd; // 0.2f alpha
        FUN_00455430(0.0f, 0.0f, -40.0f, (int)param_1, (int)psVar1,
                     0xea, '\0', 0, '\0', '\x01', 0);
        *psVar1 = 0xeb;
        *(char *)(param_1 + 0xa9) = 0x3d;
        FUN_00455430(0.0f, -40.0f, 45.0f, (int)param_1, (int)psVar1,
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
        FUN_00455430(0.0f, 0.0f, 0.0f, (int)param_1, (int)psVar1,
                     (int)*psVar1, '\0', 0, '\x01', '\x01', 0);
    }

    // ── 6. Scale / color from zone param + entity sub-state ──────────────────
    float fVar32 = (float)(int)param_3 * _DAT_005524f8;

    // IDA RenderCharacter: siempre arma c+800/804/808 desde
    // RequestTerrainLight(o.x, o.y, Light) + o.ColorOffset[232..240].
    {
        float terrainLight[3] = { 0.0f, 0.0f, 0.0f };
        float wx = *(float*)((char*)puVar13 + 16);
        float wy = *(float*)((char*)puVar13 + 20);
        FUN_004f7960(wx, wy, terrainLight);
        local_60 = terrainLight[0];
        local_5c = terrainLight[1];
        local_58 = terrainLight[2];
    }

    if (*(BYTE *)((int)param_1 + 0x2ea) < 6) {
        // Blend bone world position into entity light slots
        *(float *)(param_1 + 200) = local_60 + *(float *)(puVar13 + 0x3a);
        *(float *)(param_1 + 0xc9) = local_5c + *(float *)(puVar13 + 0x3b);
        *(float *)(param_1 + 0xca) = local_58 + *(float *)(puVar13 + 0x3c);

        // Sub-state zone-scale override (DAT_0055a7ac - 9 in [1..7])
        int iSub = DAT_0055a7ac - 9;
        BYTE bv2 = *(BYTE *)((int)param_1 + 0x2eb);
        bool bInRange = (0x55 < bv2 && bv2 < 0x5a) || (0x5b < bv2 && bv2 < 0x60) ||
                        (0x72 < bv2 && bv2 < 0x77) || (0x78 < bv2 && bv2 < 0x7d) ||
                        (0x7e < bv2 && bv2 < 0x83) || (0x84 < bv2 && bv2 < 0x89);
        if ((0 < iSub) && (iSub < 8) && bInRange && ((iSub / 3) != 0)) {
            *(float *)(param_1 + 200) = (float)(iSub / 3) * _DAT_00552504;
            goto LAB_004582b2;
        }
    }
    else {
        *(float *)(param_1 + 200) = 1.0f;
    LAB_004582b2:
        *(float *)(param_1 + 0xc9) = 0.1f;
        *(float *)(param_1 + 0xca) = 0.1f;
    }

    // ── 7. Entity type 0x186 weapon-slot arm render ──────────────────────────
    // (Local_74 = `Bind` de IDA RenderCharacter: arma a la espalda.)
    // +0x34E es SafeZone (NO dead_flag — el dead real es +0x2FD). Sólo se
    // renombró la variable; la lógica queda tal cual el decompile.
    {
        bool bSafeZone = *(char *)((int)param_1 + 0x34e) != '\0';
        BYTE bAnim = *(BYTE *)((int)puVar13 + 0x105);
        local_74 = (void *)((bSafeZone || (bAnim >= 0x5d && bAnim <= 0x7c)) ? 0 : 1);
        if (DAT_0055a7ac == 7 && (bAnim == 0x15 || bAnim == 0x1d))
            local_74 = (void *)1;
        if (DAT_0055a7ac > 10 && DAT_0055a7ac < 0x11)
            local_74 = (void *)0;
    }

    // ── 2026-05-04: re-apply equipment stash si está reseteado ──────────────
    if (DAT_005615c0 == 5 && param_1_ == DAT_07abf5d8) {
        HeroEquipWatchdog((int)(uintptr_t)param_1_);

        // 2026-07-27 WATCHDOG global de ItemAttribute: DAT_07d78068 se corrompe
        // a ~1 en runtime (confirmado: "SHOPINS ... attrBase=00000001" dejaba la
        // tienda vacía). Restaurarlo una vez por frame beneficia a TODOS los
        // consumidores (tienda, inventario, tooltips, stats) en vez de parchear
        // cada uno por separado.
        {
            unsigned int p = (unsigned int)(uintptr_t)DAT_07d78068;
            if ((p < 0x100000u || p >= 0x80000000u)
                && g_ItemAttribute_Backup >= 0x100000u
                && g_ItemAttribute_Backup < 0x80000000u)
            {
                DAT_07d78068 = (int)g_ItemAttribute_Backup;
            }
        }

        // 2026-07-27 WATCHDOG (tinte rojo PK): +0x2EA es el PKLevel; el render
        // pinta el cuerpo de rojo cuando es >= 6 (línea ~333). Entity_Spawn lo
        // inicializa en 3 para los mobs, pero el héroe se crea por otro path y
        // quedaba sin inicializar (0xFF → rojo permanente). El valor real lo
        // setea el F3/03; acá saneamos cualquier valor fuera del rango 0..6.
        {
            BYTE* hb = (BYTE*)param_1_;
            if (hb[0x2ea] > 6) {
                hb[0x2ea] = 0;
            }
        }

        // 2026-08-10 — WATCHDOG REMOVIDO. Ya no hace falta: no había ningún
        // "escritor misterioso" del flag. +0x34E es **SafeZone**, no dead_flag,
        // así que valía 1 legítimamente con el héroe vivo parado en el pueblo;
        // el que estaba mal era el lector de abajo (bDead), que ahora usa el
        // dead real (+0x2FD, IDA ReceiveDie L18). El watchdog además forzaba
        // SafeZone=0 en cada frame, matando la música de pueblo, el bind del
        // arma a la espalda y el gate de "no atacar en zona segura".
    }

    // ── DIAG: hero entry into case 0x186 ─────────────────────────────────────
    if (DAT_005615c0 == 5 && param_1_ == DAT_07abf5d8) {
        static DWORD s_lastH186 = 0;
        DWORD now = GetTickCount();
        if (now - s_lastH186 > 2000) {
            s_lastH186 = now;
            BYTE* be = (BYTE*)param_1_;
            int slotIdx = (int)(((uintptr_t)param_1_ - (uintptr_t)DAT_07abf5d0) / 0x394);
            char b[200];
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "UR5 case186 slot=%d sVar2=0x%X wing@2a0=%d eq=%d w0=%d w1=%d hero=%p be0=%d",
                slotIdx, (int)sVar2, (int)*(short*)(be + 0x2a0),
                (sVar2 == 0x186) ? 1 : 0,
                (int)*(short*)(be + 0x270), (int)*(short*)(be + 0x288),
                DAT_07abf5d8, (int)be[0]);
            DbgLogPublic(b);
            // 2026-07-27 DIAG alas rojas "PK": volcar el body-Light (c+0x320) y
            // los bytes de estado candidatos al tinte rojo (hit-flash/shock/PK).
            // Si el R domina sobre G/B, el personaje entero (incl. alas) se ve
            // rojo. Capturamos qué campo lo dispara la próxima vez que pase.
            float* bl = (float*)(be + 0x320);
            char b2[220];
            _snprintf_s(b2, sizeof(b2), _TRUNCATE,
                "UR5 HEROLIGHT R=%.3f G=%.3f B=%.3f | 2ea=%d 2eb=%d 2ec=%d 2ed=%d "
                "2f4=%d 2f5=%d 301=%d 303=%d 34e=%d 105=%d",
                bl[0], bl[1], bl[2],
                (int)be[0x2ea], (int)be[0x2eb], (int)be[0x2ec], (int)be[0x2ed],
                (int)be[0x2f4], (int)be[0x2f5], (int)be[0x301], (int)be[0x303],
                (int)be[0x34e], (int)be[0x105]);
            DbgLogPublic(b2);
        }
    }
    // ── DIAG: ANY entity with wing@2a0 != -1 (find where wings actually live)
    if (DAT_005615c0 == 5) {
        BYTE* be = (BYTE*)param_1_;
        if (*(short*)(be + 0x2a0) != -1 && *(short*)(be + 2) == 0x186) {
            static DWORD s_lastWE = 0;
            DWORD now = GetTickCount();
            if (now - s_lastWE > 2000) {
                s_lastWE = now;
                int slotIdx = (int)(((uintptr_t)param_1_ - (uintptr_t)DAT_07abf5d0) / 0x394);
                char b[200];
                _snprintf_s(b, sizeof(b), _TRUNCATE,
                    "UR5 wingFOUND slot=%d wing@2a0=%d w0=%d w1=%d hero=%p ptr=%p",
                    slotIdx, (int)*(short*)(be + 0x2a0),
                    (int)*(short*)(be + 0x270), (int)*(short*)(be + 0x288),
                    DAT_07abf5d8, param_1_);
                DbgLogPublic(b);
            }
        }
    }

    // ── 7a. NPC / monster body render (IDA L347-401) ─────────────────────────
    // 2026-05-08: missing port — sin esto NPCs/monsters renderean SOLO efectos
    // especiales (entity-type switch al final) pero nunca su BODY geometry.
    // Resultado visual: monsters invisibles excepto por sparkles/particles.
    //
    // Gate: entity_type != 390 (player) && kind != 8 (KIND_TRAP).
    // Tipos de monstruo excluidos (+0x2EB):
    //   25 (cloth-cape variants) / 22 (frost) / 42 (special-weapon dual-axe)
    //   -14 = 242 (reserved) / 59 (charge state) / 63 (reserved)
    // Excluded World == 10 (heaven map — different render path)
    // Excluded alpha < 0.3 (entity fading out / invisible)
    // Excluded entity_type 330/331 (terrain decorations rendered elsewhere)
    if (sVar2 != 0x186 && *(BYTE *)((char *)puVar13 + 0x84) != 8) {
        BYTE v11 = bVar7;   // *(c + 747)
        int  World = (int)DAT_0055a7ac;
        float alpha = *(float *)((char *)puVar13 + 0x168);   // o + 360

        if (v11 != 25 && v11 != 22 && v11 != 42 && v11 != (BYTE)-14 &&
            v11 != 59 && v11 != 63 &&
            World != 10 && alpha >= 0.3f)
        {
            // Atlans/Tarkan worlds (11..16): clamp Z to terrain height when
            // entity is dead+action-start (Blood Castle special case).
            if (World >= 11 && World <= 16) {
                if (*(BYTE *)((char *)puVar13 + 0x195) != 0 &&     // o+405 m_bActionStart
                    *(BYTE *)((char *)param_1 + 0x2FD) != 0) {     // c+765 Dead>0 byte
                    float wx = *(float *)((char *)puVar13 + 0x10);
                    float wy = *(float *)((char *)puVar13 + 0x14);
                    float th = FUN_004f7500(wx, wy);
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
                FUN_00505a10((int)param_1,
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

    bool bSkipWeaponLoop = false;
    if (sVar2 == 0x186) {
        // ── BACK weapon render (IDA 1166-1239: gated por Bind=1) ────────────
        // Renderiza arma en la espalda cuando es bow/crossbow + Bind activo.
        // Bind = 1 sólo en in-game con World en 10-16, fuera de greeting anims.
        // Para char-select (DAT_005615c0 == 4) Bind siempre = 0 → este bloque
        // NO debe disparar y dejar el render de armas al `Render_PlayerWeaponLoop`
        // (líneas finales de este case 0x186) que las pone en la mano.
        //
        // BUGFIX 2026-04-27: antes este bloque corría incondicionalmente y
        // duplicaba el render del crossbow del Elf — render en espalda Y en mano
        // → flicker visible cuando ambos paths competían por DAT_06989c9c.
        //
        // Bind detection (port simplificado IDA líneas 1140-1153):
        //   Bind = (DAT_0055a7ac in 10..16) && (anim < 93 || anim > 124) &&
        //          !c+0x34E && !(c+747 in special-skill ranges)
        bool Bind = false;
        if (DAT_005615c0 == 5) {  // in-game
            int World = (int)DAT_0055a7ac;
            if (World >= 10 && World <= 16) {
                BYTE anim = *(BYTE*)((int)puVar13 + 0x105);
                if ((anim < 93 || anim > 124) && *(char*)((int)param_1 + 0x34E) == 0) {
                    Bind = 1;
                }
            }
        }

        if (Bind) {
            // Determine which weapon slot to render based on equipped items
            int  iVar9 = 0;
            void *param_3b = NULL;
            short sA  = *(short *)(param_1 + 0xa2);
            short sB  = *(short *)(param_1 + 0x9c);
            char  cLv = *(char *)((int)param_1 + 0x272);
            UINT  uOp = *(BYTE *)((int)param_1 + 0x273);

            if (local_74 == (void *)0) {
                // Dead / hidden: only show specific weapon types
                if      (sA == 0x217) { param_3b = (void *)1; iVar9 = 1; }
                else if (sB == 0x21f) { param_3b = (void *)1; }
            } else {
                param_3b = (void *)1;
                if (!((sA >= 0x210 && sA <= 0x216) || sA == 0x221)) {
                    if (!((sB >= 0x218 && sB <= 0x21e) || sB == 0x220 || sB == 0x222)) {
                        if (sA == 0x217) iVar9 = 1;
                        else param_3b = NULL;
                    } else {
                        iVar9 = 0;
                    }
                } else {
                    iVar9 = 1;
                }
            }

            int iVar18 = (int)*(short *)(param_1 + (iVar9 * 3 + 0x4e) * 2);
            cLv = *(char *)((int)param_1 + iVar9 * 0x18 + 0x272);
            uOp = *(BYTE *)((int)param_1 + iVar9 * 0x18 + 0x273);

            // Overridden by zone state
            bool zoneOverride = false;
            if ((*(char *)((int)puVar13 + 0x21) == '\x04') &&
                (DAT_0055a7ac == 0) &&
                (puVar13[1] > 0xcd) && (puVar13[1] < 0xd1)) {
                iVar18 = 0x219;
                cLv = '\b';
                zoneOverride = true;
            }
            if (zoneOverride || (param_3b != NULL && iVar18 != -1)) {
                void *local_68 = (void *)param_1[0xaa];
                *(BYTE *)(param_1 + 0xa9) = 0x2f;
                BYTE bAnim = *(BYTE *)((int)puVar13 + 0x105);
                param_1[0xac] = ((bAnim == 0x1e) || (bAnim == 0x1f)) ? 0x3f800000 : 0x3e800000;
                FUN_00455430(0.0f, 0.0f, 15.0f, (int)param_1, (int)(param_1 + 0xa8),
                             iVar18, cLv, uOp, (char)(size_t)(zoneOverride ? (void*)1 : param_3b), '\x01', 0);
                param_1[0xaa] = (int)local_68;
            }

            // Secondary weapon override (param_1[0xba] = secondary item type, Blood Castle)
            if ((DAT_0055a7ac > 10) && (DAT_0055a7ac < 0x11) &&
                (*(char *)(param_1 + 0xba) != '\0')) {
                *(BYTE *)(param_1 + 0xa9) = 0x2f;
                BYTE bAnim = *(BYTE *)((int)puVar13 + 0x105);
                param_1[0xac] = ((bAnim == 0x1e) || (bAnim == 0x1f)) ? 0x3f800000 : 0x3e800000;
                char cType = *(char *)(param_1 + 0xba);
                int iSecType = (cType == '\x01') ? 0x23a :
                               (cType == '\x02') ? 0x1a3 :
                               (cType == '\x03') ? 0x222 : 0;
                FUN_00455430(0.0f, 0.0f, 15.0f, (int)param_1, (int)(param_1 + 0xa8),
                             iSecType, '\0', 0, '\x01', '\x01', 0);
            }
        }

        // Tertiary weapon slot (param_1[0xa8] != -1)
        if (*(short *)(param_1 + 0xa8) != -1) {
            *(BYTE *)(param_1 + 0xa9) = 0x2f;
            BYTE bAnim = *(BYTE *)((int)puVar13 + 0x105);
            param_1[0xac] = ((bAnim == 0x1e) || (bAnim == 0x1f)) ? 0x3f800000 : 0x3e800000;
            FUN_00455430(0.0f, 0.0f, 15.0f, (int)param_1, (int)(param_1 + 0xa8),
                         (int)*(short *)(param_1 + 0xa8),
                         *(char *)((int)param_1 + 0x2a2),
                         *(BYTE *)((int)param_1 + 0x2a3), '\0', '\x01', 0);
        }

        // Wings / shield bone (slot 0xae == 0x331)
        if (*(short *)(param_1 + 0xae) == 0x331) {
            *(BYTE *)(param_1 + 0xaf) = 0x22;
            param_1[0xb2] = 0x3f000000; // 0.5f
            FUN_00455430(20.0f, 0.0f, 0.0f, (int)param_1, (int)(param_1 + 0xae),
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
            FUN_004795c0(0x47e, fWorldPos, 1.5f, fColor2, (int)puVar13, 0, 0);
        }

        // ── Helper render (IDA 1267-1288) ──────────────────────────────────
        // Pet helper (Type 817) attached at bone 34 of player.
        Render_PlayerHelper((int)param_1, (int)puVar13);

        // ── Back-render decision (port WeaponView.cpp:49) ───────────────────
        // 2026-05-04: si bBindBack=1, renderiza armas en la espalda (LinkBone
        // 47) y se SALTA Render_PlayerWeaponLoop. Caso típico: safe-zone.
        int bBindBack = RenderCharacterBackItem((int)param_1, (int)puVar13);

        bSkipWeaponLoop = bBindBack != 0;
    }

    // IDA LABEL_330 is reached by every entity. Only the back-item/wing/helper
    // block above is player-specific; ordinary monster weapons continue through
    // the shared two-slot renderer.
    if (!bSkipWeaponLoop)
        Render_PlayerWeaponLoop((int)param_1, (int)puVar13);

    // ── 7b. Body-part render loop (Ghidra FUN_00456770 lines 1622-1700) ──────
    // Missing in previous port — this is what actually draws player geometry.
    // Player.bmd is skeleton-only (numMesh=0); body geometry lives in separate
    // BMD models (HelmClass##/ArmorClass##/PantClass##/GloveClass##/BootClass##)
    // referenced by entity equipment slots and rendered here via FUN_00505a10
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
    // 2026-08-08 PORT FALTANTE — "el Golden Archer / esqueleto de Lorencia no se
    // dibuja". CreateMonster case 236 (0x45CCF0 L803) crea la entidad como
    //     OpenNpc(390); c = CreateCharacter(Key, 390, ...);
    //     o->SubType (o+4) = 207;  o->Kind (o+132) = 4;  c+446 = 8;
    // o sea Type = 390 = MODEL_PLAYER. Con Type 390 el render cae en el loop de
    // body-parts de abajo, pero este NPC no tiene NINGUNA parte equipada
    // (+0x1e0…+0x258 todos -1) → no se dibujaba nada.
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
            if (*(BYTE *)((char *)puVar13 + 0x84) == 4 && (int)DAT_0055a7ac == 0) {
                flags = 8 * (int)*(unsigned short *)((char *)param_1 + 446);
            }
            FUN_00505a10((int)param_1, subType, 0,
                         (float *)(param_1 + 200), alphaST,
                         flags, 0, '\0', 0, '\x01', 0, 2);
            bSubTypeNpcRendered = true;
        }
    }

    if (!bSubTypeNpcRendered && *(char *)((int)param_1 + 0x34f) == '\0') {
        int *piVar16 = param_1 + 0x7d;   // int*-index: byte offset +0x1f4
        // ── DIAG: dump body-part slots + positions for char-select entities
        if (DAT_005615c0 == 4) {
            int csSlot = (int)(((uintptr_t)param_1_ - (uintptr_t)DAT_07abf5d0) / 0x394);
            if (csSlot >= 0 && csSlot < 5) {
                static DWORD s_lastBP[5] = {0,0,0,0,0};
                DWORD now = GetTickCount();
                if (now - s_lastBP[csSlot] > 1000) {
                    s_lastBP[csSlot] = now;
                    BYTE* be = (BYTE*)param_1_;
                    float wx = *(float*)(be + 0x10);
                    float wy = *(float*)(be + 0x14);
                    float wz = *(float*)(be + 0x18);
                    float fAng = *(float*)(be + 0x24);
                    float fScl = *(float*)(be + 0x0c);   // anim scale used by Entity_DrawAt
                    BYTE  cls  = be[0x1bc];
                    BYTE  anim = be[0x105];
                    char  hide = *(char*)(be + 0x34f);
                    char b[256];
                    _snprintf_s(b, sizeof(b), _TRUNCATE,
                        "BP slot=%d cls=0x%02x pos=(%.1f,%.1f,%.1f) ang=%.2f scl@0c=%.4f "
                        "anim=%d hide@34f=%d parts=%d/%d/%d/%d/%d/%d wing=%d "
                        "wpn0@270=%d wpn1@288=%d helper@2b8=%d",
                        csSlot, cls, wx, wy, wz, fAng, fScl, anim, hide,
                        *(short*)(be + 0x1e0), *(short*)(be + 0x1f8),
                        *(short*)(be + 0x210), *(short*)(be + 0x228),
                        *(short*)(be + 0x240), *(short*)(be + 0x258),
                        *(short*)(be + 0x2a0),
                        *(short*)(be + 0x270), *(short*)(be + 0x288),
                        *(short*)(be + 0x2b8));
                    DbgLogPublic(b);
                }
            }
        }
        for (int nSlot = 6; nSlot > 0; nSlot--) {
            short sSlot = (short)piVar16[-5];   // slot model_idx at -20 bytes
            if (sSlot != (short)-1) {
                int iVar9 = (int)sSlot;
                // Class byte @ entity+0x1BC (IDA sub_456770: *(BYTE*)(c + 444))
                // Original Ghidra port had `(int)param_1 + 0x6f` — 0x6f*4 = 0x1bc,
                // so the decomp was int-indexed (param_1[0x6f]) not byte-offset.
                // Byte-offset 0x6f is an unrelated field; reading it here made all
                // login demo characters render as DW (class=0). Fixed to 0x1bc.
                BYTE bClassByte = *(BYTE *)((int)param_1 + 0x1bc);
                *(BYTE *)(DAT_05828d58 + iVar9 * 0xbc + 0x98) =
                    (BYTE)(((bClassByte & 7) << 1) | (bClassByte >> 3));
                // ── DIAG: log per-call to FUN_00505a10 (Entity_DrawAt) for char-select
                // dump scale, model addr, animCount@26, and DAT_005524f8 (cull thresh).
                if (DAT_005615c0 == 4) {
                    int csSlot = (int)(((uintptr_t)param_1_ - (uintptr_t)DAT_07abf5d0) / 0x394);
                    if (csSlot >= 0 && csSlot < 5) {
                        static DWORD s_lastDA[5] = {0,0,0,0,0};
                        DWORD now = GetTickCount();
                        if (now - s_lastDA[csSlot] > 1000) {
                            // log only the FIRST body-part draw of this slot per second
                            s_lastDA[csSlot] = now;
                            void* bm = (void*)(DAT_05828d58 + iVar9 * 0xbc);
                            float fScale = *(float *)(puVar13 + 0x5a);
                            char b[200];
                            _snprintf_s(b, sizeof(b), _TRUNCATE,
                                "DA slot=%d part_idx=%d (0x%X) scale=%.4f thresh=%.4f cull=%d "
                                "bm=%p anims@26=%d",
                                csSlot, iVar9, iVar9, fScale, _DAT_005524f8,
                                (_DAT_005524f8 >= fScale) ? 1 : 0,
                                bm, *(short*)((char*)bm + 0x26));
                            DbgLogPublic(b);
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
                    if (DAT_005615c0 == 4) {
                        int csSlot = (int)(((uintptr_t)param_1_ - (uintptr_t)DAT_07abf5d0) / 0x394);
                        if (csSlot >= 0 && csSlot < 5 && iVar9 >= 624 && iVar9 < 1000) {
                            static DWORD s_lastBPL[5] = {0};
                            DWORD now = GetTickCount();
                            if (now - s_lastBPL[csSlot] > 1000) {
                                s_lastBPL[csSlot] = now;
                                char b[160];
                                _snprintf_s(b, sizeof(b), _TRUNCATE,
                                    "BPLVL slot=%d part=%d rawLvl=%d rawOpt=%d shifted=0x%x piVar16=%p",
                                    csSlot, iVar9, (int)rawLvl, (int)rawOpt, shiftedLvl, piVar16);
                                DbgLogPublic(b);
                            }
                        }
                    }
                    FUN_00505a10((int)param_1,
                                 iVar9,
                                 (int)(piVar16 - 5),
                                 (float *)(param_1 + 200),
                                 *(float *)(puVar13 + 0x5a),
                                 shiftedLvl,
                                 rawOpt,
                                 '\0', 0, '\x01',
                                 (int)(size_t)param_3, 2);
                }
                // Optional widget spawn for iVar9 == 0x2c2 (guild-mark related)
                // skipped — anti-tamper obfuscation touches this, original
                // path rarely hits unless 0x2c2 model is equipped.
            }
            piVar16 += 6;   // advance to next slot (+0x18 bytes)
        }
        // IDA: FUN_00456770 llama CreateGuildMark/FUN_004552C0 después de
        // renderizar las seis piezas, sólo para modelos de jugador visibles.
        const short guildMarkIndex = *(short*)((BYTE*)param_1 + 474);
        if (guildMarkIndex >= 0 &&
            *(short*)((BYTE*)puVar13 + 2) == 390 &&
            *(float*)((BYTE*)puVar13 + 360) != 0.0f) {
            CreateGuildMark(guildMarkIndex, true);
            FUN_004552c0((int)(uintptr_t)puVar13,
                          *(short*)((BYTE*)param_1 + 528));
        }
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
    // Antes era `pvVar23 = local_78` (= NULL) → TODOS los BMD_TransformPosition con
    // pvVar23 dereferenciaban NULL y crasheaban en model+0x68.
    pvVar23 = model;
    local_78 = model;

    switch (sVar2) {

    case 0x10e:  // Boat-like entity type 270
    case 300:
        // If weapon anim state == 1 (armed standing) or type == 300: sail effect
        if (((sVar2 == 0x10e) && (*(short *)((int)param_1 + 0x1be) == 1)) ||
            (sVar2 == 300)) {
            puVar13 = (int *)FUN_00456650((int)puVar13, 0x16, 0x17, 1.0f);
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
                FUN_004795c0((int)uType, &local_54, fSc, &local_60, (int)puVar13, 0, (int)!bAlt);
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
        FUN_00456590((int)puVar13, 0x497, 4.0f, 9, 0.0f, 0, 5.0f);
        puVar13 = (int *)FUN_00456590((int)puVar13, 0x4d0, 3.0f, 9, 0.0f, 0, 5.0f);
        return puVar13;

    case 0x130:  // Entity 304 — six-point smoke/fire/magic
        FUN_00456590((int)puVar13, 0x4a7, 0.5f, 0x1e, 0.0f, 0, -5.0f);
        FUN_00456590((int)puVar13, 0x4a7, 0.5f, 0x27, 0.0f, 0, -5.0f);
        FUN_00456590((int)puVar13, 0x497, 4.0f, 0x1e, 0.0f, 0, -5.0f);
        FUN_00456590((int)puVar13, 0x497, 4.0f, 0x27, 0.0f, 0, -5.0f);
        FUN_00456590((int)puVar13, 0x4d0, 2.0f, 0x1e, 0.0f, 0, -5.0f);
        puVar13 = (int *)FUN_00456590((int)puVar13, 0x4d0, 2.0f, 0x27, 0.0f, 0, -5.0f);
        return puVar13;

    case 0x132:  // Entity 306 — claw/spider: beam + 8 effect bones
        FUN_00456650((int)puVar13, 0x2a, 0x2b, 1.0f);
        FUN_00456590((int)puVar13, 0x497, 2.0f, 0x1a, 0.0f, 0, 0.0f);
        FUN_00456590((int)puVar13, 0x497, 2.0f, 0x1f, 0.0f, 0, 0.0f);
        FUN_00456590((int)puVar13, 0x497, 2.0f, 0x24, 0.0f, 0, 0.0f);
        FUN_00456590((int)puVar13, 0x497, 2.0f, 0x29, 0.0f, 0, 0.0f);
        FUN_00456590((int)puVar13, 0x4d0, 1.0f, 0x1a, 0.0f, 0, 0.0f);
        FUN_00456590((int)puVar13, 0x4d0, 1.0f, 0x1f, 0.0f, 0, 0.0f);
        FUN_00456590((int)puVar13, 0x4d0, 1.0f, 0x24, 0.0f, 0, 0.0f);
        puVar13 = (int *)FUN_00456590((int)puVar13, 0x4d0, 1.0f, 0x29, 0.0f, 0, 0.0f);
        return puVar13;

    case 0x133:  // Entity 307 — large fire/smoke at bone 0x3f
        FUN_00456590((int)puVar13, 0x4a7, 1.0f, 0x3f, 0.0f, 0, 20.0f);
        puVar13 = (int *)FUN_00456590((int)puVar13, 0x4d0, 4.0f, 0x3f, 0.0f, 0, 20.0f);
        return puVar13;

    case 0x142:  // Entity 322 — dual beam bones 0x1a/0x1b  scale=2
        puVar13 = (int *)FUN_00456650((int)puVar13, 0x1a, 0x1b, 2.0f);
        return puVar13;

    case 0x15c:  // Entity 348 — soft sparkle at bone 0x20
        puVar13 = (int *)FUN_00456590((int)puVar13, 0x47e, 1.5f, 0x20, 0.0f, 0, 0.0f);
        return puVar13;

    case 0x15d:  // Entity 349 — sin-wave orb cloud + occasional lightning joint
    {
        float fSin = (float)(double)fsin((double)(DAT_05826e08 * _DAT_005528e0));
        local_48 = 3.5f; local_44 = -12.0f; local_40 = 10.0f;
        float fCol = fSin * _DAT_0055294c + _DAT_00552948;
        local_60 = fCol; local_5c = fCol; local_58 = fCol;
        BMD_TransformPosition(pvVar23, (float *)(puVar13[0x45] + 0x3c0),
                     &local_48, &local_54, '\x01');
        FUN_004795c0(0x4a7, &local_54, 0.3f, &local_60, (int)puVar13,
                     (float)DAT_05826e08 * _DAT_00552944, 0);
        FUN_004795c0(0x4a7, &local_54, 0.3f, &local_60, (int)puVar13,
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
        puVar13 = (int *)FUN_00456590((int)puVar13, 0x47e, 2.0f, 6, 0.0f, 0, 0.0f);
        return puVar13;

    case 0x186:  // Entity 390 (Player) — port completo IDA case 390 (líneas 2111-2269)
    {
        // CheckFullSet detecta si las 5 piezas de armadura forman un set
        // matcheado con level >= 9 → setea `EquipmentLevelSet` global y devuelve
        // true si el char tiene 5 piezas (aunque no matcheen, para v230=true).
        bool v230 = FUN_00451b20((int)param_1);
        local_48 = 0.0f; local_44 = 0.0f; local_40 = 0.0f;

        // ── Capa del Magic Gladiator (IDA RenderCharacter L667-743) ───────────
        // 2026-08-11: bloque que faltaba portar. La capa NO es una malla del
        // modelo: es un objeto de **tela** (cloth/verlet) construido con
        // `sub_408130`, texturizado con Robe01.jpg/Robe02.jpg = slots 490/491
        // (`OpenPlayerTextures` 0x507610 los carga y nuestro Model_Monsters.cpp
        // ya lo hacía; sólo faltaba el consumidor).
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
                        FUN_00408130(cloth, PtrAsFloatBits(puVar13), 19, 10.0f, 0.0f,
                                     10, 10, 55.0f, 140.0f, 492, 492, 4097);
                        FUN_00409250(cloth, -10.0f, -10.0f, -10.0f, 35.0f, 17);
                        FUN_00409250(cloth,  10.0f, -10.0f, -10.0f, 35.0f, 17);
                        FUN_00409250(cloth,   0.0f, -10.0f, -20.0f, 50.0f, 19);
                    } else {
                        FUN_00408130(cloth, PtrAsFloatBits(puVar13), 19, 10.0f, 0.0f,
                                     10, 10, 75.0f, 120.0f, 490, 491, 1029);
                        FUN_00409250(cloth, -10.0f, -10.0f, -10.0f, 25.0f, 17);
                        FUN_00409250(cloth,  10.0f, -10.0f, -10.0f, 25.0f, 17);
                        FUN_00409250(cloth, -10.0f, -10.0f,  20.0f, 27.0f, 17);
                        FUN_00409250(cloth,  10.0f, -10.0f,  20.0f, 27.0f, 17);
                    }
                    param_1[0x61] = (int)cloth;
                    *(char *)(param_1 + 0x60) = 1;
                }
                int *pCloth = (int *)param_1[0x61];
                if (pCloth) {
                    int alive = (int)(size_t)FUN_00408900(pCloth, 0x3ba3d70a, 5);
                    if (alive == 0) {
                        FUN_00449840((int)param_1, (int)puVar13, 0);
                    } else {
                        // IDA: `(*(void (__thiscall **)(int,_DWORD))(*(_DWORD *)v49 + 12))(v49, 0);`
                        // La vtable `off_552520` no está portada (FUN_00407fe0
                        // tiene el vtable-set saltado), así que la indirección
                        // saltaba a basura → crash por ejecución (param0=8).
                        // 2026-08-11: leí la vtable del binario original
                        // (`Cliente armado/main.exe`, MD5 eb95ac…):
                        //     off_552520 = { 0x0045AAA0, 0x00408780,
                        //                    0x004089B0, 0x00408FF0 }
                        // o sea **+0xC = sub_408FF0**. Llamada directa al destino
                        // real en vez de la indirección.
                        FUN_00408ff0((void *)pCloth);
                    }
                }
            }
        }
        if (DAT_005615c0 == 2) {
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
            FUN_004795c0(0x498, &local_54, 0.6f, &local_60, 0, 0, 0);
            float fS = (float)(double)fsin((double)(DAT_05826e08 * _DAT_00552500));
            FUN_004795c0(0x4cf, &local_54, (float)(fS * _DAT_005528b4),
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
            FUN_004795c0(0x4a7, &local_54, Scalep * 0.30000001f,
                         &local_60, (int)puVar13, 0, 0);
            float WorldTime = (float)DAT_05826e08;
            FUN_004795c0(0x4a7, &local_54, Scalep, (float*)(puVar13 + 0x3a),
                         (int)puVar13, -WorldTime * 0.1f, 0);
            FUN_004795c0(0x4a7, &local_54, Scalep * 2.5f, (float*)(puVar13 + 0x3a),
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
                FUN_004795c0(0x4cf, &local_54, 1.5f, &local_60, (int)puVar13, 0, 0);
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbVar20 - 6) * 0x30),
                    &local_48, &local_54, '\x01');
                FUN_004795c0(0x4cf, &local_54, 1.5f, &local_60, (int)puVar13, 0, 0);
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbVar20 - 7) * 0x30),
                    &local_48, &local_54, '\x01');
                FUN_004795c0(0x4cf, &local_54, 1.5f, &local_60, (int)puVar13, 0, 0);
            }
        }

        // ── 4. Sin full set (v230==false) → no glow, exit ─────────────────
        if (!v230) {
            return puVar13;
        }

        // ── 5. Full set body glow: PartObjectColor + 6 sparkles (3 bones × 2) ──
        // IDA línea 2191: PartObjectColor(c->Boot.Type, alpha, 0.5, Light, 0)
        // → escribe en Light[3] el RGB del glow basado en el tipo de bota.
        FUN_00503cf0(*(short*)((int)param_1 + 600),
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
                FUN_004795c0(0x47e, &local_54, 1.3f, &local_60, (int)puVar13, 0, 0);  // CreateSprite 1150
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbV193 - 6) * 0x30),
                    &local_48, &local_54, '\x01');
                FUN_004795c0(0x47e, &local_54, 1.3f, &local_60, (int)puVar13, 0, 0);
                BMD_TransformPosition(pvVar23,
                    (float *)(puVar13[0x45] + ((int)*pbV193 - 7) * 0x30),
                    &local_48, &local_54, '\x01');
                FUN_004795c0(0x47e, &local_54, 1.3f, &local_60, (int)puVar13, 0, 0);
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
