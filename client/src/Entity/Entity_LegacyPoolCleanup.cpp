// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"
#include "Party/Party.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);
extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

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


// FUN_0045ac20 @ 0x0045AC20 — DeleteCharacter(int Key)  (95 bytes)
// Linear-scans CharactersClient (400 slots × 0x394) for entity_id (+0x1DC).
// On match, marks slot inactive (+0 = 0), kills any butterfly that owned this
// character (Butterfles[+0xFC] == slot_ptr → +0 = 0), then calls DeleteCloth.
extern void __cdecl FUN_00449840(int c, int o, int flag);  // DeleteCloth
extern "C" void __cdecl DeleteCharacter(int Key)
{
    DWORD v1 = (DWORD)DAT_07abf5d0;
    int v2 = 0;
    while (!*(BYTE*)v1 || *(short*)(v1 + 0x1DC) != (short)Key) {
        ++v2;
        v1 += 916;  // 0x394
        if (v2 >= 400) return;
    }
    *(BYTE*)v1 = 0;

    // Walk Butterfles pool: each entry 0x1BC bytes (= 111 dwords = 444b),
    // entry layout has Owner at +0xFC (= dword[63]).
    DWORD* v3 = (DWORD*)&DAT_083a1218;
    DWORD* end = (DWORD*)((char*)&DAT_083a1218 + 0x1158);
    while (v3 < end) {
        if (*(BYTE*)v3 != 0 && v3[63] == v1) {
            *(BYTE*)v3 = 0;
        }
        v3 += 111;  // stride 444 bytes = 0x1BC
    }

    FUN_00449840((int)v1, (int)v1, 0);

    // The reconstructed Party table is separate storage, so re-arm the
    // original runtime sentinels at the entity-destruction boundary instead
    // of retaining a destroyed/reused CharactersClient index.
    Party_RefreshViewportLinks();
}
void __cdecl FUN_0045ac20(int Key) { DeleteCharacter(Key); }

// FUN_00460d20 @ 0x00460D20 — DeleteEffect(int Type, DWORD Owner, int iSubType)
// (69 bytes) — walks Effects pool DAT_07b11670 (124 × 0x1BC), zeroes any slot
// whose Type/Owner/SubType match. Active flag at slot+0; Type at slot+2 (short);
// SubType at slot+4 (dword); Owner at slot+248 (= dword[62]).
//
// Slot anchor: original IDA points `o` at slot+4 (so `o[-4]` = active flag,
// `o[-2]` (short) = Type, o[0] = SubType, o[62] = Owner). Stride is 111 dwords.
extern "C" void __cdecl DeleteEffect(int Type, DWORD Owner, int iSubType)
{
    DWORD* o = (DWORD*)((char*)&DAT_07b11670[0] + 4);
    DWORD* end = (DWORD*)((char*)&DAT_07b11670[0] + 124 * 0x1BC + 4);
    while (o < end) {
        if (*((BYTE*)o - 4) != 0 &&
            *((short*)o - 1) == (short)Type &&
            (iSubType == -1 || (int)*o == iSubType) &&
            o[62] == Owner)
        {
            *((BYTE*)o - 4) = 0;
        }
        o += 111;  // stride 0x1BC
    }
}
void __cdecl FUN_00460d20(int Type, DWORD Owner, int iSubType)
{ DeleteEffect(Type, Owner, iSubType); }
