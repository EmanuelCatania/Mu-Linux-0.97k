#pragma once
#include "types.h"
#include "Entity/EntityTypes.h"
#include <cstddef>

// IDA: JOINT, 0x9D8 bytes; pool de 500 en 0x07B27150 (DAT_07b27150).
//
// Los nombres siguen al struct JOINT de MU 5.2 (_struct.h). El 0.97k no tiene
// TexType, RenderType/RenderFace, HeadAngle ni byOnlyOneRender, y Skill es un
// byte. Anclajes en IDA: CreateJoint (0x0046D840) escribe Live, Type, SubType,
// Position, Angle, Light (1,1,1), Target/TargetPosition, NumTails = 0,
// Tails[0] (los cuatro vértices del primer segmento), LifeTime (+0x9B8),
// Collision (+0x9BC), Velocity (+0x9C0), PKKey (+0x9D0) y Skill (+0x9D2);
// MoveJoint (0x00470030) usa StartPosition como paso por tick, Direction
// (+0x9C4) con AngleMatrix, y pasa Skill y Weapon (+0x9D3) a sub_45FEC0;
// sub_46FE90 desplaza Tails hasta MaxTails (+0x54).
constexpr int MAX_JOINT_TAILS = 50;

#pragma pack(push, 1)
struct JOINT {
    BYTE     Live;                         // 0x000
    BYTE     _unk001[3];
    int      Type;                         // 0x004
    int      SubType;                      // 0x008
    float    Scale;                        // 0x00C
    vec3_t   Position;                     // 0x010
    vec3_t   StartPosition;                // 0x01C
    vec3_t   Angle;                        // 0x028
    vec3_t   Light;                        // 0x034
    OBJECT*  Target;                       // 0x040
    vec3_t   TargetPosition;               // 0x044
    int      NumTails;                     // 0x050
    int      MaxTails;                     // 0x054
    vec3_t   Tails[MAX_JOINT_TAILS][4];    // 0x058
    int      LifeTime;                     // 0x9B8
    BYTE     Collision;                    // 0x9BC
    BYTE     _unk9BD[3];
    float    Velocity;                     // 0x9C0
    vec3_t   Direction;                    // 0x9C4
    short    PKKey;                        // 0x9D0
    BYTE     Skill;                        // 0x9D2
    BYTE     Weapon;                       // 0x9D3
    int      MultiUse;                     // 0x9D4
};
#pragma pack(pop)

#define MU_ASSERT_FIELD(T, f, off) static_assert(offsetof(T, f) == off, #T "::" #f)
static_assert(sizeof(JOINT) == 0x9D8, "JOINT");
MU_ASSERT_FIELD(JOINT, Type, 0x004);
MU_ASSERT_FIELD(JOINT, SubType, 0x008);
MU_ASSERT_FIELD(JOINT, Scale, 0x00C);
MU_ASSERT_FIELD(JOINT, Position, 0x010);
MU_ASSERT_FIELD(JOINT, StartPosition, 0x01C);
MU_ASSERT_FIELD(JOINT, Angle, 0x028);
MU_ASSERT_FIELD(JOINT, Light, 0x034);
MU_ASSERT_FIELD(JOINT, Target, 0x040);
MU_ASSERT_FIELD(JOINT, TargetPosition, 0x044);
MU_ASSERT_FIELD(JOINT, NumTails, 0x050);
MU_ASSERT_FIELD(JOINT, MaxTails, 0x054);
MU_ASSERT_FIELD(JOINT, Tails, 0x058);
MU_ASSERT_FIELD(JOINT, LifeTime, 0x9B8);
MU_ASSERT_FIELD(JOINT, Collision, 0x9BC);
MU_ASSERT_FIELD(JOINT, Velocity, 0x9C0);
MU_ASSERT_FIELD(JOINT, Direction, 0x9C4);
MU_ASSERT_FIELD(JOINT, PKKey, 0x9D0);
MU_ASSERT_FIELD(JOINT, Skill, 0x9D2);
MU_ASSERT_FIELD(JOINT, Weapon, 0x9D3);
MU_ASSERT_FIELD(JOINT, MultiUse, 0x9D4);
#undef MU_ASSERT_FIELD

inline JOINT* AsJoint(uintptr_t p) { return reinterpret_cast<JOINT*>(p); }
inline JOINT* AsJoint(void* p) { return static_cast<JOINT*>(p); }
inline const JOINT* AsJoint(const void* p) { return static_cast<const JOINT*>(p); }
