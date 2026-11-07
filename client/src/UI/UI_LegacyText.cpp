// UI_LegacyText.cpp
#include "stdafx.h"
#include "Text/TextCore.h"
#include <string_view>
#include "Platform/Text.h"
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
// SeparateTextIntoLines @ 0x0051D600 -- corta un texto en lineas de ancho fijo.
//
// IDA: avanza por caracteres MBCS y, al llenar la linea (iLineSize - 1 bytes),
// rebobina al ultimo espacio solo si esta dentro de los ultimos
// min(iLineSize/2, 10) caracteres; si no, parte a lo ancho. El terminador de
// cada linea se escribe antes de saltar al slot siguiente y el retorno es
// `indiceDeLinea + 1` -- nunca 0.
//
// DESVIACION (Fase 4, cortes UAX #14): en vez del ultimo espacio se usa el
// ultimo punto de corte de libunibreak (entre ideogramas, despues de un
// guion, antes de un salto de linea...), con la misma ventana de rebobinado;
// fuera de ella se corta a lo ancho sin partir un grapheme cluster. Los
// saltos de linea del texto cortan la linea y no se copian.
int __cdecl SeparateTextIntoLines(const char *lpszText, char *lpszSeparated, int iMaxLine, int iLineSize) {
    if (!lpszText || !lpszSeparated || iMaxLine <= 0 || iLineSize <= 0) return 0;  // guard de port

    const std::size_t width = (std::size_t)(iLineSize - 1);
    std::size_t rewind = (std::size_t)(iLineSize / 2);
    if (rewind > 10) rewind = 10;

    std::string_view rest(lpszText);
    int iLine = 0;
    while (true) {
        char *pDst = lpszSeparated + (std::size_t)iLine * (std::size_t)iLineSize;
        std::size_t take = TextEngine::FitLineBreak(rest, width);
        if (rest.size() > width && (take == 0 || width - take + 1 >= rewind))
            take = TextEngine::FitBytes(rest, width);
        if (take == 0 && !rest.empty())
            take = TextEngine::NextCluster(rest, 0);   // un cluster mas ancho que la linea
        std::size_t copy = take < width ? take : width;
        while (copy > 0 && (rest[copy - 1] == '\n' || rest[copy - 1] == '\r')) --copy;
        memcpy(pDst, rest.data(), copy);
        pDst[copy] = 0;
        rest.remove_prefix(take);
        if (rest.empty() || iLine >= iMaxLine - 1) break;
        ++iLine;
    }
    return iLine + 1;
}




// RenderCenterText @ 0x00514270 (18 lines) — Draw horizontally centered text
// Measures text width via GDI, converts to 640-based virtual coords, centers.
void __cdecl RenderCenterText(int x, int y, char *text) {
    int len = Platform::TextLength(text);
    SIZE sz;
    gFont.MeasureText(gFont.GetTextDC(), text, len, &sz);
    // Convert pixel width to virtual 640-wide coords, halve for centering
    int halfWidth = (int)((unsigned int)(sz.cx * 0x280) / (unsigned int)gWindow.GetWidth()) >> 1;
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

// RenderText @ 0x0047F650 — same address as UI_RenderText (Chat_DrawEntry).
// IDA's canonical name for that function IS "RenderText"; our codebase
// labels it Chat_DrawEntry but it is the universal text-draw used everywhere
// (HUD, error messages, inventory tooltips). Without this alias all the
// HUD_Pass3/6 RenderText() calls were silently no-op'd → invisible labels
// on stat panels, inventory headers, party UI, etc.
void __cdecl RenderText(int x, int y, char *text, int p1, int p2, void *p3) {
    (void)p3;  // matches "extra" stack arg, unused by the underlying call
    UI_RenderText((undefined4)x, (undefined4)y, (LPCSTR)text,
                 (LPSIZE)(uintptr_t)p1, (char)p2, (undefined4)0);
}

// ═══════════════════════════════════════════════════════════════════════════════
