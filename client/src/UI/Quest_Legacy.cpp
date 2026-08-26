// Quest_Legacy.cpp
// Extracted from stubs_bulk_misc.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_00402ff0 @ 0x00402FF0 (~59 lines) — Quest dialog: render message box text + answer choices
// __fastcall(ecx=questObj). Renders g_lpszMessageBoxCustom lines centered, then answer list
// with mouse hover highlight (red if hovered). Uses g_iNumAnswer, g_iNumLineMessageBoxCustom.
void __fastcall FUN_00402ff0(int param_1) {
    m_dwTextColor = 0xffd2e6ff;
    m_dwBackColor = 0;
    EnableAlphaTest(true);
    int iPos_y = (((7 - g_iNumAnswer) - g_iNumLineMessageBoxCustom) * 0x12) / 2 + 0x42;
    if (0 < g_iNumLineMessageBoxCustom) {
        for (int i = 0; i < g_iNumLineMessageBoxCustom; i++) {
            RenderCenterText(0x226, iPos_y, g_lpszMessageBoxCustom[i]);
            iPos_y += 0x12;
        }
    }
    if (*(char*)(param_1 + 0x1c882) != '\x01' && *(char*)(param_1 + 0x1c87f) == '\x01') {
        iPos_y = 0xfa;
    }
    int hovered = ((int)MouseY - iPos_y) / 18;
    SelectObject(m_hFontDC, g_hFontBold);
    for (int i = 0; i < g_iNumAnswer; ++i) {
        int dx = 556 - (int)MouseX;
        if (dx < 0) dx = -dx;
        if (hovered == i && dx <= 106) {
            m_dwTextColor = 0xff0000ff;
        } else {
            m_dwTextColor = 0xff67bfdf;
        }
        char* line = &g_lpszDialogAnswer[i][0][0];
        if (*line) {
            RenderCenterText(550, iPos_y, line);
        }
        iPos_y += 18;
    }
    glColor3f(1.0f, 1.0f, 1.0f);
}

extern "C" void __cdecl RenderTipText(int sx, int sy, const char* Text);
int __cdecl FUN_004c3dd0(int param_1);

// sub_403150 @ 0x00403150 (438 bytes) — lista de items que pide la quest.
//
// Devuelve 1 si el personaje TIENE todos los items requeridos, 0 si falta
// alguno.  sub_403320 usa ese retorno para pintar el boton de "Proceder" en
// claro o en gris; sub_4BFDE0 la llama con a3 = 0 para dibujar los modelos 3D
// de los items en el panel.
//
//   a2 = estado esperado (se compara contra This + 0x1C882)
//   a3 = 0 -> dibuja los items en 3D;  != 0 -> dibuja "Nombre x N" como texto
//
// 2026-08-22: portada desde el DISASSEMBLY, no desde el decompile.  Hex-Rays
// emite "positive sp value has been detected, the output may be wrong" para
// esta funcion y pierde los parametros (lee Buffer[92] y v18 sin inicializar),
// asi que el decompile no sirve.  El disassembly, en cambio, sale limpio.
//
// Layout de la entrada de quest (18 bytes, arranca en pQuest + 40 + 18*i):
//   +0  categoria del item      -> nType = categoria * 32 + indice
//   +1  indice del item
//   +2  cantidad pedida
//   +4  flags de disponibilidad por clase (se indexa con This[4])
//   -1  (= pQuest + 39 + 18*i) 1 = la entrada pide un item
char __fastcall FUN_00403150(void *pThis, int /*edx*/, char a2, char a3)
{
    const int This = (int)(uintptr_t)pThis;

    if (*(BYTE *)(This + 0x1c87f) == 0) return 0;      // el panel no esta abierto
    if (*(char *)(This + 0x1c882) != a2) return 0;     // el estado no es el pedido

    float sy  = 235.0f;
    char  ret = 1;
    if (a3 != 0) {
        SelectObject(m_hFontDC, (HGDIOBJ)g_hFontBold);
        sy = 240.0f;
        m_dwBackColor = 0;
    }

    const int    questIdx = *(unsigned char *)(This + 0x1c87a);
    const BYTE  *pQuest   = (const BYTE *)(This + 584 * questIdx + 8);
    const int    nEntry   = *(const short *)pQuest;
    const int    klass    = *(unsigned char *)(This + 4);

    char buf[100];
    for (int i = 0; i < nEntry; ++i) {
        const BYTE *e = pQuest + 40 + 18 * i;
        if (*(const BYTE *)(e + 4 + klass) != 1) continue;   // no aplica a esta clase
        if (*(const BYTE *)(e - 1) != 1)         continue;   // la entrada no pide item

        const int nType  = (int)e[1] + 32 * (int)e[0];
        const int nCount = (int)e[2];

        if (a3 != 0) {
            // FindQuestItemsInInven devuelve 0 si ya los tiene, o cuantos
            // faltan.  De ahi que "0" pinte en celeste y "!= 0" en rojo.
            int missing = FUN_00482dd0(nType, nCount, 0xFFFFFFFFu);
            if (missing == 0) {
                m_dwTextColor = 0xFF67BFDFu;
            } else {
                m_dwTextColor = 0xFF1E1EFFu;
                ret = 0;
            }
            const char *name = "";
            unsigned int abase = (unsigned int)(uintptr_t)DAT_07d78068;
            if (abase >= 0x100000u && abase < 0x80000000u && nType >= 0 && nType < 1024)
                name = (const char *)(uintptr_t)(abase + (unsigned int)nType * 64u);
            crt_sprintf(buf, "%s x %d", name, nCount);
            RenderText(510, (int)sy, buf, 0, 0, nullptr);
        } else {
            // Desviacion conocida del binario: el DLL de inyeccion parchea el
            // byte de 0x004032A8 ("Fix Quest Item Preview") para que este
            // Level sea 0 en vez de -1, porque con -1 la vista previa del item
            // sale mal.  Se deja fiel a IDA.
            RenderItem3D(480.0f, sy, 20.0f, 20.0f, nType, -1, 0, 0, false);
        }
        sy += _DAT_00552464;
    }
    return ret;
}

// FUN_00403320 @ 0x00403320 (955 bytes) — ventana de quest del NPC
// Port fiel del decompile.  2026-08-21: acá había un resumen que sólo dibujaba
// el fondo y dejaba el resto como comentarios ("stub: full render logic
// omitted"); ni siquiera llamaba a FUN_00402ff0, que es la que dibuja el texto
// del diálogo y las respuestas.  Con el flag del panel prendido, GetScreenWidth
// angostaba el viewport a 450 y esa franja quedaba en negro.
//
// Desviación: la llamada a sub_403150 (que dibuja la lista de items pedidos por
// la quest y devuelve si están todos en el inventario) queda pendiente — su
// decompile sale con "positive sp value has been detected, the output may be
// wrong" y Hex-Rays perdió los parámetros.  Acá se asume "cumple" para el color
// del botón; lo único que cambia es que el botón sale habilitado y la lista de
// items no se dibuja.
void __fastcall FUN_00403320(void* param_1) {
    char Buffer[100];
    int  This = (int)(uintptr_t)param_1;

    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    RenderInventoryInterface_stub(450, 0, 1);
    m_dwTextColor = 0xFFD2E6FFu;
    m_dwBackColor = 0;

    char state = *(char*)(This + 116866);      // +0x1C882
    if (state == 1) {
        FUN_005125a0(279, 450.0f, 325.0f, 190.0f, 10.0f,
                     0.0f, 0.0f, 0.7421875f, 0.625f, 1, 1);
        if (FUN_00403150(param_1, 0, state, 1)) {
            m_dwTextColor = 0xFFD2E6FFu;
        } else {
            glColor3f(0.3f, 0.3f, 0.3f);
        }

        if ((double)MouseX >= 485.0 && (double)MouseX < 605.0 &&
            (double)MouseY >= 355.0 && (double)MouseY < 379.0 && MouseLButtonPush) {
            glColor3f(0.4f, 0.4f, 0.4f);
            if (MouseLButtonPop) {
                MouseLButtonPush = 0;
                MouseLButton = 0;
            }
        }
        SelectObject(m_hFontDC, (HGDIOBJ)(DWORD)DAT_055ca00c);
        FUN_005125a0(240, 485.0f, 355.0f, 120.0f, 24.0f,
                     0.0f, 0.0f, 0.83203125f, 1.0f, 1, 1);
        RenderCenterText(545, 360, GlobalText[699]);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    else if (state == 3) {
        FUN_005125a0(271, 500.0f, 367.7f, 113.0f, 18.0f,
                     0.0f, 0.0f, 0.8828125f, 0.5625f, 1, 1);
        m_dwBackColor = 0xFF282828u;
        m_dwTextColor = 0xFF96DCFFu;
        RenderText(470, 370, GlobalText[198], 0, 0, nullptr);
        int zen = *(int*)(This + 116868);       // +0x1C884
        m_dwTextColor = (DWORD)FUN_004c3dd0(zen);
        ConvertGold64_stub(zen, Buffer);
        RenderText(510, 370, Buffer, 0, 0, nullptr);
    }

    // Botón de cerrar + su tooltip
    FUN_005125a0(280, 475.0f, 395.0f, 24.0f, 24.0f,
                 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    if ((double)MouseX >= 475.0 && (double)MouseX < 499.0 &&
        (double)MouseY >= 395.0 && (double)MouseY < 419.0) {
        SelectObject(m_hFontDC, (HGDIOBJ)(DWORD)DAT_055ca00c);
        m_dwTextColor = 0xFFFFFFFFu;
        m_dwBackColor = 0xFF000000u;
        RenderTipText(475, 382, GlobalText[225]);
    }

    m_dwBackColor = 0;
    SelectObject(m_hFontDC, (HGDIOBJ)(DWORD)DAT_055ca00c);
    m_dwTextColor = 0xFFF0FF96u;

    int questIdx  = *(unsigned char*)(This + 116858);   // +0x1C87A
    int questBase = This + 584 * questIdx;
    int nameW = 120 * (int)WindowWidth / 0x280;
    // getMonsterName devuelve NULL mientras MonsterScript no esté parseada
    // (ver la nota de CLAUDE.md); el original no lo contempla.
    char* npcName = getMonsterName(*(unsigned char*)(questBase + 12));
    if (npcName) RenderText(485, 12, npcName, nameW, 1, (void*)3);

    m_dwTextColor = 0xFFFFDCC8u;
    RenderText(472, 22, (char*)(questBase + 13),
               150 * (int)WindowWidth / 0x280, 1, nullptr);

    FUN_00402ff0(This);      // texto del diálogo + respuestas
    glColor3f(1.0f, 1.0f, 1.0f);
}

// FUN_00403f30 @ 0x00403F30 (30 bytes) — dispatcher del render de quest.
// 2026-08-21: no existía y nadie lo llamaba.  sub_4F5820 (Render_QuickButtons_)
// lo invoca entre RenderGoldenArcherWindow y RenderServerDivision; sin eso el
// panel de quest nunca se dibujaba aunque su flag estuviera prendido.
void __fastcall FUN_00403a40(void* param_1);
char __fastcall FUN_00403f30(void* param_1) {
    if (param_1 == nullptr) return 1;
    unsigned char mode = *(unsigned char*)((int)(uintptr_t)param_1 + 116863);  // +0x1C87F
    if (mode == 1)      FUN_00403320(param_1);
    else if (mode == 2) FUN_00403a40(param_1);
    return 1;
}

// FUN_00403700 @ 0x00403700 (~116 lines) — Quest list: render quest entries filtered by state
// __thiscall(this=questObj, param_1=filterState). Iterates up to 200 quests via CSQuest__getQuestState,
// shows matching quests in a scrollable list. Mouse click selects quest + calls CSQuest__CheckQuestState
// and CSQuest__ShowDialogText. Tab buttons "Progress"/"Complete" at top. Selected quest shows arrow icon.
UINT __fastcall FUN_00403700(void* ecx, void* /*edx*/, UINT param_1) {
    (void)ecx; (void)param_1;
    // Tab buttons rendered at offset _DAT_005524a4, hover highlight
    // Quest loop: for questIndex 0..199:
    //   if getQuestState(questIndex) == param_1 and name != empty:
    //     render quest name, highlight on hover, click selects
    //     selected quest gets arrow bitmap (id=9)
    // Returns 0 (via pSVar9)
    return 0;
}

// FUN_00403a40 @ 0x00403A40 (~88 lines) — Quest UI main panel: tabs + quest list + close button
// __fastcall(ecx=questObj). Renders 3-tab quest panel (tab 0=progress, 1=complete, 2=special).
// Calls FUN_00403700 for tab 0/1 content, FUN_00403a30 for tab 2.
// Renders NPC name, close button with tooltip, dialog answers via FUN_00402ff0.
void __fastcall FUN_00403a40(void* param_1) {
    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    RenderInventoryInterface_stub(0x1c2, 0, 1);
    m_dwTextColor = 0xffd2e6ff;
    m_dwBackColor = 0;
    // SelectObject(m_hFontDC, g_hFont); // globals not yet declared
    // Render 3 tab buttons (stride 0x37, bitmaps 0x115/0x116 for normal/selected)
    // Tab click sets *(param_1+0x1c87d)
    // Render separator lines (bitmap 0x117) and border lines (bitmap 0x104)
    // Dispatch to FUN_00403700(1 or 2) or FUN_00403a30() based on selected tab
    // Close button, NPC name, dialog answers
    char cVar1 = *(char*)((int)param_1 + 0x1c87d);
    if (cVar1 == '\0') {
        FUN_00403700(param_1, NULL, 1);
    } else if (cVar1 == '\x01') {
        FUN_00403700(param_1, NULL, 2);
    } else if (cVar1 == '\x02') {
        FUN_00403a30();
    }
    // 2026-08-21: faltaba el render del texto del dialogo + respuestas.
    FUN_00402ff0((int)(uintptr_t)param_1);
    glColor3f(1.0f, 1.0f, 1.0f);
}
