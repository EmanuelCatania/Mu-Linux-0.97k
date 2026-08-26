// Combat_LegacyAnimation.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// FUN_00408cb0 vive ahora en Scene/Scene_CharSelect_Nav.cpp y FUN_00408e30 en
// Net/Crypto.cpp; antes se definian en este archivo.
void __fastcall FUN_00408cb0(int*, float);
int  __cdecl    FUN_00408e30(DWORD *a1);

#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl FUN_0054158c(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif


// ── Particle / animation / bone math ─────────────────────────────────────────

// FUN_0043e820 — replaced by full implementation above (line ~6057)

// FUN_00443e70 @ 0x00443E70 — SetAttackSpeed
// Real logic (after anti-tamper hash table blocks):
//   1. Reads CharacterAttribute->MagicDamageMax and AttackDamageMinRight
//   2. Computes animation speed: fVar2 = AttackDamageMinRight * _DAT_005524bc
//      fStack_8 = MagicDamageMax * _DAT_005524bc, local_18 = MagicDamageMax * _DAT_005528e0
// SetAttackSpeed @ 0x00443E70 — set player animation speeds based on
// CharacterAttribute stats. Port FIEL desde IDA decompile (2026-05-02).
//
// Reads CharacterAttribute[+0x38] (AttackSpeed) and [+0x44] (MagicSpeed),
// computes scale factors, then writes per-animation speed floats into the
// player model's animation table (Models[Player=390].Data[+0x30]).
//
// Anti-tamper hash table operations (sub_403F80/sub_4041E0/sub_404370/etc
// wrapping CharacterMachine encrypt/decrypt) are skipped per project policy.
//
// Note: IDA decompile shows v33 and v38 as uninitialized stack locals used
// for FIST/SWORD/RIDE-attack speeds. The disasm doesn't show explicit
// assignments visible in hex-rays output — treating as 0 (stack default).
// For attack-speed stat scaling on melee, this means baseline speeds are
// used. Magic skills (which use v34 = AttackSpeed*0.004 and v35/v39 from
// MagicSpeed) DO scale per-stat correctly.
//
// Animation table layout (from Models[390]+0x30 base, P):
//   P+548   FIST          v33 + 0.6
//   P+560..880 (stride 16, at i-12)  SWORD/RIDE attacks v33 + 0.25
//   P+900,916 SWORD1/2 magic         v33 + 0.3
//   P+932   SWORD3                   v33 + 0.27
//   P+948   SWORD4                   v33 + 0.3
//   P+964,980 SWORD5/WHEEL           v33 + 0.24
//   P+996   FURY_STRIKE              0.38 (constant 0x3EC28F5C)
//   P+1012  VITALITY                 0.34 (constant 0x3EAE147B)
//   P+1028  RIDER                    v33 + 0.3
//   P+1044  RIDER_FLY                v33 + 0.3
//   P+1060  SPEAR                    v33 + 0.3
//   P+1076  ONE_TO_ONE               v33 + 0.3
//   P+736..784 (stride 16, at i-12)  BOW attacks  v35 = MagicSpeed*0.002
//   P+864..880 (stride 16)           RIDE BOW     v35
//   P+1300  TWO_HAND_SWORD_TWO       v33 + 0.25
//   P+1312..1360 (stride 16)         HAND/WEAPON  v34 + 0.29
//   P+1380  ELF1                     v38 + 0.25
//   P+1396  TELEPORT                 v34 + 0.3
//   P+1412  FLASH                    v34 + 0.4
//   P+1428  INFERNO                  v34 + 0.6
//   P+1444  HELL                     v34 + 0.5
//   P+1460  RIDE_SKILL               v34 + 0.3
extern DWORD CharacterAttribute_var;     // alias - already in our globals
void __cdecl FUN_00443e70(void) {
    DWORD ca = (DWORD)DAT_07cf1ff4;       // CharacterAttribute base
    if (ca == 0) return;
    DWORD models = DAT_05828d58;          // Models base
    if (models == 0) return;

    // Read stats (16-bit unsigned)
    int  attackSpeed = (int)(*(unsigned short*)(ca + 0x38));
    int  magicSpeed  = (int)(*(unsigned short*)(ca + 0x44));

    float v34 = (float)attackSpeed * _DAT_005524bc;  // = AttackSpeed * 0.004
    float v39 = (float)magicSpeed  * _DAT_005524bc;  // = MagicSpeed  * 0.004
    float v35 = (float)magicSpeed  * _DAT_005528e0;  // = MagicSpeed  * 0.002

    // 2026-08-24 FIX (animaciones largas terminaban tarde): aca decia
    //     float v33 = 0.0f;  float v38 = 0.0f;
    // con el comentario "uninitialized in IDA decompile", asi que TODA la tabla
    // de velocidades quedaba en su valor base (0.6, 0.25, 0.30...) sin escalar
    // por los stats del personaje. Con AttackSpeed alto la diferencia es del
    // orden del doble: el original terminaba la animacion y el nuestro seguia.
    //
    // Que v33/v38 aparezcan sin inicializar es un artefacto de Hex-Rays, no del
    // binario: `v34` (= AttackSpeed*0.004) y `v39` (= MagicSpeed*0.004) SI se
    // calculan y despues no se usan para nada, mientras v33/v38 se usan sin
    // origen. Los slots intermedios (v34/v35/v36) estan declarados BYREF porque
    // se los pasa al hash-table anti-tamper — o sea el valor real se guarda,
    // pasa por el encrypt/decrypt y se recupera, y el decompile perdio el
    // vinculo entre el que se guarda y el que se lee.
    //
    // El emparejamiento se confirma por el USO: v33 alimenta todas las anims de
    // ATAQUE (FIST +0.6, SWORD +0.25, SKILL_SWORD3 +0.27...) y v38 la de
    // PLAYER_SKILL_ELF1 (+0.25), que es magia.
    float v33 = v34;   // AttackSpeed * 0.004
    float v38 = v39;   // MagicSpeed  * 0.004

    // Models[Player(390)] base animation table pointer at offset 0x11E18
    // (= 188*390 + 48 = entry +0x30 = animation array ptr).
    int* tablePtrPtr = (int*)((char*)(uintptr_t)models + 73368);  // 0x11E18
    if (*tablePtrPtr == 0) return;
    char* P = (char*)(uintptr_t)*tablePtrPtr;

    // FIST
    *(float*)(P + 548) = v33 + 0.6f;

    // SWORD/RIDE attacks: P+i-12 for i ∈ {560,576,592,...,880}
    float swordSpeed = v33 + 0.25f;
    for (int i = 560; i <= 880; i += 16) {
        *(float*)(P + i - 12) = swordSpeed;
    }

    // Magic sword skills
    float skillSpeed3 = v33 + 0.3f;
    *(float*)(P + 900)  = skillSpeed3;   // SKILL_SWORD1
    *(float*)(P + 916)  = skillSpeed3;   // SKILL_SWORD2
    *(float*)(P + 932)  = v33 + 0.27f;   // SKILL_SWORD3
    *(float*)(P + 948)  = skillSpeed3;   // SKILL_SWORD4
    float skillSpeed5 = v33 + 0.24f;
    *(float*)(P + 964)  = skillSpeed5;   // SKILL_SWORD5
    *(float*)(P + 980)  = skillSpeed5;   // SKILL_WHEEL
    *(float*)(P + 1076) = skillSpeed3;   // ATTACK_ONETOONE
    *(DWORD*)(P + 996)  = 0x3EC28F5C;    // FURY_STRIKE = 0.38
    *(DWORD*)(P + 1012) = 0x3EAE147B;    // VITALITY    = 0.34
    *(float*)(P + 1060) = skillSpeed3;   // SKILL_SPEAR
    *(float*)(P + 1028) = skillSpeed3;   // SKILL_RIDER
    *(float*)(P + 1044) = skillSpeed3;   // SKILL_RIDER_FLY

    // Two-hand sword2
    *(float*)(P + 1300) = swordSpeed;

    // BOW attacks: P+i-12 for i ∈ {752,768,784} (loop starts at 736+16)
    int v28 = 736;
    do {
        v28 += 16;
        *(float*)(P + v28 - 12) = v35;
    } while (v28 <= 784);

    // RIDE BOW: P+j-12 for j ∈ {864, 880}
    for (int j = 864; j <= 880; j += 16) {
        *(float*)(P + j - 12) = v35;
    }

    // Skills
    *(float*)(P + 1380) = v38 + 0.25f;        // ELF1
    float magicSkill = v34 + 0.29f;
    for (int k = 1312; k <= 1360; k += 16) {
        *(float*)(P + k - 12) = magicSkill;   // HAND1..WEAPON2
    }

    float teleportSpeed = v34 + 0.3f;
    *(float*)(P + 1396) = teleportSpeed;       // TELEPORT
    *(float*)(P + 1412) = v34 + 0.4f;          // FLASH
    *(float*)(P + 1428) = v34 + 0.6f;          // INFERNO
    *(float*)(P + 1444) = v34 + 0.5f;          // HELL
    *(float*)(P + 1460) = teleportSpeed;       // RIDE_SKILL
}
