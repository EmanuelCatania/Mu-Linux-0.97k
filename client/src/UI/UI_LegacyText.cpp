// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"
#include <mbstring.h>   // _mbclen (SeparateTextIntoLines, fiel a 0x0051D600)

extern "C" DWORD GoldenArcherOpenType;   // Golden Archer panel flag (globals.cpp)
extern void __cdecl operator_delete(void* ptr);
extern void FUN_004fa5a0(void);

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
// 2026-09-20: reescrita 1:1 contra el raw.  La anterior era una aproximacion
// con dos reglas propias:
//   (a) rebobinaba al ultimo espacio si caia en la mitad final de la linea
//       (`lastSpace > maxChars/2`).  El binario rebobina solo si el espacio
//       esta dentro de los ultimos min(iLineSize/2, 10) caracteres, o sea es
//       mas estricto: parte un poco antes y las lineas salen mas cortas.
//   (b) cuando no rebobinaba, cortaba a los 10 caracteres.  El binario no
//       corta ahi: parte a lo ancho de la linea, sin rebobinar.
//
// Medido: para prosa normal las dos dan el mismo resultado, porque siempre hay
// un espacio en la mitad final y la rama (b) no llega a correr.  La diferencia
// aparece con palabras largas sin espacios (URLs, nombres pegados), donde la
// version vieja cortaba a 10 caracteres.  O sea esto es fidelidad, no el
// arreglo de ningun sintoma reportado.
//
// Detalles fieles que importan: avanza por caracteres MBCS (_mbclen; con el
// locale "C" que usa este build devuelve siempre 1, igual que la version por
// bytes), el terminador de cada linea se escribe ANTES de saltar al slot
// siguiente, y el retorno es `indiceDeLinea + 1` -- nunca 0 para texto no
// vacio, cosa que la version vieja si podia devolver.
int __cdecl SeparateTextIntoLines(const char *lpszText, char *lpszSeparated, int iMaxLine, int iLineSize) {
    if (!lpszText || !lpszSeparated || iMaxLine <= 0 || iLineSize <= 0) return 0;  // guard de port

    const unsigned char *pSrc      = (const unsigned char *)lpszText;
    char                *pDst      = lpszSeparated;
    const unsigned char *pPrevSpc  = nullptr;   // v6: espacio visto en la vuelta anterior
    const unsigned char *pLastSpc  = nullptr;   // String: ultimo espacio de esta linea
    const char          *pLineHead = lpszText;  // v13
    char                *pLineBase = lpszSeparated;  // v14
    int                  iLine     = 0;         // v12

    if (*lpszText) {
        while (true) {
            const unsigned int nChar = (unsigned int)_mbclen(pSrc);

            if ((int)(nChar + (unsigned int)(pSrc - (const unsigned char *)pLineHead)) >= iLineSize) {
                if (pPrevSpc) {
                    int iBack = iLineSize / 2;
                    if (iBack > 10) iBack = 10;
                    if ((int)(pSrc - pPrevSpc) < iBack) {
                        const int iDelta = (int)(pPrevSpc - pSrc);
                        pSrc  = pPrevSpc + 1;
                        pDst += iDelta + 1;
                    }
                }
                pLineHead = (const char *)pSrc;
                *pDst = 0;
                if (iLine >= iMaxLine - 1) break;
                pDst = &pLineBase[iLineSize];
                ++iLine;
                pLineBase += iLineSize;
                pLastSpc = nullptr;
            }

            memcpy(pDst, pSrc, nChar);
            if (*pSrc == ' ') pLastSpc = pSrc;
            pSrc += nChar;
            pDst += nChar;
            if (!*pSrc) break;
            pPrevSpc = pLastSpc;
        }
    }
    *pDst = 0;
    return iLine + 1;
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
