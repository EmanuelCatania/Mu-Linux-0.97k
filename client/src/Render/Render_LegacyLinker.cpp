// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD DAT_07eaa128;
extern void __cdecl FUN_0054158c(void* ptr);
extern void FUN_004fa5a0(void);
#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y) (0)
#endif
#ifndef LODWORD
#define LODWORD(x) (*((DWORD*)&(x)))
#define HIDWORD(x) (*(((DWORD*)&(x))+1))
#define SLOBYTE(x) (*((char*)&(x)))
#define SLOWORD(x) (*((short*)&(x)))
#define SLODWORD(x) (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x) (*((unsigned char*)&(x)))
#define HIBYTE(x) (*(((unsigned char*)&(x))+1))
#define LOWORD(x) (*((unsigned short*)&(x)))
#define HIWORD(x) (*(((unsigned short*)&(x))+1))
#endif
#define ITEM_SPECIAL_SKILL_OPTION 0
#define ITEM_SPECIAL_LUCK_OPTION 1
#define ITEM_OPTION_ADD_PHYSI_DAMAGE_CODE 60
#define ITEM_OPTION_ADD_MAGIC_DAMAGE_CODE 61
#define ITEM_OPTION_ADD_DEFENSE_RATE_CODE 62
#define ITEM_OPTION_ADD_DEFENSE_CODE 63
#define ITEM_OPTION_ADD_EXCELLENT_DAMAGE_CODE 72
// EnableAlphaBlend @ 0x00511710 — GL additive blending setup.
// 2026-06-29 BUG-FIX (depth-mask cache desync → estructuras opacas desaparecen):
// esta copia llamaba glDepthMask(0) y glDisable(GL_CULL_FACE) DIRECTOS, sin tocar
// los caches DAT_083a42e8 (depth mask) ni DAT_083a411c (cull). El render de meshes
// usa la versión cacheada GL_SetBlendAdditive (GL_State.cpp); cuando algún caller pasaba
// por ESTA copia, el cache quedaba en "depth write ON" mientras el GL real estaba
// en OFF → el EnableDepthMask cacheado de DisableAlphaBlend se volvía no-op → el
// objeto opaco siguiente renderizaba con mask=0, no escribía depth, y geometría
// más lejana lo tapaba. El source 5.2 (ZzzOpenglUtil.cpp:468) confirma que
// EnableAlphaBlend usa el DisableDepthMask CACHEADO, nunca glDepthMask directo.
// Fix: delegar a la versión canónica cacheada (idéntico address 0x00511710).
void __cdecl EnableAlphaBlend(void) {
    GL_SetBlendAdditive();
}

// EnableAlphaTest @ 0x00511680 — GL standard alpha blend + alpha test.
// 2026-06-29 BUG-FIX (mismo desync de cache que EnableAlphaBlend): esta copia
// llamaba glDepthMask(1)/glDisable(GL_CULL_FACE) DIRECTOS sin tocar los caches.
// El source 5.2 (ZzzOpenglUtil.cpp:443) confirma que EnableAlphaTest(DepthMask)
// usa el EnableDepthMask CACHEADO condicional. Fix: delegar a la versión canónica
// cacheada GL_SetBlendSrcOver (idéntico address 0x00511680; param = flag DepthMask).
void __cdecl EnableAlphaTest(bool enable) {
    GL_SetBlendSrcOver(enable ? '\x01' : '\0');
}


// Linker stubs — external functions called by OpenNpc/RenderEquipment3D/RenderItems3D
// These are placeholders until the actual implementations are decompiled.
// ═══════════════════════════════════════════════════════════════════════════════

// 2026-05-05: AccessModel era stub vacío → ningún BMD de NPC se cargaba.
// Solo el guardia (type=249) renderizaba porque usa player model 390 ya
// cargado. Los demás NPCs (Storage, Smith, Wizard, etc.) llamaban a
// AccessModel("Data\\Npc\\", "Storage", 1) etc pero el modelo nunca se
// cargaba → invisible.
//
// FUN_005060b0 es la impl real del BMD loader (Monster_LoadModel) — ya
// usado por OpenWorld para cargar Object1, Object11, etc. Misma signatura
// (id, path, name, idx). Delegamos directamente.
//
// 2026-05-05 (followup): además llamar FUN_00505c80 (OpenTexture) post-BMD
// load. Sin esto los NPCs cargaban geometría pero las texturas no se
// resolvían en los slots (IndexTexture[]) → render en blanco. El cliente
// original sí hace este paso después del BMD load para NPCs.
void __cdecl AccessModel(int id, char* path, char* name, int param) {
    FUN_005060b0(id, path, name, param);
    // Path para OpenTexture: typically "Npc\" sin "Data\" prefijo (los
    // path-strippers en FUN_00529bd0/740 ya lo manejan si viene completo).
    if (path) {
        FUN_00505c80(id, path, 0x2600, '\x01');
    }
    // 2026-05-05: setup de animation speeds (idéntico al patrón que
    // FUN_005098c0 hace para monsters). Sin esto, los NPCs cargan
    // geometry/textures pero entity[+0x105] action speed = 0 →
    // CharacterAnimation no avanza el frame → NPCs estáticos.
    //
    // CharacterAnimation lee de model+48 (=bones table per FUN_004423e0
    // alloc) con stride 16 bytes. Esa tabla tiene `numBones` entries de 0x10
    // bytes c/u. Para evitar buffer overflow (crashes vimos con NPCs de
    // pocos bones), solo escribir speeds hasta el límite de bones disponibles.
    int slotBase = DAT_05828d58 + id * 0xbc;
    int actionsTable = *(int*)(slotBase + 48);
    short numBones = *(short*)(slotBase + 38);   // realmente numBones, but anim speed lookup uses this
    if (actionsTable && numBones > 0) {
        static const unsigned int kSpeeds[7] = {
            0x3e800000,  // 0.25f - action 0 (idle)
            0x3e4ccccd,  // 0.2f  - action 1
            0x3eae147b,  // 0.34f - action 2
            0x3ea8f5c3,  // 0.33f - action 3
            0x3ea8f5c3,  // 0.33f - action 4
            0x3f000000,  // 0.5f  - action 5
            0x3f0ccccd   // 0.55f - action 6
        };
        int maxSpeeds = (numBones < 7) ? numBones : 7;
        for (int i = 0; i < maxSpeeds; ++i) {
            *(unsigned int*)(actionsTable + i * 0x10 + 0x04) = kSpeeds[i];
        }
    }
}

// OpenTexture @ 0x00505C80 — forward al símbolo FUN_00505c80 (implementado arriba).
// functions.h lo declara con esta firma (void*, bool); en x86 cdecl los tipos
// son binariamente compatibles con (const char*, char).
void __cdecl OpenTexture(int id, void* path, int flags, bool param) {
    FUN_00505c80(id, (const char*)path, flags, (char)param);
}

// LoadWaveFile @ 0x00404A10 — real implementation in src/Sound/Sound.cpp.

void __cdecl OpenModel(int id, char* path, ...) {
    // 0x00505E90 — Load model with variable texture/normal args
    (void)id; (void)path;
}

// Camera_BuildMouseRay is CreateScreenVector.  RenderObjectScreen lives in
// BMD_LegacyDraw.cpp and owns the native BMD animation/draw path.
extern void __cdecl Camera_BuildMouseRay(int sx, int sy, float* out);
extern void __cdecl FUN_004e13a0(int param_1, unsigned int param_2,
                                   unsigned char param_3, unsigned char param_4,
                                   float* param_5, int param_6, char param_7);

// RenderItem3D @ 0x004E1BE0 — port FIEL del binario original.
//
// IDA decompile (raw 0x4E1BE0):
//   1. Calcula Success (hit-test mouse vs item rect).
//   2. Switch by Type → ajusta sx/sy con factor de offset (0.5..1.1 de width/height)
//      para centrar correctamente el modelo 3D en el slot del inventario.
//   3. Llama CreateScreenVector(sx, sy, Position) → world-space pos.
//   4. Llama RenderObjectScreen(modelId, Level, Option1, Position, Success, PickUp).
//      modelId = Type + 400 (default), o IDs específicos para items 459/457/469/435 según Level.
//
// Esto reemplaza el placeholder que pintaba quads coloreados por grupo.
void __cdecl RenderItem3D(float sx, float sy, float Width, float Height,
                           int Type, int Level, int Option1, int ExtOption, bool PickUp)
{
    bool Success = false;
    float Position[3];

    // Hit-test: mouse over item rect (only when no item is picked-up, OR this IS the picked-up item).
    if (!DAT_07e91388 || PickUp) {
        float fMouseX = (float)DAT_083a427c;   // MouseX
        float fMouseY = (float)DAT_083a4278;   // MouseY
        if (fMouseX >= sx && fMouseX < sx + Width &&
            fMouseY >= sy && fMouseY < sy + Height) {
            Success = true;
        }
    }

    // Per-type screen-position offset (centro del modelo dentro del slot).
    // Branch tree extraído fielmente del IDA decompile.
    float ofsXmul = 0.5f, ofsYmul = 0.5f;  // defaults
    bool resolved = false;

    if (Type >= 0) {
        if (Type < 32) {
            ofsXmul = 0.80f; ofsYmul = 0.85f; resolved = true;
        } else if (Type < 96) {
            ofsXmul = 0.80f; ofsYmul = 0.70f; resolved = true;
        } else if (Type < 128) {
            ofsXmul = 0.60f; ofsYmul = 0.65f; resolved = true;
        }
    }

    if (!resolved) {
        if (Type == 145) {
            ofsXmul = 0.50f; ofsYmul = 0.50f; resolved = true;
        } else if (Type >= 136 && Type < 160) {
            ofsXmul = 0.70f; ofsYmul = 0.70f; resolved = true;
        } else if (Type >= 160 && Type < 192) {
            ofsXmul = 0.60f; ofsYmul = 0.55f; resolved = true;
        } else if (Type >= 192 && Type < 224) {
            ofsXmul = 0.50f;
            if (Type == 207) ofsYmul = 0.70f;
            else if (Type == 208) ofsYmul = 0.90f;
            else ofsYmul = 0.60f;
            resolved = true;
        } else if (Type >= 224 && Type < 256) {
            ofsXmul = 0.50f; ofsYmul = 0.80f; resolved = true;
        } else if (Type >= 256 && Type < 288) {
            ofsXmul = 0.50f;
            if (Type == 258 || Type == 260 || Type == 262) ofsYmul = 1.05f;
            else if (Type == 259 || Type == 264) ofsYmul = 1.10f;
            else ofsYmul = 0.80f;
            resolved = true;
        } else if (Type >= 288 && Type < 384) {
            ofsXmul = 0.50f; ofsYmul = 0.90f; resolved = true;
        }
    }

    if (!resolved) {
        switch (Type) {
            case 430: {
                ofsXmul = 0.60f; ofsYmul = 1.00f;
                resolved = true;
                break;
            }
            case 431:
                ofsXmul = 0.60f; ofsYmul = 1.00f; resolved = true; break;
            case 432:
            case 433:
                ofsXmul = 0.50f; ofsYmul = 0.90f; resolved = true; break;
            case 434:
                ofsXmul = 0.50f; ofsYmul = 0.75f; resolved = true; break;
        }
    }

    if (!resolved && Type == 435) {
        int lvl3 = Level >> 3;
        if (lvl3 == 0)      { ofsXmul = 0.50f; ofsYmul = 0.50f; }
        else if (lvl3 == 1) { ofsXmul = 0.70f; ofsYmul = 0.80f; }
        else if (lvl3 == 2) { ofsXmul = 0.70f; ofsYmul = 0.70f; }
        resolved = true;
    }

    if (!resolved && Type >= 416 && Type < 448) {
        ofsXmul = 0.50f; ofsYmul = 0.70f; resolved = true;
    }

    // 2026-08-11 — REMOVIDO: bloque inventado por el port (el comentario original
    // decía que estos items "expect to be centered ... not biased downward",
    // o sea una heurística a ojo, no un decompile). Forzaba ofsYmul = 0.50 para
    // 416-419/428/429 y, al no llevar guard `!resolved`, PISABA el valor correcto
    // de IDA para el rango [416,448) que asigna la rama de arriba (0.50/0.70).
    // Efecto: la Uniria (tipo 418) se anclaba en el centro de la casilla en vez
    // de al 70% → se veía más arriba que en el original. IDA `RenderItem3D`
    // (0x4E1BE0): `if (Type >= 416 && Type < 448) { _sx += W*0.5; _sy += H*0.7; }`
    // sin ninguna excepción para esos tipos.

    if (!resolved) {
        switch (Type) {
            case 457: {
                int lvl3 = (Level >> 3) & 0x0F;
                ofsXmul = 0.50f;
                ofsYmul = (lvl3 == 1) ? 0.80f : 0.95f;
                resolved = true;
                break;
            }
            // IDA: `case 460: case 459:` comparten cuerpo —
            //   if ((Level & 0xF8) == 24) goto LABEL_62 (0.5/0.5)
            //   else                      LABEL_90      (0.5/0.95)
            // 2026-08-11 FIX: el 460 estaba agrupado con 465-467 (0.5/0.5 fijo)
            // y el 459 tenía ramas inventadas (lvl3 13/14/15) que no están en el
            // 0.97k.
            case 459:
            case 460: {
                ofsXmul = 0.50f;
                ofsYmul = ((Level & 0xF8) == 24) ? 0.50f : 0.95f;
                resolved = true;
                break;
            }
            case 465:
            case 466:
            case 467:
                // IDA: goto LABEL_62
                ofsXmul = 0.50f; ofsYmul = 0.50f; resolved = true; break;
            case 469: {
                // IDA: if (!(Level>>3)) LABEL_62 (0.5/0.5)
                //      else if ((Level>>3) == 1) { sx += W*0.4; LABEL_73: sy += H*0.8 }
                //      else                      LABEL_93 (SIN offset)
                int lvl3 = Level >> 3;
                if (lvl3 == 0)      { ofsXmul = 0.50f; ofsYmul = 0.50f; }
                else if (lvl3 == 1) { ofsXmul = 0.40f; ofsYmul = 0.80f; }
                else                { ofsXmul = 0.00f; ofsYmul = 0.00f; }
                resolved = true;
                break;
            }
            // IDA: `if (Type >= 470) { if (Type < 473) LABEL_90; if (Type < 475) LABEL_79; }`
            case 470:
            case 471:
            case 472:                      // faltaba 472 (el rango es [470,473))
                ofsXmul = 0.50f; ofsYmul = 0.95f; resolved = true; break;
            case 473:
            case 474:
                ofsXmul = 0.50f; ofsYmul = 0.90f; resolved = true; break;
            // 2026-08-11 — REMOVIDOS los casos 475/476/477/478/479: no existen en
            // el 0.97k. En IDA caen al final de la cadena y, por estar dentro de
            // [448,480), terminan en `goto LABEL_90` = 0.50/0.95 (el mismo
            // fallback de más abajo). Los valores que había (0.90 / 0.5-0.5 /
            // 0.55-0.80) eran invenciones del port.
        }
    }

    if (!resolved && Type >= 416 && Type < 448) {
        ofsXmul = 0.50f;
        ofsYmul = 0.70f;
        resolved = true;
    }

    // IDA (cola): `if (Type < 448 || Type >= 480) { 0.5 / 0.60 } else goto LABEL_90 (0.5/0.95)`
    // 2026-08-11 FIX: el rango era [448,512), así que los tipos 480-511 tomaban
    // 0.95 cuando en IDA les corresponde 0.60.
    if (!resolved && Type >= 448 && Type < 480) {
        ofsXmul = 0.50f;
        ofsYmul = 0.95f;
        resolved = true;
    }

    if (!resolved) {
        switch (Type) {
            case 387: ofsXmul = 0.50f; ofsYmul = 0.45f; resolved = true; break;
            case 388: ofsXmul = 0.50f; ofsYmul = 0.40f; resolved = true; break;
            case 389: ofsXmul = 0.50f; ofsYmul = 0.75f; resolved = true; break;
            case 390: ofsXmul = 0.50f; ofsYmul = 0.55f; resolved = true; break;
        }
    }

    if (!resolved && (Type < 448 || Type >= 480)) {
        ofsXmul = 0.50f; ofsYmul = 0.60f;
    }

    float _sx = sx + Width  * ofsXmul;
    float _sy = sy + Height * ofsYmul;

    // Convert screen-space → world-space ray endpoint.
    Camera_BuildMouseRay((int)_sx, (int)_sy, Position);

    // Per-type modelId override (jewels/wings/special).
    int modelId = Type + 400;
    int levelArg = Level;

    switch (Type) {
        case 459: {
            int lvl3 = (Level >> 3) & 0x0F; // ITEM_POTION+11
            if (lvl3 == 1) modelId = MODEL_EVENT + 4;
            else if (lvl3 == 2) modelId = MODEL_EVENT + 5;
            else if (lvl3 == 3) modelId = MODEL_EVENT + 6;
            else if (lvl3 == 5) modelId = MODEL_EVENT + 8;
            else if (lvl3 == 6) modelId = MODEL_EVENT + 9;
            else if (lvl3 >= 8 && lvl3 <= 12) modelId = MODEL_EVENT + 10;
            break;
        }
        case 457: { // ITEM_POTION+9
            if (((Level >> 3) & 0x0F) == 1) modelId = MODEL_EVENT + 7;
            break;
        }
        case 469: { // ITEM_POTION+21
            if ((Level >> 3) == 1) modelId = 958;
            break;
        }
        case 435: {
            int lvl3 = Level >> 3;
            if (lvl3 == 0)      { modelId = MODEL_STAFF + 10; levelArg = -1; }
            else if (lvl3 == 1) { modelId = MODEL_SWORD + 19; levelArg = -1; }
            else if (lvl3 == 2) { modelId = MODEL_BOW + 18;   levelArg = -1; }
            break;
        }
        }

    // Guard contra modelo no cargado o pointer corrupto. RenderObjectScreen
    // (FUN_004e13a0) deferenciaría el modelEntry → libjpeg crash si meshBase
    // o numMesh están en garbage. Retornar silencioso si modelo no listo.
    {
        if (modelId < 0 || modelId >= 1200) return;
        char* modelEntry = (char*)DAT_05828d58 + modelId * 0xbc;
        short numMesh = *(short*)(modelEntry + 0x24);
        int meshBase = *(int*)(modelEntry + 0x28);
        // Books 1..16 are loaded at model IDs 880..895.  The original
        // RenderItem3D has no early-out here; specifically do not hide Book16
        // (Soul Barrier) because a transient mesh check says it is unavailable.
        const bool nativeBook = modelId >= 880 && modelId <= 895;
        if (!nativeBook && (numMesh <= 0 || numMesh > 1000)) return;
        if (!nativeBook && (meshBase == 0 || (uintptr_t)meshBase < 0x100000)) return;
    }

    // IDA 0x004E1BE0 calls RenderObjectScreen(Type+400, Level, Option1, Position, Success, PickUp).
    // The original path does not forward ExtOption here.
    FUN_004e13a0(modelId, (unsigned int)levelArg, (unsigned char)Option1,
                 0, Position, Success ? 1 : 0, PickUp ? 1 : 0);
}

// Batch 21 — helper function stubs (called by MoveObjects, CollisionDetectLineToMesh, CheckMixRecipe)
// FUN_004fa5f0 (IDA-activated, was Ghidra stub)
void __cdecl FUN_004fa5f0(int a1)
{
  int v1; // edi
  short v2; // ax
  float *v3; // edi
  float v4; // ecx
  float v5; // edx
  bool v6; // zf
  int v7; // ebx
  float Position[3]; // [esp+8h] [ebp-Ch] BYREF

  if ( DAT_0055a7b4 < 0 )
  {
    return;
  }
  if ( DAT_0055a7b0 < 0 )
  {
    return;
  }
  v1 = DAT_0055a7b8;
  if ( DAT_0055a7b8 < 0 || World != DAT_0055a7b4 )
  {
    return;
  }
  v2 = *(WORD *)(a1 + 2);
  if ( v2 != DAT_0055a7b0 )
  {
    if ( v2 == 9 )
    {
LABEL_9:
      if ( !DAT_0055a7b8 )
      {
        *(DWORD *)(a1 + 88) = -1;
        *(WORD *)(a1 + 134) = 4;
      }
      return;
    }
    if ( v2 != 10 )
    {
      return;
    }
  }
  if ( v2 == 9 || v2 == 10 )
  {
    goto LABEL_9;
  }
  if ( v2 == DAT_0055a7b0 )
  {
    if ( DAT_0055a7b8 == 20 )
    {
      *(DWORD *)(a1 + 28) = 1108082688;
      *(DWORD *)(a1 + 88) = -1;
      PlayBuffer(108, 0, 0);
      v1 = DAT_0055a7b8;
    }
    if ( v1 >= 0 )
    {
      v3 = (float *)(a1 + 28);
      *(float *)(a1 + 28) = DAT_0055a7bc + *(float *)(a1 + 28);
      DAT_0055a7bc = DAT_0055a7bc + 1.5;
      if ( *(float *)(a1 + 28) >= 90.0 )
      {
        *v3 = *v3 - (double)DAT_0055a7b8;
        DAT_0055a7bc = 2.0;
        v4 = *(float *)(a1 + 20);
        v5 = *(float *)(a1 + 24);
        Position[0] = *(float *)(a1 + 16);
        v6 = *(DWORD *)v3 == 1117782016;
        Position[1] = v4;
        Position[2] = v5;
        if ( v6 )
        {
          v7 = 10;
          do
          {
            Position[0] = (double)(rand() % 300) - 150.0 + *(float *)(a1 + 16);
            Position[1] = *(float *)(a1 + 20) - ((double)(rand() % 20) + 600.0);
            Particle_Spawn(1221, Position, (float *)(a1 + 28), (float *)(a1 + 232), 0, 1.0, 0);
            --v7;
          }
          while ( v7 );
        }
      }
      if ( !DAT_0055a7b8 )
      {
        *(DWORD *)(a1 + 88) = -2;
        *v3 = 90.0;
        FUN_004fa5a0();
        AddTerrainAttributeRange(13, 70, 3, 6, 8u, 0);
      }
      --DAT_0055a7b8;
    }
  }
}
