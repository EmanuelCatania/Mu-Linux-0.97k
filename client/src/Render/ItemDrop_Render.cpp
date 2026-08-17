// ItemDrop_Render.cpp
// ItemDrop_Render @ 0x00473710  (214 lines, COMPLETO)
//
// Renderiza todos los items tirados en el suelo (drops). Soporta múltiples
// tipos visuales: cofres, espadas, cadenas/proyectiles, y misceláneos.
// Usa UV interpolados para animar los segmentos de cadena y proyectiles.
//
// ── POOL DE ITEMS ─────────────────────────────────────────────────────────────
//
//   Iterador base: DAT_07b27b08 (piVar5 = &DAT_07b27b08)
//   Los campos son negativos al iterador → el struct real empieza antes.
//   La posición real del struct = piVar5 - 0x26e * 4 = piVar5 - 0x9B8
//   Stride: no determinado en las primeras 120 líneas (loop end at 0x7c602bc)
//
//   Campos del struct (relativo a piVar5):
//     piVar5[-0x26e] — char: active/visible flag
//     piVar5[-0x26d] — int: tipo visual del item
//       0x10a = cofre/caja     0x4e2 = especial
//       0x4e6 = espada A       0x4e7 = espada B (alias de 0x4e6 para tex)
//       0x4e5 = item genérico  0x4ec = cadena A
//       0x4ed = cadena B       0x4ee = misceláneo
//     piVar5[-0x26c] — int: sub-tipo (modifica textura y comportamiento)
//     piVar5[-0x261] — float: color R
//     piVar5[-0x260] — float: color G
//     piVar5[-0x25f] — float: color B
//     piVar5[-0x25a] — int: punto actual de la cadena (countdown)
//     piVar5[-0x259] — int: puntos totales de la cadena
//     *piVar5        — int: lifetime countdown (cap at 0x14 = 20)
//     piVar5[-0x25e] — posiciones de puntos (array, stride variable)
//
// ── SETUP GL POR TIPO ─────────────────────────────────────────────────────────
//
//   Items 0x4ec, 0x4ed, y 0x4e6 sub {7,10}: FUN_00511790() → GL_SetBlend2()
//   Todos los demás: FUN_00511710() → Frame_UpdateTimer()
//
//   Type 0x10a (cofre/caja) — color faded por lifetime:
//     sub 0, 4: tex 0x4fd, glColor3f(lifetime/20 * R, ...)
//     sub 1:    tex 0x4e5, glColor3f(lifetime/20 * R, ...)
//     sub 2:    tex 0x4fe, glColor3f(lifetime/20 * R, ...)
//     sub 3:    tex 0x4e5, glColor3fv(full R,G,B)
//
//   Type 0x4e7 → tex 0x4e6 (alias); glColor3fv(R,G,B)
//   Todos los otros: FUN_00511480(type); glColor3fv(R,G,B)
//
// ── LOOP DE PUNTOS (para items multi-punto: cadenas, proyectiles) ─────────────
//
//   if piVar5[-0x25a] > 0:   // item tiene segmentos
//     pfVar4 = array de posiciones (stride: 3 floats × 2 = 6 floats por punto)
//
//     Para cada punto:
//       // UV interpolation (fade out desde punta hasta cola)
//       if tipo in {0x4ec, 0x4ed} o (0x4e6 sub {7,10}):
//         local_2c = (total - current) * _DAT_005526dc    (UV lineal simple)
//         local_28 = (total - current - 1) * _DAT_005526dc
//       else:
//         local_2c = (total - current) / (total_pts - 1)  (UV normalizado)
//         local_28 = (total - current - 1) / (total_pts - 1)
//
//       // Ángulo aleatorio por lifetime
//       local_24 = (ftol() % 1000) * 0.001f
//
//       // Ajuste de UV para 0x4e6/0x4e7 (espadas):
//       local_2c = (local_2c + local_2c) - local_24
//       local_28 = (local_28 + local_28) - local_24
//
//       // Tipo 0x4ec/0x4ed (cadenas): doble offset
//       local_24 += local_24; local_2c -= local_24; local_28 -= local_24
//
//       // Tipo 0x4ee (misceláneo) — draw explícito con glBegin:
//         fVar8 = (total_pts - current) / total_pts * 2 * R   (brightness fade)
//         glColor3f(fVar8, fVar8, fVar8)
//         glBegin(GL_QUADS = 7)
//         glTexCoord2f(local_2c, 0);   glVertex3fv(pfVar4 - 1)  (punto A)
//         glTexCoord2f(local_2c, 1.0); glVertex3fv(pfVar4 + 2)  (punto B)
//         // Para tipo 0x4ee (misceláneo) — quad completo:
//         glTexCoord2f(local_2c, 0);   glVertex3fv(pfVar4 - 1)  — punto anterior A
//         glTexCoord2f(local_2c, 1.0); glVertex3fv(pfVar4 + 2)  — punto anterior B
//         glTexCoord2f(local_28, 1.0); glVertex3fv(pfVar4 + 14) — punto actual B
//         glTexCoord2f(local_28, 0);   → glVertex3fv(pfVar4 + 11) — punto actual A
//         glEnd()
//
//       Para tipo 0x10a sub {0,4} (cofre) con punto especial:
//         if (local_20 == total/2) — punto central: dibujar sprite de destello
//           centroid = average(4 puntos)
//           FUN_004795c0(0x4fd, &centroid, alpha=0.2, &color, 0,0,0)
//             → DrawSprite(tex=0x4fd, pos, scale, color, …)
//
//       Para tipo 0x4e7 (espada B) — color fade basado en lifetime:
//         if ftol() == local_20: color = blue_fade_down
//         elif ftol() < local_20: color = 0.0
//         else: color = 0.7
//
//       Para otros (0x4e6, 0x4e5, 0x4ec, 0x4ed) — quad flipped:
//         if tipo != 0x4e5 sub 2:
//           glBegin(GL_QUADS)
//           glTexCoord2f(local_2c, 1.0); glVertex3fv(pfVar4 + 5)
//           glTexCoord2f(local_2c, 0);   glVertex3fv(pfVar4 + 8)
//           glTexCoord2f(local_28, 0);   glVertex3fv(pfVar4 + 20)
//           glTexCoord2f(local_28, 1.0); glVertex3fv(pfVar4 + 17)
//           glEnd()
//         Para 0x4e6/0x4e7: ajusta UV: local_2c += 2*local_24; local_28 += 2*local_24
//         glBegin(GL_QUADS)  — quad principal (igual que 0x4ee pero UV flipped)
//         glEnd()
//
//       local_20++; pfVar4 += 0xc  — siguiente punto (stride 12 floats = 3 pos × 4)
//
// ── STRIDE Y LÍMITE DEL POOL ────────────────────────────────────────────────
//
//   piVar5 avanza += 0x276 * 4 = 0x9D8 bytes por item
//   Límite: 0x7c5b4e8  → pool de ~12-16 items simultáneos
//   piVar5[-0x26e] = active (char): si '\0', salta item
//   piVar5[-0x26d] = tipo visual (int)
//   piVar5[-0x26c] = sub-tipo (int)
//   piVar5[-0x25a] = puntos actuales (int, countdown desde -0x259)
//   piVar5[-0x259] = puntos totales (int)
//   piVar5[-0x261,-0x260,-0x25f] = color R,G,B (float)
//   *piVar5 = lifetime countdown (cap 0x14=20)
//   pfVar4 = puntero a array de posiciones (stride 3 floats × 4 = 12 por punto)
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   FUN_00511790  → GL_SetBlend2() — blend mode especial para cadenas/transparentes
//   FUN_00511710  → Frame_UpdateTimer()
//   FUN_00511480  → Particle_SetTexture(type) — glBindTexture
//   DAT_005526dc  → UV step constant
//   DAT_00552500  → lifetime-to-UV scale (0.001f aprox)
//   glBegin(7)    → GL_QUADS
//   glColor3f     → OpenGL color sin alpha
//   glColor3fv    → OpenGL color desde array
//   glTexCoord2f  → OpenGL UV coords
//   glVertex3fv   → OpenGL vertex position

#include "stdafx.h"
#include "Render/ItemDrop_Render.h"
