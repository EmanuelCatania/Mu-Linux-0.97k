// Scene_OpenWorld.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_0050e5a0 @ 0x0050e5a0 — OpenWorld(int Map)
// Per IDA decompile (raw/0050E5A0_OpenWorld.c, 1500 bytes).
// Loads all terrain and tile textures for the current world map.
//
// World name = "World<N>" where N = DAT_0055a7ac+1 (capped at 12 for dungeons 11-16).
// Loads Terrain.map, Terrain<N>.att, terrain.obj (or terrain<N>.obj for maps 2/3),
// TerrainHeight.bmp, TerrainLight.jpg, then 14 tile JPGs (slots 0x23-0x30) +
// 3 alpha-overlay TGAs (slots 0x32-0x34) + leaf01/02 + rain01/02 (always from
// World1) + rain03 (always from World10).
//
// BUG-FIX 2026-04-27: fixed path strings to match IDA exactly:
//   - "World_%d"           → "World%d"            (no underscore)
//   - "Data/%s/Terrain/%d" → "Data/%s/Terrain%d"  (no extra slash)
//   - "Data/%s/terrain/%d" → "Data/%s/terrain%d"  (no extra slash)
//   - rain01/02 use "World1" hardcoded; rain03 uses "World10" hardcoded.
//   - Pass FileName to OpenTerrainAttribute (was called with no args → no-op).
void __cdecl FUN_0050e5a0(void) {
    BYTE  uVar1;
    CHAR  world_name[32];
    CHAR  local_40[64];

    FUN_004ffd50();             // DeleteObjects
    FUN_00509190();             // DeleteNpcs
    FUN_00509880();             // DeleteMonsters
    FUN_00502b80();             // ClearItems
    FUN_0045abb0(DAT_05826cac); // ClearCharacters(HeroKey)

    // BUG-FIX 2026-04-28: limpiar TODOS los pools de char-select que
    // sobreviven al world load. Sin esto los tick-functions iteran slots
    // con punteros garbage → AV.
    memset(DAT_07abf5f0, 0, sizeof(DAT_07abf5f0));   // particle pool (3000×0x70)
    memset(DAT_07c5ab3c, 0, sizeof(DAT_07c5ab3c));   // skill effect pool (200×0x70)
    memset(DAT_07b11670, 0, sizeof(DAT_07b11670));   // effect pool (124×0x1bc)
    memset(DAT_07b27150, 0, sizeof(DAT_07b27150));   // joint pool (200×0x9d8)
    memset(DAT_07c74ec8, 0, sizeof(DAT_07c74ec8));   // fade-effect pool (40×0x1bc)
    memset(DAT_07c80128, 0, sizeof(DAT_07c80128));   // spark pool (100×0x70)
    memset(DAT_07c82cdc, 0, sizeof(DAT_07c82cdc));   // flare pool (63×0x70)
    memset(DAT_083a2e90, 0, sizeof(DAT_083a2e90));   // boids pool (10×0x1bc)
    memset(DAT_083a2f78, 0, sizeof(DAT_083a2f78));   // ambient particle pool (10×0x1bc)
    memset(DAT_07e016f8, 0, sizeof(DAT_07e016f8));   // tooltip pool (26×0x254)

    FUN_0050c4d0();             // OpenWorldModels

    int iVar2 = DAT_0055a7ac + 1;
    if (DAT_0055a7ac >= 11 && DAT_0055a7ac <= 16) iVar2 = 12;

    crt_sprintf(world_name, "World%d", iVar2);

    // BUG-FIX 2026-05-01: los archivos reales en bin/Client/Data/World%d/
    // son EncTerrain%d.{map,att,obj} (versiones encrypted). El loader de
    // texturas tiene auto-fallback OZ*↔jpg/tga, pero los loaders de map/
    // att/obj NO. Sin esto los modelos cargan (Object*.bmd OK) pero las
    // INSTANCIAS (qué objeto va dónde) jamás se leen → mapa renderiza
    // solo terreno + hero, sin casas/NPCs estáticos/props.
    crt_sprintf(local_40, "Data/%s/EncTerrain%d.map", world_name, iVar2);
    FUN_004f6f90(local_40);     // OpenTerrainMapping

    // 2026-05-04: el archivo `EncTerrain%d.att` mide 131076 bytes (formato
    // encriptado custom) pero `OpenTerrainAttribute` solo acepta 65539 bytes
    // (formato vanilla 0.97k). Sin .att cargado → DAT_0838bc70 queda en 0
    // → todas las tiles son walkable → atravesamos casas y NPCs.
    // Intentamos el archivo unencrypted `Terrain%d.att` primero (mismo formato
    // que IDA espera). Fallback a EncTerrain*.att si no existe.
    crt_sprintf(local_40, "Data/%s/Terrain%d.att", world_name, iVar2);
    if (FUN_004f6ce0(local_40) == 0) {
        crt_sprintf(local_40, "Data/%s/EncTerrain%d.att", world_name, iVar2);
        FUN_004f6ce0(local_40);     // OpenTerrainAttribute(FileName)
    }

    crt_sprintf(local_40, "Data/%s/EncTerrain%d.obj", world_name, iVar2);
    FUN_004ffe70(local_40);     // OpenObjectsEnc

    uVar1 = DAT_0055a7c4;
    if (DAT_083a410c != '\0') DAT_0055a7c4 = 0;

    // BUG-FIX 2026-05-01: archivos reales en filesystem son OZ* (encrypted),
    // no .bmp/.jpg/.tga. La función FUN_00529740 no hace ext-swap automático
    // a menos que DAT_0055a7c4 != 0 — y en in-game está en 0. Usamos extensiones
    // reales directamente para que fopen abra el archivo correcto.
    crt_sprintf(local_40, "%s/TerrainHeight.OZB", world_name); FUN_004f7270(local_40);
    crt_sprintf(local_40, "%s/TerrainLight.OZJ",  world_name); FUN_004f7250(local_40);

    // Tile textures (OZJ: slots 0x23-0x30; OZT alpha overlays: slots 0x32-0x34)
    crt_sprintf(local_40, "%s/TileGrass01.OZJ",  world_name); FUN_00529740(local_40, 0x23, 0x2600, 0x2901, 0, '\x01');
    crt_sprintf(local_40, "%s/TileGrass01.OZT",  world_name); FUN_00529bd0(local_40, 0x32, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGrass02.OZT",  world_name); FUN_00529bd0(local_40, 0x33, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGrass03.OZT",  world_name); FUN_00529bd0(local_40, 0x34, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGrass02.OZJ",  world_name); FUN_00529740(local_40, 0x24, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGround01.OZJ", world_name); FUN_00529740(local_40, 0x25, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGround02.OZJ", world_name); FUN_00529740(local_40, 0x26, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGround03.OZJ", world_name); FUN_00529740(local_40, 0x27, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileWater01.OZJ",  world_name); FUN_00529740(local_40, 0x28, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileWood01.OZJ",   world_name); FUN_00529740(local_40, 0x29, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock01.OZJ",   world_name); FUN_00529740(local_40, 0x2a, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock02.OZJ",   world_name); FUN_00529740(local_40, 0x2b, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock03.OZJ",   world_name); FUN_00529740(local_40, 0x2c, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock04.OZJ",   world_name); FUN_00529740(local_40, 0x2d, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock05.OZJ",   world_name); FUN_00529740(local_40, 0x2e, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock06.OZJ",   world_name); FUN_00529740(local_40, 0x2f, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock07.OZJ",   world_name); FUN_00529740(local_40, 0x30, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/leaf01.OZT",  world_name); FUN_00529bd0(local_40, 100,  0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "%s/leaf01.OZJ",  world_name); FUN_00529740(local_40, 100,  0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "%s/leaf02.OZJ",  world_name); FUN_00529740(local_40, 0x65, 0x2600, 0x2900, 0, '\0');
    // rain01/02 always loaded from World1; rain03 always from World10 (per IDA).
    crt_sprintf(local_40, "World1/rain01.OZT"); FUN_00529bd0(local_40, 0x66, 0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "World1/rain02.OZT"); FUN_00529bd0(local_40, 0x67, 0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "World10/rain03.OZT"); FUN_00529bd0(local_40, 0x68, 0x2600, 0x2900, 0, '\0');

    if (DAT_083a410c != '\0') DAT_0055a7c4 = uVar1;
}

// FUN_0050f690 @ 0x0050f690 — Font_Init
// Resets font state, loads FontInput.tga (slot 0) and FontTest.tga (slot 1) as TGA,
// then builds the font DIB (FUN_0050f5f0) and renderer (FUN_0040f570).
void __cdecl FUN_0050f690(void) {
    PathFinder_ResetContext();
    FUN_00529bd0("Interface/FontInput.tga", 0, 0x2600, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/FontTest.tga",  1, 0x2600, 0x2900, 0, '\x01');
    Font_CreateTextDib(DAT_055ca004);
    Font_CreateRenderer(DAT_055c9ff8, (int)lpData_055ca044, DAT_055ca004);
}
// Scene resource load/unload routines are implemented in src/Scene/Scene_Resources.cpp.
