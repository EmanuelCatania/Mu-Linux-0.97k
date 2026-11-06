// Terrain_RenderTerrain.cpp
// Dibujo del terreno (RenderTerrain), agua y frustum 2D del terreno.
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

// Combat_SendMovePathPacket (Send_MovePacket), Combat_DispatchHeroSkillAttack (Attack), Combat_CheckArrowRequirement (CheckArrow),
// Combat_UseElfSkill (UseSkillElf), Action (Action big switch):
// viven en src/Combat/Combat.cpp.

// IDA: TERRAIN_INDEX (0x004F6C30)
int  __cdecl TERRAIN_INDEX(int param_1, int param_2) { return param_2 * 0x100 + param_1; }
// IDA: RenderTerrain (0x004F9AC0)
// Reemplaza la fallback flat-shaded previa por el decompile fiel de IDA (352 b).
// Mantiene flujo y orden de Render States del binario:
//   sub_4F98C0 (terrain light setup) → WaterMove update → [Edit: SelectFlag=0 +
//   Map_InitRayCast | Run: DisableAlphaBlend] → TerrainFlag=0 →
//   RenderTerrainFrustrum(EditFlag) → [Edit: si SelectFlag, RenderTerrainTile del
//   tile pickeado | Run: EnableAlphaTest(1) → overlay pass TerrainFlag=2 →
//   ambient objects → DisableDepthTest/EnableCullFace/AlphaBitmaps/EnableDepthTest]
//   → toggle ^=1 → sub_4F9A30.
//
// Dependencias verificadas en IDA (bytes de operando):
//   Hero=0x07abf5d8, World(0x0055a7ac), WorldTime=0x05826e08,
//   WaterMove=0x07eeb214, SelectFlag=0x07eab1fc, SelectXF/YF=0x080ab288/28c,
//   TerrainFlag=0x0838bc44, toggle=0x0839bc88, unk_55A76C=0x0055a76c.
//   - WorldTime: en el binario es float ((float)timeGetTime() en CalcFPS 0x43FD70);
//     en nuestro codebase es int g_AnimTick y todos sus readers lo usan como int.
//     Se lee `(long long)WorldTime % N`, NO `(int)`.  `timeGetTime()`
//     pasa de 2^31 ms a las ~24.8 dias de uptime y ahi el cast a int satura, con
//     lo que la animacion de agua queda congelada.  IDA usa `(std::int64_t)WorldTime`
//     en todos sus sitios justamente por eso.
//   - unk_55A76C: único xref es el read de abajo (sin writer en el binario) → el
//     2º pass overlay (TerrainFlag=2) es inerte también en el original.
//   - Callees aún fallback (a portar en esta cadena): RenderTerrainFrustrum
//     (#2, 0x004F97E0), RenderTerrainTile RenderTerrainTile (#3, 0x004F8480).
static void Terrain_BeginWavePass() {
    FUN_004f98c0(
        (int)(*(float*)(Hero + 16) * 0.039999999f),   // hero X * 0.04 → tile coord
        (int)(*(float*)(Hero + 20) * 0.039999999f),   // hero Y * 0.04
        3,
        -70,
        (int)DAT_0839bc88);
}
static void Terrain_EndWavePass() {
    DAT_0839bc88 ^= 1u;
    Terrain_WaterWaveUpdate((int)DAT_0839bc88);
}
// DESVIACION: la pasada visual avanza el agua una vez por paso de 40 ms.
// El picking conserva sus llamadas adicionales dentro de SIM.
void Terrain_TickRenderWater() {
    if (SceneFlag != 5 || DAT_07e11d1c > 30 || !DAT_07abf5d8) return;
    if (gMapManager.GetCurrentMap() != 10 && !gUserSettings.GetAntilag(ANTILAG_FLOOR)) {
        Terrain_BeginWavePass();
        Terrain_EndWavePass();
    }
}

void __cdecl RenderTerrain(char EditFlag) {
    // DESVIACION (DLL Antilag, ret en 0x004F9AC0): sin piso.  El DLL cortaba
    // también la pasada de picking (EditFlag=1, Player_InputTick) y el click al
    // suelo dejaba de mover al personaje; acá sólo se omite el dibujo.
    if (!EditFlag && gUserSettings.GetAntilag(ANTILAG_FLOOR)) return;
    if (EditFlag) Terrain_BeginWavePass();

    if (gMapManager.GetCurrentMap() == 8)
        DAT_07eeb214 = (float)((long long)WorldTime % 40000) * 0.000024999999f;  // WaterMove (Tarkan)
    else
        DAT_07eeb214 = (float)((long long)WorldTime % 20000) * 0.000049999999f;  // WaterMove

    if (EditFlag) {
        DAT_07eab1fc = 0;                 // SelectFlag = 0
        Map_InitRayCast();                   // Map_InitRayCast (sub_512D30)
    } else {
        GL_ResetState();                   // DisableAlphaBlend
    }

    DAT_0838bc44 = 0;                     // TerrainFlag = 0
    const bool modern=!EditFlag && TerrainDraw::Requested() && !DAT_0839bc86;
    if(modern)Terrain_BeginBufferedPass();
    RenderTerrainFrustrum(EditFlag != 0);
    if(modern && !TerrainDraw::End())RenderTerrainFrustrum(false);

    if (EditFlag) {
        if (DAT_07eab1fc) {               // SelectFlag → render del tile pickeado
            float sxf = *(float*)&DAT_080ab288;   // SelectXF
            float syf = *(float*)&DAT_080ab28c;   // SelectYF
            RenderTerrainTile(*(int*)&sxf, *(int*)&syf, (int)sxf, (int)syf,
                         1.0f, 1, (int)(unsigned char)EditFlag);
        }
    } else {
        GL_SetBlendSrcOver('\x01');             // EnableAlphaTest(1)
        if (DAT_0055a76c && gMapManager.GetCurrentMap() != 7) { // overlay (inerte: unk_55A76C nunca seteado)
            DAT_0838bc44 = 2;             // TerrainFlag = 2
            RenderTerrainFrustrum(false);
        }
        Terrain_SpawnAmbientObjects();                   // Terrain_SpawnAmbientObjects (sub_4F7060)
        GL_DisableDepthTest();                   // DisableDepthTest
        GL_EnableCullFace();                   // EnableCullFace
        RenderTerrainAlphaBitmaps();                   // RenderTerrainAlphaBitmaps (sub_479540)
        GL_EnableDepthTest();                   // EnableDepthTest
    }

    if (EditFlag) Terrain_EndWavePass();
}

#if 0
// ── Fallback flat-shaded previa (REEMPLAZADA por el port 1:1 de RenderTerrain) ──
// Bloque inactivo, conservado solo como referencia del path cámara/proyección.
static void RenderTerrain_FallbackUnused(char EditFlag) {
    if (!DAT_07abf5d8) return;
    // BUG-FIX 2026-04-28: Edit mode (EditFlag=1) lo llama Player_InputTick para
    // mouse picking — necesita iterar tiles y llamar a RenderTerrainTile con flag de
    // picking, que calcula la intersección rayo-tile y guarda el resultado en
    // DAT_080ab288 / DAT_080ab28c. Sin esto el click al suelo no genera path,
    // entonces el hero nunca se mueve.

    // Simplified iteration: 64x64 tile area centered on hero.
    if (!DAT_07abf5d8) return;
    BYTE* hero = (BYTE*)DAT_07abf5d8;
    float hx = *(float*)(hero + 0x10);
    float hy = *(float*)(hero + 0x14);
    int gridCenterX = (int)(hx / _DAT_005524f0);
    int gridCenterY = (int)(hy / _DAT_005524f0);

    // BUG-FIX 2026-04-29: 64x64 era muy chico. Cámara con pitch -55° y zoom
    // estándar muestra ~80-100 tiles. Cursor podía caer fuera del scan.
    int xStart = gridCenterX - 64, xEnd = gridCenterX + 64;
    int yStart = gridCenterY - 64, yEnd = gridCenterY + 64;
    if (xStart < 0) xStart = 0;
    if (yStart < 0) yStart = 0;
    if (xEnd > 254) xEnd = 254;
    if (yEnd > 254) yEnd = 254;

    if (EditFlag) {
        // Mouse-pick mode: don't draw, just iterate tiles + call RenderTerrainTile
        // with picking flag.  RenderTerrainTile internally tests mouse ray against
        // el quad del tile y guarda las coordenadas de grilla del impacto en DAT_080ab288/28c.
        for (int yi = yStart; yi < yEnd; ++yi) {
            for (int xi = xStart; xi < xEnd; ++xi) {
                float xf = (float)xi;
                float yf = (float)yi;
                RenderTerrainTile(*(int*)&xf, *(int*)&yf, xi, yi, 1.0f, 1, (int)'\x01');
            }
        }
        return;
    }

    // IDA RenderTerrain (0x004F9AC0) abre el path EditFlag=0 con DisableAlphaBlend().
    // Usamos el wrapper 1:1 (GL_ResetState) en vez de glEnable/glDisable crudo para
    // mantener sincronizado el caché de estado GL (AlphaBlendType/AlphaTestEnable/
    // TextureEnable) que comparten todos los passes. Estado resultante: blend OFF,
    // cull ON, escritura de profundidad ON, alpha-test OFF, textura ON.
    GL_EnableDepthTest();                  // EnableDepthTest (la fallback dibuja con depth test)
    GL_ResetState();                  // DisableAlphaBlend — estado opaco base

    // Trackea la última textura bindeada para minimizar los binds.
    int lastTex = -1;

    for (int yi = yStart; yi < yEnd; ++yi) {
        for (int xi = xStart; xi < xEnd; ++xi) {
            float xf = (float)xi;
            float yf = (float)yi;
            int idx0 = yi * 256 + xi;
            int idx1 = yi * 256 + (xi + 1);
            int idx2 = (yi + 1) * 256 + (xi + 1);
            int idx3 = (yi + 1) * 256 + xi;
            float wx0 = xf * 100.0f, wy0 = yf * 100.0f;
            float wx1 = wx0 + 100.0f, wy1 = wy0 + 100.0f;
            float h0 = DAT_080cb2cc[idx0];
            float h1 = DAT_080cb2cc[idx1];
            float h2 = DAT_080cb2cc[idx2];
            float h3 = DAT_080cb2cc[idx3];

            // BUG-FIX 2026-05-01: el .map almacena tile-type INDICES (0-13)
            // por celda (TileGrass01=0, TileGrass02=1, TileGround01=2, ...,
            // TileRock07=13). El GL slot real es 0x23 + index. Antes el código
            // usaba el index directo como slot → leía texturas de UI/items
            // (slots 0..0xC son cursor sprites) → "suelo mosaico" UI.
            // Tiles cargados en OpenWorld: 0x23 grass1, 0x24 grass2, 0x25 ground1,
            // 0x26 ground2, 0x27 ground3, 0x28 water, 0x29 wood, 0x2a-0x30 rock1-7.
            int tileIdx = (int)(unsigned char)TerrainMappingLayer1[idx0];
            int tileTex = 0x23 + tileIdx;
            if (tileTex < 0x23 || tileTex > 0x30) tileTex = 0x23;   // fallback grass01
            if (tileTex != lastTex) {
                GL_BindTextureSlot(tileTex);          // glBindTexture (texture system)
                lastTex = tileTex;
            }

            // Per-vertex lighting (DAT_0828b608 ya confirmado en valores 0.84-0.94 OK).
            // BUG-FIX 2026-05-03: algunos tiles se renderizan violeta/magenta porque
            // DAT_0828b608 tiene valores por canal raros (p.ej. R=alto, G=0, B=alto)
            // en tiles donde TerrainLight.OZJ tiene un color inusual o
            // FUN_004f71c0 wasn't fully populated. Helper sanitizes per-vertex
            // luz: si algún canal está en 0, lo reemplazamos por el promedio de los otros o 0.85.
            auto sanitize = [](float r, float g, float b, float* out) {
                int zeros = (r == 0.0f) + (g == 0.0f) + (b == 0.0f);
                if (zeros >= 2) {
                    out[0] = out[1] = out[2] = 0.85f;
                    return;
                }
                if (zeros == 1) {
                    float avg = (r + g + b) * 0.5f;
                    r = (r == 0.0f) ? avg : r;
                    g = (g == 0.0f) ? avg : g;
                    b = (b == 0.0f) ? avg : b;
                }
                // Anti-violet: si la diferencia entre el canal max y min es muy
                // grande (>0.4), el tile tiene una dominancia chromática
                // probablemente del lightmap mal-interpretado. Promediamos para
                // eliminar el tinte saturado pero conservar brillo.
                float mx = r > g ? (r > b ? r : b) : (g > b ? g : b);
                float mn = r < g ? (r < b ? r : b) : (g < b ? g : b);
                if (mx - mn > 0.4f) {
                    float avg = (r + g + b) * (1.0f / 3.0f);
                    out[0] = out[1] = out[2] = avg;
                    return;
                }
                out[0] = r; out[1] = g; out[2] = b;
            };
            float c0[3], c1[3], c2[3], c3[3];
            sanitize(DAT_0828b608[idx0*3+0], DAT_0828b608[idx0*3+1], DAT_0828b608[idx0*3+2], c0);
            sanitize(DAT_0828b608[idx1*3+0], DAT_0828b608[idx1*3+1], DAT_0828b608[idx1*3+2], c1);
            sanitize(DAT_0828b608[idx2*3+0], DAT_0828b608[idx2*3+1], DAT_0828b608[idx2*3+2], c2);
            sanitize(DAT_0828b608[idx3*3+0], DAT_0828b608[idx3*3+1], DAT_0828b608[idx3*3+2], c3);

            BufferedDraw::Begin(BufferedDraw::Primitive::Quads);
            BufferedDraw::Color3fv(c0);
            BufferedDraw::TexCoord2f(0.0f, 0.0f);
            BufferedDraw::Vertex3f(wx0, wy0, h0);
            BufferedDraw::Color3fv(c1);
            BufferedDraw::TexCoord2f(1.0f, 0.0f);
            BufferedDraw::Vertex3f(wx1, wy0, h1);
            BufferedDraw::Color3fv(c2);
            BufferedDraw::TexCoord2f(1.0f, 1.0f);
            BufferedDraw::Vertex3f(wx1, wy1, h2);
            BufferedDraw::Color3fv(c3);
            BufferedDraw::TexCoord2f(0.0f, 1.0f);
            BufferedDraw::Vertex3f(wx0, wy1, h3);
            BufferedDraw::End();
        }
    }

    // IDA RenderTerrain: tras el pass de terreno llama EnableAlphaTest(1). Deja
    // GL_ALPHA_TEST ON (+ blend SRC_ALPHA/ONE_MINUS_SRC_ALPHA, cull OFF) y ese
    // estado PERSISTE hacia los passes de objetos del mundo (Terrain_Render →
    // Entity_PrepareRender), que renderean vegetación/decals con alpha. Sin esto
    // GL_ALPHA_TEST quedaba OFF → las esquinas de los quads con alpha salían
    // opacas = los triángulos negros alrededor de vegetación/objetos.
    GL_SetBlendSrcOver('\x01');            // EnableAlphaTest(1)
}
#endif  // fallback flat-shaded obsoleta

// IDA: CreateFrustrum2D (0x004F8EB0)
// GetScreenWidth NO es un frame counter — es GetScreenWidth(). Los
// constants `_DAT_00552cbc=1190.0` y `_DAT_00552cb8=540.0` son las half-widths
// de la frustum quad (en view-space units), NO velocidades de rotación.
// _DAT_0055283c = 1/640 (= screen pixel→aspect ratio).
//
// Construye un quad 2D top-view del frustum proyectado: rota por Z=45° las 4
// corners hardcoded, las traslada por param_1 (cam pos), y las escribe a
// FrustrumX/218 (X/Y respectivamente) en tile coords (×0.01 = ÷100).
//
// Esta versión SÍ funciona para in-game. La duplicada Camera_SetMatrix en
// src/Render/Camera.cpp es código muerto que opera sobre DAT_07eab1bc..1e8
// (corners ya transformadas por Camera_SetupFrustum a world coords), lo cual
// duplicaría la transformación si se invocara — no se llama desde ningún
// lado y no debe wirearse.
// DESVIACION (in-game): el cuadro de tiles sale de cortar el frustum 3D que
// acaba de armar Camera_SetupFrustum (FrustrumVertex: ápice + 4 esquinas en
// coordenadas de mundo) con el plano del suelo, en vez de las constantes
// fijas del binario, que estaban calibradas para el FOV de 35 grados.  Con
// el FOV y el alcance mayores las esquinas de la pantalla quedaban sin
// terreno.  El plano va 300 unidades por debajo del héroe y el cuadro se
// agranda 800 unidades (8 tiles) hacia afuera: TestFrustrum2D mide la
// tolerancia de bloques y objetos con el producto cruz sin normalizar, así
// que con un cuadro más grande esa tolerancia se achica y los bloques de
// 16x16 cerca del borde quedaban afuera (objetos que no aparecían).
static bool CreateFrustrum2D_FromFrustum(const float *ground)
{
    const float apex[3] = { FrustrumVertex, DAT_07eab1b4, DAT_07eab1b8 };
    const float corner[4][3] = {
        { DAT_07eab1bc, DAT_07eab1c0, DAT_07eab1c4 },   // arriba-izquierda
        { DAT_07eab1c8, DAT_07eab1cc, DAT_07eab1d0 },   // arriba-derecha
        { DAT_07eab1d4, DAT_07eab1d8, DAT_07eab1dc },   // abajo-derecha
        { DAT_07eab1e0, DAT_07eab1e4, DAT_07eab1e8 },   // abajo-izquierda
    };
    const float groundZ = ground[2] - 300.0f;
    if (apex[2] <= groundZ) return false;

    float p[4][2];
    float cx = 0.0f, cy = 0.0f;
    for (int i = 0; i < 4; ++i) {
        const float dz = corner[i][2] - apex[2];
        float t = 1.0f;
        if (dz < 0.0f) {
            t = (groundZ - apex[2]) / dz;
            if (t > 1.0f) t = 1.0f;
        }
        p[i][0] = apex[0] + (corner[i][0] - apex[0]) * t;
        p[i][1] = apex[1] + (corner[i][1] - apex[1]) * t;
        cx += p[i][0] * 0.25f;
        cy += p[i][1] * 0.25f;
    }
    for (int i = 0; i < 4; ++i) {
        float dx = p[i][0] - cx, dy = p[i][1] - cy;
        const float len = sqrtf(dx * dx + dy * dy);
        if (len > 0.001f) { dx = dx / len * 800.0f; dy = dy / len * 800.0f; }
        FrustrumX[i] = (p[i][0] + dx) * _DAT_005524f8;
        FrustrumY[i] = (p[i][1] + dy) * _DAT_005524f8;
    }
    return true;
}

void __cdecl CreateFrustrum2D(float *param_1)
{
    if (SceneFlag == 5 && CreateFrustrum2D_FromFrustum(param_1)) return;

    float pts[15];   // euler[0..2], then 4×vec3 input offsets [3..14]
    float rot[12];   // 3×4 rotation matrix
    float out[12];   // 4 transformed output positions

    int iVar2 = GetScreenWidth();   // GetScreenWidth (= 640 normalmente)
    // En pantalla ancha el mundo es de ancho completo: la fracción sale del
    // viewport y el semiancho crece con el aspecto (el frustum de 4:3 lo recortaría).
    float widthScale = (float)iVar2 * _DAT_0055283c;
    if (gWindow.IsWide() && gWindow.GetHeight() > 0)
        widthScale = ((float)gWindow.GetRealWidth() / (float)gWindow.GetHeight()) / (4.0f / 3.0f);
    float angle_cbc = widthScale * _DAT_00552cbc;  // sw/640 * 1190 = far half-width
    float angle_cb8 = widthScale * _DAT_00552cb8;  // sw/640 * 540  = near half-width

    // Euler angles for AngleMatrix (X=0, Y=0, Z=45°) — top-down orientation
    pts[0] = 0.0f; pts[1] = 0.0f; pts[2] = 45.0f;

    // 4 view-space corner offsets (X stride = far/near width, Y stride = depth)
    pts[3]  = -angle_cbc; pts[4]  =  1272.0f; pts[5]  = 0.0f;  // top-left
    pts[6]  =  angle_cbc; pts[7]  =  1272.0f; pts[8]  = 0.0f;  // top-right
    pts[9]  =  angle_cb8; pts[10] =  -672.0f; pts[11] = 0.0f;  // bot-right
    pts[12] = -angle_cb8; pts[13] =  -672.0f; pts[14] = 0.0f;  // bot-left

    Matrix_BuildFromEuler(pts, rot);   // AngleMatrix (Euler→3x4)

    for (int i = 0, j = 0; i < 0x30; j += 4, i += 0xc) {
        float *pfVar1 = (float*)((char*)out + i);
        Vector_Rotate((float*)((char*)pts + i + 0xc), rot, pfVar1);  // VectorRotate
        pfVar1[0] += param_1[0];
        pfVar1[1] += param_1[1];
        pfVar1[2] += param_1[2];
        *(float*)((char*)&FrustrumX + j) = pfVar1[0] * _DAT_005524f8;  // X (tile coords)
        *(float*)((char*)&FrustrumY + j) = pfVar1[1] * _DAT_005524f8;  // Y
    }
}

// ── FUN_004f98c0 (IDA-activated, absent in Ghidra) ──
int __cdecl FUN_004f98c0(int a1, int a2, int a3, int a4, int a5)
{
  int v5; // esi
  int v6; // edi
  int v7; // ebx
  int v8; // ecx
  int result; // eax
  int v10; // ebp
  int v11; // edx
  int v12; // ebx
  int v13; // esi
  int v14; // ebp
  std::int64_t v15; // rax
  char *v16; // [esp+0h] [ebp-10h]
  float v17; // [esp+8h] [ebp-8h]
  int v18; // [esp+Ch] [ebp-4h]
  int v19; // [esp+14h] [ebp+4h]
  int v20; // [esp+18h] [ebp+8h]
  int v21; // [esp+24h] [ebp+14h]

  v5 = a3;
  v6 = a1;
  v16 = (char *)&DAT_0814b2e0 + 0x40000 * a5;
  v17 = 1024.0 / (double)a3 * (1024.0 / (double)a3);
  if ( a1 < 0 )
  {
    v6 = rand() % (255 - 2 * a3) + a3 + 1;
  }
  v7 = a2;
  if ( a2 < 0 )
  {
    v7 = rand() % (255 - 2 * a3) + a3 + 1;
  }
  v8 = a3 * a3;
  v18 = a3 * a3;
  result = -a3;
  v10 = -a3;
  v21 = -a3;
  v19 = a3;
  if ( v6 - a3 < 1 )
  {
    v21 = 1 - v6;
    v10 = 1 - v6;
  }
  if ( v7 - a3 < 1 )
  {
    result = 1 - v7;
  }
  if ( v6 + a3 > 255 )
  {
    a3 = 255 - v6;
  }
  if ( v7 + v5 > 255 )
  {
    v19 = 255 - v7;
  }
  v11 = result;
  v20 = result;
  if ( result < v19 )
  {
    v12 = v6 + ((result + v7) << 8);
    do
    {
      v13 = v10;
      if ( v10 < a3 )
      {
        v14 = v11 * v11;
        do
        {
          if ( v14 + v13 * v13 < v8 )
          {
            v15 = (std::int64_t)((cos(sqrt((double)(v14 + v13 * v13) * v17)) + 65535.0) * (double)a4);
            v11 = v20;
            *(DWORD *)&v16[4 * v12 + 4 * v13] += (int)v15 >> 19;
            v8 = v18;
          }
          ++v13;
        }
        while ( v13 < a3 );
        v10 = v21;
      }
      result = v19;
      ++v11;
      v12 += 256;
      v20 = v11;
    }
    while ( v11 < v19 );
  }
  return result;
}
