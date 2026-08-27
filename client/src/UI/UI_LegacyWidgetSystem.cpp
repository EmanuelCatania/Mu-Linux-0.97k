// UI_LegacyWidgetSystem.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// FUN_00408cb0 vive ahora en Scene/Scene_CharSelect_Nav.cpp y FUN_00408e30 en
// Net/Crypto.cpp; antes se definian en este archivo.
void __fastcall FUN_00408cb0(int*, float);
int  __cdecl    FUN_00408e30(DWORD *a1);

#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl FUN_0054158c(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

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


// Widget / skill-beam system
// FUN_004086e0 @ 0x004086E0 — Widget_Release(widget_ptr) [__fastcall]
// Releases a widget/beam linked-list container.  Clean decompile, 35 lines, __fastcall(int).
// Logic:
//   1. Walk forward-linked list from *(int**)(param1+0x4c)+8 to sentinel *(param1+0x50):
//      for each node: if node->vtable != 0, call vtable[0](1)  (decref child)
//   2. Clear back-pointer: *(*(param1+0x50)+4)+8 = 0
//   3. Free all list nodes (operator_delete each)
//   4. Relink sentinel: head->prev = param1+0x4c, tail->next = param1+0x50
//   5. *(param1+0x48) = 0  (count = 0)
//   6. operator_delete(*(param1+0x3c))  (free internal buffer)
//   7. if *(param1+0x34) != 0: call vtable[0](3) on it  (parent release)
//   8. FUN_004080f0(param1)  (base widget destructor — not declared)
// FUN_004080f0 @ 0x004080F0 — Widget_NodeInit
// Zeros all node fields and sets the four scale floats to 1.0f.
void __cdecl FUN_004080f0(int param_1)
{
    *(int *)(param_1 + 0x04) = 0; *(int *)(param_1 + 0x08) = 0;
    *(int *)(param_1 + 0x0c) = 0; *(int *)(param_1 + 0x10) = 0;
    *(int *)(param_1 + 0x14) = 0; *(int *)(param_1 + 0x18) = 0;
    *(int *)(param_1 + 0x1c) = 0;
    *(unsigned int *)(param_1 + 0x20) = 0x3f800000u; // 1.0f
    *(unsigned int *)(param_1 + 0x24) = 0x3f800000u; // 1.0f
    *(int *)(param_1 + 0x28) = 0; *(int *)(param_1 + 0x2c) = 0;
    *(int *)(param_1 + 0x30) = 0; *(int *)(param_1 + 0x34) = 0;
    *(int *)(param_1 + 0x38) = 0; *(int *)(param_1 + 0x3c) = 0;
    *(unsigned int *)(param_1 + 0x40) = 0x3f800000u; // 1.0f
    *(unsigned int *)(param_1 + 0x44) = 0x3f800000u; // 1.0f
}

// FUN_004086e0 @ 0x004086E0 — Widget_Release(widget, 0, 0)
// Iterates linked list at (*(widget+0x4c)+8), calls vtable[0](1) on each node,
// frees list nodes, relinks head/tail sentinels, zeroes count, frees buffer at
// widget+0x3c, calls vtable[0](3) on widget+0x34, then calls Widget_BaseRelease.
void __cdecl FUN_004086e0(int param_1, int, int) {
    int *piVar1 = *(int **)(*(int *)(param_1 + 0x4c) + 8);
    if (piVar1 != *(int **)(param_1 + 0x50)) {
        do {
            if (piVar1 == nullptr) break;
            if (*piVar1 != 0) {
                // IDA `sub_4086E0` L11: `(**vtable)(obj, 1)`. Los objetos de
                // esta lista son ANCLAS (0x24 B, vtable off_552514), no nodos
                // de la grilla (0x3C B, vtable 0x5524E8): llamaba al dtor
                // equivocado. Con la vtable del ancla ya cableada
                // (g_ClothAnchorVTable), se despacha por ella como el original.
                void **vt = *(void ***)(*piVar1);
                if (vt) ((void (__cdecl *)(void *, char))vt[0])((void *)*piVar1, 1);
            }
            piVar1 = (int *)piVar1[2];
        } while (*(int **)(param_1 + 0x50) != piVar1);
    }
    *(DWORD *)(*(int *)(*(int *)(param_1 + 0x50) + 4) + 8) = 0;
    BYTE *puVar2 = *(BYTE **)(*(int *)(param_1 + 0x4c) + 8);
    while (BYTE *puVar3 = puVar2) {
        puVar2 = *(BYTE **)(puVar3 + 8);
        operator_delete(puVar3);
    }
    *(DWORD *)(*(int *)(param_1 + 0x4c) + 8) = *(DWORD *)(param_1 + 0x50);
    *(DWORD *)(*(int *)(param_1 + 0x50) + 4) = *(DWORD *)(param_1 + 0x4c);
    *(DWORD *)(param_1 + 0x48) = 0;
    operator_delete(*(BYTE **)(param_1 + 0x3c));
    if (*(DWORD *)(param_1 + 0x34) != 0) {
        // array de nodos: dtor de array (flags 3 = vector + free)
        FUN_00408680((void *)*(int *)(param_1 + 0x34), 3);
    }
    FUN_004080f0(param_1);
}

// FUN_00407fe0 @ 0x00407FE0 — Widget_CtorBase: allocate and cross-link two doubly-linked-list
// sentinel nodes, zero the count field, set vtable, then zero all widget node fields.
// List layout at param_1: [+0x4c]=head_sentinel*, [+0x50]=tail_sentinel*, [+0x48]=count.
// Sentinel node (0xc bytes): [+0]=unused, [+4]=prev, [+8]=next.
// Initially: head->+8 = tail; tail->+4 = head (empty list).
// Vtable de la tela (off_552520, leída del binario original):
//     { 0x0045AAA0, 0x00408780, 0x004089B0, 0x00408FF0 }
// El slot 0 (dtor) se llama directo en este build. Los slots 1 y 2 los invoca
// `sub_408CB0` en cada tick de simulación: 1 = re-fijar la fila superior al
// hueso (Cloth_PinTopRow), 2 = repartir viento/gravedad (Cloth_Wind).
void __cdecl FUN_00408780(int _this, float (*Matrix)[4]);
void __cdecl FUN_004089b0(DWORD *_this);
static void __cdecl Cloth_VSlot0(int a)  { (void)a; }  // dtor: se llama directo
static void *g_ClothVTable[4] = {
    (void *)Cloth_VSlot0,  (void *)FUN_00408780,
    (void *)FUN_004089b0,  (void *)FUN_00408ff0
};

void* __fastcall FUN_00407fe0(void *param_1)
{
    // head sentinel
    int *head = (int *)operator_new(0xc);
    if (head) { head[1] = 0; head[2] = 0; }
    ((int **)param_1)[0x13] = head;   // *(param_1 + 0x4c) = head

    // tail sentinel
    int *tail = (int *)operator_new(0xc);
    if (tail) { tail[1] = 0; tail[2] = 0; }
    ((int **)param_1)[0x14] = tail;   // *(param_1 + 0x50) = tail

    // cross-link: head->+8 = tail; tail->+4 = head
    if (head) head[2] = (int)tail;
    if (tail) tail[1] = (int)head;

    ((int *)param_1)[0x12] = 0;       // count at +0x48 = 0

    // 2026-08-11 — vtable. IDA hace `*(_DWORD *)this = &off_552520;` y varias
    // rutinas la usan por indirección; con el campo sin inicializar se ejecuta
    // basura (crash 0xC0000005 param0=8 al entrar al mundo, desde
    // `FUN_00449840`/DeleteCloth que llama vtable[0](3)).
    //
    // Leída del binario original (`Cliente armado/main.exe`, MD5 eb95ac…):
    //     off_552520 = { 0x0045AAA0, 0x00408780, 0x004089B0, 0x00408FF0 }
    // De esas cuatro sólo `sub_408FF0` está portada (y es un stub vacío); las
    // otras tres no existen en este build. Se instala una vtable bien formada
    // con no-ops para que el objeto sea válido y las indirecciones no salten a
    // basura. TODO: portar 0x0045AAA0 (dtor), 0x00408780, 0x004089B0 y el
    // cuerpo real de 0x00408FF0 (render de la tela).
    *(const void **)param_1 = (const void *)g_ClothVTable;

    FUN_004080f0((int)param_1);
    return param_1;
}

// FUN_00541ec1 @ 0x00541EC1 — Array_Construct(arr, elem_size, count, ctor): calls __fastcall ctor(elem) for each element.
void __cdecl FUN_00541ec1(void *arr, int elem_size, int count, void *ctor) {
    typedef void* (__fastcall *CtorFn)(void*);
    CtorFn fn = (CtorFn)ctor;
    char *p = (char*)arr;
    for (int i = 0; i < count; i++, p += elem_size)
        fn(p);
}
// Forward declarations — physics helpers defined in VerletNode section below
void  __fastcall FUN_00407950(void *node);   // ctor (thiscall en el original: arg en ECX)  // IDA-port: void return
void  __fastcall FUN_004079b0(void *node, float x, float y, float z, int pinned);
void  __fastcall FUN_00407b30(void *node, float *out);
float __fastcall FUN_00407b50(void *nodeA, int nodeB_ptr, float *out_delta);
void  __fastcall FUN_004088b0(void *sys, int idx, short na, short nb, float rest_scaled, float dist, BYTE flags);
DWORD* __cdecl   FUN_00407e50(DWORD *node); // IDA-port: returns this
void  __fastcall FUN_00407ef0(void *node, float p1, float p2, float p3, float radius, int boneIdx);


// FUN_00408900 @ 0x00408900 — Widget_CheckState(widget, hash, flags)
// __thiscall in original (this=widget via ECX). Calls FUN_00408940 `flags` times,
// returns 0 if any fails, 1 if all pass. FUN_00408940 is a void stub → always return 1.
// Port FIEL de IDA `sub_408900` (Hex-Rays perdió el `this`, que viaja en ECX):
//     v2 = 0;
//     if (a2 <= 0) return 1;
//     while (sub_408940(a1)) { if (++v2 >= a2) return 1; }
//     return 0;
// `hash` son los BITS del dt (0x3ba3d70a = 0.005f) y `flags` el nº de
// iteraciones. 2026-08-11: era un stub que devolvía 1 SIN ejecutar la
// simulación, así que los nodos de la tela nunca se movían.
int __cdecl FUN_00408940(int *param_1, float dt);
int __cdecl FUN_00408900(int *widget, unsigned int hash, int flags) {
    if (!widget || flags <= 0) return 1;
    float dt; memcpy(&dt, &hash, 4);
    int it = 0;
    while (FUN_00408940(widget, dt)) {
        if (++it >= flags) return 1;
    }
    return 0;
}
// FUN_00408130 @ 0x00408130 — GridSpring_Create: builds cols×rows cloth spring system.
// Params: widget=system, entity=scale_factor, p3=cols(W), p4=uRange, p5=vRange,
//         p6=rows(H), p7=entity_ptr, p8=uOrigin, p9=vOrigin, ta=bone_idx, tb=bone_data_idx, flags=flag_mask.
// Layout mirrors FUN_004093e0: +4..+44 store params, +30=node_count, +34=node_array*, +38=spring_count, +3c=spring_array*.
// Nodes initialized from bone transform if entity+0x114 != 0, else from grid coords.
// Springs: vertical edges (col-to-col+W), horizontal (col-to-col+1), and diagonal shear springs.
void __cdecl FUN_00408130(void *widget, float entity, int p3, float p4, float p5,
                          int p6, int p7, float p8, float p9, int ta, int tb, int flags)
{
    char *thiz = (char*)widget;
    *(float *)(thiz + 0x04) = entity;
    *(int   *)(thiz + 0x10) = tb;
    // 2026-08-11 FIX: los tres campos de abajo estaban CORRIDOS un parámetro.
    // IDA `sub_408130` L78-89:
    //     this[6] (+0x18) = a4        ← nuestro port ponía p3
    //     this[7] (+0x1C) = a5        ← ponía p4
    //     this[8] (+0x20) = a8        ← ponía p5
    //     this[9] (+0x24) = a9        ← ok
    // El de +0x20 es el ANCHO de la grilla: con p5 (0.0 en la llamada de la
    // capa) todos los nodos quedaban en la misma columna → los 100 vértices
    // salían en (0,0,0) (medido con el probe CLOTHDBG).
    *(float *)(thiz + 0x18) = *(float*)&p4;   // a4
    *(float *)(thiz + 0x1c) = *(float*)&p5;   // a5
    // 2026-08-11 FIX: IDA `sub_408130` L80-90 guarda
    //     this[10] (+0x28) = a6   ← 6º param
    //     this[11] (+0x2c) = a7   ← 7º param  (nuestro port ponía p3, el 3º)
    //     this[12] (+0x30) = a7 * a6
    // Con p3 en +0x2c la grilla quedaba de 19 filas sobre 10x10 → el render
    // (sub_408FF0 / sub_4091D0, que leen +0x28 y +0x2c) se iba de rango.
    *(int   *)(thiz + 0x28) = p6;             // cols
    int node_count = p6 * p7;
    *(int   *)(thiz + 0x08) = p3;
    *(int   *)(thiz + 0x2c) = p7;             // rows
    *(int   *)(thiz + 0x0c) = ta;
    *(float *)(thiz + 0x20) = p8;             // a8 — ancho de la grilla
    *(unsigned int *)(thiz + 0x14) |= (unsigned int)flags;
    *(float *)(thiz + 0x24) = *(float*)&p9;   // vOrigin

    *(int *)(thiz + 0x30) = node_count;

    // allocate node array (same pattern as FUN_004093e0)
    int *raw = (int*)operator_new(node_count * 0x3c + 4);
    int *nodes;
    if (raw == NULL) {
        nodes = NULL;
    } else {
        nodes = raw + 1;
        *raw  = node_count;
        FUN_00541ec1(nodes, 0x3c, node_count, (void*)FUN_00407950);
    }
    *(int **)(thiz + 0x34) = nodes;

    // spring count: (rows-1)*cols + (cols-1)*rows horizontal+vertical connections (×2)
    int sc = (p3 - 1) * p6 + (p3 - 1) * p3;  // per Ghidra: (cols-1)*rows + (cols-1)*cols — approximate
    {
        int W = *(int *)(thiz + 0x28);   // rows stored at +28
        int H = *(int *)(thiz + 0x2c);   // cols stored at +2c
        sc = (H - 1) * W + (W - 1) * H;
    }
    *(int *)(thiz + 0x38) = sc * 2;

    void *springs = operator_new(sc * 0x20);
    *(void **)(thiz + 0x3c) = springs;

    // cell spacing
    int W = *(int *)(thiz + 0x28);   // rows
    int H = *(int *)(thiz + 0x2c);   // cols
    float cellU = *(float *)(thiz + 0x20) / (float)(W - 1);
    float cellV = *(float *)(thiz + 0x24) / (float)(H - 1);
    *(float *)(thiz + 0x40) = cellU;
    *(float *)(thiz + 0x44) = cellV;

    // compute bone rotation matrix from entity+0x1c
    // 2026-08-11 FIX (la capa salía con todos los vértices en (0,0,0)):
    // `Matrix_BuildFromEuler` es EulerToMatrix3x4 — escribe una matriz 3x4 = **12
    // floats**. Estaba declarado `float local_3c[3]` → 36 bytes de desborde de
    // stack justo encima de los locales siguientes. El probe INITDBG lo mostró
    // sin lugar a dudas: después del loop de init, `nodes` había pasado a NULL,
    // `W` a 0 y `H` a 1063105495 (= bits de 0.85f, o sea un elemento de la
    // matriz). Con `W`/`H` pisados el loop no escribía ningún nodo.
    // Mismo patrón que el `local_3c[14]` de Effect_Create (ver la nota de
    // "Locales que Ghidra separó" en CLAUDE.md): el callee escribe más de lo
    // que declara el local.
    float local_3c[12];
    Matrix_BuildFromEuler((float *)(*(int *)(thiz + 4) + 0x1c), local_3c);

    // init node positions
    int entity_ptr = *(int *)(thiz + 4);
    int has_bone = *(int *)(entity_ptr + 0x114);

    for (int row = 0; row < H; row++) {
        float scaleU = cellU;
        float uOrig  = *(float *)(thiz + 0x20);

        // flag-based UV scale (from Ghidra conditional at +0x14 bits 2-3)
        if ((*(unsigned int *)(thiz + 0x14) & 0xc) == 4) {
            float t = ((float)row * _DAT_00552538) / (float)(H - 1) + _DAT_00552534;
            uOrig  = t * *(float *)(thiz + 0x20);
            scaleU = uOrig / (float)(W - 1);
        }

        float vOffset = -((float)row * cellV);
        float halfU   = uOrig * _DAT_00552504;

        for (int col = 0; col < W; col++) {
            float lx = (float)col * scaleU - halfU;
            float ly = 20.0f;
            if ((*(unsigned int *)(thiz + 0x14) & 3) == 1) {
                float frac = fabsf((float)col / (float)(W - 1) - _DAT_00552504);
                frac += frac;
                ly = _DAT_005524fc - frac * frac * _DAT_00552488;
            }
            float lz = vOffset;

            float nx, ny, nz;
            if (has_bone == 0) {
                nx = lx; ny = ly; nz = lz;
            } else {
                float out_pos[3] = {lx, ly, lz};
                float out_col[4] = {0};
                BMD_TransformPosition((void *)(DAT_05828d58 + *(short *)(entity_ptr + 2) * 0xbc),
                             local_3c, out_pos, out_col, '\x01');
                // 2026-08-11 FIX: BMD__TransformPosition LEE del 3er arg (Pos)
                // y ESCRIBE en el 4º (WorldPos). El port leía de vuelta
                // `out_pos` — la ENTRADA sin transformar — y descartaba el
                // resultado, así que la malla quedaba en espacio LOCAL
                // (bbox -37.5..37.5 x 0..-120, medido con CLOTHDBG) en lugar de
                // en la posición del personaje.
                nx = out_col[0]; ny = out_col[1]; nz = out_col[2];
            }

            // IDA `v31 = v30 + i * v29` con v29 = this[10] = W (columnas).
            // 2026-08-11: era `H * row + col`. Coincide sólo cuando W == H
            // (la capa del MG es 10x10); los demás cloths quedaban barajados.
            int ni = W * row + col;
            FUN_004079b0((char*)nodes + ni * 0x3c, nx, ny, nz, 0);
        }
    }

    // ── INITDBG (temporal): ¿el loop de init escribió las posiciones?
    // Lee de vuelta el nodo 0 y el 50 en +0x1c, que es donde FUN_004079b0
    // escribe y FUN_00407b30 lee. Si acá salen no-cero pero CLOTHDBG ve (0,0,0),
    // el problema está en la lectura/puntero; si acá ya salen cero, en la
    // escritura.
    {
        char ib[240];
        if (nodes && node_count > 0) {
            const float *n0 = (const float *)((char *)nodes + 0x1c);
            int last = (node_count > 1) ? (node_count - 1) : 0;
            const float *nL = (const float *)((char *)nodes + (size_t)last * 0x3c + 0x1c);
            _snprintf_s(ib, sizeof(ib), _TRUNCATE,
                "INITDBG nodes=%p cnt=%d W=%d H=%d uRange=%.1f vRange=%.1f hasBone=%d "
                "n0=(%.1f,%.1f,%.1f) nLast=(%.1f,%.1f,%.1f)",
                (void *)nodes, node_count, W, H,
                *(float *)(thiz + 0x20), *(float *)(thiz + 0x24), has_bone,
                n0[0], n0[1], n0[2], nL[0], nL[1], nL[2]);
        } else {
            _snprintf_s(ib, sizeof(ib), _TRUNCATE,
                "INITDBG nodes=%p cnt=%d W=%d H=%d  (SIN ARRAY)",
                (void *)nodes, node_count, W, H);
        }
        DbgLogPublic(ib);
    }

    // build spring edges
    // IDA: `if ((this[5] & 0x300) != 256) { v58 = 4; v63 = 1; }`
    //   v58 -> springs VERTICALES (bit 4 = solver de rango, sub_407B90)
    //   v63 -> springs DIAGONALES (bit 1 = solver de igualdad, sub_407C60)
    // 2026-08-11: las diagonales recibían `vert_flag` (4) en vez de v63 (1),
    // o sea entraban al solver de rango y nunca al de igualdad.
    BYTE vert_flag = (((*(unsigned int *)(thiz + 0x14) & 0x300) != 0x100)) ? 4 : 0;
    BYTE diag_flag = (((*(unsigned int *)(thiz + 0x14) & 0x300) != 0x100)) ? 1 : 0;
    int sp = 0;

    for (int row = 0; row < H; row++) {
        for (int col = 0; col < W; col++) {
            int ni = W * row + col;
            short sni = (short)ni;
            float delta[3];
            float dist;

            // vertical spring (row → row+1, same col)
            if (row < H - 1) {
                dist = FUN_00407b50((char*)nodes + ni * 0x3c,
                                   (int)nodes + (W + ni) * 0x3c, delta);
                FUN_004088b0(widget, sp, sni, sni + (short)W,
                             dist * _DAT_00552530, dist, (BYTE)(vert_flag | 2));
                sp++;
            }

            // horizontal spring (col → col+1, same row)
            if (col < W - 1) {
                dist = FUN_00407b50((char*)nodes + ni * 0x3c,
                                   (int)nodes + (ni + 1) * 0x3c, delta);
                FUN_004088b0(widget, sp, sni, sni + 1,
                             dist * _DAT_00552530, dist, 3);
                sp++;

                // diagonal shear spring down-right (to row+1, col+1)
                if (row < H - 1) {
                    dist = FUN_00407b50((char*)nodes + ni * 0x3c,
                                       (int)nodes + (ni + 1 + W) * 0x3c, delta);
                    FUN_004088b0(widget, sp, sni, sni + 1 + (short)W,
                                 dist * _DAT_00552530, dist, diag_flag);
                    sp++;
                }

                // diagonal shear spring up-right (to row-1, col+1)
                if (row > 1) {
                    dist = FUN_00407b50((char*)nodes + ni * 0x3c,
                                       (int)nodes + (ni - W + 1) * 0x3c, delta);
                    FUN_004088b0(widget, sp, sni, (short)(sni - (short)W + 1),
                                 dist * _DAT_00552530, dist, diag_flag);
                    sp++;
                }
            }
        }
    }
    *(int *)(thiz + 0x38) = sp;
    // vtable call: (*vtable[1])(local_3c) — skipped in re-impl
}

// Skills / teleport
