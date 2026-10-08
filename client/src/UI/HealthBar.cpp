#include "stdafx.h"
#include "UI/HealthBar.h"
#include "Config/UserSettings.h"
#include "Entity/EntityView.h"

extern "C" SIZE* __cdecl RenderCenteredText(int x, int y, const char* text);

CHealthBar gHealthBar;

void CHealthBar::Clear()
{
    m_Count = 0;
}

void CHealthBar::Remove(WORD index)
{
    for (int i = 0; i < m_Count; ++i)
        if (m_Entries[i].index == index) m_Entries[i].index = 0xFFFF;
}

bool CHealthBar::Receive(const BYTE* packet, int size)
{
    // Protocol.h del server: PMSG_HEALTH_BAR_SEND + count PMSG_HEALTH_BAR.
    if (!packet || size < (int)sizeof(Proto::PMSG_HEALTH_BAR_SEND)) return false;
    Proto::PMSG_HEALTH_BAR_SEND header;
    memcpy(&header, packet, sizeof(header));
    if (header.count > (size - sizeof(header)) / sizeof(Proto::PMSG_HEALTH_BAR)) return false;
    // Validar primero: no reservar una copia del viewport en el stack.
    Proto::PMSG_HEALTH_BAR entry;
    for (int i = 0; i < header.count; ++i) {
        memcpy(&entry, packet + sizeof(header) + i * sizeof(entry), sizeof(entry));
        if (entry.rateHP > 100) return false;
    }
    // Una lista truncada no debe reemplazar la foto anterior por datos parciales.
    memcpy(m_Entries, packet + sizeof(header), header.count * sizeof(entry));
    m_Count = header.count;
    return true;
}

const Proto::PMSG_HEALTH_BAR* CHealthBar::Find(WORD index, BYTE type) const
{
    if (index == 0xFFFF) return nullptr;
    for (int i = 0; i < m_Count; ++i)
        if (m_Entries[i].index == index && m_Entries[i].type == type) return &m_Entries[i];
    return nullptr;
}

const Proto::PMSG_HEALTH_BAR* CHealthBar::FindEntity(const BYTE* entity) const
{
    const EntityView view(entity);
    if (!view.IsActive() || view.Kind() != EntityView::MonsterKind || view.ModelType() == EntityView::SoccerBall)
        return nullptr;
    return Find(view.NetworkKey(), view.Kind());
}

namespace {
struct HealthBarLayout {
    static constexpr int Width = 70, Height = 6, LabelOffset = 8;
    static constexpr float WorldOffset = 100.0f;
    static constexpr int TargetX = 220, TargetY = 10, TargetWidth = 200, TargetHeight = 12;
};

void DrawBar(int x, int y, float width, float height, BYTE percent, bool selected)
{
    EnableAlphaTest(true);
    glColor4f(0.0f, 0.0f, 0.0f, selected ? 1.0f : 0.5f);
    GL_DrawRect((float)x, (float)y, width, height);
    glColor4f(selected ? 0.5f : 0.8f, 0.0f, 0.0f, 1.0f);
    GL_DrawRect((float)x + 2, (float)y + 2, (width - 4) * percent / 100.0f, height - 4);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    GL_ResetState();
}

void DrawLabel(const BYTE* entity, BYTE percent, int center, int y, DWORD background)
{
    char text[64];
    sprintf_s(text, "%.31s: %u%%", EntityView(entity).Name(), (unsigned int)percent);
    const DWORD savedBack = m_dwBackColor;
    const DWORD savedText = m_dwTextColor;
    HGDIOBJ oldFont = SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    EnableAlphaTest(true);
    glColor4f(1, 1, 1, 1);
    m_dwBackColor = background;
    m_dwTextColor = 0xFFFFFFFFu;
    RenderCenteredText(center, y, text);
    m_dwBackColor = savedBack;
    m_dwTextColor = savedText;
    SelectObject(gFont.GetTextDC(), oldFont);
    GL_ResetState();
}

} // namespace

// DESVIACION DLL HealthBar.cpp: HealthBarType=3, como Encoder/MainInfo.ini.
// IDA: Render_GameFrame (0x004BBFB0), después de RenderPartyHP (0x004BCA20).
void CHealthBar::DrawViewport() const
{
    if (SceneFlag != 5 || gUserSettings.GetDeleteHealthBar() || !DAT_07abf5d0) return;
    const BYTE* base = (const BYTE*)(uintptr_t)DAT_07abf5d0;
    for (int i = 0; i < EntityView::Capacity; ++i) {
        const BYTE* entity = base + i * EntityView::Stride;
        const auto* bar = FindEntity(entity);
        if (!bar) continue;
        const EntityView view(entity);
        float position[3] = { view.X(), view.Y(), view.Z() + view.Height() + HealthBarLayout::WorldOffset };
        int x, y;
        Camera_ProjectWorldToScreen(position, &x, &y);
        x -= HealthBarLayout::Width / 2;
        if (MouseX >= x && MouseX < x + HealthBarLayout::Width && MouseY >= y && MouseY < y + HealthBarLayout::Height)
            DrawLabel(entity, bar->rateHP, x + HealthBarLayout::Width / 2, y - HealthBarLayout::LabelOffset, 0x80000000u);
        DrawBar(x, y, (float)HealthBarLayout::Width, (float)HealthBarLayout::Height, bar->rateHP, false);
    }
}

// IDA: RenderMonsterName (0x004CB6F0); DLL DrawPointingHealthBar (0x004CB7AD).
bool CHealthBar::DrawSelected(const BYTE* entity) const
{
    if (SceneFlag != 5 || gUserSettings.GetDeleteHealthBar()) return false;
    const auto* bar = FindEntity(entity);
    if (!bar) return false;  // Sin extensión, conservar el nombre del binario.
    DrawBar(HealthBarLayout::TargetX, HealthBarLayout::TargetY,
        (float)HealthBarLayout::TargetWidth, (float)HealthBarLayout::TargetHeight, bar->rateHP, true);
    DrawLabel(entity, bar->rateHP, HealthBarLayout::TargetX + HealthBarLayout::TargetWidth / 2,
        HealthBarLayout::TargetY + 2, 0);
    return true;
}
