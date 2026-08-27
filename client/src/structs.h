#pragma once
// structs.h — Game type definitions for Mu Online 0.97k reconstruction.
// Derived from the DLL injector source (same version, same binary).
// Includes: constants (Defines), enums, and game structs.

// ── Constants ─────────────────────────────────────────────────────────────────
#define DWORD_MAX 4294967295UL

/* Viewport */
#define MAX_MAIN_VIEWPORT   400

/* Terrain */
#define TERRAIN_SCALE       100.0f
#define TERRAIN_SIZE        256
#define TERRAIN_SIZE_MASK   255

/* OpenGL color packing */
#define Color3b(r,g,b)    (((b)<<16)+((g)<<8)+(r))
#define Color4b(r,g,b,a)  (((a)<<24)+((b)<<16)+((g)<<8)+(r))

/* Item */
#define MAX_ITEM_TYPE       32
#define MAX_ITEM_SECTION    16
#define MAX_ITEM            (MAX_ITEM_SECTION * MAX_ITEM_TYPE)
#define ITEM_BASE_MODEL     400
#define GET_ITEM(x,y)       (((x)*MAX_ITEM_TYPE)+(y))
#define GET_ITEM_MODEL(x,y) (GET_ITEM(x,y)+ITEM_BASE_MODEL)
#define MAX_SPECIAL_OPTION  9
#define MAX_ITEM_PRICE      2000000000

#define INVENTORY_WEAR_SIZE     12
#define INVENTORY_COLUMN_SIZE   8
#define INVENTORY_ROW_SIZE      8
#define INVENTORY_ITEM_SIZE     (INVENTORY_COLUMN_SIZE * INVENTORY_ROW_SIZE)
#define INVENTORY_MAX_SIZE      (INVENTORY_WEAR_SIZE + INVENTORY_ITEM_SIZE)

#define GET_ITEM_OPT_LEVEL(x)   (((x) >> 3) & 0xF)
#define GET_ITEM_OPT_SKILL(x)   (((x) >> 7) & 1)
#define GET_ITEM_OPT_LUCK(x)    (((x) >> 2) & 1)
#define GET_ITEM_OPT_OPT(x,y)  (((x) & 3) + (((y) & 64) >> 4))
#define GET_ITEM_OPT_EXC(x)     ((x) & 0x3F)

/* Character / Monster */
#define MAX_CLASS               4
#define MAX_RESISTANCE          4
#define MAX_CHARACTER_LEVEL     1000
#define MAX_CHARACTER_ZEN       2000000000
#define MONSTER_BASE_MODEL      270
#define MAX_MONSTER             256

/* Textures / Models */
#define MAX_TEXTURE         5500
#define PLUS_TEXTURE        5500
#define TOTAL_TEXTURE       (MAX_TEXTURE + PLUS_TEXTURE)
#define MAX_MODELS          963
#define PLUS_MODELS         256
#define TOTAL_MODELS        (MAX_MODELS + PLUS_MODELS)
#define MAX_BITMAP_FILE_NAME 32

/* Maps */
#define MAX_MAPS    32
#define MAX_GATES   100

/* Server list */
#define MAX_SERVER_HI   25
#define MAX_SERVER_LO   20

/* Sound */
#define MAX_SOUND_LEVEL 9
#define MAX_MUSIC_LEVEL 9

/* Buffs */
#define FindEffect(x,y) ((((x)&(y))==(y))?1:0)
#define AddEffect(x,y)  ((x)|=(y))
#define DelEffect(x,y)  ((x)&=~(y))

// ── Enums ─────────────────────────────────────────────────────────────────────

enum eSceneFlags
{
    NON_SCENE       = 0,
    WEBZEN_SCENE    = 1,  // g_GameState 1 = Intro
    LOG_IN_SCENE    = 2,  // g_GameState 2 = Login
    LOADING_SCENE   = 3,  // g_GameState 3 = Loading
    CHARACTER_SCENE = 4,  // g_GameState 4 = CharSelect
    MAIN_SCENE      = 5   // g_GameState 5 = InGame
};

enum eMapNumber
{
    MAP_LORENCIA      = 0,
    MAP_DUNGEON       = 1,
    MAP_DEVIAS        = 2,
    MAP_NORIA         = 3,
    MAP_LOST_TOWER    = 4,
    MAP_EXILE         = 5,
    MAP_ARENA         = 6,
    MAP_ATLANS        = 7,
    MAP_TARKAN        = 8,
    MAP_DEVIL_SQUARE1 = 9,
    MAP_ICARUS        = 10,
    MAP_BLOOD_CASTLE1 = 11,
    MAP_BLOOD_CASTLE2 = 12,
    MAP_BLOOD_CASTLE3 = 13,
    MAP_BLOOD_CASTLE4 = 14,
    MAP_BLOOD_CASTLE5 = 15,
    MAP_BLOOD_CASTLE6 = 16
};

enum eClasses
{
    CLASS_DARK_WIZARD    = 0,
    CLASS_DARK_KNIGHT    = 1,
    CLASS_FAIRY_ELF      = 2,
    CLASS_MAGIC_GLADIATOR = 3
};

enum ePkLevel
{
    PKLVL_FULLHERO  = 1,
    PKLVL_HERO      = 2,
    PKLVL_COMMONER  = 3,
    PKLVL_WARNING   = 4,
    PKLVL_OUTLAW    = 5,
    PKLVL_KILLER    = 6
};

enum eSkillNumber
{
    SKILL_NONE              = 0,
    SKILL_POISON            = 1,
    SKILL_METEORITE         = 2,
    SKILL_LIGHTNING         = 3,
    SKILL_FIRE_BALL         = 4,
    SKILL_FLAME             = 5,
    SKILL_TELEPORT          = 6,
    SKILL_ICE               = 7,
    SKILL_TWISTER           = 8,
    SKILL_EVIL_SPIRIT       = 9,
    SKILL_HELL_FIRE         = 10,
    SKILL_POWER_WAVE        = 11,
    SKILL_AQUA_BEAM         = 12,
    SKILL_BLAST             = 13,
    SKILL_INFERNO           = 14,
    SKILL_TELEPORT_ALLY     = 15,
    SKILL_MANA_SHIELD       = 16,
    SKILL_ENERGY_BALL       = 17,
    SKILL_DEFENSE           = 18,
    SKILL_FALLING_SLASH     = 19,
    SKILL_LUNGE             = 20,
    SKILL_UPPERCUT          = 21,
    SKILL_CYCLONE           = 22,
    SKILL_SLASH             = 23,
    SKILL_TRIPLE_SHOT       = 24,
    SKILL_HEAL              = 26,
    SKILL_GREATER_DEFENSE   = 27,
    SKILL_GREATER_DAMAGE    = 28,
    SKILL_SUMMON1           = 30,
    SKILL_SUMMON2           = 31,
    SKILL_SUMMON3           = 32,
    SKILL_SUMMON4           = 33,
    SKILL_SUMMON5           = 34,
    SKILL_SUMMON6           = 35,
    SKILL_SUMMON7           = 36,
    SKILL_TWISTING_SLASH    = 41,
    SKILL_RAGEFUL_BLOW      = 42,
    SKILL_DEATH_STAB        = 43,
    SKILL_IMPALE            = 47,
    SKILL_GREATER_LIFE      = 48,
    SKILL_FIRE_BREATH       = 49,
    SKILL_MONSTER_AREA_ATTACK = 50,
    SKILL_ICE_ARROW         = 51,
    SKILL_PENETRATION       = 52,
    SKILL_FIRE_SLASH        = 55,
    SKILL_POWER_SLASH       = 56
};

enum eEffectState
{
    STATE_POISON              = 0x0001,
    STATE_FREEZE              = 0x0002,
    STATE_ATTACK              = 0x0004,
    STATE_DEFENSE             = 0x0008,
    STATE_ADD_LIFE            = 0x0010,
    STATE_HARDEN              = 0x0020,
    STATE_REDUCE_DEFENSE      = 0x0040,
    STATE_REDUCE_ATTACKDAMAGE = 0x0080,
    STATE_REDUCE_MAGICDEFENSE = 0x0100,
    STATE_REDUCE_MAGICPOWER   = 0x0200
};

enum eEquipment
{
    EQUIPMENT_WEAPON_RIGHT = 0,
    EQUIPMENT_WEAPON_LEFT  = 1,
    EQUIPMENT_HELM         = 2,
    EQUIPMENT_ARMOR        = 3,
    EQUIPMENT_PANTS        = 4,
    EQUIPMENT_GLOVES       = 5,
    EQUIPMENT_BOOTS        = 6,
    EQUIPMENT_WING         = 7,
    EQUIPMENT_HELPER       = 8,
    EQUIPMENT_AMULET       = 9,
    EQUIPMENT_RING_RIGHT   = 10,
    EQUIPMENT_RING_LEFT    = 11
};

enum eTextAligns
{
    RT3_SORT_LEFT   = 0,
    RT3_SORT_CENTER = 1
};

enum eModelNumbers
{
    MODEL_SWORD  = ITEM_BASE_MODEL,
    MODEL_AXE    = MODEL_SWORD  + MAX_ITEM_TYPE,
    MODEL_MACE   = MODEL_AXE   + MAX_ITEM_TYPE,
    MODEL_SPEAR  = MODEL_MACE  + MAX_ITEM_TYPE,
    MODEL_BOW    = MODEL_SPEAR + MAX_ITEM_TYPE,
    MODEL_STAFF  = MODEL_BOW   + MAX_ITEM_TYPE,
    MODEL_SHIELD = MODEL_STAFF + MAX_ITEM_TYPE,
    MODEL_HELM   = MODEL_SHIELD + MAX_ITEM_TYPE,
    MODEL_ARMOR  = MODEL_HELM  + MAX_ITEM_TYPE,
    MODEL_PANTS  = MODEL_ARMOR + MAX_ITEM_TYPE,
    MODEL_GLOVES = MODEL_PANTS + MAX_ITEM_TYPE,
    MODEL_BOOTS  = MODEL_GLOVES + MAX_ITEM_TYPE,
    MODEL_WING   = MODEL_BOOTS + MAX_ITEM_TYPE,
    MODEL_HELPER = MODEL_WING  + MAX_ITEM_TYPE,
    MODEL_POTION = MODEL_HELPER + MAX_ITEM_TYPE,
    MODEL_ETC    = MODEL_POTION + MAX_ITEM_TYPE,
    MODEL_EVENT  = 947
};

// ── Game Structs ──────────────────────────────────────────────────────────────

struct ITEM  // size = 68
{
    /*+00*/ short  Type;
    /*+04*/ int    Level;
    /*+08*/ BYTE   Part;
    /*+09*/ BYTE   Class;
    /*+10*/ BYTE   TwoHand;
    /*+12*/ WORD   DamageMin;
    /*+14*/ WORD   DamageMax;
    /*+16*/ BYTE   SuccessfulBlocking;
    /*+18*/ WORD   Defense;
    /*+20*/ WORD   MagicDefense;
    /*+22*/ BYTE   WeaponSpeed;
    /*+24*/ WORD   WalkSpeed;
    /*+26*/ BYTE   Durability;
    /*+27*/ BYTE   Option1;
    /*+28*/ WORD   RequireStrength;
    /*+30*/ WORD   RequireDexterity;
    /*+32*/ WORD   RequireEnergy;
    /*+34*/ WORD   RequireLevel;
    /*+36*/ BYTE   SpecialNum;
    /*+37*/ BYTE   Special[MAX_SPECIAL_OPTION];
    /*+46*/ BYTE   SpecialValue[MAX_SPECIAL_OPTION];
    /*+56*/ DWORD  Key;
    /*+60*/ BYTE   Unknown;
    /*+61*/ BYTE   byColorState;
    /*+62*/ BYTE   x;
    /*+63*/ BYTE   y;
    /*+64*/ BYTE   Color;
};

struct ITEM_ATTRIBUTE  // size = 64
{
    /*+00*/ char  Name[30];
    /*+30*/ bool  TwoHand;
    /*+31*/ BYTE  Level;
    /*+32*/ BYTE  Width;
    /*+33*/ BYTE  Height;
    /*+34*/ BYTE  DamageMin;
    /*+35*/ BYTE  DamageMax;
    /*+36*/ BYTE  DefenseRate;
    /*+37*/ BYTE  Defense;
    /*+38*/ BYTE  MagicDefense;
    /*+39*/ BYTE  AttackSpeed;
    /*+40*/ BYTE  WalkSpeed;
    /*+41*/ BYTE  Durability;
    /*+42*/ BYTE  MagicDurability;
    /*+43*/ BYTE  MagicPower;
    /*+44*/ WORD  RequireStrength;
    /*+46*/ WORD  RequireAgility;
    /*+48*/ BYTE  RequireEnergy;
    /*+49*/ BYTE  RequireLevel;
    /*+50*/ BYTE  Value;
    /*+52*/ int   Money;
    /*+56*/ BYTE  RequireClass[MAX_CLASS];
    /*+60*/ BYTE  Resistance[MAX_RESISTANCE];
};

struct GATE_ATTRIBUTE
{
    BYTE Flag;
    BYTE Map;
    BYTE StartX;
    BYTE StartY;
    BYTE EndX;
    BYTE EndY;
    BYTE Target;
    BYTE Direction;
    BYTE RequireLevel;
};

struct PARTY_t
{
    char  Name[11];
    BYTE  number;
    BYTE  map;
    BYTE  x;
    BYTE  y;
    DWORD CurLife;
    DWORD MaxLife;
    BYTE  stepHP;
    int   index;
    DWORD state;
};

struct BITMAP_t
{
    char   FileName[MAX_BITMAP_FILE_NAME];
    float  Width;
    float  Height;
    char   Components;
    GLuint TextureNumber;
    BYTE   Ref;
    BYTE  *Buffer;
};

struct MONSTER_ATTRIBUTE
{
    WORD Life;
    WORD MoveSpeed;
    WORD AttackSpeed;
    WORD AttackDamageMin;
    WORD AttackDamageMax;
    WORD Defense;
    WORD MagicDefense;
    WORD AttackRating;
    WORD SuccessfulBlocking;
};

struct MONSTER_SCRIPT
{
    BYTE Type;
    char Name[32];
    WORD Level;
    MONSTER_ATTRIBUTE Attribute;
};

struct SERVER_t
{
    char  IP[20];
    WORD  Port;
    WORD  Index;
    BYTE  Percent;
};

struct SERVER_LIST_t
{
    char     Name[20];
    BYTE     Number;
    bool     extServer;
    SERVER_t Server[MAX_SERVER_LO];
};

// ── Named global aliases (from DLL Offsets.h) ─────────────────────────────────
// These provide human-readable names for the DAT_ addresses in globals.h.
// Access via the named macros; the underlying storage is the DAT_ variable.

// Window / display
#define WindowWidth         DAT_0056156c     // int
#define WindowHeight        DAT_00561570     // int
#define g_fScreenRate_x     _DAT_055c9b70    // float
// g_hWnd, g_hDC already in stdafx.h

// State
// g_GameState (SceneFlag) already in stdafx.h
#define g_GameSubState      DAT_0055a7ac     // int (World in DLL)
#define MouseX              DAT_083a427c     // int
#define MouseY              DAT_083a4278     // int
#define MouseLButtonPop     DAT_083a413c     // bool
#define MouseLButtonPush    DAT_083a4124     // bool
#define MouseRButton        DAT_083a42ac     // bool (approx)
#define MouseLButton        DAT_083a42c4     // bool

// Character
#define Hero                DAT_07abf5d8     // DWORD — ptr to local player entity
#define CharactersClient    DAT_07abf5d0     // DWORD — entity array base
#define HeroIndex           DAT_05826ca0     // int (approx, not in our globals yet — add below)
// SelectedCharacter is a named global declared in globals.h.
// IDA provenance: DAT_00559c50.

// Camera / OpenGL — underlying storage is DWORD but the binary treats the
// bit pattern as float. Use lvalue-float reinterpretation so assignments like
// `CameraFOV = 45.0f` write the IEEE-754 bits (not an integer cast).
#define CameraFOV           (*(float*)&DAT_00561554)
#define CameraViewNear      (*(float*)&DAT_0056154c)
#define CameraViewFar       (*(float*)&DAT_00561550)
#define EarthQuake          _DAT_083a0210    // float
#define FogEnable           DAT_083a42ea     // bool

// Connection
#define g_bGameServerConnected  DAT_05826cf0 // BOOL (add to globals if needed)
#define ServerList          (*(SERVER_LIST_t(*)[MAX_SERVER_HI])&DAT_083a45d8)
// 2026-08-22: `ServerNumber` es un nombre INVENTADO del port y describe mal el
// campo.  `0x083A7C40` es la **cantidad de servidores** que trajo el F4/02
// (`ReceiveServerList` L15: `unk_83A7C40 = ReceiveBuffer[5]`), y `Game_SceneUpdate`
// lo pone en 0 al entrar al login.  Nadie lo LEE — se guarda y no se consume.
//
// El "numero de server" que decide con que nombre/color se dibuja cada rectangulo
// del select-server es OTRA cosa: es `group = ServerCode / 20` (0..N), que indexa
// la tabla de nombres `DAT_07d52c34` (stride 300) — ver `Recv_ServerList`.  El
// grupo 12 es un caso especial: usa `GlobalText[559]` y va al slot 24.
#define ServerListCount     DAT_083a7c40     // BYTE (use as char)
#define ServerNumber        ServerListCount  // alias historico, no usar en codigo nuevo

// Models
// Address of bone-scratch base, reinterpreted as an array of 3x4 bone matrices.
#define BoneTransform       ((float(*)[3][4])&DAT_06970a9c)
// g_BitmapsRaw es el array real (stride 0x38). DAT_083a7ca0 es sólo el PRIMER DWORD
// del slot[0], no el puntero — por eso indexamos sobre la dirección de g_BitmapsRaw.
extern char g_BitmapsRaw[];
#define Bitmaps             ((BITMAP_t*)&g_BitmapsRaw[0])

// ── Companion-project aliases (IDA decompile compatibility) ──────────────────
// Globals:
#define WorldTime            DAT_05826e08          // int — g_AnimTick
#define InputEnable          DAT_00559c84          // DWORD
// 2026-08-23 FIX: esto apuntaba a `DAT_07eab250`, que es un DWORD de 4 bytes.
// El propio `globals.h:837` ya documentaba el mislabel ("NO es
// PrimaryTerrainLight (ese es DAT_081cb608)") pero el macro nunca se corrigio.
// Consecuencias, las dos graves:
//   1. Los 23 call sites de `AddTerrainLight(..., PrimaryTerrainLight[0])`
//      escribian la luz dinamica en un global muerto, asi que el fuego, las
//      antorchas y los efectos NUNCA iluminaban — el render lee DAT_081cb608.
//   2. `AddTerrainLight` indexa `Buffer[768*y + 3*x]` con x,y hasta 255, o sea
//      escribia hasta ~786 KB pasado un DWORD: desborde masivo sobre BSS.
// Y como nadie resetea el buffer muerto, la luz se acumulaba sin techo (medido:
// el tile del fuego llego a 64.0 y subiendo). `Terrain_Water` (0x4F95E0) si
// resetea DAT_081cb608 por frame, que es lo que acota la acumulacion.
#define PrimaryTerrainLight  ((float(*)[3])&DAT_081cb608[0])  // float[256*256][3]
// Functions (map companion-project names → FUN_ addresses from functions.h):
#define SetAction(ent, act)  FUN_0043e820((int)(ent), (int)(act))
#define VectorRotate         Vector_InverseRotate
// 2026-08-23 FIX (el fuego no iluminaba): esto aliaseaba `AddTerrainLight` a
// `AddTerrainLightClip_stub`, que es OTRA funcion del binario.
//   AddTerrainLight     0x004F76C0  sin clamp superior  · decenas de callers
//   AddTerrainLightClip 0x004F7800  clampea a 1.0       · UN caller (0x4C0E59)
// Con el alias, toda la luz dinamica (fuego, antorchas, efectos) quedaba cortada
// en 1.0 y no llegaba a saturar — de ahi que el fuego se dibujara pero sin
// resplandor.  El port correcto de 0x4F76C0 ya existia como `FUN_004f76c0`
// (Render/SMD_Parser.cpp), mal etiquetado en functions.h como "Terrain_SetHeight
// or similar"; ese nombre inventado es lo que llevo a crear este alias.
//
// Wrapper en vez de `#define` a secas porque `FUN_004f76c0` quedo tipada con los
// punteros como `int` (artefacto del decompile) y los call sites pasan `float*`.
void __cdecl FUN_004f76c0(float, float, int, int, int);   // decl local: structs.h no incluye functions.h
inline void AddTerrainLight(float xf, float yf, float* Light, int Range, float* Buffer)
{
    FUN_004f76c0(xf, yf, (int)(uintptr_t)Light, Range, (int)(uintptr_t)Buffer);
}
#define AngleMatrix          AngleMatrix
#define CreateEffect         Effect_Create
#define CreateBomb           Effect_SpawnSmokeBurst
#define Alpha                FUN_0043e5c0   // Particle_CommitState / finalize alpha
#define AddTerrainAttributeRange Terrain_UpdateTileAttributeRect
#define AddTerrainAttribute      Terrain_SetTileAttributeBits
#define RenderBitmap         GL_DrawTexture
