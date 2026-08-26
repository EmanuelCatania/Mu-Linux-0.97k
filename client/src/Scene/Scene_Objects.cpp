#include "stdafx.h"
#include "../globals.h"
#include "../functions.h"

// Map_LoadObjectModels — port alternativo de 0x0050c4d0
// La versión activa (linkeada) vive en stubs.cpp:8626. Este port tiene strings
// distintos (ej. "SeaCreature" vs "SummonMonster") y quedó fuera del build.
// Se deja como documentación / referencia.
#if 0
void Map_LoadObjectModels(void)
{
    return;  // DISABLED: port alternativo, no usado
    char  cVar2;
    int   iVar3;
    int   iVar7;
    char  local_384[32];
    char  local_364[100];
    char  local_300[256];
    char  local_200[256];
    char  local_100[256];

    cVar2 = DAT_0055a7c4;
    if (DAT_083a410c != '\0') {
        DAT_0055a7c4 = '\0';
        DAT_0055a7ac = 7;
    }

    // Global drop texture always loaded
    FUN_00529740("Object8/drop01.jpg", 0x4d9, 0x2600, 0x2900, 0, '\x01');

    if (DAT_0055a7c4 != '\0') goto section2;

    // ── Section 1: per-zone animated creature BMD loads ─────────────────────
    switch (DAT_0055a7ac) {
    case 0: // Lorencia
        FUN_00505e90(0xae, "Data2/Object1/Animal/", "bird.smd");
        FUN_00505e90(0xb5, "Data2/Object1/Animal/", "fish.smd");
        break;
    case 1:
    case 4: // Dungeon
        FUN_00505e90(0xd7, "Data2/Object2/", "DungeonStone.smd" /* DAT_0055f6ac */);
        FUN_00505e90(0xb0, "Data2/Object2/", "Bat.smd");
        FUN_00505e90(0xb1, "Data2/Object2/", "mouse.smd");
        break;
    case 3: // Noria
        FUN_00505e90(0xaf, "Data2/Object1/Animal/", "butterfly.smd");
        break;
    case 5: // LostTower
        FUN_00505e90(0xe4, "Data2/Object6/", "Meteo1.smd"    /* DAT_0055f644 */);
        FUN_00505e90(0xe5, "Data2/Object6/", "Meteo2.smd"    /* DAT_0055f628 */);
        FUN_00505e90(0xe6, "Data2/Object6/", "Meteo3.smd"    /* DAT_0055f61c */);
        FUN_00505e90(0xe7, "Data2/Object6/", "Meteo4.smd"    /* DAT_0055f60c */);
        FUN_00505e90(0xe8, "Data2/Object6/", "Meteo5.smd"    /* DAT_0055f5fc */);
        FUN_00505e90(0xea, "Data2/Monster/", "BossHead.smd"  /* DAT_0055f5c4 */);
        FUN_00505e90(0xeb, "Data2/Object6/", "Princess.smd"  /* DAT_0055f590 */);
        break;
    case 6: // Atlans
        FUN_00505e90(0xb2, "Data2/Object7/", "SeaCreature.smd" /* DAT_0055f560 */);
        *(BYTE *)(*(int *)(DAT_05828d58 + 0x82e8) + 0x1a) = 1;
        break;
    case 7: // Icarus
        FUN_00505e90(0xb6, "Data2/Object8/", "IcarusWing1.smd" /* DAT_0055f534 */);
        FUN_00505e90(0xb7, "Data2/Object8/", "IcarusWing2.smd" /* DAT_0055f508 */);
        FUN_00505e90(0xb8, "Data2/Object8/", "IcarusWing3.smd" /* DAT_0055f4ec */);
        FUN_00505e90(0xb9, "Data2/Object8/", "IcarusWing4.smd" /* DAT_0055f4d0 */);
        FUN_00505e90(0xba, "Data2/Object8/", "IcarusWing5.smd" /* DAT_0055f4b0 */);
        FUN_00505e90(0xbb, "Data2/Object8/", "IcarusWing6.smd" /* DAT_0055f490 */);
        FUN_00505e90(0xbc, "Data2/Object8/", "IcarusWing7.smd" /* DAT_0055f470 */);
        FUN_00505e90(0xbd, "Data2/Object8/", "IcarusWing8.smd" /* DAT_0055f458 */);
        // Mark Icarus objects as billboard-capable (type range 0xbc..0x69b step 0xbc)
        iVar7 = 0xbc;
        do {
            iVar3 = iVar7 + 0xbc;
            *(BYTE *)(*(int *)(iVar7 + 0x851c + DAT_05828d58) + 10) = 1;
            iVar7 = iVar3;
        } while (iVar3 < 0x69c);
        break;
    case 8: // Barracks
        FUN_00505e90(0xb3, "Data2/Object9/", "Barracks1.smd" /* DAT_0055f434 */);
        break;
    case 10: // Clouds
        FUN_00529740("Effect/clouds.jpg", 0x4f4, 0x2601, 0x2900, 0, '\x01');
        FUN_00505e90(0xb6, "Data2/Object11/", "cloud.smd");
        FUN_005060b0(0xb6, "Data/Object11/", "cloud", -1);
        FUN_00505c80(0xb6, "Object11/", 0x2600, '\x01');
        FUN_00529740("Effect/cloudLight.jpg", 0x4f5, 0x2601, 0x2900, 0, '\x01');
        break;
    case 0xb:
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
    case 0x10: // BloodCastle
        FUN_00505e90(0xb8, "Data2/Object12/", "BC_Gate.smd" /* DAT_0055f388 */);
        break;
    }

section2:
    // ── Section 2: per-zone animation + texture loads ────────────────────────
    FUN_00505bd0(0x69);

    switch (DAT_0055a7ac) {
    case 0: // Lorencia
        FUN_005060b0(0xae, "Data/Object1/", "bird"   /* DAT_0055f370 */, 1);
        FUN_00505c80(0xae, "Object1/", 0x2600, '\x01');
        FUN_005060b0(0xb5, "Data/Object1/", "fish"   /* DAT_0055f34c */, 1);
        FUN_00505c80(0xb5, "Object1/", 0x2600, '\x01');
        break;
    case 1:
    case 4: // Dungeon
        FUN_005060b0(0xd7, "Data/Object2/", "DungeonStone", 1);
        FUN_00505c80(0xd7, "Object2/", 0x2600, '\x01');
        FUN_005060b0(0xb0, "Data/Object2/", "Bat"   /* PTR_DAT_0055f31c */, 1);
        FUN_00505c80(0xb0, "Object2/", 0x2600, '\x01');
        FUN_005060b0(0xb1, "Data/Object2/", "mouse" /* PTR_DAT_0055f318 */, 1);
        FUN_00505c80(0xb1, "Object2/", 0x2600, '\x01');
        break;
    case 3: // Noria
        FUN_005060b0(0xaf, "Data/Object1/", "Butterfly", 1);
        FUN_00505c80(0xaf, "Object1/", 0x2600, '\x01');
        break;
    case 5: // LostTower — Meteo1..5, BossHead, Princess
        iVar7 = 0xe4;
        do {
            FUN_005060b0(iVar7, "Data/Object6/", "Meteo", iVar7 + -0xe3);
            iVar3 = iVar7 + -0xe3;
            iVar7 = iVar7 + 1;
        } while (iVar3 < 5);
        FUN_005060b0(0xea, "Data/Object6/", "BossHead", 1);
        FUN_005060b0(0xeb, "Data/Object6/", "Princess", 1);
        iVar7 = 0xe4;
        do {
            FUN_00505c80(iVar7, "Object6/", 0x2600, '\x01');
            iVar7 = iVar7 + 1;
        } while (iVar7 < 0xec);
        break;
    case 6: // Atlans
        FUN_005060b0(0xb2, "Data/Object7/", "SeaCreature" /* PTR_DAT_0055f2cc */, 1);
        FUN_00505c80(0xb2, "Object7/", 0x2600, '\x01');
        break;
    case 7: // Icarus — animated wings + water textures
        iVar7 = 0xb6;
        do {
            FUN_005060b0(iVar7, "Data/Object8/", "fish" /* DAT_0055f34c */, iVar7 + -0xb4);
            FUN_00505c80(iVar7, "Object8/", 0x2600, '\x01');
            iVar3 = iVar7 + -0xb4;
            iVar7 = iVar7 + 1;
        } while (iVar3 < 9);
        // Load water tile textures into DAT_083a8ad8 array (32 entries × 0x38 stride).
        //
        // BUG-FIX 2026-05-03: bounds were absolute source-binary addresses
        // (`0x83a8d08`, `0x83a91d8`). DAT_083a8ad8 is now sized as `char[32*0x38]`
        // (was 1 byte → 32 strings of 0x38 bytes each = 1792 bytes of heap stomp
        // every time Icarus map loaded). Use indexed access; the first 10 entries
        // (offset 0..9 = bytes 0..560 = `< 0x230 from start`) get "wt0_%d.jpg"
        // (matching IDA literal 0x83a8d08 - 0x83a8ad8 = 0x230 = 10 × 0x38).
        for (int texIdx = 0; texIdx < 32; ++texIdx) {
            char* pEntry = &DAT_083a8ad8[texIdx * 0x38];
            const char* fmtPath = (texIdx < 10) ? "Object8/wt0_%d.jpg" : "Object8/wt_%d.jpg";
            const char* fmtName = (texIdx < 10) ? "wt0_%d.jpg"         : "wt_%d.jpg";
            crt_sprintf(local_364, fmtPath);
            FUN_00529740(local_364, texIdx + 0x41, 0x2601, 0x2901, 0, '\0');
            crt_sprintf(local_364, fmtName);
            // strcpy local_364 into pEntry (string + null)
            char* pDst = pEntry;
            const char* pSrc = local_364;
            while ((*pDst++ = *pSrc++) != '\0') { /* copy */ }
        }
        break;
    case 8: // Barracks — sand + impact textures
        FUN_00529740("Object9/sand01.jpg",    0x494, 0x2601, 0x2901, 0, '\x01');
        FUN_00529740("Object9/sand02.jpg",    0x495, 0x2601, 0x2901, 0, '\x01');
        FUN_00529740("Object9/Impack03.jpg",  0x597, 0x2601, 0x2900, 0, '\x01');
        FUN_005060b0(0xb3, "Data/Object9/", "Barracks" /* PTR_DAT_0055f2cc */, 2);
        FUN_00505c80(0xb3, "Object9/", 0x2600, '\x01');
        break;
    case 10: // Clouds
        FUN_00529740("Effect/clouds.jpg",     0x4f4, 0x2601, 0x2900, 0, '\x01');
        FUN_005060b0(0xb6, "Data/Object11/", "cloud", -1);
        FUN_00505c80(0xb6, "Object11/", 0x2600, '\x01');
        FUN_00529740("Effect/cloudLight.jpg", 0x4f5, 0x2601, 0x2900, 0, '\x01');
        break;
    case 0xb:
    case 0xc:
    case 0xd:
    case 0xe:
    case 0xf:
    case 0x10: // BloodCastle
        if (DAT_0055a7c4 == '\0') {
            FUN_00505e90(0xb8,  "Data2/Object12/", "BC_Gate.smd"    /* DAT_0055f388 */);
            FUN_00505e90(0x106, "Data2/Object12/", "BC_Statue1.smd" /* DAT_0055f1e8 */);
            FUN_00505e90(0x107, "Data2/Object12/", "BC_Statue2.smd" /* DAT_0055f1d8 */);
            FUN_00505e90(0x104, "Data2/Object12/", "BC_Coffin1.smd" /* DAT_0055f1c8 */);
            FUN_00505e90(0x105, "Data2/Object12/", "BC_Coffin2.smd" /* DAT_0055f1b8 */);
            FUN_00505e90(0xb9,  "Data2/Object12/", "BC_Shine.smd"   /* DAT_0055f1ac */);
        }
        FUN_005060b0(0xb8,  "Data/Object12/", "BC_Gate"    /* DAT_0055f1a4 */, 1);
        FUN_00505c80(0xb8,  "Object12/", 0x2600, '\x01');
        FUN_005060b0(0x106, "Data/Object12/", "BC_Statue"  /* DAT_0055f180 */, 1);
        FUN_005060b0(0x107, "Data/Object12/", "BC_Statue"  /* DAT_0055f180 */, 2);
        FUN_005060b0(0x104, "Data/Object12/", "StoneCoffin", 1);
        FUN_005060b0(0x105, "Data/Object12/", "StoneCoffin", 2);
        iVar7 = 0;
        do {
            FUN_00505c80(iVar7 + 0x106, "Monster/", 0x2600, '\x01');
            iVar7 = iVar7 + 1;
        } while (iVar7 < 2);
        iVar7 = 0;
        do {
            FUN_00505c80(iVar7 + 0x104, "Monster/", 0x2600, '\x01');
            iVar7 = iVar7 + 1;
        } while (iVar7 < 2);
        FUN_005060b0(0xb9,  "Data/Object12/", "Shine", 1);
        FUN_00505c80(0xb9,  "Object12/", 0x2600, '\x01');
        FUN_00529740("Effect/clouds.jpg", 0x4f4, 0x2601, 0x2900, 0, '\x01');
        FUN_00404a10(0x6e, "Data/Sound/iBloodCastle.wav", 1, '\0');
        DAT_0055a7c4 = '\x01';
        break;
    }

    // ── Section 3: main object set (Lorencia ~100 BMDs or custom map file) ──
    FUN_00505bd0(0x2ee);

    if (DAT_0055a7ac == 0) {
        // Lorencia: load all object models if not yet cached
        if (DAT_0055a7c4 == '\0') {
            // Trees
            FUN_00505e90(0x00, "Data2/Object1/", "treesmall.smd");
            FUN_00505e90(0x01, "Data2/Object1/", "treebig.smd");
            FUN_00505e90(0x02, "Data2/Object1/", "treea_01.smd");
            FUN_00505e90(0x03, "Data2/Object1/", "treea_02.smd");
            FUN_00505e90(0x04, "Data2/Object1/", "treea_03.smd");
            FUN_00505e90(0x05, "Data2/Object1/", "treea_04.smd");
            FUN_00505e90(0x06, "Data2/Object1/", "treea_05.smd");
            FUN_00505e90(0x07, "Data2/Object1/", "treea_06.smd");
            FUN_00505e90(0x08, "Data2/Object1/", "treea_07.smd");
            FUN_00505e90(0x09, "Data2/Object1/", "treea_08.smd");
            FUN_00505e90(0x0a, "Data2/Object1/", "treea_09.smd");
            FUN_00505e90(0x0b, "Data2/Object1/", "treea_10.smd");
            FUN_00505e90(0x0c, "Data2/Object1/", "treea_11.smd");
            // Grass
            FUN_00505e90(0x14, "Data2/Object1/", "grass_01.smd");
            FUN_00505e90(0x15, "Data2/Object1/", "grass_02.smd");
            FUN_00505e90(0x16, "Data2/Object1/", "grass_03.smd");
            FUN_00505e90(0x17, "Data2/Object1/", "grass_04.smd");
            FUN_00505e90(0x18, "Data2/Object1/", "grass_05.smd");
            FUN_00505e90(0x19, "Data2/Object1/", "grass_06.smd");
            // Mushrooms
            FUN_00505e90(0x1e, "Data2/Object1/", "mushroom_01.smd");
            FUN_00505e90(0x1f, "Data2/Object1/", "mushroom_02.smd");
            // Stones
            FUN_00505e90(0x28, "Data2/Object1/", "Ston_01.smd");
            FUN_00505e90(0x29, "Data2/Object1/", "Ston_02.smd");
            FUN_00505e90(0x2a, "Data2/Object1/", "Ston_03.smd");
            FUN_00505e90(0x2b, "Data2/Object1/", "Ston_04.smd");
            FUN_00505e90(0x2c, "Data2/Object1/", "Ston_05.smd");
            // Statues / grave markers
            FUN_00505e90(0x32, "Data2/Object1/", "stone_statue01.smd");
            FUN_00505e90(0x33, "Data2/Object1/", "stone_statue02.smd");
            FUN_00505e90(0x34, "Data2/Object1/", "Angel_Stone.smd");
            // Gates / doors
            FUN_00505e90(0x35, "Data2/Object1/", "steel_barred_door_side.smd");
            // Tomb decorations
            FUN_00505e90(0x36, "Data2/Object1/", "Tomb_arc.smd");
            FUN_00505e90(0x37, "Data2/Object1/", "Tomb_cross.smd");
            FUN_00505e90(0x38, "Data2/Object1/", "TombStone.smd");
            // Lights / fire
            FUN_00505e90(0x3c, "Data2/Object1/", "fire_light.smd");
            FUN_00505e90(0x3d, "Data2/Object1/", "Fire_Light_01.smd");
            FUN_00505e90(0x3e, "Data2/Object1/", "Fire.smd");
            // Dungeon gate
            FUN_00505e90(0x3f, "Data2/Object1/", "dungeon_gate_01.smd");
            // Misc props
            FUN_00505e90(0x46, "Data2/Object1/", "Drum.smd");
            FUN_00505e90(0x47, "Data2/Object1/", "Treasure_Chest.smd");
            FUN_00505e90(0x48, "Data2/Object1/", "ship.smd");
            // Walls
            FUN_00505e90(0x50, "Data2/Object1/", "wall_01.smd");
            FUN_00505e90(0x51, "Data2/Object1/", "wall_02.smd");
            FUN_00505e90(0x52, "Data2/Object1/", "wall_03.smd");
            FUN_00505e90(0x53, "Data2/Object1/", "wall_04.smd");
            FUN_00505e90(0x54, "Data2/Object1/", "wall_05.smd");
            FUN_00505e90(0x55, "Data2/Object1/", "wall_06.smd");
            FUN_00505e90(0x56, "Data2/Object1/", "c_wall01.smd");
            FUN_00505e90(0x57, "Data2/Object1/", "c_wall02.smd");
            FUN_00505e90(0x58, "Data2/Object1/", "c_wall03.smd");
            FUN_00505e90(0x59, "Data2/Object1/", "c_wall04.smd");
            FUN_00505e90(0x5a, "Data2/Object1/", "c_wall05.smd");
            FUN_00505e90(0x5b, "Data2/Object1/", "c_wall06.smd");
            FUN_00505e90(0x5c, "Data2/Object1/", "c_wall07.smd");
            FUN_00505e90(0x5d, "Data2/Object1/", "steel_barred_02.smd");
            FUN_00505e90(0x5e, "Data2/Object1/", "steel_barred_03.smd");
            FUN_00505e90(0x5f, "Data2/Object1/", "steel_barred_01.smd");
            FUN_00505e90(0x60, "Data2/Object1/", "steel_barred_door.smd");
            // Cannons / guns
            FUN_00505e90(0x64, "Data2/Object1/", "gun_01.smd");
            FUN_00505e90(0x65, "Data2/Object1/", "gun_02.smd");
            FUN_00505e90(0x66, "Data2/Object1/", "gun_03.smd");
            // Bridges / fences
            FUN_00505e90(0x67, "Data2/Object1/", "bridge_01.smd");
            FUN_00505e90(0x68, "Data2/Object1/", "fence.smd");
            FUN_00505e90(0x69, "Data2/Object1/", "joint_01.smd");
            FUN_00505e90(0x6a, "Data2/Object1/", "joint_02.smd");
            FUN_00505e90(0x6b, "Data2/Object1/", "joint_03.smd");
            FUN_00505e90(0x6c, "Data2/Object1/", "bridge_02.smd");
            // Town decorations
            FUN_00505e90(0x6d, "Data2/Object1/", "StreetLight.smd");
            FUN_00505e90(0x6e, "Data2/Object1/", "badge_01.smd");
            FUN_00505e90(0x6f, "Data2/Object1/", "horse_drawn.smd");
            FUN_00505e90(0x70, "Data2/Object1/", "carriage_01.smd");
            FUN_00505e90(0x71, "Data2/Object1/", "carriage_02.smd");
            FUN_00505e90(0x72, "Data2/Object1/", "carriage_03.smd");
            FUN_00505e90(0x73, "Data2/Object1/", "rice_01.smd");
            FUN_00505e90(0x74, "Data2/Object1/", "rice_02.smd");
            FUN_00505e90(0x75, "Data2/Object1/", "signboard_up.smd");
            FUN_00505e90(0x76, "Data2/Object1/", "signboard_down.smd");
            FUN_00505e90(0x77, "Data2/Object1/", "monster_a.smd");
            FUN_00505e90(0x78, "Data2/Object1/", "monster_b.smd");
            FUN_00505e90(0x79, "Data2/Object1/", "waterspout.smd");
            // Jars
            FUN_00505e90(0x7a, "Data2/Object1/", "jar_01.smd");
            FUN_00505e90(0x7b, "Data2/Object1/", "jar_02.smd");
            FUN_00505e90(0x7c, "Data2/Object1/", "jar_03.smd");
            FUN_00505e90(0x7d, "Data2/Object1/", "jar_04.smd");
            // Execution ground
            FUN_00505e90(0x7e, "Data2/Object1/", "execution_ground_01.smd");
            // Buildings
            FUN_00505e90(0x7f, "Data2/Object1/", "house_stone_01.smd");
            FUN_00505e90(0x80, "Data2/Object1/", "house_stone_02.smd");
            FUN_00505e90(0x81, "Data2/Object1/", "house_smith.smd");
            FUN_00505e90(0x82, "Data2/Object1/", "house_science.smd");
            FUN_00505e90(0x83, "Data2/Object1/", "house_mill.smd");
            FUN_00505e90(0x84, "Data2/Object1/", "tent_01.smd");
            FUN_00505e90(0x85, "Data2/Object1/", "stairs.smd");
            // House joints (structural connectors)
            FUN_00505e90(0x86, "Data2/Object1/", "house_joint_01.smd");
            FUN_00505e90(0x87, "Data2/Object1/", "house_joint_02.smd");
            FUN_00505e90(0x88, "Data2/Object1/", "house_joint_03.smd");
            FUN_00505e90(0x89, "Data2/Object1/", "house_joint_04.smd");
            FUN_00505e90(0x8a, "Data2/Object1/", "house_joint_05.smd");
            FUN_00505e90(0x8b, "Data2/Object1/", "house_joint_06.smd");
            FUN_00505e90(0x8c, "Data2/Object1/", "c_wall07.smd");
            FUN_00505e90(0x8d, "Data2/Object1/", "house.smd");
            FUN_00505e90(0x8e, "Data2/Object1/", "cage.smd");
            // Effects
            FUN_00505e90(0x8f, "Data2/Object1/", "effect.smd");
            FUN_00505e90(0x90, "Data2/Object1/", "effect.smd");
            FUN_00505e90(0x91, "Data2/Object1/", "effect.smd");
            FUN_00505e90(0x92, "Data2/Object1/", "pose_box.smd");
            // House interiors
            FUN_00505e90(0x93, "Data2/Object1/", "house_in_01.smd");
            FUN_00505e90(0x94, "Data2/Object1/", "house_in_02.smd");
            FUN_00505e90(0x95, "Data2/Object1/", "house_in_03.smd");
            FUN_00505e90(0x96, "Data2/Object1/", "house_in_04.smd");
            FUN_00505e90(0x97, "Data2/Object1/", "house_in_05.smd");
            FUN_00505e90(0x98, "Data2/Object1/", "house_in_06.smd");
            FUN_00505e90(0x99, "Data2/Object1/", "house_in_07.smd");
            // Candle
            FUN_00505e90(0x9a, "Data2/Object1/", "candle.smd");
            // Beer barrels / tavern props
            FUN_00505e90(0x9b, "Data2/Object1/", "house_in_beer_01.smd");
            FUN_00505e90(0x9c, "Data2/Object1/", "house_in_beer_02.smd");
            FUN_00505e90(0x9d, "Data2/Object1/", "house_in_beer_03.smd");

            DAT_083a4100 = 1;
        }

        // Animation + texture setup for Lorencia objects
        // Trees 0x00..0x0c → anim "Tree" index 1..13
        iVar7 = 0;
        do {
            iVar3 = iVar7 + 1;
            FUN_005060b0(iVar7, "Data/Object1/", "Tree" /* DAT_0055e8a8 */, iVar3);
            iVar7 = iVar3;
        } while (iVar3 < 0xd);
        // Grass 0x14..0x19 → anim "Grass" index 1..6
        iVar7 = 0x14;
        do {
            FUN_005060b0(iVar7, "Data/Object1/", "Grass", iVar7 - 0x13);
            iVar7++;
        } while (iVar7 < 0x1a);
        // Textures for all Lorencia objects 0x00..0x9f
        iVar7 = 0;
        do {
            FUN_00505c80(iVar7, "Object1/", 0x2600, '\x01');
            iVar7 = iVar7 + 1;
        } while (iVar7 < 0xa0);
    } else {
        // Custom / unknown map — load models from a data file
        if (DAT_0055a7c4 == '\0') {
            crt_sprintf(local_384, "%d" /* DAT_0055e714 format: zone index path */);
            ParserFileHandle = (FILE*)FUN_0054173f(local_384, DAT_005580ac);
            if (ParserFileHandle != NULL) {
                while (true) {
                    int typeId = ParseNextToken();
                    long long lv = (longlong)ParserTokenNumber;   // IDA: (__int64)TokenNumber  (ParserTokenNumber)
                    int puVar10 = (int)lv;
                    if (typeId == 2) break;
                    ParseNextToken();
                    // local_300 ← ParserTokenString (filename token)
                    memcpy(local_300, &ParserTokenString, sizeof(local_300));
                    ParseNextToken();
                    memcpy(local_200, &ParserTokenString, sizeof(local_200));
                    ParseNextToken();
                    memcpy(local_100, &ParserTokenString, sizeof(local_100));
                    crt_sprintf(local_384, "Data2/Object/%d/");
                    // If local_200 is empty, load model with just local_300
                    FUN_00505e90(puVar10, local_384, local_300);
                }
                FUN_0054150f(ParserFileHandle);
            }
        }
        // Animations for custom map objects 0x00..0x9f
        crt_sprintf(local_384, "Data/Object/%d/");
        iVar7 = 0;
        do {
            iVar3 = iVar7 + 1;
            FUN_005060b0(iVar7, local_384, "Object", iVar3);
            iVar7 = iVar3;
        } while (iVar3 < 0xa0);
        FUN_00505bd0(0x2ee);
        // Texture prefix for custom map objects
        crt_sprintf(local_384, "Object/%d/");
        iVar7 = 0;
        do {
            FUN_00505c80(iVar7, local_384, 0x2600, '\x01');
            iVar7 = iVar7 + 1;
        } while (iVar7 < 0xa0);
        // Zone-specific post-load fixups
        if (DAT_0055a7ac == 1) {
            // Dungeon: set a float property on model slot 0x76 (offset 0x1d90 into table)
            *(float *)(*(int *)(DAT_05828d58 + 0x1d90) + 0x14) = 0.4f; // 0x3ecccccd
        } else if (DAT_0055a7ac == 8) {
            // Barracks: clear animated flags on several model slots
            *(BYTE *)(DAT_05828d58 + 0x89c)  = 0;
            *(BYTE *)(DAT_05828d58 + 0x958)  = 0;
            *(BYTE *)(DAT_05828d58 + 0xa14)  = 0;
            *(BYTE *)(DAT_05828d58 + 0x3624) = 0;
            *(BYTE *)(DAT_05828d58 + 0x379c) = 0;
            *(BYTE *)(DAT_05828d58 + 0x3a8c) = 0;
        }
    }

    if (DAT_083a410c != '\0') {
        DAT_0055a7c4 = cVar2;
    }
}
#endif // disabled alternate Map_LoadObjectModels port
