// Item_LegacyHelpers.cpp
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


// ── New helpers needed by SecondPassword implementations ─────────────────────

// GetItemCount @ 0x00482FF0 — count of inventory items of given type+level
// IDA-ported: walks the 8×8 inventory grid (base DAT_07EA9328..DAT_07EA9504,
// 17 ints per row, 136 ints per column) and counts matches.
// iType == -1: match any; iLevel == -1: match any.
int __cdecl GetItemCount(int siType, int iLevel) {
    int result = 0;
    int *v3 = (int *)&DAT_07ea9504;
    do {
        int *v4 = v3;
        int v5 = 8;
        do {
            if ( *((short *)v4 - 28) == siType
              && (siType == -1 || *v4 > 0)
              && (iLevel == -1 || ((*(v4 - 13) >> 3) & 0xF) == iLevel) )
            {
                ++result;
            }
            v4 -= 136;
            --v5;
        } while ( v5 );
        v3 -= 17;
    } while ( (int)v3 >= (int)&DAT_07ea9328 );
    return result;
}

// GetItemSlot @ 0x00482D70 — get inventory slot index for given type+level
// IDA-ported: same 8×8 inventory grid walk as GetItemCount, returns first
// matching slot index (0..63) or -1. iLevel == -1 matches any level.
int __cdecl GetItemSlot(int siType, int iLevel) {
    int v2 = 7;
    int *v3 = (int *)&DAT_07ea9504;
LABEL_2: {
        int v4 = 7;
        int result = v2 + 56;
        int *v6 = v3;
        while ( *((short *)v6 - 28) != siType || *v6 <= 0
                || (iLevel != -1 && ((*(v6 - 13) >> 3) & 0xF) != iLevel) )
        {
            --v4;
            v6 -= 136;
            result -= 8;
            if ( v4 < 0 )
            {
                v3 -= 17;
                --v2;
                if ( (int)v3 >= (int)&DAT_07ea9328 )
                    goto LABEL_2;
                return -1;
            }
        }
        return result;
    }
}
