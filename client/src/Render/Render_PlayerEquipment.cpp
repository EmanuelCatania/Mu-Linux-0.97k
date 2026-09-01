// Render_PlayerEquipment.cpp
//
// Port directo del IDA RenderCharacter (sub_456770) líneas 1267-1981 — render
// de Helper, Wing y Weapons del player. Este bloque se llamaba en Entity_UpdateRender
// pero estaba parcialmente portado (solo wing+algunos weapons), faltaba:
//   - Render de Helper (819 = pet hada / 817 = otro)
//   - Loop principal de armas (v234=0..1) que dibuja Weapon[0] y Weapon[1]
//   - 30+ weapon-specific particle effects (glows de espadas/staffs/bows)
//
// 2026-05-04: agregado port directo de `CWeaponView::RenderCharacterBackItem`
// (DLL source `Mu-linux-97K/Source/Client/Main/WeaponView.cpp:49`) que decide
// si el arma se renderiza en la ESPALDA (LinkBone 47) o en la mano. Conditions:
//   - safe-zone (entity+0x34E set por terrain bit 0)
//   - greeting anim (93..124)
//   - Atlans swim (World 7, anim 21/29)
//   - NOT Blood Castle (World 11..16)
// Cuando bBindBack=true: weapon → bone 47 (espalda), Render_PlayerWeaponLoop
// se SALTA. Cuando false: Render_PlayerWeaponLoop renderiza en mano (default).
//
// Mapping de offsets (entity = c, model = o):
//   c+0x270 = Weapon[0] slot (24 bytes: model/lvl/opt/bone/action/...)
//   c+0x288 = Weapon[1] slot
//   c+0x2A0 = Wing slot
//   c+0x2B8 = Helper slot
//   c+0x108 = float AnimationFrame
//   c+0x105 = byte CurrentAction
//
// Models[player_type=390] action table:
//   Models + 188*390 + 48 = pointer a Actions[]
//   Actions[N].PlaySpeed @ +4 dentro del struct (16 bytes/action)
//
// API mapping IDA → nuestra:
//   CreateSprite(type, pos, scale, color, owner, alpha, mode) → FUN_004795c0
//   CreateJoint(type, p0, p1, color, flag, owner, scale, ?, mode) → Joint_Create
//   Particle_Spawn(type, pos, size, color, flag, alpha, mode) → Particle_Spawn
//   TransformPosition(model, mat3x4, pos_in, pos_out, translate) → BMD_TransformPosition
//   sub_4553C0(model, type, bone, scale, color, owner) → FUN_004553c0
//
// Anti-tamper hash-table operations (líneas IDA 1290-1505) elididas — pure
// obfuscation por CLAUDE.md, no afectan render.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include <math.h>

extern "C" void DbgLogPublic(const char*);

// EquipmentLevelSet — global escrita por CheckFullSet, leída por la rama
// case 0x186 del switch en RenderCharacter (sparkles 10=lightning sprite,
// 11=lightning joint).
int EquipmentLevelSet = 0;

// g_bAddDefense — global escrita SÓLO por CheckFullSet (IDA L77/L158), leída
// por sub_47E160 (nuestro FUN_0047e160) para el bonus de defensa de set
// completo (+5% con set +10, +10% con set +11).
//
// Semántica IDA (rara pero fiel): se pone en 1 en LABEL_15, al que se llega
// por TODOS los caminos de salida — tanto el `goto` del set completo como el
// `break`/fin-de-while que dejan `v26 = 0`. O sea vale 1 por defecto y el
// único que la baja es el caso Magic Gladiator con set completo cuyo Armor no
// es 271 ni 276. Es inofensivo que quede en 1 sin set porque el consumidor
// gatea con EquipmentLevelSet (que estos caminos dejan en 0).
int g_bAddDefense = 0;

// ── FUN_00451b20  CheckFullSet  ───────────────────────────────────────────────
// Determina si el char tiene un "full set" (5 armaduras del mismo set, todas
// con level >= 9). Devuelve 1 si full set, 0 si no. Setea EquipmentLevelSet
// al MIN level de las piezas si match, 0 si no match.
//
// Iteración hacia atrás desde Boot (c+600) hasta el body part v1 (1 normal,
// 2 para MG class==3): si alguna pieza es 0xFFFF antes de llegar a v1 → no full.
// Si llega: agarra type%16 de Boot como set_id, comprueba que las demás piezas
// matcheen el set_id Y tengan level >= 9.
//
// IDA original tiene 893 bytes — la mitad es hash-table refcount obfuscation
// que no afecta resultado. Porteado el cómputo y omitida la obfuscación.
// Cola común de CheckFullSet (IDA LABEL_15). Todos los `return` de la función
// pasan por acá; ver el comentario de g_bAddDefense arriba.
static bool CheckFullSet_Tail(int c, bool v26) {
    g_bAddDefense = 1;
    if (((*(unsigned char*)(c + 444) & 7) == 3) && v26) {   // Magic Gladiator
        // IDA: v20 = *(_WORD *)(CharacterMachine + 740)  → wear slot 3 (Armor),
        // 536 + 68*3 = 740. Sólo los armors 271 y 276 conservan el bonus.
        unsigned int cm = (unsigned int)(uintptr_t)CharacterAttribute;
        if (cm >= 0x100000u && cm < 0x80000000u) {
            unsigned short armor = *(unsigned short*)((char*)(uintptr_t)cm + 740);
            if (armor != 271 && armor != 276) {
                g_bAddDefense = 0;
            }
        }
    }
    return v26;
}

bool __cdecl FUN_00451b20(int c) {
    int v1 = ((*(unsigned char*)(c + 444) & 7) == 3) ? 2 : 1;  // MG=2, otros=1
    int v2 = 5;
    EquipmentLevelSet = 0;

    // Walk back through body parts (Boot=+600, Glove=+576, Pant=+552, Armor=+528, Helm=+504)
    short* v3 = (short*)(c + 600);
    while (*v3 != (short)0xFFFF) {
        --v2;
        v3 -= 12;  // -24 bytes (= 12 shorts) — previous body part
        if (v2 < v1) {
            // All required parts present; check set match
            int v4 = (*(short*)(c + 600) - 400) % 16;  // Boot set_id
            int v5 = 5;
            int v6 = *(unsigned char*)(c + 602) & 0xF;  // Boot level
            short* v7 = (short*)(c + 600);
            while (((v7[1]) & 0xF) >= 9 && v4 == (*v7 - 400) % 16) {
                if (v6 >= ((v7[1]) & 0xF)) {
                    v6 = (v7[1]) & 0xF;
                    EquipmentLevelSet = v6;
                }
                --v5;
                v7 -= 12;
                if (v5 < v1) {
                    return CheckFullSet_Tail(c, true);  // Full set + matched levels
                }
            }
            // 2026-08-08 FIX (glow pegado al desequiparse la armadura): acá el
            // port devolvía `true` ("tiene las 5 piezas aunque no matcheen").
            // En IDA ese camino es un `break` del while EXTERNO, y justo
            // después del while está `v26 = 0;` — o sea devuelve **false**.
            // El único camino que deja `v26 = 1` es el `goto LABEL_15` de
            // arriba (set completo Y todos los niveles >= 9).
            //
            // Por qué se notaba al desequiparse: al sacar la armadura las
            // body-parts de la entidad NO quedan en 0xFFFF sino en el modelo
            // por defecto de la clase, así que el while externo recorre las 5
            // piezas y entra acá; con niveles 0 el while interno no corre y
            // caía en este `return true` → `v230 = true` en el case 0x186 de
            // Entity_UpdateRender → se seguía ejecutando la sección 5
            // (PartObjectColor + 6 sprites en los huesos del arma) = el glow.
            // Un pj que nunca tuvo armadura sale antes por el while externo
            // (alguna pieza en 0xFFFF) y por eso se veía normal.
            EquipmentLevelSet = 0;
            return CheckFullSet_Tail(c, false);
        }
    }
    return CheckFullSet_Tail(c, false);  // Some part missing
}

// ── Helpers para Models[player].Actions[] ─────────────────────────────────────
// Models + 188*390 + 48 = byte offset 73368 dentro de DAT_05828d58.
// IDA: *(_DWORD *)(Models + 73368) = player_model->Actions ptr.
static inline float* GetPlayerActions() {
    return *(float**)((char*)(uintptr_t)DAT_05828d58 + 73368);
}
static inline float GetPlayerActionPlaySpeed(int byteOffset) {
    float* actions = GetPlayerActions();
    if (!actions) return 0.0f;
    return *(float*)((char*)actions + byteOffset);
}

// ── TransformPosition wrapper (IDA TransformPosition = nuestra BMD_TransformPosition) ─
static inline void TransformPos(void* model, float* boneMat, float* posIn, float* posOut) {
    BMD_TransformPosition(model, boneMat, posIn, posOut, '\x01');
}
// IDA usa bone*48 (= 0x30) que es el stride normal. El indexado bone*16 (caso
// 0x1D5 etc.) es matriz de 4 floats — distinto layout. Wrappers para ambos.
static inline float* BoneMat48(void* o, int bone) {
    return (float*)((char*)(uintptr_t)*(int*)((char*)o + 0x114) + 0x30 * bone);
}
static inline float* BoneMat16(void* o, int bone) {
    return (float*)((char*)(uintptr_t)*(int*)((char*)o + 0x114) + 16 * (3 * bone));
}


// ── Weapon-specific FX por tipo (IDA case 0x194..0x25E) ───────────────────────
// param_1 = entity c (int)
// puVar13 = entity o (int* alias)
// v121 = weapon slot ptr (c + 624 + 24*hand)
// targetJ = (rand()%30+70) * 0.01f
static void RenderWeaponFX(int c, int o, int v121, float Targetj, float* Light)
{
    // 'model' = global BMD entry at DAT_05828d58 + 188*entity_type. Pasado a
    // BMD_TransformPosition/TransformPos como ECX (this_) — usado para leer scale y
    // calcular bbox. NO confundir con `entity_o` que tiene el bone buffer.
    void* model = (void*)(uintptr_t)((char*)(uintptr_t)DAT_05828d58
                                     + 188 * (int)*(short*)(o + 2));
    void* entity_o = (void*)(uintptr_t)o;  // entity pointer with bone buffer at +0x114
    float WorldPosition[3];
    float v242[3];
    float v248[3];
    float TargetPosition[3];
    int   bone = *(unsigned char*)(v121 + 4);
    float* boneMat = BoneMat48(entity_o, bone);
    // BUGFIX 2026-09-01: aca habia `float WorldTime = (float)DAT_05826e08;`, pero
    // `WorldTime` es un MACRO a DAT_05826e08 (structs.h:438), asi que declaraba un
    // local que se sombreaba a si mismo y quedaba con basura (C4700): los cases que
    // animan con sin(WorldTime * ...) usaban tiempo random.  Removido: los usos de
    // abajo ya resuelven al global por el macro.
    short Type = *(short*)v121;

    switch (Type) {
        case 0x194: case 0x195: case 0x19D:
            WorldPosition[0] = 0.0f;
            WorldPosition[1] = -110.0f;
            WorldPosition[2] = 5.0f;
            TransformPos(model, boneMat, WorldPosition, v242);
            FUN_004795c0(1150, v242, 1.4f, Light, o, 0.0f, 0);
            break;

        case 0x197: case 0x19A:
            WorldPosition[0] = 0.0f;
            WorldPosition[1] = -110.0f;
            WorldPosition[2] = 0.0f;
            TransformPos(model, boneMat, WorldPosition, v242);
            FUN_004795c0(1150, v242, 1.4f, Light, o, 0.0f, 0);
            break;

        case 0x198: case 0x199:
            WorldPosition[0] = 0.0f;
            WorldPosition[1] = -110.0f;
            WorldPosition[2] = -5.0f;
            TransformPos(model, boneMat, WorldPosition, v242);
            FUN_004795c0(1150, v242, 1.4f, Light, o, 0.0f, 0);
            break;

        case 0x19B: case 0x19E:
            WorldPosition[0] = 0.0f;
            WorldPosition[1] = -150.0f;
            WorldPosition[2] = 0.0f;
            TransformPos(model, boneMat, WorldPosition, v242);
            FUN_004795c0(1150, v242, 1.4f, Light, o, 0.0f, 0);
            break;

        case 0x19C: {
            WorldPosition[0] = 0.0f;
            float L2[3] = { Targetj, Targetj, Targetj };
            WorldPosition[1] = -160.0f;
            WorldPosition[2] = 0.0f;
            TransformPos(model, boneMat, WorldPosition, v242);
            FUN_004795c0(1150, v242, 1.4f, L2, o, 0.0f, 0);
            break;
        }

        case 0x1A1: { // Sword glow with lightning joints
            float Scalek = (float)(sin(WorldTime * 0.0040000002) * 10.0 + 20.0);
            // First joint: -20Y/-40Z -> -160Y/-10Z
            WorldPosition[0] = 0.0f; WorldPosition[1] = -20.0f; WorldPosition[2] = -40.0f;
            TransformPos(model, boneMat, WorldPosition, v248);
            WorldPosition[0] = 0.0f; WorldPosition[1] = -160.0f; WorldPosition[2] = -10.0f;
            TransformPos(model, boneMat, WorldPosition, TargetPosition);
            Joint_Create(1250, v248, TargetPosition, (float*)((char*)o + 28), 4, o, Scalek, -1, 0);
            // Second joint: -10Y/28Z -> -145Y/18Z
            WorldPosition[0] = 0.0f; WorldPosition[1] = -10.0f; WorldPosition[2] = 28.0f;
            TransformPos(model, boneMat, WorldPosition, v248);
            WorldPosition[0] = 0.0f; WorldPosition[1] = -145.0f; WorldPosition[2] = 18.0f;
            TransformPos(model, boneMat, WorldPosition, TargetPosition);
            Joint_Create(1250, v248, TargetPosition, (float*)((char*)o + 28), 4, o, Scalek, -1, 0);
            break;
        }

        case 0x1A2: { // Staff blue glow
            float Scalel = (float)((sin(WorldTime * 0.0040000002) + 1.0) * 0.30000001);
            WorldPosition[0] = 0.0f; WorldPosition[1] = -20.0f; WorldPosition[2] = 15.0f;
            TransformPos(model, boneMat, WorldPosition, v242);
            float L2[3] = { Scalel * 0.2f, Scalel * 0.2f, Scalel };
            float v210 = Scalel + 1.0f;
            FUN_004795c0(1231, v242, v210, L2, o, 0.0f, 0);
            // Three joint sparkles
            WorldPosition[0] = 0.0f; WorldPosition[1] = -133.0f; WorldPosition[2] = 7.0f;
            TransformPos(model, boneMat, WorldPosition, WorldPosition);
            float Scalem = Scalel * 20.0f + 20.0f;
            Joint_Create(1254, v242, WorldPosition, (float*)((char*)o + 28), 10, 0, Scalem, -1, 0);
            Joint_Create(1254, v242, WorldPosition, (float*)((char*)o + 28), 10, 0, Scalem, -1, 0);
            Joint_Create(1254, v242, WorldPosition, (float*)((char*)o + 28), 10, 0, Scalem, -1, 0);
            break;
        }

        case 0x1D5: { // Multi-row sparkle (16 rows along Y)
            int v172 = 0;
            float Scaleh = 0.0f;
            float L2[3] = { Targetj, Targetj * 0.60000002f, Targetj * 0.40000001f };
            do {
                if (rand() % 4 < 3) {
                    WorldPosition[0] = 0.0f;
                    WorldPosition[1] = (float)Scaleh - 30.0f;
                    WorldPosition[2] = 0.0f;
                    float* mat3 = BoneMat16(entity_o, bone);
                    TransformPos(model, mat3, WorldPosition, v242);
                    FUN_004795c0(1150, v242, 1.0f, L2, o, 0.0f, 0);
                }
                v172 -= 20;
                Scaleh = (float)v172;
            } while (v172 > -160);
            break;
        }

        case 0x1D6: { // Big staff with halo + 5 row sprites + 2 bone particles
            float Scalei = (float)(sin(WorldTime * 0.0040000002) * 0.30000001 + 0.69999999);
            WorldPosition[0] = 0.0f; WorldPosition[1] = -84.0f; WorldPosition[2] = 0.0f;
            TransformPos(model, boneMat, WorldPosition, v242);
            float L2[3] = { Scalei, Scalei * 0.2f, Scalei * 0.1f };
            float v211 = Scalei + 1.5f;
            FUN_004795c0(1231, v242, v211, L2, o, 0.0f, 0);
            int v178 = 0;
            do {
                WorldPosition[0] = 0.0f;
                WorldPosition[1] = (float)v178 * 20.0f - 10.0f;
                WorldPosition[2] = 0.0f;
                TransformPos(model, boneMat, WorldPosition, v242);
                FUN_004795c0(1231, v242, 1.0f, L2, o, 0.0f, 0);
                v178--;
            } while (v178 > -5);
            float L3[3] = { Scalei * 0.5f, Scalei * 0.1f, Scalei * 0.050000001f };
            FUN_004553c0(model, 1231, 2, 1.0f, L3, o);
            FUN_004553c0(model, 1231, 6, 1.0f, L3, o);
            break;
        }

        case 0x21D: case 0x21E: { // Bow tracer
            float L2[3];
            if (Type == 541) {
                L2[0] = Targetj * 0.2f;
                L2[1] = Targetj * 0.40000001f;
                L2[2] = Targetj * 0.60000002f;
            } else {
                L2[0] = Targetj * 0.60000002f;
                L2[1] = Targetj * 0.40000001f;
                L2[2] = Targetj * 0.2f;
            }
            int v184 = 0;
            float Scalej = 0.0f;
            do {
                WorldPosition[0] = 0.0f;
                WorldPosition[1] = -20.0f;
                WorldPosition[2] = Scalej;
                float* mat3 = BoneMat16(entity_o, bone);
                TransformPos(model, mat3, WorldPosition, v242);
                FUN_004795c0(1150, v242, 2.0f, L2, o, 0.0f, 0);
                v184 -= 20;
                Scalej = (float)v184;
            } while (v184 > -120);
            break;
        }

        case 0x220: { // Crossbow tracer (vertical)
            int v168 = 0;
            float Scaleg = 0.0f;
            float L2[3] = { Targetj * 0.40000001f, Targetj * 0.60000002f, Targetj };
            do {
                WorldPosition[0] = 0.0f;
                WorldPosition[1] = -10.0f;
                WorldPosition[2] = Scaleg * 20.0f;
                float* mat3 = BoneMat16(entity_o, bone);
                TransformPos(model, mat3, WorldPosition, v242);
                FUN_004795c0(1150, v242, 2.0f, L2, o, 0.0f, 0);
                Scaleg = (float)(--v168);
            } while (v168 > -6);
            break;
        }

        case 0x234: { // Special weapon — single big sprite at mid-back
            WorldPosition[0] = 0.0f; WorldPosition[1] = -90.0f; WorldPosition[2] = 0.0f;
            float L2[3] = { Targetj * 0.40000001f, Targetj * 0.80000001f, Targetj * 0.60000002f };
            TransformPos(model, boneMat, WorldPosition, v242);
            FUN_004795c0(1231, v242, 2.0f, L2, o, 0.0f, 0);
            break;
        }

        case 0x235: { // Halo + sparks
            float L2[3] = { Targetj * 0.40000001f, Targetj * 0.60000002f, Targetj };
            WorldPosition[0] = 0.0f; WorldPosition[1] = -145.0f; WorldPosition[2] = 0.0f;
            float* mat3 = BoneMat16(entity_o, bone);
            TransformPos(model, mat3, WorldPosition, v242);
            FUN_004795c0(1231, v242, 1.5f, L2, o, 0.0f, 0);
            FUN_004795c0(1191, v242, 0.30000001f, L2, o, 0.0f, 0);
            break;
        }

        case 0x236: { // Glowing dragon weapon — many sprites + particles
            WorldPosition[0] = 0.0f; WorldPosition[1] = -145.0f; WorldPosition[2] = 0.0f;
            TransformPos(model, boneMat, WorldPosition, v242);
            float L2[3] = { Targetj, Targetj * 0.60000002f, Targetj * 0.40000001f };
            FUN_004795c0(1175, v242, 3.0f, L2, o, 0.0f, 0);
            FUN_004795c0(1232, v242, 1.5f, L2, o, 0.0f, 0);
            int v160 = 4;
            do {
                WorldPosition[0] = (float)(rand() % 20 - 10);
                WorldPosition[1] = (float)(rand() % 20 - 10) - 90.0f;
                WorldPosition[2] = (float)(rand() % 20 - 10);
                TransformPos(model, boneMat, WorldPosition, v242);
                Particle_Spawn(1175, v242, (float*)((char*)o + 28), L2, 1, 1.0f, 0);
                v160--;
            } while (v160);
            int v164 = 0;
            float Scalef = 0.0f;
            float L3[3] = { Targetj, Targetj * 0.2f, Targetj * 0.1f };
            do {
                if (rand() % 4 < 3) {
                    WorldPosition[0] = 0.0f;
                    WorldPosition[1] = Scalef + 60.0f;
                    WorldPosition[2] = 0.0f;
                    TransformPos(model, boneMat, WorldPosition, v242);
                    FUN_004795c0(1150, v242, 1.0f, L3, o, 0.0f, 0);
                }
                v164 -= 20;
                Scalef = (float)v164;
            } while (v164 > -200);
            break;
        }

        case 0x237: { // Multi-bone sparkle (bones 27-37)
            float L2[3] = { Targetj * 0.40000001f, Targetj * 0.60000002f, Targetj };
            FUN_004553c0(model, 1231, 27, 2.0f, L2, o);
            for (int j = 28; j <= 37; ++j) {
                FUN_004553c0(model, 1150, j, 1.5f, L2, o);
            }
            break;
        }

        case 0x239: { // Staff blue cross-wing
            float L2[3] = { Targetj * 0.60000002f, Targetj * 0.60000002f, Targetj + Targetj };
            WorldPosition[0] = 0.0f; WorldPosition[1] = -120.0f; WorldPosition[2] = 5.0f;
            TransformPos(model, boneMat, WorldPosition, v242);
            FUN_004795c0(1231, v242, 1.5f, L2, o, 0.0f, 0);
            float Scalen = Targetj + 1.0f;
            FUN_004795c0(1150, v242, Scalen, L2, o, 0.0f, 0);
            WorldPosition[0] = 0.0f; WorldPosition[1] = 100.0f; WorldPosition[2] = 10.0f;
            TransformPos(model, boneMat, WorldPosition, v242);
            FUN_004795c0(1150, v242, Scalen, L2, o, 0.0f, 0);
            break;
        }

        case 0x25E: { // Side-glow off-axis
            float L2[3] = { Targetj * 0.40000001f, Targetj * 0.60000002f, Targetj * 1.5f };
            WorldPosition[0] = 20.0f; WorldPosition[1] = 0.0f; WorldPosition[2] = 0.0f;
            float* mat3 = BoneMat16(entity_o, bone);
            TransformPos(model, mat3, WorldPosition, v242);
            FUN_004795c0(1231, v242, 1.5f, L2, o, 0.0f, 0);
            break;
        }

        default:
            break;
    }
}




// 2026-05-04: hero equipment stash — populado por F3/03 (Recv_JoinMapServer)
// y re-aplicado por HeroEquipWatchdog cada frame. Mitiga un bug todavía no
// localizado donde algo resetea entity+0x2a0/0x270/0x288/0x2b8 a -1 después
// de F3/03.
extern "C" {
short g_HeroEquipStash_LH = -1, g_HeroEquipStash_RH = -1;
short g_HeroEquipStash_Wing = -1, g_HeroEquipStash_Helper = -1, g_HeroEquipStash_Pendant = -1;
short g_HeroEquipStash_RingR = -1, g_HeroEquipStash_RingL = -1;
short g_HeroEquipStash_Body[6] = { -1, -1, -1, -1, -1, -1 };
unsigned char g_HeroEquipStash_LHLvl = 0, g_HeroEquipStash_RHLvl = 0;
unsigned char g_HeroEquipStash_WingLvl = 0, g_HeroEquipStash_HelperLvl = 0, g_HeroEquipStash_PendantLvl = 0;
unsigned char g_HeroEquipStash_RingRLvl = 0, g_HeroEquipStash_RingLLvl = 0;
unsigned char g_HeroEquipStash_LHDur = 0, g_HeroEquipStash_RHDur = 0;
unsigned char g_HeroEquipStash_WingDur = 0, g_HeroEquipStash_HelperDur = 0, g_HeroEquipStash_PendantDur = 0;
unsigned char g_HeroEquipStash_RingRDur = 0, g_HeroEquipStash_RingLDur = 0;
unsigned char g_HeroEquipStash_BodyDur[6] = {};
unsigned char g_HeroEquipStash_BodyLvl[6] = {};
unsigned char g_HeroEquipStash_BodyOpt1[6] = {};
unsigned char g_HeroEquipStash_BodyOpt2[6] = {};
unsigned char g_HeroEquipStash_BodyOpt3[6] = {};
unsigned char g_HeroEquipStash_LHOpt = 0, g_HeroEquipStash_RHOpt = 0;
unsigned char g_HeroEquipStash_WingOpt = 0, g_HeroEquipStash_HelperOpt = 0, g_HeroEquipStash_PendantOpt = 0;
unsigned char g_HeroEquipStash_RingROpt = 0, g_HeroEquipStash_RingLOpt = 0;
unsigned char g_HeroEquipStash_LHOpt2 = 0, g_HeroEquipStash_RHOpt2 = 0;
unsigned char g_HeroEquipStash_WingOpt2 = 0, g_HeroEquipStash_HelperOpt2 = 0, g_HeroEquipStash_PendantOpt2 = 0;
unsigned char g_HeroEquipStash_RingROpt2 = 0, g_HeroEquipStash_RingLOpt2 = 0;
unsigned char g_HeroEquipStash_LHOpt3 = 0, g_HeroEquipStash_RHOpt3 = 0;
unsigned char g_HeroEquipStash_WingOpt3 = 0, g_HeroEquipStash_HelperOpt3 = 0, g_HeroEquipStash_PendantOpt3 = 0;
unsigned char g_HeroEquipStash_RingROpt3 = 0, g_HeroEquipStash_RingLOpt3 = 0;
int g_HeroEquipStash_Valid = 0;
}

extern "C" BYTE OffsetInventoryItems[];

static void HeroEquipWatchdog_SeedCharacterMachineFromStash(BYTE* cm)
{
    if (!cm) return;

      auto seedSlot = [&](int slotIdx, short type, BYTE lvl, BYTE dur, BYTE opt1, BYTE opt2, BYTE opt3) {
          BYTE* slot = cm + 536 + 68 * slotIdx;
          if (type == -1)
              return;
          short curType = *(short*)slot;
          // 2026-06-21: watchdog sólo debe RESTAURAR slots faltantes.
          // Si CharacterMachine ya trae un type válido, lo dejamos vivir como
          // fuente de verdad y no lo sobreescribimos con stash viejo.
          if (curType != -1)
              return;

        memset(slot, 0, sizeof(ITEM));
        *(short*)(slot + 0) = type;
        *(int*)(slot + 4) = (int)opt1;     // packed option byte (level<<3, luck/exc bits path)
        *(BYTE*)(slot + 26) = dur;
        *(BYTE*)(slot + 27) = opt1;        // Option1 raw packed byte
        *(DWORD*)(slot + 56) = dur ? (DWORD)dur : 1u; // gate used by panel/world render
        *(BYTE*)(slot + 60) = opt2;        // byteHi / extra flags when available
        *(BYTE*)(slot + 61) = opt3;        // ext byte / color state when available
        FUN_0047b910((int)(uintptr_t)slot, (int)opt1, (int)opt2);
        ITEM* equip = (ITEM*)slot;
        equip->Level = (int)opt1;
        equip->Key = (*(DWORD*)(slot + 56) > 0) ? 1 : 0;
        equip->x = (BYTE)slotIdx;
        equip->y = 0;
        equip->Color = 0;
      };

    // 2026-08-08: acá había un `seedSlotFromInventory(0..11)` — SEGUNDA copia del
    // mismo bug que se removió en HeroEquipWatchdog: leía
    // `((ITEM*)OffsetInventoryItems) + slotIdx`, o sea las CELDAS 0..11 del grid
    // 8×8 (el índice de celda real es `slotIdx - 12`, ver AddItemToGrid:396), y
    // las copiaba a los slots de EQUIPO de CharacterMachine.
    // Por eso seguían apareciendo la poción en la caja de los pants y un casco
    // en las cajas de los anillos: son las celdas 0..11 del inventario visible.
    // Removido. Sólo queda el re-seed desde el stash real (abajo).

    seedSlot(0, g_HeroEquipStash_LH, g_HeroEquipStash_LHLvl, g_HeroEquipStash_LHDur,
             g_HeroEquipStash_LHOpt, g_HeroEquipStash_LHOpt2, g_HeroEquipStash_LHOpt3);
    seedSlot(1, g_HeroEquipStash_RH, g_HeroEquipStash_RHLvl, g_HeroEquipStash_RHDur,
             g_HeroEquipStash_RHOpt, g_HeroEquipStash_RHOpt2, g_HeroEquipStash_RHOpt3);
    seedSlot(2, g_HeroEquipStash_Body[0], g_HeroEquipStash_BodyLvl[0], g_HeroEquipStash_BodyDur[0],
             g_HeroEquipStash_BodyOpt1[0], g_HeroEquipStash_BodyOpt2[0], g_HeroEquipStash_BodyOpt3[0]);
    seedSlot(3, g_HeroEquipStash_Body[1], g_HeroEquipStash_BodyLvl[1], g_HeroEquipStash_BodyDur[1],
             g_HeroEquipStash_BodyOpt1[1], g_HeroEquipStash_BodyOpt2[1], g_HeroEquipStash_BodyOpt3[1]);
    seedSlot(4, g_HeroEquipStash_Body[2], g_HeroEquipStash_BodyLvl[2], g_HeroEquipStash_BodyDur[2],
             g_HeroEquipStash_BodyOpt1[2], g_HeroEquipStash_BodyOpt2[2], g_HeroEquipStash_BodyOpt3[2]);
    seedSlot(5, g_HeroEquipStash_Body[3], g_HeroEquipStash_BodyLvl[3], g_HeroEquipStash_BodyDur[3],
             g_HeroEquipStash_BodyOpt1[3], g_HeroEquipStash_BodyOpt2[3], g_HeroEquipStash_BodyOpt3[3]);
    seedSlot(6, g_HeroEquipStash_Body[4], g_HeroEquipStash_BodyLvl[4], g_HeroEquipStash_BodyDur[4],
             g_HeroEquipStash_BodyOpt1[4], g_HeroEquipStash_BodyOpt2[4], g_HeroEquipStash_BodyOpt3[4]);
    seedSlot(7, g_HeroEquipStash_Wing, g_HeroEquipStash_WingLvl, g_HeroEquipStash_WingDur,
             g_HeroEquipStash_WingOpt, g_HeroEquipStash_WingOpt2, g_HeroEquipStash_WingOpt3);
    seedSlot(8, g_HeroEquipStash_Helper, g_HeroEquipStash_HelperLvl, g_HeroEquipStash_HelperDur,
             g_HeroEquipStash_HelperOpt, g_HeroEquipStash_HelperOpt2, g_HeroEquipStash_HelperOpt3);
    seedSlot(9, g_HeroEquipStash_Pendant, g_HeroEquipStash_PendantLvl, g_HeroEquipStash_PendantDur,
             g_HeroEquipStash_PendantOpt, g_HeroEquipStash_PendantOpt2, g_HeroEquipStash_PendantOpt3);
    seedSlot(10, g_HeroEquipStash_RingR, g_HeroEquipStash_RingRLvl, g_HeroEquipStash_RingRDur,
             g_HeroEquipStash_RingROpt, g_HeroEquipStash_RingROpt2, g_HeroEquipStash_RingROpt3);
    seedSlot(11, g_HeroEquipStash_RingL, g_HeroEquipStash_RingLLvl, g_HeroEquipStash_RingLDur,
             g_HeroEquipStash_RingLOpt, g_HeroEquipStash_RingLOpt2, g_HeroEquipStash_RingLOpt3);
}

// Llamada por Entity_UpdateRender al inicio del render del hero in-game.
// Restaura equipment slots si fueron borrados.
extern "C" void HeroEquipWatchdog(int c)
{
    if (DAT_005615c0 != 5) return;
    if (!DAT_07abf5d8 || c != (int)(uintptr_t)DAT_07abf5d8) return;
    unsigned char* be = (unsigned char*)c;
    static const int kBodyOff[6] = { 0x1e0, 0x1f8, 0x210, 0x228, 0x240, 0x258 };
    BYTE* cm = (BYTE*)(uintptr_t)DAT_07cf1ffc;

    if (cm) {
        // ── 2026-08-08: RE-SIEMBRA DESDE EL STASH — REMOVIDA ─────────────────
        // Era el último resto del watchdog: si un wear slot de CharacterMachine
        // estaba en -1, lo rellenaba desde `g_HeroEquipStash_*`. O sea disparaba
        // EXACTAMENTE al desequipar → el item volvía a aparecer en su caja
        // (pants que "siguen equipados", escudo que se dibuja con un arma a dos
        // manos, etc.).
        //
        // Validado contra IDA: en el binario NADA re-siembra CharacterMachine
        // por frame. El equipo lo escribe sólo el server —  F3/10
        // (ReceiveInventory), el ack del move 0x24 y los acks de equipar —, y
        // el render (`RenderEquipment3D` 0x4E3100) lo lee directo. Este watchdog
        // era 100% invención del port para tapar que el equipo se reseteaba
        // después del F3/03.
        //
        // Si el reseteo tras F3/03 vuelve a aparecer, el diagnóstico de abajo
        // (EQUIPWIPE) lo registra: es el bug real a arreglar, no a tapar.
        {
            static int  s_prevOccupied = -1;
            int occupied = 0;
            for (int i = 0; i < 12; ++i) {
                if (*(short*)(cm + 536 + 68 * i) != -1) ++occupied;
            }
            if (s_prevOccupied > 0 && occupied == 0) {
                DbgLogPublic("EQUIPWIPE: los 12 wear slots de CharacterMachine "
                             "pasaron a -1 de golpe (buscar el writer)");
            }
            s_prevOccupied = occupied;
        }
        // ── 2026-08-08: RE-SIEMBRA DESDE EL INVENTARIO — REMOVIDA ────────────
        // Acá había un loop que, para cada slot de equipo VACÍO en
        // CharacterMachine, lo rellenaba desde `((ITEM*)OffsetInventoryItems)[slotIdx]`
        // con slotIdx = 0..11. Eso es memoria EQUIVOCADA por construcción:
        // `OffsetInventoryItems` es el pool del grid 8×8 y su índice de celda es
        // `slotIdx - 12` (ver AddItemToGrid:396) — o sea `[0..11]` son las
        // CELDAS 0..11 del grid visible (la primera fila y media del
        // inventario), NO los wear slots. Los wear slots viven sólo en
        // `CharacterMachine + 536 + 68*slot`.
        //
        // Consecuencias que explicaba, las dos reportadas por el usuario:
        //  · Render equivocado: la celda 1 del grid (p. ej. un item de mascota,
        //    tipo 418) se copiaba al slot 1 = caja del ESCUDO → el escudo se
        //    dibujaba como Uniria (modelo = Type+400 = 818). Las celdas 9/10/11
        //    caían en Ring1/Ring2/Pendant → "los anillos figuran como guantes".
        //  · El "clon" al levantar un item equipado: UI_Main limpiaba el slot y
        //    este loop lo volvía a llenar al frame siguiente con lo que hubiera
        //    en la celda del grid — de ahí el "a veces sale otro item".
        //
        // El re-seed desde `g_HeroEquipStash_*` (arriba) se mantiene por ahora:
        // es el que tapa el reseteo de equipo tras el F3/03. Sacarlo y arreglar
        // ese bug de raíz queda pendiente.
        // IDA/source base path: let SetCharacterClass rebuild the world hero
        // from CharacterMachine, instead of keeping a partial local mirror.
        //
        // ── 2026-08-15: el rebuild ya NO corre por frame ─────────────────────
        // `SetCharacterClass` (0x45C130) termina cancelando la animación en
        // curso cuando la acción está fuera de [0x22, 0x5B]:
        //     if (!(v11 >= 0x85 && v11 <= 0x8C) && (v11 < 0x22 || v11 > 0x5B))
        //         SetPlayerStop(c);
        // La caminata es la acción 13 (0x0D) → entraba SIEMPRE. Como este
        // watchdog corre en cada frame del render del hero, el ciclo por tick
        // era: acción 13 → SetPlayerStop pone 1 (frame=0) → SetPlayerWalk la
        // devuelve a 13 (frame=0 otra vez). El frame nunca pasaba de 0.3 y la
        // caminata se veía "mueve un pie y se resetea".
        // Medido con el probe FRAMEDBG (2026-08-15):
        //     act=13 spd=0.300 f=0.000->0.300   ← en CADA tick
        // mientras que las acciones de idle (1 y 9), que no pasan por este
        // camino, progresaban normal (0.28 → 0.56 → 0.84 … loop en nF=6).
        //
        // En IDA `SetCharacterClass` se llama sólo cuando CAMBIA el equipo
        // (ReceiveAddPoint / ProtocolCore / char-select), nunca por frame.
        // Reproducimos eso: rebuild sólo si los wear slots (o la entidad del
        // hero) cambiaron respecto del frame anterior.
        {
            unsigned int sig = 0u;
            for (int i = 0; i < 12; ++i) {
                const BYTE* s = cm + 536 + 68 * i;
                sig = sig * 131u + (unsigned int)(unsigned short)*(const short*)s;
                sig = sig * 131u + *(const unsigned int*)(s + 4);   // Level/options
            }
            static unsigned int s_prevSig = 0u;
            static int          s_prevHero = 0;
            static bool         s_have = false;
            if (!s_have || sig != s_prevSig || c != s_prevHero) {
                s_prevSig  = sig;
                s_prevHero = c;
                s_have     = true;
                FUN_0045c130(c);
                FUN_0045c720(c);
            }
        }
        return;
    }

    // IDA/source-base path rebuilds the visual entity from CharacterMachine via
    // SetCharacterClass. Do not mirror raw stash item types directly into the
    // world entity when CharacterMachine is unavailable: that custom fallback
    // writes inventory/raw types into slots that expect resolved visual models.
    (void)be;
    (void)kBodyOff;
}

extern "C" int RenderCharacterBackItem(int c, int o)
{
    // ── Bind (IDA RenderCharacter 0x456770, LABEL_265 .. LABEL_308) ──────────
    // REESCRITA 2026-09-01.  La version anterior era un port del DLL de
    // inyeccion (`WeaponView.cpp`) y divergia del binario en lo esencial: hacia
    // un LOOP sobre LOS DOS slots de arma y mandaba AMBOS a la espalda.  El
    // 0.97k manda UNO SOLO, elegido por `Hand`.  Con espada + escudo el vanilla
    // cuelga la ESPADA y no dibuja el escudo; nosotros dibujabamos tambien el
    // escudo, que con la traslacion (-20, 5, 40) del hueso 47 queda flotando
    // por encima de la cabeza.  (Regla del proyecto: IDA manda sobre el DLL.)
    int Bind = 0;

    // DESVIACION DEL PORT (2026-05-04, conservada): gate por state=5 (in-game).
    // IDA no lo tiene.  Sin el, char-select dibujaba el arma dos veces.
    if (DAT_005615c0 != 5)
        return 0;

    // `World` es macro a DAT_0055a7ac (structs.h:389): usar otro nombre o el
    // local se sombrearia a si mismo.
    const int iWorld = (int)DAT_0055a7ac;
    const unsigned char anim = *(unsigned char*)(o + 0x105);

    // Bind = SafeZone || saludo (93..124) || (Atlans && nadando (21|29)).
    if (*(unsigned char*)(c + 0x34E) != 0)          // c->SafeZone
        Bind = 1;
    else if (anim >= 93 && anim <= 124)             // PLAYER_GREETING1..SALUTE1
        Bind = 1;
    else if (World == 7 && (anim == 21 || anim == 29))
        Bind = 1;

    if (World >= 11 && World <= 16)                 // InBloodCastle()
        Bind = 0;

    // IDA LABEL_275: los no-jugadores saltan a LABEL_330 (loop de manos), que
    // el caller resuelve con el valor de retorno.
    if (*(short*)(o + 2) != 390)                    // o->Type != MODEL_PLAYER
        return Bind;

    // ── Que item va a la espalda, y de que mano ──────────────────────────────
    // Weapon[0] = c+624 (0x270), Weapon[1] = c+648 (0x288).
    int Hand = 0;
    int Back = 0;
    const int w0 = *(short*)(c + 624);
    const int w1 = *(short*)(c + 648);

    if (Bind) {
        Back = 1;
        if ((w1 >= 528 && w1 < 535) || w1 == 545) {         // Bow
            Hand = 1;
        } else if ((w0 >= 536 && w0 < 543) || w0 == 544 || w0 == 546) {  // Crossbow
            Hand = 0;
        } else if (w1 == 535) {                             // Bolts
            Hand = 1;
        }
    } else if (w1 == 535) {                                 // Bolts
        Back = 1;
        Hand = 1;
    } else if (w0 == 543) {                                 // Arrows
        Back = 1;
    }

    const int partBase = c + 24 * Hand;                     // 624 / 648
    int Type    = *(short*)(c + 8 * (3 * Hand + 78));
    int Level   = *(unsigned char*)(partBase + 0x272);
    int Option1 = *(unsigned char*)(partBase + 0x273);

    // Esqueleto NPC de Lorencia (Kind 4, World 0, SubType 206..208): baculo fijo.
    const unsigned char Kind = *(unsigned char*)(o + 0x84);
    const int SubType = *(int*)(o + 4);
    if (Kind != 4 || World != 0 || SubType < 206 || SubType > 208) {
        if (!Back || Type == -1)
            return Bind;                                    // IDA: goto LABEL_308
    } else {
        Type  = 537;                                        // GET_ITEM_MODEL(4, 9)
        Level = 8;
    }

    // ── Render en el hueso 47 (espalda), reusando el PART del ala ────────────
    const unsigned int fAnimationFrameBackUp = *(unsigned int*)(c + 0x2A8);
    *(unsigned char*)(c + 0x2A4) = 47;                      // w->LinkBone
    if (anim == 30 || anim == 31)                           // PLAYER_FLY / FLY_CROSSBOW
        *(float*)(c + 0x2B0) = 1.0f;
    else
        *(float*)(c + 0x2B0) = 0.25f;

    // Hex-Rays llama `Scale` a esta local, pero el 9no parametro de
    // RenderLinkObject es `Link`.  Para armas/escudos (400..623) va 1.
    char Link = '\0';
    if (Type >= 400 && Type < 624) {
        Link = '\x01';
        // PlaySpeed = Models[MODEL_PLAYER].Actions[1].PlaySpeed
        //   Models + 0x11E98 = Models + 390*0xBC + 0x30  (puntero a Actions)
        int actions = *(int*)(DAT_05828d58 + 390 * 0xbc + 0x30);
        if (actions != 0)
            *(unsigned int*)(c + 0x2B0) = *(unsigned int*)(actions + 20);
    }

    FUN_00455430(0.0f, 0.0f, 15.0f, c, c + 0x2A0,
                 Type, (char)Level, (unsigned int)Option1,
                 Link, '\x01', 0);

    *(unsigned int*)(c + 0x2A8) = fAnimationFrameBackUp;
    return Bind;
}


// ── Render del Helper (IDA líneas 1267-1288) ─────────────────────────────────
// Llamado después del Wing render. Helper.Type == 817 dispara la creación de
// un sprite-glow rojo en la mano del char (LinkBone=34).
extern "C" void Render_PlayerHelper(int c, int o)
{
    if (*(short*)(c + 0x2B8) != 817) return;

    float Targetj = (float)((rand() % 30 + 70) * 0.0099999998);
    unsigned char v101 = *(unsigned char*)(c + 0x2BA);   // Type@2 = level
    unsigned char v215 = *(unsigned char*)(c + 0x2BB);   // Option1
    *(unsigned char*)(c + 0x2BC) = 34;                   // LinkBone = 34
    *(int*)(c + 0x2C8) = 0x3F000000;                     // PlaySpeed = 0.5f
    FUN_00455430(20.0f, 0.0f, 0.0f, c, c + 0x2B8,
                 817, (char)v101, (unsigned int)v215, '\0', '\x01', 0);

    // Sprite glow at transformed position (20, 0, 15) in bone[34]
    unsigned char v102 = *(unsigned char*)(c + 0x2BC);   // = 34
    void* model = (void*)(uintptr_t)((char*)(uintptr_t)DAT_05828d58
                                     + 188 * (int)*(short*)(o + 2));
    void* entity_o = (void*)(uintptr_t)o;
    float* boneMat = BoneMat48(entity_o, v102);
    float Pos[3]   = { 20.0f, 0.0f, 15.0f };
    float WorldPos[3];
    TransformPos(model, boneMat, Pos, WorldPos);
    float Light[3] = { Targetj * 0.5f, 0.0f, 0.0f };
    FUN_004795c0(1150, WorldPos, 1.5f, Light, o, 0.0f, 0);
}


// ── Loop principal de armas (IDA `if (!Bind) { do { ... } while (v234 < 2) }`)
//   Recorre Weapon[0] (v234=0) y Weapon[1] (v234=1). Por cada arma:
//   - Skip si action 62 + frame<=4 (ataque-animación-mid sin hand swap)
//   - Setup CurrentAction (v121+5) y PlaySpeed (v121+16) según anim/weapon
//   - RenderLinkObject(0,0,0, c, weapon_slot, weapon_type, level, opt, 1, 1, 0)
//   - Glow color RGB derivado del nivel (v121+2)
//   - Switch por tipo de arma → particle FX (CreateSprite/Joint/Spawn)
extern "C" void Render_PlayerWeaponLoop(int c, int o)
{
    // IDA reaches this loop from LABEL_330 for players and monsters alike.
    if (c == 0 || o == 0) return;
    if (*(unsigned char*)c == 0) return;       // active flag

    float Targetj = (float)((rand() % 30 + 70) * 0.0099999998);
    unsigned char v118 = *(unsigned char*)(o + 261);   // CurrentAction
    unsigned char Scalee = *(unsigned char*)(c + 770); // anim flag (HashTable result en IDA — usamos byte directo como fallback)

    int v234 = 0;
    do {
        // Anim-state skip: action 62 + frame <= 4 + hand 0 cancela
        if (Scalee == 41) {
            if ((v118 == 62 && !v234 && *(float*)(o + 264) <= 4.0f)
                || (v118 == 61 && !v234)) {
                goto label_419;
            }
        } else if (v118 == 62 && !v234 && *(float*)(o + 264) <= 4.0f) {
            goto label_419;
        }

        {
        int v119  = 3 * v234 + 78;
        int v121  = c + 8 * v119;            // weapon slot ptr (c+624 o c+648)
        short v120 = *(short*)v121;          // weapon Type

        if (v120 == -1 || v120 == 535 || v120 == 543) {
            goto label_419;  // skip empty/bolts/arrows (rendered separately)
        }

        // ── Bone+anim setup ─────────────────────────────────────────────────
        bool went_to_render = false;
        if (v118 == 46 || v118 == 47 || v118 == 48 || v118 == 49) {
            // Greeting/wave anims: bone 0, action[46].PlaySpeed
            *(unsigned char*)(v121 + 5) = 0;
            *(int*)(v121 + 16) = *(int*)((char*)GetPlayerActions() + 740);
        } else if (v120 != 466) {
            if (v120 == 469) {
                // Special weapon 469: bone 0, 2x action[1].PlaySpeed
                *(unsigned char*)(v121 + 5) = 0;
                float ps = GetPlayerActionPlaySpeed(20);
                *(float*)(v121 + 16) = ps + ps;
            } else if (v120 == 566) {
                // Special weapon 566 (high-tier staff): bone 0, 15x speed
                *(unsigned char*)(v121 + 5) = 0;
                *(float*)(v121 + 16) = GetPlayerActionPlaySpeed(20) * 15.0f;
            } else if (v120 < 400 || v120 >= 432) {
                // Non-sword: bone 0, no anim
                *(unsigned char*)(v121 + 5) = 0;
                *(int*)(v121 + 16) = 0;
                *(int*)(v121 + 8)  = 0;
                *(int*)(v121 + 12) = 0;
            } else {
                // Sword in tier 1 (400-431): bone 0, action[1].PlaySpeed
                *(unsigned char*)(v121 + 5) = 0;
                *(int*)(v121 + 16) = *(int*)((char*)GetPlayerActions() + 20);
            }
            went_to_render = true;
        }
        if (!went_to_render) {
            // v120 == 466 path
            if (v118 >= 0x23u && v118 <= 0x24u) {
                // Specific anim with weapon 466: bone 2, action[35].PlaySpeed
                *(unsigned char*)(v121 + 5) = 2;
                *(int*)(v121 + 16) = *(int*)((char*)GetPlayerActions() + 564);
            } else {
                // Default for 466: bone 1, action[1].PlaySpeed
                *(unsigned char*)(v121 + 5) = 1;
                *(int*)(v121 + 16) = *(int*)((char*)GetPlayerActions() + 20);
            }
        }


        // ── DIAG: log weapon render attempt for char-select ─────────────────
        if (DAT_005615c0 == 4) {
            int csSlot = (int)(((uintptr_t)c - (uintptr_t)DAT_07abf5d0) / 0x394);
            if (csSlot >= 0 && csSlot < 5) {
                static DWORD s_lastWP[5][2] = {{0,0},{0,0},{0,0},{0,0},{0,0}};
                DWORD now = GetTickCount();
                if (now - s_lastWP[csSlot][v234] > 1000) {
                    s_lastWP[csSlot][v234] = now;
                    char b[256];
                    BYTE linkBone = *(unsigned char*)(v121 + 4);
                    BYTE curAct   = *(unsigned char*)(v121 + 5);
                    float ps      = *(float*)(v121 + 16);
                    _snprintf_s(b, sizeof(b), _TRUNCATE,
                        "WP slot=%d hand=%d type=%d link=%d act=%d ps=%.3f anim=%d",
                        csSlot, v234, (int)v120, linkBone, curAct, ps, v118);
                    DbgLogPublic(b);
                }
            }
        }

        // ── Main weapon render ──────────────────────────────────────────────
        // Args 7-8 (param_7,param_8) = Level, Option1 (interpretados como
        // Translate/RenderType en nuestro 9-arg port, pero con los fixes
        // previos en RenderLinkObject ahora son Level/Option1 reales).
        // 9th arg ('\0') = Link flag → non-Link path → renderiza al bone.
        FUN_00455430(0.0f, 0.0f, 0.0f, c, v121,
                     v120,
                     *(unsigned char*)(v121 + 2),
                     *(unsigned char*)(v121 + 3),
                     '\0', '\x01', 0);

        // ── Light color from weapon level (v121+2) ──────────────────────────
        unsigned char v123 = *(unsigned char*)(v121 + 2);
        float Light[3];
        if (v123 < 7) {
            if (v123 < 5) {
                if (v123 < 3) {
                    Light[0] = Targetj * 0.30000001f;
                    Light[1] = Targetj * 0.30000001f;
                } else {
                    Light[0] = Targetj * 0.5f;
                    Light[1] = Targetj * 0.30000001f;
                }
                Light[2] = Light[1];
            } else {
                Light[0] = Targetj * 0.30000001f;
                Light[1] = Light[0];
                Light[2] = Targetj * 0.5f;
            }
        } else {
            Light[0] = Targetj * 0.5f;
            Light[1] = Targetj * 0.40000001f;
            Light[2] = Targetj * 0.30000001f;
        }

        // ── Weapon-specific FX (sólo en world bajo 6 y level != 4) ──────────
        unsigned char v126 = *(unsigned char*)(c + 746);
        if (DAT_005615c0 == 4 || (v126 < 6 && *(short*)(c + 446) != 4)) {
            RenderWeaponFX(c, o, v121, Targetj, Light);
        }
        }

    label_419:
        ++v234;
    } while (v234 < 2);
}
