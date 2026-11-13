#include "stdafx.h"
#include "net/NetText.h"
#include "Render/Gfx.h"
#include "platform/Text.h"
#include "UI/MiniMap.h"
#include "UI/MoveList.h"
#include "UI/EventTimer.h"
#include "Core/Font.h"
#include "Core/Window.h"
#include "UI/UIState.h"
#include "Local/ClientText.h"

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
    x += gWindow.PanelOrigin(Ui::Move);
    return Ui::Rect{x, y, w, h}.Contains((int)MouseX, (int)MouseY);
}
void Text(int x, int y, const char* text, int width)
{
    // DLL MoveList.cpp: centrar dentro de cada columna; ancho físico para RenderText.
    RenderText(gWindow.PanelOrigin(Ui::Move) + x, y, const_cast<char*>(text),
        (int)(width * gWindow.GetWidth() / 640), 1, nullptr);
}
void Rect(int x, int y, int w, int h, float r, float g, float b, float a)
{
    EnableAlphaTest(true);
    Gfx::Color4f(r, g, b, a);
    GL_DrawRect((float)(gWindow.PanelOrigin(Ui::Move) + x), (float)y, (float)w, (float)h);
    GL_ResetState();
    // DLL MoveList.cpp: el texto no debe heredar el color del fondo o del hover.
    Gfx::Color4f(1, 1, 1, 1);
}
}

bool CMoveList::SetCatalog(const BYTE* records, int recordSize, int count)
{
    const int stride = (int)sizeof(Proto::CATALOG_MOVE);
    if (recordSize != stride || count < 0 || count > (int)(sizeof(m_Maps) / sizeof(m_Maps[0])) ||
        (count > 0 && !records)) return false;
    // Validar toda la lista antes de reemplazarla.
    for (int i = 0; i < count; ++i) {
        const BYTE* name = records + i * stride + offsetof(Proto::CATALOG_MOVE, Name);
        if (name[0] == 0 || !memchr(name, 0, 32)) return false;
        for (int n = 0; n < 32 && name[n]; ++n)
            if (name[n] < 32 || name[n] == 127) return false;
    }
    m_PKLimitFree = 0;
    for (int i = 0; i < count; ++i) {
        Proto::CATALOG_MOVE row;
        memcpy(&row, records + i * stride, sizeof(row));
        Proto::MOVE_LIST_INFO& map = m_Maps[i];
        map = {};
        map.MapNumber = row.Map;
        memcpy(map.MapName, row.Name, sizeof(map.MapName));
        map.CanMove = 1;
        map.MinLevel = row.MinLevel;
        map.MaxLevel = row.MaxLevel;
        map.MinReset = row.MinReset;
        map.MaxReset = row.MaxReset;
        map.AccountLevel = row.AccountLevel;
        map.Money = row.Money;
        if (row.Flags & Proto::CATALOG_MOVE_PK_FREE) m_PKLimitFree = 1;
        // DESVIACION (Fase 4, texto Unicode): nombres de mapa a UTF-8;
        // lo que ya es UTF-8 se deja y el resto se
        // toma como Windows-1252 (NormalizeTextFileInPlace).
        TextEngine::NormalizeTextFileInPlace(map.MapName, sizeof(map.MapName));
    }
    m_Count = count;
    m_Received = true;
    m_Page = 0;
    return true;
}

void CMoveList::Clear()
{
    m_Count = 0;
    m_Received = false;
    m_PKLimitFree = 0;
    ResetCharacter();
}

void CMoveList::ResetCharacter()
{
    m_Page = 0;
    m_Open = false;
    m_Resets = 0;
}

// CGate::GetMoveLevel del server: el Magic Gladiator necesita 2/3 del nivel.
// Antes lo aplicaba el server al armar el F3:E5 de cada personaje; la lista del
// catálogo es la misma para todos.
int CMoveList::MinLevelFor(const Proto::MOVE_LIST_INFO& map) const
{
    if (map.MinLevel == -1) return -1;
    const BYTE* attr = (const BYTE*)(uintptr_t)CharacterAttribute;
    if (attr && (attr[11] & 7) == 3) return (map.MinLevel * 2) / 3;
    return map.MinLevel;
}

const Proto::MOVE_LIST_INFO* CMoveList::Get(int index) const
{
    return index >= 0 && index < m_Count ? &m_Maps[index] : nullptr;
}

bool CMoveList::Blocked() const
{
    return !UIState::CanOpenInformationalPanel();
}

// DLL MoveList.cpp CheckMove: el F3:E5 llega una sola vez, al entrar al mapa, y el
// server no evalúa al personaje (manda CanMove = 1 siempre). El DLL recalculaba la
// disponibilidad en cada render con el estado actual del héroe; el server sigue
// validando el /move.
bool CMoveList::CheckMove(const Proto::MOVE_LIST_INFO& map) const
{
    if (!map.CanMove) return false;
    const BYTE* attr = (const BYTE*)(uintptr_t)CharacterAttribute;
    const BYTE* hero = (const BYTE*)(uintptr_t)Hero;
    if (!attr || !hero) return false;
    const WORD level = *(const WORD*)(attr + 0x0E);
    const int minLevel = MinLevelFor(map);
    if (minLevel != -1 && level < minLevel) return false;
    if (map.MaxLevel != -1 && level > map.MaxLevel) return false;
    if (map.MinReset != -1 && m_Resets < (DWORD)map.MinReset) return false;
    if (map.MaxReset != -1 && m_Resets > (DWORD)map.MaxReset) return false;
    if (*(const DWORD*)(attr + 0x548) < map.Money) return false;
    if (m_PKLimitFree == 0 && hero[EntityOffset(0x2EA)] >= PKLVL_OUTLAW) return false;
    const short wing = *(const short*)&((CHARACTER*)(uintptr_t)(hero))->Wing.Type;
    const short helper = *(const short*)&((CHARACTER*)(uintptr_t)(hero))->Helper.Type;
    const bool uniria = helper == GET_ITEM_MODEL(13, 2);
    const bool dinorant = helper == GET_ITEM_MODEL(13, 3);
    if (map.MapNumber == MAP_ATLANS && (uniria || dinorant)) return false;
    if (map.MapNumber == MAP_ICARUS && ((wing == -1 && !dinorant) || uniria)) return false;
    return true;
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
    if (!m_Open) { gEventTimer.Close(); gMiniMap.Close(); }
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
    const int closeX = m_Count > RowsPerPage ? Layout::CloseX : Layout::ContentX;
    const int closeWidth = m_Count > RowsPerPage ? Layout::ButtonWidth : Layout::ContentWidth;
    if (Inside(closeX, footer, closeWidth, Layout::RowStep)) { Toggle(); return; }
    if (m_Count > RowsPerPage) {
        if (Inside(Layout::PreviousX, footer, Layout::ButtonWidth, Layout::RowStep) && m_Page > 0) { --m_Page; return; }
        if (Inside(Layout::NextX, footer, Layout::ButtonWidth, Layout::RowStep) && (m_Page + 1) * RowsPerPage < m_Count) {
            ++m_Page; return;
        }
    }
    for (int i = 0; i < rows; ++i) {
        if (!Inside(Layout::ContentX, Layout::RowsY + i * Layout::RowStep, Layout::ContentWidth, Layout::RowHeight)) continue;
        const auto& map = m_Maps[m_Page * RowsPerPage + i];
        if (!CheckMove(map)) return;
        char command[64];
        Platform::TextFormatSecure(command, "/move %s", map.MapName);
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
    HGDIOBJ font = gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    m_dwBackColor = 0;
    Rect(Layout::X, Layout::Y, Layout::Width, Layout::BaseHeight + rows * Layout::RowStep, 0, 0, 0, .8f);
    m_dwTextColor = 0xFF00FFFF;
    Text(Layout::ContentX, Layout::TitleY, gClientText.Get(ClientTextId::MoveTitle), Layout::ContentWidth);
    m_dwTextColor = 0xFFFFCC66;
    Text(Layout::MapX, Layout::HeaderY, gClientText.Get(ClientTextId::MoveMap), Layout::MapWidth);
    Text(Layout::LevelX, Layout::HeaderY, GlobalText[161], Layout::LevelWidth);
    Text(Layout::ZenX, Layout::HeaderY, GlobalText[100], Layout::ZenWidth);
    Text(Layout::VipX, Layout::HeaderY, gClientText.Get(ClientTextId::MoveVip), Layout::VipWidth);
    m_dwTextColor = 0xFFFFFFFF;
    if (!m_Count) Text(Layout::ContentX, Layout::RowsY, gClientText.Get(m_Received ? ClientTextId::MoveEmpty : ClientTextId::MoveWaiting), Layout::ContentWidth);
    for (int i = 0; i < rows; ++i) {
        const auto& map = m_Maps[m_Page * RowsPerPage + i];
        const int y = Layout::RowsY + i * Layout::RowStep;
        if (Inside(Layout::ContentX, y, Layout::ContentWidth, Layout::RowHeight)) {
            Rect(Layout::ContentX, y, Layout::ContentWidth, Layout::RowHeight, .8f, .8f, .1f, .6f);
        }
        m_dwTextColor = CheckMove(map) ? 0xFFFFFFFF : 0xFF1127A4;
        Text(Layout::MapX, y, map.MapName, Layout::MapWidth);
        char text[32];
        const int minLevel = MinLevelFor(map);
        if (minLevel == -1) Platform::TextCopySecure(text, "~"); else Platform::TextFormatSecure(text, "%d", minLevel);
        Text(Layout::LevelX, y, text, Layout::LevelWidth);
        Platform::TextFormatSecure(text, "%lu", (unsigned long)map.Money); Text(Layout::ZenX, y, text, Layout::ZenWidth);
        if (map.AccountLevel > 0) Text(Layout::VipX, y, "*", Layout::VipWidth);
    }
    m_dwTextColor = 0xFFFFFFFF;
    const int footer = Layout::FooterY + rows * Layout::RowStep;
    const int closeX = m_Count > RowsPerPage ? Layout::CloseX : Layout::ContentX;
    const int closeWidth = m_Count > RowsPerPage ? Layout::ButtonWidth : Layout::ContentWidth;
    // Cierre rojo como en el DLL; dejar lugar a los botones si hay paginación.
    const bool closeHover = Inside(closeX, footer, closeWidth, Layout::RowStep);
    Rect(closeX, footer, closeWidth, Layout::RowStep, closeHover ? 1.0f : .8f, 0, 0, 1);
    Text(closeX, footer, GlobalText[247], closeWidth);
    if (m_Count > RowsPerPage) {
        if (m_Page > 0) Text(Layout::PreviousX, footer, gClientText.Get(ClientTextId::PagePrevious), Layout::ButtonWidth);
        if ((m_Page + 1) * RowsPerPage < m_Count) Text(Layout::NextX, footer, gClientText.Get(ClientTextId::PageNext), Layout::ButtonWidth);
    }
    gFont.SelectFont(gFont.GetTextDC(), font);
    m_dwTextColor = color;
    m_dwBackColor = back;
    Gfx::Color4f(1, 1, 1, 1);
}
