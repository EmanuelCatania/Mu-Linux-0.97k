// UI_DialogInterface.cpp
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
extern "C" DWORD g_ItemAttribute_Backup;
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


// FUN_0051d780 @ 0x0051D780 — CreateDialogInterface(textId, flag)
// Ported from IDA: builds a custom MessageBox-style dialog from GlobalText[textId].
//   - Wraps text into up to 7 lines × 38 chars via SeparateTextIntoLines
//   - Stores a 0x14-byte (5-int) button rect descriptor at DAT_083a42f8
//     {1, 71, 140, 70, 21} = (id=1, x=71, y=140, w=70, h=21) for the OK button
//   - Sets ErrorMessage=141 (or NextErrorMessage if one is already showing)
//
// Globals used:
//   DAT_083a7c04 = dialog text id (a1)
//   DAT_083a7c09 = flag byte (a2)
//   DAT_083a7c08 = state (cleared to 0)
//   ErrorMessage  = DAT_083a7c24 (via SetErrorMessage helper)
//   NextErrorMessage = DAT_083a7c28
void __cdecl FUN_0051d780(int a1, int a2)
{
    DAT_083a7c04 = (DWORD)a1;
    DAT_083a7c09 = (char)a2;
    DAT_083a7c08 = 0;

    g_iNumLineMessageBoxCustom = SeparateTextIntoLines(GlobalText[a1],
        g_lpszMessageBoxCustom[0], 7, 38);

    // Clear 0x28 (40 bytes = 10 ints) starting at DAT_083a42f8, then write 5 ints
    memset((void*)&DAT_083a42f8[0], 0, 0x28u);
    DAT_083a42f8[0] = 1;
    DAT_083a42f8[1] = 71;
    DAT_083a42f8[2] = 140;
    DAT_083a42f8[3] = 70;
    DAT_083a42f8[4] = 21;

    if (DAT_083a7c24 != 0) {
        DAT_083a7c28 = 141;
    } else {
        DAT_083a7c24 = 141;
    }
}
