// Scene_OpenWorld.cpp

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// IDA: OpenWorld (0x0050E5A0)
// Per IDA decompile (1500 bytes).
// Loads all terrain and tile textures for the current world map.
//
// World name = "World<N>" where N = World+1 (capped at 12 for dungeons 11-16).
// Loads Terrain.map, Terrain<N>.att, terrain.obj (or terrain<N>.obj for maps 2/3),
// TerrainHeight.bmp, TerrainLight.jpg, then 14 tile JPGs (slots 0x23-0x30) +
// 3 alpha-overlay TGAs (slots 0x32-0x34) + leaf01/02 + rain01/02 (always from
// World1) + rain03 (always from World10).
// Paths como IDA: "World%d", "Data/%s/Terrain%d", "Data/%s/terrain%d"; rain01/02
// usan "World1" y rain03 "World10" fijos.
void __cdecl OpenWorld(void) {
    BYTE  uVar1;
    CHAR  world_name[32];
    CHAR  local_40[64];

    DeleteObjects();             // DeleteObjects
    DeleteNpcs();             // DeleteNpcs
    DeleteMonsters();             // DeleteMonsters
    ClearItems();             // ClearItems
    ClearCharacters(HeroKey); // ClearCharacters(HeroKey)

    // Limpiar TODOS los pools de char-select que sobreviven al world load (si no,
    // los tick-functions iteran slots con punteros basura).
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

    OpenWorldModels();             // OpenWorldModels

    int iVar2 = World + 1;
    if (World >= 11 && World <= 16) iVar2 = 12;

    crt_sprintf(world_name, "World%d", iVar2);

    // Desviación: los archivos reales del cliente son EncTerrain%d.{map,att,obj}
    // (versiones encriptadas), y los loaders de map/att/obj no tienen fallback de
    // extensión como el de texturas.
    crt_sprintf(local_40, "Data/%s/EncTerrain%d.map", world_name, iVar2);
    OpenTerrainMapping(local_40);     // OpenTerrainMapping

    // `EncTerrain%d.att` mide 131076 bytes (formato encriptado custom) y
    // `OpenTerrainAttribute` solo acepta 65539 (formato vanilla 0.97k): se prueba
    // primero el `Terrain%d.att` sin encriptar (el formato que espera IDA) y si no
    // existe, EncTerrain*.att.
    crt_sprintf(local_40, "Data/%s/Terrain%d.att", world_name, iVar2);
    if (OpenTerrainAttribute(local_40) == 0) {
        crt_sprintf(local_40, "Data/%s/EncTerrain%d.att", world_name, iVar2);
        OpenTerrainAttribute(local_40);     // OpenTerrainAttribute(FileName)
    }

    crt_sprintf(local_40, "Data/%s/EncTerrain%d.obj", world_name, iVar2);
    OpenObjectsEnc(local_40);     // OpenObjectsEnc

    uVar1 = DAT_0055a7c4;
    if (DAT_083a410c != '\0') DAT_0055a7c4 = 0;

    // Desviación: los archivos reales son OZ* (encriptados), no .bmp/.jpg/.tga, y
    // OpenJPG sólo cambia la extensión si DAT_0055a7c4 != 0 (in-game vale 0): se
    // usan las extensiones reales.
    crt_sprintf(local_40, "%s/TerrainHeight.OZB", world_name); CreateTerrain(local_40);
    crt_sprintf(local_40, "%s/TerrainLight.OZJ",  world_name); OpenTerrainLight(local_40);

    // Tile textures (OZJ: slots 0x23-0x30; OZT alpha overlays: slots 0x32-0x34)
    crt_sprintf(local_40, "%s/TileGrass01.OZJ",  world_name); OpenJPG(local_40, 0x23, 0x2600, 0x2901, 0, '\x01');
    crt_sprintf(local_40, "%s/TileGrass01.OZT",  world_name); OpenTGA(local_40, 0x32, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGrass02.OZT",  world_name); OpenTGA(local_40, 0x33, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGrass03.OZT",  world_name); OpenTGA(local_40, 0x34, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGrass02.OZJ",  world_name); OpenJPG(local_40, 0x24, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGround01.OZJ", world_name); OpenJPG(local_40, 0x25, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGround02.OZJ", world_name); OpenJPG(local_40, 0x26, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGround03.OZJ", world_name); OpenJPG(local_40, 0x27, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileWater01.OZJ",  world_name); OpenJPG(local_40, 0x28, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileWood01.OZJ",   world_name); OpenJPG(local_40, 0x29, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock01.OZJ",   world_name); OpenJPG(local_40, 0x2a, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock02.OZJ",   world_name); OpenJPG(local_40, 0x2b, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock03.OZJ",   world_name); OpenJPG(local_40, 0x2c, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock04.OZJ",   world_name); OpenJPG(local_40, 0x2d, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock05.OZJ",   world_name); OpenJPG(local_40, 0x2e, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock06.OZJ",   world_name); OpenJPG(local_40, 0x2f, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock07.OZJ",   world_name); OpenJPG(local_40, 0x30, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/leaf01.OZT",  world_name); OpenTGA(local_40, 100,  0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "%s/leaf01.OZJ",  world_name); OpenJPG(local_40, 100,  0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "%s/leaf02.OZJ",  world_name); OpenJPG(local_40, 0x65, 0x2600, 0x2900, 0, '\0');
    // rain01/02 always loaded from World1; rain03 always from World10 (per IDA).
    crt_sprintf(local_40, "World1/rain01.OZT"); OpenTGA(local_40, 0x66, 0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "World1/rain02.OZT"); OpenTGA(local_40, 0x67, 0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "World10/rain03.OZT"); OpenTGA(local_40, 0x68, 0x2600, 0x2900, 0, '\0');

    if (DAT_083a410c != '\0') DAT_0055a7c4 = uVar1;
}

// IDA: OpenFont (0x0050F690)
// Resets font state, loads FontInput.tga (slot 0) and FontTest.tga (slot 1) as TGA,
// then builds the font DIB (FUN_0050f5f0) and renderer (FUN_0040f570).
void __cdecl OpenFont(void) {
    PathFinder_ResetContext();
    OpenTGA("Interface/FontInput.tga", 0, 0x2600, 0x2900, 0, '\x01');
    OpenTGA("Interface/FontTest.tga",  1, 0x2600, 0x2900, 0, '\x01');
    Font_CreateTextDib(DAT_055ca004);
    Font_CreateRenderer(DAT_055c9ff8, (int)lpData_055ca044, DAT_055ca004);
}
// Scene resource load/unload routines are implemented in src/Scene/Scene_Resources.cpp.
