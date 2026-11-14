// Terrain_LegacyLoad.cpp

#include "stdafx.h"
#include "audio/SoundTable.h"
#include "platform/Text.h"
#include "platform/IFileSystem.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    ClothNode_ScalarDtor(void *_this, char flags);
#include "globals.h"
#include "functions.h"

void __fastcall Cloth_Integrate(int*, float);
int  __cdecl    Cloth_Solve(DWORD *a1);

#include "net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl operator_delete(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

// Terrain / map loaders (called from OpenWorld / Map_LoadResources)

// OpenTerrainMapping @ 0x004F6F90 — Terrain_LoadMap(path)
// Reads map file: skips 1 byte, copies 0x4000×4 bytes to TerrainMappingLayer1 (tile map),
// next 0x4000×4 bytes to TerrainMappingLayer2 (alt-tile), then 0x10000 height bytes → TerrainMappingAlpha as float.
//
// `EncTerrain%d.map` está encriptado con BuxConvert_1 (3-byte XOR rolling), igual
// que el .att (OpenTerrainAttribute); se descifra antes de parsear.
void __cdecl OpenTerrainMapping(const char *path) {
    Terrain_Clear(); // InitTerrainMappingLayer (IDA)
    FILE *f = crt_fopen(path, DAT_005580ac);
    if (!f) {
        char d[256]; Platform::TextFormat(d, "TerrainMap LOAD FAIL: %s", path);
        DbgLogPublic(d);
        return;
    }
    crt_fseek((int*)f, 0, 2);
    unsigned int sz = (unsigned int)crt_ftell((char*)f);
    crt_fseek((int*)f, 0, 0);
    char *buf = (char*)operator_new(sz);
    if (!buf) { crt_fclose(f); return; }
    crt_fread(buf, 1, sz, (int*)f);
    crt_fclose(f);

    // Decrypt with MapFileDecrypt (16-byte rolling key + running counter).
    // Format post-decrypt: byte 0 = magic, bytes 1+ = 3 layers of 0x10000 bytes.
    MapFileDecrypt((BYTE*)buf, (int)sz);

    // El formato Enc tiene un byte extra de version flag:
    //   .map size = 0x30002 = 1(magic) + 1(version) + 3*0x10000(data)
    //   .obj size = 64324 = 1+1+2(count short)+30*2144 → count=0x0860
    // Por eso se lee desde buf+2 (el parser 0.85 leía desde buf+1).
    char *p = buf + 2;
    DWORD *dst = (DWORD*)TerrainMappingLayer1;
    for (int i = 0; i < 0x4000; i++) { *dst++ = *(DWORD*)p; p += 4; }
    dst = (DWORD*)TerrainMappingLayer2;
    for (int i = 0; i < 0x4000; i++) { *dst++ = *(DWORD*)p; p += 4; }
    // alpha bytes → float (per IDA 0.85: alpha[i] = byte * (1/255.0f))
    float *fDst = TerrainMappingAlpha;
    unsigned char *hSrc = (unsigned char*)p;
    for (int i = 0; i < 0x10000; i++) *fDst++ = (float)*hSrc++ * _DAT_00552b70;
    operator_delete(buf);
}

// OpenTerrainAttribute @ 0x004F6CE0 — OpenTerrainAttribute(FileName)
// Per IDA decompile (raw/004F6CE0_OpenTerrainAttribute.c, 442 bytes).
//
// Reads a 65539-byte .att file into a temp buffer, runs BuxConvert_1 (XOR
// descrambler with 3-byte key at DAT_0055a770), then memcpy bytes [+3..+65539]
// into the terrain wall array DAT_0838bc70 (256×256). First byte must be 0,
// next short must be 0xFFFF (file marker). Per-world magic-byte check at
// known offsets validates the right map. Any byte >= 0x80 triggers Error.
unsigned char* TerrainWall = (unsigned char*)&DAT_0838bc70;
int __cdecl OpenTerrainAttribute(const char *FileName) {
    FILE *fp = Platform::GetFileSystem().Open(FileName, "rb");
    if (!fp) {
        // Falla en silencio: el caller prueba dos formatos (Terrain*.att y
        // EncTerrain*.att) y un MessageBox bloqueante impediría el fallback.
        char dbg[260];
        Platform::TextFormat(dbg, "OpenTerrainAttribute: file not found '%s'", FileName);
        DbgLogPublic(dbg);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    int iSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (iSize != 65539) {
        char dbg[260];
        Platform::TextFormat(dbg, "OpenTerrainAttribute: '%s' wrong size %d (expected 65539)",
                  FileName, iSize);
        DbgLogPublic(dbg);
        fclose(fp);
        return 0;
    }

    static unsigned char attBuf[65539];
    fread(attBuf, 65539, 1, fp);
    BuxConvert(attBuf, 65539);   // BuxConvert_1 (3-byte XOR key)

    memcpy(TerrainWall, attBuf + 3, 65536);

    bool Error = false;
    if (attBuf[0] != 0 || *(unsigned short*)(attBuf + 1) != 0xFFFF)
        Error = true;

    if (DAT_083a410c == '\0') {
        // Per-world magic byte check (sanity vs distributed .att files).
        switch (gMapManager.GetCurrentMap()) {
        case MAP_LORENCIA: if (TerrainWall[31623] != 5) Error = true; break;
        case MAP_DUNGEON: if (TerrainWall[30947] != 4) Error = true; break;
        case MAP_DEVIAS: if (TerrainWall[14288] != 5) Error = true; break;
        case MAP_NORIA: if (TerrainWall[30650] != 5) Error = true; break;
        case MAP_LOST_TOWER: if (TerrainWall[19393] != 5) Error = true; break;
        }
    }
    for (int i = 0; i < 65536; ++i) {
        if (TerrainWall[i] >= 0x80) { Error = true; break; }
    }

    if (Error) {
        // Desviación: en IDA una validación fallida mata el proceso; acá se sigue
        // con walkable=0 implícito.
        DbgLogPublic("OpenTerrainAttribute: validation Error (magic-byte mismatch or byte>=0x80)");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

// OpenObjectsEnc @ 0x004FFE70 — Terrain_LoadObjects(path)
// Reads .obj file: 2-byte count, then count×0x1e entries → calls CreateObject for each.
//
// `EncTerrain%d.obj` está encriptado con BuxConvert_1 (igual que el .att); se
// descifra antes de parsear.
void __cdecl OpenObjectsEnc(const char *path) {
    FILE *f = crt_fopen(path, DAT_005580ac);
    if (!f) {
        // Text es un buffer local, como en IDA. Ghidra lo había decompilado como el
        // global DAT_083a0218, que en nuestro build es la celda 0 de la grilla de
        // buckets: escribir ahí rompe el recorrido de objetos.
        char Text[256];
        crt_sprintf(Text, "OpenObjectsEnc: file not found '%s'", path);
        DbgLogPublic(Text);
        return;
    }
    crt_fseek((int*)f, 0, 2);
    unsigned int sz = (unsigned int)crt_ftell((char*)f);
    crt_fseek((int*)f, 0, 0);
    char *buf = (char*)operator_new(sz);
    crt_fread(buf, 1, sz, (int*)f);
    crt_fclose(f);

    // Decrypt with MapFileDecrypt; format Enc post-decrypt: byte 0 = magic,
    // byte 1 = version flag, short[2..3] = count, entries from byte 4 (stride 30B).
    MapFileDecrypt((BYTE*)buf, (int)sz);

    int count = (int)*(short*)(buf + 2);
    // IDA 0x4FFE70 sólo chequea count > 0. El tope sale del tamaño del buffer
    // (header de 4 bytes + 30 por entrada) para no leer fuera; no poner un máximo
    // fijo: los conteos reales llegan a 9399 (Noria).
    int maxByBuf = ((int)sz - 4) / 30;
    if (count > maxByBuf) count = maxByBuf;
    if (count > 0) {
        short *p = (short*)(buf + 4);
        for (int i = 0; i < count; i++, p += 0xf) {
            float pos[3]  = { *(float*)(p+1), *(float*)(p+3), *(float*)(p+5) };
            float tgt[3]  = { *(float*)(p+7), *(float*)(p+9), *(float*)(p+0xb) };
            // El 4° arg de CreateObject es la escala del objeto: se lee como `*(float*)`,
            // igual que IDA, no como entero.
            CreateObject((int)*p, pos, tgt, *(float*)(p + 0xd));
        }
    }
    operator_delete(buf);
}

// OpenTerrainLight @ 0x004F7250 — Terrain_LoadLight(path)
// Loads TerrainLight.jpg into DAT_07eeb238 (RGB float buffer, 256x256x3),
// then processes via FUN_004f70b0 / FUN_004f71c0.
void __cdecl OpenTerrainLight(const char *path) {
    OpenJpegBuffer((char*)path, DAT_07eeb238);
    CreateTerrainNormal(); // FUN_004f70b0 (IDA)
    CreateTerrainLight(); // IDA: CreateTerrainLight (0x004F71C0)
}

// CreateTerrain @ 0x004F7270 — Terrain_LoadHeight(path)
// Sets flag, loads height bitmap, flushes.
void __cdecl CreateTerrain(const char *path) {
    DAT_0839bc84 = 1;
    OpenTerrainHeight((char*)path); // FUN_004f7290 (IDA)
    Terrain_SetupCulling();
}

// ClearItems @ 0x00502B80 — ClearItems / Map_InitEntities
// Limpia el flag "vivo" de cada slot del pool de GroundItem (DAT_07e12840,
// 1000 slots × 0x204 bytes). Como IDA, arranca en `&Items[0][72]`: el flag vive
// en +72, que es el que leen Net_Process (0x20), Entity_Render y MoveItems.
void __cdecl ClearItems(void) {
    for (int i = 0; i < 1000; ++i) {
        DAT_07e12840[i * GROUND_ITEM_STRIDE + 72] = 0;
    }
}

// DeleteNpcs @ 0x00509190 — Terrain_InitLayers
// Frees tile model slots (0xf604..0x11710), then frees sound channels 0x78..0xa9.
void __cdecl DeleteNpcs(void) {
    for (int slot = 0x14f; slot < 0x17c; ++slot)
        BMD__Release(AsModel(DAT_05828d58) + slot);
    for (int i = 0x78; i < 0xaa; i++) Sound_ReleaseBuffer(i); // IDA: FUN_00404AD0
}

// DeleteMonsters @ 0x00509880 — Terrain_InitWater
// Frees water model slots (0xc648..0xf604), then frees sound channels 0xaa..0x1a3.
void __cdecl DeleteMonsters(void) {
    for (int slot = 0x10e; slot < 0x14f; ++slot)
        BMD__Release(AsModel(DAT_05828d58) + slot);
    for (int i = 0xaa; i < 0x1a4; i++) Sound_ReleaseBuffer(i); // IDA: FUN_00404AD0
}

// OpenWorldModels @ 0x0050C4D0 — Map_LoadObjectModels
// Loads world-specific animated props + object models for current zone.
void __cdecl OpenWorldModels(void) {
    char cVar2 = DAT_0055a7c4;
    if (DAT_083a410c != '\0') {
        DAT_0055a7c4 = '\0';
        gMapManager.SetCurrentMap(7);
    }

    OpenJPG("Object8/drop01.jpg", 0x4d9, 0x2600, 0x2900, 0, '\x01');

    if (DAT_0055a7c4 == '\0') {
        switch (gMapManager.GetCurrentMap()) {
        case MAP_LORENCIA:
            break;
        case MAP_DUNGEON:
        case MAP_LOST_TOWER:
            break;
        case MAP_NORIA:
            break;
        case MAP_EXILE:
            break;
        case MAP_ARENA:
            *(unsigned char *)((uintptr_t)AsModel(DAT_05828d58 + BMD_STRIDE * 178)->Actions + 0x1a) = 1;
            break;
        case MAP_ATLANS:
            for (int iVar7 = 0xbc; iVar7 < 0x69c; iVar7 += 0xbc)
                *(unsigned char *)((intptr_t)AsModel(DAT_05828d58 + BMD_STRIDE * (181 + iVar7 / 0xbc))->Actions + 10) = 1;
            break;
        case MAP_TARKAN:
            break;
        case MAP_ICARUS:
            OpenJPG("Effect/clouds.jpg",      0x4f4, 0x2601, 0x2900, 0, '\x01');
            AccessModel(0xb6, "Data/Object11/", "cloud", -1);
            OpenTexture(0xb6, "Object11/", 0x2600, '\x01');
            OpenJPG("Effect/cloudLight.jpg",  0x4f5, 0x2601, 0x2900, 0, '\x01');
            break;
        case MAP_BLOOD_CASTLE1: case MAP_BLOOD_CASTLE2: case MAP_BLOOD_CASTLE3: case MAP_BLOOD_CASTLE4: case MAP_BLOOD_CASTLE5: case MAP_BLOOD_CASTLE6:
            OpenJPG("Effect/clouds.jpg", 0x4f4, 0x2601, 0x2900, 0, '\x01');
            LoadWaveFile(0x6e, "Data/Sound/iBloodCastle.wav", 1, '\0');
            DAT_0055a7c4 = '\x01';
            break;
        }
    }

    // Cargas propias de cada mapa (IDA 0x50C4D0, segundo switch, el que corre en
    // cada entrada): [[map.N.load]] de Content/map/maps.toml, en su orden.
    SetMaxTextures(0x69);
    const int map = gMapManager.GetCurrentMap();
    const MapInfo& info = gMapManager.Info(map);
    for (const MapLoad& load : info.loads) {
        switch (load.kind) {
        case MapLoad::Kind::Model:
            AccessModel(load.slot, load.path.c_str(), load.name.c_str(), load.index);
            break;
        case MapLoad::Kind::Texture:
            OpenTexture(load.slot, load.path.c_str(), 0x2600, '\x01');
            break;
        case MapLoad::Kind::Jpg:
            OpenJPG(load.path.c_str(), load.slot, 0x2601, load.repeat ? 0x2901 : 0x2900, 0, '\x01');
            break;
        }
    }
    if (map == MAP_ATLANS) {
        // Texturas de agua de Atlans (IDA L175-199). Carga wt00..wt31 en Bitmaps[65..96] y además
        // copia el nombre corto en Bitmaps[n].FileName (offset 0 del slot,
        // stride 0x38), que es de donde lo lee el render de tiles de agua.
        // El "if (v5 >= &Bitmaps[75])" del decompile es simplemente v4 >= 10:
        // wt00..wt09 llevan cero a la izquierda, wt10..wt31 no.
        {
            char Buffer[64];
            for (int v4 = 0; v4 < 32; v4++) {
                const char* fmtFull = (v4 >= 10) ? "Object8/wt%d.jpg" : "Object8/wt0%d.jpg";
                Platform::TextFormatTruncated(Buffer, sizeof(Buffer), Platform::TruncateText, fmtFull, v4);
                OpenJPG(Buffer, v4 + 65, 0x2601, 0x2901, 0, '\0');

                const char* fmtLeaf = (v4 >= 10) ? "wt%d.jpg" : "wt0%d.jpg";
                Platform::TextFormatTruncated(Buffer, sizeof(Buffer), Platform::TruncateText, fmtLeaf, v4);
                Platform::TextCopy(&g_BitmapsRaw[(v4 + 65) * TEXTURE_STRIDE], Buffer, 0x20);
            }
        }
    }
    // El sonido del evento de Blood Castle (PlayBuffer(110) del estado 0 del 0x9B).
    SoundTable_LoadMap(map);

    // Object model loading for all maps (SetMaxTextures(0x2ee) then per-map loading)
    SetMaxTextures(0x2ee);
    const int objFolder = MapInfo_ObjectFolder(map);
    char objectDir[32], textureDir[32];
    crt_sprintf(objectDir, "Data/Object%d/", objFolder);
    crt_sprintf(textureDir, "Object%d/", objFolder);
    if (!info.objects.empty()) {
        // Objetos con nombre propio ([map.N.objects]; en vanilla, Lorencia).
        if (map == MAP_LORENCIA && DAT_0055a7c4 == '\0') {
            // IDA 0x0050C4D0: mushrooms are 26..27; stones are 30..34.
            // These IDs are consumed directly as Models[Object.Type].
            DAT_083a4100 = 1;
        }
        // Los IDs de modelo son los del binario; los nombres son los .bmd del
        // pack (jar_01.smd es Well01.bmd, por ejemplo).
        for (const auto& object : info.objects) {
            char* slotPtr = (char*)((uintptr_t)DAT_05828d58 + (uintptr_t)BMD_STRIDE * object.first);
            if (AsModel(slotPtr)->NumBones > 0) continue;  // already loaded (e.g. SMD worked)
            AccessModel(object.first, objectDir, object.second.c_str(), -1);
        }
        for (int i = 0; i < 0xa0; i++)
            OpenTexture(i, textureDir, 0x2600, '\x01');
    } else {
        // Carpeta de objetos (IDA 0x50C4D0: World + 1, 12 en Blood Castle):
        // object_folder de maps.toml.
        // Dynamic map: load from per-map object file
        if (DAT_0055a7c4 == '\0') {
            char local_384[32];
            crt_sprintf(local_384, "Data2/Object%d/", objFolder);
            ParserFileHandle = Platform::GetFileSystem().Open(local_384, "rt");
            if (ParserFileHandle != nullptr) {
                char local_300[256], local_200[256], local_100[256];
                while (ParseNextToken() != 2) {
                    int objIdx = ParserCurrentToken;
                    ParseNextToken(); strncpy(local_300, ParserTokenString, 255);
                    ParseNextToken(); strncpy(local_200, ParserTokenString, 255);
                    ParseNextToken(); strncpy(local_100, ParserTokenString, 255);
                    char pathBuf[32];
                    crt_sprintf(pathBuf, "Data2/Object%d/", objFolder);
                }
                fclose(ParserFileHandle);
            }
        }
        // Register data paths for all 0xa0 object slots
        for (int i = 0; i < 0xa0; i++)
            AccessModel(i, objectDir, "Object", i + 1);
        SetMaxTextures(0x2ee);
        for (int i = 0; i < 0xa0; i++)
            OpenTexture(i, textureDir, 0x2600, '\x01');
        // Map-specific post-load fixups
        if (map == MAP_DUNGEON)
            *(unsigned int *)((uintptr_t)AsModel(DAT_05828d58 + BMD_STRIDE * 40)->Actions + sizeof(ModelAnimationAction) + 4) = 0x3ecccccd; // 0.4f
        else if (map == MAP_TARKAN) {
            *(unsigned char *)(DAT_05828d58 + BMD_STRIDE * 11 + 0x88 + BMD_FIELD_SHIFT) = 0;
            *(unsigned char *)(DAT_05828d58 + BMD_STRIDE * 12 + 0x88 + BMD_FIELD_SHIFT) = 0;
            *(unsigned char *)(DAT_05828d58 + BMD_STRIDE * 13 + 0x88 + BMD_FIELD_SHIFT) = 0;
            *(unsigned char *)(DAT_05828d58 + BMD_STRIDE * 73 + 0x88 + BMD_FIELD_SHIFT) = 0;
            *(unsigned char *)(DAT_05828d58 + BMD_STRIDE * 75 + 0x88 + BMD_FIELD_SHIFT) = 0;
            *(unsigned char *)(DAT_05828d58 + BMD_STRIDE * 79 + 0x88 + BMD_FIELD_SHIFT) = 0;
        }
    }

    if (DAT_083a410c != '\0')
        DAT_0055a7c4 = cVar2;
}

// Font helpers (called from OpenFont)
