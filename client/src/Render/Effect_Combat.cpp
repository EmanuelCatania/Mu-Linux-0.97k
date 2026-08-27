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
    CreateEffect(v2 + 197, out, Angle, Light, (float*)1, (float*)0, (float*)-1, (float*)0, 0);
    v3 = rand() % 2;
    CreateEffect(v3 + 197, out, Angle, Light, (float*)0, (float*)0, (float*)-1, (float*)0, 0);
    v4 = ++v1;
  }
  while ( v1 < 8 );
}


// RenderWheelWeapon @ 0x0046B7C0 (~101 lines) — renders spinning weapon effect
// Saves object position/rotation, modifies Z + rotation for spinning effect,
// sets up BMD model, animates, RequestTerrainLight, RenderPartObject, restores original.
void __cdecl RenderWheelWeapon_stub(DWORD o) {
    // Save original position and rotation
    float save_posX = *(float*)(o + 0x10);
    float save_posY = *(float*)(o + 0x14);
    float save_posZ = *(float*)(o + 0x18);
    float save_angX = *(float*)(o + 0x1C);
    float save_angY = *(float*)(o + 0x20);
    float save_angZ = *(float*)(o + 0x24);

    // Modify rotation: subtract _DAT_0055284c from Z height offset (o+0xC8)
    float fRot = *(float*)(o + 0xC8) - _DAT_0055284c;
    *(float*)(o + 0xC8) = fRot;

    // Apply rotation offset to angle Z, set angle Y to 90.0f (0x42b40000)
    *(float*)(o + 0x24) = fRot + save_angZ;
    *(float*)(o + 0x20) = 90.0f;

    // Raise Z position by one terrain unit
    *(float*)(o + 0x18) = save_posZ + _DAT_005524f0;

    // Compute model Type from weapon item attribute byte
    BYTE weaponByte = *(BYTE*)(*(int*)(o + 0xFC) + 0x88);
    int Type = (int)weaponByte + 400;

    // Set up BMD model data
    int modelBase = DAT_05828d58 + Type * 0xBC;
    BYTE heroClass = *(BYTE*)(Hero + 0x2B8) & 7;  // Hero->Class & 7
    BYTE animState = *(BYTE*)(o + 0x105);

    *(float*)(modelBase + 0x6C) = *(float*)(o + 0x10);
    *(float*)(modelBase + 0x70) = *(float*)(o + 0x14);
    *(float*)(modelBase + 0x74) = *(float*)(o + 0x18);
    *(BYTE*)(modelBase + 0x98) = heroClass;
    *(BYTE*)(modelBase + 0xA0) = animState;

    // Save and set object type
    short origType = *(short*)(o + 2);
    *(short*)(o + 2) = (short)Type;

    // ItemObjectAttribute — sets up object render attributes
    FUN_00502ba0(o);

    // BMD::Animation — Ghidra shows phantom register params (unaff_EBX/ESI/EDI/EBP);
    // the real call sets up bone matrices for the weapon model.
    // Parameters that depend on phantom regs are passed as zero/defaults.
    // (Animation is driven by the bone matrix pointer at o+0x108 and frame at o+0x10C)

    // RequestTerrainLight — sample terrain lighting at object position
    float terrainLight[3] = { 0.0f, 0.0f, 0.0f };
    FUN_004f7960(*(float*)(o + 0x10), *(float*)(o + 0x14), terrainLight);

    // Add object's own light contribution
    terrainLight[0] += *(float*)(o + 0xE8);
    terrainLight[1] += *(float*)(o + 0xEC);
    terrainLight[2] += *(float*)(o + 0xF0);

    // Light level from item attribute byte
    int lightLevel = (int)(*(BYTE*)(*(int*)(o + 0xFC) + 0x89)) << 3;

    // RenderPartObject(o, Type, NULL, light, alpha=0.0, level=1, opt=1, globalTrans=true, hideSkin=false, translate=true, select, renderType)
    FUN_00505a10(o, Type, 0, terrainLight, 0.0f, 1, 1, 1, 0, 1, 0, 0);

    // Restore original type
    *(short*)(o + 2) = origType;

    // Restore original position and rotation
    *(float*)(o + 0x10) = save_posX;
    *(float*)(o + 0x14) = save_posY;
    *(float*)(o + 0x18) = save_posZ;
    *(float*)(o + 0x1C) = save_angX;
    *(float*)(o + 0x20) = save_angY;
    *(float*)(o + 0x24) = save_angZ;
}

// ItemDrop_RenderGroundWeapon @ 0x0046B980 (~82 lines) — renders grounded weapon model
// If object's height offset (o+0x60) > _DAT_00552488 threshold:
// set up BMD model data, ItemObjectAttribute, RequestTerrainLight, RenderPartObject.
// Similar to RenderWheelWeapon but without position save/restore (static ground item).
// IDA: FUN_0046b980
void __cdecl ItemDrop_RenderGroundWeapon(int param_1) {
    // Only render if height above threshold
    if (_DAT_00552488 >= (float)*(int*)(param_1 + 0x60)) return;

    // Compute model Type from weapon item attribute byte
    BYTE weaponByte = *(BYTE*)(*(int*)(param_1 + 0xFC) + 0x88);
    int Type = (int)weaponByte + 400;

    // Set up BMD model data
    int modelBase = DAT_05828d58 + Type * 0xBC;
    BYTE heroClass = *(BYTE*)(Hero + 0x2B8) & 7;
    BYTE animState = *(BYTE*)(param_1 + 0x105);

    *(float*)(modelBase + 0x6C) = *(float*)(param_1 + 0x10);
    *(float*)(modelBase + 0x70) = *(float*)(param_1 + 0x14);
    *(float*)(modelBase + 0x74) = *(float*)(param_1 + 0x18);
    *(BYTE*)(modelBase + 0x98) = heroClass;
    *(BYTE*)(modelBase + 0xA0) = animState;

    // Save and set object type
    float origTypeF = (float)*(short*)(param_1 + 2);
    *(short*)(param_1 + 2) = (short)Type;

    // Set up object render attributes
    DWORD save_d8 = *(DWORD*)(param_1 + 0xD8);
    FUN_00502ba0(param_1);  // ItemObjectAttribute
    *(DWORD*)(param_1 + 0xD8) = save_d8;  // restore overwritten field

    // RequestTerrainLight — sample terrain lighting at object position
    float terrainLight[3] = { 0.0f, 0.0f, 0.0f };
    FUN_004f7960(*(float*)(param_1 + 0x10), *(float*)(param_1 + 0x14), terrainLight);

    // Add object's own light contribution
    terrainLight[0] += *(float*)(param_1 + 0xE8);
    terrainLight[1] += *(float*)(param_1 + 0xEC);
    terrainLight[2] += *(float*)(param_1 + 0xF0);

    // Light level from item attribute
    int lightLevel = (int)(*(BYTE*)(*(int*)(param_1 + 0xFC) + 0x89)) << 3;

    // BMD::Animation — Ghidra shows phantom register params (unaff_EBX/ESI/EDI);
    // driven by bone matrix pointer at o+0x108 and frame at o+0x10C.
    // Skipping direct call due to __thiscall convention + phantom regs;
    // RenderPartObject below handles the actual render.

    // RenderPartObject
    FUN_00505a10(param_1, Type, 0, terrainLight, 0.0f, 1, 1, 1, 0, 1, 0, 0);

    // Restore original type
    *(short*)(param_1 + 2) = (short)(int)origTypeF;
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
void __cdecl CreateBlood_stub(DWORD o) {
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
            Effect_Create(199, pos, ang, lit, NULL, NULL, NULL, NULL, 0);
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
