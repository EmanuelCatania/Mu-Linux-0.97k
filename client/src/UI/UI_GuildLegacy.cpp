// UI_GuildLegacy.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// FUN_00408cb0 vive ahora en Scene/Scene_CharSelect_Nav.cpp y FUN_00408e30 en
// Net/Crypto.cpp; antes se definian en este archivo.
void __fastcall FUN_00408cb0(int*, float);
int  __cdecl    FUN_00408e30(DWORD *a1);

#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl FUN_0054158c(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
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


// ── sub_51DDF0 (0x0051DDF0) — tabla de puestos del evento ────────────────────
// NO es una lista de guild: es el ranking que sale al terminar Devil Square /
// Blood Castle.  Lo dispara `RenderErrorMessage` con `case 140: sub_51DDF0()`,
// tras `sub_51D9E0` (opcode 0x93) que deja ErrorMessage = 140 y copia las
// entradas a DAT_083a7af8.
//
// 2026-09-07: estaba neutralizada con un `return 0` al entrar ("AUTO-SKIP:
// absolute end-bound loop"), asi que el cartel salia con el marco y el boton OK
// pero VACIO.  Reescrita contra IDA con los bucles acotados por contador.
//
// Registro (24 bytes, = PMSG_DEVIL_SQUARE_SCORE del server, con el padding del
// DWORD): +0 name[10] · +12 score · +16 RewardExperience · +20 RewardMoney.
int __cdecl FUN_0051ddf0(void)
{
    // Columnas: puesto, personaje, puntos, experiencia, recompensa.
    static const int kCol[5] = { 219, 235, 287, 345, 383 };
    char buf[64];

    glColor3f(0.5f, 1.0f, 0.5f);
    UI_RenderText(229, 70, GlobalText[647], (LPSIZE)0, '\0', 0);
    wsprintfA(buf, GlobalText[648], (const char *)((int)DAT_07abf5d8 + 0x1c1));
    UI_RenderText(229, 86, buf, (LPSIZE)0, '\0', 0);

    // Encabezados 680..684.  El original SALTEA el 681 ("Personaje") y le suma
    // 10 px al indice 2; es asi en el binario.
    glColor3f(0.5f, 0.5f, 1.0f);
    for (int i = 0; i < 5; ++i) {
        if (i == 1) continue;
        UI_RenderText(kCol[i] + (i == 2 ? 10 : 0), 110, GlobalText[680 + i],
                      (LPSIZE)0, '\0', 0);
    }

    int count = DAT_083a7c30;
    const int kMaxRows = GUILD_MEMBER_TABLE_BYTES / GUILD_MEMBER_STRIDE;
    if (count > kMaxRows) count = kMaxRows;
    if (count <= 0) return DAT_083a7c30;

    int y = 126;
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < count; ++i) {
        // El registro 0 del buffer es el del propio jugador y va ULTIMO: por eso
        // el original indexa con (i+1) % count.
        const BYTE *e = &DAT_083a7af8[GUILD_MEMBER_STRIDE * ((i + 1) % count)];

        if (i == count - 1) {
            glColor3f(0.9f, 0.9f, 0.0f);
            UI_RenderText(219, 286, GlobalText[685], (LPSIZE)0, '\0', 0);
            y = 302;
            wsprintfA(buf, "%2d", DAT_083a7c34);
        } else {
            wsprintfA(buf, "%2d", i + 1);
        }
        m_dwTextColor = 0xFFFFFFFFu;
        UI_RenderText(kCol[0], y, buf, (LPSIZE)0, '\0', 0);

        char name[11];
        memcpy(name, e, 10);
        name[10] = '\0';
        UI_RenderText(kCol[1], y, name, (LPSIZE)0, '\0', 0);

        m_dwTextColor = 0xFFFFD2D2u;
        wsprintfA(buf, "%10d", *(const int *)(e + 12));
        UI_RenderText(kCol[2], y, buf, (LPSIZE)0, '\0', 0);

        m_dwTextColor = 0xFFD2FFD2u;
        wsprintfA(buf, "%6d", *(const int *)(e + 16));
        UI_RenderText(kCol[3], y, buf, (LPSIZE)0, '\0', 0);

        m_dwTextColor = 0xFFD2D2FFu;
        wsprintfA(buf, "%6d", *(const int *)(e + 20));
        UI_RenderText(kCol[4], y, buf, (LPSIZE)0, '\0', 0);

        y += 16;
    }
    return DAT_083a7c30;
}

// FUN_0051db00 @ 0x0051DB00 — GuildOverview_Render
// Renders guild war overview panel: win/draw title, total score, kills, deaths.
// Uses GetTextExtentPointA for centering. unaff_EDI is the text width from
// the last GetTextExtentPointA call — approximated via lstrlenA * pixel_per_char.
// DAT_083a7c30=0 → "draw" strings; !=0 → "win/loss" strings.
int __cdecl FUN_0051db00(void)
{
    glColor3f(0.5f, 0.5f, 0.5f);
    const char *title, *subtitle;
    tagSIZE sz = {};
    HDC hdc = (HDC)(uintptr_t)DAT_055c9fec;
    if (DAT_083a7c30 == 0) {
        title    = lpString_07d68bc8;
        subtitle = lpString_07d68cf4;
    } else {
        title    = lpString_07d68970;
        subtitle = lpString_07d68a9c;
    }
    int len = lstrlenA(title);
    GetTextExtentPointA(hdc, title, len, &sz);
    int cx = (int)((unsigned int)(sz.cx * 0x280) / DAT_0056156c >> 1);
    UI_RenderText(0x140 - cx, 0x46, title, (LPSIZE)0, '\0', 0);

    len = lstrlenA(subtitle);
    GetTextExtentPointA(hdc, subtitle, len, &sz);
    cx = (int)((unsigned int)(sz.cx * 0x280) / DAT_0056156c >> 1);
    UI_RenderText(0x140 - cx, 0x56, subtitle, (LPSIZE)0, '\0', 0);

    glColor3f(1.0f, 1.0f, 1.0f);
    DAT_00559c78 = 0xffffffff;
    SelectObject(hdc, (HGDIOBJ)(uintptr_t)DAT_055ca010);

    // Score total
    char buf[256];
    DAT_00559c78 = 0xffd2ffd2;
    wsprintfA(buf, &param_2_07d68e20);
    len = lstrlenA(buf);
    GetTextExtentPointA(hdc, buf, len, &sz);
    cx = (int)((unsigned int)(sz.cx * 0x280) / DAT_0056156c >> 1);
    UI_RenderText(0x140 - cx, 0x6a, buf, (LPSIZE)0, '\0', 0);

    int iY = 0x82;
    if (DAT_083a7c30 != 0) {
        DAT_00559c78 = 0xffd2d2ff;
        wsprintfA(buf, &param_2_07d68f4c, DAT_083a7b0c);
        len = lstrlenA(buf);
        GetTextExtentPointA(hdc, buf, len, &sz);
        cx = (int)((unsigned int)(sz.cx * 0x280) / DAT_0056156c >> 1);
        UI_RenderText(0x140 - cx, 0x82, buf, (LPSIZE)0, '\0', 0);
        iY = 0x9a;
    }
    DAT_00559c78 = 0xffffd2d2;
    wsprintfA(buf, &param_2_07d69078, DAT_083a7b04);
    len = lstrlenA(buf);
    GetTextExtentPointA(hdc, buf, len, &sz);
    cx = (int)((unsigned int)(sz.cx * 0x280) / DAT_0056156c >> 1);
    return (int)UI_RenderText(0x140 - cx, iY, buf, (LPSIZE)0, '\0', 0);
}

// CreateOkMessageBox @ 0x0051D6F0 — show an OK dialog by setting the UI state.
// Wraps text at 7 chars / 0x26 lines into DAT_083a44c4, sets a fixed panel descriptor,
// then transitions DAT_083a7c24 or DAT_083a7c28 to state 0x8b.
void __cdecl CreateOkMessageBox(char *msg)
{
    DAT_083a4324 = SeparateTextIntoLines(msg, (char *)&DAT_083a44c4, 7, 0x26);
    // clear panel descriptor (10 dwords)
    int *p = (int*)&DAT_083a42f8[0];
    for (int i = 0; i < 10; i++) p[i] = 0;
    // set fixed descriptor: {1, 0x47, 0x8c, 0x46, 0x15}
    DAT_083a42f8[0] = 1;
    DAT_083a42f8[1] = 0x47;
    DAT_083a42f8[2] = 0x8c;
    DAT_083a42f8[3] = 0x46;
    DAT_083a42f8[4] = 0x15;
    if (DAT_083a7c24 != 0)
        DAT_083a7c28 = 0x8b;
    else
        DAT_083a7c24 = 0x8b;
}

// FUN_0051d9e0 @ 0x0051D9E0 — GuildMemberList_Update
// Sets UI state 0x8c, stores count/param2, copies menu descriptor and member list data.
void __cdecl FUN_0051d9e0(int count, int p2, void *data)
{
    if (DAT_083a7c24 == 0) DAT_083a7c24 = 0x8c;
    else                   DAT_083a7c28 = 0x8c;
    DAT_083a7c34 = (DWORD)p2;
    DAT_083a7c30 = count;
    // Copy menu descriptor: 5 DWORDs
    DWORD desc[5] = { 1, 0x47, 0x104, 0x46, 0x15 };
    for (int i = 0; i < 5; i++) DAT_083a42f8[i] = desc[i];
    // Copy member list data: count * 0x18 bytes into DAT_083a7af8
    // 2026-09-03: la tabla tiene 11 registros (0x108 bytes, ver globals.h).
    // IDA no acota `count` porque alli el hueco es exactamente ese; aca el
    // clamp evita que un `count` grande escriba sobre los globals vecinos.
    int nMembers = count;
    if (nMembers < 0) nMembers = 0;
    if (nMembers > GUILD_MEMBER_TABLE_BYTES / GUILD_MEMBER_STRIDE)
        nMembers = GUILD_MEMBER_TABLE_BYTES / GUILD_MEMBER_STRIDE;
    unsigned int dwords = (unsigned int)(nMembers * GUILD_MEMBER_STRIDE) >> 2;
    unsigned int *src = (unsigned int*)data;
    unsigned int *dst = (unsigned int*)&DAT_083a7af8[0];
    for (unsigned int i = 0; i < dwords; i++) *dst++ = *src++;
}

// FUN_0051da80 @ 0x0051DA80 — GuildMemberList_Add (single member record)
// Sets UI state 0x9a, stores param_1 as member count, copies 5-entry menu desc + 6 DWORDs data.
void __cdecl FUN_0051da80(int p1, void *data)
{
    if (DAT_083a7c24 == 0) DAT_083a7c24 = 0x9a;
    else                   DAT_083a7c28 = 0x9a;
    DAT_083a7c30 = (int)(DWORD)p1;
    DWORD desc[5] = { 1, 0x47, 0x82, 0x46, 0x15 };
    for (int i = 0; i < 5; i++) DAT_083a42f8[i] = desc[i];
    DWORD *src = (DWORD*)data;
    DWORD *dst = (DWORD*)&DAT_083a7af8[0];   // un registro = 6 DWORDs
    for (int i = 0; i < 6; i++) *dst++ = *src++;
}

// FUN_0051d840 @ 0x0051D840 — ItemList_Select(slot)
// Selects character slot `slot` for the in-game item/skill list display.
// Sets DAT_005615dc, populates DAT_083a4324 and skill/item display arrays,
// then transitions UI state to 0x8e.
// 2026-08-21: era un stub que salteaba el texto ("requires DAT_07cf5608 char
// data arrays not yet mapped").  La tabla ya esta reconciliada (DIALOG_SCRIPT
// en globals.h), asi que ahora es el port fiel de sub_51D840: arma el cuadro de
// dialogo desde g_DialogScript[a1] igual que CSQuest::ShowDialogText, mas el
// memset de los rects de boton y ErrorMessage = 142.
int __cdecl FUN_0051d840(int param_1) {
    char szText[72];

    if (param_1 < 0 || param_1 >= DIALOG_SCRIPT_COUNT) return 1;   // guard de port
    const DIALOG_SCRIPT *dlg = &g_DialogScript[param_1];

    g_iCurrentDialogScript = param_1;
    g_iNumLineMessageBoxCustom =
        SeparateTextIntoLines(dlg->m_lpszText, &DAT_083a44c4[0], 7, 38);

    memset(DAT_083a42f8, 0, 0x28);              // rects de los botones
    memset(g_lpszDialogAnswer, 0, sizeof(g_lpszDialogAnswer));

    int i = 0;
    g_iNumAnswer = 0;
    int nAnswer = dlg->m_iNumAnswer;
    if (nAnswer > 10) nAnswer = 10;             // la tabla tiene 10 slots
    if (nAnswer > 0) {
        char *dst = &g_lpszDialogAnswer[0][0][0];
        do {
            wsprintfA(szText, "%d) %s", i + 1, dlg->m_lpszAnswer[i]);
            int nLine = SeparateTextIntoLines(szText, dst, 1, 38);
            if (nLine < 0) g_lpszDialogAnswer[i][nLine][0] = 0;
            ++i;
            ++g_iNumAnswer;
            dst += 38;
        } while (i < nAnswer);
    }
    if (dlg->m_iNumAnswer == 0) {
        wsprintfA(szText, "%d) %s", i + 1, GlobalText[609]);
        g_iNumAnswer = 1;
        strcpy(&g_lpszDialogAnswer[0][0][0], szText);
    }

    // IDA: ErrorMessage = 142, o NextErrorMessage si ya hay un cartel arriba.
    if (DAT_083a7c24 != 0) DAT_083a7c28 = 142;
    else                   DAT_083a7c24 = 142;
    return 1;
}
// FUN_0051e7e0 — implemented in src/Scene/Scene_ServerSelect_Input.cpp
