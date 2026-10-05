# Historial de comentarios: `src/Terrain/`

Comentarios de desarrollo movidos desde `src/Terrain/` según el criterio de
[README.md](README.md). El texto está copiado tal cual; la línea indica
dónde estaba en `fase/1` antes de esta limpieza.

## `src/Terrain/Terrain.cpp`

### Línea 395 — antes de `void __cdecl Terrain_Clear(void) {`

```cpp
// =============================================================================
// 2026-05-07 B3 refactor — Terrain helpers
// moved from stubs.cpp lines 9775-10173 (399 lines).
// =============================================================================
// ── Terrain helpers ───────────────────────────────────────────────────────────
// InitTerrainMappingLayer @ 0x004F6C60 — Terrain_Clear: resets tile/height/noise arrays.
//
// BUG-FIX 2026-04-28: el decomp Ghidra usaba `(int)&DAT_xxxx + iVar2` y
// `*(float*)(iVar2 * 4 + 0x810b2c8)` — accesos por dirección absoluta /
// pointer-arith fuera del símbolo. Los símbolos eran DWORDs de 4 bytes en
// nuestra globals.cpp, así que escribir [65535] desbordaba a globals
// adyacentes y eventualmente AV en el último write (0x810B2CC). Cambiamos
// los símbolos a arrays reales (globals.cpp) y este loop a indexación normal.
//
// Layout (256x256 tile grid = 65536 entries):
//   TerrainMappingLayer1[i] = 0     (TileTex1 byte)
//   TerrainMappingLayer2[i] = 0xFF  (TileTex2 byte)
//   TerrainMappingAlpha[i] = 0.0f  (TerrainHeight)
//   DAT_0810b2cc[i] = (rand() & 3) * scale  (TerrainNoise per-tile UV jitter)
// IDA: FUN_004F6C60
```

### Línea 425 — antes de `void __cdecl CreateTerrainNormal(void) {`

```cpp
// FUN_004f70b0 @ 0x004F70B0 — CreateTerrainNormal (per IDA decomp).
// Computes per-vertex normals for a 256x256 terrain grid using face-cross of
// 3 adjacent height samples; output is DAT_07feb288 (TerrainNormal, vec3 per
// vertex, 65536 vertices).
//
// BUG-FIX 2026-04-26: el Ghidra-decomp declaraba 9 locals separadas (local_c..
// local_24) y las pasaba como `&local_c, &local_18, &local_24` asumiendo
// contigüidad de stack — MSVC no garantiza ese layout. Reemplazado por arrays
// vec3 reales. Mismo patrón que Camera_BuildMouseRay / FUN_004fad60.
//
// BUG-FIX 2026-04-28: el bound original `pfVar5 < 0x80ab288` era una dirección
// absoluta del binario (en IDA = &SelectXF, símbolo siguiente a TerrainNormal).
// En nuestro proceso esa addr no aplica → loop corría fuera del array → AV.
// Cambiado a un loop count-based (256 outer × 256 inner = 65536 vertices).
// IDA: FUN_004F70B0
```

### Línea 497 — antes de `uint __cdecl OpenTerrainHeight(char *filename)`

```cpp
// FUN_004f7290 @ 0x004F7290 — OpenTerrainHeight(filename)
// Per IDA decomp (raw/004F7290_OpenTerrainHeight.c, 611 bytes).
//
// Lee TerrainHeight.bmp (66616 bytes = 1080-byte BMP header + 256x256 pixel
// bytes = 65536 bytes; total con padding = 66616). Convierte cada byte en
// `BackTerrainHeight[y*256+x] = byte * 1.5` y guarda el header en BMPHeader.
//
// Modos:
//   - DAT_0055a7c4 == 0 (plain):     "Data2/<filename>"  ej "Data2/World3/TerrainHeight.bmp"
//   - DAT_0055a7c4 != 0 (compressed): "Data/<name_no_ext>.ozb"
//
// BUG-FIX 2026-04-28: el port previo construía paths con globals incorrectas
// (DAT_0055a7a4/79c/98 son otros símbolos), así que fopen siempre fallaba.
// Y el sprintf de error olvidaba pasar `FileName` como arg → MessageBox
// mostraba bytes de stack ("é)] file not found"). Reescrito siguiendo IDA.
// IDA: FUN_004F7290
```

## `src/Terrain/Terrain_LegacyLoad.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.
```

### Línea 10 — antes de `void __fastcall Cloth_Integrate(int*, float);`

```cpp
// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// Cloth_Integrate vive ahora en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp; antes se definian en este archivo.
```

### Línea 48 — antes de `// OpenTerrainMapping @ 0x004F6F90 — Terrain_LoadMap(path)`

```cpp
// Terrain / map loaders (called from OpenWorld / Map_LoadResources in stubs.cpp)
```

### Línea 50 — antes de `void __cdecl OpenTerrainMapping(const char *path) {`

```cpp
// OpenTerrainMapping @ 0x004F6F90 — Terrain_LoadMap(path)
// Reads map file: skips 1 byte, copies 0x4000×4 bytes to TerrainMappingLayer1 (tile map),
// next 0x4000×4 bytes to TerrainMappingLayer2 (alt-tile), then 0x10000 height bytes → TerrainMappingAlpha as float.
//
// BUG-FIX 2026-05-01: el archivo `EncTerrain%d.map` está ENCRIPTADO con el mismo
// BuxConvert_1 (3-byte XOR rolling) que usa OpenTerrainAttribute (.att). Sin
// descifrarlo, los bytes raw del file se interpretaban como tile-texture-IDs
// y heights → suelo render como mosaico de UI textures con quads de altura
// infinity (causa el triángulo cyan gigante). Aplicar BuxConvert_1 antes de parsear.
```

### Línea 79 en `OpenTerrainMapping` — antes de `char *p = buf + 2;`

```cpp
    // BUG-FIX 2026-05-01 (v3): formato Enc tiene BYTE EXTRA de version flag.
    // Verificación: archivo .map size = 0x30002 = 1(magic) + 1(version) + 3*0x10000(data).
    // Verificación: archivo .obj size = 64324 = 1+1+2(count short)+30*2144 → count=0x0860.
    // El parser 0.85 leía desde buf+1; en archivos Enc hay que leer desde buf+2.
```

### Línea 95 — antes de `unsigned char* TerrainWall = (unsigned char*)&DAT_0838bc70;`

```cpp
// BUG-FIX 2026-04-27: previously a no-op (signature was void, path
// param lost). Now properly loads the .att file via the same FUN_0054xxxx
// pipeline used by the other terrain loaders.
```

### Línea 111 en `OpenTerrainAttribute` — antes de `char dbg[260];`

```cpp
        // 2026-05-04: silent fail — el caller (stubs.cpp:1922) prueba dos
        // formatos (Terrain*.att y EncTerrain*.att). MessageBox bloqueante +
        // WM_DESTROY harían imposible el fallback. Loggear y retornar.
```

### Línea 156 en `OpenTerrainAttribute` — antes de `DbgLogPublic("OpenTerrainAttribute: validation Error (magic-byte mismatch or byte>=0x80)")`

```cpp
        // 2026-05-04: silent fail — la valid magic-byte check del IDA original
        // mata el proceso si fallía. En nuestro build preferimos seguir con
        // walkable=0 implícito (mejor que crash duro).
```

### Línea 167 — antes de `void __cdecl OpenObjectsEnc(const char *path) {`

```cpp
// OpenObjectsEnc @ 0x004FFE70 — Terrain_LoadObjects(path)
// Reads .obj file: 2-byte count, then count×0x1e entries → calls CreateObject for each.
//
// BUG-FIX 2026-05-01: el archivo `EncTerrain%d.obj` está ENCRIPTADO (mismo
// BuxConvert_1 3-byte XOR rolling key que .att). Sin descifrar, count y posiciones
// son basura → no se spawnean instancias de objetos del mundo (casas, NPCs
// estáticos, props) → mapa renderiza solo terreno + hero.
```

### Línea 177 en `OpenObjectsEnc` — antes de `char Text[256];`

```cpp
        // CRITICAL BUG-FIX 2026-05-08: previously wrote the error string into
        // `(char*)&DAT_083a0218` — the bucket-grid cell[0] start in our build.
        // IDA's original used a stack-local `char Text[256]` that Ghidra
        // mis-decompiled as the global symbol. Writing "File not found: %s"
        // there overwrote cell[0].head/tail with garbage like 0x656c6946
        // ("File"), turning the bucket walker into a deref-into-unmapped
        // memory fault on the next frame (the AV chain
        // Object_MoveUpdate → MoveObjects → FUN_004fdc00 → Alpha).
```

### Línea 202 en `OpenObjectsEnc` — antes de `int maxByBuf = ((int)sz - 4) / 30;`

```cpp
    // BUG-FIX 2026-08-17: el guard era `count > 0 && count < 5000`, un tope
    // inventado por el port (IDA 0x4FFE70 sólo chequea `> 0`). Los conteos
    // reales del 0.97k son Lorencia 2870, Dungeon 4488, Atlans 5205,
    // LostTower 5380 y Noria 9399 — o sea el cap descartaba el archivo ENTERO
    // en los tres últimos y esos mapas quedaban sin un solo objeto (paredes,
    // puentes, props). Ahora el bound sale del tamaño real del buffer, que es
    // lo único que hace falta para no leer fuera: el header son 4 bytes y cada
    // entrada 30 (verificado: 4 + 30*count == filesize exacto en los 5 mapas).
```

### Línea 217 en `OpenObjectsEnc` — antes de `CreateObject((int)*p, pos, tgt, *(float*)(p + 0xd));`

```cpp
            // BUG-FIX 2026-05-03: el 4° arg de CreateObject es `float param_4`
            // (la SCALE del objeto en el .obj). Antes leíamos como `*(unsigned int*)`
            // y la conversión implícita int→float convertía el bit pattern de 1.0f
            // (= 0x3F800000 = 1065353216) en el float 1065353216.0f literal →
            // scale gigante → vertices transformados fuera del frustum → invisible.
            // El IDA original lee como `*(float*)` (bit-cast) preservando los bits.
```

### Línea 229 — antes de `void __cdecl OpenTerrainLight(const char *path) {`

```cpp
// BUG-FIX 2026-04-28: el decomp pasaba la dirección absoluta hardcodeada
// 0x7eeb238 que en el binario original es DAT_07eeb238. En nuestro proceso
// esa dirección no existe → AV al escribir. Ahora pasamos &DAT_07eeb238,
// que es el array real.
```

### Línea 251 — antes de `void __cdecl ClearItems(void) {`

```cpp
// ClearItems @ 0x00502B80 — ClearItems / Map_InitEntities
// Clears the "alive" flag (offset 0) for every slot in the GroundItem pool.
// Pool is at DAT_07e12840, 1000 slots × 0x204 bytes.
//
// BUG-FIX 2026-04-28: el decomp Ghidra hardcodeaba la dirección absoluta
// del binario original (0x07E12840 .. 0x07E907E0). En nuestro proceso esa
// dirección no existe → AV. Indexamos el array real ahora que está en
// globals.cpp con tamaño correcto.
// 2026-08-21: limpiaba el offset 0 de cada slot.  IDA arranca en
// `&Items[0][72]` — el flag activo vive en ip+72, que es el que leen
// Net_Process (0x20), Entity_Render y MoveItems.  O sea ClearItems no borraba
// nada y los items del mapa anterior seguían "vivos" al cambiar de zona.
```

### Línea 395 en `OpenWorldModels` — antes de `for (int i = 0xb6; i < 0xbf; i++) {`

```cpp
        // BUG-FIX 2026-08-17: el basename era "Object8" → pedía Object802..Object810,
        // que no existen; los 9 peces de Atlans no cargaban. IDA 0050C4D0 L171:
        //   AccessModelWithTextures(v3, "Data\Object8\", "Fish", v3 - 180)  para v3 = 182..190
```

### Línea 402 en `OpenWorldModels` — antes de `{`

```cpp
        // BUG-FIX 2026-08-17: faltaba entero el bloque de texturas de agua de
        // Atlans (IDA L175-199). Carga wt00..wt31 en Bitmaps[65..96] y además
        // copia el nombre corto en Bitmaps[n].FileName (offset 0 del slot,
        // stride 0x38), que es de donde lo lee el render de tiles de agua.
        // El "if (v5 >= &Bitmaps[75])" del decompile es simplemente v4 >= 10:
        // wt00..wt09 llevan cero a la izquierda, wt10..wt31 no.
```

### Línea 445 en `OpenWorldModels` — antes de `AccessModel(0xb8, "Data/Object12/", "Crow", 1);`

```cpp
        //
        // 2026-09-04 FIX, tres cosas:
        //  a) el slot 184 pedia "Angel01.bmd", que no existe; es "Crow01.bmd".
        //  b) los slots 262/263 (LA PUERTA del evento) pedian
        //     "gate_entrance01/02.bmd", que tampoco existen: son "Gate01/02.bmd".
        //     Con el BMD sin cargar el modelo queda con 0 mallas, y romper la
        //     puerta terminaba trabajando sobre esa entrada vacia.
        //  c) faltaban las cuatro OpenTexture de la puerta y los sarcofagos,
        //     que salen de "Monster/" y no de "Object12/".
```

### Línea 475 en `OpenWorldModels` — antes de `OpenJPG("Effect/clouds.jpg", 0x4f4, 0x2601, 0x2900, 0, 1);`

```cpp
        // 2026-09-04 FIX: estas dos estaban en el PRIMER switch, que va dentro de
        // `if (DAT_0055a7c4 == 0)` -- el gate de "primera carga de mundo".  Como
        // cualquier mapa anterior ya deja ese flag en 1, en Blood Castle no corrian.
        // IDA las tiene en ESTE switch, fuera del `if (!unk_55A7C4)` interno
        // (0x50C4D0 L243-245), o sea se ejecutan en cada entrada al mapa.
        // Sin el LoadWaveFile el `PlayBuffer(110, 0, 1)` del estado 0 del 0x9B no
        // tenia nada que reproducir: por eso no sonaba la musica del evento.
```

### Línea 609 en `OpenWorldModels` — antes de `struct LorenciaSlot { int slot; const char* bmd; };`

```cpp
        // BUG-FIX 2026-05-04: agregar load explícito de BMDs Object1.
        // El bloque SMD arriba está gated por `DAT_0055a7c4 == 0` que en nuestro
        // build SIEMPRE es 1 (default = Data mode, no Data2/), así que las SMDs
        // nunca se cargaban. Como la distribución solo trae BMDs con nombres
        // PascalCase (House01.bmd, Tree01.bmd, Bridge01.bmd, etc.), aquí mapeamos
        // explícitamente cada slot SMD a su BMD equivalente.
```

### Línea 708 en `OpenWorldModels` — antes de `int objFolder = World + 1;`

```cpp
        // 2026-09-04 FIX: faltaba el override.  Los seis niveles de Blood Castle
        // (World 11..16) COMPARTEN Data/Object12; con `World + 1` los niveles 2 a 7
        // buscaban Object13..Object17, que no existen -- de ahi que el mapa
        // apareciera pelado, sin paredes ni props.  El nivel 1 (World 11 -> 12)
        // acertaba de casualidad.
```

### Línea 765

```cpp
// Font helpers (called from OpenFont in stubs.cpp)
```

## `src/Terrain/Terrain_LegacyRender.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
```

### Línea 46 en `Terrain_RenderQuad` — antes de `if (DAT_07eab24c == 0 || (uintptr_t)DAT_07eab24c < 0x100000) return;`

```cpp
    // BUG-FIX 2026-04-29: guard contra DAT_07eab24c (BackTerrainHeight) no
    // inicializado. Crash AV en
    // 0x410E4597 venía de cursor billboard RenderTerrainAlphaBitmap dereferenciando
    // el buffer NULL.
```

### Línea 79 en `Terrain_RenderQuad` — antes de `float *src = &DAT_081cb608[indices[v] * 3];`

```cpp
            // 2026-08-23: leia DAT_07eab250, que es un DWORD muerto y NO es
            // PrimaryTerrainLight (ver globals.h:837).  El buffer real es
            // DAT_081cb608, el mismo que resetea Terrain_Water por frame.
```

### Línea 170

```cpp
//
// 2026-09-25: aca habia un STUB NO-OP con el mismo nombre y un TODO de 6 pasos,
// mientras la implementacion completa ya existia bajo el nombre RenderTerrainAlphaBitmap.
// Los 6 call sites que llamaban por el nombre real -- las particulas de terreno
// (tipos 1191/1200/1264), el reflejo del agua y los decals -- ejecutaban el
// no-op; solo el cursor del mouse, que llamaba al FUN_, veia la implementacion.
```

## `src/Terrain/Terrain_Light.cpp`

### Línea 48 en `RequestTerrainLight` — antes de `pfVar5 = (float *)((char*)&DAT_081cb608 + ((iVar9 + 1) * 0x100 + iVar4) * 0xc);`

```cpp
      // BUG-FIX: DAT_081cb608 es DWORD → &DAT_081cb608 + N*0xc hace aritmética
      // DWORD* (= +N*0xc*4 = +N*48 bytes). Disasm @ 0x004f7a13-28 muestra
      //   LEA <reg>,[<idx>*0x4 + 0x81cb608]  donde <idx> ya viene * 3
      // → byte offset = idx*3*4 = idx*12. Castear base a char* para byte arith.
```

### Línea 90 en `Entity_GetLightScale` — antes de `float rgb[3] = {0.0f, 0.0f, 0.0f};`

```cpp
  // PORT FIX: Ghidra decompile produced three separate locals (local_c/8/4)
  // where the original binary had a contiguous float[3] on the stack.
  // RequestTerrainLight writes 3 floats starting at its output pointer, so the
  // locals MUST be contiguous. In MSVC, separate `float` declarations are
  // NOT guaranteed to be adjacent — so local_8/local_4 ended up reading
  // uninitialised stack slots, producing huge/subnormal values that were
  // added to the entity tint (+0xe8/+0xec/+0xf0) and written to the model's
  // bodyLight (+0x48/+0x4c/+0x50). Logo01/Logo03 at the login scene rendered
  // as corrupted coloured triangles because of this.  Using a proper float[3]
  // array guarantees contiguity and eliminates the uninitialised reads.
```

## `src/Terrain/Terrain_RayCollision.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Extracted from stubs.cpp; IDA provenance comments retained.
```

### Línea 11 — antes de `unsigned int __cdecl CollisionDetectLineToFace(float *Position, float *Target, int Polygon`

```cpp
// ── Terrain tile outline / ray-triangle intersection stubs ───────────────────
// RenderTerrainFace (RenderTerrainFace) — PORTADO 1:1 en src/Terrain/Terrain_RenderFace.cpp
// (antes era un no-op stub de 4 args; la firma real es 5 args con lodf).
// BUG-FIX 2026-04-26 (audit #7): activated full IDA port.  Old stub returned 0
// always, so terrain triangle picking *never* registered a hit — click-to-move
// would only land on whatever fallback path remained.  The dormant gated port
// (formerly behind IDA_PORT_00512D40) is now the live implementation.
```

### Línea 96

```cpp
// ════════════════════════════════════════════════════════════════════
// IDA HEX-RAYS PORTS reference block (307 IDA-only gated functions, ~29k lines)
// moved to src/stubs_IDA_ports.cpp (B3 refactor 2026-05-07).
// All functions there are gated by IDA_PORT_xxxxxxxx macros; none are
// active in the default build.
// ════════════════════════════════════════════════════════════════════
```

## `src/Terrain/Terrain_RenderBlocks.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Terrain_RenderBlocks.cpp
//
// Extracted from stubs_game.cpp.  Owns terrain block culling/render and the
// clipped dynamic-light variant.  Function comments retain IDA provenance.
```

### Línea 9 — antes de `void __cdecl AddTerrainLightClip(float xf, float yf, float Light[3], int Range, float Buff`

```cpp
// AddTerrainLightClip @ 0x004F7800 (~74 lines) — adds clamped light to terrain buffer
// Iterates a square region around (xf,yf). Per cell: falloff = (Range-dist)/Range.
// Adds Light * falloff to Buffer, clamps to [0.0, 1.0].
// AddTerrainLightClip (0x004F7800).
//
// 2026-08-23: quedo SIN CALLERS en nuestro arbol, y es correcto que asi sea — no
// es codigo muerto para borrar.  Hasta hoy `structs.h` aliaseaba
// `AddTerrainLight` (0x4F76C0) a esta funcion, y por eso toda la luz dinamica
// quedaba clampeada a 1.0.  En el binario esta variante tiene UN solo caller
// (0x4C0E59, dentro de una funcion que todavia no portamos); cuando se porte,
// debe llamar a esta y no a AddTerrainLight.
//
// Diferencia entre las dos: esta clampea a [0, 1]; 0x4F76C0 solo evita negativos
// y deja que la luz supere 1.0 (que es lo que produce el resplandor del fuego).
```

## `src/Terrain/Terrain_RenderFace.cpp`

### Línea 1 — antes de `#include "stdafx.h"`

```cpp
// Terrain_RenderFace.cpp — port 1:1 desde IDA (2026-06-27)
// Cadena de dibujo de tiles texturados del terreno (eslabón #4-#5 de RenderTerrain):
//   RenderTerrainTile → RenderTerrainFace (RenderTerrainFace) →
//   FaceTexture (UVs) + RenderFace / RenderFaceAlpha / RenderFaceBlend (draw).
//
// Antes: RenderTerrainFace era un no-op stub (stubs.cpp) y las 4 primitivas no existían,
// por eso el dev previo escribió una fallback flat-shaded en RenderTerrain.
//
// Direcciones/buffers verificados por bytes de operando en IDA:
//   FaceTexture        0x004F7DF0   RenderFace        0x004F7A90
//   RenderFaceAlpha    0x004F7B80   RenderFaceBlend   0x004F7CE0
//   RenderTerrainFace  0x004F7FB0   BindTexture       0x00511480 (GL_BindTextureSlot)
//   Bitmaps base       0x083A7CA0 (stride 0x38, BITMAP_t: Width@0x20 Height@0x24)
//   TerrainMappingAlpha  = TerrainMappingAlpha (float[256²])   (mislabel "TerrainHeight")
//   TerrainMappingLayer1 = TerrainMappingLayer1 (BYTE[256²])
//   TerrainMappingLayer2 = TerrainMappingLayer2 (BYTE[256²])
//   PrimaryTerrainLight  = DAT_081cb608 (float[256²][3])  ← 0x081CB608 (NO 0x07eab250;
//                          el macro PrimaryTerrainLight de structs.h apunta mal a
//                          0x07eab250 — buffer muerto. Lo leímos directo de DAT_081cb608,
//                          que Terrain_Water puebla per-frame desde BackTerrainLight 0x0828b608).
//   TerrainVertex        = g_TilePickBuf[12]   (4 corners contiguos)
//   TerrainTextureCoord  = g_TerrainTexCoord[8] (4 UV pairs)
//   TerrainGrassWind     = DAT_07eab200 (float[256²])     (mislabel "water-wave heights")
//   TerrainGrassTexture[(yi&0xFF)+1] = DAT_0810b2cc[yi&0xFF]
//   WaterMove=DAT_07eeb214  WaterTextureNumber=DAT_0839bc8c  CurrentLayer=DAT_0814b2dc
//   TerrainFlag=DAT_0838bc44  unk_839BC86=DAT_0839bc86  World
//   TerrainIndex1..4 = DAT_07eab1ec/f0/f4/f8
```

### Línea 136 en `RenderTerrainFace` — antes de `if (TER_ALPHA[TER_IDX1] <= 0.0f && TER_ALPHA[TER_IDX2] <= 0.0f &&`

```cpp
        // ── capa de billboards de pasto/arena, movida por el viento ────────
        // 2026-08-23: estuvo inerte porque `unk_55A76C` (DAT_0055a76c) estaba
        // inicializado en 0.  Nadie lo escribe, pero es constante de .data y en
        // el binario vale 1 — con 0 esta pasada no corria en ningun mapa y se
        // perdia el pasto de Lorencia/Noria y la arena volando de Tarkan.
```

## `src/Terrain/Terrain_Utils.cpp`

### Línea 31 — antes de `float __cdecl RequestTerrainHeight(float xf, float yf)`

```cpp
// Per IDA decomp (raw/004F7500_RequestTerrainHeight.c, 177 bytes).
//
//
// BUG-FIX 2026-04-28: el Ghidra decomp inferiría argumentos vía x87 FPU stack
// (`__ftol()` lee ST0/ST1) — eso solo funciona si el caller compiló con x87,
// pero MSVC en Release usa SSE/SSE2 → ftol leía basura → return 0 → todos los
// hero/entity quedaban con z=0 (heroPos.z=0.0 en el log). Cambiamos la firma a
// (xf, yf) explícitos como el IDA original y actualizamos los call-sites.
```

### Línea 43 en `RequestTerrainHeight` — antes de `if ((int)World < 0) return 0.0f;`

```cpp
    // BUG-FIX 2026-04-28: el IDA original tiene guard `if (SceneFlag != 5)`
    // pero Recv_JoinMapServer llama CreateCharacterPointer ANTES de que el state
    // pase a 5 (en MuEmu el F3/03 llega rápido y el state machine está aún en 3
    // o 4). Resultado: hero.z spawn = 0. Relajamos el guard — ahora es seguro
    // mientras el world esté cargado (World válido y DAT_080cb2cc con
    // height map real). Si el array está en 0 retornamos 0 (mismo resultado).
```
