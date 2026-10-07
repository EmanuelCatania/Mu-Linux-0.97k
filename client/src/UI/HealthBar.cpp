#include "stdafx.h"
#include "UI/HealthBar.h"
#include "Config/UserSettings.h"

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
    if (size < (int)sizeof(Proto::PMSG_HEALTH_BAR_SEND)) return false;
    Proto::PMSG_HEALTH_BAR_SEND header;
    memcpy(&header, packet, sizeof(header));
    if (header.count > (size - sizeof(header)) / sizeof(Proto::PMSG_HEALTH_BAR)) return false;
    Proto::PMSG_HEALTH_BAR entries[400];
    for (int i = 0; i < header.count; ++i) {
        memcpy(&entries[i], packet + sizeof(header) + i * sizeof(entries[i]), sizeof(entries[i]));
        if (entries[i].rateHP > 100) return false;
    }
    // Una lista truncada no debe reemplazar la foto anterior por datos parciales.
    memcpy(m_Entries, entries, header.count * sizeof(entries[0]));
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
    if (!entity || !entity[0] || entity[0x84] != 2 || *(const WORD*)(entity + 0x2EB) == 200)
        return nullptr;
    return Find(*(const WORD*)(entity + 0x1DC), entity[0x84]);
}

namespace {

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
    sprintf_s(text, "%.31s: %u%%", (const char*)(entity + 0x1C1), (unsigned int)percent);
    const DWORD savedBack = m_dwBackColor;
    const DWORD savedText = m_dwTextColor;
    HGDIOBJ oldFont = SelectObject(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    EnableAlphaTest(true);
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
    for (int i = 0; i < 400; ++i) {
        const BYTE* entity = base + i * 916;
        const auto* bar = FindEntity(entity);
        if (!bar) continue;
        float position[3] = { *(const float*)(entity + 0x10), *(const float*)(entity + 0x14),
            *(const float*)(entity + 0x18) + *(const float*)(entity + 0x12C) + 100.0f };
        int x, y;
        Camera_ProjectWorldToScreen(position, &x, &y);
        x -= 35;
        if (MouseX >= x && MouseX < x + 70 && MouseY >= y && MouseY < y + 6)
            DrawLabel(entity, bar->rateHP, x + 35, y - 8, 0x80000000u);
        DrawBar(x, y, 70.0f, 6.0f, bar->rateHP, false);
    }
}

// IDA: RenderMonsterName (0x004CB6F0); DLL DrawPointingHealthBar (0x004CB7AD).
bool CHealthBar::DrawSelected(const BYTE* entity) const
{
    if (SceneFlag != 5 || gUserSettings.GetDeleteHealthBar()) return false;
    const auto* bar = FindEntity(entity);
    if (!bar) return false;  // Sin extensión, conservar el nombre del binario.
    DrawBar(220, 10, 200.0f, 12.0f, bar->rateHP, true);
    DrawLabel(entity, bar->rateHP, 320, 12, 0);
    return true;
}
