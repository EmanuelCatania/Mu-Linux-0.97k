#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD GoldenArcherOpenType;   // Golden Archer panel flag (globals.cpp)
extern void __cdecl operator_delete(void* ptr);
extern void ClearActionObject(void);

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
#define ITEM_SPECIAL_SKILL_OPTION             0
#define ITEM_SPECIAL_LUCK_OPTION              1
#define ITEM_OPTION_ADD_PHYSI_DAMAGE_CODE     60
#define ITEM_OPTION_ADD_MAGIC_DAMAGE_CODE     61
#define ITEM_OPTION_ADD_DEFENSE_RATE_CODE     62
#define ITEM_OPTION_ADD_DEFENSE_CODE          63
#define ITEM_OPTION_ADD_EXCELLENT_DAMAGE_CODE 72
// SetPlayerStop @ 0x004430C0: el port real está en Net/SecondPassword.cpp.
// No agregar un stub acá: entity+0x1BC es el byte de CLASE/skin, no move flags.
// CErrorReport__Write @ 0x00405540 (12 lines) — Variadic error log writer
// Formats message via wvsprintfA then passes to debug info string writer.
void __cdecl CErrorReport__Write(unsigned long ctx, char *fmt, ...) {
    char buf[0x400];
    va_list args;
    va_start(args, fmt);
    wvsprintfA(buf, fmt, args);
    va_end(args);
    // In original: CErrorReport__WriteDebugInfoStr(ctx, buf);
    // For now, just format — actual file write not critical for stub
    (void)ctx;
}

// crt_atexit @ 0x005414CE (11 lines) — CRT atexit wrapper
// Registers a function pointer for cleanup at program exit.
void __cdecl crt_atexit(void *addr) {
    // Original calls crt_onexit (_onexit internal registration)
    // In our build, use standard atexit
    if (addr) atexit((void (__cdecl *)(void))addr);
}

// FUN_00543c98 @ 0x00543C98 (58 lines) — CRT free wrapper
// Dispatches to SBH/OSBH/HeapFree depending on CRT heap type.
void __cdecl FUN_00543c98(void *ptr) {
    // In our build, delegate to standard free
    free(ptr);
}


// StopBuffer @ 0x00404C60 — implementado en Render/Render_WorldHelpers.cpp (delega a Sound_StopBuffer).

// StopMp3 @ 0x004127F0 — delega al port fiel (Music_StopTrack, src/Sound/Music.cpp).
// No reimplementarlo acá: es el mismo símbolo del binario.
void __cdecl StopMp3(char *cmd, int param) {
    Music_StopTrack((DWORD)(uintptr_t)cmd, param);
}


// crt_exit @ 0x00543839 (4 lines) — CRT _cinit wrapper
// Forwards to internal CRT initializer with default params.
void __cdecl crt_exit(int param) {
    // Original: crt_doexit(param, 0, 0) — CRT initialization dispatch
    // In our build, no-op (CRT initializes through normal startup)
    (void)param;
}

// crt_tmpfile @ 0x00543D81 (~45 lines) — MSVC CRT _tmpfile()
// Creates a temporary file using CRT file table. Returns stream pointer.
void *__cdecl crt_tmpfile(void) {
    // Original: acquires CRT lock, attempts tmpnam + open with O_CREAT|O_RDWR|O_BINARY,
    // retries on EEXIST, returns FILE* stream.
    // In our build, delegate to standard tmpfile
    return (void *)tmpfile();
}

// _strncpy — CRT strncpy wrapper (already functional)
void __cdecl _strncpy(char *dst, char *src, int n) {
    if (dst && src && n > 0) strncpy(dst, src, n);
}

// crt_fflush @ 0x005436A6 (17 lines) — CRT fflush
// NULL → flushall; non-NULL → lock, flush, unlock.
void __cdecl crt_fflush(int *fp) {
    fflush((FILE *)fp);
}
