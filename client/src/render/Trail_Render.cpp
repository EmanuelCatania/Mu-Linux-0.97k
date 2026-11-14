// Trail_Render.cpp
// FUN_0046c3e0 @ 0x0046C3E0
//
// Trail_RenderAll — renders all active beam/trail effects (weapon trails, magic beams, etc.)
//
// Trail table: starts at DAT_07c608b4, stride 0xbc (int-words) = 0x2f0 = 752 bytes per slot.
// Array ends at 0x7C72E74.
//
// Each slot layout (relative to piVar2, which points at field +0):
//   piVar2[-3]  (char)   — active flag (non-zero = active trail)
//   piVar2[-2]  (int)    — segment count (number of trail points)
//   piVar2[-1]  (int)    — entity ptr (*piVar2 = entity ptr at +0x1be → is_swimming short)
//   piVar2[0]   (int)    — entity ptr (*(short*)(entity+0x1be) checked for swim state)
//   piVar2[1]   (int)    — segment count (duplicate / display count)
//   piVar2[2..4] (float) — RGB color (r, g, b)
//   piVar2[0x5f] (int)   — ptr to head of trail positions (3 floats per point)
//   piVar2+0x5f-0x5a — trail point array: each element = 3 floats (x,y,z)
//
// Rendering:
//   - Sets GL state via GL_BindTextureSlot(type + 0x48d)
//   - For each segment pair: glBegin(GL_QUAD_STRIP=6)
//       glColor3f with fade: alpha = (count-i)/count (or 1.0 if swimming)
//       4 vertices per strip quad (two points × two sides of the trail)
//     glEnd()
//
// Type lookup: if entity.is_swimming == 0 → type = piVar2[-2], else GL_SetBlendSrcAlpha
//
// Sub-functions:
//   GL_SetBlendAdditive — GL state A (no swimming)
//   GL_SetBlendSrcAlpha — GL state B (swimming)
//   GL_BindTextureSlot — SetTrailTexture(type)

#include "stdafx.h"
#include "render/BufferedDraw.h"

// FUN_0046c3e0 definition MOVED to Joint_Render.cpp — this duplicate removed to avoid LNK2005
