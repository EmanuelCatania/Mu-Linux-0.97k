#include "stdafx.h"
#include "UI/EventTimer.h"
#include "UI/MoveList.h"
#include "Core/Font.h"
#include "Core/Window.h"
#include "UI/UIState.h"

CEventTimer gEventTimer;
namespace {
constexpr int RowsPerPage = 30;
// DESVIACION: layout compacto compartido por dibujo y áreas de clic.
struct EventTimerLayout {
    static constexpr int X = 5, Y = 5, Width = 180, Margin = 5;
    static constexpr int ContentX = X + Margin, ContentWidth = Width - 2 * Margin;
    static constexpr int NameWidth = 110, TimeWidth = 60, TimeX = ContentX + NameWidth;
    static constexpr int TitleY = 12, HeaderY = 28, RowsY = 40, RowStep = 12;
    static constexpr int RowHeight = 10, BaseHeight = 60, FooterY = 45;
    static constexpr int ButtonWidth = ContentWidth / 3;
    static constexpr int PreviousX = ContentX, CloseX = X + (Width - ButtonWidth) / 2;
    static constexpr int NextX = ContentX + ContentWidth - ButtonWidth;
};
using Layout = EventTimerLayout;
enum EventState { Blank, Stand, Open, Start }; // server EventTimeManager.h
bool Inside(int x, int y, int w, int h)
{
    return MouseX >= x && MouseX < x + w && MouseY >= y && MouseY < y + h;
}
void Text(int x, int y, const char* text, int width)
{
    RenderText(x, y, const_cast<char*>(text),
        (int)(width * gWindow.GetWidth() / 640), 1, nullptr);
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
    return !UIState::CanOpenInformationalPanel();
}

int CEventTimer::VisibleRows() const
{
    const int remaining = m_Count - m_Page * RowsPerPage;
    return remaining < RowsPerPage ? remaining : RowsPerPage;
}

void CEventTimer::Toggle()
{
    if (Blocked()) { m_Open = false; return; }
    // DLL: los paneles M y H comparten espacio y se cierran mutuamente.
    if (!m_Open) gMoveList.Close();
    m_Open = !m_Open;
    PlayBuffer(25, 0, 0);
}

void CEventTimer::UpdateMouse()
{
    if (Blocked()) { m_Open = false; return; }
    if (!m_Open) return;
    const int rows = VisibleRows();
    if (!Inside(Layout::X, Layout::Y, Layout::Width, Layout::BaseHeight + rows * Layout::RowStep)) return;
    UIState::CaptureMouseForUI();
    const bool click = MouseLButtonPush != 0;
    MouseLButton = MouseLButtonPush = MouseLButtonPop = 0;
    if (!click) return;
    const int footer = Layout::FooterY + rows * Layout::RowStep;
    const int closeX = m_Count > RowsPerPage ? Layout::CloseX : Layout::ContentX;
    const int closeWidth = m_Count > RowsPerPage ? Layout::ButtonWidth : Layout::ContentWidth;
    if (Inside(closeX, footer, closeWidth, Layout::RowStep)) { Toggle(); return; }
    if (Inside(Layout::PreviousX, footer, Layout::ButtonWidth, Layout::RowStep) && m_Page > 0) --m_Page;
    else if (Inside(Layout::NextX, footer, Layout::ButtonWidth, Layout::RowStep) && (m_Page + 1) * RowsPerPage < m_Count) ++m_Page;
}

void CEventTimer::Render()
{
    if (!m_Open || Blocked()) return;
    const int rows = VisibleRows();
    const DWORD color = m_dwTextColor, back = m_dwBackColor;
    HGDIOBJ font = SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    m_dwBackColor = 0;
    Rect(Layout::X, Layout::Y, Layout::Width, Layout::BaseHeight + rows * Layout::RowStep, 0, 0, 0, .8f);
    m_dwTextColor = 0xFF1ACCFF;
    Text(Layout::ContentX, Layout::TitleY, "Event Timer", Layout::ContentWidth);
    m_dwTextColor = 0xFFFFB27F;
    Text(Layout::ContentX, Layout::HeaderY, "EVENT", Layout::NameWidth);
    Text(Layout::TimeX, Layout::HeaderY, "TIME", Layout::TimeWidth);
    m_dwTextColor = 0xFFFFFFFF;
    if (!m_Count) Text(Layout::ContentX, Layout::RowsY, m_Received ? "NO EVENT INFO" : "WAITING FOR EVENT INFO", Layout::ContentWidth);
    for (int i = 0; i < rows; ++i) {
        const auto& event = m_Events[m_Page * RowsPerPage + i];
        const int y = Layout::RowsY + i * Layout::RowStep;
        if (Inside(Layout::ContentX, y, Layout::ContentWidth, Layout::RowHeight)) Rect(Layout::ContentX, y, Layout::ContentWidth, Layout::RowHeight, .8f, .8f, .1f, .6f);
        m_dwTextColor = 0xFFFFFFFF;
        Text(Layout::ContentX, y, event.name, Layout::NameWidth);
        m_dwTextColor = TimeColor(event);
        char time[32];
        // DLL EventTimer.cpp: mostrar el tiempo recibido, sin adelantar estados localmente.
        FormatTime(event, time, sizeof(time));
        Text(Layout::TimeX, y, time, Layout::TimeWidth);
    }
    m_dwTextColor = 0xFFFFFFFF;
    const int footer = Layout::FooterY + rows * Layout::RowStep;
    const int closeX = m_Count > RowsPerPage ? Layout::CloseX : Layout::ContentX;
    const int closeWidth = m_Count > RowsPerPage ? Layout::ButtonWidth : Layout::ContentWidth;
    // Cierre rojo como en el DLL; dejar lugar a los botones si hay paginación.
    const bool closeHover = Inside(closeX, footer, closeWidth, Layout::RowStep);
    Rect(closeX, footer, closeWidth, Layout::RowStep, closeHover ? 1.0f : .8f, 0, 0, 1);
    Text(closeX, footer, GlobalText[247], closeWidth);
    if (m_Page > 0) Text(Layout::PreviousX, footer, "< Previous", Layout::ButtonWidth);
    if ((m_Page + 1) * RowsPerPage < m_Count) Text(Layout::NextX, footer, "Next >", Layout::ButtonWidth);
    SelectObject(gFont.GetTextDC(), font);
    m_dwTextColor = color;
    m_dwBackColor = back;
    glColor4f(1, 1, 1, 1);
}
