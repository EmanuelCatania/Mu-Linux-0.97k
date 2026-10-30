// Cloth_Simulation.cpp

#include "stdafx.h"
#include "Physics/ClothTypes.h"
#include "Game/RenderInterpolation.h"
#include "Game/RenderInterpolationMath.h"
#include "globals.h"
#include "functions.h"

// ── Bone / 3D sound helpers ───────────────────────────────────────────────────
// BMD_TransformPosition — implemented in src/Math/Math_3D.cpp (Bone_TransformPos)
float __fastcall SpringNode_Delta(void *a, uintptr_t b, float *c);
void  __fastcall VerletNode_AddAccel(void *a, float x, float y, float z);
void  __fastcall ClothNode_Integrate(void *a, float v);
void  __fastcall SpringNode_SetPos(void *node, float x, float y, float z, int pinned);
void  __cdecl    ClothNode_ApplyForces(uintptr_t _this, int a2, short a3);

// Cloth_PinTopRow @ 0x00408780 — Cloth_PinTopRow (slot 1 de off_552520).
// Reposiciona los W nodos de la FILA SUPERIOR sobre la matriz del hueso al que
// está atada la tela y los marca como fijados. Éste es el enganche que hace que
// la capa siga al personaje: `sub_408CB0` lo invoca con
// `BoneTransform + 48 * boneIdx` en cada tick.
// Port FIEL de IDA `sub_408780`. Layout: +0x14 flags, +0x18/+0x1C origen Y/Z,
// +0x20 ancho, +0x28 W, +0x34 nodes, +0x40 paso horizontal.
void __cdecl Cloth_PinTopRow(void* widget, float (*Matrix)[4])
{
  BYTE* _this = static_cast<BYTE*>(widget);
  float Position[3];
  float WorldPosition[3];

  int v4 = ClothSystem(_this).Columns;
  for (int v3 = 0; v3 < v4; ++v3)
  {
    int    v7 = ClothSystem(_this).Flags;   // flags
    double v8 = ClothSystem(_this).Width;   // ancho total
    double v9 = ClothSystem(_this).StepU;   // paso entre columnas
    if ((ClothSystem(_this).Flags & 0xC) == 4)
    {
      v8 = v8 * 0.60000002;
      v9 = v8 / (double)(v4 - 1);
    }
    double v10 = (double)v3;

    Position[1] = ClothSystem(_this).OriginU;
    Position[0] = (float)(v10 * v9 - v8 * 0.5);
    double v11  = ClothSystem(_this).OriginV;
    Position[2] = ClothSystem(_this).OriginV;

    if ((v7 & 3) == 1)
    {
      // Arco: los extremos de la fila cuelgan más que el centro.
      double v12 = fabs(v10 / (double)(v4 - 1) - 0.5);
      Position[1] = (float)(Position[1] - (v12 + v12) * (v12 + v12) * 10.0);
    }

    // Rotación local → espacio de hueso: (x,y,z) → (z,-y,x).
    float v13   = Position[0];
    Position[0] = (float)v11;
    Position[2] = v13;
    Position[1] = -Position[1];

    OBJECT* v15 = ClothSystem(_this).Owner;        // entity
    BMD_TransformPosition(
        static_cast<void*>(AsModel(DAT_05828d58) + v15->Type),
        (float *)Matrix,
        Position,
        WorldPosition,
        1);
    SpringNode_SetPos(ClothSystem(_this).Nodes + v3,
                 WorldPosition[0], WorldPosition[1], WorldPosition[2], 1);
    v4 = ClothSystem(_this).Columns;
  }
}

// Cloth_Wind @ 0x004089B0 — Cloth_Wind (slot 2 de off_552520).
// Elige un nodo "epicentro" que rota con el contador de frames y aplica las
// fuerzas a cada nodo con un peso que cae con la distancia Manhattan al
// epicentro (ráfagas que recorren la tela).
// Port de IDA `sub_4089B0`: ~85% del decompile es ruido de hash-table
// (ref-count sobre MoveSceneFrame + scrambling XOR), omitido per política del
// proyecto. Lo que queda es el cálculo de arriba, tal cual.
void __cdecl Cloth_Wind(void *_this)
{
  int frame = (int)MoveSceneFrame;              // MoveSceneFrame
  int v17   = (int)ClothSystem(_this).NodeCount;                 // nodeCount
  if (v17 <= 0)
    return;

  int v20 = 101 * (frame / 10) % v17;         // índice del epicentro
  int W   = (int)ClothSystem(_this).Columns;
  if (W <= 0)
    return;

  for (int v18 = 0; v18 < (int)ClothSystem(_this).NodeCount; ++v18)
  {
    int dist = abs(v20 / W - v18 / W) + abs(v20 % W - v18 % W);
    ClothNode_ApplyForces(reinterpret_cast<uintptr_t>(ClothSystem(_this).Nodes + v18), dist, (short)ClothSystem(_this).Flags);
  }
}

// Cloth_Integrate @ 0x00408CB0 — Cloth_Integrate: medio tick de simulación.
//   1. vtable[2] — reparte viento/gravedad sobre la aceleración de cada nodo.
//   2. Springs marcados con flag 2: si el par está estirado más allá del
//      reposo, aplica una fuerza de restitución a AMBOS extremos
//      (×3 cuando (flags & 0x300) == 256).
//   3. vtable[1] — vuelve a fijar la fila superior al hueso.
//   4. Integra todos los nodos.
// Port FIEL de IDA `sub_408CB0`. El `a2@<st0>` del decompile es el retorno de
// `sub_407B50` (la distancia), no un parámetro: reconstruido.
void __fastcall Cloth_Integrate(int *param_1, float a3)
{
  DWORD *a1 = (DWORD *)param_1;
  float v25[3];

  // Mismo guard que en Cloth_Solve: la vtable[1] (Cloth_PinTopRow) deferencia
  // `BoneTransform + 48*boneIdx` sin validarlo.
  if (!a1 || !ClothSystem(a1).Owner || !ClothSystem(a1).Owner->BoneMatrices)
    return;

  reinterpret_cast<void (__cdecl *)(void*)>(ClothSystem(a1).VTable[2])(a1);

  for (int v20 = 0; v20 < ClothSystem(a1).SpringCount; ++v20)
  {
    const ClothSpring& spring = ClothSystem(a1).Springs[v20];
    if ((spring.Flags & 2) == 0)
      continue;

    ClothNode* v6 = ClothSystem(a1).Nodes;
    ClothNode* v7 = v6 + spring.A;
    ClothNode* v8 = v6 + spring.B;

    double d = SpringNode_Delta(v7, reinterpret_cast<uintptr_t>(v8), v25);
    if (d < 0.001)
      d = 0.001;

    if (d > spring.Rest + 0.0099999998)
    {
      double v9  = d - spring.Rest;
      int    v10 = ClothSystem(a1).Flags & 0x300;
      float  f[3];
      for (int i = 0; i < 3; ++i)
      {
        double v12 = v9 * v25[i] / d;
        f[i] = (float)v12;
        if (v10 == 256)
          f[i] = (float)(v12 * 3.0);
      }
      VerletNode_AddAccel(v7, -f[0], -f[1], -f[2]);
      VerletNode_AddAccel(v8, f[0], f[1], f[2]);
    }
  }

  reinterpret_cast<void (__cdecl *)(void*,float (*)[4])>(ClothSystem(a1).VTable[1])(
      a1, ClothSystem(a1).Owner->BoneMatrices[ClothSystem(a1).Bone]);

  for (int v15 = 0; v15 < ClothSystem(a1).NodeCount; ++v15)
    ClothNode_Integrate(ClothSystem(a1).Nodes + v15, a3);
}

// ═══════════════════════════════════════════════════════════════════════════
// Sistema de tela (capa del MG) — SOLVER.
//
// Layout del widget (stride 0x54), índices en DWORDs como los usa IDA:
//   [1]=+0x04 entity   [2]=+0x08 boneIdx   [5]=+0x14 flags
//   [10]=+0x28 W       [11]=+0x2C H        [12]=+0x30 nodeCount
//   [13]=+0x34 nodes*  [14]=+0x38 springCount  [15]=+0x3C springs*
//   [18]=+0x48 anchorCount  [19]=+0x4C head sentinel  [20]=+0x50 tail sentinel
// Nodo (stride 60 = 0x3C):
//   +0x00 vtable  +0x04..0x0C accel  +0x10..0x18 vel  +0x1C..0x24 pos
//   +0x28 pinned(byte)  +0x2C corrCount  +0x30..0x38 corrAccum
// Spring (stride 16): [short na][short nb][float rest][byte flags]
//   flags&1 = equality (sub_407C60)   flags&2 = wind/stretch (sub_408CB0)
//   flags&4 = rango min/max (sub_407B90)
// Ancla (stride 0x24): +0x00 vtable  +0x04..0x0C localPos  +0x10 boneIdx
//   +0x14..0x1C worldPos  +0x20 radio
//
// NOTA sobre el `double a2@<st0>` de los decompiles: es un artefacto. En el
// disasm (0x407C71, 0x407C76) el valor comparado contra 0.001 lo PRODUCE
// `sub_407B50` en ST0 — Hex-Rays no modela ese retorno y lo atribuye a un
// parámetro de entrada. Los ports reconstruyen la distancia real.
// ═══════════════════════════════════════════════════════════════════════════

float __fastcall SpringNode_Delta(void *a, uintptr_t b, float *c);          // delta + |delta|
char  __fastcall VerletSystem_Flush(uintptr_t a1);                            // aplica corrección
void  __cdecl    Cloth_SpringEqual(uintptr_t a1, uintptr_t a3, float rest);        // spring de igualdad
int   __cdecl    Cloth_SpringRange(uintptr_t a1, uintptr_t a3, const float *range);// spring de rango

// Vec3_Length @ 0x004f9c40 — Vec3_Length: returns sqrt(dot(v,v)), does NOT modify v.
// (Ghidra shows return as float10 left in x87 ST0; callers use the return value as distance.)
// Vec3_Length (IDA-activated)
float __cdecl Vec3_Length(float *a1)
{
  long double v1; // st7
  int v3; // ecx
  double v4; // st5

  v1 = 0.0;
  v3 = 3;
  do
  {
    v4 = *a1 * *a1;
    ++a1;
    --v3;
    v1 = v1 + v4;
  }
  while ( v3 );
  return sqrt(v1);
}


// VerletNode_AddAccel @ 0x00407ac0 — VerletNode_AddAccel: accumulates acceleration at +4/+8/+C
void __fastcall VerletNode_AddAccel(void *a, float x, float y, float z) {
    char *p = (char*)a;
    ClothPoint(p).Acceleration[0]  += x;
    ClothPoint(p).Acceleration[1]  += y;
    ClothPoint(p).Acceleration[2]+= z;
}

// ClothNode_Integrate @ 0x00407AF0 — ClothNode_Integrate: paso de Euler semi-implícito.
// Layout: +0x04 accel[3], +0x10 vel[3], +0x1C pos[3], +0x28 pinned(byte).
// _DAT_00559070 = 400.0 (leído del binario). Port FIEL de IDA `sub_407AF0`.
void __fastcall ClothNode_Integrate(void *a, float v) {
    char *p = (char*)a;
    if (ClothPoint(p).Flags & 1) return;  // nodo fijado — no se integra
    for (int i = 0; i < 3; i++) {
        double acc = _DAT_00559070 * ClothPoint(p).Acceleration[i];
        double vel = acc * v + ClothPoint(p).Velocity[i];
        ClothPoint(p).Velocity[i] = (float)vel;
        ClothPoint(p).Position[i] = (float)(vel * v + ClothPoint(p).Position[i]);
    }
}

// ClothNode_ApplyForces @ 0x004079E0 — ClothNode_ApplyForces: reescribe la aceleración
// del nodo con viento + arrastre, y le suma la gravedad.
//   a2 = distancia Manhattan al nodo "epicentro" de la ráfaga (la calcula
//        `sub_4089B0`); a 5 celdas la ráfaga no llega (peso 0).
//   a3 = flags del widget.
// Port FIEL de IDA `sub_4079E0`. El bucle sobre `&flt_590AF4 .. &unk_590B00`
// son los 3 componentes del viento; en nuestro build esos globals no son
// contiguos, así que se despliega explícitamente.
void __cdecl ClothNode_ApplyForces(uintptr_t _this, int a2, short a3)
{
  if ((ClothPoint(_this).Flags & 1) != 0)
  {
    ClothPoint(_this).Acceleration[2] = 0;
    ClothPoint(_this).Acceleration[1]  = 0;
    ClothPoint(_this).Acceleration[0]  = 0;
    return;
  }

  // IDA: v3 = 5 - a2 (unsigned); >= 4 → 4; == 0 → peso 0; si no v3+2.
  unsigned int v3 = (unsigned int)(5 - a2);
  int v11;
  if (v3 >= 4)      v11 = 4 + 2;
  else if (v3 != 0) v11 = (int)v3 + 2;
  else              v11 = 0;

  const double v4   = (double)v11;
  const float  wind[3] = { DAT_00590af4, DAT_00590af8, DAT_00590afc };
  for (int i = 0; i < 3; ++i)
  {
    double drag = ClothPoint(_this).Velocity[i] * 0.0099999998;
    ClothPoint(_this).Acceleration[i] = (float)(v4 * wind[i] - drag);
  }

  if ((a3 & 0x300) == 256)
    ClothPoint(_this).Acceleration[2] = (float)((_DAT_00590af0 + 0.1) * v4 + ClothPoint(_this).Acceleration[2]);

  double v9 = _DAT_0055906c * _DAT_00559068;
  if ((a3 & 0xC00) == 1024)
    ClothPoint(_this).Acceleration[2] = (float)(ClothPoint(_this).Acceleration[2] - v9 * 180.0);
  else
    ClothPoint(_this).Acceleration[2] = (float)(ClothPoint(_this).Acceleration[2] - v9 * 100.0);
}

// SpringNode_Delta @ 0x00407b50 — SpringNode_Delta: out = posA - posB; returns distance (length of delta).
// param_1=nodeA_ptr (this), param_2=nodeB_ptr (dirección nativa), param_3=float[3] output.
// Does NOT normalize; caller uses return value as rest-length distance.
float __fastcall SpringNode_Delta(void *a, uintptr_t b, float *c) {
    float *posA = ClothPoint(a).Position;
    float *posB = ClothPoint(b).Position;
    c[0] = posA[0] - posB[0];
    c[1] = posA[1] - posB[1];
    c[2] = posA[2] - posB[2];
    return Vec3_Length(c);
}

// Cloth_SpringRange @ 0x00407B90 — Cloth_SpringRange: mantiene |a1-a3| dentro de
// [range[0], range[1]]. Devuelve 0 sólo si el spring está "roto" (estirado más
// de 20x el máximo) — eso corta la iteración del solver en `sub_408E30`.
// Port FIEL de IDA `sub_407B90` (el `a2@<st0>` es el retorno de sub_407B50).
// Corrige SÓLO la posición de a1 (+0x1C..+0x24), no la del partner.
int __cdecl Cloth_SpringRange(uintptr_t a1, uintptr_t a3, const float *range)
{
  float v9[3];
  double diff;

  if ((ClothPoint(a1).Flags & 1) != 0)
    return 1;                                   // nodo fijado

  double d = SpringNode_Delta((void *)a1, a3, v9);
  if (d < 0.001)
    d = 0.001;

  if (d > range[1] * 20.0)
    return 0;

  if (d > range[1])
    diff = d - range[1];
  else if (d < range[0])
    diff = d - range[0];
  else
    return 1;

  double f = diff / d;
  v9[0] = (float)(f * v9[0]);
  v9[1] = (float)(f * v9[1]);
  v9[2] = (float)(f * v9[2]);
  ClothPoint(a1).Position[0] = ClothPoint(a1).Position[0] - v9[0];
  ClothPoint(a1).Position[1] = ClothPoint(a1).Position[1] - v9[1];
  ClothPoint(a1).Position[2] = ClothPoint(a1).Position[2] - v9[2];
  return 1;
}

// Cloth_SpringEqual @ 0x00407C60 — Cloth_SpringEqual: corrección simétrica hacia la
// longitud de reposo. NO toca la posición: acumula en el buffer de corrección
// (+0x30..+0x38) e incrementa el contador (+0x2C); `sub_407D10` promedia y
// aplica. Port FIEL de IDA `sub_407C60` (verificado contra el disasm).
void __cdecl Cloth_SpringEqual(uintptr_t a1, uintptr_t a3, float rest)
{
  float v8[3];

  double d = SpringNode_Delta((void *)a1, a3, v8);
  if (d < 0.001)
    d = 0.001;

  double f = (d - rest) * 0.5 / d;
  v8[0] = (float)(f * v8[0]);
  v8[1] = (float)(f * v8[1]);
  v8[2] = (float)(f * v8[2]);

  ClothPoint(a1).Correction[0] = ClothPoint(a1).Correction[0] - v8[0];
  ClothPoint(a1).Correction[1] = ClothPoint(a1).Correction[1] - v8[1];
  ClothPoint(a1).Correction[2] = ClothPoint(a1).Correction[2] - v8[2];
  ClothPoint(a3).Correction[0] = v8[0] + ClothPoint(a3).Correction[0];
  ClothPoint(a3).Correction[1] = v8[1] + ClothPoint(a3).Correction[1];
  ClothPoint(a3).Correction[2] = v8[2] + ClothPoint(a3).Correction[2];
  ++ClothPoint(a1).CorrectionCount;
  ++ClothPoint(a3).CorrectionCount;
}

// VerletSystem_Flush @ 0x00407d10 — VerletSystem_Flush: apply accumulated position corrections, zero buffer
// VerletSystem_Flush (IDA-activated)
char __fastcall VerletSystem_Flush(uintptr_t a1)
{
  ClothNode& node = ClothPoint(a1);
  intptr_t result = node.Flags;
  if ((result & 1) != 0) {
    node.Correction[2] = node.Correction[1] = node.Correction[0] = 0;
  } else {
    result = static_cast<std::int32_t>(node.CorrectionCount);
    if (result > 0) {
      float* cursor = node.Correction;
      for (int i=0;i<3;++i) cursor[i] = cursor[i] / (double)static_cast<std::int32_t>(node.CorrectionCount);
      result = reinterpret_cast<intptr_t>(cursor + 3);
      double v4 = cursor[0] + node.Position[0];
      cursor[0] = 0;
      node.CorrectionCount = 0;
      node.Position[0] = (float)v4;
      double v5 = cursor[1] + node.Position[1];
      cursor[1] = 0;
      node.Position[1] = (float)v5;
      double v6 = node.Position[2] + cursor[2];
      cursor[2] = 0;
      node.Position[2] = (float)v6;
    }
  }
  return static_cast<char>(result);
}


// VerletNode_SetTarget @ 0x00407e10 — VerletNode_SetTarget: stores vec3 at +0x14..+0x1c
void __fastcall VerletNode_SetTarget(void *a, float b, int c, int d) {
    char *p = (char*)a;
    ClothBody(p).WorldPosition[0] = b;
    ClothBody(p).WorldPosition[1] = *(float*)&c;
    ClothBody(p).WorldPosition[2] = *(float*)&d;
}

// VerletNode_GetPos @ 0x00407e30 — VerletNode_GetPos: reads vec3 from +0x04..+0x0c into out[]
void __fastcall VerletNode_GetPos(void *a, float *b) {
    char *p = (char*)a;
    b[0] = ClothBody(p).LocalPosition[0];
    b[1] = ClothBody(p).LocalPosition[1];
    b[2] = ClothBody(p).LocalPosition[2];
}

// ── SpringNode helpers (SpringNode_Ctor family) ──────────────────────────────

// SpringNode_ZeroFields @ 0x00407980 — SpringNode_ZeroFields: zeroes accel/vel/pos/pinned in 0x3c stride node.
void __fastcall SpringNode_ZeroFields(void* node) {
    BYTE* param_1 = static_cast<BYTE*>(node);
    ClothPoint(param_1).CorrectionCount = 0;
    for (int i = 0; i < 3; i++) {
        ClothPoint(param_1).Position[i] = 0;   // pos[i]
        ClothPoint(param_1).Velocity[i] = 0;   // vel[i]
        ClothPoint(param_1).Acceleration[i] = 0;   // accel[i]
        ClothPoint(param_1).Correction[i] = 0;   // extra (puVar1[5..7])
    }
    ClothPoint(param_1).Flags = 0;
}

// SpringNode_Ctor @ 0x00407950 — SpringNode_Ctor: set vtable + zero fields.
// SpringNode_Ctor (IDA-activated)
void __fastcall SpringNode_Ctor(void *_this)
{
  extern void *g_ClothNodeVTable[1];
  ClothPoint(_this).VTable = g_ClothNodeVTable;
  SpringNode_ZeroFields(_this);
}


// SpringNode_SetPos @ 0x004079B0 — SpringNode_SetPos: write position and pinned flag.
void __fastcall SpringNode_SetPos(void *node, float x, float y, float z, int pinned) {
    char *p = (char*)node;
    ClothPoint(p).Position[0] = x;
    ClothPoint(p).Position[1] = y;
    ClothPoint(p).Position[2] = z;
    if (pinned)
        ClothPoint(p).Flags |= 1;
}

// SpringNode_GetPos @ 0x00407B30 — SpringNode_GetPos: copy position (+0x1c..+0x24) to out[3].
void __fastcall SpringNode_GetPos(void *node, float *out) {
    float *src = ClothPoint(node).Position;
    out[0] = src[0];
    out[1] = src[1];
    out[2] = src[2];
}

// Spring_StoreEdge @ 0x004088B0 — Spring_StoreEdge: write spring entry (stride 0x10) into spring array.
// Entry layout: [ushort na][ushort nb][float rest_scaled][float dist][byte flags][3 bytes pad]
void __fastcall Spring_StoreEdge(void *sys, int idx, short na, short nb,
                              float rest_scaled, float dist, BYTE edge_flags) {
    ClothSpring& edge = ClothSystem(sys).Springs[idx];
    edge.A = na;
    edge.B = nb;
    edge.Minimum = rest_scaled;
    edge.Rest = dist;
    edge.Flags = edge_flags;
}

// ── VerletNode helpers (VerletNode_CtorBase family) ─────────────────────────────

// VerletNode_ZeroFields @ 0x00407DF0 — VerletNode_ZeroFields: zero +4..+1c (7 dwords).
void __fastcall VerletNode_ZeroFields(void* node) {
    BYTE* param_1 = static_cast<BYTE*>(node);
    ClothBody(param_1).LocalPosition[0] = 0;
    ClothBody(param_1).LocalPosition[1] = 0;
    ClothBody(param_1).LocalPosition[2] = 0;
    ClothBody(param_1).Bone = 0;
    ClothBody(param_1).WorldPosition[0] = 0;
    ClothBody(param_1).WorldPosition[1] = 0;
    ClothBody(param_1).WorldPosition[2] = 0;
}

// VerletNode_CtorBase @ 0x00407DA0 — VerletNode_CtorBase: set vtable (skipped) + zero fields.
void* __fastcall VerletNode_CtorBase(void *param_1) {
    VerletNode_ZeroFields(param_1);
    return param_1;
}

// VerletNode_CtorExt @ 0x00407ED0 — VerletNode_CtorExt: zero fields + clear +0x20.
// VerletNode_CtorExt (IDA-activated)
intptr_t __cdecl VerletNode_CtorExt(DWORD *_this)
{
  // Se conserva el retorno del port actual; sus consumidores no lo utilizan.
  intptr_t result;

  VerletNode_ZeroFields(_this);
  result = reinterpret_cast<intptr_t>(_this);
  ClothBody(_this).Radius = 0;
  return result;
}


// ── vtable del ANCLA de tela (off_552514) ─────────────────────────────────
// Leída del binario original (`Cliente armado/main.exe`, MD5 eb95ac…; su
// contenido es data y no aparece en los decompiles):
//     off_552508 (base) = { 0x00407DC0, 0x0040DB50, 0x00410AE0 }
//     off_552514 (ext)  = { 0x00407EA0, 0x00410D80, 0x00407F20 }
// El slot 2 es el que importa: `sub_409310` lo invoca por ancla y por nodo
// para resolver la colisión con la esfera del cuerpo. 0x40DB50/0x410AE0/
// 0x410D80 no están portadas y ningún call site de este build las usa: van
// como no-ops para que el objeto sea válido.
void __cdecl ClothAnchor_Collide(void *_this, void *node);
void __fastcall FUN_00407ea0(void* ecx, int edx, BYTE param_1);
// Adaptador: el dtor está portado como __fastcall (this en ECX, `edx` dummy) y
// los call sites de este build invocan la vtable como `__cdecl (this, flags)`.
static void __cdecl ClothAnchor_VSlot0(void *a, char flags) { FUN_00407ea0(a, 0, (BYTE)flags); }
static void __cdecl ClothAnchor_VSlot1(void *a) { (void)a; }   // 0x410D80 — sin portar, sin call sites
void *g_ClothAnchorVTable[3] = {
    (void *)ClothAnchor_VSlot0,
    (void *)ClothAnchor_VSlot1,
    (void *)ClothAnchor_Collide
};

// vtable del NODO de tela (DAT_005524e8 del binario) = { 0x00408680 }.
void __cdecl FUN_00408680(void *_this, char flags);
void *g_ClothNodeVTable[1] = { (void *)FUN_00408680 };

// ClothAnchor_Ctor @ 0x00407E50 — ClothAnchor_Ctor: full constructor (base + ext).
// ClothAnchor_Ctor (IDA-activated)
DWORD *__cdecl ClothAnchor_Ctor(DWORD *_this)
{
  VerletNode_CtorBase(_this);
  ClothBody(_this).VTable = g_ClothAnchorVTable;
  VerletNode_CtorExt(_this);
  return _this;
}

// ClothNode_AddCorrection @ 0x00407D70 — ClothNode_AddCorrection: acumula una corrección
// de posición en el buffer (+0x30..+0x38) e incrementa el contador (+0x2C).
// Port FIEL de IDA `sub_407D70`.
void __cdecl ClothNode_AddCorrection(uintptr_t _this, const float *a2)
{
  ClothPoint(_this).Correction[0] = a2[0] + ClothPoint(_this).Correction[0];
  ClothPoint(_this).Correction[1] = a2[1] + ClothPoint(_this).Correction[1];
  double v3 = a2[2] + ClothPoint(_this).Correction[2];
  ++ClothPoint(_this).CorrectionCount;
  ClothPoint(_this).Correction[2] = (float)v3;
}

// ClothAnchor_Collide @ 0x00407F20 — ClothAnchor_Collide (slot 2 de off_552514).
// Esfera de colisión: si el nodo está DENTRO del radio (+0x20) del ancla, lo
// empuja hacia afuera. `_this[5..7]` (+0x14..+0x1C) es la posición de MUNDO
// del ancla, que `sub_408E30` refresca cada iteración desde el hueso.
// Port FIEL de IDA `sub_407F20`.
void __cdecl ClothAnchor_Collide(void *_this, void *node)
{
  float v[3];

  SpringNode_GetPos(node, v);
  v[0] = v[0] - ClothBody(_this).WorldPosition[0];
  v[1] = v[1] - ClothBody(_this).WorldPosition[1];
  v[2] = v[2] - ClothBody(_this).WorldPosition[2];

  double v3 = Vec3_Length(v);
  if (v3 < 0.0099999998)
  {
    v3   = 0.0099999998;
    v[0] = 0.0099999998f;
    v[1] = 0.0f;
    v[2] = 0.0f;
  }
  if (v3 < ClothBody(_this).Radius)
  {
    double v4 = (ClothBody(_this).Radius - v3) / v3;
    v[0] = (float)(v4 * v[0]);
    v[1] = (float)(v4 * v[1]);
    v[2] = (float)(v4 * v[2]);
    ClothNode_AddCorrection(reinterpret_cast<uintptr_t>(node), v);
  }
}

// Cloth_CollideAnchors @ 0x00409310 — Cloth_CollideAnchors: para cada ancla de la
// lista, invoca su vtable[2] contra los `nodeCount` nodos de la grilla, y
// después hace flush de las correcciones acumuladas.
// Port FIEL de IDA `sub_409310`. Ojo: todo el bloque está gateado por
// `this[18] > 0` (cantidad de anclas), así que sin anclas es un no-op.
int __cdecl Cloth_CollideAnchors(DWORD* widget)
{
  ClothWidget& w = ClothSystem(widget);
  int result = w.AnchorCount;
  if (result <= 0) return result;
  ClothLink* entry = w.Head->Next;
  if (entry != w.Tail) {
    while (entry) {
      ClothAnchor* anchor = entry->Value;
      for (int i=0;i<w.NodeCount;++i)
        reinterpret_cast<void (__cdecl *)(void*,void*)>(anchor->VTable[2])(anchor,w.Nodes+i);
      entry = entry->Next;
      if (entry == w.Tail) break;
    }
  }
  result = w.NodeCount;
  for (int i=0;i<result;++i) {
    VerletSystem_Flush(reinterpret_cast<uintptr_t>(w.Nodes+i));
    result = w.NodeCount;
  }
  return result;
}


// ClothAnchor_SetParams @ 0x00407EF0 — ClothAnchor_SetParams.
// Fields: +4/+8/+0xc = posición LOCAL, +0x20 = radio, +0x10 = índice de HUESO.
//
// Ojo: el último parámetro es un DWORD entero (IDA `sub_407EF0`: `this[4] = a6`),
// no float: `sub_408E30` lo usa como `48 * v5[4]` para indexar la matriz de
// huesos.  Los call sites de IDA pasan enteros chicos (2, 10, 17, 18, 19).
void __fastcall ClothAnchor_SetParams(void *node, float p1, float p2, float p3, float radius, int boneIdx) {
    char *p = (char*)node;
    ClothBody(p).LocalPosition[0] = p1;
    ClothBody(p).LocalPosition[1] = p2;
    ClothBody(p).LocalPosition[2] = p3;
    ClothBody(p).Radius = radius;
    ClothBody(p).Bone = boneIdx;
}

// Entity_DrawByType.cpp dependencies

// ============================================================================
// TELA POR MALLA (5.2: PhysicsManager.cpp -- CPhysicsClothMesh)
//
// Bloque aparte del resto del archivo, que es la tela del 0.97k.  Queda
// listo para cuando se avance de version: el 5.2 lo usa para tres pants
// (ZzzObject.cpp, RenderPartObject):
//   iCloth = 1 -> MODEL_PANTS + 18 (Grand Soul): rejilla 5x8, hueso 17.
//   iCloth = 2 -> MODEL_PANTS + 19 (Divine): modo por malla, portado aca.
//   iCloth = 3 -> MODEL_PANTS + 22: rejilla 7x5, hueso 17.
// Los modos de rejilla (PCT_OPT_MESHPROG) reescriben los triangulos de la
// malla del modelo y no estan portados.
// ============================================================================

// DESVIACION DOCUMENTADA (no existe en el 0.97k).
//
// El 0.97k no trae el set Divine: el modelo `Data/Player/PantMale20.bmd`
// (MODEL_PANTS + 19 = 707) que se usa en este cliente es el del 0.99, que
// agrega una cuarta malla con la falda.  Sin simulacion esa malla se mueve
// pegada al cuerpo y atraviesa las piernas.  El modelo original (0.95/0.97)
// tiene solo 3 mallas y no trae falda, asi que ahi no se hace nada.
//
// Se porta el CPhysicsClothMesh del source de MU 5.2 en su modo "por malla"
// (sin PCT_OPT_MESHPROG), tal como lo usa ZzzObject.cpp::RenderPartObject
// para MODEL_PANTS + 19:
//
//     pCloth->Create(o, 3, 2, PCT_OPT_CORRECTEDFORCE | PCT_HEAVY, Type);
//     AddCollisionSphere(0, 0, -15, 22, 2);  ... (-27, 23) (-40, 24)
//                                                (-54, 25) (-69, 26)
//     if (!pCloth->Move2(0.005f, 5)) DeleteCloth(...); else pCloth->Render();
//
// La tela no dibuja nada propio: los vertices de la malla 3 son las
// particulas (los del hueso 2 quedan fijos a la cadera), los resortes salen
// de las aristas de los triangulos, y Render() escribe las posiciones
// simuladas sobre los vertices ya transformados de la malla antes de que se
// dibuje el cuerpo.  Asi la falda conserva su textura y su luz.
//
// Se guarda en una tabla propia (no en part+0x14) porque DeleteCloth del
// 0.97k libera ese campo asumiendo el layout de su propia tela.

#include <math.h>

namespace {

const int   kDivinePantsModel = 707;             // MODEL_PANTS + 19
const int   kSkirtMesh        = 3;
const int   kFixedBone        = 2;

// Flags de 5.2 que usa este caso.
const unsigned PCT_HEAVY              = 0x00000400;
const unsigned PCT_OPT_CORRECTEDFORCE = 0x20000000;

const unsigned char PVS_FIXEDPOS = 0x01;
const unsigned char PLS_LOOSEDISTANCE  = 0x01;
const unsigned char PLS_SPRING         = 0x02;
const unsigned char PLS_STRICTDISTANCE = 0x04;

const float kGravity   = 9.8f;
const float kMass      = 0.0025f;
const float kInvOfMass = 400.0f;

struct Vec3 { float x, y, z; };

struct Vertex {
    Vec3 force, vel, pos, oneTimeMove;
    Vec3 presentationPrevious;
    Vec3 anchor;             // muestra de pose propia; no apunta al scratch del render
    int  countOneTimeMove;
    unsigned char state;
};

struct Link {
    short v[2];
    float dist[2];
    unsigned char style;
};

struct Sphere {
    Vec3  centerLocal;   // antes de transformar (ejes del source)
    Vec3  center;
    float radius;
    int   bone;
};

const int kMaxSpheres = 5;

struct MeshCloth {
    const PART_t* part;      // Clave nativa de la pieza en la caché propia.
    int     lastFrame;
    bool presentationContinuous;
    int     spawnFrame;       // identidad del owner al reutilizar un slot
    int     numVertices;
    Vertex *vertices;
    int     numLinks;
    Link   *links;
    Sphere  spheres[kMaxSpheres];
    int     numSpheres;
    unsigned type;
};

const int kSlots = 16;
MeshCloth g_Slots[kSlots];

inline float* VT(int mesh, int v) {
    return (float *)(g_BoneVertexBuf + (mesh * 15000 + v) * 12);
}

inline float Length(const Vec3 &a) { return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z); }

float GetDistance(const Vertex &a, const Vertex &b, Vec3 *d) {
    d->x = a.pos.x - b.pos.x; d->y = a.pos.y - b.pos.y; d->z = a.pos.z - b.pos.z;
    return Length(*d);
}

void Destroy(MeshCloth &c) {
    delete[] c.vertices;
    delete[] c.links;
    memset(&c, 0, sizeof(c));
}

// Mesh_t en memoria (BMD_Load): +4 NumVertices, +10 NumTriangles,
// +0x10 Vertices (stride 0x10, Node en +0), +0x1c Triangles (stride 0x24,
// VertexIndex[4] en +2).
inline char* MeshPtr(void *model, int mesh) {
    // El layout legado conserva el puntero de tabla en 32 bits hasta B2.
    const auto table = static_cast<std::uintptr_t>(
        *reinterpret_cast<const DWORD*>(static_cast<BYTE*>(model) + 0x28));
    return reinterpret_cast<char*>(table + static_cast<std::intptr_t>(mesh) * 0x28);
}
inline short TriVertex(char *mesh, int tri, int i) {
    return *(short *)(*(int *)(mesh + 0x1c) + tri * 0x24 + 2 + i * 2);
}

int FindMatchVertex(char *mesh, int v1, int v2, int v3) {
    const int nTri = *(short *)(mesh + 10);
    for (int t = 0; t < nTri; ++t) {
        for (int i = 0; i < 3; ++i) {
            const int c1 = TriVertex(mesh, t, i);
            const int c2 = TriVertex(mesh, t, (i + 1) % 3);
            const int c3 = TriVertex(mesh, t, (i + 2) % 3);
            if (v1 == c2 && v2 == c1 && v3 != c3) return c3;
        }
    }
    return -1;
}

bool FindInLink(const MeshCloth &c, int count, int v1, int v2) {
    for (int i = 0; i < count; ++i)
        if (c.links[i].v[0] == v1 && c.links[i].v[1] == v2) return true;
    return false;
}

void SetLink(MeshCloth &c, int i, int v1, int v2, float dMin, float dMax, unsigned char style) {
    c.links[i].v[0] = (short)v1; c.links[i].v[1] = (short)v2;
    c.links[i].dist[0] = dMin;  c.links[i].dist[1] = dMax;
    c.links[i].style = style;
}

// Misma transformación de Skeleton_Transform con Translate=1 y LOD=1.
// Se evalúa desde la pose SIM, sin preparar ni modificar vértices de render.
Vec3 PoseVertex(void *model, const float *bones, const char *vertex) {
    float p[3];
    Vector_Transform((float *)(vertex + 4), (float *)(bones + *(short *)vertex * 12), p);
    const float scale = *(float *)((char *)model + 0x68);
    p[0] *= scale; p[1] *= scale; p[2] *= scale;
    p[0] += *(float *)((char *)model + 0x6c);
    p[1] += *(float *)((char *)model + 0x70);
    p[2] += *(float *)((char *)model + 0x74);
    return { p[0], p[1], p[2] };
}

// CPhysicsClothMesh::Create(o, iMesh, iBone, dwType)
bool Create(MeshCloth &c, void *model, const float *bones) {
    char *mesh = MeshPtr(model, kSkirtMesh);
    const int nV   = *(short *)(mesh + 4);
    const int nTri = *(short *)(mesh + 10);
    if (nV <= 0 || nTri <= 0 || nV > 15000) return false;

    c.type        = PCT_OPT_CORRECTEDFORCE | PCT_HEAVY;
    c.numVertices = nV;
    c.vertices    = new Vertex[nV];
    memset(c.vertices, 0, sizeof(Vertex) * nV);
    c.links       = new Link[nTri * 3 * 2];

    bool anyFixed = false;
    const char *verts = (const char *)*(int *)(mesh + 0x10);
    for (int v = 0; v < nV; ++v) {
        c.vertices[v].pos = PoseVertex(model, bones, verts + v * 0x10);
        if (*(short *)(verts + v * 0x10) == kFixedBone) {
            c.vertices[v].state |= PVS_FIXEDPOS;
            anyFixed = true;
        }
    }
    if (!anyFixed) return false;   // no es la falda del 0.99

    int link = 0;
    Vec3 tmp;
    for (int t = 0; t < nTri; ++t) {
        for (int i = 0; i < 3; ++i) {
            const int v1 = TriVertex(mesh, t, i);
            const int v2 = TriVertex(mesh, t, (i + 1) % 3);
            if (v1 < 0 || v1 >= nV || v2 < 0 || v2 >= nV) continue;
            const float d = GetDistance(c.vertices[v1], c.vertices[v2], &tmp);
            unsigned char style = PLS_STRICTDISTANCE;
            if (fabsf(c.vertices[v1].pos.x - c.vertices[v2].pos.x) > 10.0f)
                style = PLS_LOOSEDISTANCE;
            SetLink(c, link++, v1, v2, d * 0.5f, d, PLS_SPRING | style);

            const int v3 = TriVertex(mesh, t, (i + 2) % 3);
            const int match = FindMatchVertex(mesh, v1, v2, v3);
            if (match > 0 && match < nV && v3 >= 0 && v3 < nV) {
                const float d2 = GetDistance(c.vertices[v3], c.vertices[match], &tmp);
                if (d2 < d * 1.2f && !FindInLink(c, link, v3, match))
                    SetLink(c, link++, v3, match, d2 * 0.5f, d2, PLS_SPRING | style);
            }
        }
    }
    c.numLinks = link;

    // AddCollisionSphere(0, 0, z, r, 2) x5 -- valores del 5.2.
    //
    // Nota: la falda del modelo del 0.99 que se usa hoy es mas larga y ancha
    // que estas esferas (cono de hasta ~59 de radio por ~80 de largo, medido
    // sobre PantMale20.bmd), asi que cuelga rozando las piernas.  Se probo
    // alargar la cadena y abrir los radios y la tela quedaba inestable, asi
    // que se mantienen los del 5.2: lo correcto es usar el modelo original
    // (PantMaleTest20.bmd del 5.2), para el que estan hechos.
    const float z[kMaxSpheres] = { -15.0f, -27.0f, -40.0f, -54.0f, -69.0f };
    const float r[kMaxSpheres] = {  22.0f,  23.0f,  24.0f,  25.0f,  26.0f };
    c.numSpheres = kMaxSpheres;
    for (int i = 0; i < kMaxSpheres; ++i) {
        c.spheres[i].centerLocal.x = 0.0f;
        c.spheres[i].centerLocal.y = 0.0f;
        c.spheres[i].centerLocal.z = z[i];
        c.spheres[i].radius = r[i];
        c.spheres[i].bone   = kFixedBone;
    }
    return true;
}

// CPhysicsVertex::UpdateForce
void UpdateForce(Vertex &v, unsigned key, unsigned type, const Vec3 &wind) {
    if (v.state & PVS_FIXEDPOS) { v.force.x = v.force.y = v.force.z = 0.0f; return; }
    int t = 5 - (int)key; if (t < 0) t = 0; if (t > 4) t = 4;
    const float fRand = (float)(t == 0 ? 0 : t + 2);
    v.force.x = fRand * wind.x - v.vel.x * 0.01f;
    v.force.y = fRand * wind.y - v.vel.y * 0.01f;
    v.force.z = fRand * wind.z - v.vel.z * 0.01f;
    if (type & PCT_HEAVY) v.force.z -= kGravity * kMass * 180.0f;
    else                  v.force.z -= kGravity * kMass * 100.0f;
}

void AddOneTimeMove(Vertex &v, const Vec3 &m) {
    v.oneTimeMove.x += m.x; v.oneTimeMove.y += m.y; v.oneTimeMove.z += m.z;
    v.countOneTimeMove++;
}

void DoOneTimeMove(Vertex &v) {
    if (v.state & PVS_FIXEDPOS) {
        v.oneTimeMove.x = v.oneTimeMove.y = v.oneTimeMove.z = 0.0f;
        return;
    }
    if (v.countOneTimeMove > 0) {
        const float n = (float)v.countOneTimeMove;
        v.pos.x += v.oneTimeMove.x / n; v.pos.y += v.oneTimeMove.y / n; v.pos.z += v.oneTimeMove.z / n;
        v.oneTimeMove.x = v.oneTimeMove.y = v.oneTimeMove.z = 0.0f;
        v.countOneTimeMove = 0;
    }
}

bool KeepLength(Vertex &v, const Vertex &other, const float *len) {
    if (v.state & PVS_FIXEDPOS) return true;
    Vec3 d;
    float dist = GetDistance(v, other, &d);
    if (dist < 0.001f) dist = 0.001f;
    if (dist > len[1] * 20.0f) return false;
    float k = 0.0f;
    if (dist > len[1])      k = (dist - len[1]) / dist;
    else if (dist < len[0]) k = (dist - len[0]) / dist;
    v.pos.x -= d.x * k; v.pos.y -= d.y * k; v.pos.z -= d.z * k;
    return true;
}

// Captura la pose de entrada antes de resolver los cinco subpasos. El solver
// conserva sus anclajes y esferas aunque otra malla reutilice los buffers VT.
void CapturePose(MeshCloth &c, void *model, const float *boneBase) {
    char *mesh = MeshPtr(model, kSkirtMesh);
    const char *verts = (const char *)*(int *)(mesh + 0x10);
    for (int v = 0; v < c.numVertices; ++v) {
        if (*(short *)(verts + v * 0x10) == kFixedBone) {
            c.vertices[v].anchor = PoseVertex(model, boneBase, verts + v * 0x10);
            c.vertices[v].state |= PVS_FIXEDPOS;
        }
    }
    for (int s = 0; s < c.numSpheres; ++s) {
        Sphere &sp = c.spheres[s];
        // Cambio de ejes del source: (x, y, z) -> (z, -y, x).
        float in[3] = { sp.centerLocal.z, -sp.centerLocal.y, sp.centerLocal.x };
        float out[3] = { 0.0f, 0.0f, 0.0f };
        BMD_TransformPosition(model, (float *)(boneBase + sp.bone * 12), in, out, '\x01');
        sp.center = { out[0], out[1], out[2] };
    }
}

// CPhysicsCloth::Move (una iteracion). Entradas de tiempo/viento explícitas;
// no consulta el modelo, huesos, buffers VT ni globals de simulación.
bool Step(MeshCloth &c, float wind, int sceneFrame, float angleZ, float fTime) {
    const float a = (180.0f + angleZ) * 3.14159265f / 180.0f;
    const Vec3 vWind = { wind * sinf(a), -wind * cosf(a), 0.0f };

    // InitForces (modo malla): la misma clave para todos los vertices.
    const int seed = ((sceneFrame / 10) * 101) % c.numVertices;
    const unsigned key = (unsigned)abs(seed % 10);
    for (int i = 0; i < c.numVertices; ++i) UpdateForce(c.vertices[i], key, c.type, vWind);

    // Resortes.
    for (int l = 0; l < c.numLinks; ++l) {
        const Link &lk = c.links[l];
        if (!(lk.style & PLS_SPRING)) continue;
        Vertex &v1 = c.vertices[lk.v[0]];
        Vertex &v2 = c.vertices[lk.v[1]];
        Vec3 d;
        float dist = GetDistance(v1, v2, &d);
        if (dist < 0.001f) dist = 0.001f;
        if (dist > lk.dist[1] + 0.01f) {
            float k = (dist - lk.dist[1]) / dist;
            if (c.type & PCT_OPT_CORRECTEDFORCE) k *= lk.dist[1] / 32.0f;
            const Vec3 f = { d.x * k, d.y * k, d.z * k };
            v1.force.x -= f.x; v1.force.y -= f.y; v1.force.z -= f.z;
            v2.force.x += f.x; v2.force.y += f.y; v2.force.z += f.z;
        }
    }

    // SetFixedVertices: los anclajes provienen de la muestra de pose capturada.
    for (int v = 0; v < c.numVertices; ++v) {
        if (c.vertices[v].state & PVS_FIXEDPOS)
            c.vertices[v].pos = c.vertices[v].anchor;
    }

    // Integracion.
    for (int i = 0; i < c.numVertices; ++i) {
        Vertex &v = c.vertices[i];
        if (v.state & PVS_FIXEDPOS) continue;
        v.vel.x += v.force.x * kInvOfMass * fTime;
        v.vel.y += v.force.y * kInvOfMass * fTime;
        v.vel.z += v.force.z * kInvOfMass * fTime;
        v.pos.x += v.vel.x * fTime; v.pos.y += v.vel.y * fTime; v.pos.z += v.vel.z * fTime;
    }

    // PreventFromStretching: esferas de colisión de la misma muestra de pose.
    if (c.numSpheres > 0) {
        for (int s = 0; s < c.numSpheres; ++s) {
            const Sphere &sp = c.spheres[s];
            for (int i = 0; i < c.numVertices; ++i) {
                Vec3 d = { c.vertices[i].pos.x - sp.center.x,
                           c.vertices[i].pos.y - sp.center.y,
                           c.vertices[i].pos.z - sp.center.z };
                float len = Length(d);
                if (len < 0.01f) { len = 0.01f; d.x = len; d.y = 0.0f; d.z = 0.0f; }
                if (len < sp.radius) {
                    const float k = (sp.radius - len) / len;
                    const Vec3 m = { d.x * k, d.y * k, d.z * k };
                    AddOneTimeMove(c.vertices[i], m);
                }
            }
        }
        for (int i = 0; i < c.numVertices; ++i) DoOneTimeMove(c.vertices[i]);
    }

    // Distancia "suelta": se acumula y se aplica en la proxima pasada.  En 5.2
    // el DoOneTimeMove que sigue recorre m_iNumVer * m_iNumHor, que en el modo
    // por malla es 0; se conserva ese comportamiento.
    for (int l = 0; l < c.numLinks; ++l) {
        const Link &lk = c.links[l];
        if (!(lk.style & PLS_LOOSEDISTANCE)) continue;
        Vertex &v1 = c.vertices[lk.v[0]];
        Vertex &v2 = c.vertices[lk.v[1]];
        Vec3 d;
        float dist = GetDistance(v1, v2, &d);
        if (dist < 0.001f) dist = 0.001f;
        const float k = (dist - lk.dist[1]) * 0.5f / dist;
        const Vec3 m = { d.x * k, d.y * k, d.z * k };
        v1.oneTimeMove.x -= m.x; v1.oneTimeMove.y -= m.y; v1.oneTimeMove.z -= m.z;
        v2.oneTimeMove.x += m.x; v2.oneTimeMove.y += m.y; v2.oneTimeMove.z += m.z;
        v1.countOneTimeMove++; v2.countOneTimeMove++;
    }

    // Distancia "estricta" (m_iNumHor = 0: aplica a todos los enlaces).
    for (int l = 0; l < c.numLinks; ++l) {
        const Link &lk = c.links[l];
        if (!(lk.style & PLS_STRICTDISTANCE)) continue;
        if (!KeepLength(c.vertices[lk.v[1]], c.vertices[lk.v[0]], lk.dist)) return false;
    }
    return true;
}

MeshCloth* FindSlot(const PART_t* part) {
    MeshCloth *freeSlot = nullptr, *oldest = &g_Slots[0];
    for (int i = 0; i < kSlots; ++i) {
        if (g_Slots[i].part == part) return &g_Slots[i];
        if (!g_Slots[i].part && !freeSlot) freeSlot = &g_Slots[i];
        if (g_Slots[i].lastFrame < oldest->lastFrame) oldest = &g_Slots[i];
    }
    MeshCloth *s = freeSlot ? freeSlot : oldest;
    if (s->part) Destroy(*s);
    s->part = part;
    return s;
}

} // namespace

// DESVIACION: la tela avanza al final de cada paso de 40 ms, incluso durante
// recuperación sin render. El dibujo sólo consume las posiciones publicadas.
void DivineSkirt_Tick(const OBJECT* entity, int modelType, const PART_t* part, void *model)
{
    if (modelType != kDivinePantsModel || !part || !model || !entity) return;
    if (*(short *)((char *)model + 0x24) <= kSkirtMesh) return;
    const float *bones = entity->BoneMatrices ? (const float *)entity->BoneMatrices : nullptr;
    if (!bones) return;
    MeshCloth *c = FindSlot(part);
    const int frame = (int)MoveSceneFrame;
    const int spawn = entity->InitialSceneFrame;
    if (c->vertices && (c->spawnFrame != spawn || frame - c->lastFrame > 3 || frame < c->lastFrame)) {
        Destroy(*c);
        c->part = part;
    }
    c->spawnFrame = spawn;
    if (!c->vertices && !Create(*c, model, bones)) { Destroy(*c); return; }
    c->presentationContinuous=c->lastFrame==frame-1;
    for (int v=0;v<c->numVertices;++v) c->vertices[v].presentationPrevious=c->vertices[v].pos;
    c->lastFrame = frame;
    CapturePose(*c, model, bones);
    const float angleZ = entity->Angle[2];
    for (int i = 0; i < 5; ++i) {
        if (!Step(*c, _DAT_00590af0, frame, angleZ, 0.005f)) {
            Destroy(*c);
            return;
        }
    }
}

// No crea ni avanza tela; el pase de brillo/sombra puede repetirse libremente.
void __cdecl DivineSkirt_Apply(const OBJECT* entity, int modelType, const PART_t* part, void *model)
{
    if (modelType != kDivinePantsModel || !part || !model || !entity) return;
    for (int i = 0; i < kSlots; ++i) {
        const MeshCloth &c = g_Slots[i];
        if (c.part != part || !c.vertices || c.spawnFrame != entity->InitialSceneFrame) continue;
        for (int v = 0; v < c.numVertices; ++v) {
            float *p = VT(kSkirtMesh, v);
            const Vertex &vertex=c.vertices[v];
            const float alpha=c.presentationContinuous && RenderInterpolation_OwnerContinuous(entity) ? RenderInterpolation_Alpha() : 1;
            p[0]=RenderBlend(vertex.presentationPrevious.x,vertex.pos.x,alpha);
            p[1]=RenderBlend(vertex.presentationPrevious.y,vertex.pos.y,alpha);
            p[2]=RenderBlend(vertex.presentationPrevious.z,vertex.pos.z,alpha);
        }
        return;
    }
}
