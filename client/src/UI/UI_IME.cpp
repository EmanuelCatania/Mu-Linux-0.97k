// UI_IME.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// SetPositionIME_Wnd @ 0x0047ECAF (~27 lines) — positions IME composition window
// Builds a COMPOSITIONFORM-like struct {style=2, x, y} and sends WM_IME_CONTROL
// (0x283) with IMC_SETCOMPOSITIONWINDOW (0xC) to the default IME window.
void __stdcall SetPositionIME_Wnd_stub(float x, float y) {
    int xRight = WindowWidth;
    DWORD style = 2;  // CFS_POINT
    LONG ptX = (LONG)x;
    LONG ptY = (LONG)y;

    // COMPOSITIONFORM-like struct: {dwStyle, ptCurrentPos.x, ptCurrentPos.y, rcArea}
    struct {
        DWORD dwStyle;
        LONG  x;
        LONG  y;
        RECT  rcArea;
    } compForm;
    compForm.dwStyle = style;
    compForm.x = ptX;
    compForm.y = ptY;
    SetRect(&compForm.rcArea, 0, 0, xRight, WindowHeight);

    HWND hImeWnd = ImmGetDefaultIMEWnd((HWND)g_hWnd);
    SendMessageA(hImeWnd, 0x283, 0xC, (LPARAM)&compForm);
}

// CheckIME_Status @ 0x0047EDC0 (~32 lines) — checks/saves/resets IME conversion status
// Globals: DAT_07e11d94 = g_dwOldConv, DAT_00559cd8 = g_dwOldSent, DAT_07e11d98 = g_dwCurrConv
bool __cdecl CheckIME_Status_stub(bool change, int mode) {
    bool bIme = false;
    HIMC hImc = ImmGetContext(g_hWnd);
    DWORD dwConv = 0, dwSent = 0;
    ImmGetConversionStatus(hImc, &dwConv, &dwSent);
    if (dwConv != 0 || dwSent != 0) {
        bIme = true;
        if (((BYTE)mode & 1) == 1) {
            DAT_07e11d94 = dwConv;   // save old conversion status
        }
        if (((BYTE)mode & 2) == 2) {
            DAT_00559cd8 = dwSent;   // save old sentence status
        }
        if (change) {
            dwConv = 0;
            dwSent = 0;
            ImmSetConversionStatus(hImc, 0, 0);
        }
    }
    ImmReleaseContext(g_hWnd, hImc);
    DAT_07e11d98 = dwConv;   // always update current conversion status
    return bIme;
}

// RenderIME_Status @ 0x0047EE52 (~64 lines) — debug: renders IME state on screen
// Renders Korean/English mode, sentence mode, old sentence mode, lock status
// at screen positions (100, 100/110/120/130).
// Uses DAT_07e11d94 (g_dwOldConv), DAT_00559cd8 (g_dwOldSent).
void __stdcall RenderIME_Status_stub(void) {
    char local_64[100];

    // Line 1: Korean/English mode based on bit 0 of g_dwOldConv
    const char* pcVar5;
    if ((DAT_07e11d94 & 1) == 1) {
        pcVar5 = "KOREAN";
    } else {
        pcVar5 = "ENGLISH";
    }
    sprintf(local_64, "%s", pcVar5);
    m_dwTextColor = 0xffd2e6ff;
    m_dwBackColor = 0;
    RenderText(100, 100, local_64, 0, 1, NULL);
    int iVar1 = lstrlenA(local_64);
    SIZE sz;
    GetTextExtentPointA(m_hFontDC, local_64, iVar1, &sz);

    // Line 2: Sentence mode (current IME conversion status)
    HIMC hImc = ImmGetContext(g_hWnd);
    DWORD dwConv = 0, dwSent = 0;
    ImmGetConversionStatus(hImc, &dwConv, &dwSent);
    ImmReleaseContext(g_hWnd, hImc);
    sprintf(local_64, "Sentence Mode : %d", dwSent);
    RenderText(100, 0x6e, local_64, 0, 1, NULL);
    iVar1 = lstrlenA(local_64);
    GetTextExtentPointA(m_hFontDC, local_64, iVar1, &sz);

    // Line 3: Old sentence mode
    sprintf(local_64, "Old Sentence Mode : %d", DAT_00559cd8);
    RenderText(100, 0x78, local_64, 0, 1, NULL);
    iVar1 = lstrlenA(local_64);
    GetTextExtentPointA(m_hFontDC, local_64, iVar1, &sz);

    // Line 4: Lock input status
    sprintf(local_64, "LockInputStatus %d", (int)DAT_07e11d6f);
    RenderText(100, 0x82, local_64, 0, 1, NULL);
    iVar1 = lstrlenA(local_64);
    GetTextExtentPointA(m_hFontDC, local_64, iVar1, &sz);
}
