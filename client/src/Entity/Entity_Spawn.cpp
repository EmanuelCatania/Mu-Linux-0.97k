// IDA: CreateCharacterPointer (0x0045ADC0)
// Inicializa una entidad del pool CharactersClient. El layout está en
// Entity/EntityTypes.h.
//
// El binario rodea las escrituras de PositionX/PositionY (0x388/0x38C) con la
// hash-table anti-tamper (líneas 1-480 del decompile); acá se escriben directo.

#include "stdafx.h"
#include "Entity/EntityTypes.h"
#include "Scene/ObjectRegistry.h"
#include "Item/ContentCatalog.h"
#include "globals.h"
#include "functions.h"
#include "Platform/PlatformHeaders.h"

// IDA: CreateCharacterPointer (0x0045ADC0)
void __cdecl CreateCharacterPointer(unsigned char *param_1, int Type,
                          int PositionX, int PositionY, float Rotation)
{
    CHARACTER* c = AsCharacter(param_1);
    OBJECT*    o = &c->Object;

    // 0.97.20: la entidad nueva no hereda los modelos propios del slot.
    gContentCatalog.ClearEntityParts(param_1);

    unsigned int grid_x      = (unsigned int)PositionX;
    unsigned int grid_y      = (unsigned int)PositionY;
    unsigned int entity_type = (unsigned int)Type;

    *(unsigned int *)&c->PositionX = grid_x;
    *(unsigned int *)&c->PositionY = grid_y;
    // IDA L354-355: la casilla destino arranca en la de spawn.
    c->TargetX = (unsigned char)PositionX;
    c->TargetY = (unsigned char)PositionY;

    // Atributo del terreno en la posición de spawn → SafeZone (+0x34E)
    int terrainAttr = Terrain_GetTileIndex((int)grid_x, (int)grid_y);
    if ((unsigned char)DAT_0838bc70[terrainAttr] & 1) {
        c->SafeZone = 1;   // spawn dentro de zona segura
    } else {
        c->SafeZone = 0;
    }

    // Reset movement/path state
    c->PathNum     = 0;
    c->PathCurrent = 0;
    c->Movement    = 0;

    // Mark active + reset anim/combat fields
    o->Live             = 1;
    o->Visible          = 0;
    o->AlphaEnable      = 1;
    o->LightEnable      = 1;
    o->ContrastEnable   = 0;
    o->EnableBoneMatrix = 1;
    o->EnableShadow     = 0;
    c->Dead             = 0;
    c->Blood            = 0;
    c->GuildTeam        = 0;
    c->Run              = 0;

    // Entity type and initial values
    c->GuildMarkIndex          = (short)0xffff;
    c->PK                      = 3;
    o->Type                    = (short)entity_type;
    o->Scale                   = 0.9f;         // 0x3f666666
    *(int *)&o->Timer          = 0;
    o->Alpha                   = 1.0f;
    o->AlphaTarget             = 1.0f;
    *(int *)&o->Velocity       = 0;
    *(int *)&o->ShadowScale    = 0;
    param_1[0x194]             = 0;

    // Class: se limpia salvo para los tipos 0xaa..0xad
    if ((int)entity_type < 0xaa || 0xad < (int)entity_type) {
        c->Class = 0;
    }

    // IDA L458-466: acción 1 sólo para el jugador (Type 390)
    if (entity_type == 0x186) {
        o->PriorAction   = 1;
        o->CurrentAction = 1;
    } else {
        o->PriorAction   = 0;
        o->CurrentAction = 0;
    }

    *(int *)&o->AnimationFrame      = 0;
    *(int *)&o->PriorAnimationFrame = 0;
    c->JumpTime                     = 0;
    o->HiddenMesh                   = -1;
    *(short *)&c->MoveSpeed         = 10;
    *(int *)(param_1 + 0x78)        = 0;
    o->Teleport                     = 0;
    o->Kind                         = 1;
    c->Change                       = 0;
    o->SubType                      = 0;
    c->MonsterIndex                 = 0xff;
    o->BlendMeshTexCoordU           = 0.0f;
    o->BlendMeshTexCoordV           = 0.0f;

    // ── Posición de mundo desde la casilla ───────────────────────────────────
    o->Position[0] = (float)(int)grid_x * _DAT_005524f0 + _DAT_00552598;
    o->Position[1] = (float)(int)grid_y * _DAT_005524f0 + _DAT_00552598;

    // World Z: terrain height + height offset based on entity type / state
    float terrainH = RequestTerrainHeight(o->Position[0], o->Position[1]);
    if (gMapManager.GetCurrentMap() == -1 ||
        c->Helper.Type != (short)0x333 ||
        c->SafeZone != '\0')
    {
        // Normal spawn
    }
    else if (gMapManager.GetCurrentMap() == 8 || gMapManager.GetCurrentMap() == 10) {
        terrainH += (float)_DAT_00552848;
    }
    else {
        terrainH += (float)_DAT_0055284c;
    }
    o->Position[2] = terrainH;

    // ── Object.Angle init + Rotation (IDA L640-651) ──────────────────────────
    *(int *)&o->Angle[0] = 0;
    *(int *)&o->Angle[1] = 0;
    o->Angle[2]          = Rotation;

    // IDA L641-642: -1032847360 = 0xC2700000 = -60.0f.
    *(int *)&o->BoundingBoxMin[0] = 0xc2700000;   // -60.0f
    *(int *)&o->BoundingBoxMin[1] = 0xc2700000;
    *(int *)&c->Light[0]          = 0;   // IDA L645 (+800)
    *(int *)&c->Light[1]          = 0;
    *(int *)&c->Light[2]          = 0;
    *(int *)&o->Light[0]          = 0;
    *(int *)&o->Light[1]          = 0;
    *(int *)&o->Light[2]          = 0;
    *(int *)&o->BoundingBoxMin[2] = 0;            // IDA L651

    // ── Per-Type bounding box (IDA L652-704) ─────────────────────────────────
    switch ((int)entity_type) {
        case 272: case 276: case 279: case 287: case 289: case 290:
            *(int *)&o->BoundingBoxMax[0] = 1112014848;
            *(int *)&o->BoundingBoxMax[1] = 1112014848;
            *(int *)&o->BoundingBoxMax[2] = 1117782016;
            break;
        case 281: case 301: case 309: case 312: case 314:
            *(int *)&o->BoundingBoxMax[0] = 1116471296;
            *(int *)&o->BoundingBoxMax[1] = 1116471296;
            *(int *)&o->BoundingBoxMax[2] = 1132068864;
            break;
        case 307:
            *(int *)&o->BoundingBoxMax[0] = 1120403456;
            *(int *)&o->BoundingBoxMax[1] = 1120403456;
            *(int *)&o->BoundingBoxMax[2] = 1125515264;
            break;
        case 330:
            *(int *)&o->BoundingBoxMin[0] = (int)0xC2380000;  // -46.0
            *(int *)&o->BoundingBoxMin[1] = (int)0xC2200000;  // -40.0
            *(int *)&o->BoundingBoxMin[2] = 0;
            *(int *)&o->BoundingBoxMax[0] = 1119092736;
            *(int *)&o->BoundingBoxMax[1] = 1112014848;
            *(int *)&o->BoundingBoxMax[2] = 1128792064;
            break;
        case 331:
            *(int *)&o->BoundingBoxMin[2] = 0;
            *(int *)&o->BoundingBoxMin[0] = (int)0xC2C00000;
            *(int *)&o->BoundingBoxMin[1] = (int)0xC2C00000;
            *(int *)&o->BoundingBoxMax[0] = 1120403456;
            *(int *)&o->BoundingBoxMax[1] = 1120403456;
            *(int *)&o->BoundingBoxMax[2] = 1133903872;
            break;
        case 390:  // hero (CRITICAL for char-select rendering)
            *(int *)&o->BoundingBoxMax[0] = 1109393408;  // 40.0f
            *(int *)&o->BoundingBoxMax[1] = 1109393408;  // 40.0f
            *(int *)&o->BoundingBoxMax[2] = 1123024896;  // 120.0f
            break;
        default:
            *(int *)&o->BoundingBoxMax[0] = 1112014848;
            *(int *)&o->BoundingBoxMax[1] = 1112014848;
            *(int *)&o->BoundingBoxMax[2] = 1125515264;
            break;
    }

    o->InitialSceneFrame = MoveSceneFrame;

    // ── BoneTransform ─────────────────────────────────────────────────────────
    // Una matriz 3x4 (0x30 bytes) por hueso; la cantidad es el short en +0x22
    // del MODEL_t, el mismo campo que recorre BMD_Animation.
    SceneObjects::UnregisterBones(param_1);
    if (*(unsigned char **)&o->BoneMatrices != NULL) {
        operator_delete(*(unsigned char **)&o->BoneMatrices);
        *(int *)&o->BoneMatrices = 0;
    }
    {
        short etype     = o->Type;
        short boneCount = *(short *)(DAT_05828d58 + etype * 0xbc + 0x22);
        if (boneCount < 0) boneCount = 0;   // defense: short garbage → signed neg
        void *boneBuf = operator_new((unsigned int)boneCount * 0x30);
        // DESVIACION: IDA 0045ADC0 L710 hace `operator_new` sin inicializar y el buffer
        // se LEE antes de escribirse (el tick del spawn, MoveCharacterVisual 0x4520C0,
        // transforma huesos que Calc_RenderObject todavia no lleno).  En release eso son
        // paginas frescas del OS (ceros); con el relleno 0xCD del CRT debug los joints
        // nacen en ~-5.6e8 y dibujan haces que cruzan la pantalla.  Se inicializa a
        // cero, que es lo que el original obtiene de hecho.
        if (boneBuf && boneCount > 0)
            memset(boneBuf, 0, (size_t)boneCount * 0x30);
        *(void **)&o->BoneMatrices = boneBuf;
        SceneObjects::RegisterBones(param_1, boneBuf, (size_t)boneCount * 0x30);
    }

    // ── Partes ────────────────────────────────────────────────────────────────
    // Weapon[0..1]
    {
        unsigned char *slot = (unsigned char *)&c->Weapon[0].Level;
        for (int i = 0; i < 2; i++) {
            *(short *)(slot - 2) = (short)0xffff;
            slot[0] = 0;
            slot[1] = 0;
            slot += 0x18;
        }
    }
    // BodyPart[0..5]
    {
        unsigned char *slot = (unsigned char *)&c->BodyPart[0].Level;
        for (int i = 0; i < 6; i++) {
            *(short *)(slot - 2) = (short)0xffff;
            slot[0] = 0;
            slot[1] = 0;
            slot += 0x18;
        }
    }

    // ── Final fixed values ────────────────────────────────────────────────────
    c->Wing.Type                 = (short)0xffff;
    c->Helper.Type               = (short)0xffff;
    c->Flag.Type                 = (short)0xffff;
    c->LongRangeAttack           = -1;              // 0xffffffff
    c->CollisionTime             = 0;
    *(int *)&o->CollisionRange   = 0x43480000;      // 200.0f
    *(int *)&c->Rot              = 0;
    *(short *)&c->Level          = 0;
    c->Item                      = -1;
    o->BlendMesh                 = -1;
    o->BlendMeshLight            = 1.0f;

    // Weapon attachment bones. IDA 0045ADC0 L744-857 selects these from the
    // parent model; RenderLinkObject then uses Weapon[n].LinkBone.
    BYTE& lb0 = c->Weapon[0].LinkBone;
    BYTE& lb1 = c->Weapon[1].LinkBone;
    switch ((int)entity_type) {
        case 270: case 300: lb0 = 42; lb1 = 33; break;
        case 271:           lb0 = 19; lb1 = 14; break;
        case 273:           lb0 = 26; lb1 = 36; break;
        case 274: case 275: case 280: lb0 = 41; lb1 = 32; break;
        case 278:           lb0 = 29; lb1 = 38; break;
        case 281: case 286: case 299: case 304: case 305: case 323: case 327:
                            lb0 = 30; lb1 = 39; break;
        case 288:           lb0 = 26; lb1 = 35; break;
        case 289:           lb0 = 31; lb1 = 22; break;
        case 291:           lb0 = 24; lb1 = 19; break;
        case 292:           lb0 = 25; lb1 = 16; break;
        case 294:           lb0 = 39; lb1 = 30; break;
        case 296:           lb0 = 16; lb1 = 25; break;
        case 297:           lb0 = 17; lb1 = 28; break;
        case 306:           lb0 = 52; lb1 = 65; break;
        case 310:           lb0 = 20; lb1 = 33; break;
        case 311:           lb0 = 23; break;
        case 312:           lb0 = 43; break;
        case 314:           lb0 = 55; lb1 = 70; break;
        case 316:           lb0 = 39; lb1 = 39; break;
        case 317:           lb0 = 27; lb1 = 38; break;
        case 318:           lb0 = 32; lb1 = 43; break;
        case 322:           lb0 = 36; lb1 = 45; break;
        case 325:           lb0 = 27; lb1 = 18; break;
        case 329:           lb0 = 33; lb1 = 20; break;
        case 330:           lb0 = 1;  lb1 = 1;  break;
        default:            lb0 = 33; lb1 = 42; break;
    }
}
