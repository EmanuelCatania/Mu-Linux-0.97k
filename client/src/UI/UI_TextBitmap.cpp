// UI_TextBitmap.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_0047f360 @ 0x0047F360 (~93 lines) — text render to bitmap with color mask
// TextOutA to m_hFontDC, then pixel-copy loop: nonzero->m_dwTextColor, zero->m_dwBackColor.
void __cdecl FUN_0047f360(int p1, int p2, LPCSTR p3, int p4, int p5, int p6, int p7, int p8, LPCSTR p9) {
    // 0x0047F360 — text render to bitmap with color mask
    // Renders text string to m_hFontDC, then copies pixels to Bitmaps[0] pixel buffer
    // applying color mask: nonzero pixels → m_dwTextColor (or prefix color), zero → m_dwBackColor
    // p1 = bitmap width, p2 = bitmap height, p3 = main text, p6 = x offset
    // p8 = color split X (0 = use p1), p9 = prefix string (optional)
    (void)p4; (void)p5; (void)p7;

    int bmpWidth = p1;
    // FIDELIDAD 2026-08-15: IDA inicializa `sz.cx = a1` ANTES del check de '\n'
    // (sub_47F360 L21-23).  Si el texto empieza en '\n' la función saltea todo el
    // TextOut y sz.cx queda en a1, así que el split de color nunca se cruza y la
    // fila entera se pinta con SetTextColor_0.  Antes arrancábamos en 0 (=todo
    // m_dwTextColor), que es el caso opuesto.
    int prefixWidth = bmpWidth;

    if (*p3 != '\n') {
        // Render optional prefix string
        if (p9 == NULL) {
            prefixWidth = 0;
        } else {
            SIZE sz;
            int prefixLen = lstrlenA(p9);
            GetTextExtentPointA(m_hFontDC, p9, prefixLen, &sz);
            prefixWidth = sz.cx;

            // Output prefix text at (p6, 0)
            int slen = lstrlenA(p9);
            TextOutA(m_hFontDC, p6, 0, p9, slen);
        }

        // Set text color to white for main text
        SetTextColor(m_hFontDC, 0xffffff);

        // Output main text at (prefixWidth + p6, 0)
        int mainLen = lstrlenA(p3);
        TextOutA(m_hFontDC, prefixWidth + p6, 0, p3, mainLen);
    }

    // If p8 == 0, use p1 (bitmap width) as the color-split boundary
    if (p8 == 0) {
        p8 = bmpWidth;
    }

    // Clamp height to Bitmaps[0].Height (float → int)
    int maxH = (int)(*(float*)(&DAT_083a7cc4));
    int height = p2;
    if (maxH < p2) {
        height = maxH;
    }

    // Pixel-copy loop: read 3-byte pixels from ppvBits, write 4-byte DWORD to Bitmaps[0] buffer
    // Bitmaps[0] buffer stride = 0x100 DWORDs per row (256 pixels * 4 bytes)
    // ppvBits stride = 0x600 bytes per row (512 pixels * 3 bytes, or 256 * 6 — double-height?)
    DWORD* dstRow = (DWORD*)(uintptr_t)DAT_083a7cd4;
    char*  srcRow = (char*)ppvBits_055c9e4c;

    // BUG-FIX 2026-07-19 (CRASH 0xC0000005 addr=FUN_0047f360+0x14B, param1=0):
    // el loop leía `*src` con `srcRow = ppvBits_055c9e4c` en NULL. `ppvBits` es
    // el puntero a los bits del DIB de la fuente (lo crea Font_BuildLayout vía
    // CreateDIBSection, Font_Layout.cpp:39); si esa init no corrió todavía queda
    // en nullptr. Hasta ahora no se notaba porque esta función solo se alcanza
    // desde RenderBoolean (burbujas de chat), que era código muerto — el pool
    // estaba partido en 3 globals y nunca tenía slots activos.
    // Mismo guard que ya usa HUD_Pass4.cpp:663 para este idéntico pixel-copy.
    if (!dstRow || !srcRow) {
        return;
    }

    if (height > 0) {
        int rowsLeft = height;
        do {
            int col = 0;
            DWORD* dst = dstRow;
            char* src = srcRow;
            if (bmpWidth > 0) {
                do {
                    DWORD color;
                    if (*src == '\0') {
                        // Background pixel: m_dwBackColor if col < p8, else 0 (transparent)
                        // asm: SETGE(col>=p8)→DEC→AND m_dwBackColor
                        color = (col < p8) ? m_dwBackColor : 0;
                    } else {
                        // Text pixel: use prefix color if col < prefixWidth, else m_dwTextColor
                        if (col < prefixWidth) {
                            // 0x00559C7C — IDA `SetTextColor_0`: color del prefijo
                            // (nombre de guild).  2026-08-15: esto leía el global
                            // `DAT_00559c7c`, que en nuestro build era una memoria
                            // SEPARADA de `SetTextColor_0` (la que sí escriben
                            // RenderBoolean/RenderPartyHP) y quedaba en 0 → el
                            // [guild] salía transparente.  Unificados en globals.h.
                            color = SetTextColor_0;
                        } else {
                            color = DAT_00559c78;  // m_dwTextColor
                        }
                    }
                    *dst = color;
                    src += 3;
                    dst++;
                    col++;
                } while (col < bmpWidth);
            }
            rowsLeft--;
            dstRow += 0x100;   // 256 DWORDs per row
            srcRow += 0x600;   // 1536 bytes per row (512 * 3)
        } while (rowsLeft != 0);
    }
}

// FUN_0047f4c0 @ 0x0047F4C0 (~64 lines) — upload text bitmap + render as sprite
// glTexImage2D uploads Bitmaps[0xd]. Clamps to screen bounds. RenderBitmap.
// FUN_0047f4c0 (IDA-activated, was Ghidra stub)
void __cdecl FUN_0047f4c0(int a1, int a2, float Width, float Height, int a5, int a6, float a7, int a8)
{
  int v8; // ecx
  DWORD v9; // eax
  int v10; // edi
  DWORD v11; // eax
  signed int v12; // esi
  float x; // [esp+0h] [ebp-34h]
  float y; // [esp+4h] [ebp-30h]
  float uWidth; // [esp+18h] [ebp-1Ch]
  float vHeight; // [esp+1Ch] [ebp-18h]
  float Widtha; // [esp+40h] [ebp+Ch]
  float Heighta; // [esp+44h] [ebp+10h]

  glBindTexture(0xDE1u, Bitmaps[0].TextureNumber);
  glTexImage2D(
    0xDE1u,
    0,
    Bitmaps[0].Components,
    (__int64)Bitmaps[0].Width,
    (__int64)Bitmaps[0].Height,
    0,
    0x1908u,
    0x1401u,
    Bitmaps[0].Buffer);
  v8 = a1;
  if ( a1 < 0 )
  {
    v8 = 0;
    a1 = 0;
  }
  if ( (BYTE)(a7) )
  {
    v9 = 640 * LODWORD(Width) / WindowWidth;
    if ( (int)(v9 + v8) > a8 )
    {
      a1 = a8 - v9;
    }
  }
  else if ( LODWORD(Width) + v8 > (int)WindowWidth )
  {
    a1 = WindowWidth - LODWORD(Width);
  }
  if ( DAT_07e11d6e )
  {
    v10 = a2;
    if ( a2 < 0 )
    {
      v10 = 0;
      a2 = 0;
    }
    if ( (BYTE)(a7) )
    {
      v11 = 480 * LODWORD(Height) / WindowHeight;
      if ( v11 + v10 > 0x1B1 )
      {
        a2 = 433 - v11;
      }
    }
    else
    {
      v12 = WindowHeight - (int)(47 * WindowHeight) / 640;
      if ( LODWORD(Height) + v10 > v12 )
      {
        a2 = v12 - LODWORD(Height);
      }
    }
  }
  Heighta = (float)SLODWORD(Height);
  Widtha = (float)SLODWORD(Width);
  vHeight = (Heighta + 0.0099999998) / Bitmaps[0].Height;
  uWidth = (Widtha + 0.0099999998) / Bitmaps[0].Width;
  y = (float)a2;
  x = (float)a1;
  RenderBitmap(0, x, y, Widtha, Heighta, 0.0, 0.0, uWidth, vHeight, 0, SLOBYTE(a7));
}


extern "C" int Text_MeasureOrthoWidth(const char* text);   // stubs_externs.cpp
