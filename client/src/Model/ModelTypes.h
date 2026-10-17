#pragma once
#include "types.h"
#include <cstddef>

// IDA: BMD, 0xBC bytes; array Models (DAT_05828d58), un modelo por tipo.
//
// Los nombres siguen a la clase BMD de MU 5.2 (ZzzBMD.h). Anclajes verificados:
// BMD::Open (0x004423E0) escribe 0x20..0x38; BMD::Init (0x00442E00) pone
// BoneHead (0x54) y StreamMesh (0x88) en -1; RenderObjectScreen (0x004E13A0)
// escribe BodyHeight (0x84); BMD_Animation (0x00440060) usa CurrentAction
// (0xA0); SetPlayerWalk/Attack/Die leen Sounds (0xAA). Los campos entre
// anclajes siguen el orden del 5.2, que encaja exacto con ellos; la única
// diferencia es el puntero LightMaps en 0x40, que el 5.2 ya no tiene.
#pragma pack(push, 1)
struct BMD {
    char     Name[32];              // 0x00
    char     Version;               // 0x20
    BYTE     _unk21;
    short    NumBones;              // 0x22
    short    NumMeshs;              // 0x24
    short    NumActions;            // 0x26
    BYTE*    Meshs;                 // 0x28  Mesh_t[NumMeshs], 40 bytes cada uno
    BYTE*    Bones;                 // 0x2C  Bone_t[NumBones], 140 bytes cada uno
    BYTE*    Actions;               // 0x30  Action_t[NumActions], 16 bytes cada uno
    BYTE*    Textures;              // 0x34  Texture_t[NumMeshs], 32 bytes cada uno
    short*   IndexTexture;          // 0x38  en el 0.97k son shorts (new 2 * NumMeshs)
    short    NumLightMaps;          // 0x3C
    short    IndexLightMap;         // 0x3E
    BYTE*    LightMaps;             // 0x40
    BYTE     LightEnable;           // 0x44
    BYTE     ContrastEnable;        // 0x45
    BYTE     _unk46[2];
    vec3_t   BodyLight;             // 0x48
    int      BoneHead;              // 0x54
    int      BoneFoot[4];           // 0x58
    float    BodyScale;             // 0x68
    vec3_t   BodyOrigin;            // 0x6C
    vec3_t   BodyAngle;             // 0x78
    float    BodyHeight;            // 0x84
    char     StreamMesh;            // 0x88
    BYTE     _unk89[3];
    vec3_t   ShadowAngle;           // 0x8C
    char     Skin;                  // 0x98
    BYTE     HideSkin;              // 0x99
    BYTE     _unk9A[2];
    float    Velocity;              // 0x9C
    BYTE     CurrentAction;         // 0xA0
    BYTE     PriorAction;           // 0xA1
    BYTE     _unkA2[2];
    float    CurrentAnimation;      // 0xA4
    short    CurrentAnimationFrame; // 0xA8
    short    Sounds[5];             // 0xAA  paso x2, ataque x2, muerte
    BYTE     _unkB4[8];
};
#pragma pack(pop)

#define MU_ASSERT_FIELD(T, f, off) static_assert(offsetof(T, f) == off, #T "::" #f)
static_assert(sizeof(BMD) == 0xBC, "BMD");
MU_ASSERT_FIELD(BMD, Version, 0x20);
MU_ASSERT_FIELD(BMD, NumBones, 0x22);
MU_ASSERT_FIELD(BMD, NumMeshs, 0x24);
MU_ASSERT_FIELD(BMD, NumActions, 0x26);
MU_ASSERT_FIELD(BMD, Meshs, 0x28);
MU_ASSERT_FIELD(BMD, Bones, 0x2C);
MU_ASSERT_FIELD(BMD, Actions, 0x30);
MU_ASSERT_FIELD(BMD, Textures, 0x34);
MU_ASSERT_FIELD(BMD, IndexTexture, 0x38);
MU_ASSERT_FIELD(BMD, LightMaps, 0x40);
MU_ASSERT_FIELD(BMD, BodyLight, 0x48);
MU_ASSERT_FIELD(BMD, BoneHead, 0x54);
MU_ASSERT_FIELD(BMD, BodyScale, 0x68);
MU_ASSERT_FIELD(BMD, BodyOrigin, 0x6C);
MU_ASSERT_FIELD(BMD, BodyAngle, 0x78);
MU_ASSERT_FIELD(BMD, BodyHeight, 0x84);
MU_ASSERT_FIELD(BMD, StreamMesh, 0x88);
MU_ASSERT_FIELD(BMD, Velocity, 0x9C);
MU_ASSERT_FIELD(BMD, CurrentAction, 0xA0);
MU_ASSERT_FIELD(BMD, CurrentAnimationFrame, 0xA8);
MU_ASSERT_FIELD(BMD, Sounds, 0xAA);
#undef MU_ASSERT_FIELD

inline BMD* AsModel(uintptr_t p) { return reinterpret_cast<BMD*>(p); }
inline BMD* AsModel(void* p) { return static_cast<BMD*>(p); }
inline const BMD* AsModel(const void* p) { return static_cast<const BMD*>(p); }
