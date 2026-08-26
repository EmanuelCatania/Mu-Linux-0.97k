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
// SeparateTextIntoLines @ 0x0051D600 (71 lines) — Word-wrap text into fixed-size line buffer
// text = input string, out = 2D output buffer (stride=maxChars), maxLines = max output lines,
// maxChars = chars per line. Returns number of lines produced.
int __cdecl SeparateTextIntoLines(const char *text, char *out, int maxLines, int maxChars) {
    if (!text || !out || maxLines <= 0 || maxChars <= 0) return 0;
    int lineCount = 0;
    int col = 0;
    int lastSpace = -1;
    const char *lineStart = text;
    const char *p = text;

    while (*p != '\0' && lineCount < maxLines) {
        if (*p == ' ') lastSpace = col;
        col++;
        if (col >= maxChars) {
            // Line full — break at last space or hard-break
            int breakAt;
            if (lastSpace > 0 && lastSpace > maxChars / 2) {
                breakAt = lastSpace;
            } else {
                breakAt = (col < 10) ? col : 10;
            }
            char *dst = out + lineCount * maxChars;
            memcpy(dst, lineStart, breakAt);
            dst[breakAt] = '\0';
            lineCount++;
            lineStart = lineStart + breakAt;
            if (*lineStart == ' ') lineStart++; // skip space after break
            p = lineStart;
            col = 0;
            lastSpace = -1;
            continue;
        }
        p++;
    }
    // Copy remaining text
    if (col > 0 && lineCount < maxLines) {
        char *dst = out + lineCount * maxChars;
        memcpy(dst, lineStart, col);
        dst[col] = '\0';
        lineCount++;
    }
    return lineCount;
}



bool __cdecl FindTextA(char *haystack, char *needle, bool caseSensitive) {
    if (!haystack || !needle) return false;
    int tokenLen = lstrlenA(needle);
    int textLen = lstrlenA(haystack);
    if (tokenLen == 0) return false;
    int maxPos = caseSensitive ? 0 : (textLen - tokenLen);
    if (maxPos < 0) return false;

    int pos = 0;
    while (pos <= maxPos) {
        // Compare needle against haystack+pos
        bool match = true;
        for (int i = 0; i < tokenLen; i++) {
            if (haystack[pos + i] != needle[i]) { match = false; break; }
        }
        if (match) return true;
        // MBCS advance: skip 2 bytes for DBCS lead byte, else 1
        if (IsDBCSLeadByte((BYTE)haystack[pos])) pos += 2;
        else pos += 1;
    }
    return false;
}


// RenderCenterText @ 0x00514270 (18 lines) — Draw horizontally centered text
// Measures text width via GDI, converts to 640-based virtual coords, centers.
void __cdecl RenderCenterText(int x, int y, char *text) {
    int len = lstrlenA(text);
    SIZE sz;
    GetTextExtentPointA(m_hFontDC, text, len, &sz);
    // Convert pixel width to virtual 640-wide coords, halve for centering
    int halfWidth = (int)((unsigned int)(sz.cx * 0x280) / (unsigned int)WindowWidth) >> 1;
    RenderText(x - halfWidth, y, text, 0, 0, NULL);
}

// FUN_0051d740 — NOT a real function entry (falls mid-CreateOkMessageBox @ 0x0051D6F0)
// Kept as no-op stub; real init logic is in CreateOkMessageBox/InitGame.
void __cdecl FUN_0051d740(void) {}

// FUN_00482350 — NOT a real function entry (falls mid-FUN_004824c0 item slot lookup)
// Kept as no-op stub; address was incorrectly identified as function start.
void __cdecl FUN_00482350(void) {}

// FUN_004827a0 — NOT a real function entry (also mid-FUN_004824c0)
// Kept as no-op stub.
void __cdecl FUN_004827a0(void) {}

// FUN_00433830 — NOT a real function entry (falls mid-ReceiveTradeExit @ 0x004337F0)
// Kept as no-op stub.
void __cdecl FUN_00433830(void) {}

// RenderText @ 0x0047F650 — same address as FUN_0047f650 (Chat_DrawEntry).
// IDA's canonical name for that function IS "RenderText"; our codebase
// labels it Chat_DrawEntry but it is the universal text-draw used everywhere
// (HUD, error messages, inventory tooltips). Without this alias all the
// HUD_Pass3/6 RenderText() calls were silently no-op'd → invisible labels
// on stat panels, inventory headers, party UI, etc.
void __cdecl RenderText(int x, int y, char *text, int p1, int p2, void *p3) {
    (void)p3;  // matches "extra" stack arg, unused by the underlying call
    FUN_0047f650((undefined4)x, (undefined4)y, (LPCSTR)text,
                 (LPSIZE)(uintptr_t)p1, (char)p2, (undefined4)0);
}

// ═══════════════════════════════════════════════════════════════════════════════
