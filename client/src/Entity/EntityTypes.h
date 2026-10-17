#pragma once
#include "Platform/PlatformHeaders.h"
#include "types.h"
#include <cstddef>

// Layout de las entidades del 0.97k (OBJECT 0x1BC, CHARACTER 0x394).
//
// Los nombres siguen a MU 5.2 (ZzzCharacter.cpp / w_ObjectInfo.h). Cada campo
// se verificó emparejando sentencia por sentencia CreateCharacterPointer
// (0x0045ADC0) con su equivalente del 5.2, o con los writers en IDA que se
// citan al lado. Lo que todavía no está verificado queda como relleno `_unkXXX`:
// no se nombra un offset por suposición.
//
// El layout es el del binario x86; los static_assert fallan si algo se corre.

#pragma pack(push, 1)

// IDA: OBJECT, 0x1BC bytes. Lo usan personajes, efectos y objetos del mapa
// (los pools de efectos y objetos tienen paso 0x1BC).
struct OBJECT {
    BYTE     Live;                  // 0x000
    BYTE     _unk001;
    short    Type;                  // 0x002
    int      SubType;               // 0x004
    BYTE     _unk008[4];
    float    Scale;                 // 0x00C
    vec3_t   Position;              // 0x010
    vec3_t   Angle;                 // 0x01C  Angle[2] = orientación
    vec3_t   HeadAngle;             // 0x028  IDA: BMD_Animation (0x00440060)
    vec3_t   HeadTargetAngle;       // 0x034  IDA: MoveCharacterVisual (0x004520C0)
    BYTE     _unk040[0x18];
    int      HiddenMesh;            // 0x058
    BYTE     _unk05C[4];
    int      LifeTime;              // 0x060  IDA: MoveEffect (0x00466AD0)
    int      BlendMesh;             // 0x064
    float    BlendMeshLight;        // 0x068
    float    BlendMeshTexCoordU;    // 0x06C
    float    BlendMeshTexCoordV;    // 0x070
    BYTE     RenderType;            // 0x074  IDA: CreateEffect (0x00460DC0)
    BYTE     _unk075[7];
    BYTE     Teleport;              // 0x07C
    BYTE     _unk07D[3];
    float    Timer;                 // 0x080
    BYTE     Kind;                  // 0x084
    BYTE     Skill;                 // 0x085  IDA: CreateEffect (0x00460DC0)
    short    PKKey;                 // 0x086  IDA: CreateEffect (0x00460DC0)
    BYTE     _unk088[4];
    BYTE     EnableShadow;          // 0x08C
    BYTE     _unk08D[0x33];
    vec3_t   Direction;             // 0x0C0  IDA: CreateEffect (0x00460DC0)
    float    Velocity;              // 0x0CC
    float    CollisionRange;        // 0x0D0
    float    ShadowScale;           // 0x0D4
    float    Gravity;               // 0x0D8  IDA: CreateEffect (0x00460DC0)
    BYTE     LightEnable;           // 0x0DC
    BYTE     _unk0DD[7];
    BYTE     ContrastEnable;        // 0x0E4
    BYTE     _unk0E5[3];
    vec3_t   Light;                 // 0x0E8
    int      AttackPoint;           // 0x0F4  5.2: AttackPoint[0]
    BYTE     _unk0F8[4];
    OBJECT*  Owner;                 // 0x0FC  IDA: CreateEffect (0x00460DC0)
    BYTE     _unk100[5];
    BYTE     CurrentAction;         // 0x105
    BYTE     PriorAction;           // 0x106
    BYTE     _unk107;
    float    AnimationFrame;        // 0x108
    float    PriorAnimationFrame;   // 0x10C
    BYTE     EnableBoneMatrix;      // 0x110
    BYTE     _unk111[3];
    vec34_t* BoneMatrices;          // 0x114  5.2: BoneTransform; una matriz 3x4 por hueso
    vec3_t   BoundingBoxMin;        // 0x118
    vec3_t   BoundingBoxMax;        // 0x124
    float    OBB[12];               // 0x130  IDA: sub_4B0240 (picking): centro + 3 semiejes
    BYTE     Visible;               // 0x160  lo escribe Terrain_Render en cada frame
    BYTE     AlphaEnable;           // 0x161
    BYTE     _unk162[2];
    float    AlphaTarget;           // 0x164
    float    Alpha;                 // 0x168
    BYTE     _unk16C[4];
    vec3_t   StartPosition;         // 0x170  IDA: CreateEffect (0x00460DC0)
    int      InitialSceneFrame;     // 0x17C  MoveSceneFrame del spawn
    BYTE     _unk180[0x3C];
};

// IDA: PART_t, 24 bytes. Arma, pieza del cuerpo, ala, helper o flag.
struct PART_t {
    short    Type;                  // 0x00  -1 = vacío
    BYTE     Level;                 // 0x02
    BYTE     Option1;               // 0x03
    BYTE     LinkBone;              // 0x04  hueso donde se cuelga
    BYTE     CurrentAction;         // 0x05
    BYTE     PriorAction;           // 0x06
    BYTE     _unk07;
    float    AnimationFrame;        // 0x08
    float    PriorAnimationFrame;   // 0x0C
    float    PlaySpeed;             // 0x10  IDA: RenderCharacter (alas, 1.0 / 0.25)
    BYTE     _unk14[4];
};

// IDA: CHARACTER, 0x394 bytes; pool CharactersClient (400 entradas).
struct CHARACTER {
    OBJECT   Object;                // 0x000
    BYTE     Class;                 // 0x1BC  (Class & 7) = clase, (Class >> 3) = 2da clase
    BYTE     Skin;                  // 0x1BD  IDA: SetCharacterScale (0x0045C050)
    WORD     Level;                 // 0x1BE
    BYTE     _unk1C0;
    char     ID[11];                // 0x1C1  nombre
    BYTE     _unk1CC[0x0E];
    short    GuildMarkIndex;        // 0x1DA  -1 = sin guild
    WORD     Key;                   // 0x1DC  índice de red
    BYTE     _unk1DE[2];
    PART_t   BodyPart[6];           // 0x1E0  casco, armadura, pantalón, guantes, botas, ...
    PART_t   Weapon[2];             // 0x270
    PART_t   Wing;                  // 0x2A0
    PART_t   Helper;                // 0x2B8  816..819 = pet equipado
    PART_t   Flag;                  // 0x2D0
    BYTE     EtcPart;               // 0x2E8  arma del evento colgada en la espalda
    BYTE     GuildTeam;             // 0x2E9
    BYTE     PK;                    // 0x2EA  3 = neutral
    BYTE     MonsterIndex;          // 0x2EB  tipo de Monster.txt; 0xFF = jugador
    BYTE     Movement;              // 0x2EC
    BYTE     MovementType;          // 0x2ED  acción encolada (4 = MOVEMENT_OPERATE)
    BYTE     _unk2EE[7];
    BYTE     SkillStage;            // 0x2F5  IDA: AttackStage (0x00448930): 0/1/8/15
    BYTE     _unk2F6[4];
    WORD     MoveSpeed;             // 0x2FA
    BYTE     _unk2FC;
    BYTE     Dead;                  // 0x2FD  IDA: ReceiveDie (0x0042F030)
    BYTE     Blood;                 // 0x2FE
    BYTE     _unk2FF;
    BYTE     Run;                   // 0x300  contador de stamina (>= 0x28 = cansado)
    BYTE     SkillSuccess;          // 0x301  IDA: Skills_PacketHandler (0x0042BCA0)
    BYTE     Skill;                 // 0x302  skill encolado
    BYTE     SwordCount;            // 0x303  IDA: SetPlayerAttack (0x00444410), variante de la animación
    BYTE     _unk304;
    BYTE     JumpTime;              // 0x305
    BYTE     TargetX;               // 0x306
    BYTE     TargetY;               // 0x307
    BYTE     _unk308[8];
    short    TargetCharacter;       // 0x310
    BYTE     _unk312[2];
    vec3_t   TargetPosition;        // 0x314  IDA: UseSkillWarrior (0x00485780); destino de los joints de c->LongRangeAttack
    vec3_t   Light;                 // 0x320
    BYTE     CollisionTime;         // 0x32C
    BYTE     _unk32D[3];
    int      LongRangeAttack;       // 0x330
    float    Rot;                   // 0x334
    BYTE     _unk338[4];
    int      Item;                  // 0x33C
    BYTE     _unk340[0x0E];
    BYTE     SafeZone;              // 0x34E  TerrainWall & 1 en la casilla actual
    BYTE     Change;                // 0x34F
    BYTE     _unk350[4];
    BYTE     PathCurrent;           // 0x354  waypoint actual
    BYTE     PathSubstep;           // 0x355
    BYTE     PathNum;               // 0x356  cantidad de waypoints
    BYTE     PathX[15];             // 0x357
    BYTE     PathY[15];             // 0x366
    BYTE     _unk375[0x13];
    int      PositionX;             // 0x388  casilla (el binario la cifra con la hash-table)
    int      PositionY;             // 0x38C
    BYTE     _unk390[4];
};

#pragma pack(pop)

#define MU_ASSERT_FIELD(T, f, off) static_assert(offsetof(T, f) == off, #T "::" #f)
static_assert(sizeof(OBJECT) == 0x1BC, "OBJECT");
static_assert(sizeof(PART_t) == 0x18, "PART_t");
static_assert(sizeof(CHARACTER) == 0x394, "CHARACTER");
MU_ASSERT_FIELD(OBJECT, Type, 0x002);
MU_ASSERT_FIELD(OBJECT, Scale, 0x00C);
MU_ASSERT_FIELD(OBJECT, Position, 0x010);
MU_ASSERT_FIELD(OBJECT, HiddenMesh, 0x058);
MU_ASSERT_FIELD(OBJECT, LifeTime, 0x060);
MU_ASSERT_FIELD(OBJECT, BlendMesh, 0x064);
MU_ASSERT_FIELD(OBJECT, RenderType, 0x074);
MU_ASSERT_FIELD(OBJECT, Teleport, 0x07C);
MU_ASSERT_FIELD(OBJECT, Timer, 0x080);
MU_ASSERT_FIELD(OBJECT, Kind, 0x084);
MU_ASSERT_FIELD(OBJECT, Skill, 0x085);
MU_ASSERT_FIELD(OBJECT, PKKey, 0x086);
MU_ASSERT_FIELD(OBJECT, EnableShadow, 0x08C);
MU_ASSERT_FIELD(OBJECT, Direction, 0x0C0);
MU_ASSERT_FIELD(OBJECT, Velocity, 0x0CC);
MU_ASSERT_FIELD(OBJECT, Gravity, 0x0D8);
MU_ASSERT_FIELD(OBJECT, LightEnable, 0x0DC);
MU_ASSERT_FIELD(OBJECT, ContrastEnable, 0x0E4);
MU_ASSERT_FIELD(OBJECT, Light, 0x0E8);
MU_ASSERT_FIELD(OBJECT, AttackPoint, 0x0F4);
MU_ASSERT_FIELD(OBJECT, Owner, 0x0FC);
MU_ASSERT_FIELD(OBJECT, CurrentAction, 0x105);
MU_ASSERT_FIELD(OBJECT, AnimationFrame, 0x108);
MU_ASSERT_FIELD(OBJECT, EnableBoneMatrix, 0x110);
MU_ASSERT_FIELD(OBJECT, BoneMatrices, 0x114);
MU_ASSERT_FIELD(OBJECT, BoundingBoxMin, 0x118);
MU_ASSERT_FIELD(OBJECT, BoundingBoxMax, 0x124);
MU_ASSERT_FIELD(OBJECT, OBB, 0x130);
MU_ASSERT_FIELD(OBJECT, Visible, 0x160);
MU_ASSERT_FIELD(OBJECT, AlphaTarget, 0x164);
MU_ASSERT_FIELD(OBJECT, Alpha, 0x168);
MU_ASSERT_FIELD(OBJECT, StartPosition, 0x170);
MU_ASSERT_FIELD(OBJECT, InitialSceneFrame, 0x17C);
MU_ASSERT_FIELD(PART_t, LinkBone, 0x04);
MU_ASSERT_FIELD(PART_t, AnimationFrame, 0x08);
MU_ASSERT_FIELD(PART_t, PlaySpeed, 0x10);
MU_ASSERT_FIELD(CHARACTER, Class, 0x1BC);
MU_ASSERT_FIELD(CHARACTER, Level, 0x1BE);
MU_ASSERT_FIELD(CHARACTER, ID, 0x1C1);
MU_ASSERT_FIELD(CHARACTER, GuildMarkIndex, 0x1DA);
MU_ASSERT_FIELD(CHARACTER, Key, 0x1DC);
MU_ASSERT_FIELD(CHARACTER, BodyPart, 0x1E0);
MU_ASSERT_FIELD(CHARACTER, Weapon, 0x270);
MU_ASSERT_FIELD(CHARACTER, Wing, 0x2A0);
MU_ASSERT_FIELD(CHARACTER, Helper, 0x2B8);
MU_ASSERT_FIELD(CHARACTER, Flag, 0x2D0);
MU_ASSERT_FIELD(CHARACTER, EtcPart, 0x2E8);
MU_ASSERT_FIELD(CHARACTER, MonsterIndex, 0x2EB);
MU_ASSERT_FIELD(CHARACTER, MovementType, 0x2ED);
MU_ASSERT_FIELD(CHARACTER, SkillStage, 0x2F5);
MU_ASSERT_FIELD(CHARACTER, MoveSpeed, 0x2FA);
MU_ASSERT_FIELD(CHARACTER, Dead, 0x2FD);
MU_ASSERT_FIELD(CHARACTER, Run, 0x300);
MU_ASSERT_FIELD(CHARACTER, Skill, 0x302);
MU_ASSERT_FIELD(CHARACTER, SwordCount, 0x303);
MU_ASSERT_FIELD(CHARACTER, JumpTime, 0x305);
MU_ASSERT_FIELD(CHARACTER, TargetX, 0x306);
MU_ASSERT_FIELD(CHARACTER, TargetCharacter, 0x310);
MU_ASSERT_FIELD(CHARACTER, TargetPosition, 0x314);
MU_ASSERT_FIELD(CHARACTER, Light, 0x320);
MU_ASSERT_FIELD(CHARACTER, CollisionTime, 0x32C);
MU_ASSERT_FIELD(CHARACTER, LongRangeAttack, 0x330);
MU_ASSERT_FIELD(CHARACTER, Item, 0x33C);
MU_ASSERT_FIELD(CHARACTER, SafeZone, 0x34E);
MU_ASSERT_FIELD(CHARACTER, PathCurrent, 0x354);
MU_ASSERT_FIELD(CHARACTER, PathNum, 0x356);
MU_ASSERT_FIELD(CHARACTER, PathX, 0x357);
MU_ASSERT_FIELD(CHARACTER, PathY, 0x366);
MU_ASSERT_FIELD(CHARACTER, PositionX, 0x388);
MU_ASSERT_FIELD(CHARACTER, PositionY, 0x38C);
#undef MU_ASSERT_FIELD

// Acceso tipado a una dirección de entidad que el código heredado guarda
// como entero o como puntero a byte.
inline CHARACTER* AsCharacter(uintptr_t p) { return reinterpret_cast<CHARACTER*>(p); }
inline CHARACTER* AsCharacter(void* p) { return static_cast<CHARACTER*>(p); }
inline const CHARACTER* AsCharacter(const void* p) { return static_cast<const CHARACTER*>(p); }
inline OBJECT* AsObject(uintptr_t p) { return reinterpret_cast<OBJECT*>(p); }
inline OBJECT* AsObject(void* p) { return static_cast<OBJECT*>(p); }
inline const OBJECT* AsObject(const void* p) { return static_cast<const OBJECT*>(p); }
