#include "stdafx.h"
#include "UI/EventTimer.h"
#include "Core/Font.h"
#include "Core/Window.h"

CEventTimer gEventTimer;
namespace {
constexpr int RowsPerPage = 30;
constexpr int Width = 250;
enum EventState { Blank, Stand, Open, Start }; // server EventTimeManager.h
bool Inside(int x, int y, int w, int h)
{
    return MouseX >= x && MouseX < x + w && MouseY >= y && MouseY < y + h;
}
void Text(int x, int y, const char* text, int width = 0)
{
    RenderText(x, y, const_cast<char*>(text),
        (int)(width * gWindow.GetWidth() / 640), 0, nullptr);
}
void Rect(int x, int y, int w, int h, float r, float g, float b, float a)
{
    EnableAlphaTest(true);
    glColor4f(r, g, b, a);
    GL_DrawRect((float)x, (float)y, (float)w, (float)h);
    GL_ResetState();
    // DLL EventTimer.cpp: el texto no debe heredar el color del fondo o del hover.
    glColor4f(1, 1, 1, 1);
}
DWORD TimeColor(const Proto::PMSG_EVENT_TIME& event)
{
    switch (event.status) {
    case Blank: return 0xFF0505E6;
    case Stand: return event.time <= 300 ? 0xFF0505E6 : 0xFF67BFDF;
    case Open: return 0xFF00B749;
    case Start: return 0xFFFF9664;
    default: return 0xFF67BFDF;
    }
}
}

bool CEventTimer::Receive(const BYTE* packet, int size)
{
    const int header = sizeof(Proto::PMSG_EVENT_TIME_SEND);
    const int stride = sizeof(Proto::PMSG_EVENT_TIME);
    if (!packet || size < header || packet[0] != 0xC2 ||
        packet[3] != 0xF3 || packet[4] != 0xE6 ||
        ((packet[1] << 8) | packet[2]) != size) return false;
    const int count = packet[5];
    if (size != header + count * stride) return false;
    // Validar la lista completa antes de publicar una actualización.
    for (int i = 0; i < count; ++i) {
        const BYTE* row = packet + header + i * stride;
        if (row[0] == 0 || !memchr(row, 0, 32)) return false;
        for (int n = 0; n < 32 && row[n]; ++n)
            if (row[n] < 32 || row[n] == 127) return false;
    }
    if (count) memcpy(m_Events, packet + header, count * stride);
    m_Count = count;
    m_Received = true;
    // Los refrescos periódicos no deben devolver al usuario a la primera página.
    if (m_Page * RowsPerPage >= m_Count) m_Page = m_Count ? (m_Count - 1) / RowsPerPage : 0;
    return true;
}

void CEventTimer::Clear()
{
    m_Count = 0;
    m_Page = 0;
    m_Open = false;
    m_Received = false;
}

const Proto::PMSG_EVENT_TIME* CEventTimer::Get(int index) const
{
    return index >= 0 && index < m_Count ? &m_Events[index] : nullptr;
}

void CEventTimer::FormatTime(const Proto::PMSG_EVENT_TIME& event, char* text, size_t capacity)
{
    switch (event.status) {
    case Blank: sprintf_s(text, capacity, "Disabled"); break;
    case Open: sprintf_s(text, capacity, "Open now"); break;
    case Start: sprintf_s(text, capacity, "Started"); break;
    case Stand: {
        // El server escribe RemainTime (int) en DWORD; un atraso negativo no son miles de días.
        const DWORD seconds = event.time <= 0x7FFFFFFF ? event.time : 0;
        if (seconds >= 86400) sprintf_s(text, capacity, "%02lu days", (unsigned long)(seconds / 86400));
        else sprintf_s(text, capacity, "%02lu:%02lu:%02lu", (unsigned long)(seconds / 3600),
            (unsigned long)((seconds / 60) % 60), (unsigned long)(seconds % 60));
        break;
    }
    default: sprintf_s(text, capacity, "--"); break;
    }
}

bool CEventTimer::Blocked() const
{
    // DLL Defines.h: CheckInputInterfaces / CheckRightInterfaces.
    return SceneFlag != 5 || InputEnable || GuildInputEnable || GoldInputEnable ||
        DAT_07e11d71 || DAT_083a7c24 || DAT_07eaa165 || DAT_07eaa117 || DAT_07eaa116 ||
        DAT_07eaa114 || DAT_07eaa115 || DAT_07eaa118 || DAT_07eaa119 ||
        DAT_07eaa11a || DAT_07eaa11b || DAT_07eaa11c || DAT_07eaa124 ||
        _g_bEventChipDialogEnable || ServerDivisionOpened ||
        (g_csQuest && *(BYTE*)((uintptr_t)g_csQuest + 0x1c87f));
}

int CEventTimer::VisibleRows() const
{
    const int remaining = m_Count - m_Page * RowsPerPage;
    return remaining < RowsPerPage ? remaining : RowsPerPage;
}

void CEventTimer::Toggle()
{
    if (Blocked()) { m_Open = false; return; }
    m_Open = !m_Open;
    PlayBuffer(25, 0, 0);
}

void CEventTimer::UpdateMouse()
{
    if (Blocked()) { m_Open = false; return; }
    if (!m_Open) return;
    const int rows = VisibleRows();
    if (!Inside(5, 5, Width, 60 + rows * 12)) return;
    DAT_07d78094 = 1; // MouseOnWindow: el clic del panel no llega al mundo.
    const bool click = MouseLButtonPush != 0;
    MouseLButton = MouseLButtonPush = MouseLButtonPop = 0;
    if (!click) return;
    const int footer = 45 + rows * 12;
    if (Inside(100, footer, 60, 12)) { Toggle(); return; }
    if (Inside(10, footer, 65, 12) && m_Page > 0) --m_Page;
    else if (Inside(180, footer, 70, 12) && (m_Page + 1) * RowsPerPage < m_Count) ++m_Page;
}

void CEventTimer::Render()
{
    if (!m_Open || Blocked()) return;
    const int rows = VisibleRows();
    const DWORD color = m_dwTextColor, back = m_dwBackColor;
    HGDIOBJ font = SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    m_dwBackColor = 0;
    Rect(5, 5, Width, 60 + rows * 12, 0, 0, 0, .8f);
    m_dwTextColor = 0xFF1ACCFF;
    Text(85, 12, "Event Timer");
    m_dwTextColor = 0xFFFFB27F;
    Text(10, 28, "EVENT"); Text(180, 28, "TIME");
    m_dwTextColor = 0xFFFFFFFF;
    if (!m_Count) Text(40, 40, m_Received ? "NO EVENT INFO" : "WAITING FOR EVENT INFO");
    for (int i = 0; i < rows; ++i) {
        const auto& event = m_Events[m_Page * RowsPerPage + i];
        const int y = 40 + i * 12;
        if (Inside(10, y, Width - 10, 10)) Rect(10, y, Width - 10, 10, .8f, .8f, .1f, .6f);
        m_dwTextColor = 0xFFFFFFFF;
        Text(10, y, event.name, 160);
        m_dwTextColor = TimeColor(event);
        char time[32];
        // DLL EventTimer.cpp: mostrar el tiempo recibido, sin adelantar estados localmente.
        FormatTime(event, time, sizeof(time));
        Text(180, y, time, 70);
    }
    m_dwTextColor = 0xFFFFFFFF;
    const int footer = 45 + rows * 12;
    Text(110, footer, "Close");
    if (m_Page > 0) Text(10, footer, "< Previous");
    if ((m_Page + 1) * RowsPerPage < m_Count) Text(180, footer, "Next >");
    SelectObject(gFont.GetTextDC(), font);
    m_dwTextColor = color;
    m_dwBackColor = back;
    glColor4f(1, 1, 1, 1);
}
