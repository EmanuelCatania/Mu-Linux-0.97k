// UI_LegacyWidgetSystem.cpp

#include "stdafx.h"
#include "Physics/ClothTypes.h"
#include "Entity/ClothPresentation.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones definidas en otros modulos ------------------
// Cloth_Integrate esta en Physics/Cloth_Simulation.cpp y Cloth_Solve en
// Net/Crypto.cpp.
void __fastcall Cloth_Integrate(int*, float);
int  __cdecl    Cloth_Solve(DWORD *a1);

#include "Net/Net.h"

extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl operator_delete(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) operator_delete((unsigned char*)(p))
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
// Widget_Release @ 0x004086E0 — Widget_Release(widget_ptr) [__fastcall]
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
//   8. Widget_NodeInit(param1)  (base widget destructor — not declared)
// Widget_NodeInit @ 0x004080F0 — Widget_NodeInit
// Zeros all node fields and sets the four scale floats to 1.0f.
void __cdecl Widget_NodeInit(void* widget)
{
    ClothPresentation_Forget(widget);
    ClothWidget& w = ClothSystem(widget);
    w.Owner = nullptr;
    w.Bone = w.FrontTexture = w.BackTexture = 0;
    w.Flags = 0;
    w.OriginU = w.OriginV = 0;
    w.Width = w.Height = 1;
    w.Columns = w.Rows = w.NodeCount = 0;
    w.Nodes = nullptr;
    w.SpringCount = 0;
    w.Springs = nullptr;
    w.StepU = w.StepV = 1;
}

// Widget_Release @ 0x004086E0 — Widget_Release(widget, 0, 0)
// Iterates linked list at (*(widget+0x4c)+8), calls vtable[0](1) on each node,
// frees list nodes, relinks head/tail sentinels, zeroes count, frees buffer at
// widget+0x3c, calls vtable[0](3) on widget+0x34, then calls Widget_BaseRelease.
void __cdecl Widget_Release(void* param_1, int, int) {
    ClothWidget& w = ClothSystem(param_1);
    ClothLink* entry = w.Head->Next;
    if (entry != w.Tail) {
        do {
            if (!entry) break;
            if (entry->Value) {
                void** vt = entry->Value->VTable;
                if (vt) reinterpret_cast<void (__cdecl *)(void*,char)>(vt[0])(entry->Value,1);
            }
            entry = entry->Next;
        } while (w.Tail != entry);
    }
    w.Tail->Previous->Next = nullptr;
    entry = w.Head->Next;
    while (entry) {
        ClothLink* next = entry->Next;
        operator_delete(entry);
        entry = next;
    }
    w.Head->Next = w.Tail;
    w.Tail->Previous = w.Head;
    w.AnchorCount = 0;
    operator_delete(w.Springs);
    if (w.Nodes) FUN_00408680(w.Nodes,3);
    Widget_NodeInit(param_1);
}

// IDA: Widget_CtorBase (0x00407FE0)
// Widget_CtorBase: allocate and cross-link two doubly-linked-list
// sentinel nodes, zero the count field, set vtable, then zero all widget node fields.
// List layout at param_1: [+0x4c]=head_sentinel*, [+0x50]=tail_sentinel*, [+0x48]=count.
// Sentinel node (0xc bytes): [+0]=unused, [+4]=prev, [+8]=next.
// Initially: head->+8 = tail; tail->+4 = head (empty list).
// Vtable de la tela (off_552520, leída del binario original):
//     { 0x0045AAA0, 0x00408780, 0x004089B0, 0x00408FF0 }
// El slot 0 (dtor) se llama directo en este build. Los slots 1 y 2 los invoca
// `sub_408CB0` en cada tick de simulación: 1 = re-fijar la fila superior al
// hueso (Cloth_PinTopRow), 2 = repartir viento/gravedad (Cloth_Wind).
void __cdecl Cloth_PinTopRow(void* widget, float (*Matrix)[4]);
void __cdecl Cloth_Wind(void *_this);
static void __cdecl Cloth_VSlot0(int a)  { (void)a; }  // dtor: se llama directo
static void *g_ClothVTable[4] = {
    (void *)Cloth_VSlot0,  (void *)Cloth_PinTopRow,
    (void *)Cloth_Wind,  (void *)FUN_00408ff0
};

void* __fastcall Widget_CtorBase(void *param_1)
{
    ClothWidget& w = ClothSystem(param_1);
    auto* head = static_cast<ClothLink*>(operator_new(sizeof(ClothLink)));
    if (head) { head->Previous = nullptr; head->Next = nullptr; }
    w.Head = head;
    auto* tail = static_cast<ClothLink*>(operator_new(sizeof(ClothLink)));
    if (tail) { tail->Previous = nullptr; tail->Next = nullptr; }
    w.Tail = tail;
    if (head) head->Next = tail;
    if (tail) tail->Previous = head;
    w.AnchorCount = 0;
    w.VTable = g_ClothVTable;
    Widget_NodeInit(param_1);
    return param_1;
}

// L_YGXPAXIHP6EX0_Z1_Z @ 0x00541EC1 — Array_Construct(arr, elem_size, count, ctor): calls __fastcall ctor(elem) for each element.
void __cdecl L_YGXPAXIHP6EX0_Z1_Z(void *arr, int elem_size, int count, void *ctor) {
    typedef void* (__fastcall *CtorFn)(void*);
    CtorFn fn = (CtorFn)ctor;
    char *p = (char*)arr;
    for (int i = 0; i < count; i++, p += elem_size)
        fn(p);
}
// Forward declarations — physics helpers defined in VerletNode section below
void  __fastcall SpringNode_Ctor(void *node);   // ctor (thiscall en el original: arg en ECX)  // IDA-port: void return
void  __fastcall SpringNode_SetPos(void *node, float x, float y, float z, int pinned);
void  __fastcall SpringNode_GetPos(void *node, float *out);
float __fastcall SpringNode_Delta(void *nodeA, uintptr_t nodeB_ptr, float *out_delta);
void  __fastcall Spring_StoreEdge(void *sys, int idx, short na, short nb, float rest_scaled, float dist, BYTE flags);
DWORD* __cdecl   ClothAnchor_Ctor(DWORD *node); // IDA-port: returns this
void  __fastcall ClothAnchor_SetParams(void *node, float p1, float p2, float p3, float radius, int boneIdx);


// IDA: Widget_CheckState (0x00408900)
// Widget_CheckState(widget, hash, flags)
// __thiscall in original (this=widget via ECX). Calls Sound_UpdateChannel3D_Tick
// (sub_408940) `flags` times; returns 0 if any fails, 1 if all pass.
// Port FIEL de IDA `sub_408900` (Hex-Rays perdió el `this`, que viaja en ECX):
//     v2 = 0;
//     if (a2 <= 0) return 1;
//     while (sub_408940(a1)) { if (++v2 >= a2) return 1; }
//     return 0;
// `hash` son los BITS del dt (0x3ba3d70a = 0.005f) y `flags` el nº de
// iteraciones.
int __cdecl Sound_UpdateChannel3D_Tick(int *param_1, float dt);
int __cdecl Widget_CheckState(void *widget, unsigned int hash, int flags) {
    if (!widget || flags <= 0) return 1;
    float dt; memcpy(&dt, &hash, 4);
    ClothPresentation_Before(widget);
    int it = 0;
    while (Sound_UpdateChannel3D_Tick(static_cast<int*>(widget), dt)) {
        if (++it >= flags) { ClothPresentation_After(widget); return 1; }
    }
    ClothPresentation_Forget(widget);
    return 0;
}
// GridSpring_Create @ 0x00408130 — GridSpring_Create: builds cols×rows cloth spring system.
// Params: widget=system, entity=owner, p3=source bone index, p4/uRange,
//         p5/vRange, p6=grid columns, p7=grid rows, p8/uRange2,
//         p9/vOrigin, ta/type, tb=bone-data index, flags=flag_mask.
// Layout mirrors SpringMesh_Create: +4..+44 store params, +30=node_count, +34=node_array*, +38=spring_count, +3c=spring_array*.
// Nodes initialized from bone transform if entity+0x114 != 0, else from grid coords.
// Springs: vertical edges (col-to-col+W), horizontal (col-to-col+1), and diagonal shear springs.
void __cdecl GridSpring_Create(void *widget, OBJECT* entity, int p3, float p4, float p5,
                          int p6, int p7, float p8, float p9, int ta, int tb, int flags)
{
    ClothPresentation_Forget(widget);
    char *thiz = (char*)widget;
    ClothSystem(thiz).Owner = entity;
    ClothSystem(thiz).BackTexture = tb;
    // IDA `sub_408130` L78-89:
    //     this[6] (+0x18) = a4
    //     this[7] (+0x1C) = a5
    //     this[8] (+0x20) = a8   (ANCHO de la grilla)
    //     this[9] (+0x24) = a9
    ClothSystem(thiz).OriginU = *(float*)&p4;   // a4
    ClothSystem(thiz).OriginV = *(float*)&p5;   // a5
    // IDA `sub_408130` L80-90 guarda
    //     this[10] (+0x28) = a6   ← 6º param
    //     this[11] (+0x2c) = a7   ← 7º param
    //     this[12] (+0x30) = a7 * a6
    // El render (sub_408FF0 / sub_4091D0) lee +0x28 y +0x2c.
    ClothSystem(thiz).Columns = p6;             // cols
    int node_count = p6 * p7;
    ClothSystem(thiz).Bone = p3;
    ClothSystem(thiz).Rows = p7;             // rows
    ClothSystem(thiz).FrontTexture = ta;
    ClothSystem(thiz).Width = p8;             // a8 — ancho de la grilla
    ClothSystem(thiz).Flags |= (unsigned int)flags;
    ClothSystem(thiz).Height = *(float*)&p9;   // vOrigin

    ClothSystem(thiz).NodeCount = node_count;

    // allocate node array (same pattern as SpringMesh_Create)
    int *raw = (int*)operator_new(node_count * sizeof(ClothNode) + 4);
    ClothNode *nodes;
    if (raw == NULL) {
        nodes = NULL;
    } else {
        nodes = reinterpret_cast<ClothNode*>(raw + 1);
        *raw  = node_count;
        ClothConstruct(nodes, node_count, SpringNode_Ctor);
    }
    ClothSystem(thiz).Nodes = nodes;

    // spring count: (rows-1)*cols + (cols-1)*rows horizontal+vertical connections (×2)
    int sc = (p3 - 1) * p6 + (p3 - 1) * p3;  // per Ghidra: (cols-1)*rows + (cols-1)*cols — approximate
    {
        int W = ClothSystem(thiz).Columns;   // rows stored at +28
        int H = ClothSystem(thiz).Rows;   // cols stored at +2c
        sc = (H - 1) * W + (W - 1) * H;
    }
    ClothSystem(thiz).SpringCount = sc * 2;

    void *springs = operator_new(sc * 0x20);
    ClothSystem(thiz).Springs = static_cast<ClothSpring*>(springs);

    // cell spacing
    int W = ClothSystem(thiz).Columns;   // rows
    int H = ClothSystem(thiz).Rows;   // cols
    float cellU = ClothSystem(thiz).Width / (float)(W - 1);
    float cellV = ClothSystem(thiz).Height / (float)(H - 1);
    ClothSystem(thiz).StepU = cellU;
    ClothSystem(thiz).StepV = cellV;

    // IDA `sub_408130` first builds the orientation from the owner's angles,
    // then replaces only matrix[3], matrix[7] and matrix[11] with the source
    // bone translation.  Copying the full bone matrix changes the grid's
    // rotation basis and makes broad cloth surfaces shear/stretch.
    float local_3c[12] = {};
    OBJECT* entity_ptr = ClothSystem(thiz).Owner;
    AngleMatrix(entity_ptr->Angle, (float (*)[4])local_3c);
    const auto* boneBase = entity_ptr->BoneMatrices;
    if (boneBase) {
        const float *bone = reinterpret_cast<const float*>(boneBase + p3);
        local_3c[3]  = bone[3];
        local_3c[7]  = bone[7];
        local_3c[11] = bone[11];
    }

    // init node positions
    const auto* has_bone = entity_ptr->BoneMatrices;

    for (int row = 0; row < H; row++) {
        float scaleU = cellU;
        float uOrig  = ClothSystem(thiz).Width;

        // flag-based UV scale (from Ghidra conditional at +0x14 bits 2-3)
        if ((ClothSystem(thiz).Flags & 0xc) == 4) {
            float t = ((float)row * _DAT_00552538) / (float)(H - 1) + _DAT_00552534;
            uOrig  = t * ClothSystem(thiz).Width;
            scaleU = uOrig / (float)(W - 1);
        }

        float vOffset = -((float)row * cellV);
        float halfU   = uOrig * _DAT_00552504;

        for (int col = 0; col < W; col++) {
            float lx = (float)col * scaleU - halfU;
            float ly = 20.0f;
            if ((ClothSystem(thiz).Flags & 3) == 1) {
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
                BMD_TransformPosition(static_cast<void*>(AsModel(DAT_05828d58) + entity_ptr->Type),
                             local_3c, out_pos, out_col, '\x01');
                // BMD__TransformPosition LEE del 3er arg (Pos) y ESCRIBE en el 4º
                // (WorldPos): el resultado transformado es out_col, no `out_pos`.
                nx = out_col[0]; ny = out_col[1]; nz = out_col[2];
            }

            // IDA `v31 = v30 + i * v29` con v29 = this[10] = W (columnas), no H.
            int ni = W * row + col;
            SpringNode_SetPos(nodes + ni, nx, ny, nz, 0);
        }
    }

    // build spring edges
    // IDA: `if ((this[5] & 0x300) != 256) { v58 = 4; v63 = 1; }`
    //   v58 -> springs VERTICALES (bit 4 = solver de rango, sub_407B90)
    //   v63 -> springs DIAGONALES (bit 1 = solver de igualdad, sub_407C60)
    BYTE vert_flag = (((ClothSystem(thiz).Flags & 0x300) != 0x100)) ? 4 : 0;
    BYTE diag_flag = (((ClothSystem(thiz).Flags & 0x300) != 0x100)) ? 1 : 0;
    int sp = 0;

    for (int row = 0; row < H; row++) {
        for (int col = 0; col < W; col++) {
            int ni = W * row + col;
            short sni = (short)ni;
            float delta[3];
            float dist;

            // vertical spring (row → row+1, same col)
            if (row < H - 1) {
                dist = SpringNode_Delta(nodes + ni,
                                   reinterpret_cast<uintptr_t>(nodes) + (W + ni) * sizeof(ClothNode), delta);
                Spring_StoreEdge(widget, sp, sni, sni + (short)W,
                             dist * _DAT_00552530, dist, (BYTE)(vert_flag | 2));
                sp++;
            }

            // horizontal spring (col → col+1, same row)
            if (col < W - 1) {
                dist = SpringNode_Delta(nodes + ni,
                                   reinterpret_cast<uintptr_t>(nodes) + (ni + 1) * sizeof(ClothNode), delta);
                Spring_StoreEdge(widget, sp, sni, sni + 1,
                             dist * _DAT_00552530, dist, 3);
                sp++;

                // diagonal shear spring down-right (to row+1, col+1)
                if (row < H - 1) {
                    dist = SpringNode_Delta(nodes + ni,
                                       reinterpret_cast<uintptr_t>(nodes) + (ni + 1 + W) * sizeof(ClothNode), delta);
                    Spring_StoreEdge(widget, sp, sni, sni + 1 + (short)W,
                                 dist * _DAT_00552530, dist, diag_flag);
                    sp++;
                }

                // diagonal shear spring up-right (to row-1, col+1)
                if (row > 1) {
                    dist = SpringNode_Delta(nodes + ni,
                                       reinterpret_cast<uintptr_t>(nodes) + (ni - W + 1) * sizeof(ClothNode), delta);
                    Spring_StoreEdge(widget, sp, sni, (short)(sni - (short)W + 1),
                                 dist * _DAT_00552530, dist, diag_flag);
                    sp++;
                }
            }
        }
    }
    ClothSystem(thiz).SpringCount = sp;
    // IDA invokes vtable slot 1 here to pin the top row immediately to the
    // source bone.  The same callback is used by the simulation on later ticks.
    Cloth_PinTopRow(widget, (float (*)[4])local_3c);
}

// Skills / teleport
