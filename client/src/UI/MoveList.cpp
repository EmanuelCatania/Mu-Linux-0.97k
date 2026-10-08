#include "stdafx.h"
#include "UI/MoveList.h"
#include "UI/EventTimer.h"
#include "Core/Font.h"
#include "Core/Window.h"

CMoveList gMoveList;
namespace {
constexpr int RowsPerPage = 30;
constexpr int Width = 250;
bool Inside(int x, int y, int w, int h)
{
    return MouseX >= x && MouseX < x + w && MouseY >= y && MouseY < y + h;
}
void Text(int x, int y, const char* text)
{
    RenderText(x, y, const_cast<char*>(text), 0, 0, nullptr);
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
void Range(char* out, size_t capacity, const char* label, short low, short high)
{
    char minimum[16], maximum[16];
    if (low == -1) strcpy_s(minimum, "--"); else sprintf_s(minimum, "%d", low);
    if (high == -1) strcpy_s(maximum, "--"); else sprintf_s(maximum, "%d", high);
    sprintf_s(out, capacity, "%s: %s / %s (min/max)", label, minimum, maximum);
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
    // DLL Defines.h: CheckInputInterfaces / CheckRightInterfaces.
    return SceneFlag != 5 || InputEnable || GuildInputEnable || GoldInputEnable ||
        DAT_07e11d71 || DAT_083a7c24 || DAT_07eaa165 || DAT_07eaa117 || DAT_07eaa116 ||
        DAT_07eaa114 || DAT_07eaa115 || DAT_07eaa118 || DAT_07eaa119 ||
        DAT_07eaa11a || DAT_07eaa11b || DAT_07eaa11c || DAT_07eaa124 ||
        _g_bEventChipDialogEnable || ServerDivisionOpened ||
        (g_csQuest && *(BYTE*)((uintptr_t)g_csQuest + 0x1c87f));
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
    if (!Inside(5, 5, Width, 60 + rows * 12)) return;
    DAT_07d78094 = 1; // MouseOnWindow: no caminar al pulsar o mantener el botón.
    const bool click = MouseLButtonPush != 0;
    MouseLButton = 0;
    MouseLButtonPush = 0;
    MouseLButtonPop = 0;
    if (!click) return;
    const int footer = 45 + rows * 12;
    if (Inside(100, footer, 60, 12)) { Toggle(); return; }
    if (m_Count > RowsPerPage) {
        if (Inside(10, footer, 65, 12) && m_Page > 0) { --m_Page; return; }
        if (Inside(180, footer, 70, 12) && (m_Page + 1) * RowsPerPage < m_Count) {
            ++m_Page; return;
        }
    }
    for (int i = 0; i < rows; ++i) {
        if (!Inside(10, 40 + i * 12, Width - 10, 10)) continue;
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
    Rect(5, 5, Width, 60 + rows * 12, 0, 0, 0, .8f);
    m_dwTextColor = 0xFF00FFFF;
    Text(75, 12, "Teleport Window");
    m_dwTextColor = 0xFFFFCC66;
    Text(10, 28, "Map"); Text(126, 28, "Level"); Text(167, 28, "Zen"); Text(227, 28, "VIP");
    m_dwTextColor = 0xFFFFFFFF;
    if (!m_Count) Text(40, 40, m_Received ? "NO MOVE INFO" : "WAITING FOR MOVE INFO");
    int hovered = -1;
    for (int i = 0; i < rows; ++i) {
        const auto& map = m_Maps[m_Page * RowsPerPage + i];
        const int y = 40 + i * 12;
        if (Inside(10, y, Width - 10, 10)) {
            hovered = m_Page * RowsPerPage + i;
            Rect(10, y, Width - 10, 10, .8f, .8f, .1f, .6f);
        }
        m_dwTextColor = map.CanMove ? 0xFFFFFFFF : 0xFF1127A4;
        // El ancho en RenderText se expresa en píxeles físicos, como en el DLL.
        RenderText(10, y, const_cast<char*>(map.MapName),
            (int)(110 * gWindow.GetWidth() / 640), 0, nullptr);
        char text[32];
        if (map.MinLevel == -1) strcpy_s(text, "~"); else sprintf_s(text, "%d", map.MinLevel);
        Text(126, y, text);
        sprintf_s(text, "%lu", (unsigned long)map.Money); Text(167, y, text);
        if (map.AccountLevel > 0) Text(230, y, "*");
    }
    m_dwTextColor = 0xFFFFFFFF;
    const int footer = 45 + rows * 12;
    Text(110, footer, "Close");
    if (m_Count > RowsPerPage) {
        if (m_Page > 0) Text(10, footer, "< Previous");
        if ((m_Page + 1) * RowsPerPage < m_Count) Text(180, footer, "Next >");
    }
    if (hovered >= 0) {
        const auto& map = m_Maps[hovered];
        const int y = 40 + (hovered % RowsPerPage) * 12;
        const int top = y > 340 ? 340 : y;
        Rect(260, top, 280, 88, 0, 0, 0, .9f);
        char text[128];
        Text(265, top + 3, map.MapName);
        Range(text, sizeof(text), "Level", map.MinLevel, map.MaxLevel); Text(265, top + 17, text);
        Range(text, sizeof(text), "Reset", map.MinReset, map.MaxReset); Text(265, top + 31, text);
        sprintf_s(text, "Account: %d   Zen: %lu", map.AccountLevel, (unsigned long)map.Money);
        Text(265, top + 45, text);
        Text(265, top + 59, m_PKLimitFree ? "PK limit: free" : "PK limit: active");
        Text(265, top + 73, "Requirements checked by server");
    }
    SelectObject(gFont.GetTextDC(), font);
    m_dwTextColor = color;
    m_dwBackColor = back;
    glColor4f(1, 1, 1, 1);
}
