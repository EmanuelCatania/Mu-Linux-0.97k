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
// SetPlayerStop @ 0x004430C0 (504 lines) — Set player entity to idle/stop animation
// Selects animation based on equipment, class, terrain. Most bulk is anti-tamper hash ops.
// 2026-08-08 BUG-FIX (el MG se renderizaba como Dark Wizard, con casco y con
// rayas): este stub coexistía con el port REAL de SetPlayerStop
// (`FUN_004430c0`, Net/SecondPassword.cpp). `FUN_0045c720` llamaba a ESTE, y el
// stub hacía:
//     *(BYTE*)(entity + 0x1bc) &= ~0x07;   // "clear movement bits"
// pero **0x1BC NO son move flags: es el byte de CLASE/skin** (lo leen
// `SetCharacterClass` como `skin`, `CheckFullSet` como `(c+444)&7`, y
// `RenderEquipmentBox` vía `CA[11]`). O sea el stub borraba la clase:
//     DW  0x00 -> 0x00   (sin cambio, por eso nunca se notó)
//     SM  0x08 -> 0x08   (sin cambio)
//     DK  0x01 -> 0x00   ✗ pasa a Dark Wizard
//     FE  0x02 -> 0x00   ✗
//     MG  0x03 -> 0x00   ✗
// Cazado con las sondas CLSPROBE: F(post-45c130)=3 → G(post-45c720)=0.
// Delegamos al port real; el stub no debe existir.
void __cdecl FUN_004430c0(int c);
void __cdecl SetPlayerStop(void *entity) {
    if (!entity) return;
    FUN_004430c0((int)(uintptr_t)entity);
}

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

// FUN_005414ce @ 0x005414CE (11 lines) — CRT atexit wrapper
// Registers a function pointer for cleanup at program exit.
void __cdecl FUN_005414ce(void *addr) {
    // Original calls FUN_00541450 (_onexit internal registration)
    // In our build, use standard atexit
    if (addr) atexit((void (__cdecl *)(void))addr);
}

// FUN_00543c98 @ 0x00543C98 (58 lines) — CRT free wrapper
// Dispatches to SBH/OSBH/HeapFree depending on CRT heap type.
void __cdecl FUN_00543c98(void *ptr) {
    // In our build, delegate to standard free
    free(ptr);
}


// StopBuffer @ 0x00404C60 — real implementation at stubs.cpp:275 (forwards to FUN_00404c60).

// StopMp3 @ 0x004127F0 — delega al port fiel (Music_StopTrack, src/Sound/Music.cpp).
//
// Esta era una SEGUNDA implementacion del mismo simbolo del binario, y es la que
// usaba StopMusic. Estaba mal en tres cosas: ignoraba `cmd` (cerraba el
// reproductor aunque estuviera sonando otro track), mandaba WM_DESTROY en vez de
// WM_CLOSE, y no limpiaba Mp3FileName — asi que el siguiente PlayMp3 creia que
// el track viejo seguia en curso. Ver [[simbolo-duplicado-patron]].
void __cdecl StopMp3(char *cmd, int param) {
    Music_StopTrack((DWORD)(uintptr_t)cmd, param);
}


// FUN_00543839 @ 0x00543839 (4 lines) — CRT _cinit wrapper
// Forwards to internal CRT initializer with default params.
void __cdecl FUN_00543839(int param) {
    // Original: FUN_0054385b(param, 0, 0) — CRT initialization dispatch
    // In our build, no-op (CRT initializes through normal startup)
    (void)param;
}

// FUN_00543d81 @ 0x00543D81 (~45 lines) — MSVC CRT _tmpfile()
// Creates a temporary file using CRT file table. Returns stream pointer.
void *__cdecl FUN_00543d81(void) {
    // Original: acquires CRT lock, attempts tmpnam + open with O_CREAT|O_RDWR|O_BINARY,
    // retries on EEXIST, returns FILE* stream.
    // In our build, delegate to standard tmpfile
    return (void *)tmpfile();
}

// _strncpy — CRT strncpy wrapper (already functional)
void __cdecl _strncpy(char *dst, char *src, int n) {
    if (dst && src && n > 0) strncpy(dst, src, n);
}

// FUN_005436a6 @ 0x005436A6 (17 lines) — CRT fflush
// NULL → flushall; non-NULL → lock, flush, unlock.
void __cdecl FUN_005436a6(int *fp) {
    fflush((FILE *)fp);
}
