#include <cstdint>
// MoveEffect.cpp
// MoveEffect @ 0x00466AD0  [Kayito: MoveEffect]
//
// Per-frame particle/effect spawner for active effect slots.
// Called by Effect_TickAll (MoveEffects) once per slot per frame.
//
// param_1 = float* pointing to effect slot (stride 0x1bc, base DAT_07b11670)
// param_2 = slot index
//
// Effect slot layout (float offsets):
//   [+0x00/char] = active flag
//   [+0x02/short]= entity type id (iVar9)
//   [+0x04]      = position X
//   [+0x05]      = position Y
//   [+0x06]      = position Z
//   [+0x07..09]  = angle/dir XYZ
//   [+0x0a..0c]  = angle2 XYZ
//   [+0x10]      = cached pos X
//   [+0x11]      = cached pos Y
//   [+0x12]      = cached pos Z
//   [+0x16]      = some float state
//   [+0x18]      = lifetime counter (fVar13)
//   [+0x1a]      = alpha/scale
//   [+0x1b]      = secondary scale
//   [+0x1e]      = flags word
//   [+0x21/byte] = byte flag
//   [+0x22/byte] = byte flag2
//   [+0x30..32]  = velocity XYZ
//   [+0x33]      = some float
//   [+0x36]      = Z velocity / bounce
//   [+0x3a..3c]  = color RGB
//   [+0x3d]      = float flag
//   [+0x3f]      = entity ptr (target entity)
//   [+0x42..43]  = float counters
//   [+0x5a]      = alpha override
//   [+0x5c..5e]  = secondary position XYZ
//   [+0x85/byte] = byte param
//   [+0x86/short]= short param
//   [+0x105/byte]= anim state
//   [+0x106/byte]= anim state prev
//
// External functions:
//   Particle_Spawn(type, pos, dir, color, mode, size, entity_ptr)
//   CreateEffect(type, pos, dir, color, v1, v2, v3, v4, flag)
//   Joint_Create = CreateJoint(type, pos, target, dir, mode, entity, size, ...)
//   Matrix_BuildFromEuler = AngleMatrix(angles, matrix)
//   Vector_Rotate = EulerToMatrix3x4(v, matrix, out)
//   RequestTerrainHeight(x, y)
//   AddTerrainLight = RequestTerrainLight(x, y, light_ptr, mode, addr)
//   CreateSprite (Flare_Spawn?)
//   Joint_BoneOffsetApply (effect color update)
//   Effect_SpawnProximityHit (effect deactivate?)
//   Effect_CollisionCheck (effect helper)
//   Effect_SpawnSmokeBurst (effect helper2)
//   Effect_SpawnSmokeExplosion (effect helper3)
//   FUN_0046c3e0 = Trail_RenderAll?
//   BMD__PlayAnimation = BMD_SetAnim?
//   Entity_FindNearby_SendPacket = Entity_SpawnImpact?
//   PlayBuffer = Sound_Play(id, slot, flag)
//   Ring_ComputeOrbit (ring?)
//   Terrain_GetTileIndex = Terrain_GetTileAttr(x, y)
//   TERRAIN_INDEX (terrain helper)

#include "stdafx.h"
#include "Scene/ObjectRegistry.h"
#include "Platform/Text.h"
extern "C" void DbgLogPublic(const char*);

namespace {
// Owner puede ser un personaje, efecto, objeto del mapa o de los pools
// ambientales. Aceptar solo comienzos de slots, nunca direcciones interiores.
static bool IsEffectOwner(const void* owner)
{
    if (!owner) return false;
    const auto allocation = (uintptr_t)DAT_055c9e48;
    const auto characters = (uintptr_t)DAT_07abf5d0;
    if (allocation && characters >= allocation &&
        characters - allocation < 0x80u * 0x394u &&
        (characters - allocation) % 0x394u == 0 &&
        SceneObjects::IsSlot(owner, (void*)characters, 0x59740, 0x394)) return true;
    return SceneObjects::Contains(owner) ||
        SceneObjects::IsSlot(owner, DAT_07b11670, sizeof(DAT_07b11670), 0x1bc) ||
        SceneObjects::IsSlot(owner, DAT_07c85890, sizeof(DAT_07c85890), 0x1bc) ||
        SceneObjects::IsSlot(owner, DAT_07c74ec8, sizeof(DAT_07c74ec8), 0x1bc) ||
        SceneObjects::IsSlot(owner, DAT_083a1218, sizeof(DAT_083a1218), 0x1bc) ||
        SceneObjects::IsSlot(owner, DAT_083a2e90, sizeof(DAT_083a2e90), 0x1bc);
}

// IDA: MoveEffect (0x00466AD0), desvanecimiento 247/248/250/251.
struct EffectFadePreset { int DescentThreshold; const float* AlphaFactor; };
constexpr EffectFadePreset EffectFadePresets[] = {
    { 13, &_DAT_00552a00 }, { 10, &_DAT_005529d4 },
};
// IDA: MoveEffect (0x00466AD0), fragmentos 260..263.
struct EffectBouncePreset { const float* Gravity; const float* Bounce; };
constexpr EffectBouncePreset EffectBouncePresets[] = {
    { &_DAT_00552540, &_DAT_00552570 }, { &_DAT_00552650, &_DAT_00552a14 },
};
constexpr float EffectTrailAlpha[] = { 0.6f, 0.5f, 0.4f, 0.3f };
} // namespace

// Tamachan -- IDA 0.98j MoveEffect (sub_4723F0) case 183 (aca, tipo 193; ver
// Tamachan_Create en CreateEffect.cpp).  `life` es el contador +96 leido
// ANTES del decremento de la cola, como el `v3` del binario.  Al volver, el
// caller cae a la cola comun (animacion del modelo, avance por +192 y
// decremento de +96), igual que el `break` / LABEL_524 del 0.98j.
//
// Estados (+260): 0 baja hasta 5 sobre el suelo mirando al heroe;
//                 1 sube hasta 40 mirando al heroe;
//                 2 camina hacia +368 levantando polvo (particula 103);
//                 3 se va: se hunde 5 por frame (lo pone el 0x0B de fin).
// En Atlans (World 7) se queda vivo; en cualquier otro mapa que no sea
// Lorencia desaparece.
//
// Desviacion: los sonidos 101/102 del 0.98j son 118/119 aca (ver
// Sound_LoadGameSamples).
static void Tamachan_FaceHero(char *o)
{
    char *hero = (char *)DAT_07abf5d8;
    if (!hero) return;   // guard del port: el binario asume Hero
    float target = CreateAngle(*(float *)(o + 16), *(float *)(o + 20),
                                *(float *)(hero + 16), *(float *)(hero + 20));
    float diff = target - *(float *)(o + 36);
    if (diff > 5.0f)       *(float *)(o + 36) = *(float *)(o + 36) + 5.0f;
    else if (diff < -5.0f) *(float *)(o + 36) = *(float *)(o + 36) - 5.0f;
}

static void Tamachan_Move(char *o, int life)
{
    if (gMapManager.GetCurrentMap() == 7) {
        *(int *)(o + 96) = 10;
        return;
    }
    if (gMapManager.GetCurrentMap()) {
        *(int *)(o + 96) = -1;
        *o = 0;
        return;
    }
    switch ((unsigned char)o[260]) {
    case 0:
        if (life < 10) {
            *(int *)(o + 216) = 0;
            int state = rand() % 2 + 1;
            o[260] = (char)state;
            SetAction((int)o, state);
            if (o[260] == 2) goto walk;       // LABEL_312
            goto newLife;                     // LABEL_313
        }
        *(float *)(o + 24) = *(float *)(o + 60) + *(float *)(o + 216);
        if (*(float *)(o + 216) > 5.0f) *(float *)(o + 216) = *(float *)(o + 216) - 1.0f;
        else                           *(float *)(o + 216) = 5.0f;
        Tamachan_FaceHero(o);
        if (!(rand() % 100)) PlayBuffer(119, 0, 0);
        return;
    case 1:
        if (life < 10) {
            *(float *)(o + 216) = 40.0f;
            o[260] = 0;
            SetAction((int)o, 0);
            if (o[260] == 2) goto walk;
            goto newLife;
        }
        *(float *)(o + 24) = *(float *)(o + 60) + *(float *)(o + 216);
        if (*(float *)(o + 216) < 40.0f) *(float *)(o + 216) = *(float *)(o + 216) + 1.0f;
        else                            *(float *)(o + 216) = 40.0f;
        Tamachan_FaceHero(o);
        if (!(*(int *)(o + 96) % (rand() % 20 + 10))) PlayBuffer(118, 0, 0);
        return;
    case 2: {
        float dx = *(float *)(o + 16) - *(float *)(o + 368);
        float dy = *(float *)(o + 20) - *(float *)(o + 372);
        *(int *)(o + 96) = 10;
        float dist = (float)sqrt(dy * dy + dx * dx);
        MoveHumming((float *)(o + 16), (float *)(o + 28), (float *)(o + 368), 3000.0f / dist);
        *(float *)(o + 28) = -10.0f;
        *(float *)(o + 32) = 0.0f;
        // En el binario el valor es el st0 de RequestTerrainHeight menos 10
        // (0x475722: `fstp st` descarta 3000/dist, `fsub flt_566460`).
        *(float *)(o + 24) = RequestTerrainHeight(*(float *)(o + 16), *(float *)(o + 20)) - 10.0f;
        if (dist <= 10.0f) {
            float offset[3], angle[3], matrix[12], rotated[3];
            int r = rand();
            offset[0] = 0.0f;
            offset[1] = (float)(-150.0 - (double)(r % 150));
            offset[2] = 0.0f;
            angle[0] = 0.0f;
            angle[1] = 0.0f;
            angle[2] = (float)((double)(rand() % 120) + 180.0 - 60.0);
            Matrix_BuildFromEuler(angle, matrix);
            Vector_Rotate(offset, matrix, rotated);
            for (int i = 0; i < 3; ++i)
                ((float *)(o + 368))[i] = rotated[i] + ((float *)(o + 52))[i];
            *(int *)(o + 96) = rand() % 100 + 50;
            int state = rand() % 3;
            o[260] = (char)state;
            SetAction((int)o, state);
            if (o[260] != 2) {
                *(float *)(o + 192) = 0.0f;
                *(float *)(o + 196) = 0.0f;
                *(float *)(o + 200) = 0.0f;
            }
        }
        if (!(rand() % 3)) {
            float pos[3] = { *(float *)(o + 16), *(float *)(o + 20), *(float *)(o + 24) + 11.0f };
            Particle_Spawn(103, pos, (float *)(o + 28), (float *)(o + 232), 1, 2.0f, 0);
        }
        PlayBuffer(11, 0, 0);
        return;
    }
    case 3:
        *(float *)(o + 24) = *(float *)(o + 24) - 5.0f;
        return;
    }
    return;
walk:
    *(float *)(o + 192) = 0.0f;
    *(float *)(o + 196) = -2.0f;
    *(float *)(o + 200) = 0.0f;
    return;
newLife:
    *(int *)(o + 96) = rand() % 50 + 50;
}

// IDA: MoveEffect (0x00466AD0)
void MoveEffect(float *param_1, int param_2)
{
  const auto validOwner = [param_1](const void* owner) {
    if (IsEffectOwner(owner)) return true;
    const int type = *(short*)((char*)param_1 + 2);
    static bool reported[2048] = {};
    if (type >= 0 && type < 2048 && !reported[type]) {
      reported[type] = true;
      char message[160];
      Platform::TextFormat(message, "MoveEffect: owner fuera de los pools, Type=%d owner=%08X; efecto desactivado",
                           type, (unsigned)(uintptr_t)owner);
      DbgLogPublic(message);
    }
    *(char*)param_1 = 0;
    return false;
  };
  const void* owner = *(const void**)((char*)param_1 + 0xfc);
  // Null es legitimo en efectos sin seguimiento. Los no nulos siempre
  // deben pertenecer a un pool; validar null solo en las ramas que lo leen.
  if (owner && !validOwner(owner)) return;

  // Locales contiguos: el codigo pasa `&local_XXX` a funciones que leen 3 floats
  // consecutivos (Position, Light, matrices), o sea depende de que los locales
  // queden contiguos y en el orden del frame original.  MSVC no lo garantiza:
  // se reconstruye el frame como UN bloque y cada `local_XXX` es una referencia
  // a su offset real (idx = 0x36c - offset).
  alignas(16) unsigned char __mfr[0x374];

  // `*(float *)&AsObject(param_1)->Owner` es el PUNTERO al owner del efecto: IDA lo lee como DWORD
  // (`*((_DWORD *)param_1 + 63)`), no como float.  Son los efectos que siguen a
  // su entidad duena.
  int __owner_fVar13 = 0;
  int __owner_fVar4 = 0;
  int __owner_fVar5 = 0;


  void *pvVar1;
  char cVar2;
  short sVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  uint uVar8;
  int iVar9;
  float *pfVar10;
  char *pcVar11;
  int iVar12;
  float fVar13;
  undefined2 extraout_var;
  float *pfVar14;
  float *pfVar15;
  bool bVar16;
  float10 fVar17;
  float10 fVar18;
  longlong lVar19;
  longlong lVar20;
  float *pfVar21;
  float *pfVar22;
  float *pfVar23;
  int iVar24;
  float *pfVar25;
  float *pfVar26;
  float *pfVar27;
  byte bVar28;
  undefined4 uVar29;
  int iVar30;
  float &local_36c = *(float*)&__mfr[0x0];
  float &local_368 = *(float*)&__mfr[0x4];
  float &local_364 = *(float*)&__mfr[0x8];
  float &local_360 = *(float*)&__mfr[0xc];
  float *local_35c;
  float &local_358 = *(float*)&__mfr[0x14];
  float &local_354 = *(float*)&__mfr[0x18];
  float &local_350 = *(float*)&__mfr[0x1c];
  float &local_34c = *(float*)&__mfr[0x20];
  float &local_348 = *(float*)&__mfr[0x24];
  float &local_344 = *(float*)&__mfr[0x28];
  float &local_340 = *(float*)&__mfr[0x2c];
  float &local_33c = *(float*)&__mfr[0x30];
  float &local_338 = *(float*)&__mfr[0x34];
  float &local_334 = *(float*)&__mfr[0x38];
  float &local_330 = *(float*)&__mfr[0x3c];
  float &local_32c = *(float*)&__mfr[0x40];
  float &local_328 = *(float*)&__mfr[0x44];
  float &local_324 = *(float*)&__mfr[0x48];
  float &local_320 = *(float*)&__mfr[0x4c];
  float &local_31c = *(float*)&__mfr[0x50];
  undefined4 &local_318 = *(undefined4*)&__mfr[0x54];
  undefined4 &local_314 = *(undefined4*)&__mfr[0x58];
  float &local_310 = *(float*)&__mfr[0x5c];
  float &local_30c = *(float*)&__mfr[0x60];
  undefined4 &local_308 = *(undefined4*)&__mfr[0x64];
  float &local_304 = *(float*)&__mfr[0x68];
  float &local_300 = *(float*)&__mfr[0x6c];
  float &local_2fc = *(float*)&__mfr[0x70];
  float &local_2f8 = *(float*)&__mfr[0x74];
  float &local_2f4 = *(float*)&__mfr[0x78];
  float &local_2f0 = *(float*)&__mfr[0x7c];
  float &local_2ec = *(float*)&__mfr[0x80];
  float &local_2e8 = *(float*)&__mfr[0x84];
  float &local_2e4 = *(float*)&__mfr[0x88];
  float &local_2e0 = *(float*)&__mfr[0x8c];
  float &local_2dc = *(float*)&__mfr[0x90];
  float &local_2d8 = *(float*)&__mfr[0x94];
  float &local_2d4 = *(float*)&__mfr[0x98];
  float &local_2d0 = *(float*)&__mfr[0x9c];
  float &local_2cc = *(float*)&__mfr[0xa0];
  float &local_2c8 = *(float*)&__mfr[0xa4];
  float &local_2c4 = *(float*)&__mfr[0xa8];
  float &local_2c0 = *(float*)&__mfr[0xac];
  float &local_2bc = *(float*)&__mfr[0xb0];
  float &local_2b8 = *(float*)&__mfr[0xb4];
  float &local_2b4 = *(float*)&__mfr[0xb8];
  float &local_2b0 = *(float*)&__mfr[0xbc];
  float &local_2ac = *(float*)&__mfr[0xc0];
  float &local_2a8 = *(float*)&__mfr[0xc4];
  float &local_2a4 = *(float*)&__mfr[0xc8];
  float &local_2a0 = *(float*)&__mfr[0xcc];
  float &local_29c = *(float*)&__mfr[0xd0];
  float &local_298 = *(float*)&__mfr[0xd4];
  float &local_294 = *(float*)&__mfr[0xd8];
  float &local_290 = *(float*)&__mfr[0xdc];
  float &local_28c = *(float*)&__mfr[0xe0];
  uint &local_288 = *(uint*)&__mfr[0xe4];
  float &local_284 = *(float*)&__mfr[0xe8];
  float &local_280 = *(float*)&__mfr[0xec];
  float &local_27c = *(float*)&__mfr[0xf0];
  float &local_278 = *(float*)&__mfr[0xf4];
  undefined4 &local_274 = *(undefined4*)&__mfr[0xf8];
  float &local_270 = *(float*)&__mfr[0xfc];
  undefined4 &local_26c = *(undefined4*)&__mfr[0x100];
  undefined4 &local_268 = *(undefined4*)&__mfr[0x104];
  undefined4 &local_264 = *(undefined4*)&__mfr[0x108];
  float &local_260 = *(float*)&__mfr[0x10c];
  float &local_25c = *(float*)&__mfr[0x110];
  float &local_258 = *(float*)&__mfr[0x114];
  float &local_254 = *(float*)&__mfr[0x118];
  undefined4 &local_250 = *(undefined4*)&__mfr[0x11c];
  undefined4 &local_24c = *(undefined4*)&__mfr[0x120];
  float &local_248 = *(float*)&__mfr[0x124];
  float &local_244 = *(float*)&__mfr[0x128];
  float &local_240 = *(float*)&__mfr[0x12c];
  float &local_23c = *(float*)&__mfr[0x130];
  float &local_238 = *(float*)&__mfr[0x134];
  float &local_234 = *(float*)&__mfr[0x138];
  float &local_230 = *(float*)&__mfr[0x13c];
  float &local_22c = *(float*)&__mfr[0x140];
  undefined4 &local_228 = *(undefined4*)&__mfr[0x144];
  float &local_224 = *(float*)&__mfr[0x148];
  float &local_220 = *(float*)&__mfr[0x14c];
  float &local_21c = *(float*)&__mfr[0x150];
  float &local_218 = *(float*)&__mfr[0x154];
  float &local_214 = *(float*)&__mfr[0x158];
  undefined4 &local_210 = *(undefined4*)&__mfr[0x15c];
  float &local_20c = *(float*)&__mfr[0x160];
  undefined4 &local_208 = *(undefined4*)&__mfr[0x164];
  undefined4 &local_204 = *(undefined4*)&__mfr[0x168];
  float &local_200 = *(float*)&__mfr[0x16c];
  float &local_1fc = *(float*)&__mfr[0x170];
  undefined4 &local_1f8 = *(undefined4*)&__mfr[0x174];
  float &local_1f4 = *(float*)&__mfr[0x178];
  float &local_1f0 = *(float*)&__mfr[0x17c];
  float &local_1ec = *(float*)&__mfr[0x180];
  float &local_1e8 = *(float*)&__mfr[0x184];
  undefined4 &local_1e4 = *(undefined4*)&__mfr[0x188];
  float &local_1e0 = *(float*)&__mfr[0x18c];
  float &local_1dc = *(float*)&__mfr[0x190];
  undefined4 &local_1d8 = *(undefined4*)&__mfr[0x194];
  float &local_1d4 = *(float*)&__mfr[0x198];
  float &local_1d0 = *(float*)&__mfr[0x19c];
  float &local_1cc = *(float*)&__mfr[0x1a0];
  float &local_1c8 = *(float*)&__mfr[0x1a4];
  float &local_1c4 = *(float*)&__mfr[0x1a8];
  undefined4 &local_1c0 = *(undefined4*)&__mfr[0x1ac];
  float &local_1bc = *(float*)&__mfr[0x1b0];
  float &local_1b8 = *(float*)&__mfr[0x1b4];
  undefined4 &local_1b4 = *(undefined4*)&__mfr[0x1b8];
  undefined4 &local_1b0 = *(undefined4*)&__mfr[0x1bc];
  undefined4 &local_1ac = *(undefined4*)&__mfr[0x1c0];
  undefined4 &local_1a8 = *(undefined4*)&__mfr[0x1c4];
  float (&local_1a4)[18] = *(float(*)[18])&__mfr[0x1c8];
  float (&local_15c)[12] = *(float(*)[12])&__mfr[0x210];
  float (&local_12c)[12] = *(float(*)[12])&__mfr[0x240];
  float (&local_fc)[12] = *(float(*)[12])&__mfr[0x270];
  float (&local_cc)[12] = *(float(*)[12])&__mfr[0x2a0];
  float (&local_9c)[12] = *(float(*)[12])&__mfr[0x2d0];
  float (&local_6c)[13] = *(float(*)[13])&__mfr[0x300];
  float (&local_38)[14] = *(float(*)[14])&__mfr[0x334];
  do {
    uVar8 = _rand() % 4;
    local_35c = (float *)(uVar8 + 7);
    // El counter (+96) es un DWORD. IDA: `v4 = *(int *)(o + 96)` y compara `v4`;
    // `v356 = *(float *)&v4` es solo la copia de los BITS.
    const int __cnt = AsObject(param_1)->LifeTime;
    fVar13 = *(float *)&AsObject(param_1)->LifeTime;
    local_36c = (float)(int)local_35c * _DAT_005524f4;
    if (__cnt < 5) {
      local_35c = (float *)(5 - __cnt);
      local_36c = local_36c - (float)(int)local_35c * _DAT_005526e4;
      if (local_36c < _DAT_00552580) {
        local_36c = 0.0;
      }
    }
    sVar3 = AsObject(param_1)->Type;
    local_368 = 1.0;
    iVar9 = (int)sVar3;
    local_364 = 1.0;
    local_360 = 1.0;
    local_358 = fVar13;
    if (0x1f0 < iVar9) {
      if (iVar9 < 0x4b1) {
        if (iVar9 == 0x4b0) {
          uVar8 = _rand() % 8;
          bVar16 = uVar8 == 0;
          if (bVar16) {
            bVar28 = 0;
            pfVar25 = (float *)0x0;
            pfVar23 = (float *)0xffffffff;
            pfVar21 = (float *)0x0;
            pfVar14 = param_1 + 0x3a;
            pfVar22 = (float *)0x0;
            pfVar10 = param_1 + 7;
            pfVar15 = param_1 + 4;
            uVar8 = _rand() % 2;
            CreateEffect(uVar8 + 0xc5,pfVar15,pfVar10,pfVar14,pfVar22,pfVar21,pfVar23,pfVar25,bVar28);
          }
          pfVar10 = param_1 + 4;
          iVar9 = 6;
          do {
            iVar12 = _rand();
            local_354 = (float)(iVar12 % 0x32 + -0x19);
            iVar12 = _rand();
            local_34c = AsObject(param_1)->Position[2];
            local_354 = local_354 + *pfVar10;
            local_350 = (float)(iVar12 % 0x32 + -0x19) + AsObject(param_1)->Position[1];
            Particle_Spawn(0x4b0,&local_354,AsObject(param_1)->Angle,&local_368,0,1.0,0);
            iVar9 = iVar9 + -1;
          } while (iVar9 != 0);
          local_364 = local_36c * _DAT_005528b4;
          local_368 = local_36c;
          local_360 = 0.0;
          AddTerrainLight(*pfVar10,AsObject(param_1)->Position[1], (float*)&local_368,3, (float*)DAT_081cb608);
          if (((char *)*(int *)&AsObject(param_1)->Owner == DAT_07abf5d8) && ((AsObject(param_1)->LifeTime) % 0x14 == 0)) {
            Entity_FindNearby_SendPacket((uint)AsObject(param_1)->Skill,pfVar10,150.0,
                         *(byte *)(param_1 + 0x22),AsObject(param_1)->PKKey);
          }
        }
        else if (iVar9 < 0x499) {
          if (iVar9 == 0x498) {
            local_294 = AsObject(param_1)->Position[0];
            local_290 = AsObject(param_1)->Position[1];
            local_28c = AsObject(param_1)->Position[2];
            local_368 = (float)__cnt * _DAT_005524f4;
            iVar9 = 0;
            local_364 = local_368;
            local_360 = local_368;
            do {
              local_28c = local_28c + _DAT_005529a8;
              if (iVar9 == 0) {
                fVar13 = 12.0;
              }
              else {
                fVar13 = 6.0;
              }
              Particle_Spawn(0x498,&local_294,AsObject(param_1)->Angle,&local_368,1,fVar13,0);
              iVar9 = iVar9 + 1;
            } while (iVar9 < 0x12);
          }
          else if (iVar9 < 0x47f) {
            if (iVar9 == 0x47e) {
              if (*(float *)&AsObject(param_1)->SubType == 0.0) {
                AsObject(param_1)->Position[0] = AsObject(param_1)->Direction[0] + AsObject(param_1)->Position[0];
                AsObject(param_1)->Position[1] = AsObject(param_1)->Direction[1] + AsObject(param_1)->Position[1];
                AsObject(param_1)->Position[2] = AsObject(param_1)->Direction[2] + AsObject(param_1)->Position[2];
                AsObject(param_1)->Direction[2] = AsObject(param_1)->Direction[2] - _DAT_005524f8;
                Particle_Spawn(0x47e,AsObject(param_1)->Position,AsObject(param_1)->Angle,AsObject(param_1)->Light,1,AsObject(param_1)->Scale,0);
                if (AsObject(param_1)->Direction[2] < _DAT_005529ac) goto LAB_0046a366;
              }
              else if (AsObject(param_1)->SubType == 1) {
                pcVar11 = (char *)*(int *)&AsObject(param_1)->Owner;
                if (((pcVar11 == (char *)0x0) || (*pcVar11 != '\x01')) ||
                   (((byte)*(undefined4 *)(pcVar11 + 0x78) & 0x10) != 0x10)) {
                  *(float *)&AsObject(param_1)->LifeTime = 0.0;
                }
                else {
                  *(float *)&AsObject(param_1)->LifeTime = 1.4013e-44;
                }
                local_368 = 1.0;
                local_364 = 0.5;
                local_360 = 0.1;
                iVar9 = _rand();
                Particle_Spawn(0x47e,AsObject(param_1)->Position,&local_334,&local_368,4,
                             (float)(byte)(DAT_00559b78)[iVar9 % 7],*(int *)&AsObject(param_1)->Owner);
                Particle_Spawn(0x47e,AsObject(param_1)->Position,&local_334,&local_368,4,
                             (float)(byte)(DAT_00559b7f)[-(iVar9 % 7)],*(int *)&AsObject(param_1)->Owner);
              }
            }
            else if (iVar9 == 0x1f1) {
              local_254 = 0.0;
              local_250 = 0xc2c80000;
              local_24c = 0;
              iVar9 = 3;
              do {
                iVar12 = _rand();
                local_274 = 0;
                local_278 = (float)(iVar12 % 0x5a);
                iVar12 = _rand();
                local_270 = (float)(iVar12 % 0x168);
                Matrix_BuildFromEuler(&local_278,local_fc);
                Vector_Rotate(&local_254,local_fc,&local_2a0);
                local_2a0 = AsObject(param_1)->Position[0] - local_2a0;
                local_29c = AsObject(param_1)->Position[1] - local_29c;
                local_298 = AsObject(param_1)->Position[2] - local_298;
                Joint_Create(0x4eb,&local_2a0,AsObject(param_1)->Position,&local_278,6,0,5.0,-1,0);
                iVar9 = iVar9 + -1;
              } while (iVar9 != 0);
            }
            else if (iVar9 == 0x238) {
              pfVar10 = param_1 + 4;
              fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
              if ((float10)AsObject(param_1)->Position[2] < fVar17) {
                local_354 = *pfVar10;
                local_350 = AsObject(param_1)->Position[1];
                local_34c = AsObject(param_1)->Position[2] + _DAT_00552878;
                Particle_Spawn(0x4bf,&local_354,AsObject(param_1)->Angle,&local_368,0,1.0,0);
                iVar9 = 6;
                do {
                  bVar28 = 0;
                  pfVar27 = (float *)0x0;
                  pfVar25 = (float *)0xffffffff;
                  pfVar23 = (float *)0x0;
                  pfVar15 = param_1 + 0x3a;
                  pfVar21 = (float *)0x0;
                  pfVar14 = pfVar10;
                  pfVar22 = param_1 + 7;
                  uVar8 = _rand() % 2;
                  CreateEffect(uVar8 + 0xc5,pfVar14,pfVar22,pfVar15,pfVar21,pfVar23,pfVar25,pfVar27,bVar28);
                  iVar9 = iVar9 + -1;
                } while (iVar9 != 0);
                *(undefined1 *)param_1 = 0;
              }
              local_368 = local_36c * _DAT_005526e4;
              local_360 = local_36c;
              local_364 = local_36c * _DAT_005528b4;
              AddTerrainLight(*pfVar10,AsObject(param_1)->Position[1], (float*)&local_368,2, (float*)DAT_081cb608);
            }
          }
          else if ((iVar9 == 0x490) && (*(int*)&fVar13 == 30)) {
            local_354 = AsObject(param_1)->Position[0];
            local_350 = AsObject(param_1)->Position[1];
            local_34c = AsObject(param_1)->Position[2] + _DAT_005524f0;
            Joint_Create(0x4ee,&local_354,&local_354,AsObject(param_1)->HeadAngle,0,*(int *)&AsObject(param_1)->Owner,150.0,
                         AsObject(param_1)->PKKey,AsObject(param_1)->Skill);
          }
        }
        else {
          switch(iVar9) {
          case 0x49c:
            fVar13 = (float)__cnt * _DAT_005526e4;
            fVar4 = fVar13;
            fVar5 = fVar13;
            fVar6 = fVar13;
            if ((*(float *)&AsObject(param_1)->SubType != 0.0) &&
               (fVar4 = local_368, fVar5 = local_364, fVar6 = local_360, AsObject(param_1)->SubType == 1))
            {
              local_368 = 1.0;
              local_364 = 0.8;
              local_360 = 0.2;
              fVar4 = local_368;
              fVar5 = local_364;
              fVar6 = local_360;
            }
            local_360 = fVar6;
            local_364 = fVar5;
            local_368 = fVar4;
            pfVar10 = param_1 + 4;
            Particle_Spawn(0x49c,pfVar10,AsObject(param_1)->Angle,&local_368,0,1.0,0);
            Particle_Spawn(0x498,pfVar10,AsObject(param_1)->Angle,&local_368,0,4.0,0);
            local_368 = fVar13 * _DAT_005526e4;
            local_364 = fVar13 * _DAT_005528b4;
            local_360 = fVar13;
            AddTerrainLight(*pfVar10,AsObject(param_1)->Position[1], (float*)&local_368,2, (float*)DAT_081cb608);
            Effect_SpawnProximityHit((int)param_1);
            break;
          case 0x4a7:
            local_360 = (float)__cnt * _DAT_005526e4;
            local_368 = local_360 * _DAT_005526e4;
            local_364 = local_360 * _DAT_00552504;
            goto LAB_004695b5;
          case 0x4ab:
            Particle_Spawn(0x4ab,AsObject(param_1)->Position,AsObject(param_1)->Angle,&local_368,9,1.0,*(int *)&AsObject(param_1)->Owner);
            break;
          case 0x4ac:
            local_368 = 1.0;
            local_364 = 1.0;
            local_360 = 1.0;
            Particle_Spawn(0x4ac,AsObject(param_1)->Position,AsObject(param_1)->Angle,&local_368,1,1.0,0);
            local_364 = local_36c * _DAT_00552534;
            local_368 = local_36c;
            local_360 = local_36c * _DAT_005528b8;
            AddTerrainLight(AsObject(param_1)->Position[0],AsObject(param_1)->Position[1], (float*)&local_368,2, (float*)DAT_081cb608);
          }
        }
      }
// SECTION_2
      else {
        switch(iVar9) {
        case 0x4df:
          if (*(int*)&fVar13 == 1) {
            iVar9 = _rand();
            iVar12 = _rand();
            local_368 = (float)(iVar12 % 3) * _DAT_005528b8 + _DAT_005528b4;
            uVar8 = _rand() % 4;
            local_360 = 0.0;
            iVar12 = 0x50;
            local_364 = (float)(int)uVar8 * _DAT_005524f4;
            do {
              Particle_Spawn(0x4df,AsObject(param_1)->Position,AsObject(param_1)->Angle,&local_368,iVar9 % 0x1e,1.0,0);
              iVar12 = iVar12 + -1;
            } while (iVar12 != 0);
            PlayBuffer(0x45,(unsigned int)(uintptr_t)param_1,0);
          }
          Particle_Spawn(0x4df,AsObject(param_1)->Position,AsObject(param_1)->Angle,AsObject(param_1)->Light,-1,1.0,0);
          local_368 = local_36c * _DAT_005528b4;
          local_364 = local_36c * _DAT_005528b8;
          local_360 = local_36c * _DAT_005526e4;
          AddTerrainLight(AsObject(param_1)->Position[0],AsObject(param_1)->Position[1], (float*)&local_368,1, (float*)DAT_081cb608);
          break;
        case 0x4e0:
          if ((__cnt % 5 == 0) && (iVar9 = _rand(), iVar9 % 3 == 0)) {
            CreateEffect(0x4df,AsObject(param_1)->Position,AsObject(param_1)->Angle,AsObject(param_1)->Light,(float *)0x0,(float *)0x0,
                         (float *)0xffffffff,(float *)0x0,0);
          }
          break;
        case 0x4f0:
        case 0x4f1:
          if ((0 < AsObject(param_1)->SubType) && (AsObject(param_1)->SubType != 4)) {
            Effect_SpawnLightningBurst((int)param_1);
          }
          break;
        case 0x4f3:
          if (AsObject(param_1)->SubType == 1) {
            local_31c = 0.0;
            local_318 = 0;
            local_314 = 0x41200000;
            if (!validOwner(owner)) return;
            iVar9 = *(int *)(*(int *)&AsObject(param_1)->Owner + 0x114);
            if (!SceneObjects::HasBoneRange(owner, (void*)(uintptr_t)iVar9, 0x630, 12)) {
              *(char*)param_1 = 0; return;
            }
            const int ownerModel = *(const short*)((const char*)owner + 2);
            if (!DAT_05828d58 || ownerModel < 0 || ownerModel >= MODEL_MAX_TOTAL) {
              *(char*)param_1 = 0; return;
            }
            pvVar1 = (void *)(DAT_05828d58 + ownerModel * 0xbc);
            *(float *)((int)pvVar1 + 0x6c) = AsObject(param_1)->StartPosition[0];
            *(float *)((int)pvVar1 + 0x70) = AsObject(param_1)->StartPosition[1];
            *(float *)((int)pvVar1 + 0x74) = AsObject(param_1)->StartPosition[2];
            BMD_TransformPosition(pvVar1,(float *)(iVar9 + 0x630),&local_31c,AsObject(param_1)->Position,'\x01');
          }
          pfVar10 = param_1 + 4;
          iVar9 = 3;
          do {
            local_31c = 0.0;
            local_318 = 0x42f00000;
            local_314 = 0;
            iVar12 = _rand();
            local_330 = 0.0;
            local_334 = (float)(iVar12 % 0x168);
            iVar12 = _rand();
            local_32c = (float)(iVar12 % 0x168);
            Matrix_BuildFromEuler(&local_334,local_1a4 + 6);
            Vector_Rotate(&local_31c,local_1a4 + 6,&local_354);
            local_354 = local_354 + *pfVar10;
            local_350 = local_350 + AsObject(param_1)->Position[1];
            local_34c = local_34c + AsObject(param_1)->Position[2];
            if (AsObject(param_1)->SubType == 1) {
              uVar8 = (*(uint *)&AsObject(param_1)->LifeTime) & 0x80000001;
              bVar16 = uVar8 == 0;
              if ((int)uVar8 < 0) {
                bVar16 = (uVar8 - 1 | 0xfffffffe) == 0xffffffff;
              }
              if (bVar16) {
                Joint_Create(0x4e6,&local_354,pfVar10,&local_334,3,0,10.0,10,10);
              }
              else {
                pfVar15 = param_1;
                iVar12 = _rand();
                Particle_Spawn(0x498,&local_354,&local_334,&local_368,2,
                             (float)(iVar12 % 0x32 + 10) * _DAT_00552594,(int)pfVar15);
              }
              uVar29 = 0;
              iVar12 = _rand();
              pfVar15 = param_1 + 0x3a;
              fVar13 = (float)(iVar12 % 0x168);
              pfVar14 = param_1;
              uVar8 = _rand() % 8;
              CreateSprite(0x4cf,pfVar10,(float)(int)(uVar8 + 8) * _DAT_005526e4,pfVar15,(int)pfVar14,fVar13,uVar29);
            }
            else {
              uVar29 = 0;
              iVar12 = _rand();
              pfVar15 = param_1 + 0x3a;
              fVar13 = (float)(iVar12 % 0x168);
              pfVar14 = param_1;
              uVar8 = _rand() % 8;
              CreateSprite(0x4cf,pfVar10,(float)(int)(uVar8 + 8) * _DAT_005528b8,pfVar15,(int)pfVar14,fVar13,uVar29);
              pfVar15 = param_1;
              iVar12 = _rand();
              Particle_Spawn(0x498,&local_354,&local_334,&local_368,2,
                           (float)(iVar12 % 0x32 + 10) * _DAT_00552594,(int)pfVar15);
            }
            iVar9 = iVar9 + -1;
          } while (iVar9 != 0);
          break;
        case 0x4f7:
          if (__cnt < 0x10) {
            iVar9 = _rand();
            pfVar10 = param_1 + 4;
            fVar17 = (float10)fsin((float10)(iVar9 % 1000) * (float10)_DAT_005524f8);
            *pfVar10 = (float)(fVar17 * (float10)_DAT_00552488 + (float10)AsObject(param_1)->StartPosition[0]);
            iVar9 = _rand();
            fVar17 = (float10)fsin((float10)(iVar9 % 1000) * (float10)_DAT_005524f8);
            AsObject(param_1)->Position[1] = (float)(fVar17 * (float10)_DAT_00552488 + (float10)AsObject(param_1)->StartPosition[1]);
            AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] + AsObject(param_1)->Gravity;
            AsObject(param_1)->Gravity = AsObject(param_1)->Gravity - _DAT_0055264c;
            AsObject(param_1)->StartPosition[0] = AsObject(param_1)->StartPosition[0] - _DAT_00552488;
            Particle_Spawn(0x4ad,pfVar10,AsObject(param_1)->Angle,&local_368,7,1.0,0);
            fVar13 = AsObject(param_1)->Light[0] + _DAT_005524f4;
            pfVar15 = param_1 + 0x3a;
            uVar29 = 0;
            *pfVar15 = fVar13;
            AsObject(param_1)->Light[1] = fVar13;
            AsObject(param_1)->Light[2] = fVar13;
            iVar9 = _rand();
            fVar13 = (float)(iVar9 % 0x168);
            pfVar14 = pfVar15;
            pfVar22 = param_1;
            uVar8 = _rand() % 4;
            CreateSprite(0x4cf,pfVar10,(float)(int)(uVar8 + 4) * _DAT_005526e4,pfVar14,(int)pfVar22,fVar13,uVar29);
            uVar29 = 0;
            iVar9 = _rand();
            CreateSprite(0x47e,pfVar10,1.0f,pfVar15,(int)param_1,(float)(iVar9 % 0x168),uVar29);
            if ((AsObject(param_1)->SubType == 1) &&
               (fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]), (float10)AsObject(param_1)->Position[2] < fVar17)) {
              local_354 = *pfVar10;
              local_350 = AsObject(param_1)->Position[1];
              local_34c = AsObject(param_1)->Position[2] + _DAT_00552598;
              iVar9 = 0;
              local_368 = 1.0;
              local_364 = 1.0;
              local_360 = 1.0;
              uVar8 = _rand() % 32;
              Particle_Spawn(0x4c4,&local_354,AsObject(param_1)->Angle,&local_368,0xb,
                           (float)(int)(uVar8 + 0x50) * _DAT_005529cc,iVar9);
            }
          }
          break;
        case 0x4fa:
          if (!validOwner(owner)) return;
          if (*(char *)*(int *)&AsObject(param_1)->Owner == '\0') {
            *(float *)&AsObject(param_1)->LifeTime = 0.0;
            goto LAB_0046a85e;
          }
          switch(AsObject(param_1)->SubType) {
          case 0:   // 0.0f
            if ((*(byte*)&param_1[0x1e] & 0x40) == 0x40) {
              *(float *)&AsObject(param_1)->LifeTime = 1.4013e-44f;
            }
          case 1:   // 1.4013e-45f
          case 3:   // 4.2039e-45f
            local_35c = (float *)0x0;
            do {
              pfVar10 = local_35c;
              fVar13 = *(float *)&AsObject(param_1)->SubType;
              fVar17 = (float10)(int)local_35c * (float10)_DAT_005529a0 +
                       (float10)(AsObject(param_1)->LifeTime) * (float10)_DAT_005529a4;
              if (fVar13 == 4.2039e-45f) {
                fVar17 = -fVar17;
              }
              if (!validOwner(owner)) return;
              __owner_fVar4 = *(int *)&AsObject(param_1)->Owner;   // ver nota de __owner_* arriba
              fVar18 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_00552998 +
                                     (float10)(int)local_35c * (float10)_DAT_0055299c);
              local_35c = (float *)(int)(float)(fVar18 * (float10)_DAT_005524fc + (float10)_DAT_00552598);
              fVar18 = (float10)fsin(fVar17);
              local_260 = (float)(fVar18 * (float10)(int)local_35c +
                                 (float10)*(float *)(__owner_fVar4 + 0x10));
              fVar17 = (float10)fcos(fVar17);
              local_25c = (float)(fVar17 * (float10)(int)local_35c +
                                 (float10)*(float *)(__owner_fVar4 + 0x14));
              fVar5 = _DAT_0055285c;
              if (fVar13 == 4.2039e-45f) {
                fVar5 = _DAT_00552994;
              }
              local_258 = fVar5 * *(float *)(__owner_fVar4 + 0xc) + *(float *)(__owner_fVar4 + 0x18);
              local_1a4[0] = 0.65f;
              local_1a4[1] = 0.65f;
              local_1a4[2] = 0.65f;
              local_1a4[3] = 0.1f;
              local_1a4[4] = 1.0f;
              local_1a4[5] = 1.0f;
              Particle_Spawn(0x4b0,&local_260,AsObject(param_1)->Angle,local_1a4 + (uint)(fVar13 == 4.2039e-45f) * 3,
                           5,0.4f,0);
              CreateSprite(0x4fa,&local_260,1.0f,&local_368,*(int *)&AsObject(param_1)->Owner,0,0);
              local_35c = (float *)((int)pfVar10 + 1);
            } while ((int)local_35c < 3);
            break;
          case 2:   // 2.8026e-45f
            local_1f4 = (float)__cnt * _DAT_005524f4;
            local_1f0 = local_1f4;
            local_1ec = local_1f4;
            CreateSprite(0x4fa,AsObject(param_1)->Position,1.5f,&local_1f4,*(int *)&AsObject(param_1)->Owner,0,0);
          }
          break;
        case 0x566:
          local_368 = local_36c * _DAT_00552504;
          pfVar10 = param_1 + 4;
          local_360 = local_36c * _DAT_00552530;
          local_364 = local_368;
          AddTerrainLight(AsObject(param_1)->Position[0],AsObject(param_1)->Position[1], (float*)&local_368,2, (float*)DAT_081cb608);
          local_334 = AsObject(param_1)->Angle[0];
          local_330 = AsObject(param_1)->Angle[1];
          local_32c = AsObject(param_1)->Angle[2] + _DAT_00552848;
          local_368 = 1.0;
          local_364 = 1.0;
          local_360 = 1.0;
          iVar9 = _rand();
          AsObject(param_1)->Scale = (float)(0xf - (AsObject(param_1)->LifeTime)) * _DAT_00552a10 * (float)(iVar9 % 3 + 2);
          Matrix_BuildFromEuler(&local_334,local_1a4 + 6);
          local_31c = 0.0;
          local_318 = 0x41a00000;
          local_314 = 0;
          Vector_Rotate(&local_31c,local_1a4 + 6,&local_354);
          pfVar15 = param_1 + 0x5c;
          *pfVar15 = local_354 + *pfVar10;
          AsObject(param_1)->StartPosition[1] = local_350 + AsObject(param_1)->Position[1];
          AsObject(param_1)->StartPosition[2] = local_34c + AsObject(param_1)->Position[2];
          Particle_Spawn(0x4ad,pfVar15,AsObject(param_1)->Angle,&local_368,10,AsObject(param_1)->Scale,0);
          local_31c = 0.0;
          local_318 = 0xc1a00000;
          local_314 = 0;
          Vector_Rotate(&local_31c,local_1a4 + 6,&local_354);
          *pfVar15 = local_354 + *pfVar10;
          AsObject(param_1)->StartPosition[1] = local_350 + AsObject(param_1)->Position[1];
          AsObject(param_1)->StartPosition[2] = local_34c + AsObject(param_1)->Position[2];
          Particle_Spawn(0x4ad,pfVar15,AsObject(param_1)->Angle,&local_368,10,AsObject(param_1)->Scale,0);
          if (AsObject(param_1)->LifeTime == 1) {
            Effect_SpawnSmokeExplosion((undefined4 *)pfVar10,'\x01');
          }
          break;
        case 0x596:
          if (!validOwner(owner)) return;
          pcVar11 = (char *)*(int *)&AsObject(param_1)->Owner;
          if ((*pcVar11 == '\0') || (((byte)*(undefined4 *)(pcVar11 + 0x78) & 2) != 2)) {
LAB_0046a366:
            *(float *)&AsObject(param_1)->LifeTime = 0.0;
          }
          else if (__cnt < 0x14) {
            *(float *)&AsObject(param_1)->LifeTime = 4.2039e-44;
          }
          else {
            fVar4 = *(float *)(pcVar11 + 0x15c);
            fVar5 = AsObject(pcVar11)->Position[2];
            AsObject(param_1)->Position[0] = AsObject(pcVar11)->Position[0];
            AsObject(param_1)->Position[1] = AsObject(pcVar11)->Position[1];
            AsObject(param_1)->Position[2] = fVar4 + fVar5 + _DAT_00552878;
            Particle_Spawn(0x596,AsObject(param_1)->Position,AsObject(param_1)->Angle,AsObject(param_1)->Light,(0x1e - __cnt) / 2,1.0,0);
          }
        }
      }
      goto switchD_00466b93_caseD_c1;
    }
    if (iVar9 == 0x1f0) {
      if (AsObject(param_1)->SubType == 1) {
        Joint_Create(0x4e1,AsObject(param_1)->Position,AsObject(param_1)->Position,AsObject(param_1)->Angle,0xc,(int)param_1,100.0,-1,0);
      }
      else if (*(float *)&AsObject(param_1)->SubType == 0.0) {
        Joint_Create(0x4e1,AsObject(param_1)->Position,AsObject(param_1)->Position,AsObject(param_1)->Angle,4,(int)param_1,50.0,-1,0);
      }
      goto switchD_00466b93_caseD_c1;
    }
// SECTION_3
    switch(iVar9) {
    case 0xc1:                                  // Tamachan (0.98j case 183)
      Tamachan_Move((char *)param_1, __cnt);
      break;
    case 0xbe:
      fVar13 = *(float *)&AsObject(param_1)->SubType;
      if (fVar13 == 0.0) {
        if (_DAT_00552660 <= AsObject(param_1)->AnimationFrame) {
          AsObject(param_1)->Velocity = 0.0;
          uVar8 = _rand() % 2;
          bVar16 = uVar8 == 0;
          if (bVar16) {
            uVar8 = _rand() % 64;
            local_1d0 = (float)(int)(uVar8 - 0x20) + AsObject(param_1)->Position[0];
            uVar8 = _rand() % 64;
            local_1cc = (float)(int)(uVar8 - 0x20) + AsObject(param_1)->Position[1];
            uVar8 = _rand() % 128;
            local_1c8 = (float)(int)(uVar8 + 0x20) + AsObject(param_1)->Position[2];
            Particle_Spawn(0x4c4,&local_1d0,AsObject(param_1)->Angle,AsObject(param_1)->Light,0,1.0,0);
          }
          fVar13 = AsObject(param_1)->Alpha - _DAT_00552874;
          AsObject(param_1)->Alpha = fVar13;
          if (fVar13 < _DAT_00552580) {
            *(undefined1 *)param_1 = 0;
          }
        }
        local_368 = local_36c * _DAT_00552990;
        local_364 = local_36c * _DAT_005529c0;
        local_360 = local_36c * _DAT_00552570;
        goto LAB_004695b5;
      }
      if (((*(int*)&fVar13) < 1) || (2 < (*(int*)&fVar13))) break;
      pcVar11 = (char *)*(int *)&AsObject(param_1)->Owner;
      if (pcVar11 == (char *)0x0) goto LAB_0046a85e;
      if ((*pcVar11 == '\0') || (((byte)*(undefined4 *)(pcVar11 + 0x78) & 0x20) != 0x20)) {
        AsObject(param_1)->Alpha = AsObject(param_1)->Alpha - _DAT_00552874;
      }
      else {
        bVar28 = *(char *)&AsObject(pcVar11)->CurrentAction;
        if (AsObject(pcVar11)->Type == 0x186) {
          if (((0xc < bVar28) && (bVar28 < 0x22)) || ((0x4b < bVar28 && (bVar28 < 0x4e)))) {
            AsObject(param_1)->AnimationFrame = 0.0;
            AsObject(param_1)->PriorAnimationFrame = 0.0;
            goto LAB_00469366;
          }
LAB_0046934f:
          AsObject(param_1)->HiddenMesh = -1;   // IDA: HiddenMesh = -1 (ninguna)
        }
        else {
          if (bVar28 != 2) goto LAB_0046934f;
          AsObject(param_1)->PriorAnimationFrame = 0.0;
          AsObject(param_1)->AnimationFrame = 0.0;
LAB_00469366:
          AsObject(param_1)->HiddenMesh = -2;   // IDA LABEL_246: HiddenMesh = -2 (oculta el hielo)
        }
        bVar16 = _DAT_00552650 <= AsObject(param_1)->AnimationFrame;
        AsObject(param_1)->Position[0] = AsObject(pcVar11)->Position[0];
        fVar4 = AsObject(pcVar11)->Position[2];
        AsObject(param_1)->Position[1] = AsObject(pcVar11)->Position[1];
        AsObject(param_1)->Position[2] = fVar4;
        if (bVar16) {
          AsObject(param_1)->AnimationFrame = 4.0;
        }
        *(float *)&AsObject(param_1)->LifeTime = 1.4013e-44;
      }
      if (*(int*)&fVar13 == 1) {
        local_31c = 60.0;
        local_318 = 0;
        AsObject(param_1)->HeadAngle[2] = AsObject(param_1)->HeadAngle[2] + _DAT_005524fc;
        local_314 = 0;
        fVar17 = (float10)fsin((float10)DAT_05826e08 * (float10)_DAT_005524f8);
        AsObject(param_1)->Gravity = (float)(fVar17 * (float10)_DAT_005524fc + (float10)_DAT_0055284c);
        Matrix_BuildFromEuler(AsObject(param_1)->HeadAngle,local_1a4 + 6);
        Vector_Rotate(&local_31c,local_1a4 + 6,&local_354);
        if (!validOwner(owner)) return;
        __owner_fVar13 = *(int *)&AsObject(param_1)->Owner;   // ver nota de __owner_* arriba
        local_354 = local_354 + *(float *)(__owner_fVar13 + 0x10);
        local_350 = local_350 + *(float *)(__owner_fVar13 + 0x14);
        local_368 = 1.0;
        local_34c = local_34c + *(float *)(__owner_fVar13 + 0x18) + AsObject(param_1)->Gravity;
        local_364 = 1.0;
        local_360 = 1.0;
        Particle_Spawn(0x4ad,&local_354,AsObject(param_1)->Angle,&local_368,10,2.0,0);
      }
      break;
    case 0xbf:
      fVar13 = *(float *)&AsObject(param_1)->SubType;
      if ((fVar13 != 0.0) && (*(int*)&fVar13 != 2)) {
        if (*(int*)&fVar13 == 4) goto LAB_00469772;
        if (*(int*)&fVar13 == 6) goto LAB_00469769;
        if (*(int*)&fVar13 == 5) {
          AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] + AsObject(param_1)->Gravity;
          AsObject(param_1)->Gravity = AsObject(param_1)->Gravity - _DAT_0055264c;
          fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
          local_348 = (float)fVar17;
          if (AsObject(param_1)->Position[2] < local_348) {
            fVar13 = AsObject(param_1)->Position[2] + _DAT_00552488;
            AsObject(param_1)->Gravity = 10.0;
            AsObject(param_1)->Position[2] = fVar13;
            AsObject(param_1)->Direction[1] = AsObject(param_1)->Direction[1] * _DAT_00552504;
            AsObject(param_1)->Scale = AsObject(param_1)->Scale * _DAT_005529b4;
          }
          lVar19 = (longlong)(AsObject(param_1)->Position[0] * 0.0099999998f);   // IDA Terrain_Load((std::int64_t)(x*0.01),...)
          lVar20 = (longlong)(AsObject(param_1)->Position[1] * 0.0099999998f);   // IDA Terrain_Load(...,(std::int64_t)(y*0.01))
          iVar9 = Terrain_GetTileIndex((uint)lVar19,(uint)lVar20);
          if ((AsObject(param_1)->LifeTime == 1) || (((DAT_0838bc70)[iVar9] & 8) == 8)) {
            local_224 = AsObject(param_1)->Position[0];
            local_21c = local_348 + _DAT_00552878;
            AsObject(param_1)->Position[2] = local_348;
            local_220 = AsObject(param_1)->Position[1];
            AsObject(param_1)->Direction[0] = 0.0;
            AsObject(param_1)->Direction[1] = 0.0;
            AsObject(param_1)->Direction[2] = 0.0;
            Particle_Spawn(0x4bf,&local_224,AsObject(param_1)->Angle,&local_368,0,1.0,0);
            iVar9 = 6;
            do {
              bVar28 = 0;
              pfVar25 = (float *)0x0;
              pfVar23 = (float *)0xffffffff;
              pfVar21 = (float *)0x0;
              pfVar10 = param_1 + 0x3a;
              pfVar22 = (float *)0x0;
              pfVar15 = param_1 + 4;
              pfVar14 = param_1 + 7;
              uVar8 = _rand() % 2;
              CreateEffect(uVar8 + 0xc5,pfVar15,pfVar14,pfVar10,pfVar22,pfVar21,pfVar23,pfVar25,bVar28);
              iVar9 = iVar9 + -1;
            } while (iVar9 != 0);
            *(undefined1 *)param_1 = 0;
          }
        }
        goto LAB_00469a2f;
      }
LAB_00469769:
      if (*(int*)&fVar13 == 4) {
LAB_00469772:
        lVar19 = (longlong)(AsObject(param_1)->Position[0] * 0.0099999998f);   // IDA Terrain_Load((std::int64_t)(x*0.01),...)
        lVar20 = (longlong)(AsObject(param_1)->Position[1] * 0.0099999998f);   // IDA Terrain_Load(...,(std::int64_t)(y*0.01))
        iVar9 = Terrain_GetTileIndex((uint)lVar19,(uint)lVar20);
        if (((DAT_0838bc70)[iVar9] & 8) == 8) {
          if (AsObject(param_1)->Position[2] < _DAT_005529b0) {
            fVar17 = (float10)AsObject(param_1)->Position[2];
            goto LAB_004698ca;
          }
        }
        else {
          fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
          local_344 = (float)fVar17;
          if (AsObject(param_1)->Position[2] < local_344) {
            uVar8 = _rand() % 4;
            iVar9 = 0;
            _DAT_083a0210 = (float)(int)(uVar8 - 4) * _DAT_005524f4;
            do {
              local_348 = AsObject(param_1)->Position[0] - *(float *)(iVar9 + 0x10 + DAT_07abf5d0);
              pcVar11 = (char *)(iVar9 + DAT_07abf5d0);
              local_35c = (float *)(int)(AsObject(param_1)->Position[1] - AsObject(pcVar11)->Position[1]);
              if ((((*pcVar11 != '\0') && (*(char *)&AsObject(pcVar11)->Visible != '\0')) && (pcVar11 != DAT_07abf5d8)) &&
                 ((pcVar11[0x2fd] == '\0' &&
                  (SQRT(local_348 * local_348 + (float)(int)local_35c * (float)(int)local_35c) <=
                   _DAT_0055285c)))) {
                if (AsObject(pcVar11)->Type == 0x186) {
                  uVar8 = 0x82;
                }
                else {
                  uVar8 = 5;
                }
                SetAction((int)pcVar11,uVar8);
              }
              iVar9 = iVar9 + 0x394;
            } while (iVar9 < 0x59740);
            fVar17 = (float10)local_344;
            goto LAB_004698ca;
          }
        }
      }
      else {
        fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
        if (fVar17 <= (float10)AsObject(param_1)->Position[2]) goto LAB_00469a2f;
LAB_004698ca:
        pfVar15 = param_1 + 4;
        local_2b8 = *pfVar15;
        local_2b4 = AsObject(param_1)->Position[1];
        AsObject(param_1)->Position[2] = (float)fVar17;
        fVar18 = (float10)_DAT_00552878;
        AsObject(param_1)->Direction[0] = 0.0;
        local_2b0 = (float)(fVar17 + fVar18);
        AsObject(param_1)->Direction[1] = 0.0;
        AsObject(param_1)->Direction[2] = 0.0;
        pfVar10 = param_1 + 7;
        Particle_Spawn(0x4bf,&local_2b8,pfVar10,&local_368,0,1.0,0);
        iVar9 = 6;
        do {
          bVar28 = 0;
          pfVar26 = (float *)0x0;
          pfVar27 = (float *)0xffffffff;
          pfVar25 = (float *)0x0;
          pfVar14 = param_1 + 0x3a;
          pfVar23 = (float *)0x0;
          pfVar22 = pfVar15;
          pfVar21 = pfVar10;
          uVar8 = _rand() % 2;
          CreateEffect(uVar8 + 0xc5,pfVar22,pfVar21,pfVar14,pfVar23,pfVar25,pfVar27,pfVar26,bVar28);
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
        if (AsObject(param_1)->SubType == 6) {
          iVar9 = 0x1e;
          do {
            iVar12 = _rand();
            local_2b8 = (float)(iVar12 % 0xa0 + -0x50) + *pfVar15;
            iVar12 = _rand();
            iVar24 = 0;
            local_368 = 0.2;
            local_364 = 1.0;
            local_360 = 0.2;
            local_2b4 = (float)(iVar12 % 0xa0 + -100) + AsObject(param_1)->Position[1];
            local_2b0 = AsObject(param_1)->Position[2] + _DAT_00552598;
            uVar8 = _rand() % 32;
            Particle_Spawn(0x4c4,&local_2b8,pfVar10,&local_368,0xb,
                         (float)(int)(uVar8 + 0x50) * _DAT_005529cc,iVar24);
            iVar9 = iVar9 + -1;
          } while (iVar9 != 0);
        }
        *(undefined1 *)param_1 = 0;
      }
LAB_00469a2f:
      fVar13 = *(float *)&AsObject(param_1)->SubType;
      if (*(int*)&fVar13 == 5) {
        local_364 = local_36c * _DAT_005524f4;
        local_368 = local_36c;
        *(float *)&AsObject(param_1)->HiddenMesh = 0.0;
        AsObject(param_1)->BlendMeshLight = 0.0;
        local_360 = 0.0;
        AddTerrainLight(AsObject(param_1)->Position[0],AsObject(param_1)->Position[1], (float*)&local_368,2, (float*)DAT_081cb608);
        iVar9 = _rand();
        if (iVar9 % 5 != 0) {
          Particle_Spawn(0x567,AsObject(param_1)->Position,AsObject(param_1)->Angle,&local_368,0,1.0,0);
        }
      }
      else {
        if (*(int*)&fVar13 == 3) {
          *(float *)&AsObject(param_1)->HiddenMesh = 0.0;
LAB_00469ab9:
          AsObject(param_1)->BlendMeshLight = 0.0;
        }
        else if (fVar13 == 0.0) {
          uVar8 = _rand() % 4;
          AsObject(param_1)->BlendMeshLight = (float)(int)(uVar8 + 4) * _DAT_005524f4;
        }
        else {
          if (*(int*)&fVar13 != 6) goto LAB_00469ab9;
          uVar8 = _rand() % 4;
          AsObject(param_1)->BlendMeshLight = (float)(int)(uVar8 + 4) * _DAT_005524f4;
        }
        if (AsObject(param_1)->SubType == 6) {
          local_368 = local_36c * _DAT_005524f4;
          local_364 = local_36c;
        }
        else {
          local_364 = local_36c * _DAT_005524f4;
          local_368 = local_36c;
        }
        pfVar10 = param_1 + 4;
        local_360 = 0.0;
        AddTerrainLight(AsObject(param_1)->Position[0],AsObject(param_1)->Position[1], (float*)&local_368,2, (float*)DAT_081cb608);
        if (AsObject(param_1)->SubType == 6) {
          Particle_Spawn(0x4ab,pfVar10,AsObject(param_1)->Angle,&local_368,5,1.0,0);
          Particle_Spawn(0x4ab,pfVar10,AsObject(param_1)->Angle,&local_368,0,1.0,0);
        }
        else if (AsObject(param_1)->SubType == 3) {
          Ring_ComputeOrbit(param_2 * 0x14fb,&local_2f8,1.0);
          local_2f8 = local_2f8 * _DAT_00552598;
          local_2f4 = local_2f4 * _DAT_00552598;
          local_334 = 0.0;
          local_330 = 0.0;
          local_2f0 = local_2f0 * _DAT_00552598;
          local_32c = 0.0;
          local_368 = 1.0;
          local_364 = 1.0;
          local_360 = 1.0;
          local_2ec = *pfVar10 + local_2f8;
          local_2e8 = AsObject(param_1)->Position[1] + local_2f4;
          local_2e4 = AsObject(param_1)->Position[2] + local_2f0;
          Particle_Spawn(0x4ab,&local_2ec,&local_334,&local_368,8,AsObject(param_1)->Scale * _DAT_00552950,0);
          uVar29 = 0;
          iVar9 = _rand();
          CreateSprite(0x4cf,&local_2ec,AsObject(param_1)->Scale * _DAT_00552540,&local_368,0,
                       (float)(iVar9 % 0x168),uVar29);
        }
        else {
          Particle_Spawn(0x4ab,pfVar10,AsObject(param_1)->Angle,&local_368,5,1.0,0);
        }
        if (AsObject(param_1)->SubType == 1) {
          Effect_SpawnProximityHit((int)param_1);
        }
      }
      break;
    case 0xc0:
      local_364 = local_36c;
      fVar13 = (float)__cnt * _DAT_005524f4;
      AsObject(param_1)->BlendMeshLight = fVar13;
      AsObject(param_1)->Alpha = fVar13;
      local_368 = local_36c * _DAT_005528b8;
      local_360 = local_36c * _DAT_00552534;
LAB_004695b5:
      iVar9 = 2;
      goto LAB_004695c0;
    case 0xc2:
    case 0x103:
      if (!validOwner(owner)) return;
      __owner_fVar13 = *(int *)&AsObject(param_1)->Owner;   // ver nota de __owner_* arriba
      fVar4 = *(float *)(__owner_fVar13 + 0x14);
      fVar5 = *(float *)(__owner_fVar13 + 0x18);
      AsObject(param_1)->Position[0] = *(float *)(__owner_fVar13 + 0x10);
      AsObject(param_1)->Position[1] = fVar4;
      AsObject(param_1)->Position[2] = fVar5;
      break;
    case 0xc5:
    case 0xc6:
    case 0xd5:
    case 0xd6:
      if (AsObject(param_1)->SubType != 5) goto switchD_00466b93_caseD_c7;
      if (!validOwner(owner)) return;
      pcVar11 = (char *)*(int *)&AsObject(param_1)->Owner;
LAB_00468772:
      AsObject(param_1)->Position[0] = AsObject(pcVar11)->Position[0] + AsObject(param_1)->StartPosition[0];
      AsObject(param_1)->Position[1] = AsObject(pcVar11)->Position[1] + AsObject(param_1)->StartPosition[1];
      AsObject(param_1)->Position[2] = AsObject(pcVar11)->Position[2] + AsObject(param_1)->StartPosition[2];
      break;
    case 0xc9:
      // IDA uses the DWORD at o+0x60 as an integer counter here
      // (SLODWORD(v356)); fVar13 only carries those same bits as a float.
      // Converting fVar13 numerically turns e.g. counter 40 into zero.
      iVar12 = *(int *)((char *)param_1 + 0x60);
      iVar9 = iVar12;
      if (0x1d < iVar12) {
        iVar9 = 0x28 - iVar12;
      }
      AsObject(param_1)->BlendMeshLight = (float)iVar9 * _DAT_005524f4;
      AsObject(param_1)->BlendMeshTexCoordU = (float)iVar12 * _DAT_005529d0;
      iVar9 = _rand();
      _DAT_083a0210 = (float)(iVar9 % 6 + -6) * _DAT_005524f4;
      uVar8 = _rand() % 4;
      bVar16 = uVar8 == 0;
      if (bVar16) {
        local_200 = 0.0;
        iVar9 = _rand();
        local_1f8 = 0;
        local_1e8 = 0.0;
        local_1e4 = 0;
        local_1fc = (float)(iVar9 % 300);
        iVar9 = _rand();
        local_1e0 = (float)(iVar9 % 0x168);
        Matrix_BuildFromEuler(&local_1e8,local_6c);
        Vector_Rotate(&local_200,local_6c,&local_2d0);
        local_2d0 = local_2d0 + AsObject(param_1)->Position[0];
        pfVar15 = param_1 + 0x3a;
        pfVar10 = param_1 + 7;
        pfVar14 = &local_2d0;
        bVar28 = 0;
        pfVar25 = (float *)0x0;
        local_2cc = local_2cc + AsObject(param_1)->Position[1];
        pfVar23 = (float *)0xffffffff;
        pfVar21 = (float *)0x0;
        pfVar22 = (float *)0x1;
        local_2c8 = local_2c8 + AsObject(param_1)->Position[2];
        uVar8 = _rand() % 2;
        CreateEffect(uVar8 + 0xc5,pfVar14,pfVar10,pfVar15,pfVar22,pfVar21,pfVar23,pfVar25,bVar28);
      }
      local_364 = local_36c * _DAT_00552530;
      iVar9 = 4;
      local_360 = local_36c * _DAT_005526e4;
      goto LAB_00468736;
    case 0xca:
      if (!validOwner(owner)) return;
      __owner_fVar4 = *(int *)&AsObject(param_1)->Owner;   // ver nota de __owner_* arriba
      fVar6 = *(float *)(__owner_fVar4 + 0x18) + _DAT_005524f0;
      fVar5 = *(float *)(__owner_fVar4 + 0x14);
      AsObject(param_1)->Position[0] = *(float *)(__owner_fVar4 + 0x10);
      AsObject(param_1)->Position[1] = fVar5;
      AsObject(param_1)->Position[2] = fVar6;
      AsObject(param_1)->Angle[1] = AsObject(param_1)->Angle[1] + _DAT_005524fc;
    case 200:
      // LABEL_36 in IDA: scale = integer lifetime * 0.1.
      AsObject(param_1)->BlendMeshLight = (float)*(int *)((char *)param_1 + 0x60) * _DAT_005524f4;
      break;
    case 0xcb:
      pfVar10 = param_1 + 7;
      pfVar15 = param_1 + 4;
      iVar9 = 4;
      AsObject(param_1)->BlendMeshLight = (float)__cnt * _DAT_005524f4;
      AsObject(param_1)->BlendMeshTexCoordU = (float)__cnt * _DAT_00552570;
      do {
        Particle_Spawn(0x4c4,pfVar15,pfVar10,AsObject(param_1)->Light,3,1.0,0);
        iVar9 = iVar9 + -1;
      } while (iVar9 != 0);
      local_368 = local_36c * _DAT_005528b8;
      local_360 = local_36c;
      local_364 = local_36c * _DAT_00552534;
      AddTerrainLight(*pfVar15,AsObject(param_1)->Position[1], (float*)&local_368,3, (float*)DAT_081cb608);
      if (AsObject(param_1)->SubType == 2) {
        local_334 = *pfVar10;
        local_330 = AsObject(param_1)->Angle[1];
        local_32c = AsObject(param_1)->Angle[2];
        local_354 = *pfVar15;
        local_350 = AsObject(param_1)->Position[1];
        local_34c = AsObject(param_1)->Position[2];
        *(float *)&AsObject(param_1)->HiddenMesh = 0.0;
        iVar9 = _rand();
        iVar12 = 0;
        local_32c = (float)(iVar9 % 10 + -5) + local_32c;
        uVar8 = _rand() % 32;
        Particle_Spawn(0x4c4,pfVar15,pfVar10,&local_368,0xb,(float)(int)(uVar8 + 0x50) * _DAT_005529cc,iVar12);
        local_34c = local_34c + _DAT_00552598;
        local_26c = 0x3f800000;
        local_268 = 0x3f800000;
        local_264 = 0x3f800000;
        uVar29 = 0;
        iVar9 = _rand();
        CreateSprite(0x4cf,&local_354,1.5f,(float*)&local_26c,0,(float)(iVar9 % 0x168),uVar29);
        uVar29 = 0;
        iVar9 = _rand();
        CreateSprite(0x4cf,&local_354,1.5f,(float*)&local_26c,0,(float)(iVar9 % 0x168),uVar29);
        uVar29 = 0;
        iVar9 = _rand();
        CreateSprite(0x47e,&local_354,3.5f,&local_368,0,(float)(iVar9 % 0x168),uVar29);
      }
      break;
    case 0xcc:
      pfVar14 = param_1 + 0x3a;
      pfVar10 = param_1 + 7;
      pfVar15 = param_1 + 4;
      AsObject(param_1)->BlendMeshLight = (float)__cnt * _DAT_005524f4;
      AsObject(param_1)->BlendMeshTexCoordU = (float)__cnt * _DAT_005529c8;
      Particle_Spawn(0x4c4,pfVar15,pfVar10,pfVar14,3,1.0,0);
      local_32c = AsObject(param_1)->Angle[2];
      local_334 = 90.0;
      local_330 = 0.0;
      uVar8 = _rand() % 2;
      bVar16 = uVar8 == 0;
      if (bVar16) {
        local_354 = *pfVar15 - _DAT_0055285c;
        local_350 = AsObject(param_1)->Position[1];
        local_34c = AsObject(param_1)->Position[2] + _DAT_005529c4;
        Joint_Create(0x4e6,&local_354,pfVar15,&local_334,0,(int)param_1,10.0,-1,0);
      }
      uVar8 = _rand() % 2;
      bVar16 = uVar8 == 0;
      if (bVar16) {
        local_354 = *pfVar15 + _DAT_0055285c;
        local_350 = AsObject(param_1)->Position[1];
        local_34c = AsObject(param_1)->Position[2] + _DAT_005529c4;
        Joint_Create(0x4e6,&local_354,pfVar15,&local_334,0,(int)param_1,10.0,-1,0);
      }
      fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
      AsObject(param_1)->Position[2] = (float)fVar17;
      uVar8 = _rand() % 4;
      bVar16 = uVar8 == 0;
      if (bVar16) {
        bVar28 = 0;
        pfVar27 = (float *)0x0;
        pfVar25 = (float *)0xffffffff;
        pfVar23 = (float *)0x0;
        pfVar21 = (float *)0x2;
        pfVar22 = pfVar15;
        uVar8 = _rand() % 2;
        CreateEffect(uVar8 + 0xc5,pfVar22,pfVar10,pfVar14,pfVar21,pfVar23,pfVar25,pfVar27,bVar28);
      }
      local_368 = local_36c * _DAT_00552990;
      local_364 = local_36c * _DAT_005529c0;
      local_360 = local_36c * _DAT_00552570;
      AddTerrainLight(*pfVar15,AsObject(param_1)->Position[1], (float*)&local_368,5, (float*)DAT_081cb608);
      if (((AsObject(param_1)->LifeTime) % 0xf == 0) && ((char *)*(int *)&AsObject(param_1)->Owner == DAT_07abf5d8)) {
        Entity_FindNearby_SendPacket((uint)AsObject(param_1)->Skill,pfVar15,150.0,*(byte *)(param_1 + 0x22),
                     AsObject(param_1)->PKKey);
      }
      break;
    case 0xd1:
      AsObject(param_1)->Angle[2] = AsObject(param_1)->Angle[2] - _DAT_0055284c;
      break;
    case 0xd2:
    case 0xd3:
    case 0xe2:
    case 0xe3:
      if (AsObject(param_1)->SubType == 5) {
        if (!validOwner(owner)) return;
        pcVar11 = (char *)*(int *)&AsObject(param_1)->Owner;
        if (*pcVar11 == '\0') {
          *(undefined1 *)param_1 = 0;
        }
        goto LAB_00468772;
      }
    case 199:
    case 0xe7:
    case 0xe8:
    case 0xed:
switchD_00466b93_caseD_c7:
      if (*(float *)&AsObject(param_1)->SubType == 0.0) {
        AsObject(param_1)->Position[0] = AsObject(param_1)->Direction[0] + AsObject(param_1)->Position[0];
        AsObject(param_1)->Position[1] = AsObject(param_1)->Direction[1] + AsObject(param_1)->Position[1];
        fVar13 = AsObject(param_1)->Direction[2];
      }
      else {
        Matrix_BuildFromEuler(AsObject(param_1)->Angle,local_1a4 + 6);
        Vector_Rotate(AsObject(param_1)->Direction,local_1a4 + 6,&local_354);
        AsObject(param_1)->Position[0] = local_354 + AsObject(param_1)->Position[0];
        AsObject(param_1)->Position[1] = local_350 + AsObject(param_1)->Position[1];
        fVar13 = local_34c;
      }
      AsObject(param_1)->Position[2] = fVar13 + AsObject(param_1)->Position[2];
      AsObject(param_1)->Direction[0] = AsObject(param_1)->Direction[0] * _DAT_005526e8;
      AsObject(param_1)->Direction[1] = AsObject(param_1)->Direction[1] * _DAT_005526e8;
      AsObject(param_1)->Direction[2] = AsObject(param_1)->Direction[2] * _DAT_005526e8;
      AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] + AsObject(param_1)->Gravity;
      if (*(float *)&AsObject(param_1)->SubType == 0.0) {
        AsObject(param_1)->Gravity = AsObject(param_1)->Gravity - _DAT_00552540;
        fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
        if (fVar17 <= (float10)AsObject(param_1)->Position[2]) {
          fVar13 = AsObject(param_1)->Scale * _DAT_005529b8;
        }
        else {
          AsObject(param_1)->Position[2] = (float)fVar17;
          fVar13 = AsObject(param_1)->Gravity * _DAT_00552a14;
          AsObject(param_1)->LifeTime = AsObject(param_1)->LifeTime + -4;   // counter DWORD
          AsObject(param_1)->Gravity = fVar13;
          fVar13 = AsObject(param_1)->Scale * _DAT_005529bc;
        }
        pfVar10 = param_1 + 7;
        *pfVar10 = *pfVar10 - fVar13;
        if ((*(float *)&AsObject(param_1)->SubType == 0.0) && (iVar9 = _rand(), iVar9 % 10 == 0)) {
          sVar3 = AsObject(param_1)->Type;
          if ((sVar3 == 199) || ((sVar3 == 0xe7 || (sVar3 == 0xe8)))) {
            Particle_Spawn(0x4c4,AsObject(param_1)->Position,pfVar10,AsObject(param_1)->Light,0,1.0,0);
          }
          else if ((sVar3 == 0xc5) || (sVar3 == 0xc6)) {
            iVar12 = 0;
            fVar13 = 1.0;
            iVar9 = _rand();
            Particle_Spawn(0x4ab,AsObject(param_1)->Position,pfVar10,AsObject(param_1)->Light,iVar9 % 3 + 1,fVar13,iVar12);
          }
        }
      }
      else {
        if (AsObject(param_1)->SubType != 1) {
          AsObject(param_1)->Angle[2] = AsObject(param_1)->Angle[2] + _DAT_005524fc;
        }
        AsObject(param_1)->Gravity = AsObject(param_1)->Gravity + _DAT_00552504;
      }
      break;
    case 0xd4:
      AsObject(param_1)->Direction[2] = AsObject(param_1)->Direction[2] - _DAT_00552950;
      Effect_SpawnProximityHit((int)param_1);
      break;
    case 0xd7:
      AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] + AsObject(param_1)->Gravity;
      AsObject(param_1)->Gravity = AsObject(param_1)->Gravity - _DAT_0055256c;
      fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
      if ((float10)AsObject(param_1)->Position[2] < fVar17) {
        AsObject(param_1)->Position[2] = (float)fVar17;
        fVar13 = AsObject(param_1)->Gravity * _DAT_00552990;
        AsObject(param_1)->LifeTime = AsObject(param_1)->LifeTime + -4;   // counter DWORD
        AsObject(param_1)->Gravity = fVar13;
        AsObject(param_1)->Direction[1] = AsObject(param_1)->Direction[1] - _DAT_0055264c;
      }
      break;
    case 0xd8:
      local_368 = local_36c * _DAT_00552530;
      local_364 = local_36c * _DAT_00552504;
      local_360 = local_36c * _DAT_005526e4;
      AddTerrainLight(AsObject(param_1)->Position[0],AsObject(param_1)->Position[1], (float*)&local_368,2, (float*)DAT_081cb608);
      pfVar10 = param_1 + 7;
      Particle_Spawn(0x4ab,AsObject(param_1)->Position,pfVar10,&local_368,0,1.0,0);
      Effect_CollisionCheck((int)param_1);
      if (AsObject(param_1)->SubType == 3) {
        fVar13 = *pfVar10 + _DAT_00552488;
        *pfVar10 = fVar13;
        if (_DAT_00552844 < fVar13) {
          *pfVar10 = 45.0;
        }
        lVar19 = (longlong)(AsObject(param_1)->Position[0] * 0.0099999998f);   // IDA Terrain_Load((std::int64_t)(x*0.01),...)
        lVar20 = (longlong)(AsObject(param_1)->Position[1] * 0.0099999998f);   // IDA Terrain_Load(...,(std::int64_t)(y*0.01))
        iVar9 = Terrain_GetTileIndex((uint)lVar19,(uint)lVar20);
        if ((((DAT_0838bc70)[iVar9] & 8) != 8) &&
           (fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]), (float10)AsObject(param_1)->Position[2] < fVar17)) {
          fVar18 = (float10)_DAT_00552488;
          AsObject(param_1)->Direction[0] = 0.0;
          AsObject(param_1)->Direction[1] = 0.0;
          AsObject(param_1)->Direction[2] = 0.0;
          AsObject(param_1)->Position[2] = (float)(fVar17 + fVar18);
        }
      }
      break;
    case 0xd9:
    case 0xda:
    case 0xdb:
    case 0xdc:
    case 0xdd:
    case 0xde:
    case 0xdf:
    case 0xe0:
      if (sVar3 == 0xde) {
        pfVar15 = param_1 + 0x3a;
        pfVar10 = param_1 + 4;
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: (std::int64_t)WorldTime * 0.1
        CreateSprite(0x4a7,pfVar10,0.5f,pfVar15,(int)param_1,(float)(int)lVar19 * _DAT_005524f4,uVar29);
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: -(std::int64_t)WorldTime * 0.1
        CreateSprite(0x4a7,pfVar10,1.0f,pfVar15,(int)param_1,(float)-(int)lVar19 * _DAT_005524f4,uVar29);
        iVar9 = 4;
        do {
          uVar8 = _rand() % 32;
          local_354 = (float)(int)(uVar8 - 0x10);
          uVar8 = _rand() % 64;
          local_350 = (float)(int)(uVar8 - 0x20);
          uVar8 = _rand() % 32;
          iVar30 = 0;
          fVar13 = 1.0;
          local_354 = local_354 + *pfVar10;
          iVar24 = 0;
          pfVar14 = param_1 + 7;
          pfVar22 = &local_354;
          local_350 = local_350 + AsObject(param_1)->Position[1];
          local_34c = (float)(int)(uVar8 - 0x10) + AsObject(param_1)->Position[2];
          pfVar21 = pfVar15;
          iVar12 = _rand();
          Particle_Spawn(iVar12 % 3 + 0x4da,pfVar22,pfVar14,pfVar21,iVar24,fVar13,iVar30);
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
        AsObject(param_1)->Angle[1] = AsObject(param_1)->Angle[1] + _DAT_0055284c;
      }
      else if (sVar3 == 0xe0) {
        pfVar15 = param_1 + 0x3a;
        pfVar10 = param_1 + 4;
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: (std::int64_t)WorldTime * 0.1
        CreateSprite(0x4a7,pfVar10,0.5f,pfVar15,(int)param_1,(float)(int)lVar19 * _DAT_005524f4,uVar29);
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: -(std::int64_t)WorldTime * 0.1
        CreateSprite(0x4a7,pfVar10,1.0f,pfVar15,(int)param_1,(float)-(int)lVar19 * _DAT_005524f4,uVar29);
        local_35c = (float *)0x4;
        do {
          uVar8 = _rand() % 16;
          local_354 = (float)(int)(uVar8 - 8);
          uVar8 = _rand() % 16;
          local_350 = (float)(int)(uVar8 - 8);
          uVar8 = _rand() % 16;
          local_354 = local_354 + *pfVar10;
          local_350 = local_350 + AsObject(param_1)->Position[1];
          local_34c = (float)(int)(uVar8 - 8) + AsObject(param_1)->Position[2];
          Particle_Spawn(0x4d9,&local_354,AsObject(param_1)->Angle,pfVar15,1,1.0,0);
          local_35c = (float *)((int)local_35c + -1);
        } while (local_35c != (float *)0x0);
        Particle_Spawn(0x4c4,pfVar10,AsObject(param_1)->Angle,pfVar15,0,1.0,0);
      }
      else if (sVar3 == 0xdf) {
        local_368 = 1.0;
        local_364 = 0.6;
        local_360 = 0.4;
        pfVar10 = param_1 + 4;
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: (std::int64_t)WorldTime * 0.1
        CreateSprite(0x47e,pfVar10,1.0f,&local_368,(int)param_1,(float)(int)lVar19 * _DAT_005524f4,uVar29);
        uVar29 = 0;
        lVar19 = (longlong)DAT_05826e08;   // IDA: -(std::int64_t)WorldTime * 0.1
        CreateSprite(0x47e,pfVar10,2.0f,&local_368,(int)param_1,(float)-(int)lVar19 * _DAT_005524f4,uVar29);
        iVar9 = 4;
        do {
          uVar8 = _rand() % 16;
          local_354 = (float)(int)(uVar8 - 8);
          uVar8 = _rand() % 16;
          local_350 = (float)(int)(uVar8 - 8);
          uVar8 = _rand() % 16;
          local_354 = local_354 + *pfVar10;
          local_350 = local_350 + AsObject(param_1)->Position[1];
          local_34c = (float)(int)(uVar8 - 8) + AsObject(param_1)->Position[2];
          Particle_Spawn(0x4d9,&local_354,AsObject(param_1)->Angle,AsObject(param_1)->Light,1,1.0,0);
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
        AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] + AsObject(param_1)->Gravity;
        AsObject(param_1)->Gravity = AsObject(param_1)->Gravity - _DAT_0055256c;
        fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
        if ((float10)AsObject(param_1)->Position[2] < fVar17) {
          AsObject(param_1)->Position[2] = (float)fVar17;
          AsObject(param_1)->Gravity = AsObject(param_1)->Gravity * _DAT_00552990;
          AsObject(param_1)->Direction[1] = AsObject(param_1)->Direction[1] * _DAT_005528b4;
        }
        if ((AsObject(param_1)->LifeTime == 1) &&
           (Effect_SpawnSmokeBurst(pfVar10,'\x01'), (char *)*(int *)&AsObject(param_1)->Owner == DAT_07abf5d8)) {
          Entity_FindNearby_SendPacket((uint)AsObject(param_1)->Skill,pfVar10,100.0,*(byte *)(param_1 + 0x22),
                       AsObject(param_1)->PKKey);
        }
      }
      local_368 = local_36c * _DAT_00552534;
      local_364 = local_36c * _DAT_00552530;
      local_360 = local_364;
      AddTerrainLight(AsObject(param_1)->Position[0],AsObject(param_1)->Position[1], (float*)&local_368,2, (float*)DAT_081cb608);
      Effect_CollisionCheck((int)param_1);
      break;
    case 0xe1:
      if (!validOwner(owner)) return;
      pcVar11 = (char *)*(int *)&AsObject(param_1)->Owner;
      AsObject(param_1)->BlendMeshLight = 1.3;
      fVar13 = AsObject(pcVar11)->Position[1];
      fVar4 = AsObject(param_1)->Angle[2] + _DAT_00552488;
      AsObject(param_1)->Position[0] = AsObject(pcVar11)->Position[0];
      AsObject(param_1)->Position[2] = AsObject(pcVar11)->Position[2];
      cVar2 = *pcVar11;
      AsObject(param_1)->Angle[2] = fVar4;
      AsObject(param_1)->Position[1] = fVar13;
      if ((cVar2 == '\0') || (*(char *)&AsObject(pcVar11)->Visible == '\0')) {
        *(undefined1 *)param_1 = 0;
      }
      break;
    case 0xe4:
    case 0xe5:
    case 0xe6:
      pfVar10 = param_1 + 4;
      lVar19 = (longlong)AsObject(param_1)->Position[1];   // IDA Terrain_Load(x/100, y/100): este es y
      uVar8 = (int)lVar19 / 100;
      lVar19 = (longlong)AsObject(param_1)->Position[0];   // IDA Terrain_Load(x/100, y/100): este es x
      iVar9 = Terrain_GetTileIndex((int)lVar19 / 100,uVar8);
      if ((((DAT_0838bc70)[iVar9] & 8) != 8) &&
         (fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]), (float10)AsObject(param_1)->Position[2] < fVar17)) {
        AsObject(param_1)->Position[2] = (float)fVar17;
        *(undefined1 *)param_1 = 0;
        uVar8 = _rand() % 8;
        iVar9 = 5;
        _DAT_083a0210 = (float)(int)(uVar8 - 8) * _DAT_005524f4;
        do {
          bVar28 = 0;
          pfVar27 = (float *)0x0;
          pfVar25 = (float *)0xffffffff;
          pfVar23 = (float *)0x0;
          pfVar21 = (float *)0x0;
          pfVar15 = param_1 + 7;
          pfVar14 = pfVar10;
          pfVar22 = param_1 + 0x3a;
          uVar8 = _rand() % 2;
          CreateEffect(uVar8 + 0xe7,pfVar14,pfVar15,pfVar22,pfVar21,pfVar23,pfVar25,pfVar27,bVar28);
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
      }
      pfVar15 = param_1 + 7;
      *pfVar10 = AsObject(param_1)->Direction[0] + *pfVar10;
      AsObject(param_1)->Position[1] = AsObject(param_1)->Direction[1] + AsObject(param_1)->Position[1];
      AsObject(param_1)->Position[2] = AsObject(param_1)->Direction[2] + AsObject(param_1)->Position[2];
      *pfVar15 = _DAT_00552488 / AsObject(param_1)->Scale + *pfVar15;
      Particle_Spawn(0x4c4,pfVar10,pfVar15,AsObject(param_1)->Light,3,1.0,0);
      return;
    case 0xe9:
      if (AsObject(param_1)->SubType == 1) {
        local_368 = local_36c;
        AsObject(param_1)->Scale = AsObject(param_1)->Scale + _DAT_005524f8;
        local_364 = 0.0;
        local_360 = 0.0;
        AddTerrainLight(AsObject(param_1)->Position[0],AsObject(param_1)->Position[1], (float*)&local_368,3, (float*)DAT_081cb608);
        AsObject(param_1)->BlendMeshLight = (float)(AsObject(param_1)->LifeTime) * _DAT_00552a00;
      }
      else {
        if (!validOwner(owner)) return;
        __owner_fVar13 = *(int *)&AsObject(param_1)->Owner;   // ver nota de __owner_* arriba
        fVar4 = *(float *)(__owner_fVar13 + 0x10);
        fVar5 = *(float *)(__owner_fVar13 + 0x14);
        AsObject(param_1)->Scale = AsObject(param_1)->Scale + _DAT_005524f8;
        fVar13 = *(float *)(__owner_fVar13 + 0x18);
        AsObject(param_1)->Position[0] = fVar4;
        AsObject(param_1)->Position[1] = fVar5;
        AsObject(param_1)->Position[2] = fVar13;
        AsObject(param_1)->BlendMeshLight = (float)(AsObject(param_1)->LifeTime) * _DAT_00552a00;
      }
      break;
    case 0xee:
      CreateEffect(0xef,AsObject(param_1)->Position,AsObject(param_1)->Angle,AsObject(param_1)->Light,(float *)(4 - __cnt),
                   (float *)*(int *)&AsObject(param_1)->Owner,(float *)(uint)*(ushort *)&AsObject(param_1)->PKKey,
                   (float *)CONCAT31((int3)(CONCAT22(extraout_var,sVar3) >> 8),
                                     AsObject(param_1)->Skill),AsObject(param_1)->Kind);
      break;
    case 0xef:
      if (*(unsigned int *)&AsObject(param_1)->SubType >= 1 && *(unsigned int *)&AsObject(param_1)->SubType <= 4)
        AsObject(param_1)->Alpha = EffectTrailAlpha[*(unsigned int *)&AsObject(param_1)->SubType - 1];
      if (!validOwner(owner)) return;
      if ((*(byte *)(*(int *)&AsObject(param_1)->Owner + 0x88) < 0x60) ||
         (0x7f < *(byte *)(*(int *)&AsObject(param_1)->Owner + 0x88))) {
        local_318 = 0xc3160000;
      }
      else {
        local_318 = 0xc3340000;
      }
      local_31c = 0.0;
      pfVar10 = param_1 + 7;
      local_314 = 0;
      Matrix_BuildFromEuler(pfVar10,local_1a4 + 6);
      Vector_Rotate(&local_31c,local_1a4 + 6,&local_354);
      if (!validOwner(owner)) return;
      __owner_fVar13 = *(int *)&AsObject(param_1)->Owner;   // ver nota de __owner_* arriba
      pfVar15 = param_1 + 4;
      *pfVar15 = local_354 + *(float *)(__owner_fVar13 + 0x10);
      AsObject(param_1)->Position[1] = local_350 + *(float *)(__owner_fVar13 + 0x14);
      AsObject(param_1)->Position[2] = local_34c + *(float *)(__owner_fVar13 + 0x18);
      AsObject(param_1)->Angle[2] = AsObject(param_1)->Angle[2] - _DAT_005529fc;
      Particle_Spawn(0x4c4,pfVar15,pfVar10,AsObject(param_1)->Light,3,1.0,0);
      local_368 = local_36c * _DAT_005528b8;
      local_364 = local_368;
      local_360 = local_368;
      AddTerrainLight(*pfVar15,AsObject(param_1)->Position[1], (float*)&local_368,3, (float*)DAT_081cb608);
      local_368 = 1.0;
      local_364 = 1.0;
      local_360 = 1.0;
      iVar9 = 0;
      do {
        iVar12 = _rand();
        local_330 = 0.0;
        local_35c = (float *)(iVar12 % 0x3c + -0x1e);
        local_334 = (float)(int)local_35c;
        iVar12 = _rand();
        local_350 = AsObject(param_1)->Position[1];
        local_34c = AsObject(param_1)->Position[2];
        local_35c = (float *)(iVar12 % 0x1e + 0x5a);
        local_354 = *pfVar15;
        local_334 = local_334 + *pfVar10;
        local_330 = local_330 + AsObject(param_1)->Angle[1];
        local_32c = (float)(int)local_35c + AsObject(param_1)->Angle[2];
        iVar12 = _rand();
        local_35c = (float *)(iVar12 % 0x14 + -10);
        local_354 = (float)(int)local_35c + local_354;
        iVar12 = _rand();
        local_35c = (float *)(iVar12 % 0x14 + -10);
        local_350 = (float)(int)local_35c + local_350;
        Joint_Create(0x4e9,&local_354,&local_354,&local_334,0,0,10.0,-1,0);
        if (iVar9 == 0) {
          Particle_Spawn(0x497,&local_354,&local_334,&local_368,0,1.0,0);
        }
        iVar9 = iVar9 + 1;
      } while (iVar9 < 4);
      local_354 = *pfVar15;
      local_350 = AsObject(param_1)->Position[1];
      local_34c = AsObject(param_1)->Position[2] + _DAT_005524fc;
      local_368 = 1.0;
      local_364 = 0.8;
      local_360 = 0.6;
      CreateSprite(0x47e,&local_354,2.0f,&local_368,(int)param_1,0,0);
      if (*(float *)&AsObject(param_1)->SubType == 0.0) {
        uVar8 = (*(uint *)&AsObject(param_1)->LifeTime) & 0x80000007;
        bVar16 = uVar8 == 0;
        if ((int)uVar8 < 0) {
          bVar16 = (uVar8 - 1 | 0xfffffff8) == 0xffffffff;
        }
        if ((bVar16) && ((char *)*(int *)&AsObject(param_1)->Owner == DAT_07abf5d8)) {
          Entity_FindNearby_SendPacket((uint)AsObject(param_1)->Skill,pfVar15,150.0,*(byte *)(param_1 + 0x22),
                       AsObject(param_1)->PKKey);
        }
      }
      break;
    case 0xf0:
      pfVar10 = param_1 + 4;
      fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
      if ((float10)AsObject(param_1)->Position[2] < fVar17) {
        local_284 = *pfVar10;
        local_280 = AsObject(param_1)->Position[1];
        AsObject(param_1)->Position[2] = (float)fVar17;
        pfVar15 = param_1 + 7;
        local_27c = (float)(fVar17 + (float10)_DAT_00552878);
        AsObject(param_1)->Direction[0] = 0.0;
        AsObject(param_1)->Direction[1] = 0.0;
        AsObject(param_1)->Direction[2] = 0.0;
        Particle_Spawn(0x4bf,&local_284,pfVar15,&local_368,0,1.0,0);
        Particle_Spawn(0x4d2,&local_284,pfVar15,&local_368,0,1.0,0);
        iVar9 = 6;
        do {
          bVar28 = 0;
          pfVar26 = (float *)0x0;
          pfVar27 = (float *)0xffffffff;
          pfVar25 = (float *)0x0;
          pfVar14 = param_1 + 0x3a;
          pfVar23 = (float *)0x0;
          pfVar22 = pfVar10;
          pfVar21 = pfVar15;
          uVar8 = _rand() % 2;
          CreateEffect(uVar8 + 0xc5,pfVar22,pfVar21,pfVar14,pfVar23,pfVar25,pfVar27,pfVar26,bVar28);
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
        bVar16 = (char *)*(int *)&AsObject(param_1)->Owner == DAT_07abf5d8;
        *(undefined1 *)param_1 = 0;
        if (bVar16) {
          Entity_FindNearby_SendPacket((uint)AsObject(param_1)->Skill,pfVar10,150.0,*(byte *)(param_1 + 0x22),
                       AsObject(param_1)->PKKey);
        }
      }
      local_344 = *pfVar10;
      param_1[0x10] = local_344;
      local_348 = AsObject(param_1)->Position[1];
      param_1[0x12] = AsObject(param_1)->Position[2];
      param_1[0x11] = local_348;
      local_368 = local_36c * _DAT_005526e4;
      local_360 = local_36c;
      local_364 = local_36c * _DAT_005528b4;
      AddTerrainLight(local_344,local_348, (float*)&local_368,2, (float*)DAT_081cb608);
      break;
    case 0xf1:
      fVar4 = *(float *)&AsObject(param_1)->SubType;
      if (*(int*)&fVar4 == 2) {
        AsObject(param_1)->Scale = AsObject(param_1)->Scale + _DAT_00552a0c;
LAB_00466e5e:
        if (*(int*)&fVar4 == 4) break;
      }
      else {
        if (*(int*)&fVar4 == 3) {
          if (!validOwner(owner)) return;
          __owner_fVar5 = *(int *)&AsObject(param_1)->Owner;   // ver nota de __owner_* arriba
          AsObject(param_1)->Position[1] = *(float *)(__owner_fVar5 + 0x14);
          iVar9 = *(int *)(__owner_fVar5 + 0xfc);
          if (!validOwner((void*)(uintptr_t)iVar9)) return;
          AsObject(param_1)->Position[2] = *(float *)(__owner_fVar5 + 0x18);
          AsObject(param_1)->Position[0] = *(float *)(iVar9 + 0x10) + _DAT_0055297c;
          goto LAB_00466e5e;
        }
        if (*(int*)&fVar4 == 4) {
          fVar7 = AsObject(param_1)->Scale + _DAT_00552a08;
          if (!validOwner(owner)) return;
          __owner_fVar5 = *(int *)&AsObject(param_1)->Owner;   // ver nota de __owner_* arriba
          fVar6 = *(float *)(__owner_fVar5 + 0x10);
          AsObject(param_1)->Scale = fVar7;
          fVar7 = fVar7 * _DAT_00552a04;
          AsObject(param_1)->Position[0] = fVar6;
          AsObject(param_1)->Position[1] = *(float *)(__owner_fVar5 + 0x14);
          fVar7 = fVar7 + AsObject(param_1)->Gravity;
          AsObject(param_1)->Gravity = fVar7;
          AsObject(param_1)->Position[2] = fVar7 + *(float *)(__owner_fVar5 + 0x18);
          goto LAB_00466e5e;
        }
      }
      AsObject(param_1)->BlendMeshLight = (float)__cnt * _DAT_00552a10;
      goto LAB_00466d90;
    case 0xf2:
      fVar13 = AsObject(param_1)->Angle[1] + _DAT_0055284c;
      param_1[0x12] = AsObject(param_1)->Position[2];
      local_360 = local_36c;
      AsObject(param_1)->Angle[1] = fVar13;
      local_344 = AsObject(param_1)->Position[0];
      param_1[0x10] = local_344;
      local_348 = AsObject(param_1)->Position[1];
      param_1[0x11] = local_348;
      local_368 = local_36c * _DAT_005526e4;
      local_364 = local_36c * _DAT_005528b4;
      AddTerrainLight(local_344,local_348, (float*)&local_368,2, (float*)DAT_081cb608);
      Effect_CollisionCheck((int)param_1);
      break;
    case 0xf3:
      local_360 = local_36c;
      AsObject(param_1)->Angle[1] = AsObject(param_1)->Angle[1] + _DAT_0055284c;
      local_368 = local_36c * _DAT_005526e4;
      local_364 = local_36c * _DAT_005528b4;
      AddTerrainLight(AsObject(param_1)->StartPosition[0],AsObject(param_1)->StartPosition[1], (float*)&local_368,2, (float*)DAT_081cb608);
      if (*(float *)&AsObject(param_1)->SubType != 0.0) {
        if (AsObject(param_1)->LifeTime == 13) {
          CreateEffect(0xff,AsObject(param_1)->Position,AsObject(param_1)->Angle,AsObject(param_1)->Light,(float *)0x0,param_1,
                       (float *)0xffffffff,(float *)0x0,0);
        }
        else if (AsObject(param_1)->LifeTime == 30) {
          *(float *)&AsObject(param_1)->AttackPoint = 0.0;
          AsObject(param_1)->Kind = 1;
        }
      }
      break;
    case 0xf4:
      // Rageful Blow: las cuatro ramas de abajo comparan `v4 = *(int *)(o + 96)`
      // (IDA L419), o sea el LIFETIME, que aca es `__cnt`.  El efecto nace con
      // vida 20 y va bajando:
      //   15     -> invierte el signo del termino vertical (o+216)
      //   13     -> estela de efectos 254 + sonido 89
      //   10..15 -> el arma DESCIENDE (-8/frame)
      //    3     -> impacto en el suelo (grietas, chispas, 247/245/246)
      //    2     -> reporte de blancos (sub_45FEC0) y muerte del efecto
      local_340 = 0.0;
      local_33c = 0.0;
      local_338 = 0.0;
      local_310 = 0.0;
      local_30c = 0.0;
      local_308 = 0;
      if (__cnt == 3) {
        local_20c = 1.0;
        local_208 = 0x3f800000;
        local_204 = 0x3f800000;
        local_340 = 0.0;
        local_33c = 0.0;
        local_338 = 0.0;
        local_310 = -25.0;
        local_30c = -80.0;
        local_308 = 0;
        if (!validOwner(owner)) return;
        Matrix_BuildFromEuler((float *)(*(int *)&AsObject(param_1)->Owner + 0x1c),local_15c);
        Vector_Rotate(&local_310,local_15c,&local_328);
        local_348 = local_328 + AsObject(param_1)->Position[0];
        pfVar10 = param_1 + 0x5c;
        *pfVar10 = local_348;
        // Rageful Blow, punto de impacto (IDA):
        //  - Y es float: `v355 = TargetPosition[1] + *(float*)(o+20)` (sin truncar).
        //  - RequestTerrainHeight recibe el punto YA rotado (v60/v299, o sea o+368 y
        //    o+372), no la posicion original del efecto: el golpe cae donde cae el
        //    arma, 80 unidades adelante.
        AsObject(param_1)->StartPosition[1] = local_324 + AsObject(param_1)->Position[1];
        AsObject(param_1)->StartPosition[2] = local_320 + AsObject(param_1)->Position[2];
        fVar17 = RequestTerrainHeight(AsObject(param_1)->StartPosition[0], AsObject(param_1)->StartPosition[1]);
        AsObject(param_1)->StartPosition[2] = (float)(fVar17 + (float10)_DAT_00552464);
        Particle_Spawn(0x4bf,pfVar10,&local_340,&local_20c,0,0.5,0);
        iVar9 = 0;
        do {
          iVar12 = _rand();
          local_33c = 0.0;
          local_35c = (float *)(iVar12 % 0x3c);
          local_340 = (float)(int)local_35c - _DAT_0055290c;
          iVar12 = _rand();
          local_324 = AsObject(param_1)->StartPosition[1];
          local_320 = AsObject(param_1)->StartPosition[2];
          local_35c = (float *)(iVar12 % 0x1e + 0x5a);
          local_328 = *pfVar10;
          local_340 = local_340 + AsObject(param_1)->Angle[0];
          local_33c = local_33c + AsObject(param_1)->Angle[1];
          local_338 = (float)(int)local_35c + AsObject(param_1)->Angle[2];
          iVar12 = _rand();
          local_35c = (float *)(iVar12 % 0x14 + -10);
          local_328 = (float)(int)local_35c + local_328;
          iVar12 = _rand();
          local_35c = (float *)(iVar12 % 0x14 + -10);
          local_324 = (float)(int)local_35c + local_324;
          Joint_Create(0x4e9,&local_328,&local_328,&local_340,0,0,10.0,-1,0);
          if (iVar9 == 0) {
            Particle_Spawn(0x497,&local_328,&local_340,&local_368,0,1.0,0);
          }
          iVar9 = iVar9 + 1;
        } while (iVar9 < 8);
        pfVar15 = param_1 + 0x3a;
        local_340 = 0.0;
        local_33c = 0.0;
        local_338 = 0.0;
        CreateEffect(0xfd,pfVar10,&local_340,pfVar15,(float *)0x0,(float *)0x0,(float *)0xffffffff,(float *)0x0,0);
        AsObject(param_1)->StartPosition[2] = AsObject(param_1)->StartPosition[2] - _DAT_005524a8;
        CreateEffect(0xf7,pfVar10,&local_340,pfVar15,(float *)0x0,param_1,(float *)0x96,(float *)0x0,0);
        CreateEffect(0xf5,pfVar10,&local_340,pfVar15,(float *)0x0,param_1,(float *)0x96,(float *)0x0,0);
        CreateEffect(0xf6,pfVar10,&local_340,pfVar15,(float *)0x0,param_1,(float *)0x96,(float *)0x0,0);
        local_358 = 0.0;
        do {
          local_310 = 0.0;
          iVar9 = _rand();
          local_308 = 0;
          local_340 = 0.0;
          local_33c = 0.0;
          local_35c = (float *)(iVar9 % 0x96 + 100);
          local_30c = (float)(int)local_35c;
          // IDA L2095: `(double)SLODWORD(v356) * 72.0 + (double)*(int *)(o + 4)`
          // — el SubType es un DWORD; convertirlo como float daba siempre 0.
          local_338 = (float)(int)local_358 * _DAT_005529f8 + (float)AsObject(param_1)->SubType;
          Matrix_BuildFromEuler(&local_340,local_15c);
          Vector_Rotate(&local_310,local_15c,&local_328);
          local_328 = local_328 + *pfVar10;
          local_324 = local_324 + AsObject(param_1)->StartPosition[1];
          local_320 = local_320 + AsObject(param_1)->StartPosition[2] + _DAT_00552540;
          iVar9 = _rand();
          local_340 = 0.0;
          local_33c = 0.0;
          pfVar14 = (float *)(iVar9 % 0x32 + 0x28);
          iVar9 = _rand();
          local_35c = (float *)(iVar9 % 0x1e + -0xf);
          local_338 = (float)(int)local_35c + _DAT_00552844;
          lVar19 = (longlong)(local_328 * 0.0099999998f);   // IDA TERRAIN_INDEX((std::int64_t)(TargetPosition[0]*0.01),...)
          lVar20 = (longlong)(local_324 * 0.0099999998f);   // IDA TERRAIN_INDEX(...,(std::int64_t)(TargetPosition[1]*0.01))
          iVar9 = TERRAIN_INDEX((int)lVar19,(int)lVar20);
          bVar28 = (DAT_0838bc70)[iVar9];
          if ((((bVar28 & 4) != 4) || ((bVar28 & 8) != 8)) || ((bVar28 & 0x10) != 0x10)) {
            fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
            local_320 = (float)(fVar17 + (float10)_DAT_00552540);
            CreateEffect(0xf8,&local_328,&local_340,pfVar15,(float *)0x0,param_1,pfVar14,(float *)0x0,0);
            CreateEffect(0xf9,&local_328,&local_340,pfVar15,(float *)0x0,param_1,pfVar14,(float *)0x0,0);
          }
          local_358 = (float)((int)local_358 + 1);
        } while ((int)local_358 < 5);
      }
      else if (__cnt == 2) {
        pfVar10 = local_38;
        local_1b8 = 0.0;
        local_1b4 = 0;
        local_1b0 = 0;
        local_1ac = 0;
        local_1a8 = 0;
        iVar9 = 5;
        do {
          pfVar10[-1] = AsObject(param_1)->StartPosition[0];
          *pfVar10 = AsObject(param_1)->StartPosition[1];
          pfVar10[1] = AsObject(param_1)->StartPosition[2];
          pfVar10 = pfVar10 + 3;
          iVar9 = iVar9 + -1;
        } while (iVar9 != 0);
        local_358 = 0.0;
        local_348 = 0.0;
        do {
          local_310 = 0.0;
          iVar9 = _rand();
          local_308 = 0;
          local_35c = (float *)(iVar9 % 0xf);
          local_30c = (float)(int)local_35c + _DAT_00552854;
          if (2 < (int)local_348) {
            local_358 = (float)_rand();
          }
          local_344 = 0.0;
          local_288 = (uint)local_358 & 0x80000001;
          if ((int)local_288 < 0) {
            local_288 = (local_288 - 1 | 0xfffffffe) + 1;
          }
          pfVar10 = &local_1b8;
          pfVar15 = local_38;
          do {
            local_35c = pfVar10;
            if (local_288 == 0) {
              iVar9 = _rand();
              local_36c = (float)(iVar9 % 0x1e + 0x32);
            }
            else {
              iVar9 = _rand();
              local_36c = (float)(-0x32 - iVar9 % 0x1e);
            }
            fVar13 = *pfVar10;
            *pfVar10 = (float)(int)local_36c + fVar13;
            iVar9 = _rand();
            local_338 = (float)((iVar9 % 10 + 0x3e) * (int)local_344) +
                        (float)(int)local_36c + fVar13;
            Matrix_BuildFromEuler(&local_340,local_15c);
            Vector_Rotate(&local_310,local_15c,&local_328);
            pfVar14 = pfVar15 + -1;
            *pfVar14 = local_328 + pfVar15[-1];
            *pfVar15 = local_324 + *pfVar15;
            pfVar15[1] = local_320 + pfVar15[1];
            lVar19 = (longlong)(pfVar15[-1] * 0.0099999998f);   // IDA TERRAIN_INDEX((std::int64_t)(*(v74-1)*0.01),...)
            lVar20 = (longlong)(pfVar15[0] * 0.0099999998f);   // IDA TERRAIN_INDEX(...,(std::int64_t)(*v74*0.01))
            iVar9 = TERRAIN_INDEX((int)lVar19,(int)lVar20);
            bVar28 = (DAT_0838bc70)[iVar9];
            if ((((bVar28 & 4) != 4) || ((bVar28 & 8) != 8)) || ((bVar28 & 0x10) != 0x10)) {
              fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
              pfVar15[1] = (float)(fVar17 + (float10)_DAT_00552540);
              local_338 = local_338 + _DAT_00552864;
              CreateEffect(0xfb,pfVar14,&local_340,AsObject(param_1)->Light,(float *)0x0,param_1,(float *)0x64,(float *)0x0,0);
              CreateEffect(0xfc,pfVar14,&local_340,AsObject(param_1)->Light,(float *)0x0,param_1,(float *)0x64,(float *)0x0,0);
              pfVar10 = local_35c;
            }
            pfVar10 = pfVar10 + 1;
            local_344 = (float)((int)local_344 + 1);
            pfVar15 = pfVar15 + 3;
          } while ((int)local_344 < 5);
          local_358 = (float)((int)local_358 + 1);
          local_348 = (float)((int)local_348 + 1);
          local_35c = pfVar10;
        } while ((int)local_348 < 4);
        bVar16 = (char *)*(int *)&AsObject(param_1)->Owner == DAT_07abf5d8;
        *(float *)&AsObject(param_1)->LifeTime = 0.0;
        if (bVar16) {
          Entity_FindNearby_SendPacket((uint)AsObject(param_1)->Skill,AsObject(param_1)->StartPosition,300.0,
                       *(byte *)(param_1 + 0x22),AsObject(param_1)->PKKey);
        }
      }
      else {
        local_358 = (float)__cnt;
        local_2e0 = 15.0;
        if ((9 < __cnt) && (__cnt < 0x10)) {
          local_358 = 12.5;
          local_2e0 = 18.0;
          if (__cnt == 15) {
            AsObject(param_1)->Gravity = AsObject(param_1)->Gravity * _DAT_005526d8;
          }
        }
        if (__cnt == 13) {
          local_340 = 0.0;
          local_33c = 0.0;
          local_338 = 0.0;
          local_310 = -25.0;
          local_30c = -40.0;
          local_308 = 0;
          if (!validOwner(owner)) return;
          Matrix_BuildFromEuler((float *)(*(int *)&AsObject(param_1)->Owner + 0x1c),local_15c);
          Vector_Rotate(&local_310,local_15c,&local_328);
          local_304 = local_328 + AsObject(param_1)->Position[0];
          local_35c = (float *)0x0;
          local_300 = local_324 + AsObject(param_1)->Position[1];
          local_2fc = local_320 + AsObject(param_1)->Position[2];
          do {
            pfVar10 = local_35c;
            local_2bc = local_2fc - (float)(int)local_35c;
            local_2c4 = local_304;
            local_2c0 = local_300;
            CreateEffect(0xfe,&local_2c4,&local_340,AsObject(param_1)->Light,(float *)0x0,(float *)0x0,(float *)0xffffffff,(float *)0x0,0);
            local_35c = (float *)((int)pfVar10 + 0x32);
          } while ((int)local_35c < 200);
          iVar9 = _rand();
          local_304 = (float)(iVar9 % 0x1e + 0x14) + local_304;
          iVar9 = _rand();
          local_35c = (float *)0x0;
          local_2fc = (float)(iVar9 % 500 + -0xfa) + local_2fc;
          do {
            pfVar10 = local_35c;
            local_2bc = local_2fc - (float)(int)local_35c;
            local_2c4 = local_304;
            local_2c0 = local_300;
            CreateEffect(0xfe,&local_2c4,&local_340,AsObject(param_1)->Light,(float *)0x0,(float *)0x0,(float *)0xffffffff,(float *)0x0,0);
            local_35c = (float *)((int)pfVar10 + 0x1e);
          } while ((int)local_35c < 0x78);
          PlayBuffer(0x59,(unsigned int)(uintptr_t)param_1,0);
        }
        fVar13 = AsObject(param_1)->Angle[0] + _DAT_00552878;
        AsObject(param_1)->Position[0] = AsObject(param_1)->StartPosition[0];
        AsObject(param_1)->Position[1] = AsObject(param_1)->StartPosition[1];
        AsObject(param_1)->Position[2] = AsObject(param_1)->StartPosition[2];
        AsObject(param_1)->Angle[0] = fVar13;
        fVar17 = (float10)fsin(((float10)_DAT_005524fc - (float10)local_358) * (float10)local_2e0 *
                               (float10)_DAT_005529f0 * (float10)_DAT_005529e8);
        AsObject(param_1)->Direction[1] = (float)(fVar17 * (float10)_DAT_005529e0);
        if ((local_358 < (float)_DAT_005529d8) || ((float)_DAT_005529d8 < local_358)) {
          fVar13 = AsObject(param_1)->Gravity + _DAT_00552658;
        }
        else {
          fVar13 = AsObject(param_1)->Gravity - _DAT_00552658;
        }
        AsObject(param_1)->Gravity = fVar13;
        Matrix_BuildFromEuler(AsObject(param_1)->HeadAngle,local_15c);
        Vector_Rotate(AsObject(param_1)->Direction,local_15c,&local_328);
        AsObject(param_1)->Position[0] = local_328 + AsObject(param_1)->StartPosition[0];
        AsObject(param_1)->Position[1] = local_324 + AsObject(param_1)->StartPosition[1];
        AsObject(param_1)->Position[2] = local_320 + AsObject(param_1)->StartPosition[2] + AsObject(param_1)->Gravity + _DAT_0055285c;
      }
      break;
    case 0xf5:
      AsObject(param_1)->BlendMeshLight = (float)__cnt * _DAT_005524f4 * _DAT_005529d4;
      if (__cnt < 10) {
        AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] - _DAT_00552504;
      }
      if ((0xf < __cnt) && (__cnt % 3 == 0)) {
        uVar8 = _rand() % 8;
        _DAT_083a0210 = (float)(int)(uVar8 - 4) * _DAT_005524f4;
      }
      break;
    case 0xf6:
      if (_DAT_00552598 < AsObject(param_1)->Scale) {
        *(float *)&AsObject(param_1)->LifeTime = 0.0;
      }
      local_358 = *(float *)&AsObject(param_1)->LifeTime;
      if ((int)local_358 < 10) {
        if ((int)local_358 < 5) {
          AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] - _DAT_00552504;
        }
        AsObject(param_1)->BlendMeshLight = (float)(int)local_358 * _DAT_005524f4;
        if (4 < (int)local_358) {
          iVar9 = _rand();
          if ((float)(iVar9 % 10) == 0.0) {
            local_230 = (float)(iVar9 % 10);
            iVar9 = _rand();
            local_228 = 0;
            local_1dc = 0.0;
            local_1d8 = 0;
            local_22c = (float)(iVar9 % 0x96);
            iVar9 = _rand();
            local_1d4 = (float)(iVar9 % 0x168);
            Matrix_BuildFromEuler(&local_1dc,local_cc);
            Vector_Rotate(&local_230,local_cc,&local_2ac);
            local_2ac = local_2ac + AsObject(param_1)->Position[0];
            pfVar15 = param_1 + 0x3a;
            pfVar10 = param_1 + 7;
            pfVar14 = &local_2ac;
            bVar28 = 0;
            pfVar25 = (float *)0x0;
            local_2a8 = local_2a8 + AsObject(param_1)->Position[1];
            pfVar23 = (float *)0xffffffff;
            pfVar21 = (float *)0x0;
            pfVar22 = (float *)0x0;
            local_2a4 = local_2a4 + AsObject(param_1)->Position[2];
            uVar8 = _rand() % 2;
            CreateEffect(uVar8 + 0xc5,pfVar14,pfVar10,pfVar15,pfVar22,pfVar21,pfVar23,pfVar25,bVar28);
          }
        }
      }
      else {
        AsObject(param_1)->BlendMeshLight = (float)(0x14 - (int)local_358) * _DAT_005524f4;
      }
      goto LAB_00467f78;
    case 0xf7: case 0xfa:
    case 0xf8: case 0xfb: {
      const auto& fade = EffectFadePresets[(iVar9 == 0xf7 || iVar9 == 0xfa) ? 0 : 1];
      AsObject(param_1)->BlendMeshLight = (float)__cnt * _DAT_005524f4 * *fade.AlphaFactor;
      if (__cnt < fade.DescentThreshold)
        AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] - _DAT_00552504;
      break;
    }
    case 0xf9:
      if (__cnt < 0x1e) {
        if (__cnt < 0xf) {
          AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] - _DAT_00552504;
        }
        AsObject(param_1)->BlendMeshLight = (float)__cnt * _DAT_005524f4;
        if (4 < __cnt) {
          iVar9 = _rand();
          if ((float)(iVar9 % 0xf) == 0.0) {
            local_218 = (float)(iVar9 % 0xf);
            iVar9 = _rand();
            local_210 = 0;
            local_1c4 = 0.0;
            local_1c0 = 0;
            local_214 = (float)(iVar9 % 0x96);
            iVar9 = _rand();
            local_1bc = (float)(iVar9 % 0x168);
            Matrix_BuildFromEuler(&local_1c4,local_12c);
            Vector_Rotate(&local_218,local_12c,&local_2dc);
            local_2dc = local_2dc + AsObject(param_1)->Position[0];
            pfVar15 = param_1 + 0x3a;
            pfVar10 = param_1 + 7;
            pfVar14 = &local_2dc;
            bVar28 = 0;
            pfVar25 = (float *)0x0;
            local_2d8 = local_2d8 + AsObject(param_1)->Position[1];
            pfVar23 = (float *)0xffffffff;
            pfVar21 = (float *)0x0;
            pfVar22 = (float *)0x0;
            local_2d4 = local_2d4 + AsObject(param_1)->Position[2];
            uVar8 = _rand() % 2;
            CreateEffect(uVar8 + 0xc5,pfVar14,pfVar10,pfVar15,pfVar22,pfVar21,pfVar23,pfVar25,bVar28);
          }
        }
      }
      else {
        AsObject(param_1)->BlendMeshLight = (float)(0x28 - __cnt) * _DAT_005524f4;
      }
LAB_00467f78:
      local_364 = 0.0;
      local_360 = 0.0;
      iVar9 = 1;
      AsObject(param_1)->BlendMeshTexCoordU = (float)(AsObject(param_1)->LifeTime) * _DAT_005529d0;
LAB_00468736:
      local_368 = local_36c;
      AddTerrainLight(AsObject(param_1)->Position[0],AsObject(param_1)->Position[1], (float*)&local_368,iVar9, (float*)DAT_081cb608);
      break;
    case 0xfc:
      fVar4 = fVar13;
      if (0x1d < __cnt) {
        fVar4 = (float)(0x28 - __cnt);
      }
      AsObject(param_1)->BlendMeshLight = (float)(int)fVar4 * _DAT_005524f4;
      if (__cnt < 0xf) {
        AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] - _DAT_00552504;
      }
      if (0xc < __cnt) {
        _rand();
      }
      local_368 = local_36c;
      AsObject(param_1)->BlendMeshTexCoordU = (float)(AsObject(param_1)->LifeTime) * _DAT_005529d0;
      local_364 = 0.0;
      local_360 = 0.0;
      AddTerrainLight(AsObject(param_1)->Position[0],AsObject(param_1)->Position[1], (float*)&local_368,1, (float*)DAT_081cb608);
      break;
    case 0xfd:
      if (AsObject(param_1)->Scale <= _DAT_0055264c) {
        AsObject(param_1)->Scale = AsObject(param_1)->Scale + _DAT_00552a1c;
        AsObject(param_1)->Position[0] = AsObject(param_1)->Position[0] - _DAT_00552a1c;
        AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] - _DAT_00552a18;
      }
      else {
        AsObject(param_1)->Scale = AsObject(param_1)->Scale + _DAT_005524f4;
        AsObject(param_1)->Position[0] = AsObject(param_1)->Position[0] - _DAT_0055256c;
        AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] - _DAT_005528f0;
        AsObject(param_1)->BlendMeshLight = (float)__cnt * _DAT_00552a20;
      }
LAB_00466d90:
      local_368 = local_36c * _DAT_00552a14;
      iVar9 = 5;
      local_364 = local_368;
      local_360 = local_368;
LAB_004695c0:
      AddTerrainLight(AsObject(param_1)->Position[0],AsObject(param_1)->Position[1], (float*)&local_368,iVar9, (float*)DAT_081cb608);
      break;
    case 0xfe:
      AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] - AsObject(param_1)->Gravity;
      AsObject(param_1)->Gravity = AsObject(param_1)->Gravity + _DAT_0055290c;
      AsObject(param_1)->BlendMeshLight = (float)__cnt * _DAT_00552a10;
      break;
    case 0xff:
      if (!validOwner(owner)) return;
      pcVar11 = (char *)*(int *)&AsObject(param_1)->Owner;
      if (*pcVar11 == '\0') {
        AsObject(param_1)->LifeTime = -1;   // IDA: lifetime = -1
      }
      else {
        pfVar10 = param_1 + 4;
        AsObject(param_1)->Angle[0] = AsObject(pcVar11)->Angle[0];
        AsObject(param_1)->Angle[1] = AsObject(pcVar11)->Angle[1];
        AsObject(param_1)->Angle[2] = AsObject(pcVar11)->Angle[2];
        AsObject(param_1)->HeadAngle[0] = AsObject(pcVar11)->Angle[0];
        AsObject(param_1)->HeadAngle[1] = AsObject(pcVar11)->Angle[1];
        AsObject(param_1)->HeadAngle[2] = AsObject(pcVar11)->Angle[2];
        AsObject(param_1)->StartPosition[0] = *pfVar10;
        AsObject(param_1)->StartPosition[1] = AsObject(param_1)->Position[1];
        fVar4 = AsObject(param_1)->Gravity + _DAT_00552848;
        AsObject(param_1)->StartPosition[2] = AsObject(param_1)->Position[2];
        *pfVar10 = AsObject(pcVar11)->Position[0];
        fVar13 = AsObject(pcVar11)->Position[2];
        AsObject(param_1)->Position[1] = AsObject(pcVar11)->Position[1];
        AsObject(param_1)->Gravity = fVar4;
        AsObject(param_1)->Position[2] = fVar13;
        if (AsObject(param_1)->SubType == 1) {
          local_368 = 1.0;
          local_364 = 1.0;
          local_360 = 1.0;
          Joint_Create(0x4e6,pfVar10,pfVar10,AsObject(param_1)->Angle,3,0,20.0,7,0);
          uVar29 = 0;
          iVar9 = _rand();
          pfVar15 = &local_368;
          fVar13 = (float)(iVar9 % 0x168);
          pfVar14 = param_1;
          uVar8 = _rand() % 8;
          CreateSprite(0x4cf,pfVar10,(float)(int)(uVar8 + 8) * _DAT_005526e4,pfVar15,(int)pfVar14,fVar13,uVar29);
        }
        AsObject(param_1)->HeadAngle[1] = AsObject(param_1)->Gravity;
      }
      break;
    case 0x100:
      uVar8 = _rand() % 2;
      bVar16 = uVar8 == 0;
      if (bVar16) {
        Particle_Spawn(0x4c4,AsObject(param_1)->Position,AsObject(param_1)->Angle,&local_368,0,1.0,0);
      }
      pfVar10 = param_1 + 4;
      Particle_Spawn(0x4ab,pfVar10,AsObject(param_1)->Angle,&local_368,5,1.0,0);
      Particle_Spawn(0x4ab,pfVar10,AsObject(param_1)->Angle,&local_368,5,1.0,0);
      local_368 = local_36c * _DAT_00552534;
      local_364 = local_36c * _DAT_00552530;
      local_360 = local_364;
      AddTerrainLight(*pfVar10,AsObject(param_1)->Position[1], (float*)&local_368,2, (float*)DAT_081cb608);
      break;
    case 0x104:
    case 0x105:
    case 0x106:
    case 0x107: {
      const bool firstPair = iVar9 < 0x106;
      const auto& bounce = EffectBouncePresets[firstPair ? 0 : 1];
      AsObject(param_1)->Position[0] = AsObject(param_1)->Direction[0] + AsObject(param_1)->Position[0];
      AsObject(param_1)->Position[1] = AsObject(param_1)->Position[1] + AsObject(param_1)->Direction[1];
      AsObject(param_1)->Position[2] = AsObject(param_1)->Direction[2] + AsObject(param_1)->Position[2];
      fVar13 = AsObject(param_1)->Direction[0] * _DAT_005526e8;
      AsObject(param_1)->Direction[0] = fVar13;
      local_348 = AsObject(param_1)->Direction[1] * _DAT_005526e8;
      AsObject(param_1)->Direction[1] = local_348;
      local_344 = AsObject(param_1)->Direction[2] * _DAT_005526e8;
      AsObject(param_1)->Direction[2] = local_344;
      AsObject(param_1)->Position[2] = AsObject(param_1)->Position[2] + AsObject(param_1)->Gravity;
      if (AsObject(param_1)->SubType == 1) {
        AsObject(param_1)->Direction[0] = fVar13 * _DAT_005529b4;
        AsObject(param_1)->Direction[1] = local_348 * _DAT_005529b4;
        AsObject(param_1)->Direction[2] = local_344 * _DAT_005529b4;
      }
      AsObject(param_1)->Gravity = AsObject(param_1)->Gravity - *bounce.Gravity;
      fVar17 = RequestTerrainHeight(AsObject(param_1)->Position[0], AsObject(param_1)->Position[1]);
      // Se conservan ambos predicados del port, incluso ante valores no ordenados.
      if (firstPair ? (float10)AsObject(param_1)->Position[2] < fVar17 : !(fVar17 <= (float10)AsObject(param_1)->Position[2])) {
        AsObject(param_1)->Position[2] = (float)fVar17;
        AsObject(param_1)->Gravity = AsObject(param_1)->Gravity * *bounce.Bounce;
        fVar13 = AsObject(param_1)->Scale * _DAT_005529bc;
        AsObject(param_1)->LifeTime = AsObject(param_1)->LifeTime - 5;
      } else {
        fVar13 = AsObject(param_1)->Scale * _DAT_005526e0;
      }
      pfVar10 = param_1 + 7;
      *pfVar10 = *pfVar10 - fVar13;
      iVar9 = _rand();
      if (iVar9 % 10 == 0) {
        Particle_Spawn(0x4c5,&local_354,pfVar10,&local_368,0,1.0,0);
      }
      break;
    }
    case 0x109:
      if (!validOwner(owner)) return;
      if ((((byte)*(undefined4 *)((char *)*(int *)&AsObject(param_1)->Owner + 0x78) & 0x20) == 0x20) &&
         (*(char *)*(int *)&AsObject(param_1)->Owner != '\0')) {
        if (0x32 < __cnt) {
          iVar9 = 3;
          do {
            iVar12 = _rand();
            local_354 = (float)(iVar12 % 0x28 + -0x14);
            iVar12 = _rand();
            local_350 = (float)(iVar12 % 0x28 + -0x14);
            iVar12 = _rand();
            local_334 = 0.0;
            local_330 = 0.0;
            local_34c = (float)(iVar12 % 100);
            iVar12 = _rand();
            bVar28 = 0;
            pfVar25 = (float *)0x0;
            pfVar23 = (float *)0xffffffff;
            pfVar21 = (float *)0x5;
            pfVar10 = &local_354;
            pfVar15 = &local_334;
            local_32c = (float)(iVar12 % 0x168);
            pfVar14 = param_1 + 0x3a;
            pfVar22 = param_1;
            uVar8 = _rand() % 2;
            CreateEffect(uVar8 + 0xe2,pfVar10,pfVar15,pfVar14,pfVar21,pfVar22,pfVar23,pfVar25,bVar28);
            iVar9 = iVar9 + -1;
          } while (iVar9 != 0);
        }
      }
      else {
LAB_0046a85e:
        *(undefined1 *)param_1 = 0;
      }
    } // end switch

switchD_00466b93_caseD_c1:
    sVar3 = AsObject(param_1)->Type;
    if ((((sVar3 != 0xee) && (sVar3 != 0xef)) && (sVar3 != 0xf4)) &&
       (((sVar3 != 0xc5 && (sVar3 != 0xc6)) || (AsObject(param_1)->SubType != 5)))) {
      if ((0xad < sVar3) && (sVar3 < 0x10d)) {
        pvVar1 = (void *)(DAT_05828d58 + sVar3 * 0xbc);
        fVar13 = AsObject(param_1)->Velocity;
        *(undefined1 *)((int)pvVar1 + 0xa0) = AsObject(param_1)->CurrentAction;
        BMD__PlayAnimation(pvVar1,param_1 + 0x42,param_1 + 0x43,(undefined1 *)((int)param_1 + 0x106),fVar13);
      }
      sVar3 = AsObject(param_1)->Type;
      if ((sVar3 < 0x4ba) || (0x4bc < sVar3)) {
        if (sVar3 == 0xd1) {
LAB_0046b3bf:
          iVar9 = 0;
        }
        else {
          if (sVar3 == 0xf3) {
            local_248 = AsObject(param_1)->Angle[0];
            local_244 = AsObject(param_1)->Angle[1];
            local_240 = AsObject(param_1)->Angle[2];
            Matrix_BuildFromEuler(&local_248,local_9c);
            Vector_Rotate(AsObject(param_1)->Direction,local_9c,&local_23c);
            AsObject(param_1)->StartPosition[0] = local_23c + AsObject(param_1)->StartPosition[0];
            AsObject(param_1)->StartPosition[1] = local_238 + AsObject(param_1)->StartPosition[1];
            AsObject(param_1)->StartPosition[2] = local_234 + AsObject(param_1)->StartPosition[2];
            goto LAB_0046b3ca;
          }
          if (sVar3 == 0x10a) goto LAB_0046b3bf;
          iVar9 = 1;
        }
        Joint_BoneOffsetApply((int)param_1,iVar9);
      }
    }
LAB_0046b3ca:
    // IDA decrements the raw DWORD at o+0x60.  It is not an IEEE float:
    // the original tests the resulting signed integer and writes it back as
    // a DWORD, preserving the effect's full lifetime.
    iVar9 = *(int *)((char *)param_1 + 0x60) - 1;
    *(int *)((char *)param_1 + 0x60) = iVar9;
    if (iVar9 < 1) {
      *(undefined1 *)param_1 = 0;
      return;
    }
    if (AsObject(param_1)->Type == 0xbf) {
      if (AsObject(param_1)->SubType != 3) {
        return;
      }
    }
    else {
      if (AsObject(param_1)->Type != 0x47e) {
        return;
      }
      if (*(float *)&AsObject(param_1)->SubType != 0.0) {
        return;
      }
    }
    bVar16 = ((uint)iVar9 & 0x80000001) == 0;
    if ((int)((uint)iVar9 & 0x80000001) < 0) {
      bVar16 = (((uint)iVar9 & 0x80000001) - 1 | 0xfffffffe) == 0xffffffff;
    }
    if (bVar16) {
      return;
    }
  } while( true );
}
