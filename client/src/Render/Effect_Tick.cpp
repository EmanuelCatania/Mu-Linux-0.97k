// Effect_Tick.cpp
// Per-frame tick dispatchers for effect, joint, and particle pools.
//
// MoveEffects @ 0x0046b790 — Effect_TickAll
// MoveJoints @ 0x004736e0 — Joint_TickAll
// Effect_DrawRing @ 0x00473ea0 — Effect_DrawRing
// RenderPlane @ 0x00474f90 — Effect_DrawQuad
// MovePlanes @ 0x00475090 — Effect_TickFade
// MovePointers @ 0x004794a0 — Effect_TickFlare
// MovePoints @ 0x00479380 — Effect_TickSpark
//
// Effect pool:  DAT_07b11670, stride 0x1bc bytes (0x6f * sizeof(float))
// Joint pool:   DAT_07b27150, stride 0x9d8 bytes

#include "stdafx.h"
#include "Render/Gfx.h"
#include "Render/BufferedDraw.h"
#include "Platform/GuardedOperation.h"
#include "Platform/Text.h"
extern "C" void DbgLogPublic(const char* msg);


// MoveEffects — Effect_TickAll
// Iterates the effect pool (base DAT_07b11670, stride 0x1bc, end 0x7b27150).
// For each active slot (first byte != 0), calls MoveEffect(slot, index).
// IDA: MoveEffects
void Effect_TickAll(void)
{
  // Pool real DAT_07b11670[200 × 0x1bc].
  float *pfVar1 = (float *)DAT_07b11670;
  const uintptr_t poolEnd = (uintptr_t)DAT_07b11670 + sizeof(DAT_07b11670);
  int iVar2 = 0;
  do {
    if (*(char *)pfVar1 != '\0') {
      // MoveEffect valida owners en los pools y en cada rama que requiere uno.
      // Windows conserva ademas su captura SEH y diagnostico heredados.
      if (!Platform::RunValidatedOperation([&]() {
        MoveEffect(pfVar1,iVar2);
      })) {
        const int   type    = (int)*(short*)((char*)pfVar1 + 2);
        const int   subType = *(int*)((char*)pfVar1 + 4);
        const int   owner   = *(int*)((char*)pfVar1 + 252);
        static unsigned char s_seen[2048] = {0};
        if (type >= 0 && type < 2048 && !s_seen[type]) {
          s_seen[type] = 1;
          char dbg[176];
          Platform::TextFormat(dbg, "OWNERDBG: MoveEffect crash -> Type=%d SubType=%d owner=0x%08X (slot %d, desactivado)",
                    type, subType, (unsigned)owner, iVar2);
          DbgLogPublic(dbg);
        }
        *(char *)pfVar1 = '\0';   // matar el efecto para no repetir el crash
      }
    }
    pfVar1 = pfVar1 + 0x6f;
    iVar2 = iVar2 + 1;
  } while ((uintptr_t)pfVar1 < poolEnd);
}


// MoveJoints — Joint_TickAll
// Llama a MoveJoint(joint, índice) para cada joint activo del pool.
// IDA: MoveJoints
void Joint_TickAll(void)
{
  JOINT* joints = AsJoint(DAT_07b27150);
  const uint count = (uint)(sizeof(DAT_07b27150) / sizeof(JOINT));   // 500
  for (uint i = 0; i < count; ++i) {
    if (joints[i].Live != 0) {
      MoveJoint((undefined1 *)&joints[i], i);
    }
  }
}


// Effect_DrawRing — Effect_DrawRing
// Draws a cylindrical ring effect by emitting GL_QUADS segments along a helical
// arc. Uses EulerToMatrix to build rotation matrix from Euler angles, and
// Vector_Rotate (EulerToMatrix3x4) to transform each ring-segment midpoint.
// param_1: texture slot
// param_2: center position float[3]
// param_3/4/5: radii bit-patterns (float bits in undefined4) for inner/outer ring corners
// param_6: z offset base
// param_7: alpha channel param (color for inner verts, packed bits)
// param_8: V-coord offset
//
// El decompile de Ghidra tiene decenas de variables `local_<N>` declaradas
// escalares pero usadas como vec3 contiguos pasados a Vector_Rotate y luego
// leídos en bloque por glVertex3fv con `((int)&local_f0 + iVar5)` y stride de
// 12. MSVC no preserva ese layout, así que se usan arrays float[4][3]
// explícitos. Mismo patrón que Camera_BuildMouseRay (mouse-ray) y
// FUN_004f70b0 (terrain normals).
void __cdecl
Effect_DrawRing(int param_1,float *param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,
            float param_6,undefined4 param_7,float param_8)
{
  // 4 quad vertices: positions (12 contiguous floats), colors (12), UVs (8 = 4×2).
  float verts [4][3];
  unsigned int colors[4][3];   // stored as bit-patterns; reinterpreted by glColor3fv as float
  float uv    [4][2];
  float matA  [12], matB[12];
  float angles[3];
  float input_vec[3];

  GL_BindTextureSlot(param_1);

  // Verts 0,1: white (1.0f bits = 0x3f800000); Verts 2,3: param_7 alpha bits.
  colors[0][0] = colors[0][1] = colors[0][2] = 0x3f800000;
  colors[1][0] = colors[1][1] = colors[1][2] = 0x3f800000;
  colors[2][0] = colors[2][1] = colors[2][2] = (unsigned int)param_7;
  colors[3][0] = colors[3][1] = colors[3][2] = (unsigned int)param_7;

  float t = 0.0f;
  float fVar2 = 0.0f;
  do {
    float fVar1 = t * _DAT_00552aac;
    fVar2 = t + _DAT_0055256c;
    float fVar3 = fVar2 * _DAT_00552aac;

    // UV ramp — TEXCOORD(dest, U-float, V-bits) writes [U as float][V as int-bits]
    TEXCOORD(&uv[0][0], fVar1, 0x3f800000);
    TEXCOORD(&uv[1][0], fVar3, 0x3f800000);
    TEXCOORD(&uv[2][0], fVar3, 0);
    TEXCOORD(&uv[3][0], fVar1, 0);

    // Two rotation matrices around Z at angle1 / angle2.
    angles[0] = 0.0f;
    angles[1] = 0.0f;
    angles[2] = t * _DAT_0055284c + param_6;
    EulerToMatrix(angles, matA);
    angles[2] = fVar2 * _DAT_0055284c + param_6;
    EulerToMatrix(angles, matB);

    // Vert 0: matA * (0, param_3, 0) + center
    input_vec[0] = 0.0f;
    *(unsigned int*)&input_vec[1] = (unsigned int)param_3;
    input_vec[2] = 0.0f;
    Vector_Rotate(input_vec, matA, &verts[0][0]);
    verts[0][0] += param_2[0];
    verts[0][1] += param_2[1];
    verts[0][2] += param_2[2];

    // Vert 1: matB * (0, param_3, 0) + center
    input_vec[0] = 0.0f;
    *(unsigned int*)&input_vec[1] = (unsigned int)param_3;
    input_vec[2] = 0.0f;
    Vector_Rotate(input_vec, matB, &verts[1][0]);
    verts[1][0] += param_2[0];
    verts[1][1] += param_2[1];
    verts[1][2] += param_2[2];

    // Vert 2: matB * (0, param_4, 0) + center  (param_5 also stored at +8 in original but unused for vec3 input)
    input_vec[0] = 0.0f;
    *(unsigned int*)&input_vec[1] = (unsigned int)param_4;
    *(unsigned int*)&input_vec[2] = (unsigned int)param_5;
    Vector_Rotate(input_vec, matB, &verts[2][0]);
    verts[2][0] += param_2[0];
    verts[2][1] += param_2[1];
    verts[2][2] += param_2[2];

    // Vert 3: matA * (0, param_4, param_5) + center
    input_vec[0] = 0.0f;
    *(unsigned int*)&input_vec[1] = (unsigned int)param_4;
    *(unsigned int*)&input_vec[2] = (unsigned int)param_5;
    Vector_Rotate(input_vec, matA, &verts[3][0]);
    verts[3][0] += param_2[0];
    verts[3][1] += param_2[1];
    verts[3][2] += param_2[2];

    BufferedDraw::Begin(BufferedDraw::Primitive::Quads);  // GL_QUADS
    for (int i = 0; i < 4; i++) {
      BufferedDraw::TexCoord2f(uv[i][0], param_8 + uv[i][1]);
      BufferedDraw::Color3fv((const float*)&colors[i][0]);
      BufferedDraw::Vertex3fv(&verts[i][0]);
    }
    BufferedDraw::End();
    t = fVar2;
  } while (fVar2 < _DAT_00552664);
  return;
}


// RenderPlane — Effect_DrawQuad
// Draws a world-space textured quad (GL_QUADS) at param_2 position,
// rotated by param_4 around Z axis, with half-size param_3.
// Uses GL_SetBlendAdditive to set blend mode.
void __cdecl RenderPlane(int param_1,undefined4 *param_2,float param_3,undefined4 param_4)
{
  float local_30;
  float local_2c;
  undefined4 local_28;
  float local_24;
  float local_20;
  undefined4 local_1c;
  float local_18;
  float local_14;
  undefined4 local_10;
  float local_c;
  float local_8;
  undefined4 local_4;

  GL_BindTextureSlot(param_1);
  GL_SetBlendAdditive();
  Gfx::PushMatrix();
  Gfx::Translate(*(float*)&param_2[0], *(float*)&param_2[1], *(float*)&param_2[2]);
  // 0x3f800000 son los bits de 1.0f: pasarlos como int haría int→float
  // (1065353216.0f) en el eje Z y en los UVs. Van como floats.
  Gfx::Rotate(*(float*)&param_4, 0.0f, 0.0f, 1.0f);
  local_30 = -param_3;
  local_28 = 0;
  local_24 = param_3;
  local_1c = 0;
  local_14 = param_3;
  local_10 = 0;
  local_c = param_3;
  local_8 = param_3;
  local_4 = 0;
  local_2c = local_30;
  local_20 = local_30;
  local_18 = local_30;
  BufferedDraw::Begin(BufferedDraw::Primitive::Quads);   // GL_QUADS
  BufferedDraw::TexCoord2f(0.0f, 1.0f);
  BufferedDraw::Vertex3fv(&local_30);
  BufferedDraw::TexCoord2f(1.0f, 1.0f);
  BufferedDraw::Vertex3fv(&local_18);
  BufferedDraw::TexCoord2f(1.0f, 0.0f);
  BufferedDraw::Vertex3fv(&local_c);
  BufferedDraw::TexCoord2f(0.0f, 0.0f);
  BufferedDraw::Vertex3fv(&local_24);
  BufferedDraw::End();
  Gfx::PopMatrix();
  GL_ResetState();
  return;
}


// IDA: MovePlanes (0x00475090), mismo pool que Player_Render (100 OBJECT).
void Effect_TickFade(void)
{
  for (int index=0;index<100;++index) {
    char *slot=g_PlayerRenderPool+index*0x1bc;
    if (!slot[0]) continue;
    int &life=*(int *)(slot+0x60);
    --life;
    if (life<=0) slot[0]=0;
    Joint_BoneOffsetApply(reinterpret_cast<BYTE*>(slot),1);
    // IDA lee/escribe estos campos como float, salvo el contador de vida.
    if (life<10) {
      const double fade=(double)life*(double)_DAT_005524f4;
      *(float *)(slot+0xe8)=(float)(fade*(double)_DAT_00552534);
      *(float *)(slot+0xec)=(float)fade;
      *(float *)(slot+0xf0)=(float)(fade*(double)_DAT_00552530);
    }
    *(float *)(slot+0x24)-=(float)life;
  }
}

// MovePointers — Effect_TickFlare
// Iterates the flare-effect pool (base DAT_07c82cdc, stride 0x1c*4=0x70,
// end 0x7c8589b). Per-tick: decrement life counter; handle sub-type state
// machine for position and alpha.
// IDA: MovePointers
void Effect_TickFlare(void)
{
  float fVar1;
  float fVar2;
  float *pfVar3;

  // IDA: anclaje +12 de Pointers, 100 slots de 0x70.
  pfVar3 = (float *)(g_PointerPool+12);
  const uintptr_t poolEnd = (uintptr_t)g_PointerPool + sizeof(g_PointerPool)+12;
  do {
    if (*(char *)(pfVar3 + -3) != '\0') {
      // IDA MovePointers (0x004794A0) L20-21 -> `v1 = v0[11] - 1; v0[11] = v1;` con
      // `v0` = _DWORD*: el contador y el TIPO del slot son ENTEROS, no floats.
      // Leerlos con `(int)float` convertiría numéricamente un bit-pattern (casi
      // siempre 0) y el contador nunca bajaría.
      const int __pcnt = *(int*)&pfVar3[0xb] - 1;
      *(int*)&pfVar3[0xb] = __pcnt;
      fVar2 = pfVar3[0xb];
      fVar1 = pfVar3[-2];
      const int __ptype = *(int*)&fVar1;
      if (__ptype < 0x4b7) {
        if (__ptype < 0x4b5) {
          if ((__ptype == 7) &&
             (fVar1 = *pfVar3 - _DAT_00552874, *pfVar3 = fVar1, fVar1 < _DAT_005524f4)) {
            *(undefined1 *)(pfVar3 + -3) = 0;
          }
        }
        else {
LAB_004794f9:
          if (__ptype == 1205) {
            *pfVar3 = *pfVar3 + _DAT_005524bc;
          }
          pfVar3[7] = 0.1;
          pfVar3[8] = 0.0;
          pfVar3[9] = 0.0;
          if (__pcnt < 1) {
            *(undefined1 *)(pfVar3 + -3) = 0;
          }
          if (__pcnt < 0x32) {
            pfVar3[10] = (float)__pcnt * _DAT_00552914;
          }
        }
      }
      else if (__ptype == 1240) goto LAB_004794f9;
    }
    pfVar3 = pfVar3 + 0x1c;
    if ((uintptr_t)pfVar3 >= poolEnd) {
      return;
    }
  } while( true );
}


// MovePoints — Effect_TickSpark
// = IDA `MovePoints` @0x479380: tick de los NÚMEROS DE DAÑO. Por slot activo:
// decrementa el contador de delay; cuando dispara, sube el número (pos.z +=
// lifetime), decae el lifetime 0.3, lo desactiva al llegar a 0, y encoge la
// escala 5.0 por tick con piso en 15.
//
// En el binario `unk_7C80128` no es un pool aparte: es `DAT_07c80110 + 0x18`
// — el MISMO pool de números que llena `CreatePoint` y recorre `RenderPoints`,
// sólo que las tres funciones lo abordan desde offsets distintos del slot y
// usan índices relativos.
// IDA: MovePoints
void DamageNumbers_Tick(void)
{
  // Mismo pool que CreatePoint/RenderPoints, desplazado +0x18 como en IDA.
  float *pfVar2 = (float *)((char *)&DAT_07c80110[0] + 0x18);
  const uintptr_t poolEnd = (uintptr_t)&DAT_07c80110[0] + sizeof(DAT_07c80110);
  float fVar1;

  do {
    // El contador de delay (+0x38) es un DWORD en IDA
    // (`v1 = *((_DWORD *)v0 + 8) - 1;`), no un float: se decrementa como entero.
    int *pDelay = (int *)&pfVar2[8];
    if ((*(char *)(pfVar2 + -6) != '\0') && (--(*pDelay) < 0)) {
      *pfVar2 = pfVar2[0xc] + *pfVar2;
      fVar1 = pfVar2[0xc] - _DAT_005528b8;
      pfVar2[0xc] = fVar1;
      if (fVar1 <= _DAT_00552580) {
        *(undefined1 *)(pfVar2 + -6) = 0;
      }
      fVar1 = pfVar2[-3] - _DAT_00552660;
      pfVar2[-3] = fVar1;
      if (fVar1 < _DAT_00552834) {
        pfVar2[-3] = 15.0;
      }
    }
    pfVar2 = pfVar2 + 0x1c;
  } while ((uintptr_t)pfVar2 < poolEnd);
}
