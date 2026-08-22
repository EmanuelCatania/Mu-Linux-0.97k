// stubs_misc_helpers.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 8815-9667 (853 lines).
//
// Mixed helper sections:
//   Filter BMD helpers           — image-filter BMD setup
//   Char-select click helpers    — called from char-select click handlers
//   Bone/3D sound helpers        — bone-attached 3D sound
//   SpringNode helpers           — spring-physics node ops
//   VerletNode helpers           — Verlet integration nodes
//   Widget ctor chain            — second widget type constructors
//   Scene entity grid helpers    — scene-graph grid utilities

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif

// ── Filter BMD helpers ────────────────────────────────────────────────────────
// Checksum is computed by walking the encrypted blob 4 bytes at a time, XOR/ADD
// alternating on a seed pointer, with a rotate-mix every 16th iteration.
// FUN_00479a50 @ 0x00479A50 — Filter_SaveBMD
// Copies 1000 word-filter entries (stride 0x14) from DAT_07d73104 into a
// 20000-byte heap buffer, BuxConvert_0-encrypts each entry, computes the
// checksum starting from &DAT_007cfa00, then writes buffer + checksum to file.
void __cdecl FUN_00479a50(const char* path)
{
    // Port of IDA sub_479A50 (Filter_SaveBMD).  Not called in the .bmd branch
    // (DAT_0055a7c4==1), only in the txt-source branch.  Kept for parity.
    FILE* pFVar1 = fopen(path, "wb");
    if (!pFVar1) return;
    BYTE* lpMem  = (BYTE*)operator_new(20000);
    char* src = DAT_07d73104;
    BYTE* dst = lpMem;
    char* srcEnd = DAT_07d73104 + sizeof(DAT_07d73104);
    while (src < srcEnd) {
        memcpy(dst, src, 0x14);
        FUN_00479910((int)dst, 0x14);
        src += 0x14;
        dst += 0x14;
    }
    uintptr_t acc = (uintptr_t)DAT_007cfa00;
    for (unsigned int i = 0; i <= 0x4e1c; i += 4) {
        unsigned int v = *(unsigned int*)(lpMem + i);
        unsigned int k = ((unsigned char)((i >> 2) - 1)) & 1;
        if (k == 0)        acc ^= v;
        else /* k == 1 */  acc += v;
        if ((i & 0xf) == 0)
            acc ^= (acc + 15997) >> (((i >> 2) & 7) + 1);
    }
    fwrite(lpMem, 20000, 1, pFVar1);
    fwrite(&acc,      4, 1, pFVar1);
    operator_delete(lpMem);
    fclose(pFVar1);
}

// FUN_00479b30 @ 0x00479B30 — Filter_LoadBMD (OpenFilterFile)
// Port of IDA sub_479B30 (raw/00479B30_OpenFilterFile.c).
// Reads 20000-byte blob + 4-byte checksum, validates ring checksum
// (seed 0x7cfa00, magic 15997), then BuxConvert_0-decrypts each 20-byte
// entry into DAT_07d73104[]; stops on empty-first-byte sentinel or when
// the 20000-byte target buffer is full; writes count into DAT_07d78070.
void __cdecl FUN_00479b30(const char* path)
{
    char local_100[256];
    FILE* Stream = fopen(path, "rb");
    if (!Stream) {
        sprintf(local_100, "%s - File not exist.", path);
        MessageBoxA(g_hWnd, local_100, NULL, 0);
        SendMessageA(g_hWnd, 2, 0, 0);
        return;
    }
    BYTE* Buffer = (BYTE*)operator_new(20000);
    uintptr_t Checksum = 0;
    fread(Buffer,     20000, 1, Stream);
    fread(&Checksum,      4, 1, Stream);
    fclose(Stream);
    // Validate ring checksum — IDA-exact.
    uintptr_t acc = (uintptr_t)DAT_007cfa00;  // 0x007cfa00 (literal seed)
    for (unsigned int i = 0; i <= 0x4e1c; i += 4) {
        unsigned int v = *(unsigned int*)(Buffer + i);
        unsigned int k = ((unsigned char)((i >> 2) - 1)) & 1;
        if (k == 0)        acc ^= v;
        else /* k == 1 */  acc += v;   // IDA: "if(k!=0){ if(k==1) add; }"
        if ((i & 0xf) == 0)
            acc ^= (acc + 15997) >> (((i >> 2) & 7) + 1);
    }
    if (Checksum != acc) {
        sprintf(local_100, "%s - File corrupted.", path);
        MessageBoxA(g_hWnd, local_100, NULL, 0);
        SendMessageA(g_hWnd, 2, 0, 0);
        operator_delete(Buffer);
        return;
    }
    // Decrypt and copy entries until sentinel (empty first byte) or buffer full.
    int count = 0;
    BYTE* src = Buffer;
    char* dst = DAT_07d73104;
    char* end = DAT_07d73104 + sizeof(DAT_07d73104);
    while (dst < end) {
        FUN_00479910((int)src, 0x14);
        memcpy(dst, src, 0x14);
        if (*dst == '\0') break;
        src   += 0x14;
        dst   += 0x14;
        count += 1;
    }
    if (dst < end) DAT_07d78070 = count;
    operator_delete(Buffer);
}

// FUN_00479cf0 @ 0x00479CF0 — FilterName_LoadData
// Reads word tokens from a text file via GetToken(); stores each into
// DAT_07d27610 (stride 0x14), incrementing DAT_07d78074 per token.
void __cdecl FUN_00479cf0(const char* path)
{
    DAT_07d7806c = (FILE*)fopen(path, DAT_005580ac);
    if (!DAT_07d7806c) return;
    int iVar2;
    while ((iVar2 = FUN_0050e2c0()) != 2) {
        char* dst = DAT_07d27610 + DAT_07d78074 * 0x14;
        int   len = (int)strlen(DAT_083a3ff4) + 1;  // TokenString
        memcpy(dst, DAT_083a3ff4, len);
        DAT_07d78074++;
    }
    fclose(DAT_07d7806c);
}

// FUN_00479d70 @ 0x00479D70 — FilterName_SaveBMD
// Mirror of FUN_00479a50 for the name filter (DAT_07d27610, seed DAT_00578200,
// magic 0x2bc1).
void __cdecl FUN_00479d70(const char* path)
{
    // Port of IDA sub_479D70 (FilterName_SaveBMD).  Not called in the .bmd
    // branch (DAT_0055a7c4==1), only in the txt-source branch.  Kept for parity.
    FILE* pFVar1 = fopen(path, "wb");
    if (!pFVar1) return;
    BYTE* lpMem  = (BYTE*)operator_new(20000);
    char* src = DAT_07d27610;
    BYTE* dst = lpMem;
    char* srcEnd = DAT_07d27610 + sizeof(DAT_07d27610);
    while (src < srcEnd) {
        memcpy(dst, src, 0x14);
        FUN_00479910((int)dst, 0x14);
        src += 0x14;
        dst += 0x14;
    }
    uintptr_t acc = (uintptr_t)DAT_00578200;
    for (unsigned int i = 0; i <= 0x4e1c; i += 4) {
        unsigned int v = *(unsigned int*)(lpMem + i);
        unsigned int k = ((unsigned char)((i >> 2) - 1)) & 1;
        if (k == 0)        acc ^= v;
        else /* k == 1 */  acc += v;
        if ((i & 0xf) == 0)
            acc ^= (acc + 11201) >> (((i >> 2) & 7) + 1);
    }
    fwrite(lpMem, 20000, 1, pFVar1);
    fwrite(&acc,      4, 1, pFVar1);
    operator_delete(lpMem);
    fclose(pFVar1);
}

// FUN_00479e50 @ 0x00479E50 — FilterName_LoadBMD (OpenNameFilterFile)
// Port of IDA sub_479E50 (raw/00479E50_OpenNameFilterFile.c).
// Mirror of FUN_00479b30 for the name filter (seed 0x578200, magic 11201,
// target DAT_07d27610[], count in DAT_07d78074).
void __cdecl FUN_00479e50(const char* path)
{
    char local_100[256];
    FILE* Stream = fopen(path, "rb");
    if (!Stream) {
        sprintf(local_100, "%s - File not exist.", path);
        MessageBoxA(g_hWnd, local_100, NULL, 0);
        SendMessageA(g_hWnd, 2, 0, 0);
        return;
    }
    BYTE* Buffer = (BYTE*)operator_new(20000);
    uintptr_t Checksum = 0;
    fread(Buffer,     20000, 1, Stream);
    fread(&Checksum,      4, 1, Stream);
    fclose(Stream);
    uintptr_t acc = (uintptr_t)DAT_00578200;  // 0x00578200 (literal seed)
    for (unsigned int i = 0; i <= 0x4e1c; i += 4) {
        unsigned int v = *(unsigned int*)(Buffer + i);
        unsigned int k = ((unsigned char)((i >> 2) - 1)) & 1;
        if (k == 0)        acc ^= v;
        else /* k == 1 */  acc += v;
        if ((i & 0xf) == 0)
            acc ^= (acc + 11201) >> (((i >> 2) & 7) + 1);
    }
    if (Checksum != acc) {
        sprintf(local_100, "%s - File corrupted.", path);
        MessageBoxA(g_hWnd, local_100, NULL, 0);
        SendMessageA(g_hWnd, 2, 0, 0);
        operator_delete(Buffer);
        return;
    }
    int count = 0;
    BYTE* src = Buffer;
    char* dst = DAT_07d27610;
    char* end = DAT_07d27610 + sizeof(DAT_07d27610);
    while (dst < end) {
        FUN_00479910((int)src, 0x14);
        memcpy(dst, src, 0x14);
        if (*dst == '\0') break;
        src   += 0x14;
        dst   += 0x14;
        count += 1;
    }
    if (dst < end) DAT_07d78074 = count;
    operator_delete(Buffer);
}
// FUN_0047b020 — implemented in src/Item/Dialog_Data.cpp  (Dialog_LoadBMD)
// FUN_00401040 — implemented in src/Item/Quest_Data.cpp   (Quest_LoadBMD)
// FUN_0047d120 — implemented in src/Item/NPC_Data.cpp     (NPCName_Load)

// ── Char-select click helpers (called from FUN_00402850/402f40) ───────────────
extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);

// CSQuest::clearQuest @ 0x00401960 — cierra la ventana de quest.
// 2026-08-21: acá había un "CharSelect_SendClickPacket" que SÓLO mandaba el
// paquete.  Le faltaban las dos cosas que realmente cierran el panel, así que
// el botón X (y cualquier otro camino de cierre) no hacía nada: el flag
// +0x1C87F seguía en 1, GetScreenWidth seguía devolviendo 450 y el panel
// quedaba dibujado para siempre.  IDA:
//     *(_BYTE *)(This + 116863) = 0;
//     CloseInventoryRelatedWindows();
//     send([C1][03][31]);
// El 0x31 (49) va como C1 plano — HackPacketCheck le da Encrypt = 0.
void __fastcall FUN_00401960(int param_1) {
    if (param_1 == 0) return;
    *(BYTE *)(param_1 + 0x1c87f) = 0;
    CloseInventoryRelatedWindows();
    BYTE pkt[3] = { 0xC1, 0x03, 0x31 };
    Net_SendC1Packet(pkt, 3);
}

// ── Bone / 3D sound helpers ───────────────────────────────────────────────────
// FUN_004409a0 — implemented in src/Math/Math_3D.cpp (Bone_TransformPos)
float __fastcall FUN_00407b50(void *a, int b, float *c);
void  __fastcall FUN_00407ac0(void *a, float x, float y, float z);
void  __fastcall FUN_00407af0(void *a, float v);
void  __fastcall FUN_004079b0(void *node, float x, float y, float z, int pinned);
void  __cdecl    FUN_004079e0(int _this, int a2, short a3);

// FUN_00408780 @ 0x00408780 — Cloth_PinTopRow (slot 1 de off_552520).
// Reposiciona los W nodos de la FILA SUPERIOR sobre la matriz del hueso al que
// está atada la tela y los marca como fijados. Éste es el enganche que hace que
// la capa siga al personaje: `sub_408CB0` lo invoca con
// `BoneTransform + 48 * boneIdx` en cada tick.
// Port FIEL de IDA `sub_408780`. Layout: +0x14 flags, +0x18/+0x1C origen Y/Z,
// +0x20 ancho, +0x28 W, +0x34 nodes, +0x40 paso horizontal.
void __cdecl FUN_00408780(int _this, float (*Matrix)[4])
{
  float Position[3];
  float WorldPosition[3];

  int v4 = *(int *)(_this + 40);
  for (int v3 = 0, v6 = 0; v3 < v4; ++v3, v6 += 60)
  {
    int    v7 = *(int   *)(_this + 20);   // flags
    double v8 = *(float *)(_this + 32);   // ancho total
    double v9 = *(float *)(_this + 64);   // paso entre columnas
    if ((*(BYTE *)(_this + 20) & 0xC) == 4)
    {
      v8 = v8 * 0.60000002;
      v9 = v8 / (double)(v4 - 1);
    }
    double v10 = (double)v3;

    Position[1] = *(float *)(_this + 24);
    Position[0] = (float)(v10 * v9 - v8 * 0.5);
    double v11  = *(float *)(_this + 28);
    Position[2] = *(float *)(_this + 28);

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

    int v15 = *(int *)(_this + 4);        // entity
    FUN_004409a0(
        (void *)(DAT_05828d58 + 0xbc * (int)*(short *)(v15 + 2)),
        (float *)Matrix,
        Position,
        WorldPosition,
        1);
    FUN_004079b0((void *)(v6 + *(int *)(_this + 52)),
                 WorldPosition[0], WorldPosition[1], WorldPosition[2], 1);
    v4 = *(int *)(_this + 40);
  }
}

// FUN_004089b0 @ 0x004089B0 — Cloth_Wind (slot 2 de off_552520).
// Elige un nodo "epicentro" que rota con el contador de frames y aplica las
// fuerzas a cada nodo con un peso que cae con la distancia Manhattan al
// epicentro (ráfagas que recorren la tela).
// Port de IDA `sub_4089B0`: ~85% del decompile es ruido de hash-table
// (ref-count sobre MoveSceneFrame + scrambling XOR), omitido per política del
// proyecto. Lo que queda es el cálculo de arriba, tal cual.
void __cdecl FUN_004089b0(DWORD *_this)
{
  int frame = (int)DAT_083a7c00;              // MoveSceneFrame
  int v17   = (int)_this[12];                 // nodeCount
  if (v17 <= 0)
    return;

  int v20 = 101 * (frame / 10) % v17;         // índice del epicentro
  int W   = (int)_this[10];
  if (W <= 0)
    return;

  for (int v18 = 0; v18 < (int)_this[12]; ++v18)
  {
    int dist = abs(v20 / W - v18 / W) + abs(v20 % W - v18 % W);
    FUN_004079e0((int)(_this[13] + 60 * v18), dist, (short)_this[5]);
  }
}

// FUN_00408cb0 @ 0x00408CB0 — Cloth_Integrate: medio tick de simulación.
//   1. vtable[2] — reparte viento/gravedad sobre la aceleración de cada nodo.
//   2. Springs marcados con flag 2: si el par está estirado más allá del
//      reposo, aplica una fuerza de restitución a AMBOS extremos
//      (×3 cuando (flags & 0x300) == 256).
//   3. vtable[1] — vuelve a fijar la fila superior al hueso.
//   4. Integra todos los nodos.
// Port FIEL de IDA `sub_408CB0`. El `a2@<st0>` del decompile es el retorno de
// `sub_407B50` (la distancia), no un parámetro: reconstruido.
void __fastcall FUN_00408cb0(int *param_1, float a3)
{
  DWORD *a1 = (DWORD *)param_1;
  float v25[3];

  // Mismo guard que en FUN_00408e30: la vtable[1] (Cloth_PinTopRow) deferencia
  // `BoneTransform + 48*boneIdx` sin validarlo.
  if (!a1 || !a1[1] || !*(DWORD *)(a1[1] + 276))
    return;

  (*(void (__cdecl **)(DWORD *))(*a1 + 8))(a1);

  for (int v20 = 0, v4 = 0; v20 < (int)a1[14]; ++v20, v4 += 16)
  {
    float *v5 = (float *)(a1[15] + v4);
    if ((*(BYTE *)&v5[3] & 2) == 0)
      continue;

    int    v6 = a1[13];
    char  *v7 = (char  *)(v6 + 60 * *(short *)v5);
    float *v8 = (float *)(v6 + 60 * *((short *)v5 + 1));

    double d = FUN_00407b50(v7, (int)v8, v25);
    if (d < 0.001)
      d = 0.001;

    if (d > v5[2] + 0.0099999998)
    {
      double v9  = d - v5[2];
      int    v10 = a1[5] & 0x300;
      float  f[3];
      for (int i = 0; i < 3; ++i)
      {
        double v12 = v9 * v25[i] / d;
        f[i] = (float)v12;
        if (v10 == 256)
          f[i] = (float)(v12 * 3.0);
      }
      FUN_00407ac0((float *)v7, -f[0], -f[1], -f[2]);
      FUN_00407ac0(v8, f[0], f[1], f[2]);
    }
  }

  (*(void (__cdecl **)(DWORD *, int))(*a1 + 4))(a1, 48 * a1[2] + *(int *)(a1[1] + 276));

  for (int v15 = 0, v16 = 0; v15 < (int)a1[12]; ++v15, v16 += 60)
    FUN_00407af0((float *)(v16 + a1[13]), a3);
}

// ═══════════════════════════════════════════════════════════════════════════
// Sistema de tela (capa del MG) — SOLVER. Port 2026-08-11.
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

float __fastcall FUN_00407b50(void *a, int b, float *c);          // delta + |delta|
char  __fastcall FUN_00407d10(int a1);                            // aplica corrección
void  __cdecl    FUN_00407c60(int a1, int a3, float rest);        // spring de igualdad
int   __cdecl    FUN_00407b90(int a1, int a3, const float *range);// spring de rango

// FUN_004f9c40 @ 0x004f9c40 — Vec3_Length: returns sqrt(dot(v,v)), does NOT modify v.
// (Ghidra shows return as float10 left in x87 ST0; callers use the return value as distance.)
// FUN_004f9c40 (IDA-activated, was Ghidra stub)
float __cdecl FUN_004f9c40(float *a1)
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


// FUN_00407ac0 @ 0x00407ac0 — VerletNode_AddAccel: accumulates acceleration at +4/+8/+C
void __fastcall FUN_00407ac0(void *a, float x, float y, float z) {
    char *p = (char*)a;
    *(float*)(p+4)  += x;
    *(float*)(p+8)  += y;
    *(float*)(p+0xc)+= z;
}

// FUN_00407af0 @ 0x00407AF0 — ClothNode_Integrate: paso de Euler semi-implícito.
// Layout: +0x04 accel[3], +0x10 vel[3], +0x1C pos[3], +0x28 pinned(byte).
// _DAT_00559070 = 400.0 (leído del binario). Port FIEL de IDA `sub_407AF0`.
//
// 2026-08-11: el port limpiaba la aceleración al final. Eso NO está en IDA —
// la aceleración la reescribe entera `sub_4079E0` (gravedad + viento) al
// principio de cada tick, así que el clear extra era inofensivo pero falso.
void __fastcall FUN_00407af0(void *a, float v) {
    char *p = (char*)a;
    if (*(BYTE*)(p+0x28) & 1) return;  // nodo fijado — no se integra
    for (int i = 0; i < 3; i++) {
        double acc = _DAT_00559070 * *(float*)(p+0x04 + i*4);
        double vel = acc * v + *(float*)(p+0x10 + i*4);
        *(float*)(p+0x10 + i*4) = (float)vel;
        *(float*)(p+0x1c + i*4) = (float)(vel * v + *(float*)(p+0x1c + i*4));
    }
}

// FUN_004079e0 @ 0x004079E0 — ClothNode_ApplyForces: reescribe la aceleración
// del nodo con viento + arrastre, y le suma la gravedad.
//   a2 = distancia Manhattan al nodo "epicentro" de la ráfaga (la calcula
//        `sub_4089B0`); a 5 celdas la ráfaga no llega (peso 0).
//   a3 = flags del widget.
// Port FIEL de IDA `sub_4079E0`. El bucle sobre `&flt_590AF4 .. &unk_590B00`
// son los 3 componentes del viento; en nuestro build esos globals no son
// contiguos, así que se despliega explícitamente.
void __cdecl FUN_004079e0(int _this, int a2, short a3)
{
  if ((*(BYTE *)(_this + 40) & 1) != 0)
  {
    *(DWORD *)(_this + 12) = 0;
    *(DWORD *)(_this + 8)  = 0;
    *(DWORD *)(_this + 4)  = 0;
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
    double drag = *(float *)(_this + 0x10 + i * 4) * 0.0099999998;
    *(float *)(_this + 0x04 + i * 4) = (float)(v4 * wind[i] - drag);
  }

  if ((a3 & 0x300) == 256)
    *(float *)(_this + 12) = (float)((_DAT_00590af0 + 0.1) * v4 + *(float *)(_this + 12));

  double v9 = _DAT_0055906c * _DAT_00559068;
  if ((a3 & 0xC00) == 1024)
    *(float *)(_this + 12) = (float)(*(float *)(_this + 12) - v9 * 180.0);
  else
    *(float *)(_this + 12) = (float)(*(float *)(_this + 12) - v9 * 100.0);
}

// FUN_00407b50 @ 0x00407b50 — SpringNode_Delta: out = posA - posB; returns distance (length of delta).
// param_1=nodeA_ptr (this), param_2=nodeB_ptr (as int), param_3=float[3] output.
// Does NOT normalize; caller uses return value as rest-length distance.
float __fastcall FUN_00407b50(void *a, int b, float *c) {
    float *posA = (float*)((char*)a + 0x1c);
    float *posB = (float*)(b + 0x1c);
    c[0] = posA[0] - posB[0];
    c[1] = posA[1] - posB[1];
    c[2] = posA[2] - posB[2];
    return FUN_004f9c40(c);
}

// FUN_00407b90 @ 0x00407B90 — Cloth_SpringRange: mantiene |a1-a3| dentro de
// [range[0], range[1]]. Devuelve 0 sólo si el spring está "roto" (estirado más
// de 20x el máximo) — eso corta la iteración del solver en `sub_408E30`.
// Port FIEL de IDA `sub_407B90` (el `a2@<st0>` es el retorno de sub_407B50).
// Corrige SÓLO la posición de a1 (+0x1C..+0x24), no la del partner.
int __cdecl FUN_00407b90(int a1, int a3, const float *range)
{
  float v9[3];
  double diff;

  if ((*(BYTE *)(a1 + 40) & 1) != 0)
    return 1;                                   // nodo fijado

  double d = FUN_00407b50((void *)a1, a3, v9);
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
  *(float *)(a1 + 28) = *(float *)(a1 + 28) - v9[0];
  *(float *)(a1 + 32) = *(float *)(a1 + 32) - v9[1];
  *(float *)(a1 + 36) = *(float *)(a1 + 36) - v9[2];
  return 1;
}

// FUN_00407c60 @ 0x00407C60 — Cloth_SpringEqual: corrección simétrica hacia la
// longitud de reposo. NO toca la posición: acumula en el buffer de corrección
// (+0x30..+0x38) e incrementa el contador (+0x2C); `sub_407D10` promedia y
// aplica. Port FIEL de IDA `sub_407C60` (verificado contra el disasm).
void __cdecl FUN_00407c60(int a1, int a3, float rest)
{
  float v8[3];

  double d = FUN_00407b50((void *)a1, a3, v8);
  if (d < 0.001)
    d = 0.001;

  double f = (d - rest) * 0.5 / d;
  v8[0] = (float)(f * v8[0]);
  v8[1] = (float)(f * v8[1]);
  v8[2] = (float)(f * v8[2]);

  *(float *)(a1 + 48) = *(float *)(a1 + 48) - v8[0];
  *(float *)(a1 + 52) = *(float *)(a1 + 52) - v8[1];
  *(float *)(a1 + 56) = *(float *)(a1 + 56) - v8[2];
  *(float *)(a3 + 48) = v8[0] + *(float *)(a3 + 48);
  *(float *)(a3 + 52) = v8[1] + *(float *)(a3 + 52);
  *(float *)(a3 + 56) = v8[2] + *(float *)(a3 + 56);
  ++*(DWORD *)(a1 + 44);
  ++*(DWORD *)(a3 + 44);
}

// FUN_00407d10 @ 0x00407d10 — VerletSystem_Flush: apply accumulated position corrections, zero buffer
// FUN_00407d10 (IDA-activated, was Ghidra stub)
char __fastcall FUN_00407d10(int a1)
{
  int v1; // eax
  float *v2; // esi
  int v3; // edx
  double v4; // st7
  double v5; // st7
  double v6; // st7

  *((BYTE*)&v1) = *(BYTE *)(a1 + 40);
  if ( (v1 & 1) != 0 )
  {
    *(DWORD *)(a1 + 56) = 0;
    *(DWORD *)(a1 + 52) = 0;
    *(DWORD *)(a1 + 48) = 0;
  }
  else
  {
    v1 = *(DWORD *)(a1 + 44);
    if ( v1 > 0 )
    {
      v2 = (float *)(a1 + 48);
      v1 = a1 + 48;
      v3 = 3;
      do
      {
        v1 += 4;
        --v3;
        *(float *)(v1 - 4) = *(float *)(v1 - 4) / (double)*(int *)(a1 + 44);
      }
      while ( v3 );
      v4 = *v2 + *(float *)(a1 + 28);
      *v2 = 0.0;
      *(DWORD *)(a1 + 44) = 0;
      *(float *)(a1 + 28) = v4;
      v5 = *(float *)(a1 + 52) + *(float *)(a1 + 32);
      *(DWORD *)(a1 + 52) = 0;
      *(float *)(a1 + 32) = v5;
      v6 = *(float *)(a1 + 36) + *(float *)(a1 + 56);
      *(DWORD *)(a1 + 56) = 0;
      *(float *)(a1 + 36) = v6;
    }
  }
  return v1;
}


// FUN_00407e10 @ 0x00407e10 — VerletNode_SetTarget: stores vec3 at +0x14..+0x1c
void __fastcall FUN_00407e10(void *a, float b, int c, int d) {
    char *p = (char*)a;
    *(float*)(p + 0x14) = b;
    *(float*)(p + 0x18) = *(float*)&c;
    *(float*)(p + 0x1c) = *(float*)&d;
}

// FUN_00407e30 @ 0x00407e30 — VerletNode_GetPos: reads vec3 from +0x04..+0x0c into out[]
void __fastcall FUN_00407e30(void *a, float *b) {
    char *p = (char*)a;
    b[0] = *(float*)(p + 0x04);
    b[1] = *(float*)(p + 0x08);
    b[2] = *(float*)(p + 0x0c);
}

// ── SpringNode helpers (FUN_00407950 family) ──────────────────────────────

// FUN_00407980 @ 0x00407980 — SpringNode_ZeroFields: zeroes accel/vel/pos/pinned in 0x3c stride node.
void __fastcall FUN_00407980(int param_1) {
    *(int *)(param_1 + 0x2c) = 0;
    for (int i = 0; i < 3; i++) {
        *(int *)(param_1 + 0x1c + i*4) = 0;   // pos[i]
        *(int *)(param_1 + 0x10 + i*4) = 0;   // vel[i]
        *(int *)(param_1 + 0x04 + i*4) = 0;   // accel[i]
        *(int *)(param_1 + 0x30 + i*4) = 0;   // extra (puVar1[5..7])
    }
    *(BYTE *)(param_1 + 0x28) = 0;
}

// FUN_00407950 @ 0x00407950 — SpringNode_Ctor: set vtable + zero fields.
// FUN_00407950 (IDA-activated, was Ghidra stub)
void __fastcall FUN_00407950(void *_this)
{
  extern void *g_ClothNodeVTable[1];
  *(DWORD *)_this = (DWORD)g_ClothNodeVTable;
  FUN_00407980((int)_this);
}


// FUN_004079b0 @ 0x004079B0 — SpringNode_SetPos: write position and pinned flag.
void __fastcall FUN_004079b0(void *node, float x, float y, float z, int pinned) {
    char *p = (char*)node;
    *(float *)(p + 0x1c) = x;
    *(float *)(p + 0x20) = y;
    *(float *)(p + 0x24) = z;
    if (pinned)
        *(BYTE *)(p + 0x28) |= 1;
}

// FUN_00407b30 @ 0x00407B30 — SpringNode_GetPos: copy position (+0x1c..+0x24) to out[3].
void __fastcall FUN_00407b30(void *node, float *out) {
    float *src = (float *)((char*)node + 0x1c);
    out[0] = src[0];
    out[1] = src[1];
    out[2] = src[2];
}

// FUN_004088b0 @ 0x004088B0 — Spring_StoreEdge: write spring entry (stride 0x10) into spring array.
// Entry layout: [ushort na][ushort nb][float rest_scaled][float dist][byte flags][3 bytes pad]
void __fastcall FUN_004088b0(void *sys, int idx, short na, short nb,
                              float rest_scaled, float dist, BYTE edge_flags) {
    char *arr = *(char **)((char*)sys + 0x3c);
    int off = idx * 0x10;
    *(short  *)(arr + off + 0x00) = na;
    *(short  *)(arr + off + 0x02) = nb;
    *(float  *)(arr + off + 0x04) = rest_scaled;
    *(float  *)(arr + off + 0x08) = dist;
    *(BYTE   *)(arr + off + 0x0c) = edge_flags;
}

// ── VerletNode helpers (FUN_00407da0 family) ─────────────────────────────

// FUN_00407df0 @ 0x00407DF0 — VerletNode_ZeroFields: zero +4..+1c (7 dwords).
void __fastcall FUN_00407df0(int param_1) {
    *(int *)(param_1 + 0x04) = 0;
    *(int *)(param_1 + 0x08) = 0;
    *(int *)(param_1 + 0x0c) = 0;
    *(int *)(param_1 + 0x10) = 0;
    *(int *)(param_1 + 0x14) = 0;
    *(int *)(param_1 + 0x18) = 0;
    *(int *)(param_1 + 0x1c) = 0;
}

// FUN_00407da0 @ 0x00407DA0 — VerletNode_CtorBase: set vtable (skipped) + zero fields.
void* __fastcall FUN_00407da0(void *param_1) {
    FUN_00407df0((int)param_1);
    return param_1;
}

// FUN_00407ed0 @ 0x00407ED0 — VerletNode_CtorExt: zero fields + clear +0x20.
// FUN_00407ed0 (IDA-activated, was Ghidra stub)
int __cdecl FUN_00407ed0(DWORD *_this)
{
  int result; // eax

  FUN_00407df0((int)_this);
  result = (int)_this;
  _this[8] = 0;
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
void __cdecl FUN_00407f20(float *_this, int *node);
void __fastcall FUN_00407ea0(int ecx, int edx, BYTE param_1);
// Adaptador: el dtor está portado como __fastcall (this en ECX, `edx` dummy) y
// los call sites de este build invocan la vtable como `__cdecl (this, flags)`.
static void __cdecl ClothAnchor_VSlot0(void *a, char flags) { FUN_00407ea0((int)a, 0, (BYTE)flags); }
static void __cdecl ClothAnchor_VSlot1(void *a) { (void)a; }   // 0x410D80 — sin portar, sin call sites
void *g_ClothAnchorVTable[3] = {
    (void *)ClothAnchor_VSlot0,
    (void *)ClothAnchor_VSlot1,
    (void *)FUN_00407f20
};

// vtable del NODO de tela (DAT_005524e8 del binario) = { 0x00408680 }.
void __cdecl FUN_00408680(void *_this, char flags);
void *g_ClothNodeVTable[1] = { (void *)FUN_00408680 };

// FUN_00407e50 @ 0x00407E50 — ClothAnchor_Ctor: full constructor (base + ext).
// FUN_00407e50 (IDA-activated, was Ghidra stub)
DWORD *__cdecl FUN_00407e50(DWORD *_this)
{
  FUN_00407da0(_this);
  *_this = (DWORD)g_ClothAnchorVTable;
  FUN_00407ed0(_this);
  return _this;
}

// FUN_00407d70 @ 0x00407D70 — ClothNode_AddCorrection: acumula una corrección
// de posición en el buffer (+0x30..+0x38) e incrementa el contador (+0x2C).
// Port FIEL de IDA `sub_407D70`.
void __cdecl FUN_00407d70(int _this, const float *a2)
{
  *(float *)(_this + 48) = a2[0] + *(float *)(_this + 48);
  *(float *)(_this + 52) = a2[1] + *(float *)(_this + 52);
  double v3 = a2[2] + *(float *)(_this + 56);
  ++*(DWORD *)(_this + 44);
  *(float *)(_this + 56) = (float)v3;
}

// FUN_00407f20 @ 0x00407F20 — ClothAnchor_Collide (slot 2 de off_552514).
// Esfera de colisión: si el nodo está DENTRO del radio (+0x20) del ancla, lo
// empuja hacia afuera. `_this[5..7]` (+0x14..+0x1C) es la posición de MUNDO
// del ancla, que `sub_408E30` refresca cada iteración desde el hueso.
// Port FIEL de IDA `sub_407F20`.
void __cdecl FUN_00407f20(float *_this, int *node)
{
  float v[3];

  FUN_00407b30(node, v);
  v[0] = v[0] - _this[5];
  v[1] = v[1] - _this[6];
  v[2] = v[2] - _this[7];

  double v3 = FUN_004f9c40(v);
  if (v3 < 0.0099999998)
  {
    v3   = 0.0099999998;
    v[0] = 0.0099999998f;
    v[1] = 0.0f;
    v[2] = 0.0f;
  }
  if (v3 < _this[8])
  {
    double v4 = (_this[8] - v3) / v3;
    v[0] = (float)(v4 * v[0]);
    v[1] = (float)(v4 * v[1]);
    v[2] = (float)(v4 * v[2]);
    FUN_00407d70((int)node, v);
  }
}

// FUN_00409310 @ 0x00409310 — Cloth_CollideAnchors: para cada ancla de la
// lista, invoca su vtable[2] contra los `nodeCount` nodos de la grilla, y
// después hace flush de las correcciones acumuladas.
// Port FIEL de IDA `sub_409310`. Ojo: todo el bloque está gateado por
// `this[18] > 0` (cantidad de anclas), así que sin anclas es un no-op.
int __cdecl FUN_00409310(DWORD *_this)
{
  int result = _this[18];
  if (result <= 0)
    return result;

  DWORD *v3 = *(DWORD **)(_this[19] + 8);
  if (v3 != (DWORD *)_this[20])
  {
    for (DWORD *i = *(DWORD **)(_this[19] + 8); v3; i = v3)
    {
      int v4 = *v3;                       // el objeto ancla
      if ((int)_this[12] > 0)
      {
        for (int v5 = 0, v6 = 0; v5 < (int)_this[12]; ++v5, v6 += 60)
          (*(void (__cdecl **)(int, int))(*(DWORD *)v4 + 8))(v4, v6 + _this[13]);
        v3 = i;
      }
      v3 = (DWORD *)v3[2];
      if ((DWORD *)_this[20] == v3)
        break;
    }
  }

  result = _this[12];
  for (int v7 = 0, v8 = 0; v7 < result; ++v7, v8 += 60)
  {
    FUN_00407d10(v8 + _this[13]);
    result = _this[12];
  }
  return result;
}


// FUN_00407ef0 @ 0x00407EF0 — ClothAnchor_SetParams.
// Fields: +4/+8/+0xc = posición LOCAL, +0x20 = radio, +0x10 = índice de HUESO.
//
// 2026-08-11: el último parámetro era `float`. En IDA (`sub_407EF0`) es
// `this[4] = a6` — un DWORD entero, y `sub_408E30` lo usa como
// `48 * v5[4]` para indexar la matriz de huesos. Con 17.0f guardado como
// float, `v5[4]` valía 0x41880000 y el índice se iba a 52 GB del arranque
// de la tabla. Los call sites de IDA lo confirman: los 5 primeros args son
// bits de float y el 6º un entero chico (2, 10, 17, 18, 19).
void __fastcall FUN_00407ef0(void *node, float p1, float p2, float p3, float radius, int boneIdx) {
    char *p = (char*)node;
    *(float *)(p + 0x04) = p1;
    *(float *)(p + 0x08) = p2;
    *(float *)(p + 0x0c) = p3;
    *(float *)(p + 0x20) = radius;
    *(int   *)(p + 0x10) = boneIdx;
}

// Entity_DrawByType.cpp dependencies
// FUN_00440d30 @ 0x00440D30 — BMD_PushMatrix: calls glPushMatrix().
void __cdecl FUN_00440d30(void) { glPushMatrix(); }
// FUN_00440d50 @ 0x00440D50 — BMD_DrawMesh: implemented in Render/BMD_DrawMesh.cpp
// FUN_00441be0 @ 0x00441BE0 — Model_KillAnim
// Iterates bone meshes; for each vertex in bone 'b', spawns kill/death particle effects.
void __cdecl FUN_00441be0(void *model, int param_1, int param_2)
{
    char *this_ = (char*)model;
    if (*(short*)(this_ + 0x24) == 0) return;
    float scale[6] = { 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
    int meshCount = *(short*)(this_ + 0x24);
    int iVar3 = param_1;
    for (int local_14 = 0; local_14 < meshCount; local_14++) {
        int iVar1 = *(int*)(this_ + 0x28) + local_14 * 0x28;
        int meshVerts = *(short*)(iVar1 + 10);
        if (meshVerts <= 0) continue;
        for (int local_20 = 0; local_20 < meshVerts; local_20++) {
            short *psVar6 = (short*)(local_20 * 0x24 + *(int*)(iVar1 + 0x1c));
            if ((char)*psVar6 <= 0) continue;
            for (int iVar7 = 0; iVar7 < (char)*psVar6; iVar7++) {
                int vertIdx = psVar6[iVar7 + 1];
                float *vpos = (float*)((char*)(char*)&DAT_0584621c + (vertIdx + iVar3 * 15000) * 3 * 4);
                if (param_2 == 0x104) {
                    if (_rand() % 2 == 0)
                        FUN_00460dc0(0x105, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                    if (_rand() % 10 == 0)
                        FUN_00460dc0(0x104, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                } else if (param_2 == 0x106) {
                    scale[0] = scale[1] = scale[2] = 0.2f;
                    if (_rand() % 12 == 0)
                        FUN_00460dc0(0x107, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                    if (_rand() % 50 == 0)
                        FUN_00460dc0(0x106, vpos, scale+3, scale, nullptr, nullptr, (float*)(UINT_PTR)0xffffffff, nullptr, 0);
                }
            }
        }
    }
}

// FUN_00441f00 @ 0x00441F00 — Model_RenderShadow
// Renders shadow triangles for all mesh bones except bone param_2 (using anim param_1).
void __cdecl FUN_00441f00(void *model, int param_1, int param_2)
{
    char *this_ = (char*)model;
    if (*(short*)(this_ + 0x24) == 0) return;
    FUN_00511590('\0');
    FUN_00511530();
    FUN_00440d30();
    int meshCount = *(short*)(this_ + 0x24);
    int local_20 = 0;
    for (int local_14 = 0; local_14 < meshCount; local_14++, local_20 += 15000) {
        int iVar5 = *(int*)(this_ + 0x28) + local_14 * 0x28;
        if (*(short*)(iVar5 + 10) <= 0) { continue; }
        if (*(short*)(iVar5 + 2) == (short)param_1) { continue; }
        if (local_14 == param_2) { continue; }
        glBegin(GL_TRIANGLES);
        int polyCount = *(short*)(iVar5 + 10);
        for (int local_1c = 0; local_1c < polyCount; local_1c++) {
            short *psVar1 = (short*)(local_1c * 0x24 + *(int*)(iVar5 + 0x1c));
            for (int iVar7 = 0; iVar7 < (char)*psVar1; iVar7++) {
                int vertIdx = (int)psVar1[iVar7 + 1] + local_20;
                float *vp = (float*)((char*)(char*)&DAT_0584621c + vertIdx * 3 * 4);
                float lc = vp[0] - *(float*)(this_ + 0x6c);
                float lz = vp[2] - *(float*)(this_ + 0x74);
                float shadow_x = lc + *(float*)(this_ + 0x6c);
                float shadow_y = *(float*)(this_ + 0x70);
                float shadow_z = *(float*)(this_ + 0x74);
                glVertex3f(shadow_x, shadow_y, shadow_z);
            }
        }
        glEnd();
    }
    glPopMatrix();
    FUN_00511510();
}




// FUN_0040a660 @ 0x0040A660 — WidgetB_CtorFull: construct two linked WidgetB objects.
// Calls FUN_00409ed0 on self, sets vtable, then constructs a local WidgetB and copies it.
void* __fastcall FUN_0040a660(void *param_1)
{
    FUN_00409ed0(param_1);
    // vtable = &PTR_FUN_00552588 — skipped

    // local_2c is a stack-local WidgetB (0x20 bytes, 8 dwords)
    int local_2c[8] = {};
    FUN_00409ed0(local_2c);
    FUN_00409f10(local_2c);
    return param_1;
}

// FUN_0040a860 @ 0x0040A860 — VTable_Release
// Iterates object->edge_list and calls FUN_0040a8f0 on each edge.
void __cdecl FUN_0040a860(void *obj, int param_1)
{
    char *this_ = (char*)obj;
    int count = *(int*)(this_ + 0x18);
    if (count <= 0) return;
    for (int i = 0; i < count; i++) {
        short *psVar1 = (short*)(*(int*)(this_ + 0x1c) + i * 10);
        int iVar2 = (int)psVar1[2] * 15000;
        float *p1 = (float*)(param_1 + (*psVar1 + iVar2) * 12);
        float *p2 = (float*)(param_1 + (psVar1[1] + iVar2) * 12);
        FUN_0040a8f0(obj, p1, p2);
    }
}

// ── Scene entity grid helpers ─────────────────────────────────────────────────
// FUN_004ffcc0 @ 0x004FFCC0 — Entity_GridUnlink(node, cell_ptr)
// Unlinks node from its doubly-linked list in the 16x16 scene grid.
void __cdecl FUN_004ffcc0(void *param_1_v, int param_2) {
    char *param_1 = (char*)param_1_v;
    if (!param_1) return;
    int iVar1 = *(int*)(param_1 + 0x1b8); // next
    int iVar2 = *(int*)(param_1 + 0x1b4); // prev
    if (iVar1 != 0) {
        if (iVar2 != 0) {
            *(int*)(iVar2 + 0x1b8) = iVar1;
            *(int*)(iVar1 + 0x1b4) = *(int*)(param_1 + 0x1b4);
            operator_delete(param_1);
            return;
        }
        *(int*)(iVar1 + 0x1b4) = 0;
        *(int*)(param_2 + 4) = iVar1;
        operator_delete(param_1);
        return;
    }
    if (iVar2 != 0) {
        *(int*)(iVar2 + 0x1b8) = 0;
        *(int*)(param_2 + 8) = iVar2;
        operator_delete(param_1);
        return;
    }
    *(int*)(param_2 + 4) = 0;
    *(int*)(param_2 + 8) = 0;
    operator_delete(param_1);
}

// FUN_00404ad0 @ 0x00404AD0 — Sound_FreeChannel(Buffer)
// Releases DirectSound buffers for the given slot (all loaded channels),
// resets slot count and 3D-anchor table.
//
// BUG-FIX 2026-04-28: el IDA original usaba `g_lpDSBuffer[0][v3]` con
// v3 = 4*Buffer + channel — un acceso flatten que el compilador C++ trata
// como "fila 0, índice fuera de rango". MSVC en Release lo computa offset-
// based (funciona) pero con ITERACIONES ilimitadas (MaxBufferChannel sin
// clamp) leía mucho más allá del array, devolviendo basura tipo 0xC2A00000
// (-80.0f bit-pattern) → v4->Release() → AV.
//
// Cambios:
//   1. Indexar con 2D plano: g_lpDSBuffer[Buffer][channel].
//   2. Clamp MaxBufferChannel a [0, 4] — array tiene exactamente 4 canales.
//   3. Bounds-check Buffer < 420.
//   4. Enable3DSound check usa el slot Buffer, no v2 (era bug del IDA).
HRESULT __cdecl FUN_00404ad0(int Buffer)
{
  if (!g_EnableSound) return 0;
  if (Buffer < 0 || Buffer >= 420) return 0;

  int channels = MaxBufferChannel[Buffer];
  if (channels < 0) channels = 0;
  if (channels > 4) channels = 4;

  for (int ch = 0; ch < channels; ++ch) {
    LPDIRECTSOUNDBUFFER v4 = g_lpDSBuffer[Buffer][ch];
    if (v4) {
      v4->Release();
      g_lpDSBuffer[Buffer][ch] = nullptr;
    }
    if (Enable3DSound[Buffer]) {
      LPDIRECTSOUND3DBUFFER v5 = g_lpDS3DBuffer[Buffer][ch];
      if (v5) {
        v5->Release();
        g_lpDS3DBuffer[Buffer][ch] = nullptr;
      }
    }
  }
  MaxBufferChannel[Buffer] = 0;
  Object3DSound[Buffer][0] = 0;
  Object3DSound[Buffer][1] = 0;
  Object3DSound[Buffer][2] = 0;
  Object3DSound[Buffer][3] = 0;
  if (SoundLoadCount > 0) --SoundLoadCount;
  return 0;
}

