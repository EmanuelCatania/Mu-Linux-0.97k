// Legacy_Runtime.cpp
// Extracted from stubs.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// ── Missing globals (extern-declared in various .cpp but never defined) ──────
// Trade.cpp
BYTE  g_trade_active   = 0;   // DAT_05826d30
BYTE  g_duel_mode      = 0;   // DAT_05826d31
BYTE  g_trade_param    = 0;   // DAT_05826d32
BYTE  g_trade_accepted = 0;   // DAT_05826c04
DWORD g_trade_requester= 0;   // DAT_05826c00
char  g_active_shop_name[4]= {};  // lpString_05826bfc
int   g_cached_shop_slot   = 0;   // DAT_00559684
BYTE* g_EntityBase    = nullptr;  // DAT_07abf5d0
DWORD g_LocalEntity   = 0;        // DAT_07abf5d8

// Sound.cpp — real DirectSound8 globals live in globals.cpp:
//   g_EnableSound, g_Enable3DSound, g_lpDS, g_lpDS3DListener, wavefile,
//   g_dwBufferBytes, g_lpDSBuffer[420][4], g_lpDS3DBuffer[420][4],
//   MaxBufferChannel[], BufferChannel[], Enable3DSound[], BufferName[][].

// Texture.cpp
int g_bound_texture_id = 0;   // DAT_00561574
int g_screen_height    = 480; // DAT_00561570
int g_vram_used        = 0;   // DAT_083bb9d0

// Party.cpp
BYTE* g_PartyHPTable   = nullptr; // DAT_07e11e98

// ── CRT wrappers (these are real implementations) ────────────────────────────

// FUN_005416bc — crt_sprintf (MSVC CRT sprintf stub)
int __cdecl FUN_005416bc(char* buf, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int r = vsprintf(buf, fmt, args);
    va_end(args);
    return r;
}

// FUN_0054158c — operator_delete (free)
void __cdecl FUN_0054158c(void* ptr) {
    free(ptr);
}


// ── FUN_ stubs (void returning) ───────────────────────────────────────────────
// FUN_00403a30 @ 0x00403A30 — NOP (empty function in original binary).
void __cdecl FUN_00403a30(void) {}

// ── CRT wrappers (forwarded to real CRT) ─────────────────────────────────────
// FUN_00543037 — CRT fseek wrapper
void __cdecl FUN_00543037(int *fp, int offset, int whence) {
    fseek((FILE*)fp, offset, whence);
}

// FUN_00542eb4 — CRT ftell wrapper
int __cdecl FUN_00542eb4(char *fp) {
    return (int)ftell((FILE*)fp);
}

// FUN_00541597 — CRT fread wrapper
int __cdecl FUN_00541597(void *dst, int size, int count, int *fp) {
    return (int)fread(dst, (size_t)size, (size_t)count, (FILE*)fp);
}

