// Terrain_LegacyLoad.cpp

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

void __fastcall Cloth_Integrate(int*, float);
int  __cdecl    Cloth_Solve(DWORD *a1);

#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl operator_delete(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) operator_delete((unsigned char*)(p))
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
        char d[256]; wsprintfA(d, "TerrainMap LOAD FAIL: %s", path);
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
    FILE *fp = fopen(FileName, "rb");
    if (!fp) {
        // Falla en silencio: el caller prueba dos formatos (Terrain*.att y
        // EncTerrain*.att) y un MessageBox bloqueante impediría el fallback.
        char dbg[260];
        wsprintfA(dbg, "OpenTerrainAttribute: file not found '%s'", FileName);
        DbgLogPublic(dbg);
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    int iSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (iSize != 65539) {
        char dbg[260];
        wsprintfA(dbg, "OpenTerrainAttribute: '%s' wrong size %d (expected 65539)",
                  FileName, iSize);
        DbgLogPublic(dbg);
        fclose(fp);
        return 0;
    }

    static unsigned char attBuf[65539];
    fread(attBuf, 65539, 1, fp);
    BuxConvert((int)attBuf, 65539);   // BuxConvert_1 (3-byte XOR key)

    memcpy(TerrainWall, attBuf + 3, 65536);

    bool Error = false;
    if (attBuf[0] != 0 || *(unsigned short*)(attBuf + 1) != 0xFFFF)
        Error = true;

    if (DAT_083a410c == '\0') {
        // Per-world magic byte check (sanity vs distributed .att files).
        switch (World) {
        case 0: if (TerrainWall[31623] != 5) Error = true; break;
        case 1: if (TerrainWall[30947] != 4) Error = true; break;
        case 2: if (TerrainWall[14288] != 5) Error = true; break;
        case 3: if (TerrainWall[30650] != 5) Error = true; break;
        case 4: if (TerrainWall[19393] != 5) Error = true; break;
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
    OpenJpegBuffer((char*)path, (int)(uintptr_t)DAT_07eeb238);
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
        DAT_07e12840[i * 0x204 + 72] = 0;
    }
}

// DeleteNpcs @ 0x00509190 — Terrain_InitLayers
// Frees tile model slots (0xf604..0x11710), then frees sound channels 0x78..0xa9.
void __cdecl DeleteNpcs(void) {
    for (int i = 0xf604; i < 0x11710; i += 0xbc)
        BMD__Release(i + DAT_05828d58);
    for (int i = 0x78; i < 0xaa; i++) Sound_ReleaseBuffer(i); // IDA: FUN_00404AD0
}

// DeleteMonsters @ 0x00509880 — Terrain_InitWater
// Frees water model slots (0xc648..0xf604), then frees sound channels 0xaa..0x1a3.
void __cdecl DeleteMonsters(void) {
    for (int i = 0xc648; i < 0xf604; i += 0xbc)
        BMD__Release(i + DAT_05828d58);
    for (int i = 0xaa; i < 0x1a4; i++) Sound_ReleaseBuffer(i); // IDA: FUN_00404AD0
}

// OpenWorldModels @ 0x0050C4D0 — Map_LoadObjectModels
// Loads world-specific animated props + object models for current zone.
// (Scene_Objects.cpp tiene un port alternativo con strings distintos.)
void __cdecl OpenWorldModels(void) {
    char cVar2 = DAT_0055a7c4;
    if (DAT_083a410c != '\0') {
        DAT_0055a7c4 = '\0';
        World = 7;
    }

    OpenJPG("Object8/drop01.jpg", 0x4d9, 0x2600, 0x2900, 0, '\x01');

    if (DAT_0055a7c4 == '\0') {
        switch (World) {
        case 0:
            OpenModel((int)0xae, "Data2/Object1/Animal/", "bird.smd");
            OpenModel((int)0xb5, "Data2/Object1/Animal/", "fish.smd");
            break;
        case 1:
        case 4:
            OpenModel((int)0xd7, "Data2/Object2/", "DungeonStone.smd");
            OpenModel((int)0xb0, "Data2/Object2/", "Bat.smd");
            OpenModel((int)0xb1, "Data2/Object2/", "mouse.smd");
            break;
        case 3:
            OpenModel((int)0xaf, "Data2/Object1/Animal/", "butterfly.smd");
            break;
        case 5:
            OpenModel((int)0xe4, "Data2/Object6/", "Meteo.smd");
            OpenModel((int)0xe5, "Data2/Object6/", "Meteo.smd");
            OpenModel((int)0xe6, "Data2/Object6/", "Meteo.smd");
            OpenModel((int)0xe7, "Data2/Object6/", "Meteo.smd");
            OpenModel((int)0xe8, "Data2/Object6/", "Meteo.smd");
            OpenModel((int)0xea, "Data2/Monster/", "BossHead.smd");
            OpenModel((int)0xeb, "Data2/Object6/", "Princess.smd");
            break;
        case 6:
            OpenModel((int)0xb2, "Data2/Object7/", "SummonMonster.smd");
            *(unsigned char *)(*(int *)(DAT_05828d58 + 0x82e8) + 0x1a) = 1;
            break;
        case 7:
            OpenModel((int)0xb6, "Data2/Object8/", "WaterMill.smd");
            OpenModel((int)0xb7, "Data2/Object8/", "BladedStatue.smd");
            OpenModel((int)0xb8, "Data2/Object8/", "Stairway.smd");
            OpenModel((int)0xb9, "Data2/Object8/", "Bridge.smd");
            OpenModel((int)0xba, "Data2/Object8/", "Trap01.smd");
            OpenModel((int)0xbb, "Data2/Object8/", "Trap02.smd");
            OpenModel((int)0xbc, "Data2/Object8/", "Trap03.smd");
            OpenModel((int)0xbd, "Data2/Object8/", "FireArrow.smd");
            for (int iVar7 = 0xbc; iVar7 < 0x69c; iVar7 += 0xbc)
                *(unsigned char *)(*(int *)(iVar7 + 0x851c + DAT_05828d58) + 10) = 1;
            break;
        case 8:
            OpenModel((int)0xb3, "Data2/Object9/", "SandPillar.smd");
            break;
        case 10:
            OpenJPG("Effect/clouds.jpg",      0x4f4, 0x2601, 0x2900, 0, '\x01');
            OpenModel((int)0xb6, "Data2/Object11/", "cloud.smd");
            AccessModel(0xb6, "Data/Object11/", "cloud", -1);
            OpenTexture(0xb6, "Object11/", 0x2600, '\x01');
            OpenJPG("Effect/cloudLight.jpg",  0x4f5, 0x2601, 0x2900, 0, '\x01');
            break;
        case 0xb: case 0xc: case 0xd: case 0xe: case 0xf: case 0x10:
            OpenModel((int)0xb8, "Data2/Object12/", "Angel.smd");
            OpenModel((int)0x106, "Data2/Object12/", "gate_entrance.smd");
            OpenModel((int)0x107, "Data2/Object12/", "gate_entrance2.smd");
            OpenModel((int)0x104, "Data2/Object12/", "gate_left.smd");
            OpenModel((int)0x105, "Data2/Object12/", "gate_right.smd");
            OpenModel((int)0xb9, "Data2/Object12/", "shine.smd");
            OpenJPG("Effect/clouds.jpg", 0x4f4, 0x2601, 0x2900, 0, '\x01');
            LoadWaveFile(0x6e, "Data/Sound/iBloodCastle.wav", 1, '\0');
            DAT_0055a7c4 = '\x01';
            break;
        }
    }

    // Object type texture/name registration (second pass, all maps)
    SetMaxTextures(0x69);
    switch (World) {
    case 0:
        AccessModel(0xae, "Data/Object1/", "bird", 1);
        OpenTexture(0xae, "Object1/", 0x2600, '\x01');
        AccessModel(0xb5, "Data/Object1/", "fish", 1);
        OpenTexture(0xb5, "Object1/", 0x2600, '\x01');
        break;
    case 1: case 4:
        AccessModel(0xd7, "Data/Object2/", "DungeonStone", 1);
        OpenTexture(0xd7, "Object2/", 0x2600, '\x01');
        AccessModel(0xb0, "Data/Object2/", "Bat", 1);
        OpenTexture(0xb0, "Object2/", 0x2600, '\x01');
        AccessModel(0xb1, "Data/Object2/", "mouse", 1);
        OpenTexture(0xb1, "Object2/", 0x2600, '\x01');
        break;
    case 3:
        AccessModel(0xaf, "Data/Object1/", "Butterfly", 1);
        OpenTexture(0xaf, "Object1/", 0x2600, '\x01');
        break;
    case 5:
        for (int i = 0xe4; i < 0xec; i++) {
            AccessModel(i, "Data/Object6/", "Meteo", i - 0xe3);
            OpenTexture(i, "Object6/", 0x2600, '\x01');
        }
        AccessModel(0xea, "Data/Object6/", "BossHead", 1);
        AccessModel(0xeb, "Data/Object6/", "Princess", 1);
        break;
    case 6:
        AccessModel(0xb2, "Data/Object7/", "SummonMonster", 1);
        OpenTexture(0xb2, "Object7/", 0x2600, '\x01');
        break;
    case 7:
        // IDA 0x50C4D0: ++v3; while (v3 - 181 < 9) loads 182..189
        // (Fish02..Fish09). Slot 190 is Ice01: loading its textures from
        // Object8 corrupts Ice Arrow for this map and subsequent maps.
        for (int i = 182; i < 190; ++i) {
            AccessModel(i, "Data/Object8/", "Fish", i - 0xb4);
            OpenTexture(i, "Object8/", 0x2600, '\x01');
        }
        // Texturas de agua de Atlans (IDA L175-199). Carga wt00..wt31 en Bitmaps[65..96] y además
        // copia el nombre corto en Bitmaps[n].FileName (offset 0 del slot,
        // stride 0x38), que es de donde lo lee el render de tiles de agua.
        // El "if (v5 >= &Bitmaps[75])" del decompile es simplemente v4 >= 10:
        // wt00..wt09 llevan cero a la izquierda, wt10..wt31 no.
        {
            char Buffer[64];
            for (int v4 = 0; v4 < 32; v4++) {
                const char* fmtFull = (v4 >= 10) ? "Object8/wt%d.jpg" : "Object8/wt0%d.jpg";
                _snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, fmtFull, v4);
                OpenJPG(Buffer, v4 + 65, 0x2601, 0x2901, 0, '\0');

                const char* fmtLeaf = (v4 >= 10) ? "wt%d.jpg" : "wt0%d.jpg";
                _snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, fmtLeaf, v4);
                lstrcpynA(&g_BitmapsRaw[(v4 + 65) * 0x38], Buffer, 0x20);
            }
        }
        break;
    case 8:
        OpenJPG("Object9/sand01.jpg",    0x494, 0x2601, 0x2901, 0, '\x01');
        OpenJPG("Object9/sand02.jpg",    0x495, 0x2601, 0x2901, 0, '\x01');
        OpenJPG("Object9/Impack03.jpg",  0x597, 0x2601, 0x2900, 0, '\x01');
        AccessModel(0xb3, "Data/Object9/", "SandPillar", 2);
        OpenTexture(0xb3, "Object9/", 0x2600, '\x01');
        break;
    case 10:
        // IDA 0050C4D0 L209-214 (segundo switch, el que corre siempre):
        //     OpenJPG("Effect\clouds.jpg",     0x4F4u, ...);
        //     AccessModelWithTextures(182, "Data\Object11", "cloud", -1);
        //     OpenTexture(182, "Object11\\", 9728, 1);
        //     OpenJPG("Effect\cloudLight.jpg", 0x4F5u, ...);
        //
        // Al port le faltaban las DOS texturas: solo las cargaba el PRIMER
        // switch, gateado por `DAT_0055a7c4 == 0`, que ya no corre cuando se
        // llega a Icarus. La textura 1268 quedaba sin handle GL y las ~6800
        // nubes por frame se dibujaban invisibles.
        OpenJPG("Effect/clouds.jpg",     0x4f4, 0x2601, 0x2900, 0, '\x01');
        AccessModel(0xb6, "Data/Object11/", "cloud", -1);
        OpenTexture(0xb6, "Object11/", 0x2600, '\x01');
        OpenJPG("Effect/cloudLight.jpg", 0x4f5, 0x2601, 0x2900, 0, '\x01');
        break;
    case 0xb: case 0xc: case 0xd: case 0xe: case 0xf: case 0x10:
        // Blood Castle.  Port 1:1 de IDA 0x50C4D0 L228-242:
        //     AccessModelWithTextures(184, "Data\\Object12\\", "Crow", 1);
        //     OpenTexture(184, "Object12\\", 9728, 1);
        //     AccessModelWithTextures(262/263, "Data\\Object12\\", "Gate", 1/2);
        //     AccessModelWithTextures(260/261, "Data\\Object12\\", "StoneCoffin", 1/2);
        //     for (k=0;k<2;k++) OpenTexture(k+262, "Monster\\", 9728, 1);
        //     for (m=0;m<2;m++) OpenTexture(m+260, "Monster\\", 9728, 1);
        //     AccessModelWithTextures(185, "Data\\Object12\\", "Shine", 1);
        //     OpenTexture(185, "Object12\\", 9728, 1);
        AccessModel(0xb8, "Data/Object12/", "Crow", 1);
        OpenTexture(0xb8, "Object12/", 0x2600, '\x01');
        AccessModel(0x106, "Data/Object12/", "Gate", 1);
        AccessModel(0x107, "Data/Object12/", "Gate", 2);
        AccessModel(0x104, "Data/Object12/", "StoneCoffin", 1);
        AccessModel(0x105, "Data/Object12/", "StoneCoffin", 2);
        for (int k = 0; k < 2; ++k)
            OpenTexture(k + 0x106, "Monster/", 0x2600, '\x01');
        for (int m = 0; m < 2; ++m)
            OpenTexture(m + 0x104, "Monster/", 0x2600, '\x01');
        AccessModel(0xb9, "Data/Object12/", "Shine", 1);
        OpenTexture(0xb9, "Object12/", 0x2600, '\x01');
        // Van en este switch, fuera del `if (!unk_55A7C4)` interno, como en IDA
        // (0x50C4D0 L243-245): se ejecutan en cada entrada al mapa. El LoadWaveFile hace
        // falta para el `PlayBuffer(110, 0, 1)` del estado 0 del 0x9B (música del evento).
        OpenJPG("Effect/clouds.jpg", 0x4f4, 0x2601, 0x2900, 0, 1);
        LoadWaveFile(0x6e, "Data/Sound/iBloodCastle.wav", 1, 0);
        break;
    }

    // Object model loading for all maps (SetMaxTextures(0x2ee) then per-map loading)
    SetMaxTextures(0x2ee);
    if (World == 0) {
        // Lorencia (Object1) — load SMD models on first call
        if (DAT_0055a7c4 == '\0') {
            OpenModel((int)0x00, "Data2/Object1/", "treesmall.smd");
            OpenModel((int)0x01, "Data2/Object1/", "treebig.smd");
            OpenModel((int)0x02, "Data2/Object1/", "treea_01.smd");
            OpenModel((int)0x03, "Data2/Object1/", "treea_02.smd");
            OpenModel((int)0x04, "Data2/Object1/", "treea_03.smd");
            OpenModel((int)0x05, "Data2/Object1/", "treea_04.smd");
            OpenModel((int)0x06, "Data2/Object1/", "treea_05.smd");
            OpenModel((int)0x07, "Data2/Object1/", "treea_06.smd");
            OpenModel((int)0x08, "Data2/Object1/", "treea_07.smd");
            OpenModel((int)0x09, "Data2/Object1/", "treea_08.smd");
            OpenModel((int)0x0a, "Data2/Object1/", "treea_09.smd");
            OpenModel((int)0x0b, "Data2/Object1/", "treea_10.smd");
            OpenModel((int)0x0c, "Data2/Object1/", "treea_11.smd");
            OpenModel((int)0x14, "Data2/Object1/", "grass_01.smd");
            OpenModel((int)0x15, "Data2/Object1/", "grass_02.smd");
            OpenModel((int)0x16, "Data2/Object1/", "grass_03.smd");
            OpenModel((int)0x17, "Data2/Object1/", "grass_04.smd");
            OpenModel((int)0x18, "Data2/Object1/", "grass_05.smd");
            OpenModel((int)0x19, "Data2/Object1/", "grass_06.smd");
            // IDA 0x0050C4D0: mushrooms are 26..27; stones are 30..34.
            // These IDs are consumed directly as Models[Object.Type].
            OpenModel((int)0x1a, "Data2/Object1/", "mushroom_01.smd");
            OpenModel((int)0x1b, "Data2/Object1/", "mushroom_02.smd");
            OpenModel((int)0x1e, "Data2/Object1/", "Ston_01.smd");
            OpenModel((int)0x1f, "Data2/Object1/", "Ston_02.smd");
            OpenModel((int)0x20, "Data2/Object1/", "Ston_03.smd");
            OpenModel((int)0x21, "Data2/Object1/", "Ston_04.smd");
            OpenModel((int)0x22, "Data2/Object1/", "Ston_05.smd");
            OpenModel((int)0x28, "Data2/Object1/", "stone_statue01.smd");
            OpenModel((int)0x29, "Data2/Object1/", "stone_statue02.smd");
            OpenModel((int)0x2a, "Data2/Object1/", "Angel_Stone.smd");
            OpenModel((int)0x2b, "Data2/Object1/", "steel_barred_door_side.smd");
            OpenModel((int)0x2c, "Data2/Object1/", "Tomb_arc.smd");
            DAT_083a4100 = 1;
            OpenModel((int)0x2d, "Data2/Object1/", "Tomb_cross.smd");
            OpenModel((int)0x2e, "Data2/Object1/", "TombStone.smd");
            OpenModel((int)0x32, "Data2/Object1/", "fire_light.smd");
            OpenModel((int)0x33, "Data2/Object1/", "Fire_Light_01.smd");
            OpenModel((int)0x34, "Data2/Object1/", "Fire.smd");
            OpenModel((int)0x37, "Data2/Object1/", "dungeon_gate_01.smd");
            OpenModel((int)0x3a, "Data2/Object1/", "Drum.smd");
            OpenModel((int)0x3b, "Data2/Object1/", "Treasure_Chest.smd");
            OpenModel((int)0x3c, "Data2/Object1/", "ship.smd");
            OpenModel((int)0x38, "Data2/Object1/", "monster_a.smd");
            OpenModel((int)0x39, "Data2/Object1/", "monster_b.smd");
            OpenModel((int)0x41, "Data2/Object1/", "steel_barred_wall01.smd");
            OpenModel((int)0x42, "Data2/Object1/", "steel_barred_wall02.smd");
            OpenModel((int)0x43, "Data2/Object1/", "steel_barred_wall03.smd");
            OpenModel((int)0x44, "Data2/Object1/", "steel_barred_door.smd");
            OpenModel((int)0x45, "Data2/Object1/", "wall_01.smd");
            OpenModel((int)0x46, "Data2/Object1/", "wall_02.smd");
            OpenModel((int)0x47, "Data2/Object1/", "wall_03.smd");
            OpenModel((int)0x48, "Data2/Object1/", "wall_04.smd");
            OpenModel((int)0x49, "Data2/Object1/", "wall_05.smd");
            OpenModel((int)0x4a, "Data2/Object1/", "wall_06.smd");
            OpenModel((int)0x4b, "Data2/Object1/", "c_wall01.smd");
            OpenModel((int)0x4c, "Data2/Object1/", "c_wall02.smd");
            OpenModel((int)0x4d, "Data2/Object1/", "c_wall03.smd");
            OpenModel((int)0x4e, "Data2/Object1/", "c_wall04.smd");
            OpenModel((int)0x4f, "Data2/Object1/", "c_wall05.smd");
            OpenModel((int)0x50, "Data2/Object1/", "bridge_01.smd");
            OpenModel((int)0x51, "Data2/Object1/", "fence_01.smd");
            OpenModel((int)0x52, "Data2/Object1/", "fence_02.smd");
            OpenModel((int)0x53, "Data2/Object1/", "fence_03.smd");
            OpenModel((int)0x54, "Data2/Object1/", "fence_04.smd");
            OpenModel((int)0x55, "Data2/Object1/", "bridge_stone.smd");
            OpenModel((int)0x5a, "Data2/Object1/", "StreetLight.smd");
            OpenModel((int)0x5b, "Data2/Object1/", "cannon_01.smd");
            OpenModel((int)0x5c, "Data2/Object1/", "cannon_02.smd");
            OpenModel((int)0x5d, "Data2/Object1/", "cannon_03.smd");
            OpenModel((int)0x5f, "Data2/Object1/", "badge_01.smd");
            OpenModel((int)0x60, "Data2/Object1/", "signboard_01.smd");
            OpenModel((int)0x61, "Data2/Object1/", "signboard_02.smd");
            OpenModel((int)0x62, "Data2/Object1/", "carriage_01.smd");
            OpenModel((int)0x63, "Data2/Object1/", "carriage_02.smd");
            OpenModel((int)0x64, "Data2/Object1/", "carriage_03.smd");
            OpenModel((int)0x65, "Data2/Object1/", "carriage_04.smd");
            OpenModel((int)0x66, "Data2/Object1/", "straw_01.smd");
            OpenModel((int)0x67, "Data2/Object1/", "straw_02.smd");
            OpenModel((int)0x69, "Data2/Object1/", "waterspout.smd");
            OpenModel((int)0x6a, "Data2/Object1/", "jar_01.smd");
            OpenModel((int)0x6b, "Data2/Object1/", "jar_02.smd");
            OpenModel((int)0x6c, "Data2/Object1/", "jar_03.smd");
            OpenModel((int)0x6d, "Data2/Object1/", "jar_04.smd");
            OpenModel((int)0x6e, "Data2/Object1/", "hanging_01.smd");
            OpenModel((int)0x6f, "Data2/Object1/", "stair_01.smd");
            OpenModel((int)0x73, "Data2/Object1/", "house_01.smd");
            OpenModel((int)0x74, "Data2/Object1/", "house_02.smd");
            OpenModel((int)0x75, "Data2/Object1/", "house_03.smd");
            OpenModel((int)0x76, "Data2/Object1/", "house_04.smd");
            OpenModel((int)0x77, "Data2/Object1/", "house_05.smd");
            OpenModel((int)0x78, "Data2/Object1/", "tent_01.smd");
            OpenModel((int)0x79, "Data2/Object1/", "house_wall_01.smd");
            OpenModel((int)0x7a, "Data2/Object1/", "house_wall_02.smd");
            OpenModel((int)0x7b, "Data2/Object1/", "house_wall_03.smd");
            OpenModel((int)0x7c, "Data2/Object1/", "house_wall_04.smd");
            OpenModel((int)0x7d, "Data2/Object1/", "house_wall_05.smd");
            OpenModel((int)0x7e, "Data2/Object1/", "house_wall_06.smd");
            OpenModel((int)0x7f, "Data2/Object1/", "house_etc_01.smd");
            OpenModel((int)0x80, "Data2/Object1/", "house_etc_02.smd");
            OpenModel((int)0x81, "Data2/Object1/", "house_etc_03.smd");
            OpenModel((int)0x82, "Data2/Object1/", "light_01.smd");
            OpenModel((int)0x83, "Data2/Object1/", "light_02.smd");
            OpenModel((int)0x84, "Data2/Object1/", "light_03.smd");
            OpenModel((int)0x85, "Data2/Object1/", "posebox_01.smd");
            OpenModel((int)0x8c, "Data2/Object1/", "furniture_01.smd");
            OpenModel((int)0x8d, "Data2/Object1/", "furniture_02.smd");
            OpenModel((int)0x8e, "Data2/Object1/", "furniture_03.smd");
            OpenModel((int)0x8f, "Data2/Object1/", "furniture_04.smd");
            OpenModel((int)0x90, "Data2/Object1/", "furniture_05.smd");
            OpenModel((int)0x91, "Data2/Object1/", "furniture_06.smd");
            OpenModel((int)0x92, "Data2/Object1/", "furniture_07.smd");
            OpenModel((int)0x96, "Data2/Object1/", "candle.smd");
            OpenModel((int)0x97, "Data2/Object1/", "beer_01.smd");
            OpenModel((int)0x98, "Data2/Object1/", "beer_02.smd");
            OpenModel((int)0x99, "Data2/Object1/", "beer_03.smd");
        }
        // Desviación: carga explícita de los BMD de Object1. El bloque SMD de arriba
        // está gated por `DAT_0055a7c4 == 0`, que en nuestro build siempre es 1, y la
        // distribución sólo trae BMDs (House01.bmd, Tree01.bmd, Bridge01.bmd, etc.):
        // acá se mapea cada slot SMD a su BMD equivalente.
        struct LorenciaSlot { int slot; const char* bmd; };
        static const LorenciaSlot lorenciaSlots[] = {
            // Trees (slots 0x00..0x0c → Tree01..Tree13)
            { 0x00, "Tree01" }, { 0x01, "Tree02" },
            { 0x02, "Tree03" }, { 0x03, "Tree04" }, { 0x04, "Tree05" },
            { 0x05, "Tree06" }, { 0x06, "Tree07" }, { 0x07, "Tree08" },
            { 0x08, "Tree09" }, { 0x09, "Tree10" }, { 0x0a, "Tree11" },
            { 0x0b, "Tree12" }, { 0x0c, "Tree13" },
            // Grass (0x14..0x19 → Grass01..Grass06)
            { 0x14, "Grass01" }, { 0x15, "Grass02" }, { 0x16, "Grass03" },
            { 0x17, "Grass04" }, { 0x18, "Grass05" }, { 0x19, "Grass06" },
            // Mushrooms not distributed as BMDs (only OZJ texture)
            // Stones — IDA 0x0050C4D0: object/model IDs 30..34 (0x1e..0x22).
            // The .obj record type is used directly as the Models[] index by
            // Draw_RenderObject (0x004FAE00); these are not file ordinals.
            { 0x1e, "Stone01" }, { 0x1f, "Stone02" }, { 0x20, "Stone03" },
            { 0x21, "Stone04" }, { 0x22, "Stone05" },
            // Statues / Tomb
            { 0x28, "StoneStatue01" }, { 0x29, "StoneStatue02" },
            { 0x2a, "StoneStatue03" },
            { 0x2c, "Tomb01" }, { 0x2d, "Tomb02" }, { 0x2e, "Tomb03" },
            // Fire / Light
            { 0x32, "FireLight01" }, { 0x33, "FireLight02" },
            { 0x34, "Bonfire01" },
            // Merchant animals
            { 0x38, "MerchantAnimal01" }, { 0x39, "MerchantAnimal02" },
            // Doungeon / drum / chest / ship
            { 0x37, "DoungeonGate01" },
            { 0x3a, "TreasureDrum01" }, { 0x3b, "TreasureChest01" },
            { 0x3c, "Ship01" },
            // Steel barred wall variants → SteelWall + SteelDoor in distrib
            { 0x41, "SteelWall01" }, { 0x42, "SteelWall02" },
            { 0x43, "SteelWall03" }, { 0x44, "SteelDoor01" },
            // Town walls → StoneWall in distrib (no plain Wall*.bmd)
            { 0x45, "StoneWall01" }, { 0x46, "StoneWall02" }, { 0x47, "StoneWall03" },
            { 0x48, "StoneWall04" }, { 0x49, "StoneWall05" }, { 0x4a, "StoneWall06" },
            // Castle/city walls → StoneMuWall in distrib (only 4 exist)
            { 0x4b, "StoneMuWall01" }, { 0x4c, "StoneMuWall02" },
            { 0x4d, "StoneMuWall03" }, { 0x4e, "StoneMuWall04" },
            // Bridge / fence
            { 0x50, "Bridge01" }, { 0x55, "BridgeStone01" },
            { 0x51, "Fence01" }, { 0x52, "Fence02" },
            { 0x53, "Fence03" }, { 0x54, "Fence04" },
            // Streetlight / cannon
            { 0x5a, "StreetLight01" },
            { 0x5b, "Cannon01" }, { 0x5c, "Cannon02" }, { 0x5d, "Cannon03" },
            { 0x5f, "Curtain01" },
            // Signboard → Sign in distrib
            { 0x60, "Sign01" }, { 0x61, "Sign02" },
            // Carriage
            { 0x62, "Carriage01" }, { 0x63, "Carriage02" },
            { 0x64, "Carriage03" }, { 0x65, "Carriage04" },
            // Straw / waterspout (Jar01..04 not distributed)
            { 0x66, "Straw01" }, { 0x67, "Straw02" },
            { 0x69, "Waterspout01" },
            // Hanging / stair
            { 0x6e, "Hanging01" }, { 0x6f, "Stair01" },
            // Houses (0x73..0x77 → House01..House05)
            { 0x73, "House01" }, { 0x74, "House02" }, { 0x75, "House03" },
            { 0x76, "House04" }, { 0x77, "House05" },
            { 0x78, "Tent01" },
            // House walls
            { 0x79, "HouseWall01" }, { 0x7a, "HouseWall02" }, { 0x7b, "HouseWall03" },
            { 0x7c, "HouseWall04" }, { 0x7d, "HouseWall05" }, { 0x7e, "HouseWall06" },
            // House etc
            { 0x7f, "HouseEtc01" }, { 0x80, "HouseEtc02" }, { 0x81, "HouseEtc03" },
            // Light / posebox per IDA AccessModel slots
            { 0x82, "Light01" }, { 0x83, "Light02" }, { 0x84, "Light03" },
            { 0x85, "PoseBox01" },
            // Furniture lives in 0x8c..0x92 in the original Object1 table
            { 0x8c, "Furniture01" }, { 0x8d, "Furniture02" }, { 0x8e, "Furniture03" },
            { 0x8f, "Furniture04" }, { 0x90, "Furniture05" }, { 0x91, "Furniture06" },
            { 0x92, "Furniture07" },
            // Beer / Candle / Bonfire
            { 0x96, "Candle01" },
            { 0x97, "Beer01" }, { 0x98, "Beer02" }, { 0x99, "Beer03" },
            // Bird/Fish/Butterfly handled by switch case 0 explicit calls above
        };
        for (size_t i = 0; i < sizeof(lorenciaSlots)/sizeof(lorenciaSlots[0]); ++i) {
            int slotIdx = lorenciaSlots[i].slot;
            char* slotPtr = (char*)((uintptr_t)DAT_05828d58 + 0xbcLL * slotIdx);
            if (*(short*)(slotPtr + 0x22) > 0) continue;  // already loaded (e.g. SMD worked)
            AccessModel(slotIdx, "Data/Object1/", lorenciaSlots[i].bmd, -1);
            // Post-load verification log
            short nMesh   = *(short*)(slotPtr + 0x24);
            short nAction = *(short*)(slotPtr + 0x22);
            short nBone   = *(short*)(slotPtr + 0x26);
        }

        // Register all Object1 slots 0..0x9f
        for (int i = 0; i < 0xa0; i++)
            OpenTexture(i, "Object1/", 0x2600, '\x01');
    } else {
        // Numero de carpeta de objetos.  IDA 0x50C4D0:
        //     v33 = World + 1;
        //     if ( World >= 11 && World <= 16 ) v33 = 12;
        int objFolder = World + 1;
        if (World >= 11 && World <= 16)
            objFolder = 12;

        // Dynamic map: load from per-map object file
        if (DAT_0055a7c4 == '\0') {
            char local_384[32];
            crt_sprintf(local_384, "Data2/Object%d/", objFolder);
            ParserFileHandle = fopen(local_384, "rt");
            if (ParserFileHandle != nullptr) {
                char local_300[256], local_200[256], local_100[256];
                while (ParseNextToken() != 2) {
                    int objIdx = ParserCurrentToken;
                    ParseNextToken(); strncpy(local_300, ParserTokenString, 255);
                    ParseNextToken(); strncpy(local_200, ParserTokenString, 255);
                    ParseNextToken(); strncpy(local_100, ParserTokenString, 255);
                    char pathBuf[32];
                    crt_sprintf(pathBuf, "Data2/Object%d/", objFolder);
                    OpenModel(objIdx, pathBuf, local_300);
                }
                fclose(ParserFileHandle);
            }
        }
        // Register data paths for all 0xa0 object slots
        char local_384[32];
        crt_sprintf(local_384, "Data/Object%d/", objFolder);
        for (int i = 0; i < 0xa0; i++)
            AccessModel(i, local_384, "Object", i + 1);
        SetMaxTextures(0x2ee);
        crt_sprintf(local_384, "Object%d/", objFolder);
        for (int i = 0; i < 0xa0; i++)
            OpenTexture(i, local_384, 0x2600, '\x01');
        // Map-specific post-load fixups
        if (World == 1)
            *(unsigned int *)(*(int *)(DAT_05828d58 + 0x1d90) + 0x14) = 0x3ecccccd; // 0.4f
        else if (World == 8) {
            *(unsigned char *)(DAT_05828d58 + 0x89c)  = 0;
            *(unsigned char *)(DAT_05828d58 + 0x958)  = 0;
            *(unsigned char *)(DAT_05828d58 + 0xa14)  = 0;
            *(unsigned char *)(DAT_05828d58 + 0x3624) = 0;
            *(unsigned char *)(DAT_05828d58 + 0x379c) = 0;
            *(unsigned char *)(DAT_05828d58 + 0x3a8c) = 0;
        }
    }

    if (DAT_083a410c != '\0')
        DAT_0055a7c4 = cVar2;
}

// Font helpers (called from OpenFont)
