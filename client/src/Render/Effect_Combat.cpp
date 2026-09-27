// Effect_Combat.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// Effect_SpawnBombRing @ 0x00466300 (~166 lines) — bomb/explosion ring particle effect
// Creates ring of 8 particles evenly spaced around a circle via AngleMatrix + VectorRotate.
// For each: CreateBomb at rotated position, then 2x CreateEffect (type 0xC5 or 0xC6, random).
// param_1 = float[3] center position
// IDA: FUN_00466300
void __cdecl Effect_SpawnBombRing(float *a1)
{
  int v1; // edi
  int v2; // eax
  int v3; // eax
  int v4; // [esp+8h] [ebp-64h]
  float out[3]; // [esp+Ch] [ebp-60h] BYREF
  float Angle[3]; // [esp+18h] [ebp-54h] BYREF
  float Light[3]; // [esp+24h] [ebp-48h] BYREF
  float in1[3]; // [esp+30h] [ebp-3Ch] BYREF
  float in2[3][4]; // [esp+3Ch] [ebp-30h] BYREF

  v1 = 0;
  Light[0] = 1.0;
  Light[1] = 1.0;
  Light[2] = 1.0;
  v4 = 0;
  do
  {
    in1[0] = 0.0;
    in1[1] = -220.0;
    in1[2] = 0.0;
    Angle[0] = 0.0;
    Angle[1] = 0.0;
    Angle[2] = (double)v4 * 45.0;
    AngleMatrix(Angle, in2);
    VectorRotate(in1, (float*)in2, out);
    out[0] = out[0] + *a1;
    out[1] = out[1] + a1[1];
    out[2] = out[2] + a1[2];
    CreateBomb(out, 1);
    v2 = rand() % 2;
    void *r197 = CreateEffect(v2 + 197, out, Angle, Light, (float*)1, (float*)0, (float*)-1, (float*)0, 0);
    v3 = rand() % 2;
    void *r198 = CreateEffect(v3 + 197, out, Angle, Light, (float*)0, (float*)0, (float*)-1, (float*)0, 0);
    v4 = ++v1;
  }
  while ( v1 < 8 );
}


// RenderWheelWeapon vive en Render_WorldHelpers.cpp.
//
// 2026-09-26: aca habia una copia bajo el nombre RenderWheelWeapon.  Las dos
// implementaciones son equivalentes; se deja una sola, con el nombre de IDA.

// ItemDrop_RenderGroundWeapon @ 0x0046B980 (sub_46B980, 377 bytes)
// Renderer propio del efecto 244 (Rageful Blow): RenderEffects lo aparta del
// rango generico y lo dibuja por aca.  Toma el slot del efecto, le cambia el
// Type por el modelo del ARMA del dueno (owner[136] + 400), lo posa con la
// animacion del efecto y lo restaura.  Hermana de RenderWheelWeapon (0x46B7C0),
// pero sin el save/restore de posicion: este no mueve el slot.
//
// 2026-09-26: la version anterior de este port estaba rota en cuatro puntos y
// por eso el Rageful Blow no mostraba el arma:
//   - el byte de clase se leia de Hero+0x2B8 (helper/pet) en vez de Hero+444;
//   - alpha se pasaba como 0.0f, y RenderPartObject (0x505A10) sale temprano
//     con `if (_DAT_005524f8 < param_5)` -> con 0 no dibujaba NADA;
//   - los argumentos 6..12 de RenderPartObject estaban corridos;
//   - faltaba BMD_Animation, o sea el arma nunca se posaba.
void __cdecl ItemDrop_RenderGroundWeapon(int o) {
    // IDA: if ((double)*(int *)(o + 96) > 10.0)  -- o+96 es la vida del efecto.
    if (_DAT_00552488 >= (float)*(int*)(o + 96)) return;

    const DWORD owner = *(DWORD*)(o + 252);
    if (!owner || !DAT_05828d58 || !DAT_07abf5d8) return;

    const int model_id = *(unsigned char*)(owner + 136) + 400;
    if (model_id < 0 || model_id >= 1024) return;

    const float alpha = *(float*)(o + 360);
    BYTE* mdl = (BYTE*)(uintptr_t)DAT_05828d58 + 188 * model_id;

    // Volcar posicion / accion / clase al slot del modelo compartido.
    *(int*) (mdl + 108) = *(int*)(o + 16);
    *(int*) (mdl + 112) = *(int*)(o + 20);
    *(int*) (mdl + 116) = *(int*)(o + 24);
    *(BYTE*)(mdl + 152) = (BYTE)(*(BYTE*)((uintptr_t)DAT_07abf5d8 + 444) & 7);
    *(BYTE*)(mdl + 160) = *(BYTE*)(o + 261);

    const short savedType = *(short*)(o + 2);
    *(short*)(o + 2) = (short)model_id;

    const DWORD saved216 = *(DWORD*)(o + 216);
    ItemObjectAttribute(o);
    *(DWORD*)(o + 216) = saved216;   // ItemObjectAttribute lo pisa

    float Light[3];
    RequestTerrainLight(*(float*)(o + 16), *(float*)(o + 20), Light);
    Light[0] += *(float*)(o + 232);
    Light[1] += *(float*)(o + 236);
    Light[2] += *(float*)(o + 240);

    float Angle[3] = { *(float*)(o + 28), *(float*)(o + 32), *(float*)(o + 36) };
    BMD_Animation(mdl, (int)&DAT_06970a9c,
                  *(float*)(o + 264), *(unsigned int*)(o + 268),
                  *(BYTE*)(o + 262),
                  (unsigned int*)Angle, (float*)(o + 40), 0, 0);

    RenderPartObject(o, model_id, 0, Light, alpha,
                     (unsigned int)(8 * *(unsigned char*)(owner + 137)),
                     0, 1, 1, 1, 0, 2);

    *(short*)(o + 2) = savedType;
}

// IDA compatibility bridges: stubs_IDA_ports.cpp intentionally retains these ABI names.
void __cdecl FUN_00466300(float* position)
{
    Effect_SpawnBombRing(position);
}

void __cdecl FUN_0046b980(int entity)
{
    ItemDrop_RenderGroundWeapon(entity);
}

// FUN_0046c5a0 @ 0x0046C5A0 (~117 lines) — skill impact particle burst
// AngleMatrix from direction (param_4), VectorRotate to get offset.
// Spawn one main particle (0x498) at param_3 position,
// then 20 trail particles (0x497) at the rotated offset position.
// param_1 = unused (Ghidra phantom), param_2 = entity base (angles at +0x1C)
// param_3 = float[3] position, param_4 = float[3] direction angles
void __cdecl FUN_0046c5a0(int param_1, int param_2, float* param_3, float* param_4) {
    (void)param_1;

    float light[3] = { 1.0f, 1.0f, 1.0f };
    float angle[3] = { 1.0f, 0.0f, 0.0f };  // scale=1.0 in angle[0]

    // Spawn main impact particle (type 0x498) at param_3 position
    Particle_Spawn(0x498, param_3, angle, light, 0, 0.0f, 0);

    // Build rotation matrix from direction angles
    float dirAngles[3] = { param_4[0], param_4[1], param_4[2] };
    // Ghidra: local_3c=0, local_38=50.0f (0x42480000), local_34=0  → offset vector (0, 50, 0)
    float localOffset[3] = { 0.0f, 50.0f, 0.0f };
    float matrix[3][4];
    AngleMatrix(dirAngles, matrix);

    // Rotate offset vector by direction matrix
    float worldOffset[3];
    Vector_Rotate(localOffset, (float*)matrix, worldOffset);

    // Final trail spawn position = rotated offset + source position
    float trailPos[3];
    trailPos[0] = worldOffset[0] + param_3[0];
    trailPos[1] = worldOffset[1] + param_3[1];
    trailPos[2] = worldOffset[2] + param_3[2];

    // Spawn 20 trail particles (type 0x497) at the offset position
    for (int i = 0; i < 20; i++) {
        rand();  // random variation X
        rand();  // random variation Y
        Particle_Spawn(0x497, param_3, angle, light, 0, 0.0f, 0);
    }
}

// CreateBlood @ 0x0046C680 (~58 lines) — blood particles on entity hit/death
// Type 0x11d: 10 effect(199) particles + kill entity.
// Others: BMD bone transform to position blood at hit location.
// Models base = DAT_05828d58, stride 0xbc. OBJECT struct (Ghidra): Type@+0x02, Live@+0x00,
// Position@+0x10, Angle@+0x1C, Light@+0xE8, BoneTransform2@+0x114.
void __cdecl CreateBlood(DWORD o) {
    WORD wType = *(WORD*)(o + 0x02);  // o->Type
    // Models[type].Data + 0x54 = bone index for blood attachment point
    int boneIdx = *(int*)(DAT_05828d58 + (short)wType * 0xbc + 0x54);
    if (boneIdx == -1) return;

    if (wType == 0x11d) {
        // Kill entity and spawn 10 death-blood effect particles (type 199)
        *(bool*)(o + 0x00) = false;  // o->Live = false
        int count = 10;
        do {
            // CreateEffect(199, o->Position, o->Angle, o->Light, ...)
            // Ghidra shows phantom register args for the trailing params;
            // pass position/angle/light arrays from the OBJECT.
            float pos[3] = { *(float*)(o + 0x10), *(float*)(o + 0x14), *(float*)(o + 0x18) };
            float ang[3] = { *(float*)(o + 0x1C), *(float*)(o + 0x20), *(float*)(o + 0x24) };
            float lit[3] = { *(float*)(o + 0xE8), *(float*)(o + 0xEC), *(float*)(o + 0xF0) };
            CreateEffect(199, pos, ang, lit, NULL, NULL, NULL, NULL, 0);
            count = count - 1;
        } while (count != 0);
        return;
    }

    // Skip blood for specific monster types (skeletal/transparent models)
    if ((wType == 0x115) || (wType == 0x11c) || (wType == 0x120)) return;

    // Normal blood: transform a random bone-local offset into world space
    // via BMD::TransformPosition, then spawn blood particle at that position.
    float localPos[3];
    float worldPos[3];

    rand();  // random X bone-local offset
    rand();  // random Z bone-local offset

    localPos[0] = 0.0f;
    localPos[1] = 0.0f;
    localPos[2] = 0.0f;
    worldPos[0] = 0.0f;
    worldPos[1] = 0.0f;
    worldPos[2] = 0.0f;

    // BMD::TransformPosition(Models + type, o->BoneTransform2[boneIdx], localPos, worldPos, Translate)
    // BoneTransform2 is a pointer at o+0x114; each bone matrix is float[3][4] = 48 bytes
    float* boneMatrix = (float*)(*(DWORD*)(o + 0x114) + boneIdx * 48);
    void* model = (void*)(DAT_05828d58 + (short)wType * 0xbc);
    BMD__TransformPosition(model, (float(*)[4])boneMatrix, localPos, worldPos, true);
}
