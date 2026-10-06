#include "stdafx.h"
#include "functions.h"
#include "globals.h"

// IDA: sub_50F5F0 (0x0050F5F0) — superficie donde GDI rasteriza el texto.
// La crea CFont::CreateTextSurface (Core/Font.h): DIB de 24 bits y DC de memoria
// con fondo transparente. Se llama antes de Font_CreateRenderer (0x40F570).
void __cdecl Font_CreateTextDib(int dc)
{
    // Ancho y alto salen de Bitmaps[0]. IDA sub_50F5F0(HDC hdc, int a2):
    //     biWidth  = 2 * (__int64)*(float *)(a2 + 32);
    //     biHeight =    -(__int64)*(float *)(a2 + 36);
    // OpenFont (0x50F690) la llama con a2 = tabla de bitmaps; +32/+36 son
    // Bitmaps[0].Width/Height (floats), que en nuestro build son DAT_083a7cc0 /
    // DAT_083a7cc4. Los puebla el OpenTGA de "Interface/FontInput.tga" que corre
    // justo antes en OpenFont.
    const LONG fontW = (LONG)(*(float *)&DAT_083a7cc0);   // Bitmaps[0].Width
    const LONG fontH = (LONG)(*(float *)&DAT_083a7cc4);   // Bitmaps[0].Height

    // DESVIACION: si FontInput.tga todavía no pobló Bitmaps[0], estos valores
    // vienen en 0 o basura, y un DIB de esas dimensiones hace que el pixel-copy
    // de sub_47F360 se salga del buffer (AV dentro de GDI). En ese caso no se
    // crea la superficie: sub_47F360 saltea el copy porque los bits quedan nulos.
    // Valores verificados en runtime: 256 x 32 -> DIB de 512 x 32.
    if (fontW <= 0 || fontH <= 0 || fontW > 4096 || fontH > 4096) {
        return;
    }

    gFont.CreateTextSurface((HDC)(HANDLE_PTR)dc, 2 * fontW, fontH);
}
