// Ambient_Particles.cpp
// IDA: Ambient_ParticleUpdate @ 0x00502320 — AmbientParticles_Update
//
// Fauna ambiental que nada/camina cerca del héroe: los peces de Lorencia y
// Atlans, y los bichos de Dungeon, Lost Tower y Tarkan. Comparte el pool con
// RenderFishs (0x00502200): DAT_083a2e90, 10 slots × OBJECT_STRIDE. El binario
// recorre el pool desde el ancla 0x083A2F78 (= slot + 0xE8), por eso el
// decompile lee todos los campos con índices negativos; acá se usan los campos
// del OBJECT.
//
// Mapa → modelo (IDA L118-176):
//   World 0 (Lorencia) → 181 (fish)                       sobre tiles con Layer1 == 5
//   World 1 (Dungeon)  → 177                              sobre TerrainWall < 8
//   World 6 (Arena)    → 178                              sobre TerrainWall < 8
//   World 7 (Atlans)   → Y < 128: 184..187 (Fish04..07)   sobre TerrainWall ∈ {0, 2}
//                        Y >= 128: 188/189 (Fish08/09)
//   World 8 (Tarkan)   → 179 + joint 1258                 sobre TerrainWall ∈ {0, 2}
// Los slots 3..9 sólo se usan en Atlans y Tarkan.
//
// Campos del OBJECT que usa esta función:
//   SubType         contador de rebotes: dos seguidos contra pared → muere
//   Scale / Velocity / Gravity, LifeTime (frames de animación activa),
//   Timer (fase del brillo de Fish08/09 en BlendMeshLight),
//   +0x40 vec3      posición previa del bicho 179 (sin nombre en OBJECT),
//   +0x8A byte      = 1 al nacer (sin nombre en OBJECT),
//   +0x90 matriz    3x4 de orientación (AngleMatrix),
//   Direction[0..1] punto adelantado (posición + paso × 3).

#include "stdafx.h"
#include "Game/RenderFxSnapshots.h"
#include "Render/ResourceTables.h"

namespace {

// Campos del OBJECT sin nombre en EntityTypes.h. Todos caen antes de Owner
// (0xFC), así que el offset es el mismo en x86 y x64.
inline float* AmbientPrevPos(OBJECT* o)  { return reinterpret_cast<float*>(reinterpret_cast<BYTE*>(o) + 0x40); }
inline BYTE*  AmbientFlag8A(OBJECT* o)   { return reinterpret_cast<BYTE*>(o) + 0x8A; }
inline float* AmbientMatrix(OBJECT* o)   { return reinterpret_cast<float*>(reinterpret_cast<BYTE*>(o) + 0x90); }

inline float HeroX() { return *(const float*)(DAT_07abf5d8 + 0x10); }
inline float HeroY() { return *(const float*)(DAT_07abf5d8 + 0x14); }
inline float HeroZ() { return *(const float*)(DAT_07abf5d8 + 0x18); }

inline int TileAt(float x, float y)
{
    // IDA: Terrain_Load((__int64)(x * 0.0099999998), (__int64)(y * 0.0099999998))
    return Terrain_GetTileIndex((unsigned int)(long long)(x * 0.0099999998f),
                                (unsigned int)(long long)(y * 0.0099999998f));
}

// IDA L82-118: el slot sólo nace sobre ciertos tiles según el mapa.
bool AmbientCanSpawnOn(int world, int tile)
{
    switch (world) {
    case 0:  return TerrainMappingLayer1[tile] == 5;
    case 1:
    case 6:  return TerrainWall[tile] < 8;
    case 7:
    case 8:  return TerrainWall[tile] == 0 || TerrainWall[tile] == 2;
    default: return false;
    }
}

// IDA L105-176. Devuelve false si el slot no quedó vivo.
bool AmbientSpawn(OBJECT* o, int world)
{
    o->Position[0] = (float)(rand() % 1024 - 512) + HeroX();
    o->Position[1] = (float)(rand() % 1024 - 512) + HeroY();
    o->Position[2] = HeroZ();
    if (!AmbientCanSpawnOn(world, TileAt(o->Position[0], o->Position[1])))
        return false;

    o->Light[0] = 0.5f;
    o->Light[1] = 0.5f;
    o->Light[2] = 0.5f;
    RenderFxSnapshots_New(o);
    o->Live = 1;
    o->Alpha = 0.0f;
    o->AlphaTarget = 1.0f;
    o->BlendMesh = -1;
    o->Angle[0] = 0.0f;
    o->Angle[1] = 0.0f;
    o->Angle[2] = 0.0f;
    o->LightEnable = 1;
    o->AlphaEnable = 1;
    o->SubType = 0;
    o->HiddenMesh = -1;
    o->LifeTime = rand() % 128;
    const int scaleRand = rand() % 4;
    o->Gravity = 13.0f;
    *AmbientFlag8A(o) = 1;
    o->Scale = (float)(scaleRand + 4) * 0.1f;

    switch (world) {
    case 0:
        o->Type = 181;
        o->AlphaTarget = (float)(rand() % 2 + 2) * 0.1f;
        o->Velocity = 0.6f / o->Scale;
        break;
    case 1:
        o->Type = 177;
        o->Velocity = 0.6f / o->Scale;
        break;
    case 6:
        o->Type = 178;
        o->Velocity = 0.6f / o->Scale;
        break;
    case 7: {
        o->Scale = (float)(rand() % 2 + 8) * 0.1f;
        if (HeroY() * 0.0099999998f >= 128.0f) {
            // Atlans 3: Fish08/Fish09, con brillo animado en la malla 0.
            const short type = (short)(rand() % 2 + 188);
            o->Type = type;
            if (type == 188 || type == 189) {
                o->BlendMesh = 0;
                o->BlendMeshLight = 1.0f;
            }
            o->Velocity = 0.5f / o->Scale;
            o->Gravity = (rand() % 2 == 0) ? 1.0f : 2.0f;
        } else {
            // Atlans 1 y 2: Fish04..Fish07.
            const short type = (short)(rand() % 4 + 184);
            o->Velocity = 1.0f / o->Scale;
            o->Type = type;
            o->Gravity = (rand() % 2 != 0) ? 3.0f : 2.0f;
        }
        // IDA LABEL_40: nadan entre 50 y 200 unidades sobre el fondo.
        o->Timer = (float)(rand() % 32) * 0.1f;
        const float floor = RequestTerrainHeight(o->Position[0], o->Position[1]);
        o->Position[2] = (float)(rand() % 150 + 50) + floor;
        break;
    }
    case 8: {
        o->Type = 179;
        const int r = rand() % 4;
        float* prev = AmbientPrevPos(o);
        prev[0] = o->Position[0];
        prev[1] = o->Position[1];
        prev[2] = o->Position[2];
        o->Scale = (float)(r + 8) * 0.1f;
        o->Gravity = 9.0f;
        o->LifeTime = 100;
        o->Velocity = 2.5f / o->Scale;
        Joint_Create(1258, o->Position, o->Position, o->Angle, 4,
                     (intptr_t)o, 30.0f, -1, 0);
        break;
    }
    default:
        break;
    }
    return o->Live != 0;
}

// IDA L182-310: movimiento, rebote contra paredes y vida útil.
void AmbientMove(OBJECT* o, int slotIndex)
{
    void* model = (void*)(DAT_05828d58 + o->Type * BMD_STRIDE);
    *(BYTE*)((BYTE*)model + 0xa0 + BMD_FIELD_SHIFT) = o->CurrentAction;
    BMD__PlayAnimation(model, &o->AnimationFrame, &o->PriorAnimationFrame,
                       &o->PriorAction, o->Velocity * 0.5f);

    const short type = o->Type;
    if (type == 188 || type == 189) {
        o->BlendMeshLight = sinf(o->Timer) * 0.40000001f + 0.5f;
        o->Timer += 0.1f;
    }

    if ((type >= 181 && type <= 191) || o->LifeTime > 0) {
        SetAction(o, type == 178);
        Particle_PathUpdate(o, slotIndex, DAT_083a2e90, 10);
        Matrix_BuildFromEuler(o->Angle, AmbientMatrix(o));

        float step[3] = { (float)(rand() % 4 + 6) * o->Velocity, 0.0f, 0.0f };
        float out[3] = { 0.0f, 0.0f, 0.0f };
        Vector_Rotate(step, AmbientMatrix(o), out);

        const int world = gMapManager.GetCurrentMap();
        o->Position[0] += out[0];
        o->Position[1] += out[1];
        o->Position[2] += out[2];
        if (world != 7)
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
        o->Direction[0] = out[0] * 3.0f + o->Position[0];
        o->Direction[1] = out[1] * 3.0f + o->Position[1];

        const int tile = TileAt(o->Position[0], o->Position[1]);
        bool blocked = (type == 181 && TerrainMappingLayer1[tile] != 5) ||
                       (type == 177 && TerrainWall[tile] >= 8);
        if (!blocked && (world == 7 || world == 8)) {
            const BYTE wall = TerrainWall[tile];
            blocked = (wall == 1 || wall >= 8);
        }

        if (blocked) {
            // Media vuelta; dos choques seguidos lo matan.
            o->Angle[2] += 180.0f;
            if (o->Angle[2] >= 360.0f)
                o->Angle[2] -= 360.0f;
            o->SubType += 1;
        } else if (o->SubType > 0) {
            o->SubType -= 1;
        }
        if (o->SubType >= 2)
            o->Live = 0;

        if (type == 179) {
            float* prev = AmbientPrevPos(o);
            prev[0] = o->Position[0];
            prev[1] = o->Position[1];
            prev[2] = o->Position[2];
        }

        const float dy = o->Position[1] - HeroY();
        const float dx = o->Position[0] - HeroX();
        const float dist = sqrtf(dy * dy + dx * dx);
        if (dist >= 1500.0f)
            o->Live = 0;
        if (dist < 600.0f && type == 177 && rand() % 256 == 0)
            PlayBuffer(15, o, 0);
    } else {
        SetAction(o, 0);
    }

    o->LifeTime -= 1;
    if (o->LifeTime <= 0 && o->Type != 178 && rand() % 64 == 0)
        o->LifeTime = rand() % 128;

    Alpha(o);
}

} // namespace

void __cdecl AmbientParticles_Update(void)
{
    if (DAT_07abf5d8 == nullptr)
        return;   // guarda propia: el spawn se ancla en la posición del héroe
    for (int i = 0; i < 10; ++i) {
        OBJECT* o = AsObject(DAT_083a2e90 + i * OBJECT_STRIDE);
        const int world = gMapManager.GetCurrentMap();
        // IDA L77: fuera de Atlans y Tarkan sólo se usan los 3 primeros slots.
        if (world != 7 && world != 8 && i >= 3)
            continue;
        if (!o->Live && !AmbientSpawn(o, world))
            continue;
        if (o->Type < 0 || (size_t)o->Type >= ResourceTables::ModelCount())
            continue;   // guarda propia: slot reciclado con tipo basura
        AmbientMove(o, i);
    }
}
