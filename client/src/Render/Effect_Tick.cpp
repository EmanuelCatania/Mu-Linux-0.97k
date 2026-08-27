// Effect_Tick.cpp
// Per-frame tick dispatchers for effect, joint, and particle pools.
//
// FUN_0046b790 @ 0x0046b790 — Effect_TickAll
// FUN_004736e0 @ 0x004736e0 — Joint_TickAll
// FUN_00473ea0 @ 0x00473ea0 — Effect_DrawRing
// FUN_00474f90 @ 0x00474f90 — Effect_DrawQuad
// FUN_00475090 @ 0x00475090 — Effect_TickFade
// FUN_004794a0 @ 0x004794a0 — Effect_TickFlare
// FUN_00479380 @ 0x00479380 — Effect_TickSpark
//
// Effect pool:  DAT_07b11670, stride 0x1bc bytes (0x6f * sizeof(float))
// Joint pool:   DAT_07b27150, stride 0x9d8 bytes

#include "stdafx.h"
extern "C" void DbgLogPublic(const char* msg);   // OWNERDBG (temporal)


// FUN_0046b790 — Effect_TickAll
// Iterates the effect pool (base DAT_07b11670, stride 0x1bc, end 0x7b27150).
// For each active slot (first byte != 0), calls FUN_00466ad0(slot, index).
// IDA: FUN_0046b790
void Effect_TickAll(void)
{
  // BUG-FIX 2026-04-28: pool real DAT_07b11670[200 × 0x1bc] (2026-08-15: era 124).
  float *pfVar1 = (float *)DAT_07b11670;
  const uintptr_t poolEnd = (uintptr_t)DAT_07b11670 + sizeof(DAT_07b11670);
  int iVar2 = 0;
  do {
    if (*(char *)pfVar1 != '\0') {
      // ── OWNERDBG (temporal, 2026-08-16) — red de seguridad + diagnostico ───
      // Tras corregir la lectura del owner (slot+252, se leia como float y daba
      // ~0 en 39 sitios), las ramas que comparan `owner == Hero` por fin se
      // ejecutan — y ahi aparecio un AV `0xC0000005 param1=0x10`, o sea un
      // deref de `owner + 0x10` con **owner NULL**: algun caller crea el efecto
      // sin dueno. En el binario eso no pasa (el owner siempre es valido para
      // los tipos que lo deref), asi que el original no valida nada.
      // Hasta ubicar al caller, contenemos el crash: se desactiva ese slot y se
      // loguea su Type/SubType UNA vez por tipo — ese dato identifica al
      // culpable sin tener que reproducir con debugger.
      __try {
        FUN_00466ad0(pfVar1,iVar2);
      }
      __except (EXCEPTION_EXECUTE_HANDLER) {
        const int   type    = (int)*(short*)((char*)pfVar1 + 2);
        const int   subType = *(int*)((char*)pfVar1 + 4);
        const int   owner   = *(int*)((char*)pfVar1 + 252);
        static unsigned char s_seen[2048] = {0};
        if (type >= 0 && type < 2048 && !s_seen[type]) {
          s_seen[type] = 1;
          char dbg[176];
          wsprintfA(dbg, "OWNERDBG: MoveEffect crash -> Type=%d SubType=%d owner=0x%08X (slot %d, desactivado)",
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


// FUN_004736e0 — Joint_TickAll
// Iterates the joint pool (base DAT_07b27150, stride 0x9d8, end 0x7c5ab30).
// For each active slot (first byte != 0), calls FUN_00470030(slot, index).
// IDA: FUN_004736e0
void Joint_TickAll(void)
{
  // BUG-FIX 2026-04-28: pool real DAT_07b27150[500 × 0x9d8] (2026-08-15: era 200).
  char *pcVar1 = DAT_07b27150;
  const uintptr_t poolEnd = (uintptr_t)DAT_07b27150 + sizeof(DAT_07b27150);
  uint uVar2 = 0;
  do {
    if (*pcVar1 != '\0') {
      FUN_00470030((undefined1 *)pcVar1, (uint)uVar2);
    }
    pcVar1 = pcVar1 + 0x9d8;
    uVar2 = uVar2 + 1;
  } while ((uintptr_t)pcVar1 < poolEnd);
}


// FUN_00473ea0 — Effect_DrawRing
// Draws a cylindrical ring effect by emitting GL_QUADS segments along a helical
// arc. Uses FUN_004f9e90 to build rotation matrix from Euler angles, and
// Vector_Rotate (EulerToMatrix3x4) to transform each ring-segment midpoint.
// param_1: texture slot
// param_2: center position float[3]
// param_3/4/5: radii bit-patterns (float bits in undefined4) for inner/outer ring corners
// param_6: z offset base
// param_7: alpha channel param (color for inner verts, packed bits)
// param_8: V-coord offset
//
// BUG-FIX 2026-04-26: el Ghidra-decomp original tenía decenas de variables
// `local_<N>` declaradas escalares pero usadas como vec3 contiguos pasados a
// Vector_Rotate (matrix×vector) y luego leídos en bloque por glVertex3fv con
// `((int)&local_f0 + iVar5)` y stride de 12. MSVC no preserva ese layout →
// las posiciones de los 4 vértices del quad eran basura → en char-select el
// efecto rojo del char seleccionado se renderizaba como una línea horizontal
// en el suelo (mile-of-vertices) en vez de un anillo vertical alrededor del
// personaje. Reemplazado por arrays float[4][3] explícitos.
// Mismo patrón que Camera_BuildMouseRay (mouse-ray) y FUN_004f70b0 (terrain normals).
void __cdecl
FUN_00473ea0(int param_1,float *param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,
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

    // UV ramp — FUN_00511bf0(dest, U-float, V-bits) writes [U as float][V as int-bits]
    FUN_00511bf0(&uv[0][0], fVar1, 0x3f800000);
    FUN_00511bf0(&uv[1][0], fVar3, 0x3f800000);
    FUN_00511bf0(&uv[2][0], fVar3, 0);
    FUN_00511bf0(&uv[3][0], fVar1, 0);

    // Two rotation matrices around Z at angle1 / angle2.
    angles[0] = 0.0f;
    angles[1] = 0.0f;
    angles[2] = t * _DAT_0055284c + param_6;
    FUN_004f9e90(angles, matA);
    angles[2] = fVar2 * _DAT_0055284c + param_6;
    FUN_004f9e90(angles, matB);

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

    glBegin(7);  // GL_QUADS
    for (int i = 0; i < 4; i++) {
      glTexCoord2f(uv[i][0], param_8 + uv[i][1]);
      glColor3fv((const GLfloat*)&colors[i][0]);
      glVertex3fv(&verts[i][0]);
    }
    glEnd();
    t = fVar2;
  } while (fVar2 < _DAT_00552664);
  return;
}


// FUN_00474f90 — Effect_DrawQuad
// Draws a world-space textured quad (GL_QUADS) at param_2 position,
// rotated by param_4 around Z axis, with half-size param_3.
// Uses GL_SetBlendAdditive to set blend mode.
void __cdecl FUN_00474f90(int param_1,undefined4 *param_2,float param_3,undefined4 param_4)
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
  glPushMatrix();
  glTranslatef(*(float*)&param_2[0],*(float*)&param_2[1],*(float*)&param_2[2]);
  // BUG-FIX: 0x3f800000 son los bits de 1.0f. Pasarlos como int → C
  // los castea int→float = 1065353216.0f → eje Z mal definido +
  // glRotatef tira basura. Idem para los UVs (0x3f800000 → 1.06e9).
  glRotatef(*(float*)&param_4, 0.0f, 0.0f, 1.0f);
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
  glBegin(7);   // GL_QUADS
  glTexCoord2f(0.0f, 1.0f);
  glVertex3fv(&local_30);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3fv(&local_18);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3fv(&local_c);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3fv(&local_24);
  glEnd();
  glPopMatrix();
  GL_ResetState();
  return;
}


// FUN_00475090 — Effect_TickFade
// Iterates the fade-effect pool (base DAT_07c74ec8, stride 0x6f*4=0x1bc,
// end 0x7c7fc38). For active slots: decrements lifetime counter; if
// counter < 10 fades colour toward black; subtracts counter from Z velocity.
// IDA: FUN_00475090
void Effect_TickFade(void)
{
  // BUG-FIX 2026-04-28: pool real DAT_07c74ec8[40 × 0x1bc].
  int *piVar3 = (int*)DAT_07c74ec8;
  const uintptr_t poolEnd = (uintptr_t)DAT_07c74ec8 + sizeof(DAT_07c74ec8);
  int iVar1;
  float fVar2;

  do {
    if ((char)piVar3[-0x18] != '\0') {
      iVar1 = *piVar3;
      *piVar3 = iVar1 + -1;
      if (iVar1 + -1 < 1) {
        *(undefined1 *)(piVar3 + -0x18) = 0;
      }
      FUN_00465fe0((int)(piVar3 + -0x18),1);
      iVar1 = *piVar3;
      if (iVar1 < 10) {
        fVar2 = (float)iVar1 * _DAT_005524f4;
        piVar3[0x22] = (int)(fVar2 * _DAT_00552534);
        piVar3[0x23] = (int)fVar2;
        piVar3[0x24] = (int)(fVar2 * _DAT_00552530);
      }
      piVar3[-0xf] = (int)((float)piVar3[-0xf] - (float)iVar1);
    }
    piVar3 = piVar3 + 0x6f;
  } while ((uintptr_t)piVar3 < poolEnd);
}


// FUN_004794a0 — Effect_TickFlare
// Iterates the flare-effect pool (base DAT_07c82cdc, stride 0x1c*4=0x70,
// end 0x7c8589b). Per-tick: decrement life counter; handle sub-type state
// machine for position and alpha.
// IDA: FUN_004794a0
void Effect_TickFlare(void)
{
  float fVar1;
  float fVar2;
  float *pfVar3;

  // BUG-FIX 2026-04-28: pool real DAT_07c82cdc[63 × 0x70].
  pfVar3 = (float *)DAT_07c82cdc;
  const uintptr_t poolEnd = (uintptr_t)DAT_07c82cdc + sizeof(DAT_07c82cdc);
  do {
    if (*(char *)(pfVar3 + -3) != '\0') {
      fVar2 = (float)((int)pfVar3[0xb] + -1);
      pfVar3[0xb] = fVar2;
      fVar1 = pfVar3[-2];
      if ((int)fVar1 < 0x4b7) {
        if ((int)fVar1 < 0x4b5) {
          if ((fVar1 == 9.80909e-45) &&
             (fVar1 = *pfVar3 - _DAT_00552874, *pfVar3 = fVar1, fVar1 < _DAT_005524f4)) {
            *(undefined1 *)(pfVar3 + -3) = 0;
          }
        }
        else {
LAB_004794f9:
          if (fVar1 == 1.68856e-42) {
            *pfVar3 = *pfVar3 + _DAT_005524bc;
          }
          pfVar3[7] = 0.1;
          pfVar3[8] = 0.0;
          pfVar3[9] = 0.0;
          if ((int)fVar2 < 1) {
            *(undefined1 *)(pfVar3 + -3) = 0;
          }
          if ((int)fVar2 < 0x32) {
            pfVar3[10] = (float)(int)fVar2 * _DAT_00552914;
          }
        }
      }
      else if (fVar1 == 1.73761e-42) goto LAB_004794f9;
    }
    pfVar3 = pfVar3 + 0x1c;
    if ((uintptr_t)pfVar3 >= poolEnd) {
      return;
    }
  } while( true );
}


// FUN_00479380 — Effect_TickSpark
// = IDA `MovePoints` @0x479380: tick de los NÚMEROS DE DAÑO. Por slot activo:
// decrementa el contador de delay; cuando dispara, sube el número (pos.z +=
// lifetime), decae el lifetime 0.3, lo desactiva al llegar a 0, y encoge la
// escala 5.0 por tick con piso en 15.
//
// 2026-08-15 BUG-FIX (los números salían pero quedaban FIJOS en pantalla y no
// desaparecían): esto iteraba `DAT_07c80128`, declarado en globals.cpp como un
// global PROPIO de 100×0x70 ("spark-effect pool"). En el binario `unk_7C80128`
// no es un pool aparte: es `DAT_07c80110 + 0x18` — el MISMO pool de números que
// llena `CreatePoint` y recorre `RenderPoints`, sólo que las tres funciones lo
// abordan desde offsets distintos del slot y usan índices relativos.
// Al tocar otra memoria, el tick corría sobre un pool siempre vacío: los
// números nacían y nadie los movía ni los expiraba.
// IDA: FUN_00479380
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
