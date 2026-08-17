// Chat.cpp
// In-game chat log rendering and message management.
//
// FUN_0047f650 @ 0x0047f650 — Chat_DrawEntry
// FUN_0047f0b0 @ 0x0047f0b0 — Chat_DrawField
// FUN_0047fae0 @ 0x0047fae0 — Chat_AddMessage
// FUN_0047fce0 @ 0x0047fce0 — Chat_DrawMessages
// FUN_00480980 @ 0x00480980 — UI_DrawPlayerList
// FUN_004821a0 @ 0x004821a0 — UI_TickTooltips
// FUN_00513570 @ 0x00513570 — Chat_ValidateCommand
//
// Chat message ring buffer: DAT_07db80d8, 6 slots × 0x108 bytes each
//   Slot layout: [0x100] char name[], [0x104] flags byte, [0x108] next slot
// Current message count: DAT_07e11d9c (max 6)
// Chat scroll timer: DAT_00559cdc (reset to 300 on new message)

#include "stdafx.h"

// ─────────────────────────────────────────────────────────────────────────────
// ScaleGlobalTextSize — corrige el patrón `lVar = __ftol();` de Ghidra.
//
// Ghidra dejó en varias funciones de este archivo llamadas a `__ftol()` SIN
// ARGUMENTOS, que en C leen basura del tope de la pila x87. En el binario real
// son la conversión del resultado de una división FPU. Verificado contra IDA
// (RenderText 0x47F650, RenderNotices 0x47FCE0 — ambas idénticas):
//
//     GetTextExtentPointA(m_hFontDC, pszText, len, &TextSize);
//     TextSize.cx = (__int64)((double)TextSize.cx / g_fScreenRate_x);
//     TextSize.cy = (__int64)((double)TextSize.cy / g_fScreenRate_y);
//
// `TextSize` es el par de globals (lpsz_07e113d0 = cx, _DAT_07e113d4 = cy);
// _DAT_055c9b70 = g_fScreenRate_x, _DAT_055c9b74 = g_fScreenRate_y.
//
// Impacto del bug: cualquier layout que dependa del ancho medido quedaba con
// basura. El caso visible fue el caret `_` del input de chat, que se dibuja en
// `x + lpsz_07e113d0->cx` y por eso quedaba siempre pegado al inicio.
// Guarda contra rate 0 (sin inicializar) para no dividir por cero.
static void ScaleGlobalTextSize(void)
{
    float rx = _DAT_055c9b70, ry = _DAT_055c9b74;
    if (rx <= 0.0f) rx = 1.0f;
    if (ry <= 0.0f) ry = 1.0f;
    lpsz_07e113d0 = (LPSIZE)(LONG)((double)(LONG)(intptr_t)lpsz_07e113d0 / (double)rx);
    _DAT_07e113d4 = (LONG)((double)(LONG)_DAT_07e113d4 / (double)ry);
}


// FUN_0047f650 — Chat_DrawEntry
// Draws one chat entry at screen (param_1, param_2) with text param_3.
// param_4: max pixel width (truncates lpsz_07e113d0 if exceeded)
// param_5: style (1=normal, 2=bold via FUN_0047f7a0 style param)
// param_6: extra param forwarded to FUN_0047f7a0
// Returns packed (width, height) in two halves of undefined8.
undefined8 __cdecl
FUN_0047f650(undefined4 param_1,undefined4 param_2,LPCSTR param_3,LPSIZE param_4,char param_5,
            undefined4 param_6)
{
  int c;
  LPSIZE *lpsz;

  FUN_0047f7a0(param_1,param_2,(char*)param_3,(int)param_4,(param_5 != '\0') + 1,param_6);
  lpsz = &lpsz_07e113d0;
  c = lstrlenA(param_3);
  GetTextExtentPointA(DAT_055c9fec,param_3,c,(LPSIZE)lpsz);
  if ((0 < (int)param_4) && ((int)param_4 < (int)lpsz_07e113d0)) {
    lpsz_07e113d0 = param_4;
  }
  // IDA RenderText @0x47F650: TextSize.cx /= g_fScreenRate_x; .cy /= g_fScreenRate_y;
  ScaleGlobalTextSize();
  // IDA devuelve &TextSize; el CONCAT44 con lVar1>>32 era ruido de Ghidra
  // (par de registros del retorno de 64 bits).
  return CONCAT44(0,&lpsz_07e113d0);
}


// FUN_0047f0b0 — Chat_DrawField
// Draws an input field character at (param_1, param_2) for field index param_3.
// Masks characters with '*' if field type is password (DAT_07e113d8[param_3]==1).
// Shows partial mask (first 7 chars visible) for type 2.
// Also draws a caret '|' after the last character when cursor is on this field.
void __cdecl FUN_0047f0b0(int param_1,undefined4 param_2,int param_3)
{
  char cVar1;
  LPSIZE ptVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  char *pcVar6;
  char *pcVar7;
  LPCSTR *ppCVar8;
  longlong lVar9;
  LPCSTR *lpString;
  LPSIZE *pptVar10;
  undefined4 local_100;
  undefined2 local_fc;
  undefined1 local_fa;
  undefined1 auStack_f9 [249];

  ptVar2 = (LPSIZE)(uintptr_t)DAT_00559c8c;
  DAT_00559c78 = 0xffd2e6ff;
  DAT_00559c80 = 0;
  if (DAT_07e113d8[param_3] == '\x01') {
    iVar3 = -1;
    uVar5 = 0;
    pcVar6 = (char *)DAT_07db8710 + param_3 * 0x100;  // 2026-05-04: cast first — DAT_07db8710 ahora es char[10][256], aritmética de puntero estridaba 65536 bytes
    do {
      if (iVar3 == 0) break;
      iVar3 = iVar3 + -1;
      cVar1 = *pcVar6;
      pcVar6 = pcVar6 + 1;
    } while (cVar1 != '\0');
    if (iVar3 != -2) {
      do {
        *(undefined1 *)((int)&local_100 + uVar5) = 0x2a;
        uVar4 = 0xffffffff;
        uVar5 = uVar5 + 1;
        pcVar6 = (char *)DAT_07db8710 + param_3 * 0x100;  // 2026-05-04: cast first — DAT_07db8710 ahora es char[10][256], aritmética de puntero estridaba 65536 bytes
        do {
          if (uVar4 == 0) break;
          uVar4 = uVar4 - 1;
          cVar1 = *pcVar6;
          pcVar6 = pcVar6 + 1;
        } while (cVar1 != '\0');
      } while (uVar5 < ~uVar4 - 1);
    }
    *(undefined1 *)((int)&local_100 + uVar5) = 0;
  }
  else if (DAT_07e113d8[param_3] == '\x02') {
    pcVar6 = (char *)DAT_07db8710 + param_3 * 0x100;  // 2026-05-04: cast first — DAT_07db8710 ahora es char[10][256], aritmética de puntero estridaba 65536 bytes
    local_100 = *(undefined4 *)pcVar6;
    uVar5 = 0xffffffff;
    local_fc = *(undefined2 *)(&DAT_07db8714 + param_3 * 0x100);
    uVar4 = 7;
    local_fa = (&DAT_07db8716)[param_3 * 0x100];
    pcVar7 = pcVar6;
    do {
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      cVar1 = *pcVar7;
      pcVar7 = pcVar7 + 1;
    } while (cVar1 != '\0');
    if (7 < ~uVar5 - 1) {
      do {
        *(undefined1 *)((int)&local_100 + uVar4) = 0x2a;
        uVar5 = 0xffffffff;
        uVar4 = uVar4 + 1;
        pcVar7 = pcVar6;
        do {
          if (uVar5 == 0) break;
          uVar5 = uVar5 - 1;
          cVar1 = *pcVar7;
          pcVar7 = pcVar7 + 1;
        } while (cVar1 != '\0');
      } while (uVar4 < ~uVar5 - 1);
    }
    *(undefined1 *)((int)&local_100 + uVar4) = 0;
  }
  else {
    uVar5 = 0xffffffff;
    pcVar6 = (char *)DAT_07db8710 + param_3 * 0x100;  // 2026-05-04: cast first — DAT_07db8710 ahora es char[10][256], aritmética de puntero estridaba 65536 bytes
    do {
      pcVar7 = pcVar6;
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      pcVar7 = pcVar6 + 1;
      cVar1 = *pcVar6;
      pcVar6 = pcVar7;
    } while (cVar1 != '\0');
    uVar5 = ~uVar5;
    pcVar6 = pcVar7 + -uVar5;
    pcVar7 = (char *)&local_100;
    for (uVar4 = uVar5 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined4 *)pcVar7 = *(undefined4 *)pcVar6;
      pcVar6 = pcVar6 + 4;
      pcVar7 = pcVar7 + 4;
    }
    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar7 = *pcVar6;
      pcVar6 = pcVar6 + 1;
      pcVar7 = pcVar7 + 1;
    }
  }
  FUN_0047f7a0(param_1,param_2,(char *)&local_100,(int)DAT_00559c8c,1,0);
  pptVar10 = &lpsz_07e113d0;
  iVar3 = lstrlenA((LPCSTR)&local_100);
  GetTextExtentPointA(DAT_055c9fec,(LPCSTR)&local_100,iVar3,(LPSIZE)pptVar10);
  if ((0 < (int)ptVar2) && ((int)ptVar2 < (int)lpsz_07e113d0)) {
    lpsz_07e113d0 = ptVar2;
  }
  // BUG-FIX 2026-07-19 (el cursor `_` no se movía al escribir): acá había
  // `lVar9 = __ftol();` — llamada SIN argumentos, artefacto de Ghidra que lee
  // basura del tope de la pila x87. El IDA hace:
  //   TextSize.cx = (__int64)((double)TextSize.cx / g_fScreenRate_x);
  //   TextSize.cy = (__int64)((double)TextSize.cy / g_fScreenRate_y);
  // `lpsz_07e113d0` guarda el ANCHO del texto y es lo que posiciona el caret
  // más abajo (`(int)&lpsz_07e113d0->cx + param_1`). Con basura, el ancho
  // quedaba en 0/garbage → el `_` se dibujaba siempre al principio.
  ScaleGlobalTextSize();
  if (param_3 == DAT_07e11d78) {
    uVar5 = DAT_07e11d2c & 0x80000001;
    if ((int)uVar5 < 0) {
      uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
    }
    DAT_07e11d2c = DAT_07e11d2c + 1;
    if (uVar5 == 0) {
      lpString = (LPCSTR *)(&DAT_07e11cec + param_3 * 4);
      iVar3 = -1;
      ppCVar8 = lpString;
      do {
        if (iVar3 == 0) break;
        iVar3 = iVar3 + -1;
        cVar1 = *(char *)ppCVar8;
        ppCVar8 = (LPCSTR *)((int)ppCVar8 + 1);
      } while (cVar1 != '\0');
      if (iVar3 == -2) {
        FUN_0047f7a0((int)&lpsz_07e113d0->cx + param_1,param_2,(char *)&lpString_00559d40,0,1,0);
        pptVar10 = &lpsz_07e113d0;
        iVar3 = lstrlenA((LPCSTR)&lpString_00559d40);
        lpString = (LPCSTR*)&lpString_00559d40;
      }
      else if (DAT_07e113d8[param_3] == '\x01') {
        FUN_0047f7a0((int)&lpsz_07e113d0->cx + param_1,param_2,(char *)&lpString_00559d3c,0,1,0);
        pptVar10 = &lpsz_07e113d0;
        iVar3 = lstrlenA((LPCSTR)&lpString_00559d3c);
        lpString = (LPCSTR*)&lpString_00559d3c;
      }
      else {
        FUN_0047f7a0((int)&lpsz_07e113d0->cx + param_1,param_2,(char *)lpString,0,1,0);
        pptVar10 = &lpsz_07e113d0;
        iVar3 = lstrlenA((LPCSTR)lpString);
      }
      GetTextExtentPointA(DAT_055c9fec,(LPCSTR)lpString,iVar3,(LPSIZE)pptVar10);
      ScaleGlobalTextSize();   // era `__ftol()` sin args — ver helper arriba
    }
  }
  return;
}


// FUN_0047fae0 — Chat_AddMessage
// Appends a chat message string param_1 (with flag param_2) to the ring buffer.
// Ring buffer: DAT_07db80d8, 6 slots × 0x108 bytes.
// If count > 5: shifts buffer down (oldest discarded). Handles long lines by
// splitting via FUN_0047fe30 into first/overflow parts.
// Resets scroll timer DAT_00559cdc to 300.
void __cdecl FUN_0047fae0(char *param_1,unsigned char param_2)
{
    // 2026-05-04: AUTO-SKIP removed. DAT_07db80d8 ahora propiamente sized
    // (6 slots × 0x108). Reescribo el shift-buffer loop con count explícito
    // en vez del bound literal `< 0x7db8600`.
    if (!param_1) return;
    char *base = (char *)&DAT_07db80d8[0];
    SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca010);
    SIZE local_208 = {0,0};
    GetTextExtentPointA(DAT_055c9fec, param_1, lstrlenA(param_1), &local_208);

    // Si el buffer está lleno (>= 6 slots), shift down 1 slot (oldest discarded).
    if (DAT_07e11d9c > 5) {
        DAT_07e11d9c = 5;
        for (int s = 0; s < 5; ++s) {
            char *dst = base + s * 0x108;
            char *src = base + (s + 1) * 0x108;
            // Copy flag byte (offset 0x104).
            dst[0x104] = src[0x104];
            // Copy 0x100-byte string (NUL-terminated).
            lstrcpynA(dst, src, 0x100);
        }
    }

    // Append at slot DAT_07e11d9c.
    int idx = (int)DAT_07e11d9c;
    if (idx < 0) idx = 0;
    if (idx > 5) idx = 5;
    char *dst = base + idx * 0x108;

    // Truncate if too wide (256 px) — IDA splits via FUN_0047fe30; keep simple.
    lstrcpynA(dst, param_1, 0x100);
    dst[0x104] = (char)param_2;

    DAT_07e11d9c = idx + 1;
    DAT_00559cdc = 300;          // reset scroll timer
    return;
#if 0
    // Original Ghidra body kept disabled for reference:
  char cVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  char *pcVar6;
  char *pcVar7;
  char *pcVar8;
  char *pcVar9;
  tagSIZE *lpsz;
  tagSIZE local_208;
  char local_200 [256];
  char local_100 [256];

  SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca010);
  lpsz = &local_208;
  iVar2 = lstrlenA(param_1);
  GetTextExtentPointA(DAT_055c9fec,param_1,iVar2,lpsz);
  if (5 < DAT_07e11d9c) {
    DAT_07e11d9c = 5;
    pcVar6 = (char *)&DAT_07db80d8;
    do {
      pcVar8 = pcVar6 + 0x108;
      pcVar6[0x104] = pcVar6[0x20c];
      uVar4 = 0xffffffff;
      pcVar7 = pcVar8;
      do {
        pcVar9 = pcVar7;
        if (uVar4 == 0) break;
        uVar4 = uVar4 - 1;
        pcVar9 = pcVar7 + 1;
        cVar1 = *pcVar7;
        pcVar7 = pcVar9;
      } while (cVar1 != '\0');
      uVar4 = ~uVar4;
      pcVar7 = pcVar9 + -uVar4;
      for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *(undefined4 *)pcVar6 = *(undefined4 *)pcVar7;
        pcVar7 = pcVar7 + 4;
        pcVar6 = pcVar6 + 4;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *pcVar6 = *pcVar7;
        pcVar7 = pcVar7 + 1;
        pcVar6 = pcVar6 + 1;
      }
      pcVar6 = pcVar8;
    } while ((int)pcVar8 < 0x7db8600);
  }
  iVar2 = DAT_07e11d9c;
  iVar3 = DAT_07e11d9c * 0x108;
  (&DAT_07db81dc)[iVar3] = param_2;
  if (0xff < local_208.cx) {
    uVar4 = 0xffffffff;
    pcVar6 = param_1;
    do {
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      cVar1 = *pcVar6;
      pcVar6 = pcVar6 + 1;
    } while (cVar1 != '\0');
    FUN_0047fe30((undefined4 *)param_1,(int)local_100,(undefined4 *)local_200,~uVar4 - 1);
    uVar4 = 0xffffffff;
    iVar2 = DAT_07e11d9c + 1;
    pcVar6 = local_200;
    do {
      pcVar8 = pcVar6;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar8 = pcVar6 + 1;
      cVar1 = *pcVar6;
      pcVar6 = pcVar8;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    pcVar6 = pcVar8 + -uVar4;
    pcVar8 = (char *)(&DAT_07db80d8 + DAT_07e11d9c * 0x42);
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar8 = *(undefined4 *)pcVar6;
      pcVar6 = pcVar6 + 4;
      pcVar8 = pcVar8 + 4;
    }
    for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *pcVar8 = *pcVar6;
      pcVar6 = pcVar6 + 1;
      pcVar8 = pcVar8 + 1;
    }
    if (5 < iVar2) {
      iVar2 = 5;
      pcVar6 = (char *)&DAT_07db80d8;
      do {
        pcVar8 = pcVar6 + 0x108;
        pcVar6[0x104] = pcVar6[0x20c];
        uVar4 = 0xffffffff;
        pcVar7 = pcVar8;
        do {
          pcVar9 = pcVar7;
          if (uVar4 == 0) break;
          uVar4 = uVar4 - 1;
          pcVar9 = pcVar7 + 1;
          cVar1 = *pcVar7;
          pcVar7 = pcVar9;
        } while (cVar1 != '\0');
        uVar4 = ~uVar4;
        pcVar7 = pcVar9 + -uVar4;
        for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
          *(undefined4 *)pcVar6 = *(undefined4 *)pcVar7;
          pcVar7 = pcVar7 + 4;
          pcVar6 = pcVar6 + 4;
        }
        for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
          *pcVar6 = *pcVar7;
          pcVar7 = pcVar7 + 1;
          pcVar6 = pcVar6 + 1;
        }
        pcVar6 = pcVar8;
      } while ((int)pcVar8 < 0x7db8600);
    }
    param_1 = local_100;
    iVar3 = iVar2 * 0x108;
    (&DAT_07db81dc)[iVar3] = param_2;
  }
  DAT_07e11d9c = iVar2 + 1;
  uVar4 = 0xffffffff;
  do {
    pcVar6 = param_1;
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    pcVar6 = param_1 + 1;
    cVar1 = *param_1;
    param_1 = pcVar6;
  } while (cVar1 != '\0');
  uVar4 = ~uVar4;
  DAT_00559cdc = 300;
  pcVar6 = pcVar6 + -uVar4;
  pcVar8 = (char *)((int)&DAT_07db80d8 + iVar3);
  for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined4 *)pcVar8 = *(undefined4 *)pcVar6;
    pcVar6 = pcVar6 + 4;
    pcVar8 = pcVar8 + 4;
  }
  for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar8 = *pcVar6;
    pcVar6 = pcVar6 + 1;
    pcVar8 = pcVar8 + 1;
  }
  return;
#endif
}


// FUN_0047fce0 — Chat_DrawMessages
// Renders all system/global messages from DAT_07db80d8 in the 2D overlay.
// Iterates up to 6 slots starting at y=300, spacing 13 pixels.
// Colour: DAT_00559c78/80 set per-slot based on flag byte [0x104].
// Also measures text extent for cursor/scroll calculations.
void FUN_0047fce0(void)
{
  // 2026-05-04: AUTO-SKIP removed. DAT_07db80d8 ahora propiamente sized
  // (6 slots × 0x108). Reemplazo el bound literal `< 0x7db8708` con count
  // explícito de 6 iteraciones.
  int iVar1;
  LPCSTR lpString;
  int iVar2;
  longlong lVar3;
  tagSIZE *lpsz;
  LPSIZE *lpsz_00;
  tagSIZE local_8;

  FUN_00511680('\x01');
  SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca010);
  glColor3f(1.0f, 1.0f, 1.0f);
  iVar2 = 300;
  lpString = (LPCSTR)&DAT_07db80d8[0];
  for (int slot = 0; slot < 6; ++slot) {
    DAT_00559c80 = 0x80000000;
    if (lpString[0x104] == '\0') {
      DAT_00559c78 = ((4 < (int)DAT_07e11da0 % 10) - 1 & 0x81000000) - 0xaf3701;
    }
    else {
      DAT_00559c78 = -0x37009c;
    }
    lpsz = &local_8;
    iVar1 = lstrlenA(lpString);
    GetTextExtentPointA(DAT_055c9fec,lpString,iVar1,lpsz);
    iVar1 = iVar2;
    if (DAT_005590ac == 1) {
      iVar1 = iVar2 + -0x118;
    }
    FUN_0047f7a0(0x140 - ((uint)(local_8.cx * 0x280) / DAT_0056156c >> 1),iVar1,(char*)lpString,0,1,0);
    lpsz_00 = &lpsz_07e113d0;
    iVar1 = lstrlenA(lpString);
    GetTextExtentPointA(DAT_055c9fec,lpString,iVar1,(LPSIZE)lpsz_00);
    // IDA RenderNotices @0x47FCE0 hace exactamente esto acá.
    ScaleGlobalTextSize();   // era `__ftol()` sin args — ver helper arriba
    lpString = lpString + 0x108;
    iVar2 = iVar2 + 0xd;
  }
  DAT_07e11da0 = DAT_07e11da0 + 1;
  return;
}


// FUN_00480980 — UI_DrawPlayerList
// Draws the nearby-player list overlay (chat/notification area).
// Each slot entry: player name, colour-coded by channel type (0-5).
// Flashes the line if it matches the local player name + cursor mod.
// Layout: 5 visible lines at y=0..0x4d, spacing 13 pixels.
void FUN_00480980(void)
{
  byte bVar1;
  undefined4 uVar2;
  int iVar3;
  byte *pbVar4;
  int iVar5;
  byte *pbVar6;
  int iVar7;
  bool bVar8;
  longlong lVar9;
  tagSIZE *lpsz;
  LPSIZE *lpsz_00;
  int local_10c;
  tagSIZE local_108;
  CHAR local_100 [256];

  // BUG-FIX: 0x3f800000 son los bits de 1.0f. Como int → 1065353216.0f.
  glColor3f(1.0f, 1.0f, 1.0f);
  SelectObject(DAT_055c9fec,(HGDIOBJ)(uintptr_t)DAT_055ca00c);
  iVar7 = 0;
  local_10c = 0;
  do {
    iVar5 = DAT_00559ce0 + local_10c;
    // BUG-FIX Ghidra: stride real del ring buffer = 280 bytes = 0x118 (confirmado
    // vs IDA sub_480980 y UIChatLogWindow_AddText). Ghidra decompiló el acceso
    // byte con *0x46 (= 70) porque el dword_7DF948C está tipado int[] — los
    // accesos DWORD con índice 0x46 sí dan 280 bytes, pero los accesos byte
    // necesitan *0x118.  Sin este fix, el renderer lee names/msgs del slot
    // equivocado (0, 70, 140, ...) en vez de (0, 280, 560, ...) y el texto
    // nunca aparece en pantalla (lo que pasaba con el countdown de Exit).
    pbVar6 = (byte *)(DAT_07df9380 + iVar5 * 0x118);
    if ((*(char *)(&DAT_07df938b + iVar5 * 0x118) != '\0') &&
       ((DAT_00559bf1 != '\0' || ((&DAT_07df948c)[iVar5 * 0x46] != 3)))) {
      switch((&DAT_07df948c)[iVar5 * 0x46]) {
      case 0:
        DAT_00559c78 = 0xff000000;
        DAT_00559c80 = 0x9632c8ff;
        break;
      case 1:
        DAT_00559c78 = 0xffff9664;
        DAT_00559c80 = 0x96000000;
        break;
      case 2:
        DAT_00559c78 = 0xff001eff;
        DAT_00559c80 = 0x96000000;
        break;
      case 3:
        DAT_00559c78 = 0xffc8dce6;
        DAT_00559c80 = 0x96000000;
        break;
      case 4:
        DAT_00559c78 = 0xff000000;
        DAT_00559c80 = 0x96ffc800;
        break;
      case 5:
        DAT_00559c78 = 0xff000000;
        DAT_00559c80 = 0xc896ff00;
      }
      // BUG-FIX Ghidra: perdió los varargs de sprintf.  Port exacto de IDA:
      //   name  @ DAT_07df9380 + slot*280       (offset 0)
      //   msg   @ DAT_07df9380 + slot*280 + 11  (offset 0x0B)
      //   - Si los primeros 2 bytes del slot son 0x20 0x20 → "%s%s"  (sin dos puntos)
      //   - Si name[0] != 0                               → "%s: %s"
      //   - Si name[0] == 0                               → "%s"    (solo msg)
      // Las DAT_00559d4c/54/5c originales eran estos mismos formatos pero en
      // el binario aparecen como strings separados; en nuestro build están
      // declaradas como empty-string (char=0), así que usamos literales.
      {
        const char *name_ptr = (const char *)(DAT_07df9380 + iVar5 * 0x118);
        const char *msg_ptr  = (const char *)(DAT_07df9380 + iVar5 * 0x118 + 0x0B);
        if (name_ptr[0] == ' ' && name_ptr[1] == ' ') {
          crt_sprintf(local_100, "%s%s", name_ptr, msg_ptr);
        } else if (name_ptr[0] == 0) {
          crt_sprintf(local_100, "%s", msg_ptr);
        } else {
          crt_sprintf(local_100, "%s: %s", name_ptr, msg_ptr);
        }
      }
      lpsz = &local_108;
      iVar3 = lstrlenA(local_100);
      GetTextExtentPointA(DAT_055c9fec,local_100,iVar3,lpsz);
      iVar3 = DAT_083a427c;
      bVar8 = -1 < DAT_083a427c;
      // BUG-FIX pointer-arith: &DAT_07df9494 ahora es int* (alias a offset
      // 0x114 del buffer), sumarle iVar5*0x118 como int* avanzaría 4x. Casteo
      // a char* antes de sumar el stride-en-bytes 0x118 para que el LONG
      // aterrice en el slot correcto.
      *(LONG *)((char *)&DAT_07df9494 + iVar5 * 0x118) = local_108.cx;
      uVar2 = DAT_00559c80;
      if (((((bVar8) && (iVar3 < (int)((uint)(local_108.cx * 0x280) / DAT_0056156c))) &&
           (iVar7 <= DAT_083a4278)) && ((DAT_083a4278 < iVar7 + 0xd && (DAT_00559c84 != '\0')))) &&
         (*(char *)(DAT_07abf5d8 + 0x34e) != '\0')) {
        pbVar4 = (byte *)(DAT_07abf5d8 + 0x1c1);
        do {
          bVar1 = *pbVar6;
          bVar8 = bVar1 < *pbVar4;
          if (bVar1 != *pbVar4) {
LAB_00480b89:
            iVar5 = (1 - (uint)bVar8) - (uint)(bVar8 != 0);
            goto LAB_00480b8e;
          }
          if (bVar1 == 0) break;
          bVar1 = pbVar6[1];
          bVar8 = bVar1 < pbVar4[1];
          if (bVar1 != pbVar4[1]) goto LAB_00480b89;
          pbVar6 = pbVar6 + 2;
          pbVar4 = pbVar4 + 2;
        } while (bVar1 != 0);
        iVar5 = 0;
LAB_00480b8e:
        if ((iVar5 != 0) && (DAT_07e11da8 % 6 < 3)) {
          DAT_00559c80 = DAT_00559c78;
          DAT_00559c78 = uVar2;
        }
      }
      FUN_0047f7a0(0,iVar7,local_100,0,1,0);
      lpsz_00 = &lpsz_07e113d0;
      iVar5 = lstrlenA(local_100);
      GetTextExtentPointA(DAT_055c9fec,local_100,iVar5,(LPSIZE)lpsz_00);
      ScaleGlobalTextSize();   // era `__ftol()` sin args — ver helper arriba
    }
    iVar7 = iVar7 + 0xd;
    local_10c = local_10c + 1;
    if (0x4d < iVar7) {
      DAT_07e11da8 = DAT_07e11da8 + 1;
      return;
    }
  } while( true );
}


// FUN_004821a0 — UI_TickTooltips
// Ticks floating tooltip/bubble widgets stored in the DAT_07e01720 ring.
// Each entry: timers (3 ints), target entity ptr, screen XYWH (4 ints).
// Decrements timers; clears timer if entity is dead/inactive.
// On mouse-over (cursor inside widget XYWH + hover flag set + entity is local
// player's target): copies entity name into DAT_07db8810, calls FUN_00404bc0
// to trigger a UI sound.
void FUN_004821a0(void)
{
  char cVar1;
  byte bVar2;
  byte *pbVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  int *piVar7;
  byte *pbVar8;
  char *pcVar9;
  bool bVar10;

  // BUG-FIX 2026-04-28: pool real = DAT_07e01720[26 × 0x254].
  // Bounds end calculados a partir del array, no de la dirección absoluta.
  piVar7 = (int*)DAT_07e01720;
  const uintptr_t poolEnd = (uintptr_t)DAT_07e016f8 + sizeof(DAT_07e016f8);
  do {
    if (0 < *piVar7) {
      *piVar7 = *piVar7 + -1;
    }
    if (0 < piVar7[0x81]) {
      piVar7[0x81] = piVar7[0x81] + -1;
    }
    if (0 < piVar7[0x82]) {
      piVar7[0x82] = piVar7[0x82] + -1;
    }
    pcVar9 = (char *)piVar7[0x83];
    if ((pcVar9 != (char *)0x0) && ((*pcVar9 == '\0' || (pcVar9[0x160] == '\0')))) {
      *piVar7 = 0;
      piVar7[0x81] = 0;
      piVar7[0x82] = 0;
    }
    if ((piVar7[0x84] <= DAT_083a427c) &&
       (DAT_083a427c < (int)((uint)(piVar7[0x86] * 0x280) / DAT_0056156c + piVar7[0x84]))) {
      if ((piVar7[0x85] <= DAT_083a4278) &&
         (((DAT_083a4278 < (int)((uint)(piVar7[0x87] * 0x1e0) / DAT_00561570 + piVar7[0x85]) &&
           (DAT_00559c84 != '\0')) && (*(char *)(DAT_07abf5d8 + 0x34e) != '\0')))) {
        pbVar8 = (byte *)(DAT_07abf5d8 + 0x1c1);
        pbVar3 = (byte *)(piVar7 + -10);
        do {
          bVar2 = *pbVar3;
          bVar10 = bVar2 < *pbVar8;
          if (bVar2 != *pbVar8) {
LAB_004822b1:
            iVar4 = (1 - (uint)bVar10) - (uint)(bVar10 != 0);
            goto LAB_004822b6;
          }
          if (bVar2 == 0) break;
          bVar2 = pbVar3[1];
          bVar10 = bVar2 < pbVar8[1];
          if (bVar2 != pbVar8[1]) goto LAB_004822b1;
          pbVar3 = pbVar3 + 2;
          pbVar8 = pbVar8 + 2;
        } while (bVar2 != 0);
        iVar4 = 0;
LAB_004822b6:
        if ((iVar4 != 0) && (DAT_083a42d0 != '\0')) {
          uVar5 = 0xffffffff;
          pbVar3 = (byte *)(piVar7 + -10);
          do {
            pbVar8 = pbVar3;
            if (uVar5 == 0) break;
            uVar5 = uVar5 - 1;
            pbVar8 = pbVar3 + 1;
            bVar2 = *pbVar3;
            pbVar3 = pbVar8;
          } while (bVar2 != 0);
          uVar5 = ~uVar5;
          pbVar3 = pbVar8 + -uVar5;
          pbVar8 = (byte *)DAT_07db8810;
          for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
            *(undefined4 *)pbVar8 = *(undefined4 *)pbVar3;
            pbVar3 = pbVar3 + 4;
            pbVar8 = pbVar8 + 4;
          }
          for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
            *pbVar8 = *pbVar3;
            pbVar3 = pbVar3 + 1;
            pbVar8 = pbVar8 + 1;
          }
          uVar5 = 0xffffffff;
          pcVar9 = (char *)DAT_07db8810;
          do {
            if (uVar5 == 0) break;
            uVar5 = uVar5 - 1;
            cVar1 = *pcVar9;
            pcVar9 = pcVar9 + 1;
          } while (cVar1 != '\0');
          DAT_07d780ac = ~uVar5 - 1;
          DAT_083a42d0 = '\0';
          FUN_00404bc0(0x19,0,0);
        }
      }
    }
    piVar7 = piVar7 + 0x95;
    if ((uintptr_t)piVar7 >= poolEnd) {
      return;
    }
  } while( true );
}


// FUN_00513570 — Chat_ValidateCommand
// Checks if the text in DAT_07db8710 is a known chat command or valid chat text.
// Strips spaces, then looks up in two command tables (DAT_07d27610, DAT_07d73104).
// Also rejects strings matching hard-coded blocked strings (webzen brand names).
// Returns 1 (non-zero low byte) if the text should be blocked/consumed.
char FUN_00513570(void)
{
  char cVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  char *pcVar5;
  char local_100 [256];

  uVar2 = 0xffffffff;
  iVar3 = 0;
  iVar4 = 0;
  pcVar5 = (char *)DAT_07db8710;
  do {
    if (uVar2 == 0) break;
    uVar2 = uVar2 - 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
  } while (cVar1 != '\0');
  if (0 < (int)(~uVar2 - 1)) {
    do {
      if (*(char *)((int)DAT_07db8710 + iVar3) != ' ') {
        local_100[iVar4] = *(char *)((int)DAT_07db8710 + iVar3);
        iVar4 = iVar4 + 1;
      }
      uVar2 = 0xffffffff;
      iVar3 = iVar3 + 1;
      pcVar5 = (char *)DAT_07db8710;
      do {
        if (uVar2 == 0) break;
        uVar2 = uVar2 - 1;
        cVar1 = *pcVar5;
        pcVar5 = pcVar5 + 1;
      } while (cVar1 != '\0');
    } while (iVar3 < (int)(~uVar2 - 1));
  }
  local_100[iVar4] = '\0';
  iVar3 = 0;
  if (0 < DAT_07d78074) {
    pcVar5 = DAT_07d27610;
    do {
      uVar2 = FUN_004977f0(local_100,pcVar5,'\0');
      if ((char)uVar2 != '\0') goto LAB_00513792;
      iVar3 = iVar3 + 1;
      pcVar5 = pcVar5 + 0x14;
    } while (iVar3 < DAT_07d78074);
  }
  if ((char)DAT_07db8710 != '/') {
    uVar2 = 0xffffffff;
    iVar3 = 0;
    iVar4 = 0;
    pcVar5 = (char *)DAT_07db8710;
    do {
      if (uVar2 == 0) break;
      uVar2 = uVar2 - 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar5 + 1;
    } while (cVar1 != '\0');
    if (0 < (int)(~uVar2 - 1)) {
      do {
        if (*(char *)((int)DAT_07db8710 + iVar3) != ' ') {
          local_100[iVar4] = *(char *)((int)DAT_07db8710 + iVar3);
          iVar4 = iVar4 + 1;
        }
        uVar2 = 0xffffffff;
        iVar3 = iVar3 + 1;
        pcVar5 = (char *)DAT_07db8710;
        do {
          if (uVar2 == 0) break;
          uVar2 = uVar2 - 1;
          cVar1 = *pcVar5;
          pcVar5 = pcVar5 + 1;
        } while (cVar1 != '\0');
      } while (iVar3 < (int)(~uVar2 - 1));
    }
    local_100[iVar4] = '\0';
    iVar3 = 0;
    if (0 < DAT_07d78070) {
      pcVar5 = DAT_07d73104;
      do {
        uVar2 = FUN_004977f0(local_100,pcVar5,'\0');
        if ((char)uVar2 != '\0') goto LAB_00513792;
        iVar3 = iVar3 + 1;
        pcVar5 = pcVar5 + 0x14;
      } while (iVar3 < DAT_07d78070);
    }
  }
  uVar2 = FUN_004977f0((char *)DAT_07db8710,&DAT_00561740,'\0');
  if ((char)uVar2 == '\0') {
    uVar2 = FUN_004977f0((char *)DAT_07db8710,&DAT_00561744,'\0');
    if ((char)uVar2 == '\0') {
      uVar2 = FUN_004977f0((char *)DAT_07db8710,&DAT_00561748,'\0');
      if ((char)uVar2 == '\0') {
        uVar2 = FUN_004977f0((char *)DAT_07db8710,&DAT_0056174c,'\0');
        if ((char)uVar2 == '\0') {
          uVar2 = FUN_004977f0((char *)DAT_07db8710,&DAT_00561750,'\0');
          if ((char)uVar2 == '\0') {
            uVar2 = FUN_004977f0((char *)DAT_07db8710,s_Webzen_00561754,'\0');
            if ((char)uVar2 == '\0') {
              uVar2 = FUN_004977f0((char *)DAT_07db8710,s_WebZen_0056175c,'\0');
              if ((char)uVar2 == '\0') {
                uVar2 = FUN_004977f0((char *)DAT_07db8710,s_Webzen_00561764,'\0');
                if ((char)uVar2 == '\0') {
                  uVar2 = FUN_004977f0((char *)DAT_07db8710,s_WEBZEN_0056176c,'\0');
                  if ((char)uVar2 == '\0') {
                    uVar2 = FUN_004977f0((char *)DAT_07db8710,&DAT_07d4b4b0,'\0');
                    if ((char)uVar2 == '\0') {
                      uVar2 = FUN_004977f0((char *)DAT_07db8710,&DAT_07d4b5dc,'\0');
                      if ((char)uVar2 == '\0') {
                        return uVar2;
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
LAB_00513792:
  return CONCAT31((uVar2 >> 8),1);
}
