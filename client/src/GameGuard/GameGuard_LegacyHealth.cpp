// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD DAT_07eaa128;   // Golden Archer panel flag (globals.cpp)
extern void __cdecl FUN_0054158c(void* ptr);
extern void FUN_004fa5a0(void);

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
#define ITEM_SPECIAL_SKILL_OPTION             0
#define ITEM_SPECIAL_LUCK_OPTION              1
#define ITEM_OPTION_ADD_PHYSI_DAMAGE_CODE     60
#define ITEM_OPTION_ADD_MAGIC_DAMAGE_CODE     61
#define ITEM_OPTION_ADD_DEFENSE_RATE_CODE     62
#define ITEM_OPTION_ADD_DEFENSE_CODE          63
#define ITEM_OPTION_ADD_EXCELLENT_DAMAGE_CODE 72
// FUN_0053d430 @ 0x0053D430 (67 lines) — GameGuard encrypted log init
// Allocates 0x34c-byte context, inits crypto via CryptAcquireContext,
// installs exception filter, opens log file. Singleton (returns if already init).
void __cdecl FUN_0053d430(unsigned char *buf) {
    (void)buf;
    // GameGuard is disabled in this build — no-op.
    // Original: allocates crypto context, sets up encrypted log file,
    // installs TopLevelExceptionFilter, calls FUN_0053d890.
}

// FUN_0053ea90 @ 0x0053EA90 (44 lines) — GameGuard per-tick health check
// Checks GG process status, heartbeat event, returns error codes.
// In our build, GameGuard is disabled — return 0x755 (OK/running).
int __cdecl FUN_0053ea90(void *param) {
    (void)param;
    return 0x755; // GG status OK
}
