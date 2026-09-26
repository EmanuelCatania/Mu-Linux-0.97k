// Pipe_Legacy.cpp
//
// Extracted from stubs_helpers.cpp; original IDA comments and DAT_* provenance retained.

// stubs_helpers.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 12638-13754 (1117 lines).
//
// Originally tagged "New helpers needed by SecondPassword implementations" but
// content is mixed: item/inventory helpers (GetItemCount/GetItemSlot/
// CalcMaxDurability/ConvertItemType/ItemValue/ConvertGold), render helpers
// (CreateOkMessageBox/BMD::Animation/RenderObjectScreen), math helpers
// (VectorMA/VectorNormalize/RandomXY), effect helpers (SpawnEffectAtBone/
// JointBetweenBones), Pipe helpers (Pipe_Send/Recv/SetTarget), CSQuest helpers.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl operator_delete(void* ptr);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) operator_delete((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

// IDA Hex-Rays intrinsic shims.
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


// Pipe_WriteFile @ 0x005403a0 — Pipe_WriteFile
// Writes a formatted message to the named pipe handle (lpTargetHandle_00563b58).
void __cdecl Pipe_WriteFile(LPCVOID param_1, int param_2, CHAR *param_3)
{
    if (lpTargetHandle_00563b58 == NULL || lpTargetHandle_00563b58 == INVALID_HANDLE_VALUE)
        return;
    int len = crt_sprintf(lpBuffer_083bbb60, "%p:%d:%s", param_1, param_2,
                          param_3 ? param_3 : "");
    if (len <= 0) return;
    DWORD dwWritten = 0;
    WriteFile(lpTargetHandle_00563b58, lpBuffer_083bbb60, (DWORD)(len + 1), &dwWritten, NULL);
    DAT_083bbb64 = dwWritten;
}


// Pipe_Write @ 0x005404a0 — Pipe_Write (thin wrapper)
void __cdecl Pipe_Write(LPCVOID param_1, int param_2, CHAR *param_3)
{
    Pipe_WriteFile(param_1, param_2, param_3);
}


// Pipe_SetTarget @ 0x0053ed30 — Pipe_SetTarget
// Copies name into manager+0x2f0, then sends opcode 0x613 via Pipe_Write.
void __cdecl Pipe_SetTarget(void *mgr, CHAR *name)
{
    if (mgr == NULL || name == NULL) return;
    char *dst = (char *)mgr + 0x2f0;
    int len = (int)strlen(name);
    int words = len / 2;
    for (int i = 0; i < words; i++)
        ((short *)dst)[i] = ((short *)name)[i];
    if (len & 1)
        dst[len - 1] = name[len - 1];
    Pipe_Write((LPCVOID)0x8b1, 0x613, name);
}
