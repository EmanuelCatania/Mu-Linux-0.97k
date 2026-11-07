// Path_LegacyReset.cpp

#include "stdafx.h"
#include "Game/PathContext.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// Definidas en Physics/Cloth_Simulation.cpp (Cloth_Integrate) y Net/Crypto.cpp (Cloth_Solve).
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


// IDA: FUN_0043f2d0 @ 0x0043F2D0 — Pathfinder_Reset: frees + re-initialises A* context at DAT_05826df4.
// Despite the "Font_Reset" comment in functions.h, this is clearly the A* grid init.
void __cdecl PathFinder_ResetContext(void) {
    PathContext *ctx = DAT_05826df4;
    if (ctx->Flags   != nullptr) { operator_delete(ctx->Flags);   ctx->Flags = nullptr; }
    if (ctx->Cost    != nullptr) { operator_delete(ctx->Cost);    ctx->Cost = nullptr; }
    if (ctx->ParentX != nullptr) { operator_delete(ctx->ParentX); ctx->ParentX = nullptr; }
    if (ctx->ParentY != nullptr) { operator_delete(ctx->ParentY); ctx->ParentY = nullptr; }
    ctx->DirtyMin = 1950000000;
    ctx->DirtyMax = -1;
    ctx->Width    = 0x100;
    ctx->Height   = 0x100;
    ctx->Walk     = reinterpret_cast<unsigned char *>(&DAT_0838bc70);
    ctx->Size     = 0x10000;
    ctx->Flags    = static_cast<unsigned char *>(operator_new(0x10000));
    ctx->Cost     = static_cast<int *>(operator_new(ctx->Size * sizeof(int)));
    ctx->ParentX  = static_cast<int *>(operator_new(ctx->Size * sizeof(int)));
    ctx->ParentY  = static_cast<int *>(operator_new(ctx->Size * sizeof(int)));
    memset(ctx->Flags, 0, ctx->Size);
}
