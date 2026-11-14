// NPC_ModelLoad.cpp
//
// Carga en runtime de modelos/texturas de NPC.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "assets/model/NpcModelTable.h"
#include "audio/SoundTable.h"
// OpenNpc @ 0x005091D0 — carga el modelo y las texturas de un NPC.
// Cada modelo (335 Female, 338 Smith, 374 Sevina, ...) tiene su sección en
// Content/npc/models.toml.
void __cdecl OpenNpc(int Type) {
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
    char* model = (char*)DAT_05828d58 + BMD_STRIDE * Type;
    if (DAT_0055a7c4 != 0 && *(short*)(model + 0x26) > 0)
        return;

    // Qué cargar para cada modelo sale de Content/npc/models.toml (y los
    // sonidos, de [npc.N] en audio/sound/sounds.toml). El orden es el del
    // binario: para cada variante i, primero los modelos de todas las partes y
    // después sus texturas; algunos slots se pisan entre variantes.
    if (const NpcModel* npc = NpcModel_Get(Type)) {
        char* const folder = const_cast<char*>("Data\\Npc\\");
        AccessModelWithTextures(Type, folder, const_cast<char*>(npc->Model.c_str()), npc->Index);
        for (int i = 1; i <= npc->PartCount; i++) {
            for (const auto& part : npc->Parts)
                AccessModelWithTextures(part.second + i, folder, const_cast<char*>(part.first.c_str()), i);
            for (const auto& part : npc->Parts)
                OpenTexture(part.second + i, (void*)folder, 0x2600, true);
        }
        SoundTable_LoadNpc(Type);
        if (npc->Texture) OpenTexture(Type, (void*)folder, 0x2600, true);
        if (npc->MarksLoaded) DAT_0055a7c4 = '\x01';
    }

    // ── Post-switch: carga de la textura base del NPC (bloque común del original) ──
    // IDA OpenNpc @ 0x005091D0, tras el switch: `if (NumMeshes>0) OpenTexture(Type,"Npc\\",GL_NEAREST,1)`.
    // NumMeshes vive en Models[Type].Data+0x24 (stride 0xBC) — validado contra
    // BMD_Load.cpp:223 (escribe numMeshes en +0x24) y SMD_Parser BMD__RenderBody (guard de mesh).
    // Sin esto, los NPCs cuyo `case` no llama OpenTexture explícitamente (Smith 338,
    // Wizard 339, SnowMerchant/Smith/Wizard 340-342, ElfMerchant 344, Master 345,
    // Storage 346, Tournament 347, MixNpc 348, DevilNpc 349) cargan su geometría
    // (nMesh>0) pero SIN textura → se renderizan como bloque sólido ("cubo cyan").
    // Path "Npc\\": OpenTexture concatena SubFolder+texName y el
    // loader OZJ/TGA antepone "Data\\". OpenTexture ya guardea NumMeshes<=0 pero
    // replicamos el guard del binario original.
    //
    // NOTA (patch separado): el loop PlaySpeed del original (IDA offsets +0x26/+0x30)
    // NO se aplica acá. En nuestro layout real +0x26=NumBones y +0x30=Bones
    // (BMD_Load.cpp:225/233), no NumActions/Actions — se audita/aplica aparte.
    if (*(short*)(DAT_05828d58 + Type * BMD_STRIDE + 0x24) > 0)   // NumMeshes > 0
        OpenTexture(Type, (void*)"Npc\\", 0x2600 /*GL_NEAREST*/, true);
}
