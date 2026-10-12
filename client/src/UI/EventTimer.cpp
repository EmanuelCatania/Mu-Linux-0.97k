#include "stdafx.h"
#include "UI/MiniMap.h"
#include "UI/EventTimer.h"
#include "UI/MoveList.h"
#include "UI/UIState.h"
#include "Core/Font.h"
#include "Core/Window.h"
#include "Local/ClientText.h"

void* EventListBox_Construct(int x, int bottom, int width, int visibleRows);
void  EventListBox_Rebuild(void* widget, int count, bool keepPosition);
void  EventListBox_Tick(void* widget);
void  EventListBox_Render(void* widget);
void  EventListBox_Scroll(void* widget, int pages);

CEventTimer gEventTimer;
namespace {
// Misma columna y fondo que RenderGuildList (IDA 0x004F0810): 190x433 en x=450.
struct EventPanelLayout {
    static constexpr int X = 450, Y = 0;
    static constexpr int TitleX = X + 35, TitleY = Y + 12, TitleWidth = 120;
    static constexpr int MessageX = X + 20, MessageY = Y + 50;
    static constexpr int ListX = X + 10, ListBottom = Y + 385, ListWidth = 170, ListRows = 9;
    static constexpr int CloseX = X + 25, CloseY = Y + 395, CloseSize = 24;
    // Fila: recuadro de etiqueta a la izquierda como las stats del panel de
    // personaje y el tiempo centrado debajo.
    static constexpr int BoxWidth = 110, BoxHeight = 21, NameY = 4, TimeY = 22;
};
using Layout = EventPanelLayout;
enum EventState { Blank, Stand, Open, Start }; // server EventTimeManager.h

bool Inside(int x, int y, int w, int h)
{
    return MouseX >= x && MouseX < x + w && MouseY >= y && MouseY < y + h;
}
int PhysicalWidth(int width) { return (int)(width * gWindow.GetWidth() / 640); }
DWORD TimeColor(const Proto::PMSG_EVENT_TIME& event, DWORD remaining)
{
    switch (event.status) {
    case Blank: return 0xFF0505E6;
    case Stand: return remaining <= 300 ? 0xFF0505E6 : 0xFF67BFDF;
    case Open: return 0xFF00B749;
    case Start: return 0xFFFF9664;
    default: return 0xFF67BFDF;
    }
}
}

void EventTimer_DrawRow(int index, int x, int y, int width)
{
    gEventTimer.DrawRow(index, x, y, width);
}

void* CEventTimer::Widget()
{
    if (!m_Widget)
        m_Widget = EventListBox_Construct(Layout::ListX, Layout::ListBottom,
                                          Layout::ListWidth, Layout::ListRows);
    return m_Widget;
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
    // Los refrescos periódicos sólo traen tiempos nuevos: la lista del widget se
    // rearma únicamente si cambian la cantidad o los nombres.
    bool changed = count != m_Count;
    for (int i = 0; !changed && i < count; ++i)
        changed = strcmp(m_Events[i].name, (const char*)packet + header + i * stride) != 0;
    if (count) memcpy(m_Events, packet + header, count * stride);
    const bool hadList = m_Received;
    m_Count = count;
    m_Received = true;
    m_ReceivedAt = GetTickCount();
    if (changed) EventListBox_Rebuild(Widget(), m_Count, hadList);
    return true;
}

void CEventTimer::Clear()
{
    m_Count = 0;
    m_Open = false;
    m_Received = false;
    if (m_Widget) EventListBox_Rebuild(m_Widget, 0, false);
}

void CEventTimer::Close()
{
    m_Open = false;
}

const Proto::PMSG_EVENT_TIME* CEventTimer::Get(int index) const
{
    return index >= 0 && index < m_Count ? &m_Events[index] : nullptr;
}

DWORD CEventTimer::RemainingSeconds(const Proto::PMSG_EVENT_TIME& event) const
{
    // El server escribe RemainTime (int) en un DWORD; negativo = vencido.
    const DWORD sent = event.time <= 0x7FFFFFFF ? event.time : 0;
    const DWORD elapsed = (GetTickCount() - m_ReceivedAt) / 1000;
    return sent > elapsed ? sent - elapsed : 0;
}

void CEventTimer::FormatTime(const Proto::PMSG_EVENT_TIME& event, char* text, size_t capacity) const
{
    switch (event.status) {
    case Blank: strcpy_s(text, capacity, gClientText.Get(ClientTextId::EventDisabled)); break;
    case Open: strcpy_s(text, capacity, gClientText.Get(ClientTextId::EventOpen)); break;
    case Start: strcpy_s(text, capacity, gClientText.Get(ClientTextId::EventStarted)); break;
    case Stand: {
        // El estado lo decide el server: al llegar a cero se muestra 00:00:00
        // hasta el próximo F3/E6, sin adelantar el cambio a abierto.
        const DWORD seconds = RemainingSeconds(event);
        if (seconds >= 86400)
            sprintf_s(text, capacity, gClientText.Get(ClientTextId::EventDays),
                      (unsigned long)(seconds / 86400));
        else
            sprintf_s(text, capacity, "%02lu:%02lu:%02lu", (unsigned long)(seconds / 3600),
                      (unsigned long)((seconds / 60) % 60), (unsigned long)(seconds % 60));
        break;
    }
    default: strcpy_s(text, capacity, "--"); break;
    }
}

void CEventTimer::Toggle()
{
    if (m_Open) {
        Close();
        PlayBuffer(25, 0, 0);
        PlayBuffer(28, 0, 0);
        return;
    }
    if (!UIState::CanOpenSidePanel()) return;
    // Comparte la columna x=450 con inventario/personaje/guild/party: se alternan.
    if (InventoryOpened && (int)DAT_07e91388 > 0) Item_ReturnPickedItem();
    InventoryOpened = 0;
    CharacterOpened = 0;
    GuildOpened = 0;
    PartyOpened = 0;
    gMoveList.Close();
    gMiniMap.Close();
    m_Open = true;
    // Mismo par de sonidos que al abrir el inventario (Chat_InputTick 0x004B14F0).
    PlayBuffer(25, 0, 0);
    PlayBuffer(28, 0, 0);
}

void CEventTimer::ScrollPages(int pages)
{
    if (m_Open) EventListBox_Scroll(Widget(), pages);
}

void CEventTimer::UpdateMouse()
{
    if (!m_Open) return;
    // El último panel abierto gana: si se abre otro de la columna, H se cierra.
    if (SceneFlag != 5 || UIState::HasRightPanel()) { Close(); return; }
    EventListBox_Tick(Widget());
    if (UIState::HasModalDialog() || !MouseLButtonPush) return;
    if (Inside(Layout::CloseX, Layout::CloseY, Layout::CloseSize, Layout::CloseSize)) {
        MouseLButtonPush = 0;
        Close();
        PlayBuffer(25, 0, 0);
        PlayBuffer(28, 0, 0);
    }
}

void CEventTimer::DrawRow(int index, int x, int y, int width) const
{
    const auto* event = Get(index);
    if (!event) return;
    // Recuadro 245 de las filas de stats del panel de personaje
    // (RenderCharacterInfoWindow): 75x21 útiles en una textura de 128x32.  Para
    // el ancho de la fila se estira sólo el centro y se conservan los bordes.
    constexpr float U = 1.0f / 128.0f, V = 0.65625f, Cap = 12.0f, Used = 75.0f;
    const float fx = (float)x - 4.0f, fy = (float)y, fw = (float)Layout::BoxWidth;
    glColor3f(1.0f, 1.0f, 1.0f);
    GL_DrawTexture(245, fx, fy, Cap, (float)Layout::BoxHeight, 0.0f, 0.0f, Cap * U, V, 1, 1);
    GL_DrawTexture(245, fx + Cap, fy, fw - 2 * Cap, (float)Layout::BoxHeight,
                   Cap * U, 0.0f, (Used - 2 * Cap) * U, V, 1, 1);
    GL_DrawTexture(245, fx + fw - Cap, fy, Cap, (float)Layout::BoxHeight,
                   (Used - Cap) * U, 0.0f, Cap * U, V, 1, 1);
    EnableAlphaTest(true);

    m_dwBackColor = 0;
    m_dwTextColor = 0xFFE6E6E6u;
    SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_BOLD));
    RenderText(x, y + Layout::NameY, const_cast<char*>(event->name),
               PhysicalWidth(Layout::BoxWidth - 8), 1, nullptr);
    char time[32];
    FormatTime(*event, time, sizeof(time));
    m_dwTextColor = TimeColor(*event, RemainingSeconds(*event));
    SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    (void)width;  // el tiempo va centrado bajo el recuadro, no en toda la fila
    RenderText(x, y + Layout::TimeY, time, PhysicalWidth(Layout::BoxWidth - 8), 1, nullptr);
}

void CEventTimer::RenderPanel()
{
    if (!m_Open) return;
    const DWORD color = m_dwTextColor, back = m_dwBackColor;
    glColor3f(1.0f, 1.0f, 1.0f);
    GL_ResetState();
    const float x = (float)Layout::X, y = (float)Layout::Y;
    GL_DrawTexture(260, x, y,          190.0f, 256.0f, 0.0f, 0.0f, 0.7421875f, 1.0f,        1, 1);
    GL_DrawTexture(261, x, y + 256.0f, 190.0f, 177.0f, 0.0f, 0.0f, 0.7421875f, 0.69140625f, 1, 1);
    EnableAlphaTest(true);

    m_dwBackColor = 0xFF141414u;
    m_dwTextColor = 0xFFDCDCDCu;
    SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_BOLD));
    RenderText(Layout::TitleX, Layout::TitleY,
               const_cast<char*>(gClientText.Get(ClientTextId::EventTitle)),
               PhysicalWidth(Layout::TitleWidth), 1, (SIZE*)3);

    if (m_Count) {
        EventListBox_Render(Widget());
    } else {
        m_dwBackColor = 0;
        m_dwTextColor = 0xFFE6E6E6u;
        SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
        RenderText(Layout::MessageX, Layout::MessageY, const_cast<char*>(gClientText.Get(
            m_Received ? ClientTextId::EventEmpty : ClientTextId::EventWaiting)), 0, 0, nullptr);
    }

    // Botón de cierre como el del panel de personaje (fondo 0x118 + icono 280).
    const float cx = (float)Layout::CloseX, cy = (float)Layout::CloseY;
    glColor3f(1.0f, 1.0f, 1.0f);
    GL_DrawTexture(0x118, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    GL_DrawTexture(280, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    if (Inside(Layout::CloseX, Layout::CloseY, Layout::CloseSize, Layout::CloseSize)) {
        SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
        m_dwTextColor = 0xFFFFFFFFu;
        m_dwBackColor = 0xFF000000u;
        RenderTipText(Layout::CloseX, Layout::CloseY - 13, GlobalText[247]);
    }
    SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    m_dwTextColor = color;
    m_dwBackColor = back;
    GL_ResetState();
    glColor4f(1, 1, 1, 1);
}
