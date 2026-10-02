#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// ── FUN_0050f5f0 @ 0x0050F5F0 — Font_CreateTextDib(hdc) ────────────────────
// Creates a DIB section for font rendering.
// Called from the model/resource loader init path with a DC handle.
//
// Parameters:
//   param_1 (HDC) — device context handle for the DIBSection
//
// Creates a 24bpp top-down DIB of size (fontW*2 × -fontH) where fontW and
// fontH are read via __ftol() from the current streaming context.
// Stores the DIB bitmap bits pointer in ppvBits_055c9e4c.
// Creates a compatible memory DC (DAT_055c9fec), selects the bitmap into it,
// sets background mode to TRANSPARENT (1).
//
// Called before Font_CreateRenderer (FUN_0040f570) to set up the memory DC
// used for GDI text rendering into the font texture.
void __cdecl Font_CreateTextDib(int dc)
{
    BITMAPINFO *bmi = (BITMAPINFO *)operator_new(0x428);
    // IDA hace `memset(DIB_INFO, 0, 0x28)` (el "zero loop" de Ghidra escribe 10
    // veces sobre biSize en vez de limpiar la cabecera).
    memset(bmi, 0, 0x28);
    bmi->bmiHeader.biSize        = 0x28;           // sizeof(BITMAPINFOHEADER)

    // Ancho y alto salen de Bitmaps[0] (no son `__ftol()` neutralizados a 0: con
    // w=0/h=0 `CreateDIBSection` falla y deja `ppvBits` en nullptr).
    //
    // IDA sub_50F5F0(HDC hdc, int a2):
    //     biWidth  = 2 * (__int64)*(float *)(a2 + 32);
    //     biHeight =    -(__int64)*(float *)(a2 + 36);
    // y `OpenFont` @0x50F690 lo llama como `sub_50F5F0(g_hDC, (int)Bitmaps)`,
    // o sea a2 = tabla de bitmaps; +32/+36 = Bitmaps[0].Width/Height (floats),
    // que en nuestro build son DAT_083a7cc0 / DAT_083a7cc4 — los mismos que ya
    // usa sub_47F360 para clampear la altura. Los puebla el OpenTGA de
    // "Interface/FontInput.tga" que corre justo antes en OpenFont.
    const LONG fontW = (LONG)(*(float *)&DAT_083a7cc0);   // Bitmaps[0].Width
    const LONG fontH = (LONG)(*(float *)&DAT_083a7cc4);   // Bitmaps[0].Height

    // SANITY (desviación): si el OpenTGA de FontInput.tga no pobló Bitmaps[0]
    // todavía, estos floats vienen en 0/basura. Crear el DIB con dimensiones
    // absurdas hace que el pixel-copy de sub_47F360 (que recorre p1 columnas
    // × 3 bytes por fila sobre ppvBits) se salga del buffer → AV dentro de GDI.
    // Si los valores no son razonables, dejamos ppvBits en nullptr: el guard de
    // sub_47F360 saltea el copy (no se dibuja, pero NO crashea).
    // Valores correctos verificados en runtime: fontW=256 fontH=32 -> 512x32.
    if (fontW <= 0 || fontH <= 0 || fontW > 4096 || fontH > 4096) {
        operator_delete((BYTE *)bmi);
        return;
    }

    bmi->bmiHeader.biWidth       =  2 * fontW;
    bmi->bmiHeader.biHeight      =     -fontH;
    bmi->bmiHeader.biPlanes      = 1;
    bmi->bmiHeader.biBitCount    = 0x18;           // 24 bpp
    bmi->bmiHeader.biCompression = 0;              // BI_RGB

    HBITMAP hBmp = CreateDIBSection((HDC)(HANDLE_PTR)dc, bmi, 0,
                                    &ppvBits_055c9e4c, nullptr, 0);
    HDC hMemDC = CreateCompatibleDC((HDC)(HANDLE_PTR)dc);
    // m_hFontDC ES DAT_055c9fec (macro en globals.h) — el font memory DC.
    // g_hDC es DAT_055ca004, el window DC que recibe SwapBuffers. NO confundirlos.
    // IDA sub_50F5F0 hace `m_hFontDC = CreateCompatibleDC(hdc)`: es el DC que usan
    // sub_480C60 / RenderBooleans / sub_47F360 para medir y componer el texto.
    m_hFontDC = hMemDC;
    SelectObject(hMemDC, hBmp);
    SetBkMode(hMemDC, 1);  // TRANSPARENT
    operator_delete((BYTE *)bmi);
}
