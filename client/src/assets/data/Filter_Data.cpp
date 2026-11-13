#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// ── Filter_LoadTextFile @ 0x004799D0 — Filter_LoadData(path) ───────────────────────
// Reads a text-format filter/word-list data file (used for chat filtering).
// Parser uses TextParser_GetToken (type 2=EOF).
// For each token read, copies the string from TextParserTokenString into
// DAT_07d73104 + DAT_07d78070 * 0x14 (stride 0x14, each entry 20 bytes),
// then increments DAT_07d78070 (filter entry count).
// Loop ends when TextParser_GetToken returns 2 (EOF).
// IDA: sub_4799D0 (0x004799D0)
void __cdecl Filter_LoadTextFile(const char *path)
{
    DAT_07d7806c = (FILE *)crt_fopen(path, DAT_005580ac);
    if (!DAT_07d7806c) return;

    while (TextParser_GetToken() != 2) {
        // token string is in TextParserTokenString; copy to filter table entry
        char *dst = DAT_07d73104 + DAT_07d78070 * 0x14;
        // strlen(TextParserTokenString) then memcpy
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
    crt_fclose(DAT_07d7806c);
}
