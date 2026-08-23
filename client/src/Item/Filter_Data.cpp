#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// ── FUN_004799d0 @ 0x004799D0 — Filter_LoadData(path) ───────────────────────
// Reads a text-format filter/word-list data file (used for chat filtering).
// Parser uses FUN_0047a1f0 (type 2=EOF).
// For each token read, copies the string from DAT_07cf1ef0 into
// DAT_07d73104 + DAT_07d78070 * 0x14 (stride 0x14, each entry 20 bytes),
// then increments DAT_07d78070 (filter entry count).
// Loop ends when FUN_0047a1f0 returns 2 (EOF).
void __cdecl FUN_004799d0(const char *path)
{
    DAT_07d7806c = (FILE *)FUN_0054173f(path, DAT_005580ac);
    if (!DAT_07d7806c) return;

    while (FUN_0047a1f0() != 2) {
        // token string is in DAT_07cf1ef0; copy to filter table entry
        char *dst = DAT_07d73104 + DAT_07d78070 * 0x14;
        // strlen(DAT_07cf1ef0) then memcpy
        UINT len = 0xFFFFFFFF;
        const char *p = TokenString;
        while (*p++ != '\0') len--;
        len = ~len;  // actual length including NUL
        const char *src = p - len;
        for (UINT i = len >> 2; i != 0; i--) {
            *(DWORD *)dst = *(DWORD *)src;
            src += 4; dst += 4;
        }
        for (UINT r = len & 3; r != 0; r--) {
            *dst++ = *src++;
        }
        DAT_07d78070++;
    }
    FUN_0054150f(DAT_07d7806c);
}
