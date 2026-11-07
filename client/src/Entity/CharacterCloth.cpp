#include "stdafx.h"
#include "Physics/ClothTypes.h"
#include "Entity/CharacterCloth.h"
#include "Entity/ObjectPose.h"
#include "Render/EquipmentVisuals.h"
#include "Item/ContentCatalog.h"

extern "C" const char* Guild_GetMarkName(int row);
// El owner se transporta como puntero nativo, sin pasar sus bits por float.
// IDA aloca los widgets de tela con el prefijo de count del `eh vector
// constructor iterator`: `raw = operator_new(sizeof(ClothWidget)+4); *raw = 1; obj = raw + 1;`
// El objeto vive en **+4** y el count en +0. El dtor (`sub_45AAA0` con flags&2)
// lee `*((int*)obj - 1)` y libera `obj - 4`, así que sin el prefijo leía el
// header del heap y liberaba un puntero inválido → AV dentro de operator_delete.
static inline void *ClothNew(void) {
    int *raw = (int *)operator_new(sizeof(ClothWidget)+4);
    if (!raw) return nullptr;
    raw[0] = 1;                       // count = 1 elemento
    return Widget_CtorBase(raw + 1);     // el ctor recibe el objeto, no el bloque
}




// IDA: Calc_RenderObject precedía a SpringMesh_Create dentro de RenderCharacter.
// DESVIACION: recrear su geometría SIM antes del alta, sin heredar vértices del draw.
static void PrepareMeshClothVertices(CHARACTER* entity) {
    OBJECT &owner=*AsObject(entity);
    void *model=static_cast<void*>(AsModel(DAT_05828d58)+owner.Type);
    EquipmentModelContext context(model);
    float *bones=owner.EnableBoneMatrix ? (float *)owner.BoneMatrices : (float *)g_BoneScratch;
    AsModel(model)->BodyHeight=0;
    ObjectPose_PrepareBones(AsObject(entity),model,bones,1);
    float minimum[3],maximum[3],bounds[15];
    memcpy(minimum,owner.BoundingBoxMin,sizeof(minimum)); memcpy(maximum,owner.BoundingBoxMax,sizeof(maximum));
    const float lod=_DAT_005597c8;
    _DAT_005597c8=1;
    Skeleton_Transform(model,bones,minimum,maximum,bounds,1);
    _DAT_005597c8=lod;
}

// IDA: rutas de tela de RenderCharacter (0x00456770).
// DESVIACION: creación, cinco subpasos y destrucción se ejecutan por tick;
// los callers de render conservan sólo la llamada al dibujo de la tela viva.
void CharacterCloth_Tick(CHARACTER* entity)
{
    CHARACTER* character = entity;
    const BYTE bVar7 = AsCharacter(entity)->MonsterIndex;
    if (bVar7 == 89 || bVar7 == 95 || bVar7 == 112 || bVar7 == 118 ||
        bVar7 == 124 || bVar7 == 130 || bVar7 == 136) {
        if (character->Object.Cloth == 0) {
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
            void *puVar8 = operator_new(sizeof(ClothMeshWidget)+4);
            *(int *)puVar8 = 1;                    // count del eh vector ctor
            void *clothObj = (char *)puVar8 + 4;   // el objeto vive en +4
            ClothConstruct(static_cast<ClothMeshWidget*>(clothObj), 1, Widget_Ctor);
            PrepareMeshClothVertices(entity);
            SpringMesh_Create(clothObj, &character->Object, 2, 0x12, 0x400, -1);
            VerletNode_AddToSystem(clothObj, 0.0f,   0.0f, 0.0f, 50.0f, 18);
            VerletNode_AddToSystem(clothObj, 0.0f, -20.0f, 0.0f, 30.0f, 18);
            character->Object.Cloth = clothObj;
            character->Object.ClothCount = 1;
        }
        ClothWidget* piVar16 = static_cast<ClothWidget*>(character->Object.Cloth);
        if (piVar16) {
            int iVar9 = (int)(size_t)Widget_CheckState(piVar16, 0x3ba3d70a, 5);
            if (iVar9 == 0)
                DeleteCloth(character, &character->Object, nullptr);
        }

    }
    if (bVar7 == 71 || bVar7 == 74) {

        // Glow bar widget for special skill (0x1ed = Meteor / 0x1ef = another)
        if (character->Object.Cloth == 0) {
            int iType = (bVar7 == 0x47) ? 0x1ed : 0x1ef;
            void *puVar8 = ClothNew();
            // IDA L493: sub_408130(v33, c, 19, 10.0, 0, 5, 15, 30.0, 300.0, tex, tex, 0x1100)
            // (los enteros del decompile: 1106247680 = 0x41F00000 = 30.0;
            // 1133903872 = 0x43960000 = 300.0).
            GridSpring_Create(puVar8, &character->Object, 0x13, 10.0f, 0.0f,
                         5, 0xf, 30.0f, 300.0f, iType, iType, 0x1100);
            character->Object.Cloth = puVar8;
            character->Object.ClothCount = 1;
        }
        ClothWidget* piVar16 = static_cast<ClothWidget*>(character->Object.Cloth);
        if (piVar16) {
            int iVar9 = (int)(size_t)Widget_CheckState(piVar16, 0x3ba3d70a, 5);
            if (iVar9 == 0) DeleteCloth(character, &character->Object, nullptr);
        }

    }
    if (bVar7 == 77) {
        OBJECT &o = AsCharacter(entity)->Object;
        float *bones = (float *)o.BoneMatrices;
        if (bones) memcpy(g_AttackEffectMatrix_04D, bones + 1152 / sizeof(float), sizeof(g_AttackEffectMatrix_04D));
        ++o.Type;
        void *selectModel = static_cast<void*>(AsModel(DAT_05828d58) + o.Type);
        char context[sizeof(BMD)];
        memcpy(context, selectModel, sizeof(context));
        ObjectPose_PrepareSimulation(AsObject(entity), 1);
        if (bones) {
            memcpy(g_AttackEffectMatrix_04D_Alt, bones + 1104 / sizeof(float), sizeof(g_AttackEffectMatrix_04D_Alt));
            memcpy(g_AttackEffectMatrix_04D_Aux, bones + 672 / sizeof(float), sizeof(g_AttackEffectMatrix_04D_Aux));
        }
        if (character->Object.Cloth == 0) {
            void *puVar8 = ClothNew();
            // IDA L598: sub_408130(v27, o, 10, -10.0, 0, 5, 12, 15.0, 240.0, 1275, 1275, 0x1100)
            // El ancho era 60.0; 1097859072 = 0x41700000 = 15.0.
            GridSpring_Create(puVar8, &character->Object, 10, -10.0f, 0.0f,
                         5, 0xc, 15.0f, 240.0f, 0x4fb, 0x4fb, 0x1100);
            VerletNode_AddToSystem(puVar8, 0.0f, 0.0f, 40.0f, 30.0f, 10);
            character->Object.Cloth = puVar8;
            character->Object.ClothCount = 1;
        }
        ClothWidget* piVar16 = static_cast<ClothWidget*>(character->Object.Cloth);
        if (piVar16) {
            int iVar9 = (int)(size_t)Widget_CheckState(piVar16, 0x3ba3d70a, 5);
            if (iVar9 == 0) DeleteCloth(character, &character->Object, nullptr);
        }

        memcpy(selectModel, context, sizeof(context));
        --o.Type;
    }
    OBJECT &object = AsCharacter(entity)->Object;
    const bool subtypeNpc = object.Type == 390 && object.SubType >= 206 && object.SubType <= 208;
    if (!subtypeNpc && !AsCharacter(entity)->Change) {
        PART_t *piVar16 = character->BodyPart;
        for (int slot = 0; slot < 6; ++slot, ++piVar16) {
            const short raw = piVar16->Type;
            if (raw == -1) continue;
            const int iVar9 = gContentCatalog.EntityDrawModel(character, raw);
            const bool isGrandSoulPants = (iVar9 == 706);
            if (piVar16->Cloth == nullptr && isGrandSoulPants) {
                // IDA: operator_new(sizeof(ClothWidget)) + sub_407FE0, SIN el prefijo de
                // count del `eh vector` (a diferencia de la capa del MG):
                // DeleteCloth la libera con el dtor en modo 1.
                void *cloth = Widget_CtorBase(operator_new(sizeof(ClothWidget)));
                // sub_408130(obj, c, 2, 10.0, 10.0, 5, 15, 45.0, 85.0,
                //            1276, 1276, 0x1400)
                GridSpring_Create(cloth, &character->Object, 2, 10.0f, 10.0f,
                             5, 15, 45.0f, 85.0f, 1276, 1276, 0x1400);
                // sub_409250(obj, 0, -15.0, -20.0, 30.0, 2)
                VerletNode_AddToSystem(cloth, 0.0f, -15.0f, -20.0f, 30.0f, 2);
                piVar16->Cloth = cloth;
            }
            if (piVar16->Cloth) {
                if (!isGrandSoulPants) {
                    DeleteCloth(character, nullptr, nullptr);          // DeleteCloth(c, 0, 0)
                } else if (!Widget_CheckState(piVar16->Cloth, 0x3ba3d70a, 5)) {
                    DeleteCloth(character, &character->Object, nullptr);  // DeleteCloth(c, o, 0)
                }
            }

        }
    }
    if (object.Type == 390) {
        BYTE* cb = reinterpret_cast<BYTE*>(character);
        bool bCape = false;
        if (((AsCharacter(cb)->Class & 7) == 3) && (character->Object.Type == 390))
            bCape = true;                    // Magic Gladiator
        if (AsCharacter(cb)->MonsterIndex == 55)
            bCape = true;

        // IDA RenderCharacter: Soccer gives the standard cloth cape to
        // both participating guilds.  The observer branch is also kept
        // because it is part of the same original predicate, even though
        // the current MuEmu build does not emit F3/23 observer state.
        const short guildIndex = AsCharacter(cb)->GuildMarkIndex;
        if (guildIndex != -1) {
            const char* guildName = Guild_GetMarkName(guildIndex);
            if (EnableSoccer && DAT_07abf5d8) {
                const short heroGuild = AsCharacter(DAT_07abf5d8)->GuildMarkIndex;
                if (heroGuild != -1 &&
                    (strcmp(Guild_GetMarkName(heroGuild), guildName) == 0 ||
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
            if (character->Object.Cloth == 0) {
                void* cloth = ClothNew();
                if (AsCharacter(cb)->MonsterIndex == 55) {
                    GridSpring_Create(cloth, &character->Object, 19, 10.0f, 0.0f,
                                 10, 10, 55.0f, 140.0f, 492, 492, 4097);
                    VerletNode_AddToSystem(cloth, -10.0f, -10.0f, -10.0f, 35.0f, 17);
                    VerletNode_AddToSystem(cloth,  10.0f, -10.0f, -10.0f, 35.0f, 17);
                    VerletNode_AddToSystem(cloth,   0.0f, -10.0f, -20.0f, 50.0f, 19);
                } else {
                    GridSpring_Create(cloth, &character->Object, 19, 10.0f, 0.0f,
                                 10, 10, 75.0f, 120.0f, 490, 491, 1029);
                    VerletNode_AddToSystem(cloth, -10.0f, -10.0f, -10.0f, 25.0f, 17);
                    VerletNode_AddToSystem(cloth,  10.0f, -10.0f, -10.0f, 25.0f, 17);
                    VerletNode_AddToSystem(cloth, -10.0f, -10.0f,  20.0f, 27.0f, 17);
                    VerletNode_AddToSystem(cloth,  10.0f, -10.0f,  20.0f, 27.0f, 17);
                }
                character->Object.Cloth = cloth;
                character->Object.ClothCount = 1;
            }
            ClothWidget* pCloth = static_cast<ClothWidget*>(character->Object.Cloth);
            if (pCloth) {
                int alive = (int)(size_t)Widget_CheckState(pCloth, 0x3ba3d70a, 5);
                if (alive == 0) {
                    DeleteCloth(character, &character->Object, nullptr);
                }
            }
        }
    }
}
