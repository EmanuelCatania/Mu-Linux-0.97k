// CharMenu_Build.cpp — FUN_004c3530 @ 0x004c3530
// Character info / stats menu builder.  Dispatches on DAT_07e11d20 (mode 1/2/3).
//
// Populates a string list buffer (lpString_07e90798, 100 bytes/entry, ~30 slots)
// with formatted text for the character menu, then calls FUN_004c2420 to display it.
//
// ── Mode 1: class-list type A ─────────────────────────────────────────────────
//   Writes header (DAT_0055a408), subheader (DAT_0055a40c).
//   Iterates DAT_07d32af0 (stride 300, limit 0x7d34134) — class info list A.
//   Writes footer (DAT_0055a410).
//   Calls FUN_004c2420(1,1,count,0,2,1).
//
// ── Mode 2: class-list type B ─────────────────────────────────────────────────
//   Same structure with DAT_07d34260 (limit 0x7d358a4) and strings DAT_0055a414/418/41c.
//
// ── Mode 3: class stats detail ────────────────────────────────────────────────
//   Layout parameters by resolution (DAT_0056156c):
//     0x280(640)  → col_w=0x5a, pad=0x34
//     800         → col_w=0x5a, pad=0x2f
//     0x400(1024) → col_w=0x67, pad=0x28
//     0x500(1280) → col_w=0x7b, pad=0x20
//
//   Class-id ranges in DAT_07e11d24:
//     0x000..0x09F → type 1 (max 0x870 xp)
//     0x0A0..0x0BF → type 2 (max 900)
//     0x0C0..0x0DF → type 3 (max 0x708)
//     0x0E0..0x17F → type 4 (max 3000)
//     0x1E0..0x1FF → type 5 (no xp bar)
//
//   Computes local_1c = max_xp / col_width (horizontal scale for progress bar).
//   Calls FUN_004c2e20(class_id) to prepare class data.
//   Builds string slots: class name, subtype header, padding rows, then calls
//   FUN_004c2880(class_data_ptr) for the detail block.
//   Draws stat rows via FUN_004c2d50 / FUN_004c2c10 conditionally on stat flags
//   (DAT_07e91530/534/53c/540) and class-id range.
//
// After building: sets DAT_07e11d6e = 1 (dirty flag → triggers re-render).

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// String table aliases for readability
#define s_ChMenu_HdrA  DAT_0055a408
#define s_ChMenu_SubA  DAT_0055a40c
#define s_ChMenu_FtrA  DAT_0055a410
#define s_ChMenu_HdrB  DAT_0055a414
#define s_ChMenu_SubB  DAT_0055a418
#define s_ChMenu_FtrB  DAT_0055a41c
#define s_ChMenu_HdrC  DAT_0055a420
#define s_ChMenu_SubC1 DAT_0055a424
#define s_ChMenu_SubC2 DAT_0055a428
#define s_ChMenu_SubC3 DAT_0055a42c
#define s_ChMenu_SubC4 DAT_0055a430
#define s_ChMenu_FtrC  DAT_0055a434

// ─────────────────────────────────────────────────────────────────────────────

// Helper: strcpy-length into lpString_07e90798[slot] using manual word-copy loop.
// (Ghidra emits length + word-copy idiom for all string copies here.)
static void slot_strcpy(int slot, const char *src)
{
    char *dst = lpString_07e90798 + slot * 100;
    // determine source length
    int len = 0; while (src[len]) len++; len++;   // include NUL
    const char *s = src;
    char *d = dst;
    for (unsigned u = (unsigned)len >> 2; u; u--, s+=4, d+=4)
        *(unsigned int *)d = *(unsigned int *)s;
    for (unsigned u = (unsigned)len & 3; u; u--)
        *d++ = *s++;
}

// ─────────────────────────────────────────────────────────────────────────────

void FUN_004c3530(void)
{
    // ── Mode 1: build class-list A ────────────────────────────────────────────
    if (DAT_07e11d20 == 1)
    {
        FUN_00511600();
        DAT_07eaa154 = 0;

        // Slot 0: header line
        crt_sprintf(lpString_07e90798, s_ChMenu_HdrA);
        int iVar4 = DAT_07eaa154 + 1;
        DAT_07e91708[iVar4] = 1;
        DAT_07ea7b10[iVar4] = 1;
        slot_strcpy(iVar4, &DAT_07d329c4);
        DAT_07eaa154 = iVar4 + 1;   // +2

        // Slot 2 (iVar4): subheader
        crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_SubA);
        DAT_07eaa154++;

        // Class entries from list A (stride 300, limit 0x7d34134)
        char *pcVar5 = &DAT_07d32af0;
        char *local_c = lpString_07e90798 + DAT_07eaa154 * 100;
        while (pcVar5 != nullptr && (int)pcVar5 < 0x7d34134) {
            DAT_07e91708[DAT_07eaa154] = 0;
            DAT_07ea7b10[DAT_07eaa154] = 0;
            slot_strcpy(DAT_07eaa154, pcVar5);
            DAT_07eaa154++;
            pcVar5 += 300;
            local_c += 100;
        }

        // Footer
        crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_FtrA);
        DAT_07eaa154++;
        DAT_07e11d6e = 1;
        FUN_004c2420(1, 1, DAT_07eaa154, 0, 2, 1);
        return;
    }

    // ── Mode 2: build class-list B ────────────────────────────────────────────
    if (DAT_07e11d20 == 2)
    {
        FUN_00511600();
        DAT_07eaa154 = 0;

        crt_sprintf(lpString_07e90798, s_ChMenu_HdrB);
        int iVar4 = DAT_07eaa154 + 1;
        DAT_07e91708[iVar4] = 1;
        DAT_07ea7b10[iVar4] = 1;
        slot_strcpy(iVar4, &DAT_07d34134);
        DAT_07eaa154 = iVar4 + 1;   // +2

        crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_SubB);
        DAT_07eaa154++;

        char *pcVar5 = &DAT_07d34260;
        char *local_c = lpString_07e90798 + DAT_07eaa154 * 100;
        while (pcVar5 != nullptr && (int)pcVar5 < 0x7d358a4) {
            DAT_07e91708[DAT_07eaa154] = 0;
            DAT_07ea7b10[DAT_07eaa154] = 0;
            slot_strcpy(DAT_07eaa154, pcVar5);
            DAT_07eaa154++;
            pcVar5 += 300;
            local_c += 100;
        }

        crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_FtrB);
        DAT_07eaa154++;
        DAT_07e11d6e = 1;
        FUN_004c2420(1, 1, DAT_07eaa154, 0, 2, 1);
        return;
    }

    // ── Mode 3: class stats detail ────────────────────────────────────────────
    if (DAT_07e11d20 != 3) return;

    FUN_00511600();

    // Layout parameters by screen resolution
    int col_w  = 0;    // column width (iVar4 / iVar7 in Ghidra)
    int pad    = 0;    // padding (local_14)
    int col_w2 = 0;    // second col width (local_18)
    int pad2   = 0;    // second pad (local_c as int)

    if (DAT_0056156c < 0x401) {
        if      (DAT_0056156c == 0x400) { col_w=0x67; pad=0x1c; col_w2=0x67; pad2=0x28; }
        else if (DAT_0056156c == 0x280) { col_w=0x5a; pad=0x26; col_w2=0x5a; pad2=0x34; }
        else if (DAT_0056156c == 800  ) { col_w=0x5a; pad=0x21; col_w2=0x5a; pad2=0x2f; }
    } else if (DAT_0056156c == 0x500)   { col_w=0x7b; pad=0x16; col_w2=0x7b; pad2=0x20; }

    // Determine class type from DAT_07e11d24 (class ID)
    int iVar7  = 0;      // class type (1..5)
    unsigned int uVar2 = 0;   // xp max constant
    int local_10 = 0;

    int cls = (int)DAT_07e11d24;
    if      (cls < 0xa0)                        { iVar7=1; uVar2=0x870;  local_10=1; }
    else if (cls < 0xc0)                        { iVar7=2; uVar2=900;    local_10=2; }
    else if (cls < 0xe0)                        { iVar7=3; uVar2=0x708;  local_10=3; }
    else if (cls < 0x180)                       { iVar7=4; uVar2=3000;   local_10=4; }
    else if (cls >= 0x1e0 && cls <= 0x1ff)     { iVar7=5; uVar2=0x1734; local_10=5;
        if (DAT_0056156c == 800 || DAT_0056156c == 0x400) uVar2 = 0x1450; }
    else { DAT_07e11d20 = 0; return; }

    int local_1c = (col_w > 0) ? (int)((unsigned long long)uVar2 / (unsigned long long)(long long)col_w) : 0;

    // Extra slot count for class type 5
    int local_8 = (iVar7 == 5) ? 0 : 0xb;

    // Pointer to class data: DAT_07d78068[class_id * 0x40]
    int iVar4 = (int)DAT_07e11d24 * 0x40 + DAT_07d78068;

    FUN_004c2e20(DAT_07e11d24);
    DAT_07eaa154 = 0;

    // Header slot
    crt_sprintf(lpString_07e90798, s_ChMenu_HdrC);
    int iVar7b = DAT_07eaa154 + 2;
    DAT_07e91708[DAT_07eaa154 + 1] = 1;
    DAT_07ea7b10[DAT_07eaa154 + 1] = 1;
    slot_strcpy(DAT_07eaa154 + 1, &DAT_07d358a4);
    DAT_07e91708[iVar7b] = 1;
    DAT_07ea7b10[iVar7b] = 1;

    crt_sprintf(lpString_07e90798 + iVar7b * 100, s_ChMenu_SubC1);
    DAT_07e91708[DAT_07eaa154] = 0;
    DAT_07ea7b10[DAT_07eaa154] = 1;
    DAT_07eaa154 = iVar7b + 1;

    crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_SubC2);
    DAT_07eaa154++;
    crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_SubC3);
    DAT_07eaa154++;
    crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_SubC4);
    int iVar7c = DAT_07eaa154 + 1;
    DAT_07e11d6e = 1;

    // Padding row (spaces, width adjusted for resolution)
    unsigned int uVar3 = (DAT_0056156c > 800) ? 0x2e + 5 : 0x2e;
    char *pPad = lpString_07e90798 + iVar7c * 100;
    unsigned int *pu = (unsigned int *)pPad;
    for (unsigned u = uVar3 >> 2; u; u--) *pu++ = 0x20202020;
    DAT_07e91708[iVar7c] = 0;
    for (unsigned u = uVar3 & 3; u; u--) *(unsigned char *)pu++ = 0x20;
    DAT_07ea7b10[iVar7c] = 0;
    DAT_07eaa154 += 2;
    pPad[uVar3] = 0;

    // Extra blank rows for type ≠ 5
    if (local_8 > 0) {
        unsigned char *pu10 = (unsigned char *)(DAT_07ea7b10 + DAT_07eaa154);
        for (char *p2 = (char *)local_8; p2; p2--) { *pu10 = 0; pu10 += 4; }
        char *pb = &lpString_07e90798[0] + DAT_07eaa154 * 100 + 1;
        while (local_8 > 0) {
            pb[-1] = 0x20;
            pb[0]  = 0;
            DAT_07eaa154++;
            pb += 100;
            local_8--;
            DAT_07e91708[DAT_07eaa154] = 0;
        }
    }

    // Class detail block
    FUN_004c2880(iVar4);

    // Footer
    crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, s_ChMenu_FtrC);
    DAT_07eaa154++;
    FUN_004c2420(1, 1, DAT_07eaa154, col_w2, 2, 1);
    FUN_00511680('\x01');

    // Stat rows
    DAT_07eaa154 = 0;
    FUN_004c2d50(0, local_1c, pad);
    FUN_004c2c10(0, (unsigned char *)0x0055a440, &local_1c,
                 (const char *)0x0055a438, pad2, local_10);

    if (DAT_07e91530 > 0 && !(cls >= 0x1e0 && cls <= 0x1ff)) {
        FUN_004c2c10(2, (unsigned char *)0x0055a448, &local_1c,
                     (const char *)0x0055a444, pad2, 0);
        FUN_004c2d50(2, local_1c, pad);
        FUN_004c2c10(0, (unsigned char *)0x0055a450, &local_1c,
                     (const char *)0x0055a44c, pad2, 0);
    }
    if (DAT_07e91534 > 0 && !(cls >= 0x1e0 && cls <= 0x1ff)) {
        FUN_004c2c10(3, (unsigned char *)0x0055a45c, &local_1c,
                     (const char *)0x0055a454, pad2, 0);
    }
    if (cls >= 0xa0 && cls < 0xc0) {
        FUN_004c2d50(4, local_1c, pad);
        FUN_004c2c10(4, (unsigned char *)0x0055a468, &local_1c,
                     (const char *)0x0055a460, pad2, 0);
    }
    if (DAT_07e9153c > 0) {
        FUN_004c2d50(5, local_1c, pad);
        FUN_004c2c10(5, (unsigned char *)0x0055a478, &local_1c,
                     (const char *)0x0055a470, pad2, 0);
    }
    if (DAT_07e91540 > 0) {
        FUN_004c2d50(6, local_1c, pad);
        FUN_004c2c10(6, (unsigned char *)0x0055a484, &local_1c,
                     (const char *)0x0055a47c, pad2, 0);
    }
    if (!(cls >= 0x1e0 && cls <= 0x1ff)) {
        FUN_004c2d50(7, local_1c, pad);
        FUN_004c2c10(7, (unsigned char *)0x0055a494, &local_1c,
                     (const char *)0x0055a48c, pad2, 0);
    }
    if (!(cls >= 0x1e0 && cls <= 0x1ff)) {
        FUN_004c2d50(8, local_1c, pad);
        FUN_004c2c10(8, (unsigned char *)0x0055a4a0, &local_1c,
                     (const char *)0x0055a498, pad2, 0);
    }
    if (cls >= 0x1e0 && cls <= 0x1ff) {
        FUN_004c2d50(9, local_1c, pad);
        FUN_004c2c10(9, (unsigned char *)0x0055a4ac, &local_1c,
                     (const char *)0x0055a4a4, pad2, local_10);
    }
    FUN_00511600();
}


// ─────────────────────────────────────────────────────────────────────────────
// FUN_004c2420 @ 0x004c2420 — DrawItemInfoBox(x, y, count, fixedWidth, iSort, drawBox)
//
// Port fiel, verificado sobre el desensamblado 0x004c2420..0x004c285e.  Es la
// MISMA rutina que usan el menú de personaje (RenderHelpWindow @0x004c3530,
// FUN_004c2c10, FUN_004c2d50), el tooltip de ítem (RenderItemInfo @0x004c4650)
// y el de reparación (RenderRepairInfo @0x004c8d70): dibuja el recuadro y la
// lista de líneas de lpString_07e90798.
//
//   param_1 = x del CENTRO del recuadro       param_4 = ancho fijo (0 = automático)
//   param_2 = y del borde superior            param_5 = iSort (2 = centrado)
//   param_3 = cantidad de líneas              param_6 = 1 → dibuja el recuadro
//
// Globales:
//   lpString_07e90798 (0x07E90798) líneas, stride 100 — corta en la 1ª vacía
//   DAT_07e91708      (0x07E91708) TextListColor, color por línea
//   DAT_07ea7b10      (0x07EA7B10) TextBold, negrita por línea
//   m_hFontDC         (0x055C9FEC) DC de medición
//   DAT_055ca00c/010  g_hFont / g_hFontBold
//   _DAT_055c9b70/74  g_fScreenRate_x / g_fScreenRate_y
//   DAT_0056156c      WindowWidth
//   m_dwBackColor     (0x00559C80) 0xff0000a0 SOLO para el color 5, si no 0
//
// Constantes mágicas (leídas del binario):
//   0x00552504 = 0.5   0x0055256c = 1.0   0x0055264c = 2.0   0x00552650 = 4.0
//   0x00552ae8 = 0.9090909   0x005529b4 = 1.1
//
// Colores — glColor3f, jump table en 0x004c2860:
//   0 y 5 → (1,1,1)     1 → (0.5,0.7,1.0)   2 → (1.0,0.2,0.1)
//   3 → (1.0,0.8,0.1)   4 → (0.1,1.0,0.5)   6 → (1.0,0.1,1.0)
//   >6 → cae al default SIN tocar glColor (conserva el color de la línea previa)

// Los siete destinos del switch, en el orden de la jump table de 0x004c2860.
static const float DrawItemInfoBox_glColor[7][3] = {
    { 1.0f, 1.0f, 1.0f },   // 0 → 0x004c272b
    { 0.5f, 0.7f, 1.0f },   // 1 → 0x004c2737
    { 1.0f, 0.2f, 0.1f },   // 2 → 0x004c2748
    { 1.0f, 0.8f, 0.1f },   // 3 → 0x004c2754
    { 0.1f, 1.0f, 0.5f },   // 4 → 0x004c2760
    { 1.0f, 1.0f, 1.0f },   // 5 → 0x004c272b (comparte destino con el 0)
    { 1.0f, 0.1f, 1.0f },   // 6 → 0x004c2771
};

// DESVIACIÓN CONSCIENTE: en el binario el color de texto llega por glColor3f
// porque el subclass de CUIRenderText sube el glifo como textura y la MODULA
// con el color actual de GL.  Nuestro FUN_0040f610 pinta glifos con
// wglUseFontBitmaps y toma el color de m_dwTextColor (0x00559C78, formato ABGR
// 0xAABBGGRR — ver la nota de CUIRenderText_BakeTextTexture @0x0040FCD0).
// Emitimos los dos: el glColor3f fiel y el ABGR equivalente.
static const DWORD DrawItemInfoBox_TextColor[7] = {
    0xffffffff,   // 0  (1.0,1.0,1.0)
    0xffffb380,   // 1  (0.5,0.7,1.0)
    0xff1a33ff,   // 2  (1.0,0.2,0.1)
    0xff1accff,   // 3  (1.0,0.8,0.1)
    0xff80ff1a,   // 4  (0.1,1.0,0.5)
    0xffffffff,   // 5  (1.0,1.0,1.0)
    0xffff1aff,   // 6  (1.0,0.1,1.0)
};

// FUN_0040fb70 @ 0x0040FB70 — RenderText del subclass de CUIRenderText, al que
// llega DrawItemInfoBox vía el dispatcher 0x0040F610.  Portamos la parte que
// define el layout: el offset de alineación (iSort) y el AVANCE VERTICAL que
// devuelve, que es lo que hace que cada línea quede donde va.
//
//   iSort 1 → izquierda con ancho fijo   2 → centrado   3 → derecha
//   retorna (cy / g_fScreenRate_y) / (text[0]=='\n' ? 2.0 : 1.0)
//
// DESVIACIÓN: el original rasteriza la línea a una textura de iBoxWidth px con
// TextOutA desplazado fVar4 px dentro de ella (0x0040FCD0).  Nosotros pintamos
// glifos directo en unidades del ortho, así que el desplazamiento se aplica
// sobre la x, convertido de píxeles a ortho con Text_GetOrthoScaleX().
// OJO (armadilla 1 de CLAUDE.md): stubs_bulk_misc.cpp ya define un
// `FUN_0040fb70` __fastcall que es un stub vacio (return 0.0f) y no lo llama
// nadie.  Para no crear dos simbolos con el mismo nombre y distinta firma,
// esta copia lleva otro nombre; el canonico va en el comentario.
// 2026-08-18 — FIX del ancho del recuadro.
//
// Este archivo convertia anchos de texto con g_fScreenRate_x, copiando la
// formula de IDA. En el binario eso es correcto porque su CUIRenderText recibe
// un ancho de referencia (640) y reescala la x internamente. NUESTRO stack de
// texto no hace eso: FUN_0040f610 dibuja los glifos en unidades del ortho,
// convirtiendo con viewport/ortho (Text_PixelToOrthoScale).
//
// Al mezclar los dos factores, la CAJA quedaba dimensionada con un divisor y el
// TEXTO dibujado con otro: con 788 px de ancho la caja salia a 640/788 = 81%
// del texto y las lineas largas se desbordaban por la derecha.
//
// La altura no tenia el problema porque usa _DAT_055c9b74 en los dos lados
// (caja y avance por linea), asi que el factor se cancela.
//
// Es el mismo desvio ya documentado en HUD_Pass4.cpp:512 para el caret del
// input. Usamos la escala real del pipeline en todo lo que convierta anchos de
// TEXTO entre pixeles y layout.
extern "C" float Text_GetOrthoScaleX(void);   // src/stubs_externs.cpp

static float RenderText_0040fb70(int iPos_x, int iPos_y, const char *pszText,
                          int iBoxWidth, int iSort, int iMaxWidth)
{
    SIZE  local_8;
    float fVar4;
    int   iWidth;

    if ((pszText == NULL) || (*pszText == '\0')) {
        return 0.0f;
    }
    local_8.cx = 0;
    local_8.cy = 0;
    GetTextExtentPointA(m_hFontDC, pszText, lstrlenA(pszText), &local_8);
    fVar4  = 0.0f;
    iWidth = local_8.cx;
    if (iSort == 1) {
        if (0 < iBoxWidth) {
            iWidth = iBoxWidth;
        }
    }
    else if (iSort == 2) {
        fVar4  = (float)((iBoxWidth - local_8.cx) / 2);
        iWidth = local_8.cx + (int)fVar4 * 2;
    }
    else if (iSort == 3) {
        fVar4  = (float)(iBoxWidth - local_8.cx);
        iWidth = local_8.cx + (int)fVar4;
    }
    const float fTexScaleX = Text_GetOrthoScaleX();
    if ((float)iMaxWidth < (float)iPos_x + (float)iWidth / fTexScaleX) {
        iPos_x = (int)((float)iMaxWidth - (float)iWidth / fTexScaleX);
    }
    // Fondo de la linea (m_dwBackColor).  En el binario este nivel NO lo pinta:
    // CUIRenderText_BakeTextTexture (0x0040FCD0) rasteriza la linea a una
    // textura de ancho iBoxWidth — NO del ancho del texto (`iStack_260 =
    // param_2; if (param_2 == 0) iStack_260 = sz.cx;`) — y FUN_004105f0 rellena
    // con m_dwBackColor todo pixel que no sea glifo.  Por eso la franja del
    // color 5 (clase requerida) va de punta a punta de la caja.
    //
    // DESVIACIÓN: nuestro FUN_0040f610 pinta el fondo solo detras del texto, y
    // no recibe el ancho del box.  Emitimos la franja aca con el ancho
    // correcto y le sacamos el fondo al render de glifos para no pintarlo dos
    // veces.
    if (((m_dwBackColor >> 24) != 0) && (0 < iBoxWidth)) {
        GLfloat prevColor[4];
        glGetFloatv(GL_CURRENT_COLOR, prevColor);

        // OJO — el glEnable(0xde1) CRUDO que DrawItemInfoBox hace despues del
        // recuadro (fiel al binario, 0x004C2698) deja DESINCRONIZADO el cache
        // de estado de FUN_00511590/FUN_00511680: DAT_083a4125 (TextureEnable)
        // sigue diciendo "apagada" mientras GL la tiene encendida.  Si entramos
        // a FUN_005124c0 asi, FUN_00511590 se cree el cache, NO llama a
        // glDisable(0xde1), y la franja se dibuja modulada por la textura que
        // hubiera bound en ese momento — que cambia frame a frame.  Eso es el
        // parpadeo.  Resincronizamos el cache con el estado real antes de
        // dibujar; FUN_00511590 apaga la textura de verdad y ambos quedan
        // coherentes.
        DAT_083a4125 = '\x01';

        glColor4ub((GLubyte)( m_dwBackColor        & 0xff),   // R (formato ABGR)
                   (GLubyte)((m_dwBackColor >>  8) & 0xff),   // G
                   (GLubyte)((m_dwBackColor >> 16) & 0xff),   // B
                   (GLubyte)((m_dwBackColor >> 24) & 0xff));  // A
        FUN_005124c0((float)iPos_x, (float)iPos_y,
                     (float)iBoxWidth / fTexScaleX,
                     (float)local_8.cy / _DAT_055c9b74);
        glColor4fv(prevColor);
        // No volvemos a encender la textura: FUN_0040f610 la apaga por su
        // cuenta para los glifos, y dejarla apagada mantiene GL y cache de
        // acuerdo.  El proximo tooltip la reenciende via FUN_00511680.
    }
    {
        const DWORD dwSavedBack = m_dwBackColor;
        m_dwBackColor = 0;
        FUN_0040f610((HDC)(uintptr_t)DAT_055c9ff8,
                     iPos_x + (int)(fVar4 / fTexScaleX), iPos_y, pszText, 0);
        m_dwBackColor = dwSavedBack;
    }

    if (*pszText != '\n') {
        return ((float)local_8.cy / _DAT_055c9b74) / 1.0f;
    }
    return ((float)local_8.cy / _DAT_055c9b74) / 2.0f;
}

void __cdecl FUN_004c2420(int param_1, int param_2, int param_3,
                          int param_4, int param_5, int param_6)
{
    float  y;
    float  x;
    float  Height;
    int    iVar1;
    int    iVar4;
    int   *piVar2;
    char  *pCVar3;
    HFONT  pHVar9;
    float  local_18;
    int    local_14;
    int    local_10;
    SIZE   local_8;

    iVar4      = 0;
    local_8.cx = 0;
    local_8.cy = 0;
    local_10   = 0;
    local_14   = 0;
    local_18   = 0.0f;
    iVar1      = param_3;
    if (0 < param_3) {
        pCVar3 = lpString_07e90798;
        piVar2 = DAT_07ea7b10;
        do {
            iVar1 = iVar4;
            if (*pCVar3 == '\0') break;          // corta el conteo en la 1ª vacía
            pHVar9 = (HFONT)(uintptr_t)DAT_055ca00c;
            if (*piVar2 != 0) {
                pHVar9 = (HFONT)(uintptr_t)DAT_055ca010;
            }
            SelectObject(m_hFontDC, pHVar9);
            GetTextExtentPointA(m_hFontDC, pCVar3, lstrlenA(pCVar3), &local_8);
            if (local_18 < (float)local_8.cx) {
                local_18 = (float)local_8.cx;
            }
            if (*pCVar3 == '\n') {
                local_14 = local_14 + 1;         // línea de media altura
            }
            else {
                local_10 = local_10 + 1;
            }
            iVar4  = iVar4 + 1;
            piVar2 = piVar2 + 1;
            pCVar3 = pCVar3 + 100;
            iVar1  = param_3;
        } while (iVar4 < param_3);
    }
    param_3 = iVar1;
    Height = ((float)local_14 * (float)local_8.cy * 0.5f + (float)(local_10 * local_8.cy)) /
             (_DAT_055c9b74 * 0.9090909f);
    FUN_00511680(1);                             // EnableAlphaTest
    const float fTexScaleX = Text_GetOrthoScaleX();
    local_18 = local_18 / fTexScaleX;
    if (0 < param_4) {
        local_18 = (float)param_4 / fTexScaleX + (float)param_4 / fTexScaleX;
    }
    local_18 = local_18 + 4.0f;
    param_4 = (int)((float)param_1 - local_18 * 0.5f);   // el recuadro se CENTRA en param_1
    if (param_4 < 0) {
        param_4 = 0;
    }
    if ((float)DAT_0056156c / fTexScaleX < (float)param_4 + local_18) {
        param_4 = (int)((float)DAT_0056156c / fTexScaleX - local_18 - 1.0f);
    }
    if (param_6 == 1) {
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        y = (float)param_2 - 1.0f;
        x = (float)param_4 - 1.0f;
        FUN_005124c0(x, y, local_18 + 1.0f, 1.0f);                  // borde superior
        FUN_005124c0(x, y, 1.0f, Height + 1.0f);                    // borde izquierdo
        FUN_005124c0(x + local_18 + 1.0f, y, 1.0f, Height + 1.0f);  // borde derecho
        FUN_005124c0(x, y + Height + 1.0f, local_18 + 2.0f, 1.0f);  // borde inferior
        glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
        FUN_005124c0((float)param_4, (float)param_2, local_18, Height);
        glEnable(0xde1);
    }
    param_4 = param_4 + 1;
    x = (float)param_4;
    y = (float)param_2;
    iVar1 = 0;
    if (0 < param_3) {
        pCVar3 = lpString_07e90798;
        do {
            float fAdvance;
            pHVar9 = (HFONT)(uintptr_t)DAT_055ca00c;
            if (DAT_07ea7b10[iVar1] != 0) {
                pHVar9 = (HFONT)(uintptr_t)DAT_055ca010;
            }
            SelectObject(m_hFontDC, pHVar9);
            if ((*pCVar3 == '\n') || ((*pCVar3 == ' ') && (pCVar3[1] == '\0'))) {
                GetTextExtentPointA(m_hFontDC, pCVar3, lstrlenA(pCVar3), &local_8);
                if (*pCVar3 == '\n') {
                    fAdvance = ((float)local_8.cy / _DAT_055c9b74) / 2.0f;
                }
                else {
                    fAdvance = ((float)local_8.cy / _DAT_055c9b74) / 1.0f;
                }
            }
            else {
                iVar4 = DAT_07e91708[iVar1];
                if ((unsigned int)iVar4 <= 6) {
                    glColor3f(DrawItemInfoBox_glColor[iVar4][0],
                              DrawItemInfoBox_glColor[iVar4][1],
                              DrawItemInfoBox_glColor[iVar4][2]);
                    m_dwTextColor = DrawItemInfoBox_TextColor[iVar4];
                }
                m_dwBackColor = (DAT_07e91708[iVar1] != 5) ? 0 : 0xff0000a0;
                fAdvance = RenderText_0040fb70((int)x, (int)y, pCVar3,
                                        (int)((local_18 - 2.0f) * fTexScaleX),
                                        param_5, 0x280);
            }
            y = y + fAdvance * 1.1f;
            iVar1  = iVar1 + 1;
            pCVar3 = pCVar3 + 100;
        } while (iVar1 < param_3);
    }
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    FUN_00511600();                              // DisableAlphaBlend
    return;
}


// ─────────────────────────────────────────────────────────────────────────────
// FUN_004c2880 @ 0x004c2880 — CharMenu_AppendSkillReq
//
// Reads weapon skill slots at param_1+0x38 (4 slots, each 4 bytes).
// Checks hero's class (DAT_07abf5d8+0x1bc) vs required class per slot.
// Formats each slot into lpString_07e90798 using crt_sprintf with format
// strings from DAT_0055a400/DAT_0055a404.
// Increments DAT_07eaa154 per entry.

void __cdecl FUN_004c2880(int param_1)
{
    int  iVar1;
    char buf[256];
    int  heroClass = *(int*)(&DAT_07abf5d8 + 0x1bc);

    for (int s = 0; s < 4; s++) {
        int slot = *(int*)(param_1 + 0x38 + s * 4);
        if (slot == 0) continue;

        // Check class requirement
        int required = slot & 0xff;
        if (required != 0 && heroClass != required) {
            // Use "not met" format
            crt_sprintf(buf, DAT_0055a404, slot >> 8, required);
            DAT_07e91708[DAT_07eaa154] = 3;  // red
        } else {
            crt_sprintf(buf, DAT_0055a400, slot >> 8);
            DAT_07e91708[DAT_07eaa154] = 2;  // green
        }

        DAT_07ea7b10[DAT_07eaa154] = 0;
        slot_strcpy(DAT_07eaa154, buf);
        DAT_07eaa154++;
        DAT_07eaa158++;
    }
}


// ─────────────────────────────────────────────────────────────────────────────
// FUN_004c2c10 @ 0x004c2c10 — CharMenu_AppendStatRows
//
// Loops 0 to 0xb (skips if param_6==5), reads from DAT_07e9152c.
// Calls FUN_004c2420 for display.
// Uses GetTextExtentPointA for width measurement.

void __cdecl FUN_004c2c10(int row, unsigned char *color, int *value,
                            const char *label, int x, int flags)
{
    char   buf[256];
    int    iVar1;
    SIZE   sz;
    HDC    hdc = (HDC)DAT_055c9fec;

    if (flags == 5) return;

    for (int i = 0; i <= 0xb; i++) {
        // IDA sub_4C2C10: `v9 = &dword_7E9152C; ... v9 += 10;` — columna 1 de
        // cada fila, paso de 10 ints.  (El port usaba `&DAT_07e9152c + i * 4`
        // sobre un int*: 64 bytes de paso sobre un global de 4 bytes.)
        int val = DAT_07e91528[10 * i + 1];
        if (val == 0) continue;

        crt_sprintf(buf, label, val);
        GetTextExtentPointA(hdc, buf, (int)strlen(buf), &sz);

        DAT_07e91708[DAT_07eaa154] = (color ? color[i] : 0);
        DAT_07ea7b10[DAT_07eaa154] = 0;
        slot_strcpy(DAT_07eaa154, buf);
        DAT_07eaa154++;
    }

    FUN_004c2420(row, x, DAT_07eaa154, *value, 2, 1);
}


// ─────────────────────────────────────────────────────────────────────────────
// FUN_004c2d50 @ 0x004c2d50 — CharMenu_AppendSkillDesc
//
// Switches on param_1 (0-9) to select description string from
// DAT_07d359d0...DAT_07d36204. Formats into text buffer, calls FUN_004c2420.

void __cdecl FUN_004c2d50(int param_1, int param_2, int param_3)
{
    // BUG-FIX 2026-05-03: original `descTable` held literal source-binary addresses
    // (0x07d359d0 onwards) that map to text-pool data in the source binary but are
    // unmapped memory in our build. Reading `(const char*)descTable[param_1]`
    // would AV on `if (!*desc)`. Until the description text-pool is parsed
    // (Data\Local\Text.bmd handler chain), this function is a no-op.
    (void)param_1; (void)param_2; (void)param_3;
}


// ─────────────────────────────────────────────────────────────────────────────
// FUN_004c2e20 @ 0x004c2e20 — CharMenu_BuildStatRequirements
//
// Guard: if DAT_00559fe0 == param_1, return (already built for this class).
// Reads char data from DAT_07d78068 + param_1 * 0x40.
// Calculates level-scaled ATT/DEF/MANA reqs, fills DAT_07e91528-07e91550 array.
// Contains HashTable operations with XOR encryption (key at DAT_00559050,
// 0x584 bytes, ref-count at +0x161). Loops 0-11 per stat level.

void __cdecl FUN_004c2e20(int param_1)
{
    int   iVar1;
    int   iVar2;
    int   iVar3;
    int  *piDst;

    // Already computed?
    if (DAT_00559fe0 == param_1) return;
    DAT_00559fe0 = param_1;

    // Base class data pointer
    int classBase = DAT_07d78068 + param_1 * 0x40;

    // Limpiar la tabla entera (12 filas x 10 columnas).
    // 2026-08-21: antes limpiaba solo 12 ints (= la fila 0 y un poco), porque el
    // global estaba declarado como int[12] en vez de la tabla completa.
    piDst = DAT_07e91528;
    for (int i = 0; i < 12 * 10; i++) piDst[i] = 0;

    // Build per-level scaled requirements
    for (int i = 0; i <= 11; i++) {
        int base_att  = *(int*)(classBase + 0x00);
        int base_def  = *(int*)(classBase + 0x04);
        int base_mana = *(int*)(classBase + 0x08);
        int level_req = *(int*)(classBase + 0x0c + i * 4);

        if (level_req <= 0) continue;

        // Scale by level
        iVar1 = base_att  + (level_req * *(int*)(classBase + 0x30));
        iVar2 = base_def  + (level_req * *(int*)(classBase + 0x34));
        iVar3 = base_mana + (level_req * *(int*)(classBase + 0x38));

        // Fila i, columnas 0 / 2 / 5.  IDA sub_4C2E20 L220-228: `v23 = 10 * i;`
        // y despues `dword_7E91528[v23]`, `dword_7E91530[v23]`, `dword_7E9153C[v23]`
        // — o sea paso de fila de 10 ints.
        // 2026-08-21: el port hacia `&DAT_07e91530 + i * 4` sobre un int*, que
        // avanza 64 bytes por vuelta sobre un global de 4 bytes → escribia hasta
        // 176 bytes fuera, encima de lo que el linker pusiera al lado.
        DAT_07e91528[10 * i + 0] = iVar1;
        DAT_07e91528[10 * i + 2] = iVar2;
        DAT_07e91528[10 * i + 5] = iVar3;
    }

    // HashTable XOR-obfuscation (ref-count at DAT_00559050+0x161)
    unsigned char *pHT = (unsigned char*)&DAT_00559050;
    int refCount = *(int*)(pHT + 0x161);
    // (no game-logic side-effects here; this is the compiler/obfuscation artifact)
    (void)refCount;
}
