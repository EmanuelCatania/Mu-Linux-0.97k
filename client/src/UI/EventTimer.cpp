#include "stdafx.h"
#include "Net/NetText.h"
#include "Render/Gfx.h"
#include "Platform/Text.h"
#include "Platform/ITimer.h"
#include "UI/MiniMap.h"
#include "UI/EventTimer.h"
#include "UI/MoveList.h"
#include "UI/UIState.h"
#include "Core/Font.h"
#include "Core/Window.h"
#include "Local/ClientText.h"

CEventTimer gEventTimer;
namespace {
// Misma columna y fondo que RenderGuildList (IDA 0x004F0810): 190x433 en x=450.
// DESVIACION: filas paginadas como el panel de eventos de SSeMU: recuadro oscuro
// de todo el ancho, nombre y tiempo centrados, "(1/2)" con flechas abajo.
struct EventPanelLayout {
    // DESVIACION (Fase 4, pantalla ancha): pegado al borde derecho de la ventana.
    static int X() { return gWindow.PanelOrigin(Ui::EventTimer); }
    static constexpr int Y = 0;
    static int TitleX() { return X() + 35; }
    static constexpr int TitleY = Y + 12, TitleWidth = 120;
    static int MessageX() { return X() + 20; }
    static constexpr int MessageY = Y + 50;
    static constexpr int RowWidth = 136;
    static int RowX() { return X() + (190 - RowWidth) / 2; }
    static constexpr int RowY = Y + 50, ListBottom = Y + 362, MaxRowsPerPage = 15;
    // La fuente no escala con el layout de 640x480: el alto de la fila sale
    // de la línea de texto real llevada a esas unidades (dos líneas + margen).
    static int LineHeight() {
        const float rate = gWindow.GetScreenRateY() > 0.0f ? gWindow.GetScreenRateY() : 1.0f;
        const int line = (int)((float)gFont.GetHeight() / rate + 0.999f);
        return line > 6 ? line : 6;
    }
    static int RowHeight() { return LineHeight() * 2 + 4; }
    static int RowStep() { return RowHeight() + 3; }
    static int RowsPerPage() {
        const int rows = (ListBottom - RowY + 3) / RowStep();
        return rows < 1 ? 1 : rows > MaxRowsPerPage ? MaxRowsPerPage : rows;
    }
    static constexpr int NameY = 2;
    static int TimeY() { return NameY + LineHeight(); }
    static constexpr int PagerY = Y + 368, ButtonSize = 16;
    static int PreviousX() { return X() + 18; }
    static int NextX() { return X() + 156; }
    static int PageTextX() { return X() + 40; }
    static constexpr int PageTextWidth = 110;
    static int CloseX() { return X() + 25; }
    static constexpr int CloseY = Y + 395, CloseSize = 24;
};
using Layout = EventPanelLayout;
enum EventState { Blank, Stand, Open, Start }; // server EventTimeManager.h

bool Inside(int x, int y, int w, int h)
{
    return MouseX >= x && MouseX < x + w && MouseY >= y && MouseY < y + h;
}
int PhysicalWidth(int width) { return (int)(width * gWindow.GetWidth() / 640); }
// Recuadro negro translúcido, como el fondo del menú M (MoveList).
void DrawBox(float x, float y, float w, float h, float fill, float alpha)
{
    EnableAlphaTest(true);
    Gfx::Color4f(fill, fill, fill, alpha);
    GL_DrawRect(x, y, w, h);
    Gfx::Color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GL_ResetState();
}
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
    // DESVIACION (Fase 4, texto Unicode): los nombres llegan en 1252 y se
    // guardan en UTF-8; la comparación se hace sobre el nombre ya convertido.
    for (int i = 0; !changed && i < count; ++i) {
        char name[sizeof(m_Events[i].name)];
        NetText::FromServer(name, sizeof(name), packet + header + i * stride, sizeof(name));
        changed = strcmp(m_Events[i].name, name) != 0;
    }
    if (count) memcpy(m_Events, packet + header, count * stride);
    for (int i = 0; i < count; ++i)
        TextEngine::ConvertWindows1252InPlace(m_Events[i].name, sizeof(m_Events[i].name));
    m_Count = count;
    m_Received = true;
    m_ReceivedAt = Platform::GetTimer().Ticks();
    // Un cambio de lista vuelve a la primera página; los refrescos la conservan.
    if (changed) m_Page = 0;
    if (m_Page >= PageCount()) m_Page = PageCount() - 1;
    return true;
}

void CEventTimer::Clear()
{
    m_Count = 0;
    m_Open = false;
    m_Received = false;
    m_Page = 0;
}

void CEventTimer::Close()
{
    m_Open = false;
}

int CEventTimer::PageCount() const
{
    const int perPage = Layout::RowsPerPage();
    const int pages = (m_Count + perPage - 1) / perPage;
    return pages > 0 ? pages : 1;
}

DWORD CEventTimer::RemainingSeconds(const Proto::PMSG_EVENT_TIME& event) const
{
    // El server escribe RemainTime (int) en un DWORD; negativo = vencido.
    const DWORD sent = event.time <= 0x7FFFFFFF ? event.time : 0;
    const DWORD elapsed = (Platform::GetTimer().Ticks() - m_ReceivedAt) / 1000;
    return sent > elapsed ? sent - elapsed : 0;
}

void CEventTimer::FormatTime(const Proto::PMSG_EVENT_TIME& event, char* text, size_t capacity) const
{
    switch (event.status) {
    case Blank: Platform::TextCopySecure(text, capacity, gClientText.Get(ClientTextId::EventDisabled)); break;
    case Open: Platform::TextCopySecure(text, capacity, gClientText.Get(ClientTextId::EventOpen)); break;
    case Start: Platform::TextCopySecure(text, capacity, gClientText.Get(ClientTextId::EventStarted)); break;
    case Stand: {
        // El estado lo decide el server: al llegar a cero se muestra 00:00:00
        // hasta el próximo F3/E6, sin adelantar el cambio a abierto.
        const DWORD seconds = RemainingSeconds(event);
        char remaining[32];
        if (seconds >= 86400)
            Platform::TextFormatSecure(remaining, sizeof(remaining), gClientText.Get(ClientTextId::EventDays),
                      (unsigned long)(seconds / 86400));
        else
            Platform::TextFormatSecure(remaining, sizeof(remaining), "%02lu:%02lu:%02lu", (unsigned long)(seconds / 3600),
                      (unsigned long)((seconds / 60) % 60), (unsigned long)(seconds % 60));
        Platform::TextFormatSecure(text, capacity, gClientText.Get(ClientTextId::EventStartsIn), remaining);
        break;
    }
    default: Platform::TextCopySecure(text, capacity, "--"); break;
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
    if (!m_Open) return;
    int page = m_Page + pages;
    if (page < 0) page = 0;
    if (page >= PageCount()) page = PageCount() - 1;
    if (page != m_Page) { m_Page = page; PlayBuffer(25, 0, 0); }
}

void CEventTimer::UpdateMouse()
{
    if (!m_Open) return;
    // El último panel abierto gana: si se abre otro de la columna, H se cierra.
    if (SceneFlag != 5 || UIState::HasRightPanel()) { Close(); return; }
    if (UIState::HasModalDialog() || !MouseLButtonPush) return;
    if (m_Page > 0 && Inside(Layout::PreviousX(), Layout::PagerY, Layout::ButtonSize, Layout::ButtonSize)) {
        MouseLButtonPush = 0;
        ScrollPages(-1);
        return;
    }
    if (m_Page + 1 < PageCount() && Inside(Layout::NextX(), Layout::PagerY, Layout::ButtonSize, Layout::ButtonSize)) {
        MouseLButtonPush = 0;
        ScrollPages(1);
        return;
    }
    if (Inside(Layout::CloseX(), Layout::CloseY, Layout::CloseSize, Layout::CloseSize)) {
        MouseLButtonPush = 0;
        Close();
        PlayBuffer(25, 0, 0);
        PlayBuffer(28, 0, 0);
    }
}

void CEventTimer::DrawRow(const Proto::PMSG_EVENT_TIME& event, int y) const
{
    const int x = Layout::RowX();
    const bool hover = Inside(x, y, Layout::RowWidth, Layout::RowHeight());
    DrawBox((float)x, (float)y, (float)Layout::RowWidth, (float)Layout::RowHeight(),
            hover ? 0.15f : 0.0f, 0.75f);
    EnableAlphaTest(true);
    m_dwBackColor = 0;
    m_dwTextColor = TimeColor(event, RemainingSeconds(event));
    gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_BOLD));
    RenderText(x, y + Layout::NameY, const_cast<char*>(event.name),
               PhysicalWidth(Layout::RowWidth), 1, nullptr);
    char time[64];
    FormatTime(event, time, sizeof(time));
    gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    RenderText(x, y + Layout::TimeY(), time, PhysicalWidth(Layout::RowWidth), 1, nullptr);
}

void CEventTimer::DrawPager() const
{
    if (m_Count <= Layout::RowsPerPage()) return;
    const int pages = PageCount();
    char text[16];
    Platform::TextFormatSecure(text, sizeof(text), "(%d/%d)", m_Page + 1, pages);
    EnableAlphaTest(true);
    m_dwBackColor = 0;
    m_dwTextColor = 0xFFE6E6E6u;
    gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    RenderText(Layout::PageTextX(), Layout::PagerY + 2, text, PhysicalWidth(Layout::PageTextWidth), 1, nullptr);
    struct Button { bool enabled; int x; const char* label; };
    const Button buttons[] = { { m_Page > 0, Layout::PreviousX(), "<" },
                               { m_Page + 1 < pages, Layout::NextX(), ">" } };
    for (const Button& button : buttons) {
        if (!button.enabled) continue;
        const bool hover = Inside(button.x, Layout::PagerY, Layout::ButtonSize, Layout::ButtonSize);
        DrawBox((float)button.x, (float)Layout::PagerY, (float)Layout::ButtonSize, (float)Layout::ButtonSize,
                hover ? 0.2f : 0.0f, 0.8f);
        EnableAlphaTest(true);
        m_dwTextColor = 0xFFFFFFFFu;
        gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_BOLD));
        RenderText(button.x, Layout::PagerY + 2, const_cast<char*>(button.label),
                   PhysicalWidth(Layout::ButtonSize), 1, nullptr);
    }
}

void CEventTimer::RenderPanel()
{
    if (!m_Open) return;
    const DWORD color = m_dwTextColor, back = m_dwBackColor;
    Gfx::Color3f(1.0f, 1.0f, 1.0f);
    GL_ResetState();
    const float x = (float)Layout::X(), y = (float)Layout::Y;
    GL_DrawTexture(260, x, y,          190.0f, 256.0f, 0.0f, 0.0f, 0.7421875f, 1.0f,        1, 1);
    GL_DrawTexture(261, x, y + 256.0f, 190.0f, 177.0f, 0.0f, 0.0f, 0.7421875f, 0.69140625f, 1, 1);
    EnableAlphaTest(true);

    m_dwBackColor = 0xFF141414u;
    m_dwTextColor = 0xFFDCDCDCu;
    gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_BOLD));
    RenderText(Layout::TitleX(), Layout::TitleY,
               const_cast<char*>(gClientText.Get(ClientTextId::EventTitle)),
               PhysicalWidth(Layout::TitleWidth), 1, (SIZE*)3);

    if (m_Count) {
        if (m_Page >= PageCount()) m_Page = PageCount() - 1;   // cambio de resolución
        const int perPage = Layout::RowsPerPage(), first = m_Page * perPage;
        for (int row = 0; row < perPage && first + row < m_Count; ++row)
            DrawRow(m_Events[first + row], Layout::RowY + row * Layout::RowStep());
        DrawPager();
    } else {
        m_dwBackColor = 0;
        m_dwTextColor = 0xFFE6E6E6u;
        gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
        RenderText(Layout::MessageX(), Layout::MessageY, const_cast<char*>(gClientText.Get(
            m_Received ? ClientTextId::EventEmpty : ClientTextId::EventWaiting)), 0, 0, nullptr);
    }

    // Botón de cierre como el del panel de personaje (fondo 0x118 + icono 280).
    const float cx = (float)Layout::CloseX(), cy = (float)Layout::CloseY;
    Gfx::Color3f(1.0f, 1.0f, 1.0f);
    GL_DrawTexture(0x118, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    GL_DrawTexture(280, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 0.75f, 0.75f, 1, 1);
    if (Inside(Layout::CloseX(), Layout::CloseY, Layout::CloseSize, Layout::CloseSize)) {
        gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
        m_dwTextColor = 0xFFFFFFFFu;
        m_dwBackColor = 0xFF000000u;
        RenderTipText(Layout::CloseX(), Layout::CloseY - 13, GlobalText[247]);
    }
    gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    m_dwTextColor = color;
    m_dwBackColor = back;
    GL_ResetState();
    Gfx::Color4f(1, 1, 1, 1);
}
