// CharMenu_Build.cpp — ventana de ayuda F1 (RenderHelpWindow @ 0x004C3530)
// y los helpers que arman sus lineas (sub_4C2420, sub_4C2C10, sub_4C2D50,
// sub_4C2E20). El modo sale de DAT_07e11d20: 1 y 2 son listas de ayuda de
// GlobalText[120..159]; 3 es la tabla de valores del item DAT_07e11d24.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// 2026-09-03 -- GUARDA DE RANGO (no esta en IDA).
// `DAT_07eaa154` es el numero de linea del panel y se usa como indice de TRES
// arrays de 30 entradas: `lpString_07e90798` (30 x 100 bytes), `DAT_07e91708`
// (color) y `DAT_07ea7b10` (negrita).  Aca se incrementa dentro de bucles cuya
// cantidad depende de los datos (skills del personaje, filas de stats), sin
// ningun tope, asi que un personaje con muchas entradas escribia mas alla de
// los 3000 bytes del buffer y sobre los globals vecinos.
// `RenderItemInfo` -- que llena las mismas tablas -- ya corta en 28; se replica
// ese criterio: al llegar al tope las lineas extra se pisan sobre la ultima en
// vez de desbordar.  El original no lo necesita porque alli el hueco de memoria
// que sigue al buffer es de relleno.
#define CHARMENU_ROW_MAX 28


// Aliases de los globals del binario que usa esta funcion.
#define TextList       lpString_07e90798   // 30 x 100 bytes
#define TextListColor  DAT_07e91708
#define TextBold       DAT_07ea7b10
#define TextNum        DAT_07eaa154

// IDA: sub_4C3530 modo 1/2 -- una lista de lineas de ayuda de GlobalText.
static void HelpWindow_BuildTextList(int title, int firstLine, int endLine)
{
    GL_ResetState();                        // DisableAlphaBlend
    TextNum = 0;
    crt_sprintf(TextList, "\n");
    int row = 1;
    TextListColor[row] = 1;
    TextBold[row]      = 1;
    strcpy(TextList + row * 100, GlobalText[title]);
    row++;
    crt_sprintf(TextList + row * 100, "\n");
    row++;
    // IDA recorre el bloque de GlobalText con un puntero de a 300 bytes hasta
    // GlobalText[endLine]; el port lo hacia contra la direccion absoluta del
    // binario (0x7D34134), que en este build no existe y se leia fuera.
    for (int i = firstLine; i < endLine; ++i, ++row) {
        TextListColor[row] = 0;
        TextBold[row]      = 0;
        strcpy(TextList + row * 100, GlobalText[i]);
    }
    crt_sprintf(TextList + row * 100, "\n");
    TextNum = row + 1;
    DAT_07e11d6e = 1;
    FUN_004c2420(1, 1, TextNum, 0, 2, 1);
}

// IDA: RenderHelpWindow (0x004C3530)
void RenderHelpWindow(void)
{
    if (DAT_07e11d20 == 1)
        HelpWindow_BuildTextList(120, 121, 140);
    if (DAT_07e11d20 == 2) {
        HelpWindow_BuildTextList(140, 141, 160);
        return;
    }
    if (DAT_07e11d20 != 3) return;

    // ── Modo 3: tabla de valores de un item (DAT_07e11d24 = tipo) ─────────
    GL_ResetState();                        // DisableAlphaBlend
    int colW = 0, pad = 0, colW2 = 0, pad2 = 0;
    if (DAT_0056156c > 1024) {
        if (DAT_0056156c == 1280) { colW = 123; pad = 22; colW2 = 123; pad2 = 32; }
    } else switch (DAT_0056156c) {
        case 1024: colW = 103; pad = 28; colW2 = 103; pad2 = 40; break;
        case 640:  colW = 90;  pad = 38; colW2 = 90;  pad2 = 52; break;
        case 800:  colW = 90;  pad = 33; colW2 = 90;  pad2 = 47; break;
    }

    int id = (int)DAT_07e11d24;
    int kind, maxVal = 0;
    if      (id >= 0   && id < 160) { kind = 1; maxVal = 2160; }
    else if (id >= 160 && id < 192) { kind = 2; maxVal = 900;  }
    else if (id >= 192 && id < 224) { kind = 3; maxVal = 1800; }
    else if (id >= 224 && id < 384) { kind = 4; maxVal = 3000; }
    else if (id >= 480 && id < 512) {
        kind = 5;
        if (DAT_0056156c == 640 || DAT_0056156c == 1280)      maxVal = 5940;
        else if (DAT_0056156c == 800 || DAT_0056156c == 1024) maxVal = 5200;
    } else { DAT_07e11d20 = 0; return; }
    bool isKind5 = (kind == 5);

    // IDA: v30 = v12 / v10 (sin guarda; con otra resolucion divide por 0).
    int scale = (maxVal && colW) ? maxVal / colW : 0;
    int blankRows = isKind5 ? 0 : 11;
    int attr = id * 0x40 + DAT_07d78068;    // &ItemAttribute[id]

    FUN_004c2e20(id);
    TextNum = 0;
    crt_sprintf(TextList, "\n");
    strcpy(TextList + 1 * 100, GlobalText[160]);
    TextListColor[1] = 1;
    TextBold[1]      = 1;
    crt_sprintf(TextList + 2 * 100, "%s", (const char*)(uintptr_t)attr);
    TextListColor[2] = 0;
    TextBold[2]      = 1;
    crt_sprintf(TextList + 3 * 100, "\n");
    crt_sprintf(TextList + 4 * 100, " ");
    crt_sprintf(TextList + 5 * 100, "\n");
    DAT_07e11d6e = 1;
    unsigned spaces = (DAT_0056156c > 800) ? 51 : 46;
    memset(TextList + 6 * 100, ' ', spaces);
    TextList[6 * 100 + spaces] = 0;
    TextListColor[6] = 0;
    TextBold[6]      = 0;
    int row = 7;
    for (int n = 0; n < blankRows; ++n, ++row) {
        TextBold[row] = 0;
        TextList[row * 100]     = ' ';
        TextList[row * 100 + 1] = 0;
        TextListColor[row + 1]  = 0;   // IDA escribe el color de la fila siguiente
    }
    TextNum = row;

    ItemHelp_RequireClass(attr);
    crt_sprintf(TextList + TextNum * 100, "\n");
    ++TextNum;
    FUN_004c2420(1, 1, TextNum, colW2, 2, 1);
    GL_SetBlendSrcOver('\x01');             // EnableAlphaTest(1)
    TextNum = 0;

    // Filas de valores. Formatos leidos de .rdata 0x0055A438..0x0055A4B0.
    FUN_004c2d50(0, scale, pad);
    FUN_004c2c10(0, (unsigned char*)"+%d", &scale, "000000", pad2, kind);
    if (DAT_07e91530 > 0 && !isKind5) {
        FUN_004c2c10(2, (unsigned char*)"%3d", &scale, "00 ", pad2, 0);
        FUN_004c2d50(2, scale, pad);
        FUN_004c2c10(0, (unsigned char*)"~", &scale, " 00", pad2, 0);
    }
    if (DAT_07e91534 > 0 && !isKind5)
        FUN_004c2c10(3, (unsigned char*)"%3d", &scale, "00000", pad2, 0);
    if (id >= 160 && id < 192) {
        FUN_004c2d50(4, scale, pad);
        FUN_004c2c10(4, (unsigned char*)"%2d%%", &scale, "00000", pad2, 0);
    }
    if (DAT_07e9153c > 0) {
        FUN_004c2d50(5, scale, pad);
        FUN_004c2c10(5, (unsigned char*)"%3d", &scale, "000000", pad2, 0);
    }
    if (DAT_07e91540 > 0) {
        FUN_004c2d50(6, scale, pad);
        FUN_004c2c10(6, (unsigned char*)"%3d%%", &scale, "000000", pad2, 0);
    }
    if (!isKind5) {
        FUN_004c2d50(7, scale, pad);
        FUN_004c2c10(7, (unsigned char*)"%3d", &scale, "00000", pad2, 0);
        FUN_004c2d50(8, scale, pad);
        FUN_004c2c10(8, (unsigned char*)"%3d", &scale, "00000", pad2, 0);
    } else {
        FUN_004c2d50(9, scale, pad);
        FUN_004c2c10(9, (unsigned char*)"%3d", &scale, "000000", pad2, kind);
    }
    GL_ResetState();                        // DisableAlphaBlend
}

#undef TextList
#undef TextListColor
#undef TextBold
#undef TextNum


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
// con el color actual de GL.  Nuestro CUIRenderText_RenderText pinta glifos con
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
// texto no hace eso: CUIRenderText_RenderText dibuja los glifos en unidades del ortho,
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
    // DESVIACIÓN: nuestro CUIRenderText_RenderText pinta el fondo solo detras del texto, y
    // no recibe el ancho del box.  Emitimos la franja aca con el ancho
    // correcto y le sacamos el fondo al render de glifos para no pintarlo dos
    // veces.
    if (((m_dwBackColor >> 24) != 0) && (0 < iBoxWidth)) {
        GLfloat prevColor[4];
        glGetFloatv(GL_CURRENT_COLOR, prevColor);

        // OJO — el glEnable(0xde1) CRUDO que DrawItemInfoBox hace despues del
        // recuadro (fiel al binario, 0x004C2698) deja DESINCRONIZADO el cache
        // de estado de GL_SetAlphaTest/GL_SetBlendSrcOver: DAT_083a4125 (TextureEnable)
        // sigue diciendo "apagada" mientras GL la tiene encendida.  Si entramos
        // a GL_DrawRect asi, GL_SetAlphaTest se cree el cache, NO llama a
        // glDisable(0xde1), y la franja se dibuja modulada por la textura que
        // hubiera bound en ese momento — que cambia frame a frame.  Eso es el
        // parpadeo.  Resincronizamos el cache con el estado real antes de
        // dibujar; GL_SetAlphaTest apaga la textura de verdad y ambos quedan
        // coherentes.
        DAT_083a4125 = '\x01';

        glColor4ub((GLubyte)( m_dwBackColor        & 0xff),   // R (formato ABGR)
                   (GLubyte)((m_dwBackColor >>  8) & 0xff),   // G
                   (GLubyte)((m_dwBackColor >> 16) & 0xff),   // B
                   (GLubyte)((m_dwBackColor >> 24) & 0xff));  // A
        GL_DrawRect((float)iPos_x, (float)iPos_y,
                     (float)iBoxWidth / fTexScaleX,
                     (float)local_8.cy / _DAT_055c9b74);
        glColor4fv(prevColor);
        // No volvemos a encender la textura: CUIRenderText_RenderText la apaga por su
        // cuenta para los glifos, y dejarla apagada mantiene GL y cache de
        // acuerdo.  El proximo tooltip la reenciende via GL_SetBlendSrcOver.
    }
    {
        const DWORD dwSavedBack = m_dwBackColor;
        m_dwBackColor = 0;
        CUIRenderText_RenderText((HDC)(uintptr_t)DAT_055c9ff8,
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
    GL_SetBlendSrcOver(1);                             // EnableAlphaTest
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
        GL_DrawRect(x, y, local_18 + 1.0f, 1.0f);                  // borde superior
        GL_DrawRect(x, y, 1.0f, Height + 1.0f);                    // borde izquierdo
        GL_DrawRect(x + local_18 + 1.0f, y, 1.0f, Height + 1.0f);  // borde derecho
        GL_DrawRect(x, y + Height + 1.0f, local_18 + 2.0f, 1.0f);  // borde inferior
        glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
        GL_DrawRect((float)param_4, (float)param_2, local_18, Height);
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
    GL_ResetState();                              // DisableAlphaBlend
    return;
}


// ─────────────────────────────────────────────────────────────────────────────
// ItemHelp_RequireClass @ 0x004c2880 — CharMenu_AppendSkillReq
//
// Reads weapon skill slots at param_1+0x38 (4 slots, each 4 bytes).
// Checks hero's class (DAT_07abf5d8+0x1bc) vs required class per slot.
// Formats each slot into lpString_07e90798 using crt_sprintf with format
// strings from DAT_0055a400/DAT_0055a404.
// Increments DAT_07eaa154 per entry.

// IDA: RequireClass (0x004C2880)
// 2026-09-12: reescrita contra IDA.  La version anterior era inventada: leia
// `*(int*)(&DAT_07abf5d8 + 0x1bc)` (la direccion del PUNTERO al heroe + 0x1BC,
// no el heroe), trataba +0x38 como 4 "slots" de int y formateaba con
// DAT_0055a400/404, que estan vacios.
//
// pItem es la fila de ItemAttribute; +56..+59 = RequireClass[DW, DK, Elf, MG]
// (0 = no la usa, 1 = clase base, 2 = segunda clase).  El nombre de clase es
// GlobalText[4*r + 16 + c]: 20..23 = clases base, 24..26 = segundas clases.
void __cdecl ItemHelp_RequireClass(int param_1)
{
    const BYTE* pItem = (const BYTE*)(uintptr_t)param_1;
    const BYTE* hero  = (const BYTE*)(uintptr_t)DAT_07abf5d8;
    if (!pItem || !hero) return;
    if (DAT_07eaa154 > CHARMENU_ROW_MAX - 4) return;   // guard del buffer de 30 lineas

    const BYTE heroSkin  = hero[0x1BC];
    const int  heroClass = heroSkin & 7;
    const int  heroStep  = heroSkin >> 3;
    auto className = [](int cls, int req) { return GlobalText[4 * req + 16 + cls]; };
    auto line      = [](int i) { return lpString_07e90798 + i * 100; };

    // v6: 2 si la clase del heroe figura en la tabla, 5 si no.
    const int notMetColor = pItem[56 + heroClass] ? 2 : 5;

    int count = 0, cls[4], step[4];
    for (int c = 0; c < 4; ++c) {
        if (pItem[56 + c]) { cls[count] = c; step[count] = pItem[56 + c] - 1; ++count; }
    }
    DAT_07e91708[DAT_07eaa154 + 3] = 0;
    DAT_07e91708[DAT_07eaa154 + 2] = 0;
    if (count < 1) return;

    if (heroClass == 3) {                                // Magic Gladiator
        crt_sprintf(line(DAT_07eaa154), "\n");
        ++DAT_07eaa154; ++DAT_07eaa158;
        const BYTE mgReq = pItem[59];
        if (mgReq && heroStep >= mgReq - 1) {
            crt_sprintf(line(DAT_07eaa154), GlobalText[61], GlobalText[23]);
            DAT_07e91708[DAT_07eaa154] = 0;
            DAT_07ea7b10[DAT_07eaa154] = 0;
            ++DAT_07eaa154;
            return;
        }
        crt_sprintf(line(DAT_07eaa154), GlobalText[60], GlobalText[23]);
        DAT_07e91708[DAT_07eaa154] = notMetColor;
        DAT_07ea7b10[DAT_07eaa154] = 0;
        ++DAT_07eaa154;
        return;
    }
    if (count > 3) return;

    crt_sprintf(line(DAT_07eaa154), "\n");
    ++DAT_07eaa154; ++DAT_07eaa158;
    const bool okA = (heroClass == cls[0] && heroStep >= step[0]);
    if (count == 1) {
        DAT_07e91708[DAT_07eaa154] = okA ? 0 : notMetColor;
        crt_sprintf(line(DAT_07eaa154), GlobalText[61], className(cls[0], pItem[56 + cls[0]]));
    } else if (count == 2) {
        const bool ok = okA || (heroClass == cls[1] && heroStep >= step[1]);
        DAT_07e91708[DAT_07eaa154] = ok ? 0 : notMetColor;
        crt_sprintf(line(DAT_07eaa154), GlobalText[61], className(cls[0], pItem[56 + cls[0]]));
        DAT_07ea7b10[DAT_07eaa154] = 0;
        ++DAT_07eaa154;
        DAT_07e91708[DAT_07eaa154] = ok ? 0 : notMetColor;
        crt_sprintf(line(DAT_07eaa154), GlobalText[61], className(cls[1], pItem[56 + cls[1]]));
    } else {
        // Tres clases: "no puede ser equipado por <la que falta>".  El decompile
        // pasa `300 * v25 + 131249300`; se toma como el nombre de clase base de
        // v25 (GlobalText[20 + v25]) -- inferencia, la constante no cierra
        // contra la direccion de GlobalText que da IDA.
        int missing = 0;
        while (pItem[56 + missing]) { if (++missing >= 4) return; }
        DAT_07e91708[DAT_07eaa154] = notMetColor;
        crt_sprintf(line(DAT_07eaa154), GlobalText[60], GlobalText[20 + missing]);
    }
    DAT_07ea7b10[DAT_07eaa154] = 0;
    ++DAT_07eaa154;
}


// ─────────────────────────────────────────────────────────────────────────────
// IDA: sub_4C2C10 (0x004C2C10) — columna de valores de la ventana de ayuda F1.
// Escribe una linea por cada fila de la tabla DAT_07e91528 (12 filas x 10
// ints; con kind 5 solo la primera), color 0 si la fila cumple (columna 1 == 1)
// y 2 si no, la dibuja en x = *value y despues corre *value por el ancho de
// `widthRef` (o de la ultima linea si es NULL).
// 2026-09-17: el port anterior usaba el patron de ancho como formato y el
// formato como tabla de colores, y salteaba las filas en cero.
void __cdecl FUN_004c2c10(int column, unsigned char *format, int *value,
                            const char *widthRef, int y, int kind)
{
    int last = (kind == 5) ? 0 : 11;
    for (int i = 0; i <= last; i++) {
        crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, (const char*)format,
                    DAT_07e91528[10 * i + column]);
        DAT_07e91708[i] = (DAT_07e91528[10 * i + 1] == 1) ? 0 : 2;
        DAT_07ea7b10[i] = 0;
        ++DAT_07eaa154;
    }
    FUN_004c2420(*value, y, DAT_07eaa154, 0, 3, 0);

    SIZE sz = { 0, 0 };
    const char* ref = widthRef ? widthRef : lpString_07e90798 + (DAT_07eaa154 - 1) * 100;
    GetTextExtentPointA((HDC)DAT_055c9fec, ref, lstrlenA(ref), &sz);
    *value += (int)((double)sz.cx / _DAT_055c9b70);
    DAT_07eaa154 = DAT_07eaa154 - 1 - last;
}


// ─────────────────────────────────────────────────────────────────────────────
// FUN_004c2d50 @ 0x004c2d50 — CharMenu_AppendSkillDesc
//
// Switches on param_1 (0-9) to select description string from
// DAT_07d359d0...DAT_07d36204. Formats into text buffer, calls FUN_004c2420.

// IDA: sub_4C2D50 (0x004C2D50)
// 2026-09-12: era un no-op (la tabla vieja tenia direcciones literales del
// binario).  IDA usa GlobalText directamente: una linea por tipo, color 1, la
// dibuja con sub_4C2420(x, y, n, 0, 3, 0) y vuelve TextNum a 0.
void __cdecl FUN_004c2d50(int param_1, int param_2, int param_3)
{
    const char* text = "";
    switch (param_1) {
    case 0:           text = GlobalText[161]; break;
    case 2: case 3:   text = GlobalText[162]; break;
    case 4:           text = GlobalText[163]; break;
    case 5:           text = GlobalText[164]; break;
    case 6:           text = GlobalText[165]; break;
    case 7:           text = GlobalText[166]; break;
    case 8:           text = GlobalText[167]; break;
    case 9:           text = GlobalText[168]; break;
    default:          break;   // IDA: sprintf(..., NULL); ningun caller pasa otro valor
    }
    if (DAT_07eaa154 > CHARMENU_ROW_MAX - 1) DAT_07eaa154 = CHARMENU_ROW_MAX - 1;
    crt_sprintf(lpString_07e90798 + DAT_07eaa154 * 100, "%s", text);
    DAT_07e91708[DAT_07eaa154] = 1;
    ++DAT_07eaa154;
    FUN_004c2420(param_2, param_3, DAT_07eaa154, 0, 3, 0);
    DAT_07eaa154 = 0;
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
    unsigned char *pHT = (unsigned char*)&PacketXorKey16;
    int refCount = *(int*)(pHT + 0x161);
    // (no game-logic side-effects here; this is the compiler/obfuscation artifact)
    (void)refCount;
}
