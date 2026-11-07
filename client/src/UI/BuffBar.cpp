#include "stdafx.h"
#include "UI/BuffBar.h"
#include "Net/Protocol/GameServerProtocol.h"
#include "Render/Gfx.h"
#include "Platform/Text.h"
#include "UI/MoveList.h"
#include "Platform/ITimer.h"
#include "Local/ClientText.h"
#include "Core/Window.h"
#include "UI/PanelAnchor.h"
#include <cstring>

CBuffBar gBuffBar;

namespace {
// Íconos del atlas de skills (bitmap 298): 8 columnas de celdas de 32x36,
// como los dibuja sub_4BB940. En la barra van más chicos, a la izquierda en la
// altura del contador de FPS (F12), cerca del borde de arriba.
constexpr int CellW = 32, CellH = 36;
constexpr int IconW = 16, IconH = 18, Gap = 2, BarY = 10, BarX = 110;

void DrawSkillIcon(int skill, float x, float y)
{
    const int index = skill - 1;
    const float u = (float)((index % 8) * CellW) / 256.0f;
    const float v = (float)((index / 8) * CellH) / 256.0f;
    GL_DrawTexture(298, x, y, (float)IconW, (float)IconH, u, v,
                   (float)CellW / 256.0f, (float)(CellH - 1) / 256.0f, 1, 1);
}

void FormatTime(unsigned long seconds, char* out, size_t size)
{
    if (seconds >= 3600)
        Platform::TextFormatSecure(out, size, "%lu:%02lu:%02lu", seconds / 3600, seconds / 60 % 60, seconds % 60);
    else
        Platform::TextFormatSecure(out, size, "%lu:%02lu", seconds / 60, seconds % 60);
}

// Lo que mejora cada efecto (índices de Effect.txt del server).
ClientTextId BonusText(int effect)
{
    switch (effect) {
        case 1: return ClientTextId::BuffDamage;
        case 2: return ClientTextId::BuffDefense;
        case 4: return ClientTextId::BuffDamageReduction;
        case 8: return ClientTextId::BuffMaxLife;
        default: return ClientTextId::Count;
    }
}
}

void CBuffBar::Receive(const unsigned char* packet, int size)
{
    if (size < (int)sizeof(Proto::PMSG_EFFECT_INFO_SEND)) return;
    Proto::PMSG_EFFECT_INFO_SEND msg;
    std::memcpy(&msg, packet, sizeof(msg));
    int slot = 0;
    while (slot < m_Count && m_Buffs[slot].effect != msg.effect) ++slot;
    if (!msg.state) {
        if (slot == m_Count) return;
        for (int i = slot + 1; i < m_Count; ++i) m_Buffs[i - 1] = m_Buffs[i];
        --m_Count;
        return;
    }
    if (slot == m_Count) {
        if (m_Count == MaxBuffs) return;
        ++m_Count;
    }
    Buff& buff = m_Buffs[slot];
    buff = Buff{};
    buff.effect = msg.effect;
    buff.skill = msg.skill;
    buff.seconds = msg.time;
    buff.receivedAt = Platform::GetTimer().Ticks();
    std::memcpy(buff.value, msg.value, sizeof(buff.value));
    std::memcpy(buff.caster, msg.caster, sizeof(msg.caster));
    std::memcpy(buff.name, msg.name, sizeof(msg.name));
}

void CBuffBar::Clear()
{
    m_Count = 0;
}

unsigned long CBuffBar::Remaining(const Buff& buff) const
{
    const unsigned long elapsed = (Platform::GetTimer().Ticks() - buff.receivedAt) / 1000;
    return elapsed < buff.seconds ? buff.seconds - elapsed : 0;
}

void CBuffBar::Render()
{
    if (SceneFlag != 5 || m_Count == 0) return;
    int visible = 0;
    for (int i = 0; i < m_Count; ++i)
        if (!m_Buffs[i].seconds || Remaining(m_Buffs[i])) ++visible;
    if (!visible) return;

    int x = gWindow.PanelOrigin(Ui::WindowLeft) + BarX;
    const Buff* hovered = nullptr;
    int hoveredX = 0;
    Gfx::Color4f(1.0f, 1.0f, 1.0f, 1.0f);
    for (int i = 0; i < m_Count; ++i) {
        const Buff& buff = m_Buffs[i];
        const unsigned long left = Remaining(buff);
        if (buff.seconds && !left) continue;
        // Los últimos 10 segundos el ícono parpadea, como aviso.
        const bool blink = buff.seconds && left <= 10 && (Platform::GetTimer().Ticks() / 250) % 2;
        Gfx::Color4f(1.0f, 1.0f, 1.0f, blink ? 0.4f : 1.0f);
        if (buff.skill > 0 && buff.skill < 256)
            DrawSkillIcon(buff.skill, (float)x, (float)BarY);
        else {
            Gfx::Color4f(0.2f, 0.2f, 0.2f, 0.8f);
            GL_DrawRect((float)x, (float)BarY, (float)IconW, (float)IconH);
        }
        if (MouseX >= x && MouseX < x + IconW && MouseY >= BarY && MouseY < BarY + IconH) {
            hovered = &buff;
            hoveredX = x;
        }
        x += IconW + Gap;
    }
    Gfx::Color4f(1.0f, 1.0f, 1.0f, 1.0f);
    // Con el menú M abierto los íconos quedan debajo: sin tooltip.
    if (hovered && !gMoveList.IsOpen()) RenderTooltip(*hovered, hoveredX + IconW / 2, BarY + IconH + 4);
}

void CBuffBar::RenderTooltip(const Buff& buff, int x, int y) const
{
    char* line = lpString_07e90798;
    int count = 0;
    auto add = [&](int color, int bold, const char* text) {
        Platform::TextFormatSecure(line + count * 100, 100, "%s", text);
        DAT_07e91708[count] = color;
        DAT_07ea7b10[count] = bold;
        ++count;
    };
    // El nombre del skill sale de skill.bmd (traducido); si no hay skill, el
    // que manda el server.
    char name[64] = {};
    if (buff.skill > 0 && buff.skill < 256) {
        int mana = 0, distance = 0, skillMana = 0;
        GetSkillInformation(buff.skill, 1, name, &mana, &distance, &skillMana);
    }
    if (!name[0]) Platform::TextFormatSecure(name, sizeof(name), "%s", buff.name);
    add(1, 1, name);

    char text[100];
    const ClientTextId bonus = BonusText(buff.effect);
    if (bonus != ClientTextId::Count) {
        Platform::TextFormatSecure(text, sizeof(text), gClientText.Get(bonus), (int)buff.value[0]);
        add(0, 0, text);
    }
    if (buff.seconds) {
        char time[16];
        FormatTime(Remaining(buff), time, sizeof(time));
        Platform::TextFormatSecure(text, sizeof(text), gClientText.Get(ClientTextId::BuffTimeLeft), time);
        add(0, 0, text);
    }
    if (buff.caster[0]) {
        Platform::TextFormatSecure(text, sizeof(text), gClientText.Get(ClientTextId::BuffCaster), buff.caster);
        add(3, 0, text);
    }
    CharMenu_RenderTextList(x, y, count, 0, 1, 1);
}
