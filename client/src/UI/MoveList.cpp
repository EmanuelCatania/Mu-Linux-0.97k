#include "stdafx.h"
#include "UI/MoveList.h"
#include "UI/EventTimer.h"
#include "Core/Font.h"
#include "Core/Window.h"
#include "UI/UIState.h"

CMoveList gMoveList;
namespace {
constexpr int RowsPerPage = 30;
// DESVIACION: panel compacto de 180 unidades, con columnas y título centrados.
struct MoveListLayout {
    static constexpr int X = 5, Y = 5, Width = 180, Margin = 5;
    static constexpr int ContentX = X + Margin, ContentWidth = Width - 2 * Margin;
    static constexpr int MapWidth = 55, LevelWidth = 30, ZenWidth = 60, VipWidth = 25;
    static constexpr int MapX = ContentX, LevelX = MapX + MapWidth;
    static constexpr int ZenX = LevelX + LevelWidth, VipX = ZenX + ZenWidth;
    static constexpr int TitleY = 12, HeaderY = 28, RowsY = 40, RowStep = 12;
    static constexpr int RowHeight = 10, BaseHeight = 60, FooterY = 45;
    static constexpr int ButtonWidth = ContentWidth / 3;
    static constexpr int PreviousX = ContentX, CloseX = X + (Width - ButtonWidth) / 2;
    static constexpr int NextX = ContentX + ContentWidth - ButtonWidth;
};
using Layout = MoveListLayout;
bool Inside(int x, int y, int w, int h)
{
    return MouseX >= x && MouseX < x + w && MouseY >= y && MouseY < y + h;
}
void Text(int x, int y, const char* text, int width)
{
    // DLL MoveList.cpp: centrar dentro de cada columna; ancho físico para RenderText.
    RenderText(x, y, const_cast<char*>(text),
        (int)(width * gWindow.GetWidth() / 640), 1, nullptr);
}
void Rect(int x, int y, int w, int h, float r, float g, float b, float a)
{
    EnableAlphaTest(true);
    glColor4f(r, g, b, a);
    GL_DrawRect((float)x, (float)y, (float)w, (float)h);
    GL_ResetState();
    // DLL MoveList.cpp: el texto no debe heredar el color del fondo o del hover.
    glColor4f(1, 1, 1, 1);
}
}

bool CMoveList::Receive(const BYTE* packet, int size)
{
    const int header = sizeof(Proto::PMSG_MOVE_LIST_SEND);
    const int stride = sizeof(Proto::MOVE_LIST_INFO);
    if (!packet || size < header || packet[0] != 0xC2 ||
        packet[3] != 0xF3 || packet[4] != 0xE5 ||
        ((packet[1] << 8) | packet[2]) != size) return false;
    const int count = packet[6];
    if (size != header + count * stride) return false;
    // Validar toda la lista antes de reemplazarla, incluido el bool en el wire.
    for (int i = 0; i < count; ++i) {
        const BYTE* row = packet + header + i * stride;
        if (row[offsetof(Proto::MOVE_LIST_INFO, CanMove)] > 1 || row[1] == 0 ||
            !memchr(row + 1, 0, 32)) return false;
        for (int n = 1; n < 33 && row[n]; ++n)
            if (row[n] < 32 || row[n] == 127) return false;
    }
    if (count) memcpy(m_Maps, packet + header, count * stride);
    m_Count = count;
    m_PKLimitFree = packet[5];
    m_Received = true;
    m_Page = 0;
    return true;
}

void CMoveList::Clear()
{
    m_Count = 0;
    m_Page = 0;
    m_Open = false;
    m_Received = false;
    m_PKLimitFree = 0;
}

const Proto::MOVE_LIST_INFO* CMoveList::Get(int index) const
{
    return index >= 0 && index < m_Count ? &m_Maps[index] : nullptr;
}

bool CMoveList::Blocked() const
{
    return !UIState::CanOpenInformationalPanel();
}

int CMoveList::VisibleRows() const
{
    const int remaining = m_Count - m_Page * RowsPerPage;
    return remaining < RowsPerPage ? remaining : RowsPerPage;
}

void CMoveList::Toggle()
{
    if (Blocked()) { m_Open = false; return; }
    // DLL: los paneles M y H comparten espacio y se cierran mutuamente.
    if (!m_Open) gEventTimer.Close();
    m_Open = !m_Open;
    PlayBuffer(25, 0, 0);
}

void CMoveList::UpdateMouse()
{
    if (Blocked()) { m_Open = false; return; }
    if (!m_Open) return;
    const int rows = VisibleRows();
    if (!Inside(Layout::X, Layout::Y, Layout::Width, Layout::BaseHeight + rows * Layout::RowStep)) return;
    UIState::CaptureMouseForUI();
    const bool click = MouseLButtonPush != 0;
    MouseLButton = 0;
    MouseLButtonPush = 0;
    MouseLButtonPop = 0;
    if (!click) return;
    const int footer = Layout::FooterY + rows * Layout::RowStep;
    if (Inside(Layout::CloseX, footer, Layout::ButtonWidth, Layout::RowStep)) { Toggle(); return; }
    if (m_Count > RowsPerPage) {
        if (Inside(Layout::PreviousX, footer, Layout::ButtonWidth, Layout::RowStep) && m_Page > 0) { --m_Page; return; }
        if (Inside(Layout::NextX, footer, Layout::ButtonWidth, Layout::RowStep) && (m_Page + 1) * RowsPerPage < m_Count) {
            ++m_Page; return;
        }
    }
    for (int i = 0; i < rows; ++i) {
        if (!Inside(Layout::ContentX, Layout::RowsY + i * Layout::RowStep, Layout::ContentWidth, Layout::RowHeight)) continue;
        const auto& map = m_Maps[m_Page * RowsPerPage + i];
        if (!map.CanMove) return;
        char command[64];
        sprintf_s(command, "/move %s", map.MapName);
        // DESVIACION (DLL MoveList.cpp): el server valida saldo, PK, equipo y requisitos.
        SendChat(command); // C1/00 mediante gNetwork; conserva el límite de frecuencia.
        Toggle();
        return;
    }
}

void CMoveList::Render()
{
    if (!m_Open || Blocked()) return;
    const int rows = VisibleRows();
    const DWORD color = m_dwTextColor, back = m_dwBackColor;
    HGDIOBJ font = SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    m_dwBackColor = 0;
    Rect(Layout::X, Layout::Y, Layout::Width, Layout::BaseHeight + rows * Layout::RowStep, 0, 0, 0, .8f);
    m_dwTextColor = 0xFF00FFFF;
    Text(Layout::ContentX, Layout::TitleY, "Teleport Window", Layout::ContentWidth);
    m_dwTextColor = 0xFFFFCC66;
    Text(Layout::MapX, Layout::HeaderY, "Map", Layout::MapWidth);
    Text(Layout::LevelX, Layout::HeaderY, GlobalText[161], Layout::LevelWidth);
    Text(Layout::ZenX, Layout::HeaderY, GlobalText[100], Layout::ZenWidth);
    Text(Layout::VipX, Layout::HeaderY, "VIP", Layout::VipWidth);
    m_dwTextColor = 0xFFFFFFFF;
    if (!m_Count) Text(Layout::ContentX, Layout::RowsY, m_Received ? "NO MOVE INFO" : "WAITING FOR MOVE INFO", Layout::ContentWidth);
    for (int i = 0; i < rows; ++i) {
        const auto& map = m_Maps[m_Page * RowsPerPage + i];
        const int y = Layout::RowsY + i * Layout::RowStep;
        if (Inside(Layout::ContentX, y, Layout::ContentWidth, Layout::RowHeight)) {
            Rect(Layout::ContentX, y, Layout::ContentWidth, Layout::RowHeight, .8f, .8f, .1f, .6f);
        }
        m_dwTextColor = map.CanMove ? 0xFFFFFFFF : 0xFF1127A4;
        Text(Layout::MapX, y, map.MapName, Layout::MapWidth);
        char text[32];
        if (map.MinLevel == -1) strcpy_s(text, "~"); else sprintf_s(text, "%d", map.MinLevel);
        Text(Layout::LevelX, y, text, Layout::LevelWidth);
        sprintf_s(text, "%lu", (unsigned long)map.Money); Text(Layout::ZenX, y, text, Layout::ZenWidth);
        if (map.AccountLevel > 0) Text(Layout::VipX, y, "*", Layout::VipWidth);
    }
    m_dwTextColor = 0xFFFFFFFF;
    const int footer = Layout::FooterY + rows * Layout::RowStep;
    Text(Layout::CloseX, footer, GlobalText[247], Layout::ButtonWidth);
    if (m_Count > RowsPerPage) {
        if (m_Page > 0) Text(Layout::PreviousX, footer, "< Previous", Layout::ButtonWidth);
        if ((m_Page + 1) * RowsPerPage < m_Count) Text(Layout::NextX, footer, "Next >", Layout::ButtonWidth);
    }
    SelectObject(gFont.GetTextDC(), font);
    m_dwTextColor = color;
    m_dwBackColor = back;
    glColor4f(1, 1, 1, 1);
}
