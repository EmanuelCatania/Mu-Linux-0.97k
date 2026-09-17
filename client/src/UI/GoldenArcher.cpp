// GoldenArcher.cpp -- ventana del Golden Archer (NPC 236) con dos flujos.
//
//  * Original (IDA): RenderGoldenArcherWindow (0x4F46A0),
//    CheckGoldenArcherWindow (0x4E7AC0), ReceiveEventChipInfomation (0x4372C0),
//    ReceiveEventChip (0x437380), ReceiveMutoNumber (0x4373A0) y
//    ReceiveScratchResult (0x437400).  Paquetes del 0.97k: 0x95 registrar
//    chip, 0x96 pedir numero, 0x98 canjear, 0x9D ticket raspable, 0x97 cerrar.
//
//  * Evento propio del server (MuEmu + DLL, Source/Client/Main/GoldenArcher.cpp):
//    coleccionista de Renas y Piedras, canje VIP y numeros de la suerte, con
//    los paquetes 0x94/0x95/0x96, 0x97:00..04 y 0x9D con otro formato.
//
// Deteccion: el server manda C1:97:00 (configuracion del evento) al entrar con
// el personaje.  Si llega, g_GoldenArcherCustom = 1 y se usa el flujo del DLL;
// si no, el del binario.
//
// Los dos flujos comparten los globales del original: g_bEventChipDialogEnable
// (0x07EAA128), g_shEventChipCount (0x07EAA12C), g_strGiftName (0x07EA97C0) y
// g_bScratchTicket (0x07E11D73).

#include "stdafx.h"
#include "globals.h"
#include "structs.h"
#include "functions.h"
#include <string>
#include <vector>
#include <gl/GL.h>

extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);
void __cdecl CloseInventoryRelatedWindows(void);
extern "C" void   __cdecl RenderInventoryInterface(int StartX, int StartY, int Flag);
extern "C" SIZE*  __cdecl RenderCenteredText(int x, int y, const char* text);
extern "C" double __cdecl RenderNumber2D(float, float, int, float, float);
extern "C" void   __cdecl RenderTipText(int sx, int sy, const char* Text);
extern "C" void   __cdecl RenderInputText(int x, int y, int Index);

extern "C" int g_GoldenArcherCustom = 0;

// ── Globales (nombres de IDA) ───────────────────────────────────────────────
#define GA_OpenType          (*(int*)&DAT_07eaa128)        // g_bEventChipDialogEnable
#define GA_ItemCount         (*(int*)&DAT_07eaa12c)        // g_shEventChipCount
#define GA_ChipCount16       (*(short*)&DAT_07eaa12c)
#define GA_GiftName          ((char*)DAT_07ea97c0)         // g_strGiftName[64]
#define GA_ScratchTicket     DAT_07e11d73                  // g_bScratchTicket
#define GA_MouseX            ((int)DAT_083a427c)
#define GA_MouseY            ((int)DAT_083a4278)
#define GA_MouseLButtonPush  DAT_083a4124
#define GA_MouseLButtonPop   DAT_083a413c
#define GA_MouseLButton      DAT_083a42c4
#define GA_MouseOnWindow     DAT_07d78094
#define GA_MouseUpdateTime   DAT_07e11d28
#define GA_MouseUpdateMax    DAT_00559bec
#define GA_InventoryStartX   (*(int*)&DAT_07ea5288)
#define GA_InventoryStartY   (*(int*)&DAT_07ea5284)
#define GA_InputEnable       DAT_00559c84
#define GA_GoldInputEnable   DAT_07e11d72
#define GA_InputGold         DAT_07e11d74
#define GA_StorageGoldFlag   DAT_07eaa108
#define GA_InputNumber       DAT_00559c88
#define GA_InputTextWidth    DAT_00559c8c
#define GA_InputIndex        DAT_07e11d78
#define GA_WindowWidth       ((int)DAT_0056156c)
#define GA_WindowHeight      ((int)DAT_00561570)

static const int kItemRena = 469;   // GET_ITEM(14, 21)

static void GA_ClearInputFields(void)
{
    Input_ClearState(0);                // ClearInput(0)
    GA_InputEnable = 0;
    GA_GoldInputEnable = 0;
    GA_InputGold = 0;
    GA_StorageGoldFlag = 0;
}

static void GA_OpenLuckyNumberInput(void)
{
    memset(GA_GiftName, 0, 64);
    Input_ClearState(0);
    DAT_00559c94 = 12;                  // InputTextMax[0]
    GA_InputNumber = 1;
    GA_InputEnable = 0;
    GA_GoldInputEnable = 0;
    GA_InputGold = 0;
    GA_StorageGoldFlag = 0;
    GA_ScratchTicket = 1;
}

// ═══════════════════════════════════════════════════════════════════════════
//  FLUJO ORIGINAL (IDA)
// ═══════════════════════════════════════════════════════════════════════════

// ReceiveEventChipInfomation (0x4372C0) -- [3]=modo-1, [4..5]=chips, [6..11]=numero.
static void GA_Ida_RecvInfo(BYTE* Msg, int Size)
{
    if (Size < 6) return;
    CloseInventoryRelatedWindows();
    const int mode = (int)Msg[3] + 1;
    GA_ChipCount16 = *(short*)(Msg + 4);
    GA_OpenType = mode;
    if (mode == 3)
        GA_OpenLuckyNumberInput();
    InventoryOpened = 0;
    CharacterOpened = 0;
    if (mode == 1 && Size >= 12 &&
        *(short*)(Msg + 6) != -1 && *(short*)(Msg + 8) != -1 && *(short*)(Msg + 10) != -1) {
        DAT_07e11e20 = *(short*)(Msg + 6);
        DAT_07e11e22 = *(short*)(Msg + 8);
        DAT_07e11e24 = *(short*)(Msg + 10);
    }
}

// ReceiveEventChip (0x437380)
static void GA_Ida_RecvChip(BYTE* Msg, int Size)
{
    if (Size < 6) return;
    const short count = *(short*)(Msg + 4);
    if (count != -1) GA_ChipCount16 = count;
}

// ReceiveMutoNumber (0x4373A0)
static void GA_Ida_RecvMuto(BYTE* Msg, int Size)
{
    if (Size < 10) return;
    const short a = *(short*)(Msg + 4), b = *(short*)(Msg + 6), c = *(short*)(Msg + 8);
    if (a != -1 && b != -1 && c != -1) {
        DAT_07e11e20 = a; DAT_07e11e22 = b; DAT_07e11e24 = c;
    }
}

// ReceiveScratchResult (0x437400)
static void GA_Ida_RecvScratch(BYTE* Msg, int Size)
{
    if (Size < 4) return;
    const BYTE result = Msg[3];
    if (result <= 4)       CreateOkMessageBox(GlobalText[result + 886]);
    else if (result == 5)  CreateOkMessageBox(GlobalText[899]);
    const int n = Size - 4;
    if (n > 0) memcpy(GA_GiftName, Msg + 4, n < 64 ? n : 64);
}

// Estado visual de un boton del original: apretado, deshabilitado o normal.
static void GA_Ida_ButtonState(bool pressed, bool disabled)
{
    if (pressed) {
        SelectObject(m_hFontDC, g_hFont);
        glColor3f(0.4f, 0.4f, 0.4f);
        if (GA_MouseLButtonPop) { GA_MouseLButtonPush = 0; GA_MouseLButton = 0; }
    } else if (disabled) {
        SelectObject(m_hFontDC, g_hFont);
        glColor3f(0.4f, 0.4f, 0.4f);
    } else {
        SelectObject(m_hFontDC, g_hFontBold);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
}

static bool GA_In(int x, int y, int w, int h)
{
    return GA_MouseX >= x && GA_MouseX < x + w && GA_MouseY >= y && GA_MouseY < y + h;
}

// RenderGoldenArcherWindow (0x4F46A0)
static void GA_Ida_Render(void)
{
    const int mode = GA_OpenType;
    glColor3f(1.0f, 1.0f, 1.0f);
    EnableAlphaTest(true);
    GA_InventoryStartX = 450;
    GA_InventoryStartY = 0;
    RenderInventoryInterface(450, 0, 0);
    m_dwBackColor = 0;
    m_dwTextColor = 0xFFF0FF96u;
    RenderText(GA_InventoryStartX + 35, GA_InventoryStartY + 12, getMonsterName(236),
               120 * GA_WindowWidth / 640, 1, (SIZE*)3);

    int chips = 0;                                     // v46
    if (mode == 1)      chips = GetItemCount(kItemRena, -1);
    else if (mode == 2) chips = GetItemCount(kItemRena, 1);

    m_dwTextColor = 0xFFD2E6FFu;
    m_dwBackColor = 0;
    if (mode == 4) {
        if (GA_In(485, 220, 120, 24) && GA_MouseLButtonPush) GA_Ida_ButtonState(true, false);
        GL_DrawTexture(240, 485.0f, 220.0f, 120.0f, 24.0f, 0.0f, 0.0f, 0.83203125f, 1.0f, 1, 1);
        RenderCenteredText(545, 225, GlobalText[903]);
    }
    if (mode != 3) {
        GA_Ida_ButtonState(GA_In(485, 231, 120, 22) && GA_MouseLButtonPush, chips <= 0);
        GL_DrawTexture(240, 485.0f, 231.0f, 120.0f, 22.0f, 0.0f, 0.0f, 0.83203125f, 1.0f, 1, 1);
        if (mode == 1)      RenderCenteredText(545, 236, GlobalText[243]);
        else if (mode == 2) RenderCenteredText(545, 236, GlobalText[818]);
    }
    if (mode == 1) {
        GA_Ida_ButtonState(GA_In(485, 255, 120, 22) && GA_MouseLButtonPush, chips < 10);
        GL_DrawTexture(240, 485.0f, 255.0f, 120.0f, 22.0f, 0.0f, 0.0f, 0.83203125f, 1.0f, 1, 1);
        RenderCenteredText(545, 260, GlobalText[244]);
        if (DAT_07e11e20 != -1 && DAT_07e11e22 != -1 && DAT_07e11e24 != -1) {
            GA_Ida_ButtonState(GA_In(485, 202, 120, 22) && GA_MouseLButtonPush, false);
            GL_DrawTexture(240, 485.0f, 202.0f, 120.0f, 22.0f, 0.0f, 0.0f, 0.83203125f, 1.0f, 1, 1);
            RenderCenteredText(545, 207, GlobalText[218]);
        }
    } else if (mode == 3) {
        const float xa = 485.0f;
        const float yb = (float)GA_InventoryStartY + 250.0f;
        const int cx = (int)(xa + 56.5f), cy = (int)yb;
        RenderCenteredText(cx, cy - 40, GlobalText[893]);
        m_dwTextColor = 0xFF67BFDFu;
        RenderCenteredText(cx, cy - 15, GlobalText[916]);
        RenderCenteredText(cx, cy - 5, GlobalText[917]);
        SelectObject(m_hFontDC, g_hFontBold);
        RenderCenteredText(cx, cy - 25, GlobalText[894]);
        SelectObject(m_hFontDC, g_hFont);
        m_dwTextColor = 0xFFF0FF96u;
        GL_DrawTexture(271, xa + 3.0f, yb + 10.0f, 113.0f, 18.0f, 0.0f, 0.0f, 0.8828125f, 0.5625f, 1, 1);
        GA_InputTextWidth = 113 * GA_WindowWidth / 640;
        RenderInputText((int)(xa + 10.0f), (int)(yb + 15.0f), 0);
        GA_InputTextWidth = 256;
        const float by = yb + 40.0f;
        if (GA_In((int)xa, (int)by, 120, 22)) {
            if (GA_MouseLButtonPush) GA_Ida_ButtonState(true, false);
            else m_dwTextColor = 0xFF69B270u;          // -9866384
        }
        GL_DrawTexture(240, xa, by, 120.0f, 22.0f, 0.0f, 0.0f, 0.83203125f, 1.0f, 1, 1);
        RenderCenteredText((int)(xa + 60.0f), (int)(by + 5.0f), GlobalText[895]);
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    const float xb = (float)GA_InventoryStartX + 25.0f;
    const float yb2 = (float)GA_InventoryStartY + 395.0f;
    GL_DrawTexture(280, xb, yb2, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    if (GA_In((int)xb, (int)yb2, 24, 24)) {
        SelectObject(m_hFontDC, g_hFont);
        m_dwTextColor = 0xFFFFFFFFu;
        m_dwBackColor = 0xFF000000u;
        RenderTipText((int)xb, (int)yb2 - 13, GlobalText[225]);
    }
    m_dwTextColor = 0xFFD2E6FFu;
    m_dwBackColor = 0;
    SelectObject(m_hFontDC, g_hFont);

    const int tx = GA_InventoryStartX + 95;
    switch (mode) {
    case 1:
        for (int i = 0, y = GA_InventoryStartY + 100; i < 5; ++i, y += 15)
            RenderCenteredText(tx, y, GlobalText[700 + i]);
        break;
    case 2:
        for (int i = 0, y = GA_InventoryStartY + 100; i < 4; ++i, y += 15)
            RenderCenteredText(tx, y, GlobalText[882 + i]);
        break;
    case 3: {
        const int y = GA_InventoryStartY + 70;
        RenderCenteredText(tx, y, GlobalText[891]);
        RenderCenteredText(tx, y + 15, GlobalText[892]);
        RenderCenteredText(tx, y + 30, GlobalText[897]);
        RenderCenteredText(tx, y + 45, GlobalText[898]);
        if (GA_GiftName[0]) {
            glColor3f(0.0f, 0.0f, 0.0f);
            SelectObject(m_hFontDC, g_hFontBold);
            RenderCenteredText(tx + 1, y + 261, GA_GiftName);
            if (sin((double)DAT_05826e08 * 0.0099999998) < 0.0) glColor3f(1.0f, 1.0f, 1.0f);
            else glColor3f(0.6f, 0.6f, 0.6f);
            m_dwTextColor = 0xFF67D2FFu;
            RenderCenteredText(tx, y + 260, GA_GiftName);
            glColor3f(1.0f, 1.0f, 1.0f);
            m_dwTextColor = 0xFFF0FF96u;
            SelectObject(m_hFontDC, g_hFont);
        }
        break;
    }
    case 4: {
        const int y = GA_InventoryStartY + 70;
        RenderCenteredText(tx, y, GlobalText[900]);
        RenderCenteredText(tx, y + 15, GlobalText[706]);
        RenderCenteredText(tx, y + 30, GlobalText[901]);
        RenderCenteredText(tx, y + 45, GlobalText[902]);
        break;
    }
    }

    char buffer[100];
    SelectObject(m_hFontDC, g_hFontBold);
    if (mode == 1)      RenderText(485, 290, GlobalText[245], 0, 0, 0);
    else if (mode == 2) RenderText(485, 290, GlobalText[819], 0, 0, 0);
    if (mode != 3) {
        SelectObject(m_hFontDC, g_hFontBig);
        m_dwTextColor = 0xFF0096D4u;
        sprintf_s(buffer, "x%d", chips);
        if (mode == 1 || mode == 2) RenderText(560, 300, buffer, 0, 0, 0);
    }
    SelectObject(m_hFontDC, g_hFontBold);
    m_dwTextColor = 0xFFD2E6FFu;
    if (mode == 1)      RenderText(485, 325, GlobalText[246], 0, 0, 0);
    else if (mode == 2) RenderText(485, 325, GlobalText[820], 0, 0, 0);
    if (mode != 3) {
        SelectObject(m_hFontDC, g_hFontBig);
        m_dwTextColor = 0xFF0096D4u;
        sprintf_s(buffer, "x%d", (int)GA_ChipCount16);
        if (mode == 1 || mode == 2 || mode == 4) RenderText(560, 335, buffer, 0, 0, 0);
    }
    SelectObject(m_hFontDC, g_hFontBold);
    m_dwTextColor = 0xFFD2E6FFu;
    if (mode == 1) RenderText(485, 360, GlobalText[247], 0, 0, 0);
    if (mode != 3 && mode != 4) {
        SelectObject(m_hFontDC, g_hFontBig);
        m_dwTextColor = 0xFF0096D4u;
        if (DAT_07e11e20 == -1 || DAT_07e11e22 == -1 || DAT_07e11e24 == -1)
            sprintf_s(buffer, "xxx - xxx - xxx");
        else
            sprintf_s(buffer, "%.3d - %.3d - %.3d", DAT_07e11e20, DAT_07e11e22, DAT_07e11e24);
        if (mode == 1) RenderCenteredText(545, 370, buffer);
    }
    glColor3f(1.0f, 1.0f, 1.0f);
}

// CheckGoldenArcherWindow (0x4E7AC0)
static void GA_Ida_Check(void)
{
    const int mode = GA_OpenType;
    if (GA_MouseX >= 450 && GA_MouseX < 640 && GA_MouseY >= 0 && GA_MouseY < 433)
        GA_MouseOnWindow = 1;

    if (mode != 4) {
        if (mode != 3) {
            // Registrar un chip.
            if (GA_In(485, 231, 120, 22) && GA_MouseLButtonPop) {
                GA_MouseLButtonPop = 0;
                const int slot = GetItemSlot(kItemRena, mode - 1);
                if (slot != -1) {
                    const BYTE pkt[5] = { 0xC1, 0x05, 0x95, (BYTE)(mode - 1), (BYTE)slot };
                    Net_SendC1Packet(pkt, sizeof(pkt));
                }
            }
            if (mode == 1) {
                bool send = false;
                if (GA_In(485, 255, 120, 22) && GA_MouseLButtonPop) {
                    GA_MouseLButtonPop = 0;
                    if (GetItemCount(kItemRena, -1) >= 10 ||
                        (DAT_07e11e20 != -1 && DAT_07e11e22 != -1 && DAT_07e11e24 != -1))
                        send = true;
                }
                if (send) {
                    const BYTE pkt[3] = { 0xC1, 0x03, 0x96 };
                    Net_SendC1Packet(pkt, sizeof(pkt));
                } else if (DAT_07e11e20 != -1 && DAT_07e11e22 != -1 && DAT_07e11e24 != -1 &&
                           GA_In(485, 202, 120, 22) && GA_MouseLButtonPop) {
                    GA_MouseLButtonPop = 0;
                    CreateDialogInterface(713, 5);
                }
            }
        } else {
            // Ticket raspable: tres bloques de 4 caracteres con su NUL.
            const int bx = 485;
            const int by = GA_InventoryStartY + 290;
            if (GA_In(bx, by, 120, 22) && GA_MouseLButtonPop) {
                if (GetItemCount(-1, -1)) {
                    const char* text = DAT_07db8710[GA_InputIndex];
                    BYTE pkt[18] = { 0xC1, 18, 0x9D };
                    memcpy(pkt + 3, text + 0, 4);  pkt[7] = 0;
                    memcpy(pkt + 8, text + 4, 4);  pkt[12] = 0;
                    memcpy(pkt + 13, text + 8, 4); pkt[17] = 0;
                    Net_SendC1Packet(pkt, sizeof(pkt));
                } else {
                    CreateOkMessageBox(GlobalText[896]);
                }
                GA_MouseLButton = 0;
            }
        }
    } else if (GA_In(485, 220, 120, 24) && GA_MouseLButtonPop) {
        const BYTE pkt[4] = { 0xC1, 0x04, 0x98, 0x01 };
        Net_SendC1Packet(pkt, sizeof(pkt));
        GA_MouseLButtonPop = 0;
    }

    // X de cerrar: 0x97 del original.
    if (GA_In(GA_InventoryStartX + 25, GA_InventoryStartY + 395, 24, 24) && GA_MouseLButtonPush) {
        GA_MouseLButtonPush = 0;
        const BYTE pkt[3] = { 0xC1, 0x03, 0x97 };
        Net_SendC1Packet(pkt, sizeof(pkt));
        GA_OpenType = 0;
        InventoryOpened = 0;
        CloseInventoryRelatedWindows();
        GA_MouseUpdateTime = 0;
        GA_MouseUpdateMax = 6;
        GA_ClearInputFields();
        GA_ScratchTicket = 0;
    }
    if (GA_MouseLButtonPop) GA_MouseLButtonPop = 0;
}

// ═══════════════════════════════════════════════════════════════════════════
//  EVENTO PROPIO (MuEmu + DLL)
// ═══════════════════════════════════════════════════════════════════════════

enum { GA_IF_NONE = 0, GA_IF_RENA = 1, GA_IF_STONE = 2, GA_IF_LUCKY = 3, GA_IF_BINGO = 4 };

static int  s_RenaRequired[4];
static int  s_StoneRequired[4];
static DWORD s_StoneVipDuration[3];
static char s_LuckyNumber[13];
static BYTE s_CurrentPage = 1;
static BYTE s_TotalPages = 1;
static std::vector<std::string> s_MyLuckyNumbers;

static const int kStartX = 450;
static const int kStartY = 0;

// Color4b(r,g,b,a) viene de structs.h (mismo formato ABGR que el DLL).

static int RealWidth(int w) { return w * GA_WindowWidth / 640; }

static bool IsWorkZone(int x, int y, int w, int h)
{
    return GA_MouseX >= x && GA_MouseX <= x + w && GA_MouseY >= y && GA_MouseY <= y + h;
}

static int CenterTextPosY(const char* text, int y)
{
    SIZE sz = { 0, 0 };
    GetTextExtentPointA(m_hFontDC, text, lstrlenA(text), &sz);
    return y - ((480 * sz.cy / (GA_WindowHeight ? GA_WindowHeight : 480)) >> 1);
}

static void TextC(int x, int y, const char* text, int width)
{
    RenderText(x, y, (char*)text, RealWidth(width), 1, nullptr);
}

static bool ConsumeClick(void)
{
    if (GA_MouseLButton && GA_MouseLButtonPush) {
        GA_MouseLButtonPush = 0;
        GA_MouseUpdateTime = 0;
        GA_MouseUpdateMax = 6;
        return true;
    }
    return false;
}

static void RenderBox(float x, float y, float w, float h, const char* text, bool hoverable = true)
{
    GL_ResetState();                                   // DisableAlphaBlend
    if (!hoverable) {
        glColor3f(0.4f, 0.4f, 0.4f);
        GL_DrawTexture(240, x, y, w, h, 0.0f, 0.0f, 213.0f / 256.0f, 1.0f, 1, 1);
    } else {
        glColor3f(1.0f, 1.0f, 1.0f);
        GL_DrawTexture(240, x, y, w, h, 0.0f, 0.0f, 213.0f / 256.0f, 1.0f, 1, 1);
        if (IsWorkZone((int)x, (int)y, (int)w, (int)h)) {
            glColor3f(0.8f, 0.6f, 0.4f);
            GL_SetBlendAdditive();                     // EnableAlphaBlend
            GL_DrawTexture(240, x, y, w, h, 0.0f, 0.0f, 213.0f / 256.0f, 1.0f, 1, 1);
            glColor3f(1.0f, 1.0f, 1.0f);
            GL_ResetState();
        }
    }
    EnableAlphaTest(true);
    RenderText((int)x, CenterTextPosY(text, (int)(y + h / 2)), (char*)text, RealWidth((int)w), 1, nullptr);
    glColor3f(1.0f, 1.0f, 1.0f);
}

static void DrawSeparatedLines(int x, int& y, const char* text, int step)
{
    char lines[7 * 38 + 1];
    memset(lines, 0, sizeof(lines));
    const int n = SeparateTextIntoLines(text, lines, 7, 38);
    for (int i = 0; i < n; ++i, y += step)
        TextC(x, y, lines + i * 38, 190);
}

// -- Protocolo --------------------------------------------------------------
static void SendRegister(int type)
{
    const BYTE pkt[4] = { 0xC1, 0x04, 0x95, (BYTE)type };
    Net_SendC1Packet(pkt, sizeof(pkt));
}

static void SendExchange(int type, int count)
{
    BYTE pkt[8] = { 0xC1, 0x08, 0x97, 0x02, (BYTE)type, 0, 0, 0 };
    *(short*)(pkt + 6) = (short)count;
    Net_SendC1Packet(pkt, sizeof(pkt));
}

static void SendRegisterLucky(const char* number)
{
    BYTE pkt[17] = { 0xC1, 17, 0x97, 0x03 };
    memcpy(pkt + 4, number, 13);
    Net_SendC1Packet(pkt, sizeof(pkt));
}

static void SendExchangeLucky(const char* number)
{
    BYTE pkt[16] = { 0xC1, 16, 0x9D };
    strncpy_s((char*)pkt + 3, 13, number, _TRUNCATE);
    Net_SendC1Packet(pkt, sizeof(pkt));
}

static void SendClose(void)
{
    const BYTE pkt[3] = { 0xC1, 0x03, 0x31 };
    Net_SendC1Packet(pkt, sizeof(pkt));
}

extern "C" void __cdecl GoldenArcher_CustomCloseProc(void)
{
    CloseInventoryRelatedWindows();
    InventoryOpened = 0;
    CharacterOpened = 0;
    GA_ClearInputFields();
    GA_ScratchTicket = 0;
}

// -- Render / clicks del DLL ------------------------------------------------
static void RenderTitle(void)
{
    EnableAlphaTest(true);
    glColor3f(1.0f, 1.0f, 1.0f);
    SelectObject(m_hFontDC, g_hFontBold);
    m_dwBackColor = Color4b(20, 20, 20, 255);
    m_dwTextColor = Color4b(230, 230, 230, 255);
    int y = kStartY + 12;
    const char* title = getMonsterName(236);
    if (title) RenderText(kStartX + 35, CenterTextPosY(title, y), (char*)title, RealWidth(120), 1, nullptr);
    y += 10;
    static const int kSub[] = { 243, 244, 245, 246 };
    const int t = GA_OpenType;
    if (t >= 1 && t <= 4)
        RenderText(kStartX + 22, CenterTextPosY(GlobalText[kSub[t - 1]], y),
                   GlobalText[kSub[t - 1]], RealWidth(150), 1, nullptr);
}

static void TextStyleBody(void)
{
    EnableAlphaTest(true);
    m_dwBackColor = Color4b(255, 255, 255, 0);
    m_dwTextColor = Color4b(255, 230, 210, 255);
    SelectObject(m_hFontDC, g_hFont);
}

static void RenderCollectedRegistered(int y, int textCollected, int textRegistered,
                                      int collected)
{
    int x = kStartX;
    GL_DrawTexture(245, (float)x + 13, (float)y - 10, 75, 21, 0, 0, 75.0f / 128, 21.0f / 32, 1, 1);
    RenderText(x + 13, CenterTextPosY(GlobalText[textCollected], y), GlobalText[textCollected], RealWidth(75), 1, nullptr);
    x += 190 - 13 - 75;
    GL_DrawTexture(245, (float)x, (float)y - 10, 75, 21, 75.0f / 128, 0, -75.0f / 128, 21.0f / 32, 1, 1);
    RenderText(x, CenterTextPosY(GlobalText[textRegistered], y), GlobalText[textRegistered], RealWidth(75), 1, nullptr);
    y += 25;
    EnableAlphaTest(true);
    glColor3f(1.0f, 1.0f, 1.0f);
    RenderNumber2D((float)kStartX + 54, (float)y - 5, collected, 8, 8);
    glColor3f(0.8f, 0.6f, 0.0f);
    RenderNumber2D((float)kStartX + 142, (float)y - 5, GA_ItemCount, 8, 8);
    glColor3f(1.0f, 1.0f, 1.0f);
    // (El DLL dibuja ademas el modelo 3D de la Rena/Piedra entre los dos
    //  numeros con MyRenderItem3D; no se porto.)
}

static void RenderPagination(void)
{
    const int y = kStartY + 375;
    const float s = 19.0f;
    EnableAlphaTest(true);
    m_dwBackColor = Color4b(255, 255, 255, 0);
    m_dwTextColor = Color4b(255, 255, 255, 255);
    SelectObject(m_hFontDC, g_hFontBold);
    char text[32];
    sprintf_s(text, "%d/%d", s_CurrentPage, s_TotalPages);
    RenderText(kStartX, CenterTextPosY(text, (int)(y + s / 2)), text, RealWidth(190), 1, nullptr);
    const int x = kStartX + 95;
    auto arrow = [&](float ax, float u, float uw) {
        int tex = 0xFE;
        if (IsWorkZone((int)ax, y, (int)s, (int)s)) tex = GA_MouseLButtonPush ? 0x100 : 0xFF;
        GL_DrawTexture(tex, ax, (float)y, s, s, u, 0.0f, uw, 1.0f, 1, 1);
    };
    if (s_CurrentPage > 1) arrow((float)x - 40, 0.0f, 1.0f);
    if (s_CurrentPage < s_TotalPages) arrow((float)x + 21, 1.0f, -1.0f);
    SelectObject(m_hFontDC, g_hFont);
}

static bool CheckPagination(void)
{
    const int x = kStartX + 95, y = kStartY + 375, s = 19;
    if (s_CurrentPage > 1 && IsWorkZone(x - 40, y, s, s)) {
        if (ConsumeClick()) { FUN_00404bc0(25, 0, 0); --s_CurrentPage; }
        return true;
    }
    if (s_CurrentPage < s_TotalPages && IsWorkZone(x + 21, y, s, s)) {
        if (ConsumeClick()) { FUN_00404bc0(25, 0, 0); ++s_CurrentPage; }
        return true;
    }
    return false;
}

static bool CheckExplanation(int x, int y)
{
    if (!IsWorkZone(x, y, 120, 22)) return false;
    if (ConsumeClick()) { FUN_00404bc0(25, 0, 0); CreateDialogInterface(713, 5); }
    return true;
}

static void RenderRena(void)
{
    int x = kStartX, y = kStartY + 50;
    TextStyleBody();
    TextC(x, y, GlobalText[882], 190); y += 15;
    TextC(x, y, GlobalText[883], 190); y += 15;
    RenderBox((float)x + 35, (float)y, 120, 22, GlobalText[218]); y += 45;
    RenderCollectedRegistered(y, 884, 885, GetItemCount(kItemRena, 0)); y += 25;
    y += 15;
    RenderBox((float)x + 35, (float)y, 120, 22, GlobalText[886], GetItemSlot(kItemRena, 0) != -1);
    y += 30;
    GL_DrawTexture(279, (float)x, (float)y, 190, 10, 0, 0, 190.0f / 256, 10.0f / 16, 1, 1);
    y += 20;
    DrawSeparatedLines(x, y, GlobalText[887], 15);
    y += 5;
    char buffer[256];
    for (int i = 0; i < 4; ++i) {
        if (s_RenaRequired[i] <= 0) continue;
        sprintf_s(buffer, GlobalText[888], s_RenaRequired[i]);
        RenderBox((float)x + 35, (float)y, 120, 22, buffer, GA_ItemCount >= s_RenaRequired[i]);
        y += 30;
    }
}

static bool CheckRena(void)
{
    const int x = kStartX + 35;
    int y = kStartY + 80;
    if (CheckExplanation(x, y)) return true;
    y += 85;
    if (GetItemSlot(kItemRena, 0) != -1 && IsWorkZone(x, y, 120, 22)) {
        if (ConsumeClick()) { FUN_00404bc0(25, 0, 0); SendRegister(0); }
        return true;
    }
    y += 50;
    char lines[7 * 38 + 1] = {};
    y += 15 * SeparateTextIntoLines(GlobalText[887], lines, 7, 38);
    y += 5;
    for (int i = 0; i < 4; ++i) {
        if (s_RenaRequired[i] <= 0) continue;
        if (GA_ItemCount >= s_RenaRequired[i] && IsWorkZone(x, y, 120, 22)) {
            if (ConsumeClick()) { FUN_00404bc0(25, 0, 0); SendExchange(0, i); }
            return true;
        }
        y += 30;
    }
    return false;
}

static void RenderStone(void)
{
    int x = kStartX, y = kStartY + 50;
    TextStyleBody();
    TextC(x, y, GlobalText[889], 190); y += 15;
    TextC(x, y, GlobalText[890], 190); y += 15;
    RenderBox((float)x + 35, (float)y, 120, 22, GlobalText[218]); y += 45;
    RenderCollectedRegistered(y, 891, 892, GetItemCount(kItemRena, 1)); y += 25;
    y += 15;
    RenderBox((float)x + 35, (float)y, 120, 22, GlobalText[893], GetItemSlot(kItemRena, 1) != -1);
    y += 30;
    GL_DrawTexture(279, (float)x, (float)y, 190, 10, 0, 0, 190.0f / 256, 10.0f / 16, 1, 1);
    y += 20;

    char buffer[256];
    if (s_CurrentPage == 1) {
        for (int i = 0; i < 3; ++i) {
            if (s_StoneRequired[i] <= 0) continue;
            sprintf_s(buffer, GlobalText[894], s_StoneRequired[i], s_StoneVipDuration[i], i + 1);
            TextC(x, y, buffer, 190);
            y += 10;
            sprintf_s(buffer, GlobalText[895], s_StoneRequired[i]);
            RenderBox((float)x + 35, (float)y, 120, 22, buffer, GA_ItemCount >= s_StoneRequired[i]);
            y += 35;
        }
    } else if (s_CurrentPage == 2) {
        if (s_LuckyNumber[0] == '\0') {
            sprintf_s(buffer, GlobalText[896], s_StoneRequired[3]);
            TextC(x, y, buffer, 190);
            y += 10;
            sprintf_s(buffer, GlobalText[895], s_StoneRequired[3]);
            RenderBox((float)x + 35, (float)y, 120, 22, buffer, GA_ItemCount >= s_StoneRequired[3]);
            y += 40;
            DrawSeparatedLines(x, y, GlobalText[897], 15);
            m_dwTextColor = Color4b(255, 255, 255, 255);
            SelectObject(m_hFontDC, g_hFont);
            glColor3f(1.0f, 1.0f, 1.0f);
        } else {
            TextC(x, y, GlobalText[898], 190);
            y += 10;
            m_dwTextColor = Color4b(212, 150, 0, 255);
            SelectObject(m_hFontDC, g_hFontBig);
            const char* n = s_LuckyNumber;
            sprintf_s(buffer, "%c%c%c%c - %c%c%c%c - %c%c%c%c",
                      n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9], n[10], n[11]);
            TextC(x, y, buffer, 190);
            m_dwTextColor = Color4b(255, 230, 210, 255);
            SelectObject(m_hFontDC, g_hFont);
            y += 30;
            RenderBox((float)x + 35, (float)y, 120, 22, GlobalText[899]);
            y += 40;
            DrawSeparatedLines(x, y, GlobalText[897], 15);
        }
    }
    if (s_TotalPages > 1) RenderPagination();
}

static bool CheckStone(void)
{
    const int x = kStartX + 35;
    int y = kStartY + 80;
    if (CheckExplanation(x, y)) return true;
    y += 85;
    if (GetItemSlot(kItemRena, 1) != -1 && IsWorkZone(x, y, 120, 22)) {
        if (ConsumeClick()) { FUN_00404bc0(25, 0, 0); SendRegister(1); }
        return true;
    }
    y += 60;
    if (s_CurrentPage == 1) {
        for (int i = 0; i < 3; ++i) {
            if (s_StoneRequired[i] <= 0) continue;
            if (GA_ItemCount >= s_StoneRequired[i] && IsWorkZone(x, y, 120, 22)) {
                if (ConsumeClick()) { FUN_00404bc0(25, 0, 0); SendExchange(1, i); }
                return true;
            }
            y += 45;
        }
    } else if (s_CurrentPage == 2 && s_StoneRequired[3] > 0) {
        if (s_LuckyNumber[0] == '\0') {
            if (GA_ItemCount >= s_StoneRequired[3] && IsWorkZone(x, y, 120, 22)) {
                if (ConsumeClick()) { FUN_00404bc0(25, 0, 0); SendExchange(1, 3); }
                return true;
            }
        } else {
            y += 30;
            if (IsWorkZone(x, y, 120, 22)) {
                if (ConsumeClick()) { FUN_00404bc0(25, 0, 0); SendRegisterLucky(s_LuckyNumber); }
                return true;
            }
        }
    }
    return s_TotalPages > 1 && CheckPagination();
}

static void RenderLucky(void)
{
    int x = kStartX, y = kStartY + 50;
    TextStyleBody();
    TextC(x, y, GlobalText[904], 190); y += 15;
    TextC(x, y, GlobalText[905], 190); y += 15;
    RenderBox((float)x + 35, (float)y, 120, 22, GlobalText[218]); y += 30;
    GL_DrawTexture(279, (float)x, (float)y, 190, 10, 0, 0, 190.0f / 256, 10.0f / 16, 1, 1);
    y += 20;
    TextC(x, y, GlobalText[906], 190); y += 15;
    m_dwTextColor = Color4b(223, 191, 103, 255);
    SelectObject(m_hFontDC, g_hFontBold);
    TextC(x, y, GlobalText[907], 190); y += 15;
    SelectObject(m_hFontDC, g_hFont);
    TextC(x, y, GlobalText[908], 190); y += 15;
    TextC(x, y, GlobalText[909], 190); y += 15;
    TextC(x, y, GlobalText[910], 190); y += 20;
    GL_DrawTexture(271, (float)x + 35, (float)y, 120, 18, 0, 0, 113.0f / 128, 18.0f / 32, 1, 1);
    GA_InputTextWidth = RealWidth(110);
    RenderInputText(x + 40, y + 9, 0);
    GA_InputTextWidth = 256;
    y += 30;
    RenderBox((float)x + 35, (float)y, 120, 22, GlobalText[906], strlen(DAT_07db8710[GA_InputIndex]) > 0);
    if (GA_GiftName[0]) {
        y += 30;
        char lines[7 * 38 + 1] = {};
        const int n = SeparateTextIntoLines(GA_GiftName, lines, 7, 38);
        for (int i = 0; i < n; ++i, y += 15) {
            glColor3f(0.0f, 0.0f, 0.0f);
            SelectObject(m_hFontDC, g_hFontBold);
            TextC(x + 1, y + 1, lines + i * 38, 190);
            if (sin((double)DAT_05826e08 * 0.01) < 0) glColor3f(1.0f, 1.0f, 1.0f);
            else glColor3f(0.6f, 0.6f, 0.6f);
            m_dwTextColor = Color4b(255, 210, 103, 255);
            TextC(x, y, lines + i * 38, 190);
        }
        m_dwTextColor = Color4b(255, 255, 255, 255);
        SelectObject(m_hFontDC, g_hFont);
        glColor3f(1.0f, 1.0f, 1.0f);
    }
}

static bool CheckLucky(void)
{
    const int x = kStartX + 35;
    int y = kStartY + 80;
    if (CheckExplanation(x, y)) return true;
    y += 160;
    if (strlen(DAT_07db8710[GA_InputIndex]) > 0 && IsWorkZone(x, y, 120, 22)) {
        if (ConsumeClick()) { FUN_00404bc0(25, 0, 0); SendExchangeLucky(DAT_07db8710[GA_InputIndex]); }
        return true;
    }
    return false;
}

static void RenderBingo(void)
{
    int x = kStartX, y = kStartY + 50;
    TextStyleBody();
    TextC(x, y, GlobalText[915], 190); y += 15;
    TextC(x, y, GlobalText[916], 190); y += 15;
    RenderBox((float)x + 35, (float)y, 120, 22, GlobalText[218]); y += 30;
    GL_DrawTexture(279, (float)x, (float)y, 190, 10, 0, 0, 190.0f / 256, 10.0f / 16, 1, 1);
    if (s_MyLuckyNumbers.empty()) {
        y = kStartY + 216;
        SelectObject(m_hFontDC, g_hFontBold);
        m_dwBackColor = Color4b(255, 255, 255, 0);
        DrawSeparatedLines(x, y, GlobalText[897], 15);
        m_dwTextColor = Color4b(255, 230, 210, 255);
        SelectObject(m_hFontDC, g_hFont);
        return;
    }
    y += 20;
    TextC(x, y, GlobalText[917], 190);
    y += 20;
    m_dwTextColor = Color4b(212, 150, 0, 255);
    SelectObject(m_hFontDC, g_hFontBig);
    bool invert = true;
    const size_t first = (size_t)(s_CurrentPage - 1) * 10;
    for (size_t i = first; i < s_MyLuckyNumbers.size() && i < first + 10; ++i) {
        m_dwBackColor = invert ? Color4b(0, 0, 0, 128) : Color4b(255, 255, 255, 0);
        const std::string& n = s_MyLuckyNumbers[i];
        char text[64];
        if (n.size() >= 12)
            sprintf_s(text, "%c%c%c%c - %c%c%c%c - %c%c%c%c",
                      n[0], n[1], n[2], n[3], n[4], n[5], n[6], n[7], n[8], n[9], n[10], n[11]);
        else
            sprintf_s(text, "%s", n.c_str());
        RenderText(x + 13, y, text, RealWidth(190 - 26), 1, nullptr);
        y += 20;
        invert = !invert;
    }
    if (s_TotalPages > 1) RenderPagination();
}

static bool CheckBingo(void)
{
    if (CheckExplanation(kStartX + 35, kStartY + 80)) return true;
    return s_TotalPages > 1 && CheckPagination();
}

static void GA_Custom_Render(void)
{
    GA_InventoryStartX = kStartX;
    GA_InventoryStartY = kStartY;
    GL_ResetState();
    glColor3f(1.0f, 1.0f, 1.0f);
    RenderInventoryInterface(kStartX, kStartY, 0);
    RenderTitle();
    switch (GA_OpenType) {
    case GA_IF_RENA:  RenderRena();  break;
    case GA_IF_STONE: RenderStone(); break;
    case GA_IF_LUCKY: RenderLucky(); break;
    case GA_IF_BINGO: RenderBingo(); break;
    }
    const int cx = kStartX + 25, cy = kStartY + 395;
    glColor3f(1.0f, 1.0f, 1.0f);
    GL_DrawTexture(280, (float)cx, (float)cy, 24, 24, 0, 0, 0.75f, 0.75f, 1, 1);
    if (IsWorkZone(cx, cy, 24, 24)) {
        SelectObject(m_hFontDC, g_hFont);
        m_dwBackColor = Color4b(255, 255, 255, 0);
        m_dwTextColor = Color4b(255, 255, 255, 255);
        RenderTipText(cx + 2, cy - 15, GlobalText[247]);
    }
}

static void GA_Custom_Check(void)
{
    if (IsWorkZone(kStartX, kStartY, 190, 433)) GA_MouseOnWindow = 1;
    bool handled = false;
    switch (GA_OpenType) {
    case GA_IF_RENA:  handled = CheckRena();  break;
    case GA_IF_STONE: handled = CheckStone(); break;
    case GA_IF_LUCKY: handled = CheckLucky(); break;
    case GA_IF_BINGO: handled = CheckBingo(); break;
    }
    if (handled) return;
    if (IsWorkZone(kStartX + 25, kStartY + 395, 24, 24)) {
        if (ConsumeClick()) { GoldenArcher_CustomCloseProc(); SendClose(); }
        return;
    }
    if (GA_MouseOnWindow && GA_MouseLButton && GA_MouseLButtonPush) {
        GA_MouseLButtonPush = 0;
        GA_MouseUpdateTime = 0;
        GA_MouseUpdateMax = 6;
    }
}

static void GA_Custom_RecvOpen(BYTE* Msg, int Size)
{
    if (Size < 6) return;
    CloseInventoryRelatedWindows();
    InventoryOpened = 0;
    CharacterOpened = 0;
    const BYTE type = Msg[3];
    GA_OpenType = type + 1;
    GA_ItemCount = *(short*)(Msg + 4);
    memset(s_LuckyNumber, 0, sizeof(s_LuckyNumber));
    if (Size >= 19) memcpy(s_LuckyNumber, Msg + 6, 13);
    s_LuckyNumber[12] = 0;
    s_CurrentPage = 1;
    s_TotalPages = 2;
    if (type == 2) GA_OpenLuckyNumberInput();
}

static void GA_Custom_RecvList(BYTE* Msg, int Size)
{
    CloseInventoryRelatedWindows();
    InventoryOpened = 0;
    CharacterOpened = 0;
    GA_OpenType = GA_IF_BINGO;
    // PSWMSG_HEAD (C2, 5 bytes) + int count alineado a 4 -> datos desde +12.
    const int count = (Size >= 12) ? *(int*)(Msg + 8) : 0;
    s_CurrentPage = 1;
    s_TotalPages = (BYTE)(count / 10 + 1);
    s_MyLuckyNumbers.clear();
    for (int i = 0; i < count && 12 + (i + 1) * 13 <= Size; ++i) {
        char number[14] = {};
        memcpy(number, Msg + 12 + i * 13, 13);
        s_MyLuckyNumbers.push_back(number);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  SELECTORES
// ═══════════════════════════════════════════════════════════════════════════

extern "C" void __cdecl RenderGoldenArcherWindow(void)
{
    if (!GA_OpenType) return;
    if (g_GoldenArcherCustom) GA_Custom_Render();
    else                      GA_Ida_Render();
}

// IDA: CheckGoldenArcherWindow (0x004E7AC0).  (El nombre FUN_004e7ac0 se
// mantiene porque lo llama UpdateWindowsMouse; la etiqueta vieja
// "SecondPassword_Screen6" era falsa.)
void __cdecl FUN_004e7ac0(void)
{
    if (!GA_OpenType) return;
    if (g_GoldenArcherCustom) GA_Custom_Check();
    else                      GA_Ida_Check();
}

void GoldenArcher_Recv94(BYTE* Msg, int Size)
{
    if (g_GoldenArcherCustom) GA_Custom_RecvOpen(Msg, Size);
    else                      GA_Ida_RecvInfo(Msg, Size);
}

void GoldenArcher_Recv95(BYTE* Msg, int Size)
{
    if (g_GoldenArcherCustom) { if (Size >= 6) GA_ItemCount = *(short*)(Msg + 4); }
    else                      GA_Ida_RecvChip(Msg, Size);
}

void GoldenArcher_Recv96(BYTE* Msg, int Size)
{
    if (g_GoldenArcherCustom) {
        if (Size >= 16) { memcpy(s_LuckyNumber, Msg + 3, 13); s_LuckyNumber[12] = 0; }
    } else {
        GA_Ida_RecvMuto(Msg, Size);
    }
}

// 0x97 solo existe en el evento propio (el original no recibe 0x97).
void GoldenArcher_Recv97(BYTE* Msg, int Size)
{
    const bool c2 = (Msg[0] == 0xC2 || Msg[0] == 0xC4);
    const BYTE sub = c2 ? Msg[4] : Msg[3];
    switch (sub) {
    case 0x00:
        if (Size < 48) return;
        g_GoldenArcherCustom = 1;
        for (int i = 0; i < 4; ++i) s_RenaRequired[i]  = *(int*)(Msg + 4 + i * 4);
        for (int i = 0; i < 4; ++i) s_StoneRequired[i] = *(int*)(Msg + 20 + i * 4);
        for (int i = 0; i < 3; ++i) s_StoneVipDuration[i] = *(DWORD*)(Msg + 36 + i * 4);
        break;
    case 0x01:
        GA_Custom_RecvList(Msg, Size);
        break;
    case 0x03: {
        if (Size < 5) return;
        const BYTE r = Msg[4];
        CreateOkMessageBox(GlobalText[r < 4 ? 900 + r : 901]);
        break;
    }
    case 0x04:
        GoldenArcher_CustomCloseProc();
        break;
    }
}

void GoldenArcher_Recv9D(BYTE* Msg, int Size)
{
    if (!g_GoldenArcherCustom) { GA_Ida_RecvScratch(Msg, Size); return; }
    if (Size < 4) return;
    const BYTE r = Msg[3];
    char text[300];
    sprintf_s(text, "%s", GlobalText[r < 4 ? 911 + r : 901]);
    CreateOkMessageBox(text);
    memcpy(GA_GiftName, text, 64);
    GA_GiftName[63] = 0;
}

// Chatter del NPC en el evento propio (DLL MoveCharacter_GoldenArcherChatMessages,
// hook en 0x44AA70).  Devuelve true si lo manejo.
extern "C" bool __cdecl GoldenArcher_CustomNpcIdle(int c, int action)
{
    if (!g_GoldenArcherCustom) return false;
    if (action < 80)       FUN_0043e820(c, 1);
    else if (action < 85)  FUN_0043e820(c, 97);
    else if (action < 90)  FUN_0043e820(c, 99);
    else if (action < 95)  FUN_0043e820(c, 111);
    else if (action < 100) FUN_0043e820(c, 105);
    if (*(BYTE*)(c + 261) != *(BYTE*)(c + 262)) {
        int text = 0;
        if (DAT_0055a7ac == 0 || DAT_0055a7ac == 3) {
            static const int k[] = { 700, 701, 702, 703, 704 };
            text = k[rand() % 5];
        } else if (DAT_0055a7ac == 2) {
            static const int k[] = { 818, 819, 820, 821, 822, 823 };
            text = k[rand() % 6];
        }
        if (text) CreateChat((char*)(c + 0x1C1), GlobalText[text], (DWORD)c, 0, -1);
    }
    return true;
}
