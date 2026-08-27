// SkillEffect_Render.cpp
// SkillEffect_Render @ 0x0046CB70  (44 lines, decompile completo)
//
// Renderiza todos los efectos de habilidades activos. Soporta dos modos:
// en-mundo 2D (g_GameSubState == 2) y billboard 3D para otras escenas.
//
// ── POOL DE EFECTOS ───────────────────────────────────────────────────────────
//
//   Base:   DAT_07c5ab3c  (inicio de iteración, pfVar1[-3] = flag activo)
//   Stride: 0x1c * 4 = 0x70 bytes por efecto (pfVar1 += 0x1c por iteración)
//   Límite: pfVar1 < 0x7c602bc
//   Entradas: (0x7c602bc - 0x07c5ab3c) / 0x70 = 0x5780/112 = 200 efectos
//
//   Struct de efecto (relativo a pfVar1):
//     pfVar1[-3]    — char: active flag
//     pfVar1[-2]    — int/float: tipo de efecto (texture ID)
//                     0 (1.42932e-43 = 0x00000001 float): tipo especial grande (20×1)
//                     otros: tipo normal (3×3)
//     pfVar1[+0]    — float: escala / intensidad
//     pfVar1[+1..3] — float[3]: posición XYZ
//     pfVar1[+4..6] — float[3]: rotación XYZ (Euler angles)
//     pfVar1[+7..N] — datos adicionales (pasados a FUN_00511d00)
//
// ── DECOMPILE COMPLETO ────────────────────────────────────────────────────────
//
//   void FUN_0046cb70(void)
//   {
//     // Modo: update timing vs GL state
//     if (g_GameSubState == 2 || g_GameSubState == 7 || g_GameSubState == 10)
//       GL_SetBlendAdditive();           → Frame_UpdateTimer()
//     else
//       GL_SetBlendSrcOver('\x01');     → GL_SetMode(1) — 2D ortho setup para otras escenas
//
//     glColor3f(1.0, 1.0, 1.0);  // color blanco (textura sin tinte)
//
//     pfVar1 = &DAT_07c5ab3c;
//     do {
//       if (pfVar1[-3] != '\0') {             // efecto activo
//         BindTexture(*((_DWORD *)v0 - 2));    ← DWORD, no float (ver fix 2026-08-15)
//
//         if (g_GameSubState == 2) {
//           // Modo en-mundo: draw 2D en espacio mundo
//           FUN_00511d00((int)pfVar1[-2], pfVar1+1, *pfVar1, *pfVar1,
//                        pfVar1+7, 0.0, 0.0, 0.0, 1.0, 1.0);
//                        → SkillEffect_Draw2D(type, pos[3], r, g, scale, 0,0,0, 1, 1)
//         }
//         else {
//           // Modo billboard: draw 3D con matriz de rotación
//           glPushMatrix();
//           glTranslatef(pfVar1[1], pfVar1[2], pfVar1[3]);  // trasladar a posición efecto
//           FUN_004f9db0(pfVar1+4, local_30);               → Matrix_FromEuler(rot[3], mat)
//           if (pfVar1[-2] == 1.42932e-43) {    // tipo 1 (1 en float = tipo especial)
//             fVar3 = 20.0; fVar2 = 1.0;        // billboard grande y delgado
//           } else {
//             fVar3 = 3.0; fVar2 = 3.0;         // billboard cuadrado estándar
//           }
//           GL_DrawBillboard(fVar2, fVar3, local_30); → SkillEffect_DrawBillboard(w, h, rot_mat)
//           glPopMatrix();
//         }
//       }
//       pfVar1 += 0x1c;   // siguiente efecto
//     } while (pfVar1 < 0x7c602bc);
//   }
//
// ── NOTA ──────────────────────────────────────────────────────────────────────
//
//   Esta función se llama dos veces en Game_RenderTick:
//   1. Siempre (línea 106) — para todas las escenas
//   2. Solo si g_GameSubState == 2 && DAT_07e118e8 not in {3, >=10} (línea 121)
//      → segunda pasada solo en modo in-world normal
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   GL_SetBlendAdditive  → Frame_UpdateTimer()
//   GL_SetBlendSrcOver  → GL_SetMode(mode)
//   GL_BindTextureSlot  → Particle_SetTexture(type) — glBindTexture
//   FUN_00511d00  → SkillEffect_Draw2D(type, pos, r, g, scale, ...)
//   FUN_004f9db0  → Matrix_FromEuler(angles[3], out_mat[12])
//   GL_DrawBillboard  → SkillEffect_DrawBillboard(width, height, rot_mat)

#include "stdafx.h"
#include "Render/SkillEffect_Render.h"
#include "globals.h"
#include "functions.h"

// SkillEffect_Render @ 0x0046CB70 (44 lines)
// Renders all active skill effects. In sub-states 2/7/10 uses timer-driven mode;
// otherwise sets GL blend mode. Each effect: bind texture, then draw 2D or billboard.
// BUG-FIX 2026-04-28: AUTO-SKIP removed — pool DAT_07c5ab3c ahora es array
// de 200 × 0x70 bytes en globals.cpp. Loop bound count-based.
//
// Pool layout per slot (start at +0x0c offset from pfVar1, so pfVar1[-3] = +0):
//   +0x00 char  active flag
//   +0x04 int   type / texture id
//   +0x0c float scale / intensity   (= pfVar1[0])
//   +0x10..0x18 float[3] position   (= pfVar1[1..3])
//   +0x1c..0x24 float[3] euler rot  (= pfVar1[4..6])
//   +0x28..    additional data passed to FUN_00511d00
void SkillEffect_Render(void)
{
    // IDA: World 2/7/10 → EnableAlphaBlend(); resto → EnableAlphaTest(1).
    if ((DAT_0055a7ac == 2) || (DAT_0055a7ac == 7) || (DAT_0055a7ac == 10))
        GL_SetBlendAdditive();       // EnableAlphaBlend (0x511710) — NO es un timer
    else
        // ── 2026-08-16: CAUSA REAL DE LOS CUADROS BLANCOS ────────────────────
        // IDA llama `EnableAlphaTest(1)` = **0x00511680**. El port llamaba
        // `GL_SetAlphaTest`, que es **DisableTexture(bool)** (0x00511590) y termina
        // con `glDisable(GL_TEXTURE_2D)` incondicional. Con el texturizado
        // apagado, cada quad se pinta con el `glColor3f(1,1,1)` de abajo = un
        // CUADRADO BLANCO. Y como el estado GL es global y queda "pegado",
        // contaminaba todo lo que se dibujara después (de ahí los cuadros
        // blancos sobre los mobs al atacar, y el Inferno "verde" del principio).
        // Por eso el probe TEXDBG dio 0 hits: la textura se bindeaba bien, sólo
        // que GL_TEXTURE_2D estaba deshabilitado.
        // Ojo con esta familia (3ra vez que muerde, ver CLAUDE.md 2026-08-10):
        //   0x00511590 DisableTexture   0x00511680 EnableAlphaTest
        //   0x00511710 EnableAlphaBlend 0x00511790 EnableAlphaBlendMinus
        GL_SetBlendSrcOver('\x01');

    glColor3f(1.0f, 1.0f, 1.0f);

    // Iterate 200 slots × 0x70 bytes; pfVar1 lands at +0x0c per slot.
    for (int slot = 0; slot < 200; ++slot) {
        float *pfVar1 = (float*)(DAT_07c5ab3c + slot * 0x70 + 0x0c);
        if (*(char *)(pfVar1 + -3) == '\0') continue;   // not active

        // ── 2026-08-15: CAUSA DE LOS "CUADROS BLANCOS" DE LOS SKILLS ─────────
        // IDA lee este campo como **DWORD**: `BindTexture(*((_DWORD *)v0 - 2))`.
        // El port hacía `(int)pfVar1[-2]`, o sea lo leía como FLOAT y lo
        // convertía NUMÉRICAMENTE. El campo guarda un entero (el id de textura),
        // así que p.ej. 102 leído como float da 1.43e-43 y `(int)` de eso es 0
        // → se bindeaba el slot 0 (sin textura) y el sprite salía blanco.
        // Afecta a TODOS los efectos de skill: esta función dibuja sus sprites.
        // Nótese que la comparación `== 102` de más abajo ya leía bien el campo
        // (`*(int*)(pfVar1 - 2)`): el mismo campo se leía de dos formas
        // distintas dentro de la misma función.
        // Mismo primo del patrón `(float)(uintptr_t)` que corrompía los joints
        // (ver CLAUDE.md 2026-08-10).
        const int texId = *(int*)(pfVar1 - 2);
        GL_BindTextureSlot(texId);             // BindTexture

        if (DAT_0055a7ac == 2) {
            // In-world: flat 2D billboard
            FUN_00511d00(texId, pfVar1 + 1, *pfVar1, *pfVar1,
                         pfVar1 + 7, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
        } else {
            // Other states: 3D billboard via Euler matrix
            float local_30[12];
            glPushMatrix();
            glTranslatef(pfVar1[1], pfVar1[2], pfVar1[3]);
            FUN_004f9db0(pfVar1 + 4, local_30);   // Matrix_FromEuler(angles, mat)

            float fVar2, fVar3;
            if (*(int *)(pfVar1 - 2) == 102) {    // IDA: integer texture type 102
                fVar3 = 20.0f; fVar2 = 1.0f;
            } else {
                fVar3 = 3.0f;  fVar2 = 3.0f;
            }
            GL_DrawBillboard(fVar2, fVar3, local_30);
            glPopMatrix();
        }
    }
}
