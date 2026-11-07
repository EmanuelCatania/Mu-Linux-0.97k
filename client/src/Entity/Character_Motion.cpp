// Character_Motion.cpp
// Estado y movimiento por cuadro de los personajes: acciones de parado,
// caminar y muerte, clase y equipo del héroe, MoveCharacter y MoveCharacterVisual.
#include <cstdint>
#include "stdafx.h"
#include "UI/ChatListBox.h"
extern "C" const char* Guild_GetMarkName(int row);
#include "Text/TextCore.h"
#include "Party/Party.h"
#include "Entity/EntityName.h"
#include "Net/NetText.h"
#include "Render/TerrainDraw.h"
void Terrain_BeginBufferedPass();
#include "Render/BufferedDraw.h"
#include "Game/RenderFxSnapshots.h"
#include "Entity/ObjectBounds.h"
#include "Platform/Text.h"
#include "Platform/ISocket.h"
#include "Platform/ITimer.h"
#include "Platform/IInput.h"
#include "Item/ContentCatalog.h"
#include "Item/ItemDefines.h"
#include "Item/RightClickMove.h"
#include "Config/UserSettings.h"
#include "Game/MapManager.h"
#include "structs.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

// El rango de guild pide C1 plano (Encrypt=0).
extern "C" void GuildCreator_CloseFromResult(void);

// Inventory pool bases — defined in src/Render/HUD_Pass3.cpp.
// Lo usan los llamadores de FUN_004d23b0 de abajo para recorrer los arrays de grilla correctos.
extern "C" BYTE OffsetInventoryItems[];
extern "C" BOOL ChaosBoxRequestClose(void);
extern "C" BYTE Inventory[];
extern "C" BYTE ShopItems[];   // pool dedicado de la tienda
extern "C" BYTE OffsetTradeItems[];
extern "C" BYTE OffsetWarehouseItems[];
extern "C" BYTE OffsetMixItems[];
// Evento propio del Golden Archer (UI/GoldenArcher.cpp).
extern "C" int g_GoldenArcherCustom;
extern "C" bool __cdecl GoldenArcher_CustomNpcIdle(intptr_t c, int action);

// Player input helpers
// SetPlayerStop @ 0x004430C0 — SetPlayerStop(entity_ptr)
// Port directo del IDA sub_4430C0 (2155 bytes). Selecciona la animación
// idle/stop del personaje según el equipamiento (alas, armas, helm, armadura)
// y el estado del mundo (SceneFlag, World, terrain wall flag).
//
// Mapping de offsets:
//   c+0x002 short  EntityType (0x186 = Player)
//   c+0x010 float  WorldX
//   c+0x014 float  WorldY
//   c+0x105 byte   CurrentAction
//   c+0x106 byte   PriorAction
//   c+0x108 float  AnimationFrame
//   c+0x10C float  PriorAnimationFrame
//   c+0x1BC byte   Class      (low 3 bits = base class, &7==2 = Elf)
//   c+0x270 short  Helm.Type  (= c+624)
//   c+0x288 short  Armor.Type (= c+648)
//   c+0x2A0 short  Wing.Type  (= c+672)  ← controla Fly
//   c+0x2B8 short  Helper.Type(= c+696)  ← 818/819 trigger pet-fly
//   c+0x300 byte   stamina/state flag (cleared on entry)
//   c+0x34e byte   SafeZone (no dead — el dead real es +0x2FD)
//   c+0x2BE word   (= c+846) class-change indicator
//
// Hash-table refcounting (líneas 181-433 IDA) es anti-tamper, omitido.
extern std::uintptr_t DAT_07d78068;   // ItemAttribute base (declared in globals.h)

// IDA: SetPlayerStop (0x004430C0)
void __cdecl SetPlayerStop(CHARACTER* c) {
    AsCharacter(c)->Run = 0;   // c+768 = stamina counter

    auto SetAction_local = [c](unsigned char act) {
        if (AsCharacter(c)->Object.CurrentAction != act) {
            AsCharacter(c)->Object.PriorAction = AsCharacter(c)->Object.CurrentAction;
            AsCharacter(c)->Object.PriorAnimationFrame = AsCharacter(c)->Object.AnimationFrame;
            AsCharacter(c)->Object.CurrentAction = act;
            AsCharacter(c)->Object.AnimationFrame = 0.0f;
        }
    };

    // Helper para leer el byte TwoHand de ItemAttribute[type-399].
    // IDA: `*((_BYTE *)&ItemAttribute[v5 - 399] - 34)` → struct stride 0x40 con
    // un offset peculiar (-34 desde el inicio del elemento). Mu structures.h
    // muestra TwoHand en offset +30 dentro de ITEM_ATTRIBUTE. La expresión
    // `&arr[i] - 34` con sizeof=64 == (i-1)*64 + 30, o sea TwoHand del item
    // anterior. Mantenemos el cálculo idéntico al IDA para preservar semántica.
    // Se valida la base (DAT_07d78068 puede corromperse a valores CHICOS, ver
    // Item_GetAttribute en Item/Item_Inventory.cpp) y el rango de `type` (llega
    // como 0xFFFF con la mano vacía), igual que el resto de los accesos a
    // ItemAttribute.
    auto ItemTwoHand = [](int type) -> unsigned char {
        std::uintptr_t base = ItemAttribute_Base();
        if (!base) return 0;
        int idx = type - 399;
        if (idx < 1 || idx >= ITEM_MAX_EX) return 0;   // idx>=1: la expresión resta 34
        return *(unsigned char*)((char*)(uintptr_t)base + idx * 64 - 34);
    };

    if (AsCharacter(c)->Object.Type == 390) {
        // ── Player path ─────────────────────────────────────────────────────
        short v1 = AsCharacter(c)->Helper.Type;  // Helper.Type
        if ((v1 == 818 || v1 == 819) && !AsCharacter(c)->SafeZone) {
            unsigned char act = (*(unsigned short *)&AsCharacter(c)->Weapon[0].Type == 0xFFFF
                              && *(unsigned short *)&AsCharacter(c)->Weapon[1].Type == 0xFFFF) ? 11 : 12;
            SetAction_local(act);
            goto LABEL_129;
        }

        // Fly = 1 si NO está cambiando clase y Wing.Type != 0xFFFF
        char Fly = 0;
        if (!AsCharacter(c)->SafeZone && *(unsigned short *)&AsCharacter(c)->Wing.Type != 0xFFFF) {
            Fly = 1;
        }

        // Terrain wall check: en agua/Atlans (World==7) sin pared y SceneFlag==5
        // también dispara la animación de flotar. En char-select (SceneFlag==4)
        // queda solo Fly.
        bool gateA = false;
        if (SceneFlag == 5 && gMapManager.IsSwimmable(gMapManager.GetCurrentMap())) {   // IDA: World == 7
            int gx = (int)(AsCharacter(c)->Object.Position[0] * 0.0099999998f);
            int gy = (int)(AsCharacter(c)->Object.Position[1] * 0.0099999998f);
            int v3 = Terrain_GetTileIndex((unsigned int)gx, (unsigned int)gy);
            if ((DAT_0838bc70[v3] & 1) != 1) gateA = true;
        }
        if (gateA || Fly) {
            short v4 = AsCharacter(c)->Weapon[0].Type;  // Helm
            // bow/crossbow helms: 536-542, 544, 546 → action 10 (fly w/ bow)
            bool bowHelm = (v4 >= 536 && v4 < 543) || (v4 >= 544 && v4 < 545) || (v4 == 546);
            SetAction_local(bowHelm ? 10 : 9);
            goto LABEL_129;
        }

        short v5 = AsCharacter(c)->Weapon[0].Type;  // Helm
        // Sin helm/armor o cambiando clase fuera de mapas 11-16 → idle estándar
        if ((v5 == -1 && *(unsigned short *)&AsCharacter(c)->Weapon[1].Type == 0xFFFF)
            || (AsCharacter(c)->SafeZone
                && (gMapManager.GetCurrentMap() < 11 || gMapManager.GetCurrentMap() > 16))) {
            bool isElf = ((AsCharacter(c)->Class & 7) == 2);
            SetAction_local(isElf ? 2 : 1);
            goto LABEL_129;
        }

        // Helm 400-495: arma de melee → action 3 (one-hand) / 4 (two-hand) / 78 (special 431)
        if (v5 >= 400 && v5 < 496) {
            if (!ItemTwoHand(v5)) {
                SetAction_local(3);
                goto LABEL_129;
            }
            unsigned char act = (v5 == 431) ? 78 : 4;
            SetAction_local(act);
            goto LABEL_129;
        }

        // Helm 497-498: special weapons → action 5
        if (v5 == 497 || v5 == 498) {
            SetAction_local(5);
            goto LABEL_129;
        }

        // Helm 496-527 ó 560-591: depende de TwoHand → action 3 / 6
        if ((v5 >= 496 && v5 < 528) || (v5 >= 560 && v5 < 592)) {
            unsigned char act = ItemTwoHand(v5) ? 6 : 3;
            SetAction_local(act);
            goto LABEL_129;
        }

        // Resto: chequear armor (bow/crossbow staff/etc.) y clase Elf
        short v8 = AsCharacter(c)->Weapon[1].Type;
        if ((v8 >= 528 && v8 < 535) || v8 == 545) {  // Bows
            SetAction_local(7);
            goto LABEL_129;
        }
        if ((v5 >= 536 && v5 < 543) || (v5 >= 544 && v5 < 545) || v5 == 546) {  // Crossbows
            SetAction_local(8);
            goto LABEL_129;
        }
        {
            bool isElf = ((AsCharacter(c)->Class & 7) == 2);
            SetAction_local(isElf ? 2 : 1);
        }
    } else {
        // ── Non-player path (NPCs, monsters) ───────────────────────────────
        // IDA líneas 181-433: 4 hash-table operations sobre c+908 y c+904
        // (refcount + obfuscación XOR). Anti-tamper, no afecta el render.
        // Sólo el resultado final importa: terrain check + SetAction.
        int gx = (int)(AsCharacter(c)->Object.Position[0] * 0.0099999998f);
        int gy = (int)(AsCharacter(c)->Object.Position[1] * 0.0099999998f);
        int v50 = Terrain_GetTileIndex((unsigned int)gx, (unsigned int)gy);
        if (AsCharacter(c)->Object.Type == 302 && (DAT_0838bc70[v50] & 1) == 1) {
            SetAction_local(7);
        } else {
            SetAction_local(0);
        }
    }

LABEL_129:
    // 1/16 chance de reproducir un sonido random (idle voice)
    int rnd = rand() & 0x8000000F;
    bool zero = (rnd == 0);
    if (rnd < 0) {
        zero = (((unsigned char)rnd - 1) | 0xFFFFFFF0) == 0xFFFFFFFFu;
    }
    if (zero) {
        short v53 = AsCharacter(c)->Object.Type;
        if (v53 != 390 || (AsCharacter(c)->Object.SubType >= 206 && AsCharacter(c)->Object.SubType <= 208)) {
            // Models[v53].SoundIdle (offset 170)
            short modSnd = *(short*)((char*)(uintptr_t)DAT_05828d58 + BMD_STRIDE * v53 + 170 + BMD_FIELD_SHIFT);
            if (modSnd != (short)0xFFFF) {
                int v55 = rand() % 2;
                short s = *(short*)((char*)(uintptr_t)DAT_05828d58
                                    + BMD_STRIDE * v53 + 2 * v55 + 170 + BMD_FIELD_SHIFT);
                // PlayBuffer(s + 170, c, 0) — sound playback (PlayBuffer en nuestra port)
                PlayBuffer(s + 170, AsObject(c), 0);
            }
        }
    }
}

// SetPlayerWalk @ 0x00443930 — SetPlayerWalk (1337 bytes IDA)
// Full port: selects walk/run animation based on character class, equipped
// weapon, wings, stamina, and world.  IDA action IDs:
//   13 (0x0d) = walk no-weapon
//   14 (0x0e) = run no-weapon
//   15-28     = walk/run with various weapon combos (sword/bow/staff/spear/wand)
//   30-33     = wings active
//   76-77     = wings + Atlans world (8) or Aida (10) (swim)
//   79-80     = walk/run with item 431 (special weapon)
//   29        = exhausted walk in Atlans (world 7)
//   21        = walk in Atlans (world 7)
//   22        = run no-weapon (exhausted)
//   32-33     = wings stand/walk
//   2         = non-player default (NPC/monster)
//   0x20/0x21 = DarkLord stand/walk (legacy)
// IDA: SetPlayerWalk (0x00443930)
void __cdecl SetPlayerWalk(CHARACTER* param_1) {
    // +0x34E (=846) es **SafeZone**, NO dead (el dead real es +0x2FD).
    char bSafeZone0 = *(char *)&AsCharacter(param_1)->SafeZone;

    // Tick de stamina (IDA L27-56). El campo en +768 (= +0x300) es el contador de stamina.
    if (bSafeZone0) {
        AsCharacter(param_1)->Run = 0;
    } else {
        unsigned char v2 = AsCharacter(param_1)->Run;
        if (v2 < 0x28) {
            // 1) move_type bit = 3 (running): always increment.
            // 2) World 7 (Atlans): si c+0x240 (Wings1) != -1 Y c+0x242 (Wings1.lvl) > 4, incrementa.
            // 3) Si no, si c+0x258 (Helper) != -1 Y c+0x25a > 4, incrementa.
            if ((AsCharacter(param_1)->Class & 7) == 3) {
                AsCharacter(param_1)->Run = v2 + 1;
            } else if (gMapManager.IsSwimmable(gMapManager.GetCurrentMap())) {   // IDA: World == 7
                if (AsCharacter(param_1)->BodyPart[4].Type != -1 &&
                    AsCharacter(param_1)->BodyPart[4].Level > 4)
                    AsCharacter(param_1)->Run = v2 + 1;
            } else if (AsCharacter(param_1)->BodyPart[5].Type != -1 &&
                       AsCharacter(param_1)->BodyPart[5].Level > 4) {
                AsCharacter(param_1)->Run = v2 + 1;
            }
        }
    }

    // Action selection (IDA L57-227)
    short etype = AsCharacter(param_1)->Object.Type;

    // Non-player (NPC/monster): always action 2.
    if (etype != 390) {
        SetAction(AsObject(param_1), 2);
        goto label_119;
    }

    // Player path. Read pendant (c+696), L-hand (c+624), R-hand (c+648),
    // wings (c+672), stamina (c+768), is-dead (c+846).
    { // El salto de salida no entra en el ámbito de estas variables.
    short pendant = AsCharacter(param_1)->Helper.Type;
    short LH      = AsCharacter(param_1)->Weapon[0].Type;
    short RH      = AsCharacter(param_1)->Weapon[1].Type;
    short wings   = AsCharacter(param_1)->Wing.Type;
    unsigned char stamina = AsCharacter(param_1)->Run;
    char bSafeZone = *(char *)&AsCharacter(param_1)->SafeZone;

    // Pendant 818 + alive: stand-with-fairy (32) or holding-something (33).
    if (pendant == 818 && !bSafeZone) {
        if ((unsigned short)LH == 0xFFFF && (unsigned short)RH == 0xFFFF) {
            SetAction(AsObject(param_1), 32);
            goto label_119;
        }
        SetAction(AsObject(param_1), 33);
        goto label_119;
    }

    // Pendant 819 alive (master/wing pendant): special anim by world.
    if (pendant != 819 || bSafeZone) {
        // Las alas están activas cuando c+672 != -1 (sólo si está vivo).
        if (!bSafeZone && wings != -1) {
            // Wings flying: action 30 (no spear) or 31 (spear 536-543, 544, 546).
            if (LH >= 536 && LH < 543) { SetAction(AsObject(param_1), 31); goto label_119; }
            if (LH == 544 || LH == 546)    { SetAction(AsObject(param_1), 31); goto label_119; }
            SetAction(AsObject(param_1), 30);
            goto label_119;
        }
        // World 7 (Atlans): swim animations. Stamina < 0x28 → 21, else 29.
        // IDA L98: `if ( v7 && World == 7 )` — el gate `v7` faltaba. Analizando
        // los caminos que llegan acá, v7 == !SafeZone siempre:
        //   v6 = c+846 (SafeZone); v7 = (v6 == 0);
        //   if (!v6) { si hay alas → SetAction 30/31 y sale; si no, v7 = 1; }
        // O sea en zona segura NO se nada (ni se vuela): se camina.
        if (!bSafeZone && gMapManager.IsSwimmable(gMapManager.GetCurrentMap())) {   // IDA: World == 7
            SetAction(AsObject(param_1), (stamina < 0x28) ? 21 : 29);
            goto label_119;
        }
        // No weapons equipped (or dead in non-event-map world):
        bool noWeapons = ((unsigned short)LH == 0xFFFF) && ((unsigned short)RH == 0xFFFF);
        bool bSafeZoneNonEvent = bSafeZone && (gMapManager.GetCurrentMap() < 11 || gMapManager.GetCurrentMap() > 16);
        if (noWeapons || bSafeZoneNonEvent) {
            if (stamina >= 0x28) {
                // Exhausted: action 22.
                SetAction(AsObject(param_1), 22);
                goto label_119;
            }
            // Non-exhausted no-weapon walk/run: 13 (walk) or 14 (run).
            unsigned int act = ((AsCharacter(param_1)->Class & 7) == 2) ? 14u : 13u;
            SetAction(AsObject(param_1), act);
            goto label_119;
        }
        // Tiene armas, está vivo, no es Atlans, no está exhausto:
        // TwoHand: lookup REAL en
        // ItemAttribute table (DAT_07d78068, stride 64). IDA hace
        // *((BYTE*)&ItemAttribute[N-399] - 34) — stride 64, byte-offset 30
        // dentro del item anterior. Mantener idéntico para preservar semántica.
        auto ItemTwoHand = [](short type) -> unsigned char {
            if (DAT_07d78068 == 0) return 0;
            return *(unsigned char*)((char*)(uintptr_t)DAT_07d78068 + (type - 399) * 64 - 34);
        };
        if (stamina < 0x28) {
            // Sword class (LH 400..495):
            if (LH >= 400 && LH < 496) {
                if (ItemTwoHand(LH)) {
                    SetAction(AsObject(param_1), (LH == 431) ? 79 : 16);
                    goto label_119;
                }
                SetAction(AsObject(param_1), 15);   // 1H sword walk
                goto label_119;
            }
            // Lanza (560..591): IDA chequea TwoHand → 18, si no cae a LABEL_64 (15)
            if (LH >= 560 && LH < 592) {
                if (!ItemTwoHand(LH)) {
                    SetAction(AsObject(param_1), 15);  // LABEL_64
                    goto label_119;
                }
                SetAction(AsObject(param_1), 18);
                goto label_119;
            }
            // Mace (497) / War-axe (530):
            if (LH == 497 || LH == 530) { SetAction(AsObject(param_1), 17); goto label_119; }
            // IDA L155-176: NOT magic-book range (LH < 496 || LH >= 528):
            //   Bow RH → 19, Staff LH → 20, fallthrough → LABEL_87 (13/14).
            // SI NO (mano izquierda en [496, 528)): SetAction 18 (terminal).
            if (LH < 496 || LH >= 528) {
                if ((RH >= 528 && RH < 535) || RH == 545) {
                    SetAction(AsObject(param_1), 19);
                    goto label_119;
                }
                if ((LH >= 536 && LH < 543) || LH == 544 || LH == 546) {
                    SetAction(AsObject(param_1), 20);
                    goto label_119;
                }
                // LABEL_87: no-weapon walk class-conditional
                unsigned int act = ((AsCharacter(param_1)->Class & 7) == 2) ? 14u : 13u;
                SetAction(AsObject(param_1), act);
                goto label_119;
            }
            // LH in [496, 528): magic books — terminal action 18
            SetAction(AsObject(param_1), 18);
            goto label_119;
        }
        // Stamina >= 0x28 (exhausted) with weapons:
        if (LH >= 400 && LH < 496) {
            if (RH >= 400 && RH < 496) {
                SetAction(AsObject(param_1), 24);     // dual-sword exhausted
                goto label_119;
            }
            if (ItemTwoHand(LH)) {
                SetAction(AsObject(param_1), (LH == 431) ? 80 : 25);
                goto label_119;
            }
            SetAction(AsObject(param_1), 23);
            goto label_119;
        }
        // Spear exhausted: TwoHand → 26, else 23 (LABEL_97)
        if (LH >= 560 && LH < 592) {
            if (!ItemTwoHand(LH)) {
                SetAction(AsObject(param_1), 23);  // LABEL_97
                goto label_119;
            }
            SetAction(AsObject(param_1), 26);
            goto label_119;
        }
        if (LH >= 496 && LH < 528) {
            SetAction(AsObject(param_1), 26);
            goto label_119;
        }
        if ((RH >= 528 && RH < 535) || RH == 545) {
            SetAction(AsObject(param_1), 27);
            goto label_119;
        }
        if ((LH >= 536 && LH < 543) || LH == 544 || LH == 546) {
            SetAction(AsObject(param_1), 28);
            goto label_119;
        }
        SetAction(AsObject(param_1), 22);  // exhausted no-weapon walk (LABEL_59)
        goto label_119;
    }

    // Pendant 819 alive: world 8 (Tarkan) / world 10 (Aida) wings:
    if (gMapManager.GetCurrentMap() != 8 && gMapManager.GetCurrentMap() != 10) {
        if ((unsigned short)LH == 0xFFFF && (unsigned short)RH == 0xFFFF) {
            SetAction(AsObject(param_1), 32);
            goto label_119;
        }
        SetAction(AsObject(param_1), 33);
        goto label_119;
    }
    // Wings + Atlans/Aida: swim wings.
    SetAction(AsObject(param_1),
                 ((unsigned short)LH == 0xFFFF && (unsigned short)RH == 0xFFFF) ? 76 : 77);

    }
label_119:

    // Entity type 0x129 — special case with a fixed sound
    if (etype == 0x129) {
        PlayBuffer(0x5e, AsObject(param_1), 0);
        return;
    }
    // Sonido de movimiento aleatorizado (1/64 para el jugador, 1/16 para el resto)
    if (param_1 == AsCharacter(DAT_07abf5d8)) {
        if ((rand() & 0x3f) != 0) return;
    } else {
        if ((rand() & 0xf) != 0) return;
    }
    // Reproduce el sonido de paso de la tabla de sonidos del modelo si no es DarkLord (o si está en el rango de altura de paso)
    if (etype != 0x186 || (AsCharacter(param_1)->Object.SubType > 0xcd && AsCharacter(param_1)->Object.SubType < 0xd1)) {
        if (AsCharacter(param_1)->Object.SubType > 0xcd && AsCharacter(param_1)->Object.SubType < 0xd1)
            PlayBuffer(0x5d, AsObject(param_1), 0);
        if (*(short *)(DAT_05828d58 + 0xaa + BMD_FIELD_SHIFT + etype * BMD_STRIDE) != -1) {
            // IDA 0x443930 L288-289:
            //     v19 = rand() % 2;
            //     PlayBuffer(*(short*)(Models + 2*(v19 + 94*type) + 170) + 170, c, 0);
            // o sea la tabla tiene SOLO 2 entradas (los dos sonidos de paso del
            // modelo) y se elige una al azar. Indexarla con el action ID (+0x105)
            // leería campos ajenos del struct de modelo.
            int v19 = rand() % 2;
            int iVar9 = 0;
            PlayBuffer(AsModel(DAT_05828d58 + etype * BMD_STRIDE)->Sounds[v19] + 0xaa, AsObject(param_1), iVar9);
        }
    }
}

// MoveCharacterPosition @ 0x00454BA0 — Entity_StopMove(entity_ptr)
// Applies backward velocity step (CharacterMoveSpeed) rotated by entity facing matrix,
// y después fija la altura de la entidad a la del terreno (RequestTerrainHeight).
// IDA: MoveCharacterPosition (0x00454BA0)
void __cdecl MoveCharacterPosition(CHARACTER* character) {
    BYTE* param_1 = reinterpret_cast<BYTE*>(character);
    float local_30[12];
    float vel[3] = { 0.0f, -(float)CharacterMoveSpeed(AsCharacter(param_1)), 0.0f };
    // El mismo artefacto de float[3] partido por Ghidra que en Terrain_Light Entity_GetLightScale.
    // local_3c/local_38/local_34 eran el buffer de salida contiguo de 3 floats que
    // esperaba Vector_Rotate, pero MSVC no garantiza el layout de los locales.
    float out[3] = {0.0f, 0.0f, 0.0f};
    Matrix_BuildFromEuler((float*)(param_1 + 0x1c), local_30);
    Vector_Rotate(vel, local_30, out);
    *(float*)(param_1 + 0x10) = out[0] + *(float*)(param_1 + 0x10);
    *(float*)(param_1 + 0x14) = out[1] + *(float*)(param_1 + 0x14);
    *(float*)(param_1 + 0x18) = out[2] + *(float*)(param_1 + 0x18);
    float terrainH = RequestTerrainHeight(*(float*)(param_1 + 0x10), *(float*)(param_1 + 0x14));
    if (*(short*)&((CHARACTER*)(uintptr_t)(param_1))->Helper.Type == 0x333) {
        if (gMapManager.GetCurrentMap() == 8 || gMapManager.GetCurrentMap() == 10)
            terrainH += _DAT_00552848;
        else
            terrainH += _DAT_0055284c;
    }
    *(float*)(param_1 + 0x18) = terrainH;
    // IDA 00454BA0: el modelo 272 (Budge Dragon) aletea sobre el terreno.
    // sub_5129F0 es fabs y deja su resultado en st0, por eso el decompile no lo muestra.
    if (*(short*)(param_1 + 2) == 272) {
        const float bob = (float)fabs(sin(*(float*)(param_1 + 0x80)));
        *(float*)(param_1 + 0x18) = *(float*)(param_1 + 0x18) - bob * 70.0f + 70.0f;
    }
    *(float*)(param_1 + 0x80) = *(float*)(param_1 + 0x80) + _DAT_00552934;
}

// 0.97.20: vuelve a aplicar al héroe lo que el CharSet no lleva (las piezas
// del catálogo y el pet custom) a partir de CharacterMachine.  Lo necesita la
// fila propia del 0x12: `ChangeCharacterExt` limpia las piezas, borra el bug
// del pet y deja el helper en "ninguno", y como el equipo no cambió,
// SetCharacterClass no vuelve a correr.
void Hero_ApplyCatalogEquipment(CHARACTER* character)
{
    BYTE* c = reinterpret_cast<BYTE*>(character);
    if (!DAT_07cf1ffc || *(short*)(c + 2) != 390) return;
    const BYTE* v7 = static_cast<const BYTE*>(DAT_07cf1ffc) + 536;
    static const int kWearOff[9] = { 136, 204, 272, 340, 408, 0, 68, 476, 544 };
    for (int part = 0; part < 9; ++part)
        gContentCatalog.SetEntityPart(c, part, *(short*)(v7 + kWearOff[part]));
    const short pet = *(short*)(v7 + 544);
    if (pet != -1 && gContentCatalog.GetPet(pet)) {
        *(short*)&((CHARACTER*)(uintptr_t)(c))->Helper.Type = (short)ItemModel(ItemBehaviorType(pet));
        // El owner del pool aún ocupa 32 bits; su layout queda para B2.
        DeleteBug((intptr_t)c);
        CreateBug(*(short*)&((CHARACTER*)(uintptr_t)(c))->Helper.Type, (void*)(c + 16), (void*)(uintptr_t)c, 0);
    }
}

// SetCharacterClass @ 0x0045C130 — SetCharacterClass(entity)
//
// Port completo del decompile de IDA:
//
//   1. Sólo opera sobre entity_type == 390 (= 0x186, local player).
//   2. Copia cluster primario (4 slots: 624/648/672/696) con +400 offset.
//   3. Si entity[+847] != 0 (caso "ya seteado"): early return.
//   4. Si entity[+847] == 0: copia cluster secundario (504/528/552/576/600)
//      con fallback formula `(skin&7) + 4*(skin>>3) + base` cuando el slot
//      del CharData es -1.
//   5. SetCharacterScale + sub_47E3C0 al final del else branch.
//
// Hashtable obfuscation y comparación con `Hero` global: omitidos (anti-tamper
// noise / no requeridos para gameplay observable).
// IDA: SetCharacterClass (0x0045C130)
void __cdecl SetCharacterClass(CHARACTER* character) {
    BYTE* c = reinterpret_cast<BYTE*>(character);
    if (AsCharacter(c)->Object.Type != 390) return;   // 390 = 0x186 = local player

    char* cd = (char*)DAT_07cf1ffc;
    BYTE* v7 = reinterpret_cast<BYTE*>(cd) + 536;   // matches IDA: CharacterMachine + 536

    // ── Primary cluster (4 equipment slots → entity offsets 624/648/672/696) ──
    auto writeSlot = [&](int srcOff, int eOff) {
        short v = *(short*)(v7 + srcOff);
        *(short*)(c + EntityOffset(eOff)) = (v == -1) ? (short)-1 : (short)ItemEntityModel(v);
    };
    // Valor previo del helper, para detectar el CAMBIO abajo.
    const short prevHelper = AsCharacter(c)->Helper.Type;
    writeSlot(0,   624);
    writeSlot(68,  648);
    writeSlot(476, 672);
    writeSlot(544, 696);
    // 0.97.20: el helper guarda el pet vanilla que imita (bugs, monturas y
    // animaciones se deciden por ese tipo); el modelo propio se dibuja en
    // RenderBugs / Render_PlayerHelper.
    {
        const short pet = *(short*)(v7 + 544);
        if (pet != -1) AsCharacter(c)->Helper.Type = (short)ItemModel(ItemBehaviorType(pet));
    }

    // 0.97.20: modelo propio de cada pieza puesta (catálogo).
    {
        static const int kWearOff[9] = { 136, 204, 272, 340, 408, 0, 68, 476, 544 };
        for (int part = 0; part < 9; ++part)
            gContentCatalog.SetEntityPart(c, part, *(short*)(v7 + kWearOff[part]));
    }

    // ── Spawn/borrado del pet del HEROE (Guardian Angel y monturas) ─────────
    // El pet NO se dibuja desde RenderCharacter: `ChangeCharacterExt`
    // (0x45C8C0 L73-88) lo crea como entidad del pool de bugs —
    //     *(WORD *)&AsCharacter(c)->Helper.Type = Type + 816;
    //     CreateBug(816 | 195 | 267, c + 16, c, 0, 0);
    // — y de ahi lo tickea MoveBugs y lo dibuja RenderBugs. Por eso
    // RenderCharacter solo tiene rama para el Imp (817) y ninguna para el 816.
    //
    // Pero `ChangeCharacterExt` se llama SOLO con el CharSet que llega por red:
    // char-select (ReceiveCharacterList L68), otros jugadores del viewport
    // (Combat_PacketDispatch L317) y el F3/13 (ProtocolCore L1629). El equipo
    // del HEROE in-world no viene por ahi sino de CharacterMachine, o sea pasa
    // por esta funcion.
    //
    // DESVIACION documentada: IDA no crea el bug en SetCharacterClass. No
    // encontre el camino por el que el original se lo da al heroe; puede estar
    // en el arrastre del pool desde char-select. Replicamos aca el par
    // DeleteBug+CreateBug de ChangeCharacterExt, gateado al CAMBIO de tipo para
    // no re-spawnear en cada llamada (esta funcion corre al cambiar el equipo,
    // no por frame).
    {
        const short newHelper = AsCharacter(c)->Helper.Type;
        if (newHelper != prevHelper) {
            // El owner del pool aún ocupa 32 bits; su layout queda para B2.
            DeleteBug((intptr_t)c);          // DeleteBug
            int bugType = 0;
            if      (newHelper == 816) bugType = 816;   // Guardian Angel
            else if (newHelper == 818) bugType = 195;   // Uniria
            else if (newHelper == 819) bugType = 267;   // Dinorant
            else if (gContentCatalog.GetPet(*(short*)(v7 + 544))) bugType = newHelper;   // 0.97.20: CustomPet
            // 817 (Imp) NO lleva bug: lo dibuja RenderLinkObject desde
            // Render_PlayerHelper, fiel a RenderCharacter L1267-1287.
            if (bugType)
                CreateBug(bugType, (void*)(c + 16), (void*)(uintptr_t)c, 0);
        }
    }

    AsCharacter(c)->Weapon[0].Level = (unsigned char)((*(int*)(v7 + 4)   >> 3) & 0xF);
    AsCharacter(c)->Weapon[1].Level = (unsigned char)((*(int*)(v7 + 72)  >> 3) & 0xF);
    AsCharacter(c)->Weapon[0].Option1 = *(unsigned char*)(v7 + 27);
    AsCharacter(c)->Weapon[1].Option1 = *(unsigned char*)(v7 + 95);
    unsigned char v11 = AsCharacter(c)->Object.CurrentAction;   // anim_state @ 0x105
    AsCharacter(c)->Wing.Level = (unsigned char)((*(int*)(v7 + 480) >> 3) & 0xF);
    AsCharacter(c)->Helper.Level = (unsigned char)((*(int*)(v7 + 548) >> 3) & 0xF);

    bool skipCancel = (v11 >= 0x85u && v11 <= 0x8Cu);
    if (!skipCancel && (v11 < 0x22u || v11 > 0x5Bu)) {
        SetPlayerStop(AsCharacter(c));   // SetPlayerStop
    }

    // entity[+847] == 0 → secondary cluster needs filling
    if (AsCharacter(c)->Change) return;

    // ── Secondary cluster (5 slots → entity offsets 504/528/552/576/600) ──
    // Cuando el CharData slot vale -1, fallback a `(skin&7)+4*(skin>>3)+base`.
    unsigned char skin = AsCharacter(c)->Class;
    unsigned char skinLo = skin & 7;
    unsigned char skinHi = skin >> 3;

    short v17 = *(short*)(v7 + 136);
    AsCharacter(c)->BodyPart[1].Type = (v17 == -1)
        ? (short)(skinLo + 4 * skinHi + 912)
        : (short)ItemEntityModel(v17);

    short v21 = *(short*)(v7 + 204);
    AsCharacter(c)->BodyPart[2].Type = (v21 == -1)
        ? (short)(skinLo + 4 * skinHi + 919)
        : (short)ItemEntityModel(v21);

    short v22 = *(short*)(v7 + 272);
    AsCharacter(c)->BodyPart[3].Type = (v22 == -1)
        ? (short)(skinLo + 4 * skinHi + 926)
        : (short)ItemEntityModel(v22);

    short v26 = *(short*)(v7 + 340);
    AsCharacter(c)->BodyPart[4].Type = (v26 == -1)
        ? (short)(skinLo + 4 * skinHi + 933)
        : (short)ItemEntityModel(v26);

    short v27 = *(short*)(v7 + 408);
    AsCharacter(c)->BodyPart[5].Type = (v27 == -1)
        ? (short)(skinLo + 4 * skinHi + 940)
        : (short)ItemEntityModel(v27);

    AsCharacter(c)->BodyPart[1].Level = (unsigned char)((*(int*)(v7 + 140) >> 3) & 0xF);
    AsCharacter(c)->BodyPart[2].Level = (unsigned char)((*(int*)(v7 + 208) >> 3) & 0xF);
    AsCharacter(c)->BodyPart[3].Level = (unsigned char)((*(int*)(v7 + 276) >> 3) & 0xF);
    AsCharacter(c)->BodyPart[4].Level = (unsigned char)((*(int*)(v7 + 344) >> 3) & 0xF);
    AsCharacter(c)->BodyPart[5].Level = (unsigned char)((*(int*)(v7 + 412) >> 3) & 0xF);
    AsCharacter(c)->BodyPart[1].Option1 = *(unsigned char*)(v7 + 163);
    AsCharacter(c)->BodyPart[2].Option1 = *(unsigned char*)(v7 + 231);
    AsCharacter(c)->BodyPart[3].Option1 = *(unsigned char*)(v7 + 299);
    AsCharacter(c)->BodyPart[4].Option1 = *(unsigned char*)(v7 + 367);
    AsCharacter(c)->BodyPart[5].Option1 = *(unsigned char*)(v7 + 435);

    SetCharacterScale(AsCharacter(c));
    // Recalcula los ocho grupos de atributos y sus campos derivados.
    CalculateAll(reinterpret_cast<BYTE*>(cd), 0, 0);
}

// sub_47E3C0 @ 0x0047E3C0 (293 bytes) — CharData_RecalcStats wrapper.
// Port FIEL desde IDA decompile. Calls 8 stat helpers in order
// then computes derived stats:
//   - this[+1396] = this[+1388] - this[+1378]   (rango de stat máx - mín)
//   - this[+1398] = this[+1390] - this[+1378]
//   - this[+1400] = this[+1374] + rand%(1376-1374+1) - this[+78]  (tirada de daño aleatorio)
//   - this[+1402] = this[+58]  - this[+1384] (clampeado a 100)  (% de crítico)
//   - this[+1404] = this[+76]  - this[+1382] (clampeado a 100)  (% de esquive)
//   - this[+1406] = (rand()%100 < this[+1402]) ? 1 : 0  (tirada de crítico)
//   - this[+1407] = (rand()%100 < this[+1404]) ? 1 : 0  (tirada de esquive)
// IDA: FUN_0047E3C0 (0x0047E3C0)
int __cdecl CalculateAll(BYTE* characterMachine, int /*p2*/, int /*p3*/) {
    BYTE* this_ = characterMachine;
    if (!this_) return 0;

    // Anti-tamper hash table — skipped per project policy

    Stats_CalcBase(this_);            // Stats_CalcBase (attack damage)
    Stats_CalcMagicDmgRange(this_);            // Stats_CalcMagicDmgRange
    Stats_CalcAddStrength((short*)this_);    // Stats_CalcAddStrength
    CalculateAttackSpeed(this_);            // CalculateAttackSpeed
    Stats_CalcDefense(this_);            // Stats_CalcDefense
    Stats_CalcDefenseRate(this_);            // Stats_CalcCritBase / DefRate
    Stats_ExtraOptionEquip6((short*)this_);    // Stats_CalcExtraOption1
    Stats_ExtraOptionGlovesWings(this_);            // Stats_CalcExtraOption2

    // Derived stats: max-min ranges
    unsigned short v2 = *(unsigned short*)(this_ + 1390);
    *(unsigned short*)(this_ + 1396) = *(unsigned short*)(this_ + 1388) - *(unsigned short*)(this_ + 1378);
    *(unsigned short*)(this_ + 1398) = v2 - *(unsigned short*)(this_ + 1378);

    // Random damage roll
    unsigned short v3 = *(unsigned short*)(this_ + 1374);
    int range = *(unsigned short*)(this_ + 1376) - v3 + 1;
    if (range < 1) range = 1;
    int v4 = rand() % range;
    *(unsigned short*)(this_ + 1400) = (unsigned short)(v3 + v4 - *(unsigned short*)(this_ + 78));

    // Crit chance % (clamped to 100)
    unsigned short v5 = *(unsigned short*)(this_ + 76);
    unsigned short v6 = (unsigned short)(*(unsigned short*)(this_ + 58) - *(unsigned short*)(this_ + 1384));
    *(unsigned short*)(this_ + 1402) = v6;
    *(unsigned short*)(this_ + 1404) = (unsigned short)(v5 - *(unsigned short*)(this_ + 1382));
    if (*(unsigned short*)(this_ + 1402) > 100) *(unsigned short*)(this_ + 1402) = 100;
    if (*(unsigned short*)(this_ + 1404) > 100) *(unsigned short*)(this_ + 1404) = 100;

    // Did-crit / did-dodge rolls
    *(unsigned char*)(this_ + 1406) = (rand() % 100 < *(unsigned short*)(this_ + 1402)) ? 1 : 0;
    int v7 = rand();
    *(unsigned char*)(this_ + 1407) = ((v7 % 100) < *(unsigned short*)(this_ + 1404)) ? 1 : 0;
    return v7 / 100;
}
// CheckGate @ 0x004AC140.
// GateAttribute: 100 registros de 9 bytes (activo, mapa, minX, minY, maxX, maxY,
// gate destino, direccion, nivel minimo).  Si el heroe pisa el rectangulo pide
// al server el gate con C1:06:1C:<gate>:00:00.
//
// Detalles fieles a IDA:
//  - el recorrido NO corta al encontrar un gate: cada rama termina en
//    LABEL_121 y sigue con el siguiente registro;
//  - `LoadingWorld = 50` al rechazar por nivel (antirrebote del aviso) y
//    `LoadingWorld = 9999999` (`(int)&unk_98967F`) al pedir el viaje, que
//    ReceiveTeleport baja a 30; si el pedido se descarta por el antirrebote
//    vuelve a 0;
//  - los avisos van con etiqueta vacia (byte_7E11E50.. son BSS sin escritor),
//    no "ERROR".


// ─────────────────────────────────────────────────────────────────────────────
// IDA: MoveCharacter (0x00449900)
// ─────────────────────────────────────────────────────────────────────────────
// Tick de entidad por frame. Decompile de IDA: 2773 líneas / 32793 bytes; ~70% es
// anti-tamper hash-table noise (dword_55C9BC8/BCC/BD0/BD4 + sub_403F80/04280/
// 04330/0423710/0404400 ref-count + XOR encryption around CharacterMachine
// lecturas). Per la política del proyecto todas las operaciones de hash-table se saltean
// — son ofuscación, no lógica de juego.
//
// El llamador pasa la misma entidad como `c` y como `o` (MoveCharacterClient → cc, cc).
// Mapped: `c == o == ent`. Behavior:
//   1. Sync entity world pos+rot → Models[entType] render slot
//   2. Hero-only: decrement attack/magic speed buff timers (CA+42/44),
//      recalc stats when timer expires
//   3. Stationary NPC tagging (event NPCs 206..208 in cities)
//   4. Knockback/tambaleo (c+772) y deslizamiento al objetivo (c+773)
//   5. Pickup-bag bouncing physics (entType=236)
//   6. Death dissolve (c+765 ragdoll)
//   7. CharacterAnimation() drives idle action selection
//   8. Attack swing (c+757) → AttackStage / AttackEffect
//   9. Bone particle effect spawn (o+404)
//  10. Skill effect dispatch from c+770 queue (cases 5/8/9/10/12/13/14/30..36/
//      41/42/48/49/55/56) → CreateEffect/CreateJoint/PlayBuffer
//  11. Targeted-skill effects when c+784 != -1 (cases 1/2/4/7/11/16/17/24/26/27/
//      28/51/52)
//  12. Joint chain rendering (c+816 ∈ 1253/1254/1256/1260) — sword/spear trail
//  13. Weapon blur trail (CreateBlur) when AnimFrame >= 3.0
//  14. Pickup-bag splash (entType=236)
//
// Original signature: `void __cdecl MoveCharacter(DWORD c, DWORD o)`. Our ABI
// es `void __cdecl MoveCharacter(int p1)`; los dos argumentos son la misma entidad.
//
// Dependencias (todas ya en el árbol como FUN_xxxxxxxx):
//   Stats_CalcBase   Stats_CalcBase (sub_47D410)
//   Stats_CalcMagicDmgRange   Stats_CalcMagicDmgRange (sub_47DAE0)
//   CalculateAttackSpeed   CHARACTER_MACHINE::CalculateAttackSpeed
//   RequestTerrainHeight   RequestTerrainHeight
//   Particle_Spawn   Particle_Spawn
//   CharacterAnimation
//   SetPlayerStop   SetPlayerStop
//   SetAction   SetAction
//   CreateChat   CreateChat
//   CreateBlood
//   DeleteCloth   DeleteCloth
//   AttackStage
//   AttackEffect   AttackEffect
//   CreateEffect   CreateEffect
//   Joint_Create   CreateJoint
//   Effect_SpawnSmokeBurst   CreateBomb
//   BMD_TransformPosition   BMD::TransformPosition
//   BMD_Animation   BMD::Animation
//   AngleMatrix    (no FUN_)
//   VectorRotate   = Vector_InverseRotate
//   FUN_004b1170   FindHotKey
//   CreateArrows
//   Math_Fabs   fabs
//   FUN_0046fe40   Joint_Find
//   FUN_004451c0   AngleVectorOffset
//   Effect_SpawnBombRing   bomb-ring effect
//   FUN_0046c5a0   skill impact particles
//   FUN_0046c7f0   directional blood
//   FUN_0045fae0   lectura hash de 1 byte (lectura de la cola de skills)
//   Alpha   Alpha
//   PlayBuffer   PlayBuffer
//   SetPlayerDie / DeleteJoint / CreateBlur — local helpers below
//
// Anti-tamper SKIPPED everywhere — all `if (c == Hero) { hash-decrypt; ...
// hash-encrypt; }` colapsan a una lectura directa de los campos de CharacterMachine.
//
// Declaraciones externas locales a esta unidad de traducción:
extern "C" bool __cdecl CharacterAnimation(intptr_t c, intptr_t o);

extern bool __cdecl AttackStage(intptr_t c, intptr_t o);
extern void __cdecl CreateBlood(intptr_t o);
extern int  __stdcall FindHotKey(int Skill);
extern void __cdecl CreateArrows(intptr_t c, intptr_t o, intptr_t to, WORD SkillIndex, WORD Skill, WORD SKKey);
extern unsigned char __cdecl FUN_0045fae0(DWORD ecx, unsigned char* p);

// Helpers definidos localmente, usados sólo por MoveCharacter:
static inline void mc_AngleVectorOffset(float *origin7, float ax, float ay, float az, float out[3])
{
    // sub_4451C0 — offset = origin[4..6] + Rotate([ax,ay,az], AngleMatrix(origin[7..9]))
    float in1[3]  = { ax, ay, az };
    float in2[3][4];
    AngleMatrix(origin7 + 7, in2);
    VectorRotate(in1, (float*)in2, out);
    out[0] += origin7[4];
    out[1] += origin7[5];
    out[2] += origin7[6];
}

// `DeleteJoint` real (0x0046FE00): el skill 16 spawnea 5 joints 266 con
// Scale 20 y ANTES hace `DeleteJoint(266, Owner, 0)` para sacar los que ya
// hubiera — p.ej. los de Scale 50 que deja `InsertBuffPhysicalEffect`
// (0x43BDE0, el camino de viewport cuando la entidad aparece con el buff ya
// activo).  Si esto fuera un no-op convivirían los dos grupos (arcos dobles).
extern "C" void __cdecl DeleteJoint(int Type, intptr_t Target, int SubType);
static inline void mc_DeleteJoint(int Type, intptr_t Owner, int SubType)
{
    DeleteJoint(Type, Owner, SubType);
}

static inline void mc_CreateBlur(intptr_t c, float* p1, float* p2,
                                 float* color, int type, int flag)
{
    // CreateBlur (0x0046C320) — la estela que sigue la hoja del arma.  Busca un
    // slot del pool ya asignado a este owner y le agrega el par de puntos
    // (punta/base de la hoja); si no hay, toma el primero libre.  El pool es
    // g_RenderPool_07c608a8 (100 slots x 0x2f0), compartido con Trail_RenderAll,
    // que es quien lo dibuja.  Vida 15 (interpolado) o 30, tope de 29 segmentos.
    BYTE* const base = (BYTE*)g_RenderPool_07c608a8;
    constexpr int slotSize = 0x2f0;
    constexpr int slotCount = 100;

    for (int index = 0; index < slotCount; ++index) {
        BYTE* slot = base + index * slotSize;
        if (slot[0] && g_TrailOwner[index] == (uintptr_t)c) {
            BYTE* b = slot;
            *(int*)(b + 4) = type;
            *(float*)(b + 20) = color[0]; *(float*)(b + 24) = color[1]; *(float*)(b + 28) = color[2];
            const int previousCount = *(int*)(b + 16) - 1;
            for (int item = previousCount; item >= 0; --item) {
                float* dstA = (float*)(b + 44 + item * 12); float* srcA = dstA - 3;
                dstA[0] = srcA[0]; dstA[1] = srcA[1]; dstA[2] = srcA[2];
                float* dstB = (float*)(b + 404 + item * 12); float* srcB = dstB - 3;
                dstB[0] = srcB[0]; dstB[1] = srcB[1]; dstB[2] = srcB[2];
            }
            int count = *(int*)(b + 16) + 1;
            *(float*)(b + 32) = p1[0]; *(float*)(b + 36) = p1[1]; *(float*)(b + 40) = p1[2];
            *(float*)(b + 392) = p2[0]; *(float*)(b + 396) = p2[1]; *(float*)(b + 400) = p2[2];
            *(int*)(b + 16) = count >= 29 ? 29 : count;
            return;
        }
    }
    for (int index = 0; index < slotCount; ++index) {
        BYTE* slot = base + index * slotSize;
        if (!slot[0]) {
            *(DWORD*)(slot + 12) = (DWORD)c;
            g_TrailOwner[index] = (uintptr_t)c;
            RenderFxSnapshots_New(slot);
            slot[0] = 1;
            *(int*)(slot + 16) = 0;
            *(int*)(slot + 8) = flag ? 15 : 30;
            *(int*)(slot + 4) = type;
            *(float*)(slot + 20) = color[0]; *(float*)(slot + 24) = color[1]; *(float*)(slot + 28) = color[2];
            *(float*)(slot + 32) = p1[0]; *(float*)(slot + 36) = p1[1]; *(float*)(slot + 40) = p1[2];
            *(float*)(slot + 392) = p2[0]; *(float*)(slot + 396) = p2[1]; *(float*)(slot + 400) = p2[2];
            *(int*)(slot + 16) = 1;
            return;
        }
    }
}

static inline char mc_JointFind(int Type, intptr_t Owner, int flag)
{
    // sub_46FE40 (SearchJoint). IDA ancla en &Joints[0].Target y lee los otros
    // campos con índices negativos; flag == -1 ignora el SubType.
    const JOINT* joints = AsJoint(DAT_07b27150);
    const int count = (int)(sizeof(DAT_07b27150) / sizeof(JOINT));
    for (int i = 0; i < count; ++i) {
        const JOINT* j = &joints[i];
        if (j->Live == 0) continue;
        if (j->Type != Type) continue;
        if ((intptr_t)j->Target != Owner) continue;
        if (flag != -1 && j->SubType != flag) continue;
        return 1;
    }
    return 0;
}

// SetPlayerDie @ 0x00444D90 — SetPlayerDie (1057 bytes IDA, port FIEL).
// Real signature: void __cdecl SetPlayerDie(DWORD c).
//
// Differentiation:
//   - Player (type 390), normal class (NOT 206-208): SetAction(c, 131)  ← death anim
//   - Player class 206-208 (special): explosion FX (210 + 211×10), c[0]=0
//   - NPC type 295 (=v13==0): explosion FX (226+227)×8, c[0]=0
//   - NPC type 300 (=v13==5): explosion FX (210 + 211×10), c[0]=0
//   - Other NPCs: SetAction(c, 6)  ← death anim
// El ruido de hash table (camino sólo-Hero, L32-119) se saltea per la política del proyecto.
// IDA: SetPlayerDie (0x00444D90)
void __cdecl SetPlayerDie(CHARACTER* c_in)
{
    BYTE* c = reinterpret_cast<BYTE*>(c_in);
    if (!c) return;

    short v11 = *(short*)(c + 2);
    bool playFxBuf = false;

    if (v11 == 390) {
        int v12 = *(int*)(c + 4);
        if (v12 < 206 || v12 > 208) {
            SetAction(AsObject(c), 131);
            goto LABEL_41;
        }
        // Player special class 206-208: explosion
        *(BYTE*)c = 0;
        CreateEffect(210, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                     nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
        for (int i = 0; i < 10; ++i) {
            CreateEffect(211, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                         nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
        }
        playFxBuf = true;
    } else {
        int v13 = v11 - 295;
        if (v13 == 0) {
            // Type 295: 8x (226+227)
            *(BYTE*)c = 0;
            for (int i = 0; i < 8; ++i) {
                CreateEffect(226, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                             nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
                CreateEffect(227, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                             nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
            }
            playFxBuf = true;
        } else if (v13 == 5) {
            // Type 300: 1x 210 + 10x 211
            *(BYTE*)c = 0;
            CreateEffect(210, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                         nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
            for (int i = 0; i < 10; ++i) {
                CreateEffect(211, (float*)(c + 16), (float*)(c + 28), (float*)(c + 232),
                             nullptr, nullptr, (float*)(uintptr_t)-1, nullptr, 0);
            }
            playFxBuf = true;
        } else {
            SetAction(AsObject(c), 6);
            goto LABEL_41;
        }
    }
    if (playFxBuf) PlayBuffer(94, AsObject(c), 0);

LABEL_41:
    // Death sound
    if (*(float*)&((OBJECT*)(uintptr_t)(c))->AnimationFrame == 0.0f) {
        short v14 = *(short*)(c + 2);
        if (v14 == 390) {
            int v15 = *(int*)(c + 4);
            if (v15 < 206 || v15 > 208) {
                if ((*(BYTE*)&((CHARACTER*)(uintptr_t)(c))->Class & 7) == 2) {
                    PlayBuffer(80, AsObject(c), 0);
                } else {
                    PlayBuffer(78, AsObject(c), 0);
                }
                return;
            }
        }
        // Non-player or player[206-208]: model action sound at +178
        short modSnd = *(short*)((char*)(uintptr_t)DAT_05828d58 + BMD_STRIDE * v14 + 178 + BMD_FIELD_SHIFT);
        if (modSnd != -1) {
            PlayBuffer(modSnd + 170, AsObject(c), 0);
        }
    }
}

// mc_SetPlayerDie — wrapper que usa la lógica de envejecimiento del ragdoll de MoveCharacter.
// El SetPlayerDie de IDA NO toca dead_flag (lo hace el llamador, ReceiveDie), y
// este wrapper tampoco: sólo llama a SetPlayerDie.
static inline void mc_SetPlayerDie(CHARACTER* c)
{
    if (!c) return;
    SetPlayerDie(c);
    // NO escribir acá +0x2FD (+765), ni dead_flag (0x34e), ni alive_flag (0x2EC):
    // SetPlayerDie (0x00444D90) no los toca; la transición de estado de vida es
    // sólo de ReceiveDie.
    // El bloque que llama aca (LABEL_195, IDA L796-806) usa +765 como CONTADOR:
    //     if ( *(_BYTE *)(c + 765) )
    //         if ( (std::uint8_t)++*(_BYTE *)(c + 765) >= 0xFu )  SetPlayerDie(c);
    // y +765 es el filtro de "vivo" del barrido de sub_45FEC0 (IDA L168
    // `!v16[18]`), que reporta los blancos al server con el 0x1D.  Con 0x34e=1 el
    // render (bDead = c+0x34e) trata al héroe como muerto (body light rojo).
}

void __cdecl MoveCharacter(intptr_t p1)
{
    if (!p1) return;
    intptr_t c = p1;
    intptr_t o = p1;

    // Common locals
    float WorldPosition[3];
    float Position[3];
    float TargetPosition[3];
    float Light[3];
    float p1f[3];
    float p2f[3];
    float v389[3];     // out coord buffer for skill targets
    float v399[3];     // case 9 anchor pos
    float v394[3];     // case 9 angle
    float v396[3];     // case 48 base pos
    float Angle[3];    // case 48
    float v407[3];     // bone joint pos
    float v417[3];     // c+816 chain joint angle
    float v371[3], v372[3], v373[3], v379[3];  // CreateBlur params
    float Out[3];
    float in1[3], in2[3][4];
    // La estela del arma usa el buffer GLOBAL de huesos (0x06970A9C), igual que
    // IDA: `BMD_Animation(v422, (float (*)[3][4])BoneMatrix, ...)` y despues
    // `BoneMatrix[3 * bone]`.  No usar un buffer local: los huesos que
    // BMD_Animation no escribe quedarian con basura de stack.
    float (* const BoneMatrix)[3][4] = (float (*)[3][4])g_BoneScratch;
    intptr_t   v422 = BMD_STRIDE * (AsObject(o)->Type) + (intptr_t)DAT_05828d58;  // model slot
    bool  bEventNpc = false;
    intptr_t Owner = 0;
    intptr_t v390  = 0;
    int   v392 = 0;     // ground-item flag
    int   v393 = 0;     // skill-id captured (24)
    int   v375 = 0, Type = 0, v376 = 0, v378 = 0, v377 = 0;
    float v412, v413, v414;
    int   v48;          // entType16

    // ─── IDA L431-436: sync entity world pos → model render slot
    *(DWORD *)&AsModel(v422)->BodyOrigin[0] = *(DWORD *)&AsObject(o)->Position[0];   // x
    *(DWORD *)&AsModel(v422)->BodyOrigin[1] = *(DWORD *)&AsObject(o)->Position[1];   // y
    *(DWORD *)&AsModel(v422)->BodyOrigin[2] = *(DWORD *)&AsObject(o)->Position[2];   // z
    *(DWORD *)&AsModel(v422)->BodyScale = *(DWORD *)&AsObject(o)->Scale;   // scale
    AsModel(v422)->CurrentAction = AsObject(o)->CurrentAction;  // anim_state

    // ─── IDA L437-477: Hero-only buff timers (anti-tamper stripped)
    if (AsCharacter(c) == AsCharacter(Hero))
    {
        if (CharacterAttribute)
        {
            // L449-457: attack-speed buff (CA+42 timer, CA+40 bit 0)
            unsigned short* t1 = (unsigned short*)((char*)CharacterAttribute + 42);
            if (*t1) (*t1)--;
            if (!*t1) {
                *((BYTE*)CharacterAttribute + 40) &= ~1u;
                CalculateAttackSpeed(static_cast<BYTE*>(CharacterMachine));  // CalculateAttackSpeed
            }
            // L458-467: magic-speed buff (CA+44 timer, CA+40 bit 1)
            unsigned short* t2 = (unsigned short*)((char*)CharacterAttribute + 44);
            if (*t2) (*t2)--;
            if (!*t2) {
                *((BYTE*)CharacterAttribute + 40) &= ~2u;
                Stats_CalcBase(static_cast<BYTE*>(CharacterMachine));  // Stats_CalcBase
                Stats_CalcMagicDmgRange(static_cast<BYTE*>(CharacterMachine));  // Stats_CalcMagicDmgRange
            }
        }
    }

    // ─── IDA L478-498: tag stationary event NPCs in cities (Lorencia,Dungeon)
    // Evento propio del Golden Archer (DLL MoveCharacter_GoldenArcherMovement,
    // hook en 0x44A31A): tambien en Noria, con la orientacion de Lorencia.
    if (AsObject(o)->Kind == 4
        && (!gMapManager.GetCurrentMap() || gMapManager.GetCurrentMap() == 2 || (g_GoldenArcherCustom && gMapManager.GetCurrentMap() == 3))
        && AsObject(o)->Type == 390
        && AsObject(o)->SubType >= 206 && AsObject(o)->SubType <= 208)
    {
        if (gMapManager.GetCurrentMap() == 2) {
            *(DWORD *)&AsObject(o)->Angle[0] = 0;
            *(DWORD *)&AsObject(o)->Angle[1] = 0;
            *(DWORD *)&AsObject(o)->Angle[2] = 0;
        } else {
            *(DWORD *)&AsObject(o)->Angle[0] = 0;
            *(DWORD *)&AsObject(o)->Angle[1] = 0;
            *(DWORD *)&AsObject(o)->Angle[2] = 1119092736;  // 90.0f
        }
        bEventNpc = true;
    }

    // ─── IDA L499-502: decaimiento del knockback (mece el modelo hacia arriba mientras c+772 > 0)
    if (*(BYTE*)&((CHARACTER*)(uintptr_t)(c))->StormTime) {
        unsigned char k = (unsigned char)(*(BYTE*)&((CHARACTER*)(uintptr_t)(c))->StormTime);
        (*(BYTE*)&((CHARACTER*)(uintptr_t)(c))->StormTime)--;
        AsObject(o)->Angle[2] += (float)(10 * k);
    }

    // ─── IDA L503-527: deslizamiento al objetivo (cuando c+773 está activo, interpola x/y a la grilla 774/775)
    if (AsCharacter(c)->JumpTime) {
        v414 = 0.2f;
        if (AsObject(o)->Type == 322) v414 = 0.07f;
        AsObject(o)->Position[0] += (((float)AsCharacter(c)->TargetX + 0.5f) * 100.0f - AsObject(o)->Position[0]) * v414;
        AsObject(o)->Position[1] += (((float)AsCharacter(c)->TargetY + 0.5f) * 100.0f - AsObject(o)->Position[1]) * v414;
        if (AsObject(o)->Type != 236)
            AsObject(o)->Position[2] = RequestTerrainHeight(AsObject(o)->Position[0], AsObject(o)->Position[1]);
        unsigned char step = (unsigned char)(AsCharacter(c)->JumpTime);
        (AsCharacter(c)->JumpTime)++;
        if (step > 15) {
            if (AsObject(o)->Type == 322) SetPlayerStop(AsCharacter(c));
            AsCharacter(c)->JumpTime = 0;
        }
    }

    // ─── IDA L528-544: pickup-bag bouncing (entType=236)
    if (AsObject(o)->Type == 236) {
        AsObject(o)->Position[2]  += AsObject(o)->Gravity;
        AsObject(o)->Gravity -= 6.0f;
        v413 = RequestTerrainHeight(AsObject(o)->Position[0], AsObject(o)->Position[1]) + 30.0f;
        if (AsObject(o)->Position[2] < v413) {
            AsObject(o)->Position[2]  = v413;
            AsObject(o)->Gravity = -AsObject(o)->Gravity * 0.40000001f;
        }
        AsObject(o)->Angle[0] += AsObject(o)->Direction[0];
        AsObject(o)->Angle[1] += AsObject(o)->Direction[1];
        AsObject(o)->Angle[2] += AsObject(o)->Direction[2];
        AsObject(o)->Direction[0] *= 0.80000001f;
        AsObject(o)->Direction[1] *= 0.80000001f;
        AsObject(o)->Direction[2] *= 0.80000001f;
    }

    // ─── IDA L545-606: death-dissolve (c+765=ragdoll, c+820=alpha-decay timer)
    if (AsCharacter(c)->Dead) {
        if (AsObject(o)->Type == 302 || gMapManager.GetCurrentMap() == 7)
            AsCharacter(c)->Rot += 0.050000001f;
        else
            AsCharacter(c)->Rot += 0.02f;

        v412 = 1.0f;
        if (AsCharacter(c)->Rot >= 1.0f) {
            AsObject(o)->Alpha = 1.0f - (AsCharacter(c)->Rot - v412);
            if (AsObject(o)->Alpha < 0.0099999998f) {
                if (AsCharacter(c) != AsCharacter(Hero)) *(BYTE*)o = 0;
            } else {
                AsObject(o)->Position[2] -= 0.40000001f;
            }
            DeleteCloth(AsCharacter(c), AsObject(o), nullptr);  // DeleteCloth
        }
        // L572-592: Crywolf falling-debris physics (worlds 11..16)
        if (gMapManager.GetCurrentMap() >= 11 && gMapManager.GetCurrentMap() <= 16 && *(BYTE*)&((OBJECT*)(uintptr_t)(o))->ActionStart) {
            float out_[3] = {0,0,0};
            in1[0] = 0.0f; in1[1] = AsObject(o)->Direction[1]; in1[2] = 0.0f;
            AngleMatrix((float*)(o + EntityOffset(408)), in2);
            VectorRotate(in1, (float*)in2, out_);
            AsObject(o)->Direction[1] += AsObject(o)->Direction[0];
            AsObject(o)->Gravity += AsObject(o)->Velocity;
            AsObject(o)->Velocity -= 5.0f;
            AsObject(o)->Direction[2] -= 2.0f;
            AsObject(o)->Angle[0]  -= 5.0f;
            if (AsObject(o)->Direction[2] < 1.0f) *(DWORD *)&AsObject(o)->Direction[2] = 1065353216;  // 1.0f
            AsObject(o)->Position[0] = out_[0] + *(float*)&((OBJECT*)(uintptr_t)(o))->DeadPosition[0];
            AsObject(o)->Position[1] = out_[1] + *(float*)&((OBJECT*)(uintptr_t)(o))->DeadPosition[1];
            AsObject(o)->Position[2] = *(float*)&((OBJECT*)(uintptr_t)(o))->DeadPosition[2] + AsObject(o)->Gravity;
        }
        // L593-605: Atlans bubble particles (world 7 in-game)
        if (SceneFlag == 5 && gMapManager.GetCurrentMap() == 7) {
            for (int jj = 0; jj < 4; ++jj) {
                v407[0] = (float)(rand() % 128 - 64);
                v407[1] = (float)(rand() % 128 - 64);
                v407[2] = (float)(rand() % 256);
                v407[0] += AsObject(o)->Position[0];
                v407[1] += AsObject(o)->Position[1];
                v407[2] += AsObject(o)->Position[2];
                Particle_Spawn(1241, v407, (float*)(o + 28), (float*)(o + 232), 0, 1.0f, 0);
            }
        }
    }

    // ─── IDA L607: Alpha(o)
    Alpha(AsObject(o));

    // ─── IDA L608-611: shaky timer decay
    if (*(float*)&((CHARACTER*)(uintptr_t)(c))->Freeze > 0.0f) *(float*)&((CHARACTER*)(uintptr_t)(c))->Freeze -= 0.029999999f;

    bool skipIdleSelect = false;

    // ─── IDA L612-614: tick de animación — si la acción actual continúa, salta a L195
    if (CharacterAnimation((intptr_t)c, (intptr_t)o)) {
        skipIdleSelect = true;
    }

    if (!skipIdleSelect)
    {
        // L616-618: clear chain-skill state
        AsCharacter(c)->LongRangeAttack  = -1;
        *(BYTE*)&((CHARACTER*)(uintptr_t)(c))->Foot[0] = 0;
        *(BYTE*)&((CHARACTER*)(uintptr_t)(c))->Foot[1] = 0;

        // L619-739: player (entType=390) idle animation selection
        if (AsObject(o)->Type == 390)
        {
            if (!bEventNpc)
            {
                // L623-631: hit/stun anims 131/132 → Blood + return
                if (AsObject(o)->CurrentAction == 131 || AsObject(o)->CurrentAction == 132) {
                    if (!AsCharacter(c)->Blood) { AsCharacter(c)->Blood = 1; CreateBlood(o); }
                    return;
                }
                // L632-639: walking/running/dying actions stay; otherwise stop
                BYTE act = AsObject(o)->CurrentAction;
                if (act < 0x0Du
                    || (act >= 0x22u && act <= 0x82u && act != 79 && act != 80)) {
                    SetPlayerStop(AsCharacter(c));  // SetPlayerStop
                }
                skipIdleSelect = true;  // jump to L195
            }
            else
            {
                // L642-738: event NPC idle chatter / random pose dispatch
                int action = rand() % 100;
                if (GoldenArcher_CustomNpcIdle((intptr_t)c, action)) {
                    skipIdleSelect = true;
                } else
                if (AsObject(o)->CurrentAction == 1)  // PLAYER_STOP_MALE
                {
                    int TextIndex = 0;
                    if (action >= 80) {
                        if (action >= 85) {
                            if (action >= 90) {
                                if (action >= 95) {
                                    if (action < 100) {
                                        TextIndex = (gMapManager.GetCurrentMap() == 2) ? 905 : 0;
                                        if (!(rand() % 3) && TextIndex)
                                            CreateChat((char*)EntityName::Get(c), GlobalText[TextIndex], AsCharacter(c), 0, -1);
                                        SetAction(AsObject(c), 105);
                                    }
                                } else {
                                    TextIndex = (gMapManager.GetCurrentMap() == 2) ? 905 : 0;
                                    if (!(rand() % 3) && TextIndex)
                                        CreateChat((char*)EntityName::Get(c), GlobalText[TextIndex], AsCharacter(c), 0, -1);
                                    SetAction(AsObject(c), 111);
                                }
                            } else {
                                TextIndex = (gMapManager.GetCurrentMap() == 2) ? 904 : 823;
                                if (!(rand() % 2) && TextIndex)
                                    CreateChat((char*)EntityName::Get(c), GlobalText[TextIndex], AsCharacter(c), 0, -1);
                                SetAction(AsObject(c), 99);
                            }
                        } else {
                            TextIndex = (gMapManager.GetCurrentMap() == 2) ? 904 : 0;
                            if (!(rand() % 2) && TextIndex)
                                CreateChat((char*)EntityName::Get(c), GlobalText[TextIndex], AsCharacter(c), 0, -1);
                            SetAction(AsObject(c), 97);
                        }
                        skipIdleSelect = true;
                    }
                    if (!skipIdleSelect) {
                        TextIndex = (gMapManager.GetCurrentMap() == 2) ? 904 : 0;
                        if (!(rand() % 2) && TextIndex)
                            CreateChat((char*)EntityName::Get(c), GlobalText[TextIndex], AsCharacter(c), 0, -1);
                    }
                }
                if (!skipIdleSelect) SetAction(AsObject(c), 1);
                skipIdleSelect = true;  // fall to L195 either way (event NPCs done)
            }
        }
        else if (gMapManager.GetCurrentMap() == 1 && AsObject(o)->Type == 40) {
            SetAction(AsObject(o), 0);
        }
        else if (AsObject(o)->Type < 270 || AsObject(o)->Type >= 335) {
            // Non-player non-monster (NPCs, props)
            v48 = AsObject(o)->Type;
            if (v48 == 170 || v48 <= 337 || v48 > 339) {
                SetAction(AsObject(o), rand() % 2);
            } else if (rand() % 16 >= 12) {
                SetAction(AsObject(o), rand() % 2 + 1);
            } else {
                SetAction(AsObject(o), 0);
            }
        }
        else {
            // Monster (270..334)
            if (AsObject(o)->CurrentAction == 6) {  // MONSTER01_DIE
                if (!AsCharacter(c)->Blood) { AsCharacter(c)->Blood = 1; CreateBlood(o); }
                return;
            }
            BYTE act = AsObject(o)->CurrentAction;
            if (act == 1 || act == 5 || act == 3 || act == 4 || act == 8 || act == 9) {
                SetAction(AsObject(o), 0);
            }
        }
    }

    // ─── LABEL_195 (IDA L783): mid-way joint logic ──────────────────────────
    // L784-795: planta enredadera (tipo 377) — cambio de idle aleatorio al terminar la acción
    if (AsObject(o)->Type == 377
        && AsModel(v422)->CurrentAnimationFrame
            == *(short*)((intptr_t)AsModel(v422)->Actions + sizeof(ModelAnimationAction) * (AsObject(o)->CurrentAction) + 8) - 1)
    {
        SetAction(AsObject(o), (rand() % 32) ? 0 : 1);
    }

    // L796-806: ragdoll-counter aging (force die after 0xF frames)
    if (AsCharacter(c)->Dead) {
        unsigned char inc = (unsigned char)(++AsCharacter(c)->Dead);
        if (inc >= 0xFu) mc_SetPlayerDie(AsCharacter(c));
        if (gMapManager.GetCurrentMap() >= 11 && gMapManager.GetCurrentMap() <= 16 && *(BYTE*)&((OBJECT*)(uintptr_t)(o))->ActionStart) mc_SetPlayerDie(AsCharacter(c));
    }

    // L807-811: setup defaults
    /*v424 unused*/ (void)((rand() % 6 + 2) * 0.1f);
    memset(p1f, 0, sizeof(p1f));
    Light[0] = 1.0f; Light[1] = 1.0f; Light[2] = 1.0f;

    // L812-817: attack swing tick
    if (AsCharacter(c)->SkillStage) {
        AttackStage(c, o);
        AttackEffect((intptr_t)c);         // AttackEffect @ 00445230
        ++AsCharacter(c)->SkillStage;
    }

    // L818-839: bone-particle effect (o+404 = effect counter)
    if (*(BYTE*)&((OBJECT*)(uintptr_t)(o))->HurtByOneToOne) {
        intptr_t This = BMD_STRIDE * (AsObject(o)->Type) + (intptr_t)DAT_05828d58;
        memset(p1f, 0, sizeof(p1f));
        for (intptr_t kk = 0; kk < *(short*)(This + 34); ++kk) {
            intptr_t v401 = reinterpret_cast<ModelAnimationBone*>(AsModel(This)->Bones)[kk].Parent;
            if (v401 > -1 && v401 < 200) {
                BMD_TransformPosition((void*)This, (float*)(48 * kk + (intptr_t)AsObject(o)->BoneMatrices), p1f, Position, 1);
                BMD_TransformPosition((void*)This, (float*)(48 * v401 + (intptr_t)AsObject(o)->BoneMatrices), p1f, TargetPosition, 1);
                Position[0] += (float)(rand() % 41 - 20);
                Position[1] += (float)(rand() % 41 - 20);
                Position[2] += (float)(rand() % 41 - 20);
                TargetPosition[0] += (float)(rand() % 41 - 20);
                TargetPosition[1] += (float)(rand() % 41 - 20);
                TargetPosition[2] += (float)(rand() % 41 - 20);
                Joint_Create(1254, Position, TargetPosition, (float*)(o + 28), 7, 0, 20.0f, (short)-1, 0);
            }
        }
        --*(BYTE*)&((OBJECT*)(uintptr_t)(o))->HurtByOneToOne;
    }

    // ─── L840-2486: SKILL DISPATCH (when c+757 exhaust limit, drain c+770 queue)
    if (AsCharacter(c)->SkillStage >= (unsigned char)g_iLimitAttackTime)
    {
        AsCharacter(c)->SkillStage = 0;
        AsObject(o)->PKKey = -1;
        if (AsCharacter(c) == AsCharacter(Hero) && SelectedCharacter != -1) {
            intptr_t selSlot = (intptr_t)CharactersClient + (intptr_t)CHARACTER_STRIDE * SelectedCharacter;
            // Guild-war target lock (skipped guild-mark strcmp — same guild = no target)
            if (EnableGuildWar
                && AsCharacter(selSlot)->PK >= 6u
                && AsCharacter(selSlot)->GuildMarkIndex != -1
                && Hero
                && !strcmp(Guild_GetMarkName(*(const short*)(static_cast<const char*>(Hero) + EntityOffset(474))),
                           Guild_GetMarkName(AsCharacter(selSlot)->GuildMarkIndex)))
            {
                AsObject(o)->PKKey = -1;
            }
            else if ((EnableGuildWar && AsCharacter(selSlot)->GuildTeam == 2)
                  || (!EnableGuildWar && AsCharacter(selSlot)->PK >= 6u)
                  || gUserSettings.GetPvPWithoutControl()   // DESVIACION (DLL, 0x0044B532)
                  || ((unsigned short)Platform::GetInput().KeyState(17) >> 8))
            {
                AsObject(o)->PKKey = *(short *)&AsCharacter(selSlot)->Key;
            }
        }

        // Lee el ID de skill encolado en c+770 (anti-tamper: venía envuelto en
        // ref-count + XOR; we use direct byte read since no encryption in our build).
        unsigned char v328 = AsCharacter(c)->Skill;
        if (v328 == 3 || v328 == 7) {
        }

        switch ((char)v328)
        {
        case 5: {  // FireBall
            WorldPosition[0] = ((float)*(BYTE*)&((CHARACTER*)(uintptr_t)(c))->SkillX + 0.5f) * 100.0f;
            WorldPosition[1] = ((float)*(BYTE*)&((CHARACTER*)(uintptr_t)(c))->SkillY + 0.5f) * 100.0f;
            WorldPosition[2] = RequestTerrainHeight(WorldPosition[0], WorldPosition[1]);
            int hk = FindHotKey(5);
            CreateEffect(1200, WorldPosition, (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short *)&AsObject(o)->PKKey, (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(91, 0, 0);
            break;
        }
        case 8: {  // Heal
            int hk = FindHotKey(8);
            CreateEffect(204, (float*)(o + 16), (float*)(o + 28), Light, (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short *)&AsObject(o)->PKKey, (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(86, 0, 0);
            break;
        }
        case 9: {  // Twister
            v399[0] = AsObject(o)->Position[0];
            v399[1] = AsObject(o)->Position[1];
            v399[2] = AsObject(o)->Position[2] + 100.0f;
            for (int kk = 0; kk < 4; ++kk) {
                v394[0] = 0.0f; v394[1] = 0.0f; v394[2] = (float)kk * 90.0f;
                int hk = FindHotKey(9);
                Joint_Create(1253, v399, (float*)(o + 16), v394, 0, (intptr_t)o, 80.0f,
                             AsObject(o)->PKKey, (unsigned char)hk);
                Joint_Create(1253, v399, (float*)(o + 16), v394, 0, (intptr_t)o, 20.0f, (short)-1, 0);
            }
            PlayBuffer(87, 0, 0);
            break;
        }
        case 10: {  // Defense
            int hk = FindHotKey(10);
            CreateEffect(200, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short *)&AsObject(o)->PKKey, (float*)(uintptr_t)(unsigned)hk, 0);
            CreateEffect(201, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                         (float*)(uintptr_t)-1, nullptr, 0);
            PlayBuffer(89, 0, 0);
            break;
        }
        case 12: {  // FallingSlash
            mc_AngleVectorOffset((float*)o, -20.0f, -90.0f, 100.0f, WorldPosition);
            int hk = FindHotKey(12);
            CreateEffect(1210, WorldPosition, (float*)(o + 28), Light, (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short *)&AsObject(o)->PKKey, (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(92, 0, 0);
            break;
        }
        case 13: {  // Lunge / Crescent
            WorldPosition[0] = ((float)*(BYTE*)&((CHARACTER*)(uintptr_t)(c))->SkillX + 0.5f) * 100.0f;
            WorldPosition[1] = ((float)*(BYTE*)&((CHARACTER*)(uintptr_t)(c))->SkillY + 0.5f) * 100.0f;
            WorldPosition[2] = RequestTerrainHeight(WorldPosition[0], WorldPosition[1]);
            int hk = FindHotKey(13);
            CreateEffect(240, WorldPosition, (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short *)&AsObject(o)->PKKey, (float*)(uintptr_t)(unsigned)hk, 0);
            CreateEffect(240, WorldPosition, (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short *)&AsObject(o)->PKKey, (float*)(uintptr_t)(unsigned)(hk & 0xFF), 0);
            break;
        }
        case 14: {  // Decay (bomb-ring)
            Effect_SpawnBombRing((float*)(o + 16));
            int hk = FindHotKey(14);
            CreateEffect(241, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short *)&AsObject(o)->PKKey, (float*)(uintptr_t)(unsigned)hk, 0);
            break;
        }
        case 30: case 31: case 32: case 33: case 34: case 35: case 36:  // Buff / Debuff
            CreateEffect(1264, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)3, (float*)o,
                         (float*)(uintptr_t)-1, nullptr, 0);
            break;
        case 41: {  // Bow special 1
            *(BYTE*)(o + 136) = (BYTE)(AsCharacter(c)->Weapon[0].Type + 112);
            *(BYTE*)(o + 137) = AsCharacter(c)->Weapon[0].Level;
            int hk = FindHotKey(41);
            CreateEffect(238, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short *)&AsObject(o)->PKKey, (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(85, 0, 0);
            break;
        }
        case 42: {  // Bow special 2
            *(BYTE*)(o + 136) = (BYTE)(AsCharacter(c)->Weapon[0].Type + 112);
            *(BYTE*)(o + 137) = AsCharacter(c)->Weapon[0].Level;
            int hk = FindHotKey(42);
            CreateEffect(244, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short *)&AsObject(o)->PKKey, (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(85, 0, 0);
            break;
        }
        case 48: {  // Mass-stun ring (36 vertical bolts)
            memset(Angle, 0, sizeof(Angle));
            int v398 = 36;
            for (int kk = 0; kk < v398; ++kk) {
                Angle[0] = -10.0f; Angle[1] = 0.0f; Angle[2] = (float)kk * 10.0f;
                v396[0] = AsObject(o)->Position[0];
                v396[1] = AsObject(o)->Position[1];
                v396[2] = AsObject(o)->Position[2] + 100.0f;
                Joint_Create(1253, v396, v396, Angle, 2, (intptr_t)o, 60.0f, 0, 0);
                if (!(kk % 20)) {
                    CreateEffect(1264, (float*)(o + 16), Angle, (float*)(o + 232), (float*)(uintptr_t)4, (float*)o,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                }
            }
            // L1664: seteo de flag sólo para el Hero (anti-tamper removido)
            // salteado — era una ronda de hash-encrypt sobre CharacterMachine
            // Sonido del skill: IDA L1817, justo despues del bloque de hash-encrypt
            // que se omite por policy.
            PlayBuffer(97, 0, 0);
            break;
        }
        case 49: {  // Magic — uses dword_5826D10 hotkey state
            int hk = (int)CurrentSkill;
            CreateEffect(1382, (float*)(o + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, (float*)o,
                         (float*)(uintptr_t)(unsigned)*(unsigned short *)&AsObject(o)->PKKey, (float*)(uintptr_t)(unsigned)hk, 0);
            PlayBuffer(84, 0, 0);
            break;
        }
        case 55:
            *(BYTE*)(o + 136) = (BYTE)(AsCharacter(c)->Weapon[0].Type + 112);
            *(BYTE*)(o + 137) = AsCharacter(c)->Weapon[0].Level;
            break;
        case 56:
            *(BYTE*)(o + 136) = (BYTE)(AsCharacter(c)->Weapon[0].Type + 112);
            *(BYTE*)(o + 137) = AsCharacter(c)->Weapon[0].Level;
            break;
        default: break;
        }

        // ─── L1836-2477: targeted vs. self-cast skill effects
        if (AsCharacter(c)->TargetCharacter == -1)
        {
            // Self-cast (no target). Read action via FUN_0045fae0 (skipped XOR).
            v393 = (v328 == 24) ? 1 : 0;

            // L1917-2097: Bow/Crossbow arrow shoot (player anims 46..49,54,55)
            BYTE act = AsObject(o)->CurrentAction;
            if (AsObject(o)->Type == 390
                && (act == 46 || act == 47 || act == 48 || act == 49 || act == 54 || act == 55))
            {
                unsigned char arrowSkill = AsCharacter(c)->Skill;  // direct read (anti-tamper stripped)
                int hk = FindHotKey(arrowSkill);
                CreateArrows(c, o, 0, (WORD)hk, (WORD)v393, (WORD)AsCharacter(c)->Skill);
            }
            // L2098-2101: ranged-monster auto-arrows
            if (AsObject(o)->Type == 292 || AsObject(o)->Type == 305 || AsObject(o)->Type == 310 || AsObject(o)->Type == 316)
            {
                CreateArrows(c, o, 0, 0, 1, 0);
            }
        }
        else
        {
            // Targeted skill — Owner = CharactersClient[c+784]
            v390 = (intptr_t)CharactersClient + (intptr_t)CHARACTER_STRIDE * AsCharacter(c)->TargetCharacter;
            Owner = v390;

            if (v328 == 3 || v328 == 7) {
            }

            // L2107-2197: bow/crossbow arrow path (player only)
            BYTE act = AsObject(o)->CurrentAction;
            if (AsObject(o)->Type == 390
                && (act == 46 || act == 47 || act == 48 || act == 49 || act == 54 || act == 55))
            {
                unsigned char arrowSkill = AsCharacter(c)->Skill;
                int hk = FindHotKey(arrowSkill);
                CreateArrows(c, o, Owner, (WORD)hk, 0, (WORD)AsCharacter(c)->Skill);
            }
            // L2198-2201: ranged-monster
            if (AsObject(o)->Type == 292 || AsObject(o)->Type == 305 || AsObject(o)->Type == 310)
                CreateArrows(c, o, Owner, 0, 1, 0);

            // L2202-2229: target halo + Atlans bubble shower
            if (*(short*)&((CHARACTER*)(uintptr_t)(v390))->Hit) {
                if (*(short*)(Owner + 2) != 277) {
                    for (int kk = 0; kk < 10; ++kk) {
                        WorldPosition[0] = (float)(rand() % 64 - 32) + *(float*)(Owner + 16);
                        WorldPosition[1] = (float)(rand() % 64 - 32) + *(float*)(Owner + 20);
                        WorldPosition[2] = (float)(rand() % 64 + 90) + *(float*)(Owner + 24);
                        Particle_Spawn(1206, WorldPosition, (float*)(o + 28), Light, 0, 1.0f, 0);
                    }
                }
                if (*(short*)(Owner + 2) == 330) {
                    for (int kk = 0; kk < 5; ++kk) {
                        if (!(rand() % 2)) {
                            WorldPosition[0] = *(float*)(Owner + 16);
                            WorldPosition[1] = *(float*)(Owner + 20);
                            float v45 = *(float*)(Owner + 24) + 50.0f;
                            WorldPosition[2] = (float)(rand() % 30) + v45;
                            CreateEffect(261, WorldPosition, (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                                         (float*)(uintptr_t)-1, nullptr, 0);
                        }
                    }
                    PlayBuffer(107, 0, 0);
                }
            }

            // L2230-2233: Magic anim 0x38..0x3C bonus particles
            if (AsObject(o)->CurrentAction >= 0x38u && AsObject(o)->CurrentAction <= 0x3Cu) {
                FUN_0046c5a0(0, (intptr_t)v390, (float*)(Owner + 16), (float*)(o + 28));
            }

            // L2234-2237: la tercera componente del ángulo se recalcula a partir de las
            // posiciones del caster y del objetivo. A pesar de la etiqueta vieja del decompilador,
            // esto es CreateAngle/CreateAngle (00449900_MoveCharacter.c), no
            // a movement tick.
            v389[0] = AsObject(o)->Angle[0];
            v389[1] = AsObject(o)->Angle[1];
            v389[2] = AsObject(o)->Angle[2];
            v389[2] = CreateAngle(AsObject(o)->Position[0], AsObject(o)->Position[1],
                                    *(float*)(Owner + 16), *(float*)(Owner + 20));

            // L2238-2479: targeted-skill effect dispatch
            unsigned char skillId = AsCharacter(c)->Skill;
            switch (skillId)
            {
            case 1:
                if (AsObject(o)->Type == 390)
                    CreateEffect(192, (float*)(Owner + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                Light[0] = 0.40000001f; Light[1] = 0.60000002f; Light[2] = 1.0f;
                for (int kk = 0; kk < 10; ++kk)
                    Particle_Spawn(1220, (float*)(Owner + 16), (float*)(o + 28), Light, 1, 1.0f, 0);
                if (AsCharacter(c)->SkillSuccess) *(DWORD*)(Owner + 120) |= 1u;
                PlayBuffer(34, 0, 0);
                break;
            case 2:
                CreateEffect(191, (float*)(Owner + 16), (float*)(Owner + 28), (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                             (float*)(uintptr_t)-1, nullptr, 0);
                PlayBuffer(46, 0, 0);
                break;
            case 4:
                CreateEffect(191, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)1, (float*)Owner,
                             (float*)(uintptr_t)-1, nullptr, 0);
                PlayBuffer(46, 0, 0);
                break;
            case 7:
                CreateEffect(190, (float*)(Owner + 16), (float*)(o + 28), Light, (float*)(uintptr_t)0, nullptr,
                             (float*)(uintptr_t)-1, nullptr, 0);
                for (int kk = 0; kk < 5; ++kk)
                    CreateEffect(199, (float*)(Owner + 16), (float*)(o + 28), (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                if (AsCharacter(c)->SkillSuccess && (*(DWORD*)(Owner + 120) & 2) != 2)
                    *(DWORD*)(Owner + 120) |= 2u;
                PlayBuffer(90, 0, 0);
                break;
            case 11:
                if (AsObject(o)->Type == 288) {
                    v389[2] += 10.0f;
                    CreateEffect(203, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                    v389[2] -= 20.0f;
                    CreateEffect(203, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                    v389[2] += 10.0f;
                }
                CreateEffect(203, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)0, nullptr,
                             (float*)(uintptr_t)-1, nullptr, 0);
                PlayBuffer(88, 0, 0);
                break;
            case 16:
                if (AsObject(o)->Type == 325) {
                    DWORD v30 = *(DWORD*)(o + 120);
                    *((BYTE*)&v30 + 1) |= 1u;
                    *(DWORD*)(o + 120) = v30;
                } else {
                    DWORD v31 = *(DWORD*)(Owner + 120);
                    *((BYTE*)&v31 + 1) |= 1u;
                    *(DWORD*)(Owner + 120) = v31;
                    PlayBuffer(103, 0, 0);
                    mc_DeleteJoint(266, Owner, 0);
                    for (int i = 0; i < 5; ++i) {
                        Joint_Create(266, (float*)(Owner + 16), (float*)(Owner + 16),
                                     (float*)(Owner + 28), 0, (intptr_t)Owner, 20.0f, (short)-1, 0);
                    }
                }
                break;
            case 17: {
                bool skip766 = false;
                switch (AsCharacter(c)->MonsterIndex) {
                case 0x25: case 0x2E: case 0x3D: case 0x42: case 0x45: case 0x46:
                case 0x49: case 0x4B: case 0x4D: case 0x57: case 0x59: case 0x5D:
                case 0x5F: case 0x63: case 0x70: case 0x74: case 0x76: case 0x7A:
                case 0x7C: case 0x80: case 0x82: case 0x86: case 0x88:
                    skip766 = true; break;
                default:
                    if (AsObject(o)->Type == 282) {
                        CreateEffect(212, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)0, (float*)Owner,
                                     (float*)(uintptr_t)-1, nullptr, 0);
                    } else {
                        CreateEffect(1180, (float*)(o + 16), v389, (float*)(o + 232), (float*)(uintptr_t)0, (float*)Owner,
                                     (float*)(uintptr_t)-1, nullptr, 0);
                        PlayBuffer(88, 0, 0);
                    }
                }
                if (skip766) goto LABEL_766;
                break;
            }
            case 24: {
                int hk = FindHotKey(skillId);
                CreateArrows(c, o, 0, (WORD)hk, 1, 0);
                goto LABEL_720;
            }
            case 26:
                CreateEffect(1264, (float*)(Owner + 16), (float*)(Owner + 28), (float*)(Owner + 232), (float*)(uintptr_t)1, (float*)Owner,
                             (float*)(uintptr_t)-1, nullptr, 0);
                break;
            case 27:
                if (AsCharacter(c)->SkillSuccess) {
                    CreateEffect(1264, (float*)(Owner + 16), (float*)(Owner + 28), (float*)(Owner + 232), (float*)(uintptr_t)2, (float*)Owner,
                                 (float*)(uintptr_t)-1, nullptr, 0);
                    if ((*(DWORD*)(Owner + 120) & 8) == 8) {
                        if (!mc_JointFind(266, Owner, 4)) {
                            for (int i = 0; i < 5; ++i) {
                                Joint_Create(266, (float*)(Owner + 16), (float*)(Owner + 16),
                                             (float*)(Owner + 28), 4, (intptr_t)Owner, 20.0f, (short)-1, 0);
                            }
                        }
                    } else {
                        DWORD v29 = *(DWORD*)(Owner + 120);
                        *((BYTE*)&v29 + 0) = (BYTE)(v29 | 8);
                        *(DWORD*)(Owner + 120) = v29;
                        for (int i = 0; i < 5; ++i) {
                            Joint_Create(266, (float*)(Owner + 16), (float*)(Owner + 16),
                                         (float*)(Owner + 28), 4, (intptr_t)Owner, 20.0f, (short)-1, 0);
                        }
                    }
                }
                break;
            case 28:
                CreateEffect(1264, (float*)(Owner + 16), (float*)(Owner + 28), (float*)(Owner + 232), (float*)(uintptr_t)3, (float*)Owner,
                             (float*)(uintptr_t)-1, nullptr, 0);
                if (AsCharacter(c)->SkillSuccess) *(DWORD*)(Owner + 120) |= 4u;
                break;
            case 51:
            case 52:
            LABEL_720:
                {
                    unsigned char skill2 = AsCharacter(c)->Skill;
                    int hk = FindHotKey(skill2);
                    CreateArrows(c, o, 0, (WORD)hk, 0, (WORD)skill2);
                }
                break;
            default: break;
            }

        LABEL_766:
            // L2430-2479: ambient particle + shield-up sound
            WorldPosition[0] = *(float*)(Owner + 16);
            WorldPosition[1] = *(float*)(Owner + 20);
            WorldPosition[2] = *(float*)(Owner + 24) + 120.0f;
            v392 = 0;
            if (AsObject(o)->CurrentAction == 37 || AsObject(o)->CurrentAction == 38) v392 = 1;
            Light[0] = 1.0f;
            Light[1] = (AsCharacter(v390) == AsCharacter(Hero)) ? 0.0f : 0.60000002f;
            Light[2] = 0.0f;

            if (*(BYTE*)&((CHARACTER*)(uintptr_t)(c))->AttackFlag == 2) {
                Joint_Create(1258, (float*)(Owner + 16), (float*)(Owner + 16), (float*)(o + 28),
                             0, (intptr_t)o, 20.0f, (short)-1, 0);
                Joint_Create(1258, (float*)(Owner + 16), (float*)(Owner + 16), (float*)(o + 28),
                             1, (intptr_t)o, 20.0f, (short)-1, 0);
            }

            // L2455-2478: hit-impact "thud" sound for non-buff skills
            unsigned char queuedSkill = AsCharacter(c)->Skill;
            switch (queuedSkill) {
            case 16: case 26: case 27: case 28:
            case 30: case 31: case 32: case 33: case 34: case 35: case 36:
            case 43: case 47: case 48:
                break;
            default:
                if (AsCharacter(c)->MonsterIndex < 0x44u || AsCharacter(c)->MonsterIndex > 0x4Bu) {
                    PlayBuffer(rand() % 7 + 50, AsObject(o), 0);
                }
                break;
            }
        }

        // L2481-2485: clear queue + state — hash decrypt/encrypt skipped
        AsCharacter(c)->Skill = 0;
        *(short*)&((CHARACTER*)(uintptr_t)(c))->Damage = 0;
        *(BYTE*)&((CHARACTER*)(uintptr_t)(c))->AttackFlag = 0;
    }

    // ─── L2487-2507: Skill_Throw / projectile (c+912 = throwing flag)
    if (*(BYTE*)&((CHARACTER*)(uintptr_t)(c))->_unk390) {
        AsObject(o)->BlendMesh = -2;
        AsObject(o)->BlendMeshLight += 0.1f;
        float v384f = (AsCharacter(c)->TargetPosition[0] - AsObject(o)->Position[0]) * 0.30000001f;
        float v385f = (AsCharacter(c)->TargetPosition[1] - AsObject(o)->Position[1]) * 0.30000001f;
        AsObject(o)->Position[0] += v384f;
        AsObject(o)->Position[1] += v385f;
        float v33f = RequestTerrainHeight(AsObject(o)->Position[0], AsObject(o)->Position[1]);
        AsObject(o)->Position[2] = v33f;
        if ((float)Math_Fabs(v384f) < 1.0f) {
            if ((float)Math_Fabs(v385f) < 1.0f) {
                *(BYTE*)&((CHARACTER*)(uintptr_t)(c))->_unk390 = 0;
                AsObject(o)->BlendMesh = -1;
            }
        }
    }

    // ─── L2508-2540: c+816 chained joint (Sword/spear/wand trail types 1253/1254/1260)
    memset(v417, 0, sizeof(v417));
    if (AsCharacter(c)->LongRangeAttack == 1254 || AsCharacter(c)->LongRangeAttack == 1260 || AsCharacter(c)->LongRangeAttack == 1253)
    {
        int v383i = -1036779520;  // -10.0f
        if (AsCharacter(c)->LongRangeAttack == 1253) v383i = -1028390912;  // -8.0f
        // (v380/v381/v382 unused outside debug)
        memset(p1f, 0, sizeof(p1f));
        BMD_TransformPosition((void*)v422, (float*)((intptr_t)AsObject(o)->BoneMatrices + 1584), p1f, WorldPosition, 1);
        *(int*)&v417[0] = v383i;
        v417[1] = 0.0f;
        v417[2] = AsObject(o)->Angle[2] - 60.0f;
        Joint_Create(AsCharacter(c)->LongRangeAttack, WorldPosition, (float*)(c + EntityOffset(788)), v417, 0, (intptr_t)o, 50.0f, (short)-1, 0);
        if (AsCharacter(c)->LongRangeAttack == 1254) {
            Joint_Create(AsCharacter(c)->LongRangeAttack, WorldPosition, (float*)(c + EntityOffset(788)), v417, 0, (intptr_t)o, 10.0f, (short)-1, 0);
            Joint_Create(AsCharacter(c)->LongRangeAttack, WorldPosition, (float*)(c + EntityOffset(788)), v417, 0, (intptr_t)o, 10.0f, (short)-1, 0);
        }
        BMD_TransformPosition((void*)v422, (float*)((intptr_t)AsObject(o)->BoneMatrices + 2016), p1f, WorldPosition, 1);
        *(int*)&v417[0] = v383i;
        v417[1] = 0.0f;
        v417[2] = AsObject(o)->Angle[2] + 60.0f;
        Joint_Create(AsCharacter(c)->LongRangeAttack, WorldPosition, (float*)(c + EntityOffset(788)), v417, 0, (intptr_t)o, 50.0f, (short)-1, 0);
        if (AsCharacter(c)->LongRangeAttack == 1254) {
            Joint_Create(AsCharacter(c)->LongRangeAttack, WorldPosition, (float*)(c + EntityOffset(788)), v417, 0, (intptr_t)o, 10.0f, (short)-1, 0);
            Joint_Create(AsCharacter(c)->LongRangeAttack, WorldPosition, (float*)(c + EntityOffset(788)), v417, 0, (intptr_t)o, 10.0f, (short)-1, 0);
        }
    }

    // L2541-2572: Spear stab (1256) — trail at f >= 9.0
    if (AsCharacter(c)->LongRangeAttack == 1256) {
        memset(p1f, 0, sizeof(p1f));
        BMD_TransformPosition((void*)v422, (float*)((intptr_t)AsObject(o)->BoneMatrices + 1968), p1f, WorldPosition, 0);
        memset(p1f, 0, sizeof(p1f));
        BMD_TransformPosition((void*)v422, (float*)((intptr_t)AsObject(o)->BoneMatrices + 1536), p1f, WorldPosition, 0);
        if (AsObject(o)->AnimationFrame >= 9.0f) {
            if (AsCharacter(c)->Weapon[0].Type == 560) {
                WorldPosition[0] = 0.0f; WorldPosition[1] = -130.0f; WorldPosition[2] = 0.0f;
                BMD_TransformPosition((void*)v422,
                             (float*)(48 * (AsCharacter(c)->Weapon[0].LinkBone) + (intptr_t)AsObject(o)->BoneMatrices),
                             WorldPosition, p1f, 0);
            } else {
                memset(WorldPosition, 0, sizeof(WorldPosition));
                BMD_TransformPosition((void*)v422, (float*)((intptr_t)AsObject(o)->BoneMatrices + 1968), WorldPosition, p1f, 0);
            }
            v379[0] = AsObject(o)->Angle[0];
            v379[1] = AsObject(o)->Angle[1];
            v379[2] = AsObject(o)->Angle[2];
            Joint_Create(AsCharacter(c)->LongRangeAttack, p1f, (float*)(c + EntityOffset(788)), v379, 0, (intptr_t)o, 50.0f, (short)-1, 0);
            AsCharacter(c)->LongRangeAttack = -1;
        }
    }

    // ─── L2573-2768: Weapon-blur trail (when AnimFrame >= 3.0 or anim=81)
    if (AsObject(o)->AnimationFrame >= 3.0f || AsObject(o)->CurrentAction == 81) {
        v377 = 0;
        if (AsObject(o)->Type == 390 && (AsObject(o)->CurrentAction == 37 || AsObject(o)->CurrentAction == 38)) v377 = 1;
        v375 = 0; Type = 0;
        v378 = *(short*)(c + EntityOffset(24 * v377 + 624));
        v376 = *(unsigned char*)(c + (intptr_t)sizeof(PART_t) * v377 + EntityOffset(624) + (626 - 624));

        BYTE actLocal = AsObject(o)->CurrentAction;
        if (AsObject(o)->Type == 390) {
            // Player blur dispatch by weapon ItemType + action
            if (actLocal == 57 || actLocal == 58 || actLocal == 59) {
                v375 = 1;
                Type = (v378 == 414 || v378 == 431) ? 1 : 2;
            } else if (actLocal == 60) {
                v375 = 1;
                Type = (v378 == 469) ? 1 : 2;
            } else if (v378 < 400 || v378 >= 432) {
                if (v378 == 435 || (v378 >= 437 && v378 < 496)) {
                    if (actLocal >= 0x38u && actLocal <= 0x3Cu) {
                        v375 = 1; Type = 2;
                    }
                } else if (v378 >= 496 && v378 < 528 && actLocal >= 0x2Au && actLocal <= 0x2Du) {
                    v375 = 3;
                    if (v378 == 506) { v375 = 1; Type = 0; }
                    else if (actLocal == 45) { Type = 1; }
                }
            } else if ((actLocal >= 0x23u && actLocal <= 0x29u) || actLocal == 81) {
                v375 = 1;
                if (v378 == 417) Type = 6;
                else if (actLocal == 41 || actLocal == 81) Type = 1;
            }
        }
        else if (AsCharacter(c)->MonsterIndex == 71 || AsCharacter(c)->MonsterIndex == 74) {
            if (actLocal >= 3u && actLocal <= 4u) { v375 = 1; Type = 6; }
        }
        else if (v378 >= 400 && v378 < 432 && actLocal >= 3u && actLocal <= 4u) {
            v375 = 1;
        }

        if (v375 > 0) {
            v372[0] = 0.0f;
            v372[1] = (v375 == 1) ? -20.0f : -100.0f;
            v372[2] = 0.0f;
            v371[0] = 0.0f; v371[1] = -120.0f; v371[2] = 0.0f;

            if (v378 == 413 || v378 == 470 || v378 == 505) {
                v373[0] = 1.0f; v373[1] = 0.2f; v373[2] = 0.2f;
            } else if (Type) {
                v373[0] = 1.0f; v373[1] = 1.0f; v373[2] = 1.0f;
            } else if (v376 < 7) {
                if (v376 < 5) {
                    if (v376 < 3) {
                        v373[0] = 0.80000001f; v373[1] = 0.80000001f; v373[2] = 0.80000001f;
                    } else {
                        v373[0] = 1.0f; v373[1] = 0.2f; v373[2] = 0.2f;
                    }
                } else {
                    v373[0] = 0.2f; v373[1] = 0.40000001f; v373[2] = 1.0f;
                }
            } else {
                v373[0] = 1.0f; v373[1] = 0.60000002f; v373[2] = 0.2f;
            }

            if (AsObject(o)->Type != 390 || v378 == 403 || v378 == 406 || v378 == 409 || v378 == 411 || v378 == 500) {
                BMD_TransformPosition((void*)v422,
                             (float*)(48 * (*(BYTE*)(c + (intptr_t)sizeof(PART_t) * v377 + EntityOffset(624) + (628 - 624))) + (intptr_t)AsObject(o)->BoneMatrices),
                             v372, p1f, 1);
                BMD_TransformPosition((void*)v422,
                             (float*)(48 * (*(BYTE*)(c + (intptr_t)sizeof(PART_t) * v377 + EntityOffset(624) + (628 - 624))) + (intptr_t)AsObject(o)->BoneMatrices),
                             v371, p2f, 1);
                mc_CreateBlur(c, p1f, p2f, v373, Type, 0);
            } else {
                // Weapon-trail interpolation across 10 sub-frames using BMD_Animation
                float v368f = 10.0f;
                float AnimationFrame = AsObject(o)->AnimationFrame
                                     - *(float*)((intptr_t)AsModel(v422)->Actions + sizeof(ModelAnimationAction) * (AsModel(v422)->CurrentAction) + 4);
                float PriorFrame = AsObject(o)->PriorAnimationFrame;
                float v370f = *(float*)((intptr_t)AsModel(v422)->Actions + sizeof(ModelAnimationAction) * (AsModel(v422)->CurrentAction) + 4) / 10.0f;
                for (int kk = 0; kk < (int)v368f; ++kk) {
                    // El 6to argumento de BMD_Animation es el Angle de la entidad, no un
                    // buffer de salida: con ceros los huesos se posarían con facing (0,0,0) y
                    // los dos extremos de la hoja saldrían en una orientacion fija.
                    BMD_Animation((void*)v422, reinterpret_cast<float*>(BoneMatrix), AnimationFrame,
                                 *(unsigned int*)&PriorFrame,
                                 AsObject(o)->PriorAction, (unsigned int*)(o + 28),
                                 (float*)(o + 40), 0, 1);
                    // IDA indexa `BoneMatrix[3 * bone]` sobre filas de float[4], o
                    // sea 48 bytes por hueso.  Aca el puntero es (*)[3][4], donde cada
                    // elemento YA son esos 48 bytes: multiplicar por 3 otra vez daba
                    // 144*bone y leia la matriz de otro hueso.
                    BMD_TransformPosition((void*)v422,
                                 (float*)BoneMatrix[*(BYTE*)(c + (intptr_t)sizeof(PART_t) * v377 + EntityOffset(624) + (628 - 624))],
                                 v372, p1f, 0);
                    BMD_TransformPosition((void*)v422,
                                 (float*)BoneMatrix[*(BYTE*)(c + (intptr_t)sizeof(PART_t) * v377 + EntityOffset(624) + (628 - 624))],
                                 v371, p2f, 0);
                    mc_CreateBlur(c, p1f, p2f, v373, Type, 1);
                    AnimationFrame += v370f;
                }
            }
        }
    } else {
        // L2763-2768: cache pos for blur start
        *(DWORD *)&AsObject(o)->StartPosition[0] = *(DWORD *)&AsObject(o)->Position[0];
        *(DWORD *)&AsObject(o)->StartPosition[1] = *(DWORD *)&AsObject(o)->Position[1];
        *(DWORD *)&AsObject(o)->StartPosition[2] = *(DWORD *)&AsObject(o)->Position[2];
    }

    // L2769-2772: pickup-bag splash
    if (AsObject(o)->Type == 236) {
        FUN_0046c7f0(0, AsObject(o), 0.0f, 0.0f, 0.0f);
    }
}

// MoveCharacterVisual @ 0x004520C0 — Entity_UpdateState(entity_ptr)
// Updates model position/animation from entity. When model has no actions: copies cached
// los parámetros de anim de entity+0x118..+0x12c a +0x130..+0x15c, sumando la posición de mundo de la entidad.
// IDA: MoveCharacterVisual (0x004520C0)
void __cdecl MoveCharacterVisual(intptr_t entity_ptr)
{
    if (!entity_ptr) return;

    // Model slot = DAT_05828d58 + entity_type * 0xbc
    void *model = (void *)(DAT_05828d58 + (int)AsCharacter(entity_ptr)->Object.Type * BMD_STRIDE);

    // If model has no action data: copy cached transform params to render slots
    if (*(short *)(static_cast<BYTE*>(model) + 0x26) == 0) {
        OBJECT& object = AsCharacter(entity_ptr)->Object;
        ObjectBounds_Build(object.BoundingBoxMin, object.BoundingBoxMax,
                           object.Position, object.OBB);
        return;
    }

    // Con acciones: copia la posición de mundo de la entidad a los slots de transform del modelo
    *(unsigned int *)(static_cast<BYTE*>(model) + 0x6c + BMD_FIELD_SHIFT) = *(unsigned int *)&AsCharacter(entity_ptr)->Object.Position[0];
    *(unsigned int *)(static_cast<BYTE*>(model) + 0x70 + BMD_FIELD_SHIFT) = *(unsigned int *)&AsCharacter(entity_ptr)->Object.Position[1];
    *(unsigned int *)(static_cast<BYTE*>(model) + 0x74 + BMD_FIELD_SHIFT) = *(unsigned int *)&AsCharacter(entity_ptr)->Object.Position[2];
    *(unsigned int *)(static_cast<BYTE*>(model) + 0x68 + BMD_FIELD_SHIFT) = *(unsigned int *)&AsCharacter(entity_ptr)->Object.Scale;
    *(unsigned char *)(static_cast<BYTE*>(model) + 0xa0 + BMD_FIELD_SHIFT) = AsCharacter(entity_ptr)->Object.CurrentAction;

    // SafeZone update por frame (IDA MoveCharacterVisual L614):
    //     *(BYTE*)(c + EntityOffset(846)) = (TerrainWall[Terrain_Load(x, y)] & 1) == 1;
    // +0x34E (846) es **SafeZone**, NO dead_flag (el dead real es +0x2FD, ver
    // IDA ReceiveDie L18). Lo usan la música de pueblo, el bind del arma a la
    // espalda y el gate de "no atacar en zona segura".
    //
    // Hex-Rays perdió los args de Terrain_Load acá (los slots de stack `x`/`yg`
    // los reusó el ruido de hash-table), así que usamos la posición de mundo
    // actual de la entidad → celda, que es lo que hace CreateCharacterPointer
    // en el spawn y la única lectura posible (la celda del personaje).
    {
        int gx = (int)(AsCharacter(entity_ptr)->Object.Position[0] * 0.01f);
        int gy = (int)(AsCharacter(entity_ptr)->Object.Position[1] * 0.01f);
        if (gx >= 0 && gx < 256 && gy >= 0 && gy < 256) {
            int attrIdx = Terrain_GetTileIndex(gx, gy);
            AsCharacter(entity_ptr)->SafeZone =
                (unsigned char)((((unsigned char *)DAT_0838bc70)[attrIdx] & 1) == 1);
        }
    }

    // -- switch por tipo de entidad (IDA MoveCharacterVisual L764) --------
    // Efectos ambientales por NPC/monstruo.  Los **31** cases del
    // binario estan portados (0x10E 0x110 0x111 0x113 0x114 0x119 0x11A 0x11B
    // 0x11D 0x122 0x128 0x129 0x12B 0x12E 0x12F 0x133 0x135 0x137 0x138 0x139
    // 0x13A 0x13B 0x13E 0x141 0x142 0x145 0x152 0x157 0x15C 0x179 0x186).
    //
    // Lo unico del cuerpo que NO se porto es el bloque `if (c[836] > 0)` de
    // IDA L713-754 (los tipos de aura 1251 y 1252): **es codigo muerto en el
    // binario**.  Las dos ramas calculan una posicion (`v162` en la 1251, y el
    // `WorldPosition` de TransformPosition en la 1252) y despues no la usan --
    // no hay ningun Particle_Spawn ni CreateEffect detras, y L759 hace memset
    // de WorldPosition acto seguido.  Portarlo seria copiar un no-op.
    //
    // Luminosidad con flicker, común a todo el switch (IDA L761-763):
    //     v63 = (rand() % 8 + 2) * 0.1     → 0.2 .. 0.9
    {
        // IDA L756-763 — estado COMPARTIDO por todos los cases del switch, no
        // local a cada uno. Varios cases usan `Light` y `WorldPosition` sin
        // reinicializarlos, dando por sentados estos valores:
        //     Light[0..2] = 1.0
        //     memset(WorldPosition, 0, sizeof(WorldPosition))
        //     v63 = Scalec = (rand() % 8 + 2) * 0.1
        // (Sin esto no se pueden portar 0x119, 0x12B y varios más: los llaman
        //  a `Particle_Spawn(..., Light, ...)` con el (1,1,1) de acá.)
        // 004520C0 L617-679: estado visual común, previo al switch por tipo.
        // Maneja el seguimiento con la cabeza, su giro suave y el pico temporal en
        // c+848; nada de esto es lógica de combate.
        if (*(unsigned short *)&AsCharacter(entity_ptr)->Object.Type != 390) {
            MoveHead(AsCharacter(entity_ptr));
        }
        if (entity_ptr != (intptr_t)DAT_07abf5d8 &&
            !AsCharacter(entity_ptr)->Dead && !(rand() % 32)) {
            float *headTarget = (float *)(entity_ptr + 52);
            headTarget[0] = (float)(rand() % 128 - 64);
            headTarget[1] = (float)(rand() % 32 - 16);
            for (int i = 0; i < 2; ++i) {
                if (headTarget[i] < 0.0f) headTarget[i] += 360.0f;
            }
        }
        for (int i = 0; i < 2; ++i) {
            float *headCurrent = (float *)(entity_ptr + 40 + i * sizeof(float));
            const float delta = FarAngle(*headCurrent, headCurrent[3], 1) * 0.2f;
            *headCurrent = TurnAngle2(*headCurrent, headCurrent[3], delta);
        }
        if (*(unsigned char *)&((CHARACTER*)(uintptr_t)(entity_ptr))->Appear) {
            --*(unsigned char *)&((CHARACTER*)(uintptr_t)(entity_ptr))->Appear;
            float *light = (float *)(entity_ptr + 232);
            for (int i = 0; i < 20; ++i) {
                light[0] = light[1] = light[2] = 1.0f;
                float position[3] = {
                    (float)(rand() % 64 - 32) + AsCharacter(entity_ptr)->Object.Position[0],
                    (float)(rand() % 64 - 32) + AsCharacter(entity_ptr)->Object.Position[1],
                    (float)(rand() % 32 - 16) + AsCharacter(entity_ptr)->Object.Position[2]
                };
                if (!(rand() % 10))
                    Particle_Spawn(1221, position, (float *)(entity_ptr + 28), light, 1, 1.0f, 0);
                if (!(rand() % 10))
                    CreateEffect(rand() % 2 + 197, (float *)(entity_ptr + 16),
                                 (float *)(entity_ptr + 28), light, nullptr, nullptr,
                                 (float *)(uintptr_t)0xffffffff, nullptr, 0);
            }
        }

        extern void __cdecl AddTerrainLight(float x, float y, float* light, int range, float* buffer);
        extern int __cdecl FloatingLabel_Add(intptr_t a1, DWORD *a2, int a3, DWORD *a4, int a5);
        // 004520C0 L697-725: equipped-item terrain light.  The two item
        // los registros arrancan en c+624 y c+648 (stride de 24 bytes).
        if (AsCharacter(entity_ptr)->PK < 6) {
            for (int itemOffset = 624; itemOffset <= 648; itemOffset += 24) {
                const short itemType = *(short *)(entity_ptr + EntityOffset(itemOffset));
                if (itemType == 412) {
                    float itemLight[3] = {0.80000001f, 0.64000005f, 0.40000001f};
                    AddTerrainLight(AsCharacter(entity_ptr)->Object.Position[0], AsCharacter(entity_ptr)->Object.Position[1], (float*)itemLight, 3, (float*)PrimaryTerrainLight[0]);
                } else if (itemType == 419 || itemType == 546 || itemType == 570) {
                    float itemLight[3] = {0.64000005f, 0.40000001f, 0.24000001f};
                    AddTerrainLight(AsCharacter(entity_ptr)->Object.Position[0], AsCharacter(entity_ptr)->Object.Position[1], (float*)itemLight, 2, (float*)PrimaryTerrainLight[0]);
                }
            }
        }
        float Light[3] = { 1.0f, 1.0f, 1.0f };
        float WorldPosition[3] = { 0.0f, 0.0f, 0.0f };
        float Luminosity = (float)(rand() % 8 + 2) * 0.1f;   // v63 / Scalec
        short entType = AsCharacter(entity_ptr)->Object.Type;
        (void)Light; (void)WorldPosition;

        switch (entType)
        {
        // 0x10E — IDA raw L766-796.
        case 0x10E:
        {
            const unsigned char curAction = AsCharacter(entity_ptr)->Object.CurrentAction;
            const float animFrame = AsCharacter(entity_ptr)->Object.AnimationFrame;
            const bool sparkWindow =
                (!curAction && animFrame >= 15.0f && animFrame <= 20.0f) ||
                (curAction == 1 && animFrame >= 20.0f && animFrame <= 25.0f) ||
                (curAction == 2 && ((animFrame >= 2.0f && animFrame <= 3.0f) ||
                                    (animFrame >= 5.0f && animFrame <= 6.0f)));
            if (sparkWindow && !(rand() & 1)) {
                float origin[3], local[3] = { 0.0f, -4.0f, 0.0f };
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 1152), local, origin, '\x01');
                Particle_Spawn(1220, origin, (float *)(entity_ptr + 28), (float *)(entity_ptr + 232), 0, 1.0f, 0);
            }
            break;
        }

        // 0x110 / 0x111 / 0x114 / 0x122 — IDA raw L797-847.
        case 0x110:
            if (AsCharacter(entity_ptr)->Object.CurrentAction == 3 && AsCharacter(entity_ptr)->Object.AnimationFrame <= 4.0f) {
                float p[3], local[3] = {0.0f, (float)(rand() % 32 + 32), 0.0f};
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 336), local, p, '\x01');
                Particle_Spawn(1195, p, (float *)(entity_ptr + 28), Light, 1, 1.0f, 0);
            }
            // IDA fall-through to LABEL_253.
        case 0x111:
        case 0x114:
        case 0x122:
            if (entType == 0x122) {
                float p[3];
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 336), WorldPosition, p, '\x01');
                float spriteLight[3] = {Luminosity, Luminosity * 0.40000001f, Luminosity * 0.2f};
                CreateSprite(1150, p, 1.0f, spriteLight, entity_ptr, 0.0f, 0);
            }
            if (!AsCharacter(entity_ptr)->Dead && !(rand() & 3)) {
                float p[3] = {(float)(rand() % 64 - 32) + AsCharacter(entity_ptr)->Object.Position[0],
                              (float)(rand() % 64 - 32) + AsCharacter(entity_ptr)->Object.Position[1],
                              (float)(rand() % 32 - 16) + AsCharacter(entity_ptr)->Object.Position[2]};
                Particle_Spawn(gMapManager.GetCurrentMap() == 2 ? 1220 : 1221, p, (float *)(entity_ptr + 28),
                              gMapManager.GetCurrentMap() == 2 ? (float *)(entity_ptr + 232) : Light, 0, 1.0f, 0);
            }
            break;

        // 0x113 — IDA raw L848-850.
        case 0x113:
            Combat_SpawnDeathDustParticles(AsObject(entity_ptr));
            break;

        // 0x119 — IDA raw L851-873. Usa el `Light` (1,1,1) del frame compartido.
        // Gate `*(WORD*)(c + EntityOffset(446)) == 2`: sólo cuando la entidad está en ese
        // sub-estado escupe las 10 partículas de fuego. El hueso es ALEATORIO
        // (`48 * (rand() % NumBones)`), o sea las partículas salen de todo el
        // cuerpo, no de un punto fijo. `v2 + 34` es NumBones del modelo (+0x22).
        case 0x119:
        {
            AsCharacter(entity_ptr)->Object.BlendMeshLight = (float)(rand() % 10) * 0.1f;
            if (AsCharacter(entity_ptr)->Level == 2)
            {
                short numBones = *(short *)((char *)model + 0x22);
                if (numBones > 0)
                {
                    float origin[3];
                    for (int i = 0; i < 10; ++i)
                    {
                        BMD_TransformPosition(model,
                                     (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * (rand() % numBones)),
                                     WorldPosition, origin, '\x01');
                        Particle_Spawn(1195, origin, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
                    }
                }
                float L2[3] = { Luminosity, Luminosity * 0.2f, 0.0f };
                AddTerrainLight(AsCharacter(entity_ptr)->Object.Position[0], AsCharacter(entity_ptr)->Object.Position[1], (float*)L2, 2, (float*)PrimaryTerrainLight[0]);
            }
            break;
        }

        // 0x11A / 0x11B — IDA raw L874-891.
        case 0x11A:
        case 0x11B:
            if (!(rand() & 3)) {
                float local[3] = { 0.0f, 0.0f, 0.0f }, origin[3];
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 1056), local, origin, '\x01');
                Particle_Spawn(1220, origin, (float *)(entity_ptr + 28), (float *)(entity_ptr + 232), 0, 1.0f, 0);
            }
            break;

        // 0x11D / 0x128 / 0x129 — IDA raw L892-900.
        case 0x11D: AsCharacter(entity_ptr)->Object.BlendMeshTexCoordV = (float)((long long)WorldTime % 2000) * -0.00050000002f; break;
        case 0x128: AsCharacter(entity_ptr)->Object.BlendMeshTexCoordU = (float)((long long)WorldTime % 10000) * -0.000099999997f; break;
        case 0x129: AsCharacter(entity_ptr)->Object.BlendMeshTexCoordV = (float)((long long)WorldTime % 1000) * -0.001f; break;

        // 0x12B — IDA raw L901-915.
        case 0x12B:
            AsCharacter(entity_ptr)->Object.BlendMesh = 3;
            AsCharacter(entity_ptr)->Object.BlendMeshTexCoordV = (float)((long long)WorldTime % 1000) * -0.001f;
            if (!(rand() & 1)) {
                float p[3];
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 96), WorldPosition, p, '\x01');
                Particle_Spawn(1195, p, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
            }
            break;

        // 0x12E — IDA raw L916-971.
        case 0x12E:
        {
            float local[3] = { 0.0f, 0.0f, 0.0f }, origin[3];
            float light[3] = { 0.60000002f, 1.0f, 0.80000001f };
            const unsigned char action = AsCharacter(entity_ptr)->Object.CurrentAction;
            int bone = action == 3 ? 33 : action == 4 ? 20 : action == 8 ? 41 : action == 9 ? 49 : -1;
            if (bone >= 0) {
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * bone), local, origin, '\x01');
                Particle_Spawn(1180, origin, (float *)(entity_ptr + 28), light, 0, 1.0f, 0);
                float whitePink[3] = { 1.0f, 0.60000002f, 1.0f };
                Particle_Spawn(1195, origin, (float *)(entity_ptr + 28), whitePink, 0, 1.0f, 0);
            }
            if (action == 6 && AsCharacter(entity_ptr)->Object.AnimationFrame < 12.0f) {
                float green[3] = { 0.1f, 0.80000001f, 0.60000002f };
                for (int i = 0; i < 20; ++i) {
                    const int randomBone = rand() % *(short *)(static_cast<BYTE*>(model) + 34);
                    BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * randomBone), local, origin, '\x01');
                    Particle_Spawn(1195, origin, (float *)(entity_ptr + 28), green, 0, 1.0f, 0);
                }
            }
            break;
        }

        // 0x12F — IDA raw L972-990.
        case 0x12F:
            if (!AsCharacter(entity_ptr)->Dead && AsCharacter(entity_ptr)->Level == 1 && !(rand() & 3)) {
                float p[3] = {(float)(rand() % 64 - 32) + AsCharacter(entity_ptr)->Object.Position[0],
                              (float)(rand() % 64 - 32) + AsCharacter(entity_ptr)->Object.Position[1],
                              (float)(rand() % 32 - 16) + AsCharacter(entity_ptr)->Object.Position[2]};
                Particle_Spawn(1221, p, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
            }
            break;

        // 0x133 — IDA raw L991-1007.
        case 0x133:
            if (AsCharacter(entity_ptr)->Object.CurrentAction >= 3 && AsCharacter(entity_ptr)->Object.CurrentAction <= 4)
                AsCharacter(entity_ptr)->Object.BlendMeshLight = (AsCharacter(entity_ptr)->Object.BlendMeshLight + 0.1f > 1.0f) ? 1.0f : AsCharacter(entity_ptr)->Object.BlendMeshLight + 0.1f;
            else
                AsCharacter(entity_ptr)->Object.BlendMeshLight = (AsCharacter(entity_ptr)->Object.BlendMeshLight - 0.1f < 0.0f) ? 0.0f : AsCharacter(entity_ptr)->Object.BlendMeshLight - 0.1f;
            break;

        // 0x135 — IDA raw L1008-1010: sub_451EA0(o,v2,28,27).
        case 0x135:
        {
            float zero[3] = {0.0f, 0.0f, 0.0f};
            BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * 28), zero, (float *)(entity_ptr + 76), '\x01');
            BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * 27), zero, (float *)(entity_ptr + 64), '\x01');
            break;
        }
        // 0x137 — IDA raw L1011-1015.
        case 0x137:
        {
            float zero[3] = {0.0f, 0.0f, 0.0f};
            BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * 8), zero, (float *)(entity_ptr + 76), '\x01');
            BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * 9), zero, (float *)(entity_ptr + 64), '\x01');
            Combat_SpawnIdleAmbientParticle(AsObject(entity_ptr)); Combat_SpawnDeathDustParticles(AsObject(entity_ptr));
            break;
        }
        // 0x138 — IDA raw L1016-1034.
        case 0x138:
        {
            float zero[3] = {0.0f, 0.0f, 0.0f};
            BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * 24), zero, (float *)(entity_ptr + 76), '\x01');
            BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * 25), zero, (float *)(entity_ptr + 64), '\x01');
            if (AsCharacter(entity_ptr)->Object.SubType == 1) {
                float p[3];
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 288), WorldPosition, p, '\x01');
                Particle_Spawn(1195, p, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 624), WorldPosition, p, '\x01');
                Particle_Spawn(1195, p, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
                float darkness[3] = {-1.3f,-1.3f,-1.3f};
                AddTerrainLight(AsCharacter(entity_ptr)->Object.Position[0], AsCharacter(entity_ptr)->Object.Position[1], (float*)darkness, 3, (float*)PrimaryTerrainLight[0]);
            } else { Combat_SpawnIdleAmbientParticle(AsObject(entity_ptr)); Combat_SpawnDeathDustParticles(AsObject(entity_ptr)); }
            break;
        }
        // 0x139 / 0x13B — IDA raw L1035-1038 / L1114-1117.
        case 0x139:
        case 0x13B:
        {
            const int a = entType == 0x139 ? 11 : 8, b = entType == 0x139 ? 12 : 9;
            float zero[3] = {0.0f, 0.0f, 0.0f};
            BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * a), zero, (float *)(entity_ptr + 76), '\x01');
            BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * b), zero, (float *)(entity_ptr + 64), '\x01');
            Combat_SpawnIdleAmbientParticle(AsObject(entity_ptr));
            break;
        }
        // 0x13A — IDA raw L1039-1113.  Las tablas se leyeron de IDA en
        // 0x5597D8 (118 bytes: las últimas lecturas alcanzan 0x55984B).
        // Hex-Rays separa los aliases v169/v170/v171,
        // pero están en el mismo bloque contiguo: bones[0], bones[1], bones[30].
        case 0x13A:
        {
            static const unsigned char boneOrder[35] = {
                5, 6, 33, 53, 35, 49, 50, 45, 46, 41, 42, 37, 38, 11, 31, 13,
                27, 28, 23, 24, 19, 20, 15, 16, 54, 55, 62, 69, 70, 77, 2, 79,
                81, 84, 86
            };
            static const unsigned char linksA[32] = {
                0, 2, 2, 3, 2, 4, 4, 5, 5, 6, 4, 7, 7, 8, 4, 9,
                9, 10, 4, 11, 11, 12, 6, 5, 8, 7, 10, 9, 12, 11, 0, 0
            };
            static const unsigned char linksB[32] = {
                0, 13, 13, 14, 13, 15, 15, 16, 16, 17, 17, 16, 15, 18, 18, 19,
                15, 20, 20, 21, 15, 22, 22, 23, 15, 20, 20, 21, 18, 19, 21, 22
            };
            static const unsigned char linksC[8] = {29, 28, 28, 27, 34, 33, 33, 30};
            static const unsigned char linksD[8] = {26, 25, 25, 24, 32, 31, 31, 30};

            float zero[3] = {0.0f, 0.0f, 0.0f};
            BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * 8), zero,
                          (float *)(entity_ptr + 76), '\x01');
            BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * 9), zero,
                          (float *)(entity_ptr + 64), '\x01');

            if (AsCharacter(entity_ptr)->MonsterIndex == 63) {
                float targetPosition[3] = {0.0f, 0.0f, 0.0f};
                float bones[35][3] = {};

                for (int bone = 0; bone < 35; ++bone) {
                    BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * boneOrder[bone]),
                                  zero, bones[bone], '\x01');
                }

                for (int link = 0; link < 30; link += 2) {
                    float particleLight[3];
                    MoveHumming(bones[linksA[link + 1]], targetPosition, bones[linksA[link]], 360.0f);
                    particleLight[0] = Luminosity; // IDA sólo escribe este componente (Position[0] = v63).
                    Particle_Spawn(1200, bones[linksA[link]], targetPosition, particleLight, 2,
                                  link < 22 ? 1.0f : 0.5f, 0);
                    MoveHumming(bones[linksB[link + 1]], targetPosition, bones[linksB[link]], 360.0f);
                    particleLight[0] = Luminosity;
                    Particle_Spawn(1200, bones[linksB[link]], targetPosition, particleLight, 2,
                                  link < 22 ? 1.0f : 0.5f, 0);
                }

                for (int link = 0; link < 8; link += 2) {
                    float particleLight[3];
                    MoveHumming(bones[linksC[link + 1]], targetPosition, bones[linksC[link]], 360.0f);
                    particleLight[0] = Luminosity;
                    Particle_Spawn(1200, bones[linksC[link]], targetPosition, particleLight, 2, 0.60000002f, 0);
                    MoveHumming(bones[linksD[link + 1]], targetPosition, bones[linksD[link]], 360.0f);
                    particleLight[0] = Luminosity;
                    Particle_Spawn(1200, bones[linksD[link]], targetPosition, particleLight, 2, 0.60000002f, 0);
                }

                if (!((long long)WorldTime % 2)) {
                    float particleLight[3];
                    MoveHumming(bones[0], targetPosition, bones[30], 360.0f);
                    particleLight[0] = (float)WorldTime;
                    Particle_Spawn(1200, bones[30], targetPosition, particleLight, 2, 1.3f, 0);
                    Particle_Spawn(1200, bones[1], targetPosition, particleLight, 3, 0.5f, 0);
                }

                float darkness[3] = {-1.3f, -1.3f, -1.3f};
                AddTerrainLight(AsCharacter(entity_ptr)->Object.Position[0], AsCharacter(entity_ptr)->Object.Position[1], (float*)darkness, 3, (float*)PrimaryTerrainLight[0]);
            } else {
                float targetPosition[3] = {0.0f, 0.0f, 0.0f};
                float effectLight = sinf((float)WorldTime * 0.0020000001f) * 0.30000001f + 0.69999999f;
                float light[3] = {effectLight, effectLight * 0.5f, effectLight * 0.5f};
                float source[3], destination[3];

                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 2640), WorldPosition, source, '\x01');
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 2976), WorldPosition, destination, '\x01');
                MoveHumming(source, targetPosition, destination, 360.0f);
                Particle_Spawn(1200, destination, targetPosition, light, 1, 0.2f, 0);

                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 3360), WorldPosition, source, '\x01');
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 3696), WorldPosition, destination, '\x01');
                MoveHumming(source, targetPosition, destination, 360.0f);
                Particle_Spawn(1200, destination, targetPosition, light, 1, 0.2f, 0);
                Combat_SpawnIdleAmbientParticle(AsObject(entity_ptr));
                Combat_SpawnDeathDustParticles(AsObject(entity_ptr));
            }
            break;
        }
        // 0x13E — IDA raw L1118-1128; sub_4793F0 == FloatingLabel_Add (verificado en IDA).
        case 0x13E:
            if (!(rand() % 5)) {
                float p[3] = {(float)(rand() % 21 - 10) * 3.6571429f + AsCharacter(entity_ptr)->Object.Position[0],
                              (float)(rand() % 21 - 10) * 3.6571429f + AsCharacter(entity_ptr)->Object.Position[1], 0.0f};
                FloatingLabel_Add(1205, (DWORD *)p, *(int *)&AsCharacter(entity_ptr)->Object.Angle[0], (DWORD *)(entity_ptr + 232), 0x3F266666);
            }
            break;

        // 0x141 — IDA raw L1129-1178.
        case 0x141:
        {
            float local[3] = { 0.0f, 0.0f, 0.0f }, a[3], b[3];
            const intptr_t boneBase = (intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices;
            for (int offset = 96; offset < 240; offset += 48) {
                BMD_TransformPosition(model, (float *)(boneBase + offset), local, a, '\x01');
                BMD_TransformPosition(model, (float *)(boneBase + offset + 48), local, b, '\x01');
                Joint_Create(1254, a, b, (float *)(entity_ptr + 28), 7, 0, 14.0f, -1, 0);
            }
            // IDA's j=9 special case reuses bone 2 (+96) and targets
            // 16*(3*j+3) = +480, i.e. bone 10.  Using bone 9 joined the
            // lightning chain back into the wrong segment and made it appear
            // to bounce/reverse at the first branch.
            const int pairs[][2] = {{2,10},{10,11},{2,18},{18,22},{22,23},{23,24},{24,25},{18,31},{31,32},{32,33},{33,34}};
            for (int i = 0; i < 11; ++i) {
                BMD_TransformPosition(model, (float *)(boneBase + 48 * pairs[i][0]), local, a, '\x01');
                BMD_TransformPosition(model, (float *)(boneBase + 48 * pairs[i][1]), local, b, '\x01');
                Joint_Create(1254, a, b, (float *)(entity_ptr + 28), 7, 0, 14.0f, -1, 0);
            }
            break;
        }

        // 0x142 / 0x145 — IDA raw L1179-1184.
        case 0x142: AsCharacter(entity_ptr)->Object.BlendMeshTexCoordU = (float)((long long)WorldTime % 10000) * -0.00039999999f; break;
        case 0x145: AsCharacter(entity_ptr)->Object.BlendMeshTexCoordV = (float)((long long)WorldTime % 10000) * 0.000099999997f; break;

        // 0x152 — MODEL_SMITH / herrero (OpenNpc case 338), IDA raw L1185-1221.
        case 0x152:
        {

            unsigned char curAction = AsCharacter(entity_ptr)->Object.CurrentAction;  // +0x105
            float         animFrame = AsCharacter(entity_ptr)->Object.AnimationFrame;          // +0x108

            // Golpe de martillo: suena entre los frames 5 y 10 de la anim 0.
            // El buffer 120 (0x78) lo carga OpenNpc justo para este NPC.
            if (curAction == 0 && animFrame >= 5.0f && animFrame <= 10.0f)
                PlayBuffer(120, 0, 0);

            // BlendMesh = 4 → la mesh 4 de Smith01.bmd es `fire_02.jpg`, el
            // fuego de la fragua: se dibuja en modo blend en vez de opaca.
            // BlendMeshLight le da el parpadeo.
            AsCharacter(entity_ptr)->Object.BlendMesh = 4;            // +0x64 BlendMesh
            AsCharacter(entity_ptr)->Object.BlendMeshLight = Luminosity;   // +0x68 BlendMeshLight

            // Luz naranja proyectada sobre el terreno alrededor de la fragua.
            float Light[3] = { Luminosity, Luminosity * 0.40000001f, 0.0f };
            AddTerrainLight(AsCharacter(entity_ptr)->Object.Position[0], AsCharacter(entity_ptr)->Object.Position[1], (float*)Light, 3, (float*)PrimaryTerrainLight[0]);

            // Chispas: 4 por golpe, en la ventana de frames 5..6 (el impacto).
            // El origen es el hueso 17 (BoneTransform + 48*17 = +816) = el yunque.
            Light[0] = 1.0f; Light[1] = 1.0f; Light[2] = 1.0f;
            float WorldPosition[3] = { 0.0f, 0.0f, 0.0f };
            if (curAction == 0 && animFrame >= 5.0f && animFrame <= 6.0f)
            {
                float sparkOrigin[3];
                BMD_TransformPosition(model,
                             (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 816),
                             WorldPosition, sparkOrigin, '\x01');
                for (int i = 0; i < 4; ++i)
                {
                    float Angle[3];
                    Angle[0] = (float)(rand() % 60 + 150);
                    Angle[1] = 0.0f;
                    Angle[2] = (float)(rand() % 30);
                    Joint_Create(1257, sparkOrigin, sparkOrigin, Angle, 0, 0, 10.0f, -1, 0);
                    Particle_Spawn(1175, sparkOrigin, Angle, Light, 0, 1.0f, 0);
                }
            }
            break;
        }

        // 0x157 / 0x15C — IDA raw L1222-1245.
        case 0x157:
            if (!(rand() & 0xFF)) PlayBuffer(120, 0, 0);
            break;
        case 0x15C:
            if (!(rand() & 0x3F)) PlayBuffer(121, 0, 0);
            break;

        // 0x179 — IDA raw L1246-1281.
        case 0x179:
            if (!AsCharacter(entity_ptr)->Object.CurrentAction) {
                float white[3] = { 1.0f, 1.0f, 1.0f };
                float local[3] = { 0.0f, 5.0f, 10.0f }, origin[3];
                float terrainLight[3] = { Luminosity * 0.5f, Luminosity * 0.30000001f, 0.0f };
                AddTerrainLight(AsCharacter(entity_ptr)->Object.Position[0], AsCharacter(entity_ptr)->Object.Position[1], (float*)terrainLight, 3, (float*)PrimaryTerrainLight[0]);
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 1776), local, origin, '\x01');
                for (int i = 0; i < 4; ++i) {
                    float angle[3] = { (float)(rand() % 60 + 90), 0.0f, (float)(rand() % 30) };
                    Joint_Create(1257, origin, origin, angle, 0, 0, 10.0f, -1, 0);
                    if (rand() & 1)
                        Particle_Spawn(1175, origin, angle, white, 0, 1.0f, 0);
                }
            }
            break;

        // 0x186 — IDA raw L1282-1342.
        case 0x186:
        {
            float p[3];
            if (SceneFlag == 5 && gMapManager.GetCurrentMap() == 7 && (long long)WorldTime % 10000 < 1000) {
                float local[3] = {0.0f,20.0f,-10.0f};
                BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * *(int *)(static_cast<BYTE*>(model) + 84)), local, p, '\x01');
                Particle_Spawn(1241, p, (float *)(entity_ptr + 28), Light, 0, 1.0f, 0);
            }
            float local[3] = {-15.0f,0.0f,0.0f};
            if (gMapManager.GetCurrentMap() == 9) {
                if (!(rand() & 3)) { BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 1248), local, p, '\x01'); Particle_Spawn(104,p,(float *)(entity_ptr+28),Light,0,1.0f,0); }
                if (!(rand() & 3)) { BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 1680), local, p, '\x01'); Particle_Spawn(104,p,(float *)(entity_ptr+28),Light,0,1.0f,0); }
            }
            if (AsCharacter(entity_ptr)->Object.CurrentAction == 90) {
                const short n = *(short *)(static_cast<BYTE*>(model) + 34);
                if (n > 0) for (int i = 0; i < 10; ++i) {
                    BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * (rand() % n)), local, p, '\x01');
                    Particle_Spawn(1195,p,(float *)(entity_ptr+28),Light,0,1.0f,0);
                }
            }
            float tail[3] = {0.0f,-30.0f,0.0f};
            BMD_TransformPosition(model, (float *)((intptr_t)AsCharacter(entity_ptr)->Object.BoneMatrices + 48 * AsCharacter(entity_ptr)->Weapon[0].LinkBone), tail, p, '\x01');
            break;
        }

        default:
            break;
        }
    }
    // 004520C0 L1383-1396: los pasos del jugador local se eligen según las dos
    // animation-frame thresholds, once per walk/run cycle.
    if (*(unsigned short *)&AsCharacter(entity_ptr)->Object.Type == 390 &&
        entity_ptr == (intptr_t)DAT_07abf5d8) {
        const unsigned char action = AsCharacter(entity_ptr)->Object.CurrentAction;
        if ((action >= 13 && action <= 33) || action == 79 || action == 80) {
            const float frame = AsCharacter(entity_ptr)->Object.AnimationFrame;
            extern char Sound_PlayFootstep();
            if (frame >= 1.5f && !*(unsigned char *)&((CHARACTER*)(uintptr_t)(entity_ptr))->Foot[0]) {
                *(unsigned char *)&((CHARACTER*)(uintptr_t)(entity_ptr))->Foot[0] = 1;
                Sound_PlayFootstep();
            }
            if (frame >= 4.5f && !*(unsigned char *)&((CHARACTER*)(uintptr_t)(entity_ptr))->Foot[1]) {
                *(unsigned char *)&((CHARACTER*)(uintptr_t)(entity_ptr))->Foot[1] = 1;
                Sound_PlayFootstep();
            }
        }
    }
    // (Full animation bone update: HashTable obfuscation blocks — skipped)
}

// MoveMonsterClient @ 0x00454CD0 — Entity_PathTick(entity, player_entity)
// Tick de camino por frame: si +0x2fd (teleport) está seteado → saltea. Si +0x2ec (flag de movimiento) está seteado →
//   avanza el waypoint (Entity_AdvancePath), y al llegar limpia +0x2ec y cancela la acción.
// Si no se está moviendo y la grilla objetivo difiere de PositionX/Y → llama a PathFinding2.
// Los dos argumentos conservan las direcciones del personaje y su objeto.
// IDA: MoveMonsterClient (0x00454CD0)
void __cdecl MoveMonsterClient(CHARACTER* character, OBJECT* object)
{
    BYTE* entity = reinterpret_cast<BYTE*>(character);
    BYTE* param_2 = reinterpret_cast<BYTE*>(object);

    // Sale si esto es el centinela de la entidad del jugador
    if (character == AsCharacter(DAT_07abf5d8)) return;

    // 00454CD0: el flag de teleport/altura tiene una única rama de actualización visual
    // antes de suprimir el walker normal de camino. El tipo 272 se mantiene sobre
    // la superficie del terreno mientras ese flag está activo.
    if (*(char *)&AsCharacter(entity)->Dead != '\0') {
        if (*(short*)(param_2 + 2) == 272) {
            *(float*)(param_2 + 24) = RequestTerrainHeight(
                *(float*)(param_2 + 16), *(float*)(param_2 + 20));
        }
        return;
    }

    if (*(char *)&AsCharacter(entity)->Movement != '\0') {
        // Entity is currently moving — advance one step
        SetPlayerWalk(AsCharacter(entity));
        unsigned int arrived = Entity_AdvancePath((void*)entity, '\x01');
        if ((char)arrived != '\0') {
            // Arrived at waypoint — stop
            AsCharacter(entity)->Movement = 0;
            SetPlayerStop(AsCharacter(entity));
            // Actualiza el facing según el byte de dirección del camino en +0x2fc
            AsCharacter(entity)->Object.Angle[2] =
                ((float)*(unsigned char*)&((CHARACTER*)(uintptr_t)(entity))->TargetAngle - _DAT_0055256c) * _DAT_00552844;
        }
        MoveCharacterPosition(AsCharacter(entity));
        return;
    }

    // No se está moviendo: chequea si hay que pathfindear para llegar a target_grid
    if (*(char*)&((CHARACTER*)(uintptr_t)(entity))->Appear != '\0') return;
    if (*(short*)(param_2 + 2) == 0x115) return;

    // Lee PositionX y PositionY (las decoraciones de HashTable son ruido anti-tamper)
    unsigned int cachedX = *(unsigned int *)&AsCharacter(entity)->PositionX;
    unsigned int cachedY = *(unsigned int *)&AsCharacter(entity)->PositionY;

    // Si ya está en el objetivo, no hay nada que hacer
    if (cachedX == AsCharacter(entity)->TargetX &&
        cachedY == AsCharacter(entity)->TargetY)
        return;

    // Pathfind desde PositionX/Y hasta target_grid
    unsigned int found = Path_FindRoute(cachedX, (float)cachedY,
                                      (unsigned int)AsCharacter(entity)->TargetX,
                                      (unsigned int)AsCharacter(entity)->TargetY,
                                      (unsigned char*)(entity + EntityOffset(0x354)), 0.0f);
    if ((char)found != '\0') {
        AsCharacter(entity)->Movement = 1;
    }
}
