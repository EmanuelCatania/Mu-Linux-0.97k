// Entity_DrawAt.cpp
// Entity_DrawAt @ 0x00505A10  (73 lines, 23 basic blocks)
//
// Renderiza una entidad como sprite en una posición/ángulo dados.
// Llamada por Entity_Render_Sprites para la entidad principal y para los nodos
// de la cadena de trail (tipo 0x35f).
//
// ── SIGNATURA ─────────────────────────────────────────────────────────────────
//
//   void __cdecl FUN_00505a10(
//     int   param_1,   // entity ptr
//     int   param_2,   // class code (entity type)
//     uint  param_3,   // slot / extra param (unused here, passed through)
//     float*param_4,   // angle_ptr (terrain slope angle XYZ)
//     float param_5,   // rotation angle (world rot)
//     uint  param_6,   // state flags
//     byte  param_7,   // misc byte
//     char  param_8,   // bone select flag: '\0' → use entity's own bone ptr, else use DAT_06970a9c
//     byte  param_9,   // extra byte → this[+0x99]
//     char  param_10,  // passed to FUN_004404e0 and FUN_00505970
//     int   param_11,  // scale mode: 0=1.0, 1=shadow, 2=0.8, 3=1.4
//     uint  param_12   // extra uint → FUN_00504b50
//   )
//
// ── GUARD ─────────────────────────────────────────────────────────────────────
//
//   if (param_5 <= _DAT_005524f8) return   // cull: rot <= tile_scale → too small/far, skip
//
// ── CLASS REMAP ───────────────────────────────────────────────────────────────
//
//   if param_2 == 0x35c:
//     sub = (param_6 >> 3) & 0xf
//     if sub == 0: param_2 = 0x3b3
//     if sub == 2: param_2 = 0x3b4
//   this = DAT_05828d58 + param_2 * 0xbc   → sprite model entry
//
// ── COPIA DE ESTADO DESDE ENTIDAD ────────────────────────────────────────────
//
//   this[+0x99] = param_9
//   this[+0x68] = entity[+0x0c]      (animation frame float)
//   this[+0x45] = entity[+0xe4]      (byte misc A)
//   this[+0x44] = entity[+0xdc]      (byte misc B)
//   this[+0x6c] = entity[+0x10]      (world X)
//   this[+0x70] = entity[+0x14]      (world Y)
//   this[+0x74] = entity[+0x18]      (world Z)
//   DAT_005597c8 = 1.0f              (global scale = 1.0 por defecto)
//
// ── SCALE POR PARAM_11 ────────────────────────────────────────────────────────
//
//   param_11 == 3 → DAT_005597c8 = 1.4f  (agrandado)
//   param_11 == 2 → DAT_005597c8 = 1.2f  (ligeramente agrandado)
//   param_11 == 1 → shadow pass:
//     if entity[+0x84] == '\x04':
//       this[+0x48] = 0.02f, this[+0x4c] = 0.1f   (shadow color dark)
//     else:
//       this[+0x48] = 0.1f,  this[+0x4c] = 0.025f  (shadow color medium)
//     this[+0x50] = 0
//     FUN_00505970(this, entity, 0x40, param_10, 0.8f)  → Sprite_DrawShadow(model, e, 64, ...)
//     (segundo pass con colores invertidos)
//
// ── SETUP ANIMACIÓN ──────────────────────────────────────────────────────────
//
//   FUN_004fa930(entity, this)   → Sprite_SetupAnim(entity, model)
//                                  Copia datos de animación desde entidad al modelo
//
// ── BONE POINTER ──────────────────────────────────────────────────────────────
//
//   if param_8 == '\0': puVar2 = *(undefined**)(entity + 0x114)  (hueso propio)
//   else:               puVar2 = &DAT_06970a9c                   (hueso raíz global)
//
// ── DRAW CALL ─────────────────────────────────────────────────────────────────
//
//   FUN_004404e0(this, puVar2, entity+0x118, entity+0x124, entity+0x130, param_10)
//     → Sprite_DrawBone(model, bone, anim_A, anim_B, anim_C, blend_factor)
//
//   FUN_00504b50(entity, class, angle_ptr, rot, state_flags, misc_byte, scale_mode, extra)
//     → Entity_DrawSprite(entity, class, angle, rot, flags, byte, mode, extra)
//     Función final que hace los calls OpenGL reales (glVertex, glTexCoord, etc.)
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   DAT_05828d58  → sprite model table (stride 0xbc por clase)
//   DAT_005524f8  → g_TileScale (culling threshold)
//   DAT_005597c8  → global sprite scale factor
//   FUN_004fa930  → Sprite_SetupAnim(entity, model)
//   FUN_00505970  → Sprite_DrawShadow(model, entity, param, blend, alpha)
//   FUN_004404e0  → Sprite_DrawBone(model, bone, anim[3], blend)
//   FUN_00504b50  → Entity_DrawSprite(entity, class, angle_ptr, rot, flags, byte, mode, extra)
//   DAT_06970a9c  → root bone data (pose matrix, usado como fallback)

#include "stdafx.h"
#include "Render/Entity_DrawAt.h"

// Entity_DrawAt @ 0x00505A10 (73 lines)
// Sets up model state (pos, scale, flags) and draws entity sprite + shadow.
// param_11: LOD level (0=normal, 1=shadow, 2=mid, 3=far)
void __cdecl Entity_DrawAt(int param_1, int param_2, undefined4 param_3, float *param_4,
                           float param_5, uint param_6, byte param_7, char param_8,
                           undefined1 param_9, char param_10, int param_11, uint param_12)
{
    void      *pModel;
    uint       uVar1;
    undefined *puVar2;

    if (_DAT_005524f8 >= param_5)   // cull: scale too small
        return;

    // Resolve sub-class for DarkKnight (0x35c → 0x3b3/0x3b4)
    if (param_2 == 0x35c) {
        uVar1 = (int)param_6 >> 3 & 0xf;
        if      (uVar1 == 0) param_2 = 0x3b3;
        else if (uVar1 == 2) param_2 = 0x3b4;
    }

    pModel = (void *)(DAT_05828d58 + param_2 * 0xbc);

    // Copy entity fields into model block
    *(undefined1 *)((int)pModel + 0x99) = param_9;
    *(undefined4 *)((int)pModel + 0x68) = *(undefined4 *)(param_1 + 0xc);    // scale
    *(undefined1 *)((int)pModel + 0x45) = *(undefined1 *)(param_1 + 0xe4);   // flag A
    *(undefined1 *)((int)pModel + 0x44) = *(undefined1 *)(param_1 + 0xdc);   // flag B
    *(undefined4 *)((int)pModel + 0x6c) = *(undefined4 *)(param_1 + 0x10);   // world X
    *(undefined4 *)((int)pModel + 0x70) = *(undefined4 *)(param_1 + 0x14);   // world Y
    *(undefined4 *)((int)pModel + 0x74) = *(undefined4 *)(param_1 + 0x18);   // world Z

    // LOD transparency scale
    // NOTE: _DAT_005597c8 is typed `float` in globals.cpp. Writing the raw
    // bit pattern as integer (0x3f800000) causes an int→float conversion,
    // which stored 1065353216.0f instead of 1.0f → vertex positions
    // scaled by ~1e9. Use float literals directly.
    _DAT_005597c8 = 1.0f;
    if      (param_11 == 3) _DAT_005597c8 = 1.4f;
    else if (param_11 == 2) _DAT_005597c8 = 1.2f;
    else if (param_11 == 1) {
        // Shadow draw: two-pass with different alpha
        if (*(char *)(param_1 + 0x84) == '\x04') {
            *(undefined4 *)((int)pModel + 0x48) = 0x3ca3d70a;  // 0.02
            *(undefined4 *)((int)pModel + 0x4c) = 0x3dcccccd;  // 0.1
        } else {
            *(undefined4 *)((int)pModel + 0x48) = 0x3dcccccd;  // 0.1
            *(undefined4 *)((int)pModel + 0x4c) = 0x3cf5c28f;  // 0.03
        }
        *(undefined4 *)((int)pModel + 0x50) = 0;
        FUN_00505970(pModel, (void*)param_1, 0x40, param_10, 0x3f99999a);  // Sprite_DrawShadow pass 1

        if (*(char *)(param_1 + 0x84) == '\x04') {
            *(undefined4 *)((int)pModel + 0x48) = 0x3e23d70a;  // 0.16
            *(undefined4 *)((int)pModel + 0x4c) = 0x3f333333;  // 0.7
        } else {
            *(undefined4 *)((int)pModel + 0x48) = 0x3f333333;  // 0.7
            *(undefined4 *)((int)pModel + 0x4c) = 0x3e4ccccd;  // 0.2
        }
        *(undefined4 *)((int)pModel + 0x50) = 0;
        FUN_00505970(pModel, (void*)param_1, 0x40, param_10, 0x3f8a3d71);  // Sprite_DrawShadow pass 2
    }

    FUN_004fa930(param_1, (int)pModel);   // Sprite_SetupAnim

    // Bone selection: param_8 = use entity's bone ptr or fallback to root
    if (param_8 == '\0')
        puVar2 = *(undefined **)(param_1 + 0x114);
    else
        puVar2 = (undefined*)&DAT_06970a9c;

    FUN_004404e0(pModel, (int)puVar2,
                 (float *)(param_1 + 0x118),
                 (float *)(param_1 + 0x124),
                 (float *)(param_1 + 0x130),
                 param_10);   // Sprite_DrawBone(model, bone, anim[3], blend)

    FUN_00504b50(param_1, param_2, param_4, param_5, param_6,
                 param_7, param_11, param_12);   // Entity_DrawSprite
}
