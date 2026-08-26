// NPC_ModelLoad.cpp
//
// Extracted from stubs_game.cpp.  Owns runtime NPC model/texture loading.
// The original IDA symbol/address remains in the function comment.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
// OpenNpc @ 0x005091D0 (~153 lines) — loads NPC model + textures
// Switch on Type: 0x14f=Female, 0x150=Male, 0x151=Girl, 0x152=Smith (+sound),
// 0x153=Wizard, 0x154..0x15d various merchants/masters,
// 0x176+=special NPCs (Sevina, BloodCastle) via OpenModel.
void __cdecl OpenNpc_stub(int Type) {
    // 0x005091D0 — Load NPC model + textures by Type
    // Models = DAT_05828d58, each entry is a BMD struct.
    // DAT_0055a7c4 = g_tex_ext_mode (0=plain, 1=compressed OZJ/OZT)
    // If g_tex_ext_mode!=0 and model already has bones (Data+0x26 > 0), skip loading.

    DWORD* pModels = (DWORD*)DAT_05828d58;
    // Check if already loaded: DAT_0055a7c4 != 0 and *(short*)(Models[Type].Data + 0x26) > 0
    // Models[Type].Data is at pModels + Type * BMD_STRIDE — but the actual struct size is
    // unknown here. The Ghidra code references Models[Type].Data at a pointer field.
    // For simplicity, we follow the Ghidra decompile pattern with the guard check.
    // The guard check is: if (DAT_0055a7c4 != 0 && bone_count > 0) return;
    // We skip this check since we can't reliably compute BMD struct offset without BMD_STRIDE.
    // The original binary does this check before the switch.
    char* model = (char*)DAT_05828d58 + 0xBC * Type;
    if (DAT_0055a7c4 != 0 && *(short*)(model + 0x26) > 0)
        return;

    switch (Type) {
    case 0x14f:
        AccessModel(0x14f, "Data\\Npc\\", "Female", 1);
        for (int i = 1; i <= 3; i++) {
            AccessModel(i + 0x15d, "Data\\Npc\\", "FemaleHead", i);
            AccessModel(i + 0x15f, "Data\\Npc\\", "FemaleUpper", i);
            AccessModel(i + 0x161, "Data\\Npc\\", "FemaleLower", i);
            AccessModel(i + 0x165, "Data\\Npc\\", "FemaleBoots", i);
            OpenTexture(i + 0x15d, (void*)"Data\\Npc\\", 0x2600, true);
            OpenTexture(i + 0x15f, (void*)"Data\\Npc\\", 0x2600, true);
            OpenTexture(i + 0x161, (void*)"Data\\Npc\\", 0x2600, true);
            OpenTexture(i + 0x165, (void*)"Data\\Npc\\", 0x2600, true);
        }
        break;
    case 0x150:
        AccessModel(0x150, "Data\\Npc\\", "Man", 1);
        for (int i = 1; i <= 3; i++) {
            AccessModel(i + 0x167, "Data\\Npc\\", "ManHead", i);
            AccessModel(i + 0x169, "Data\\Npc\\", "ManUpper", i);
            AccessModel(i + 0x16b, "Data\\Npc\\", "ManGloves", i);
            AccessModel(i + 0x16d, "Data\\Npc\\", "ManBoots", i);
            OpenTexture(i + 0x167, (void*)"Data\\Npc\\", 0x2600, true);
            OpenTexture(i + 0x169, (void*)"Data\\Npc\\", 0x2600, true);
            OpenTexture(i + 0x16b, (void*)"Data\\Npc\\", 0x2600, true);
            OpenTexture(i + 0x16d, (void*)"Data\\Npc\\", 0x2600, true);
        }
        break;
    case 0x151:
        AccessModel(0x151, "Data\\Npc\\", "Girl", 1);
        for (int i = 1; i <= 3; i++) {
            AccessModel(i + 0x16f, "Data\\Npc\\", "GirlHead", i);
            AccessModel(i + 0x171, "Data\\Npc\\", "GirlUpper", i);
            AccessModel(i + 0x173, "Data\\Npc\\", "GirlLower", i);
            OpenTexture(i + 0x16f, (void*)"Data\\Npc\\", 0x2600, true);
            OpenTexture(i + 0x171, (void*)"Data\\Npc\\", 0x2600, true);
            OpenTexture(i + 0x173, (void*)"Data\\Npc\\", 0x2600, true);
        }
        break;
    case 0x152:
        AccessModel(0x152, "Data\\Npc\\", "Smith", 1);
        LoadWaveFile(0x78, "Data\\Sound\\nBlackSmith.wav", 1, false);   // IDA: n<herrero>.wav
        break;
    case 0x153:
        AccessModel(0x153, "Data\\Npc\\", "Wizard", 1);
        break;
    case 0x154:
        AccessModel(0x154, "Data\\Npc\\", "SnowMerchant", 1);
        break;
    case 0x155:
        AccessModel(0x155, "Data\\Npc\\", "SnowSmith", 1);
        break;
    case 0x156:
        AccessModel(0x156, "Data\\Npc\\", "SnowWizard", 1);
        break;
    case 0x157:
        AccessModel(0x157, "Data\\Npc\\", "ElfWizard", 1);
        LoadWaveFile(0x78, "Data\\Sound\\nHarp.wav", 1, false);   // IDA: n<arpa>.wav
        break;
    case 0x158:
        AccessModel(0x158, "Data\\Npc\\", "ElfMerchant", 1);
        break;
    case 0x159:
        AccessModel(0x159, "Data\\Npc\\", "Master", 1);
        break;
    case 0x15a:
        AccessModel(0x15a, "Data\\Npc\\", "Storage", 1);
        break;
    case 0x15b:
        AccessModel(0x15b, "Data\\Npc\\", "Tournament", 1);
        break;
    case 0x15c:
        AccessModel(0x15c, "Data\\Npc\\", "MixNpc", 1);
        LoadWaveFile(0x79, "Data\\Sound\\nMix.wav", 1, false);   // IDA: n<mix>.wav
        break;
    case 0x15d:
        AccessModel(0x15d, "Data\\Npc\\", "DevilNpc", 1);
        break;
    case 0x176:
        if (DAT_0055a7c4 == '\0') {
            OpenModel(0x176, "Data2\\Npc\\", "Sevina", "Sevina", (void*)&DAT_00559088);
        }
        AccessModel(0x176, "Data\\Npc\\", "Sevina", 1);
        OpenTexture(Type, (void*)"Data\\Npc\\", 0x2600, true);
        DAT_0055a7c4 = '\x01';
        break;
    case 0x177:
        if (DAT_0055a7c4 == '\0') {
            OpenModel(0x177, "Data2\\Npc\\", "BloodCastle1", "BloodCastle1", (void*)&DAT_00559088);
        }
        AccessModel(0x177, "Data\\Npc\\", "BloodCastle", 1);
        OpenTexture(Type, (void*)"Data\\Npc\\", 0x2600, true);
        DAT_0055a7c4 = '\x01';
        break;
    case 0x178:
        if (DAT_0055a7c4 == '\0') {
            OpenModel(0x178, "Data2\\Npc\\", "BloodCastle2", "BloodCastle2", (void*)&DAT_00559088);
        }
        AccessModel(0x178, "Data\\Npc\\", "BloodCastle", 2);
        OpenTexture(Type, (void*)"Data\\Npc\\", 0x2600, true);
        DAT_0055a7c4 = '\x01';
        break;
    case 0x179:
        if (DAT_0055a7c4 == '\0') {
            OpenModel(0x179, "Data2\\Npc\\", "DeviasTrader1", "DeviasTrader1", "DeviasTrader1", (void*)&DAT_00559088);
        }
        AccessModel(0x179, "Data\\Npc\\", "DeviasTrader", 1);
        OpenTexture(Type, (void*)"Data\\Npc\\", 0x2600, true);
        DAT_0055a7c4 = '\x01';
        break;
    }

    // ── Post-switch: carga de la textura base del NPC (bloque común del original) ──
    // IDA OpenNpc @ 0x005091D0, tras el switch: `if (NumMeshes>0) OpenTexture(Type,"Npc\\",GL_NEAREST,1)`.
    // NumMeshes vive en Models[Type].Data+0x24 (stride 0xBC) — validado contra
    // BMD_Load.cpp:223 (escribe numMeshes en +0x24) y SMD_Parser FUN_00441e00 (guard de mesh).
    // Sin esto, los NPCs cuyo `case` no llama OpenTexture explícitamente (Smith 338,
    // Wizard 339, SnowMerchant/Smith/Wizard 340-342, ElfMerchant 344, Master 345,
    // Storage 346, Tournament 347, MixNpc 348, DevilNpc 349) cargan su geometría
    // (nMesh>0) pero SIN textura → se renderizan como bloque sólido ("cubo cyan").
    // Path "Npc\\": OpenTexture (FUN_00505c80) concatena SubFolder+texName y el
    // loader OZJ/TGA antepone "Data\\". OpenTexture ya guardea NumMeshes<=0 pero
    // replicamos el guard del binario original.
    //
    // NOTA (patch separado): el loop PlaySpeed del original (IDA offsets +0x26/+0x30)
    // NO se aplica acá. En nuestro layout real +0x26=NumBones y +0x30=Bones
    // (BMD_Load.cpp:225/233), no NumActions/Actions — se audita/aplica aparte.
    if (*(short*)(DAT_05828d58 + Type * 0xBC + 0x24) > 0)   // NumMeshes > 0
        OpenTexture(Type, (void*)"Npc\\", 0x2600 /*GL_NEAREST*/, true);
}
