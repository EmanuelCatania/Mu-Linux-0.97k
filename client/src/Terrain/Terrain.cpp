// Terrain.cpp
// Sistema de carga y renderizado de terreno — mapas del mundo
//
// Cubre:
//   Terrain_Clear          @ 0x004f6c60  (25 líneas)
//   Terrain_GetTileIndex   @ 0x004f6c40  (7 líneas)
//   Terrain_Load           @ 0x004f6f90  (57 líneas) — carga .att
//   TerrainHeight_Load     @ 0x004f7290  (215 líneas) — carga height + texture palette
//   Terrain_ComputeNormals @ 0x004f70b0  (52 líneas)
//   Terrain_ComputeLighting @ 0x004f71c0 (38 líneas)
//   TerrainObjects_Render  @ 0x004f7060  (18 líneas)
//   Terrain_HeightAtPos    @ 0x004f7500  (40 líneas) — interpolación bilineal
//   TerrainTile_Render     @ 0x004f7fb0  (101 líneas) — renderiza 1 tile
//   TerrainTile_Draw       @ 0x004f7a90  (25 líneas)  — quad OpenGL
//   Terrain_FrameUpdate    @ 0x004f95e0  (83 líneas)  — sun cycle + water anim
//   Terrain_Render         @ 0x004fd800  (250 líneas) — loop principal
//
// ── ESTRUCTURA DEL MAPA ───────────────────────────────────────────────────────
//
//   Mapa: 256 × 256 tiles = 65.536 celdas
//   Tile index: Terrain_GetTileIndex(x, y) = (y & 0xFF) * 0x100 + (x & 0xFF)
//   Tile (0,0) = esquina noroeste; tile (255,255) = sureste
//
//   Coordenadas mundo:
//     world_x = (grid_x + 0.5) * tile_size   (tile_size = DAT_005524f0)
//     world_y = (grid_y + 0.5) * tile_size
//
//   Tamaño total del mapa: 256 * tile_size unidades de mundo
//
// ── ARRAYS GLOBALES DE TERRENO ────────────────────────────────────────────────
//
//   Nombre           | Address    | Tipo           | Contenido
//   ─────────────────┼────────────┼────────────────┼──────────────────────────────
//   DAT_080bb2b4     | 0x80BB2B4  | byte[65536]    | texture_type_id por tile
//   DAT_080ab2b4     | 0x80AB2B4  | byte[65536]    | atributo secundario por tile
//   DAT_0834b608     | 0x834B608  | float[65536]   | altura por tile (de .att)
//   DAT_080cb2cc     | 0x80CB2CC  | float[65536]   | altura por tile (de TerrainHeight)
//   DAT_0838b800     | 0x838B800  | uint[270]      | paleta de texturas (IDs de OZJ)
//   DAT_07feb288     | 0x7FEB288  | float[3×65536] | normales por tile (XYZ)
//   DAT_07eeb238     | 0x7EEB238  | float[]        | datos de luz/color ambiental
//   DAT_0828b608     | 0x828B608  | float[3×65536] | color iluminado por tile
//   DAT_081cb608     | 0x81CB608  | float[3×?]     | colores tiles visibles (viewport)
//   DAT_07eab200     | 0x7EAB200  | float[65536]   | offset de ola de agua por tile
//   DAT_0810b2cc     | 0x810B2CC  | float[65536]   | ruido aleatorio por tile (init)
//   DAT_081cb2ed     | 0x81CB2ED  | byte[?*8]      | array de objetos de terreno
//   DAT_083a021c     | 0x83A021C  | ptr[]          | display list de tiles visibles
//   DAT_07eab1ec     | 0x7EAB1EC  | uint           | índice tile esquina TL (actual)
//   DAT_07eab1f0     | 0x7EAB1F0  | uint           | índice tile esquina TR
//   DAT_07eab1f4     | 0x7EAB1F4  | uint           | índice tile esquina BL
//   DAT_07eab1f8     | 0x7EAB1F8  | uint           | índice tile esquina BR
//
// ── FORMATO DE ARCHIVO .ATT (terrain.att) ────────────────────────────────────
//
//   Cargado por Terrain_Load(path). Tamaño típico: ~196.609 bytes.
//
//   Offset      | Tamaño   | Contenido
//   ────────────┼──────────┼────────────────────────────────────────────────────
//   0x00000     | 1 byte   | version byte (ignorado)
//   0x00001     | 65.536 B | tile_attrib1 (256×256 bytes) → DAT_080bb2b4
//   0x10001     | 65.536 B | tile_attrib2 (256×256 bytes) → DAT_080ab2b4
//   0x20001     | 65.536 B | height bytes (256×256) → DAT_0834b608 (× height_scale1)
//
//   Lectura: loops de 0x4000 dwords (4 bytes por dword = 65536 bytes por sección).
//   height_scale1 = DAT_00552b70
//
// ── FORMATO DE ARCHIVO TERRAINHEIGHT ─────────────────────────────────────────
//
//   Cargado por TerrainHeight_Load(filename). Extensión: .OZB o sin extensión.
//   Tamaño típico: 0x10438 = 66.616 bytes.
//
//   Offset      | Tamaño   | Contenido
//   ────────────┼──────────┼────────────────────────────────────────────────────
//   0x000       | 0x438 B  | Texture palette: 0x10E dwords → DAT_0838b800[270]
//   0x438       | 65.536 B | height bytes (256×256) → DAT_080cb2cc (× height_scale2)
//
//   Si DAT_0055a7c4 != 0: skip 4 bytes al inicio (header comprimido).
//   Si DAT_0055a7c4 == 0: aplica FUN_00529130 (decompression/CRC).
//   height_scale2 = DAT_005528f0
//
//   Path: construido a partir de DAT_0055a79c (prefix) + base_name + DAT_0055a798 (suffix)
//         o DAT_0055a7a4 (fixed path cuando DAT_0055a7c4 == 0)
//
// ── TERRAIN_GETTILEDINDEX (0x004f6c40) ────────────────────────────────────────
//
//   int Terrain_GetTileIndex(uint x, uint y):
//     return (y & 0xFF) * 0x100 + (x & 0xFF)
//     // Rango: 0..65535 = 0x0000..0xFFFF
//
// ── TERRAIN_CLEAR (0x004f6c60) ────────────────────────────────────────────────
//
//   void Terrain_Clear():
//     Para cada i in 0..65535:
//       DAT_080bb2b4[i] = 0               // attrib1 = 0 (sin textura)
//       DAT_080ab2b4[i] = 0xFF            // attrib2 = 0xFF (por defecto)
//       DAT_0834b608[i] = 0.0f           // height = nivel cero
//       DAT_0810b2cc[i+1] = (rand() & 3) * DAT_005528dc  // ruido aleatorio
//
//   Nota: DAT_0810b2cc se inicializa con 0..3 * scale → variación de textura.
//
// ── TERRAIN_LOAD (0x004f6f90) ─────────────────────────────────────────────────
//
//   bool Terrain_Load(LPCSTR path):
//     Terrain_Clear()
//     fp = fopen(path, "rb")
//     if (!fp): return false
//
//     filesize = fsize(fp)
//     buffer = new byte[filesize]
//     fread(buffer, 1, filesize, fp)
//     fclose(fp)
//
//     // Sección 1: attrib1 (256×256 bytes como dwords)
//     src = buffer + 1
//     for 0x4000 veces: DAT_080bb2b4[i] = *(dword*)src; src += 4
//
//     // Sección 2: attrib2
//     src = buffer + 0x10001
//     for 0x4000 veces: DAT_080ab2b4[i] = *(dword*)src; src += 4
//
//     // Sección 3: heights → float
//     src = buffer + 0x20001
//     for cada float en DAT_0834b608 (65536 entradas):
//       DAT_0834b608[i] = (float)*src++ * DAT_00552b70
//
//     delete buffer
//     return true
//
// ── TERRAINHEIGHT_LOAD (0x004f7290) ───────────────────────────────────────────
//
//   bool TerrainHeight_Load(char* param_1):
//     Construye path completo desde globals de ruta
//     fp = fopen(path, "rb")
//     if (!fp): MessageBox + error log; return false
//
//     if (DAT_0055a7c4 != 0): fseek(fp, 4, SEEK_SET)  // skip compressed header
//
//     buffer = new byte[0x10438]
//     fread(buffer, 1, 0x10438, fp)
//     fclose(fp)
//
//     // Texture palette
//     src = buffer
//     for 0x10E veces: DAT_0838b800[i] = *(dword*)src; src += 4
//
//     if (DAT_0055a7c4 == 0): FUN_00529130(4, DAT_0055a798, param_1, buffer, 0x10438)
//
//     // Heights → float (256×256 = 65536 entradas)
//     src = buffer + 0x438
//     for row in 0..255:
//       for col in 0..255:
//         DAT_080cb2cc[row*256 + col] = (float)*src++ * DAT_005528f0
//
//     delete buffer
//     return true
//
//   FUN_00529130 @ 0x00529130 = Terrain_Decompress o Terrain_CRC_Check
//
// ── TERRAIN_COMPUTENORMALS (0x004f70b0) ───────────────────────────────────────
//
//   void Terrain_ComputeNormals():
//     Para cada tile (x, y) en 0..255 × 1..255:
//       i  = (x & 0xFF) + (y & 0xFF) * 0x100   // tile actual
//       ir = (x+1 & 0xFF) + (y+1 & 0xFF) * 0x100  // tile diagonal
//
//       Toma 3 alturas de esquinas vecinas (DAT_080cb2cc)
//       Triangle_ComputeNormal(pos_a, pos_b, pos_c, out_normal)  — cross product/normalización
//       → DAT_07feb288[i*3..i*3+2] = (nx, ny, nz)
//
//   DAT_07feb288 = array de normales, float[3] por tile
//   Triangle_ComputeNormal @ 0x004fa4d0 = Vec3_CrossProduct(a, b, c, out)
//
// ── TERRAIN_COMPUTELIGHTING (0x004f71c0) ──────────────────────────────────────
//
//   void Terrain_ComputeLighting():
//     Para cada tile i en 0..65535:
//       // Dot product de normal con dirección de luz + componente ambiental
//       light = (normal.x + normal.y) * 0.5f - normal.z * 0.5f + 0.5f
//       light = clamp(light, DAT_00552580, DAT_0055256c)  // [0.0, 1.0]
//
//       // Multiplica por color del canal de luz
//       DAT_0828b608[i*3+0] = light * DAT_07eeb238[i*3+0]  // R
//       DAT_0828b608[i*3+1] = light * DAT_07eeb238[i*3+1]  // G
//       DAT_0828b608[i*3+2] = light * DAT_07eeb238[i*3+2]  // B
//
//   Input: DAT_07feb288 (normales), DAT_07eeb238 (color/dirección de luz)
//   Output: DAT_0828b608 (color iluminado por tile)
//
// ── TERRAIN_FRAMEUPDATE (0x004f95e0) ──────────────────────────────────────────
//
//   void Terrain_FrameUpdate():
//     // 1. Copia colores iluminados al buffer de viewport actual
//     Para tiles (x,y) en cámara [DAT_0839bc90..DAT_0839bc94+3]:
//       DAT_081cb608[idx*3..+2] = DAT_0828b608[tile_idx*3..+2]
//
//     // 2. Calcula ángulo solar (ciclo día/noche)
//     if (DAT_083a3ff0 == 0):
//       sun_cycle = 720.000 ms total     // día largo
//     else:
//       sun_cycle = 36.000 ms total      // día rápido (evento)
//     sun_angle = (GetTickCount() % cycle) * angulo_step
//
//     // 3. Anima agua (ondas sinusoidales)
//     Para tiles visibles en viewport:
//       DAT_07eab200[tile_idx] = sin(tile_col * DAT_00552660 + sun_angle) * DAT_00552488
//       (Para g_GameSubState==8 (Devias?): usa DAT_00552598 en vez de DAT_00552660)
//
//   DAT_0839bc90  — tile X cámara (columna izquierda visible)
//   DAT_0839bc94  — tile Y cámara (fila superior visible)
//   DAT_083a3ff0  — modo ciclo día/noche (0=normal, 1=rápido)
//   DAT_07eab200  — water wave offsets (modifica altura Z del vertex de agua)
//
// ── TERRAIN_HEIGHATATPOS (0x004f7500) ─────────────────────────────────────────
//
//   float Terrain_HeightAtPos(float world_x, float world_y):
//     // Solo válido con g_GameState == 5
//     col = (int)world_x  (tile column)
//     row = (int)world_y  (tile row)
//     frac_x = world_x - col
//     frac_y = world_y - row
//
//     // Lee 4 esquinas del tile (DAT_080cb2cc)
//     h00 = DAT_080cb2cc[col + row * 256]
//     h10 = DAT_080cb2cc[(col+1) + row * 256]
//     h01 = DAT_080cb2cc[col + (row+1) * 256]
//     h11 = DAT_080cb2cc[(col+1) + (row+1) * 256]
//
//     // Bilinear interpolation
//     top    = h00 + (h10 - h00) * frac_y
//     bottom = h01 + (h11 - h01) * frac_y
//     return top + (bottom - top) * frac_x
//
//   Usada para: colisión cámara con terreno, entidades montadas, proyectiles
//
// ── TERRAINTILE_RENDER (0x004f7fb0) ───────────────────────────────────────────
//
//   void TerrainTile_Render(float tile_x, float tile_y, uint tile_param, uint rand_seed):
//     // DAT_0838bc44: modo de renderizado (2 = agua, otros = terreno normal)
//
//     Si DAT_0838bc44 == 2 (agua):
//       Si las 4 alturas del tile <= DAT_00552580 (= bajo el agua):
//         Si DAT_0814b2dc == 0 && no PvP sub-estado:
//           texture_id = DAT_080bb2b4[tile_idx] + 0x32  // textura de agua
//           GL_BindTextureSlot(texture_id)
//           glTexCoord2f(uv_x, ...)
//           // Agrega offset de ola (DAT_07eab200) a los vértices Y
//           glVertex3fv(vertex + wave_y)
//           glEnd(GL_QUADS)
//
//     Sino (terreno normal):
//       Si alguna altura del tile < 0 (borde agua-tierra):
//         texture_id = DAT_080bb2b4[tile_idx]  // textura desde attrib1
//         Si attrib1 == 5: es_borde_agua = true
//       Sino:
//         texture_id = DAT_080ab2b4[tile_idx]  // usa attrib2
//
//       TerrainTile_SetupVertices(texture_id, tile_x, tile_y, es_borde_agua, 0)
//       TerrainTile_Draw(texture_id)
//
//       Si g_GameSubState==7 (Atlans bajo el agua) && attrib2==5 && altura>0:
//         // Renderiza superficie del agua sobre el tile
//         animated_idx = DAT_0839bc8c + 0x1E  // frame animado (mod 32 + 30)
//         TerrainTile_SetupVertices(animated_idx, ...)
//         TerrainTile_DrawWater(animated_idx)
//
//       Si tile tiene borde agua-tierra && alguna altura>0:
//         // Renderiza capa de transición tierra-agua
//         texture_id = DAT_080ab2b4[tile_idx]
//         TerrainTile_SetupVertices(texture_id, ...)
//         TerrainTile_DrawTransition(texture_id)
//
//   DAT_080bb2b4 tile attrib1 values known:
//     0..N  = índice de tipo de terreno (referencia a paleta DAT_0838b800)
//     5     = borde de agua (usa lógica especial)
//
//   DAT_0839bc8c = frame index mod 32 (del Game_MainLoop)
//                → tiles de agua usan índice animado 0x1E..0x3D
//
// ── TERRAINTILE_DRAW (0x004f7a90) ─────────────────────────────────────────────
//
//   void TerrainTile_Draw(int texture_id):
//     GL_BindTextureSlot(texture_id + 0x23)   // GL_BindTextureSlot
//     if (!DAT_0839bc86):                 // no wireframe
//       glBegin(GL_QUADS)
//       for each corner (TL, TR, BR, BL):
//         glTexCoord2f(DAT_07feb238[corner], DAT_07feb23c[corner])
//         glColor3fv(&DAT_081cb608[tile_idx * 0xC])  // color iluminado (stride 12 bytes)
//         glVertex3fv(&DAT_07feb258[corner])          // posición 3D
//       glEnd()
//
//   Vertex buffer (precomputado por TerrainTile_SetupVertices):
//     DAT_07feb238/23c/240/244/248/24c/250/254 = UV coords (8 floats)
//     DAT_07feb258/264/270/27c = vertex positions (4 × vec3 = 12 floats)
//
// ── TERRAINOBJECTS_RENDER (0x004f7060) ────────────────────────────────────────
//
//   void TerrainObjects_Render():
//     // Itera array de objetos de terreno (rocks, trees, etc.)
//     obj_ptr = &DAT_081cb2ed
//     while (obj_ptr < 0x81cb60d):
//       if obj_ptr[-5] != 0:    // is_visible flag
//         FUN_004f8980(
//           8,                  // tipo de objeto (8 = static prop)
//           obj_ptr[-1],        // object_type
//           *obj_ptr,           // object_subtype
//           (float)obj_ptr[1] * DAT_00552cb0  // scale factor
//         )
//       obj_ptr += 8            // stride 8 bytes
//
//   Objeto terreno (stride 8 bytes):
//     -5: is_visible (byte)
//     -1: object_type (byte)
//     +0: object_subtype (byte)
//     +1: scale (byte) → float × DAT_00552cb0
//
// ── TERRAIN_RENDER (0x004fd800) ───────────────────────────────────────────────
//
//   void Terrain_Render():
//     color_underwater = (g_GameSubState == 10) ? 0xC1200000 : 0.0f
//
//     Itera 8×8 bloques de tiles (local_20 y local_24):
//       Para cada bloque (tile_x=8, tile_y=8..):
//         uVar3 = FUN_004f8ff0(tile_x, tile_y, -180.0)  // frustum cull
//         Si visible || CameraTopViewEnabled:
//           Para objetos en lista de ese bloque:
//             obj[+0x160] = FUN_004f8ff0(...)  // cull por objeto
//             Si visible: TerrainTile_Render(...)
//             Si g_GameSubState==2 && entity_type==100:
//               // Render entity especial (NPC marker?) con CharData check
//
//   FUN_004f8ff0 @ 0x004f8ff0 = Frustum_TestSphere(x, y, z) → visible
//   CameraTopViewEnabled = force_render_all flag (debug)
//
// ── PIPELINE DE CARGA COMPLETO ────────────────────────────────────────────────
//
//   Al entrar a un nuevo mapa:
//   1. Terrain_Load("WorldN/terrain.att")     → attribs + alturas base
//   2. TerrainHeight_Load("WorldN/height...")  → paleta texturas + alturas finas
//   3. Terrain_ComputeNormals()               → normales por tile
//   4. Terrain_ComputeLighting()              → colores iluminados (bake)
//
//   Cada frame:
//   5. Terrain_FrameUpdate()                  → sun angle + water waves
//   6. Terrain_Render() (desde Game_RenderTick) → cull + draw tiles
//   7. TerrainObjects_Render()                → props estáticos
//
// ── CONSTANTES DE MUNDO ───────────────────────────────────────────────────────
//
//   DAT_005524f0  — tile_size (grid→world): world_x = grid_x * tile_size
//   DAT_00552504  — 0.5f (medio tile, offset de centro)
//   DAT_00552b70  — height_scale1 (para .att): altura en unidades mundo
//   DAT_005528f0  — height_scale2 (para TerrainHeight)
//   DAT_005528dc  — scale_noise (para ruido random, DAT_0810b2cc)
//   DAT_00552cb0  — scale_objects (para objetos estáticos)
//   DAT_00552488  — water_amplitude (amplitud de olas sinusoidales)
//   DAT_00552660  — water_freq (frecuencia de ola, modo normal)
//   DAT_00552598  — water_freq2 (frecuencia alternativa, g_GameSubState==8)
//   DAT_00552580  — 0.0f (nivel mínimo / superficie del agua)
//   DAT_0055256c  — 1.0f (máximo normalizado)
//   DAT_0055a7c4  — compressed_flag (0=no comprimido, 1=comprimido)
//   DAT_0055a7a4  — fixed_path (ruta fija cuando no comprimido)
//   DAT_0055a79c  — world_path_prefix (ej "World1/")
//   DAT_0055a798  — file_suffix (extensión de archivo)
//   DAT_0814b2dc  — disable_water_rendering flag
//   DAT_083a3ff0  — fast_day_cycle flag (0=normal 720s, 1=rápido 36s)
//   DAT_0839bc86  — wireframe_mode flag
//   DAT_0838bc44  — tile_render_mode (2=agua surface)
//
// ── CROSS-REFERENCE ───────────────────────────────────────────────────────────
//
//   Terrain_GetTileIndex   @ 0x004f6c40  — (y*256+x), usado en Combat.cpp
//   Terrain_Clear          @ 0x004f6c60
//   Terrain_Load           @ 0x004f6f90  — carga terrain.att
//   Terrain_ComputeNormals @ 0x004f70b0
//   TerrainObjects_Render  @ 0x004f7060
//   Terrain_ComputeLighting @ 0x004f71c0
//   TerrainHeight_Load     @ 0x004f7290  — carga height + texture palette
//   Terrain_HeightAtPos    @ 0x004f7500  — bilineal interp (usado para colisión)
//   TerrainTile_Draw       @ 0x004f7a90
//   TerrainTile_Render     @ 0x004f7fb0
//   TerrainTile_SetupVertices @ 0x004f7df0
//   TerrainTile_DrawWater  @ 0x004f7b80
//   TerrainTile_DrawWaterSurface @ 0x004f7ce0
//   TerrainObjects_DrawProp @ 0x004f8980
//   Terrain_FrameUpdate    @ 0x004f95e0
//   Terrain_Render         @ 0x004fd800
//   Frustum_TestSphere     @ 0x004f8ff0
//   Vec3_CrossProduct      @ 0x004fa4d0
//   Terrain_Decompress     @ 0x00529130
//   GL_BindTextureSlot         @ 0x00511480
//   Game_RenderTick        @ 0x00525A00  (llama Terrain_Render)

#include "stdafx.h"
#include "Terrain/Terrain.h"

// =============================================================================
// 2026-05-07 B3 refactor — Terrain helpers
// moved from stubs.cpp lines 9775-10173 (399 lines).
// =============================================================================
// ── Terrain helpers ───────────────────────────────────────────────────────────
// FUN_004f6c60 @ 0x004F6C60 — Terrain_Clear: resets tile/height/noise arrays.
//
// BUG-FIX 2026-04-28: el decomp Ghidra usaba `(int)&DAT_xxxx + iVar2` y
// `*(float*)(iVar2 * 4 + 0x810b2c8)` — accesos por dirección absoluta /
// pointer-arith fuera del símbolo. Los símbolos eran DWORDs de 4 bytes en
// nuestra globals.cpp, así que escribir [65535] desbordaba a globals
// adyacentes y eventualmente AV en el último write (0x810B2CC). Cambiamos
// los símbolos a arrays reales (globals.cpp) y este loop a indexación normal.
//
// Layout (256x256 tile grid = 65536 entries):
//   DAT_080bb2b4[i] = 0     (TileTex1 byte)
//   DAT_080ab2b4[i] = 0xFF  (TileTex2 byte)
//   DAT_0834b608[i] = 0.0f  (TerrainHeight)
//   DAT_0810b2cc[i] = (rand() & 3) * scale  (TerrainNoise per-tile UV jitter)
// IDA: FUN_004F6C60
void __cdecl Terrain_Clear(void) {
    for (int i = 0; i < 0x10000; ++i) {
        DAT_080bb2b4[i] = 0;
        DAT_080ab2b4[i] = 0xff;
        DAT_0834b608[i] = 0.0f;
        unsigned int uVar1 = (unsigned int)rand() & 3u;     // 0..3
        DAT_0810b2cc[i]   = (float)(int)uVar1 * _DAT_005528dc;
    }
}

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
void __cdecl CreateTerrainNormal(void) {
    float *pfVar5 = DAT_07feb288;
    for (unsigned int yRow = 0; yRow < 256; ++yRow) {
        unsigned int yNext = yRow + 1;
        float fVar3 = (float)(int)yRow  * _DAT_005524f0;
        float fVar4 = (float)(int)yNext * _DAT_005524f0;
        int iVar6 = (int)(yNext & 0xffu) * 0x100;
        for (unsigned int xCol = 1; xCol <= 256; ++xCol) {
            float p1[3], p2[3], p3[3];
            // p2 = (x_curr, y_curr, height@(x_curr, y_curr))
            p2[0] = (float)(int)xCol * _DAT_005524f0;
            p2[1] = fVar4;
            p2[2] = DAT_080cb2cc[(xCol & 0xffu) + iVar6];
            // p1 = (x_curr, y_prev, height@(x_curr, y_prev))
            p1[0] = p2[0];
            p1[1] = fVar3;
            p1[2] = DAT_080cb2cc[(xCol & 0xffu) + (yRow & 0xffu) * 0x100];
            // p3 = (x_prev, y_curr, height@(x_prev, y_curr))
            p3[0] = (float)(int)(xCol - 1) * _DAT_005524f0;
            p3[1] = fVar4;
            p3[2] = DAT_080cb2cc[((xCol - 1) & 0xffu) + iVar6];
            Triangle_ComputeNormal(p1, p2, p3, pfVar5);
            pfVar5 += 3;
        }
    }
}

// FUN_004f71c0 @ 0x004F71C0 — Terrain_FinalizeLighting: multiplies normal buffer
// by lightmap (DAT_07eeb238) and clamps into DAT_0828b608.
// IDA: FUN_004F71C0
void __cdecl Terrain_FinalizeLighting(void) {
    int iVar4 = 0;
    do {
        int iVar6 = 0x100;
        do {
            // DAT_07feb288 is float[3*n]: [0]=x,[4]=y(+4),[8]=z(+8) per vertex
            // Ghidra offsets: +0=x, +4=y (DAT_07feb28c), +8=z (DAT_07feb290)
            float fVar1 = ((*(float*)((int)&DAT_07feb288 + iVar4) + *(float*)((char*)&DAT_07feb288 + iVar4 + 8)) *
                           _DAT_00552504 - *(float*)((char*)&DAT_07feb288 + iVar4 + 4) * _DAT_00552504) + _DAT_00552504;
            float fVar2 = _DAT_00552580;
            if (_DAT_00552580 <= fVar1) {
                fVar2 = fVar1;
                if (_DAT_0055256c < fVar1) fVar2 = _DAT_0055256c;
            }
            int iVar5 = 3;
            int iVar3 = iVar4;
            do {
                iVar4 = iVar3 + 4;
                iVar5--;
                *(float*)((int)&DAT_0828b608 + iVar3) = fVar2 * *(float*)((int)&DAT_07eeb238 + iVar3);
                iVar3 = iVar4;
            } while (iVar5 != 0);
            iVar6--;
        } while (iVar6 != 0);
    } while (iVar4 < 0xc0000);
}

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
uint __cdecl OpenTerrainHeight(char *filename)
{
    char FileName[256];
    char Text[256];

    if (DAT_0055a7c4 != '\0') {
        // compressed mode: strip extension into Text, then "Data/<Text>.ozb"
        int v3 = 0;
        int plen = (int)strlen(filename);
        for (int i = 0; i < plen; ++i) {
            Text[i] = filename[i];
            if (filename[i] == '.') { v3 = i; break; }
            v3 = i + 1;
        }
        Text[v3 + 1] = 0;
        strcpy(FileName, "Data\\");
        strcat(FileName, Text);
        strcat(FileName, "ozb");        // aOzb in IDA
    } else {
        // plain mode: "Data2\\<filename>"
        strcpy(FileName, "Data2\\");
        strcat(FileName, filename);
    }

    FILE *fp = fopen(FileName, "rb");
    if (!fp) {
        crt_sprintf(Text, "%s file not found.", FileName);
        FUN_00405540(&DAT_055c9bf0, Text);
        if (g_hWnd) {
            MessageBoxA(g_hWnd, Text, nullptr, 0);
            SendMessageA(g_hWnd, WM_DESTROY, 0, 0);
        }
        return 0;
    }
    if (DAT_0055a7c4 != '\0') {
        fseek(fp, 4, SEEK_SET);  // skip 4-byte ozb header
    }
    unsigned char *Buffer = (unsigned char*)operator_new(66616);
    fread(Buffer, 1, 66616, fp);
    fclose(fp);

    // copy 1080-byte BMP header into BMPHeader global (DAT_0838b800)
    memcpy(DAT_0838b800, Buffer, 1080);

    // optional: persist decoded copy in plain mode (not implemented, original
    // calls SaveImage(4, "ozb", filename, Buffer, 66616)).

    // convert 256×256 bytes (after the 1080-byte header) to floats * 1.5
    float *dst = DAT_080cb2cc;
    unsigned char *src = Buffer + 1080;
    for (int i = 0; i < 256 * 256; ++i) {
        dst[i] = (float)src[i] * 1.5f;
    }
    operator_delete(Buffer);
    return 1;
}

// FUN_00529130 @ 0x00529130 — SaveImage (relabeled audit #10; was mislabeled
// "Terrain_Decompress / Terrain_CRC_Check"). Persists screenshot/texture data
// to disk depending on `mode`.  Stub passthrough.
void __cdecl FUN_00529130(int mode, int ext, int path, int buf)
{
    (void)mode; (void)ext; (void)path; (void)buf;
}

// FUN_004f9c20 @ 0x004F9C20 — Terrain_SetupCulling: initialises raycast/frustum globals.
void __cdecl FUN_004f9c20(void) {
    _DAT_0838b60a = 0x47e;                  // short: horizon value
    *(DWORD*)&_DAT_0838b614 = 0x41000000u;  // float bits: 8.0f
    *(DWORD*)&_DAT_0838b710 = 0x3f800000u;  // float bits: 1.0f
}

// FUN_00529360 @ 0x00529360 — OpenJpegBuffer / Texture_LoadToBuf
// Loads OZJ/JPEG file, decompresses, converts each pixel component to float
// using _DAT_00552b70 (1/255 normalization), stores into float* buffer at dst.
// Output: 3 floats per pixel (R, G, B), rows stored bottom-up (flipped).
void __cdecl FUN_00529360(char *path, int dst)
{
    // --- Path construction (same logic as FUN_00529740) ---
    char full_path[256];
    if (DAT_0055a7c4 == '\0') {
        strcpy(full_path, (const char*)DAT_0055a7a4);
        strcat(full_path, path);
    } else {
        char basename[256];
        int dotPos = 0;
        int len = (int)strlen(path);
        for (int i = 0; i < len; i++) {
            basename[i] = path[i];
            if (path[i] == '.') { dotPos = i; break; }
        }
        basename[dotPos + 1] = '\0';
        strcpy(full_path, (const char*)DAT_0055a79c);
        strcat(full_path, basename);
        strcat(full_path, (const char*)DAT_00561b70);
    }

    // --- Open file ---
    FILE* f = fopen(full_path, "rb");
    if (f == NULL) {
        char msg[256];
        sprintf(msg, "%s : File not exist!", full_path);
        CErrorReport__Write(0x55c9bf0, msg);
        CErrorReport__Write(0x55c9bf0, (char*)"\n");
        MessageBoxA((HWND)DAT_055c9ffc, msg, NULL, 0);
        SendMessageA((HWND)DAT_055c9ffc, WM_DESTROY, 0, 0);
        return;
    }

    // --- Skip OZJ 24-byte header ---
    if (DAT_0055a7c4 == '\0') {
        fseek(f, 0x18, SEEK_SET);
    } else {
        fseek(f, 0x18, SEEK_SET);
    }

    // --- libjpeg decompress ---
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, f);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    unsigned int img_w = cinfo.output_width;
    unsigned int img_h = cinfo.output_height;
    int components = cinfo.output_components;
    int row_stride = components * img_w;

    // Allocate temp buffer for raw pixel data
    JSAMPARRAY row_buf = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    unsigned char* rawBuf = (unsigned char*)operator_new(img_h * row_stride);

    // Read scanlines into temp buffer (flipped: row 0 of JPEG → bottom of buffer)
    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned int scanline = cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, row_buf, 1);
        unsigned char* rowDst = rawBuf + (img_h - 1 - scanline) * row_stride;
        memcpy(rowDst, row_buf[0], row_stride);
    }

    // --- Convert bytes to floats ---
    // Output: 3 floats per pixel (R, G, B), stored sequentially
    float* outFloat = (float*)dst;
    int idx = 0;
    for (unsigned int y = img_h; y > 0; ) {
        y--;
        for (unsigned int x = 0; x < img_w; x++) {
            int srcOff = idx;
            outFloat[0] = (float)rawBuf[srcOff]     * _DAT_00552b70;
            outFloat[1] = (float)rawBuf[srcOff + 1]  * _DAT_00552b70;
            outFloat[2] = (float)rawBuf[srcOff + 2]  * _DAT_00552b70;
            outFloat += 3;
            idx += 3;
        }
    }

    // --- Cleanup ---
    operator_delete(rawBuf);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(f);
}

// FUN_00454b00 @ 0x00454B00 — Entity_GetMoveRate(entity_ptr) → float10
// Returns movement speed rate based on entity state (+0x300 stamina, +0x2b8 class, +0x84 flag).
// FUN_00454b00 (IDA-activated, was Ghidra stub)
float10 __cdecl FUN_00454b00(int c)
{
  double result; // st7
  short v2; // dx
  short v3; // cx

  result = (double)*(unsigned short *)(c + 762);
  if ( *(WORD *)(c + 2) != 390 || *(BYTE *)(c + 132) != 1 )
  {
    goto LABEL_12;
  }
  v2 = *(WORD *)(c + 672);
  if ( v2 == -1 && ((v3 = *(WORD *)(c + 696), v3 < 818) || v3 > 819) || *(BYTE *)(c + 846) )
  {
    if ( *(BYTE *)(c + 768) < 0x28u )
    {
      result = 12.0;
      goto LABEL_12;
    }
  }
  else
  {
    *(BYTE *)(c + 768) = 40;
    if ( v2 == 789 )
    {
      result = 16.0;
      goto LABEL_12;
    }
  }
  result = 15.0;
LABEL_12:
  if ( (*(BYTE *)(c + 120) & 2) == 2 )
  {
    return result * 0.5;
  }
  return result;
}


// addr: 0x0040A8F0  (sub_40A8F0 — VTable_DrawEdge)
// ported from IDA raw/0040A8F0_sub_40A8F0.c
// Draws one edge of a physics/collision hull as a textured glBegin(GL_QUADS) strip,
// using texture 494 and alpha-blend-minus. Widens the edge into a thin billboard
// quad by jittering perpendicular via FUN_004f9d20/FUN_004f9d60 (cross+normalize).
// Caller FUN_0040a860 passes 3 args (obj, p1, p2); IDA disassembled with 5 where
// a4/a5 were unreferenced stack slots — safely omitted here.
void __cdecl FUN_0040a8f0(void *obj, float *p1, float *p2) {
    // Forward-declares (defined later in this TU, not in functions.h).
    extern void __cdecl FUN_004f9d20(float *a, float *b, float *out);
    char *this_ = (char *)obj;
    float v27, v28, v29, v30, v31, v32, v33, v34, v35;
    float v24, v25, v26;
    float v36, v37, v38;
    float t;
    float x, y, z;
    double v6, v10;

    glColor3f(1.0f, 1.0f, 1.0f);
    v27 = p2[0] - p1[0];
    v28 = p2[1] - p1[1];
    v29 = p2[2] - p1[2];
    {
        float tmp[3] = { v27, v28, v29 };
        v6 = (double)FUN_004f9c40(tmp);
    }
    float p1y = p1[1];
    float p1z = p1[2];
    v30 = p2[0];
    v34 = p1y;
    v31 = p2[1];
    v35 = p1z;
    float p2z = p2[2];
    v37 = (float)((50.0 - v6) * 0.0099999998);
    v10 = (double)p1[0];
    v28 = v31 - p1y;
    v29 = p2z - v35;
    v27 = (float)(((double)v30 - v10) * 0.1);
    v28 = v28 * 0.1f;
    v29 = v29 * 0.1f;
    v33 = (float)(v10 - (double)v27);
    v34 = p1y - v28;
    v35 = v35 - v29;
    v30 = v30 + v27;
    v31 = v31 + v28;
    v32 = p2z + v29;
    v36 = (float)((double)(rand() % 100) * 0.0099999998);
    glColor3f(1.0f, 1.0f, 1.0f);
    GL_BindTextureSlot(494);            // BindTexture(494)
    GL_SetBlendSrcAlpha();                // EnableAlphaBlendMinus
    {
        float d[3] = { v27, v28, v29 };
        float cross[3] = { 0.0f, 0.0f, 0.0f };   // FUN_004f9d20 lo escribe (out param);
                                                 // inicializarlo saca el C4700 que tapaba
                                                 // los avisos reales.
        FUN_004f9d20((float *)(this_ + 12), d, cross);
        v24 = cross[0]; v25 = cross[1]; v26 = cross[2];
        float nrm[3] = { v24, v25, v26 };
        FUN_004f9d60(nrm);
        v24 = nrm[0]; v25 = nrm[1]; v26 = nrm[2];
    }
    v24 = v24 * 10.0f;
    v25 = v25 * 10.0f;
    v26 = v26 * 10.0f;
    glBegin(GL_QUADS);
    t = v36 + v37;
    glTexCoord2f(0.0f, t);
    z = v35 - v26;
    y = v34 - v25;
    x = v33 - v24;
    glVertex3f(x, y, z);
    v38 = 1.0f - v37 + v36;
    glTexCoord2f(0.0f, v38);
    glVertex3f(v30 - v24, v31 - v25, v32 - v26);
    glTexCoord2f(1.0f, v38);
    glVertex3f(v24 + v30, v25 + v31, v26 + v32);
    glTexCoord2f(1.0f, t);
    glVertex3f(v24 + v33, v25 + v34, v26 + v35);
    glEnd();
}

