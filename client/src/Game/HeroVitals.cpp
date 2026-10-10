#include "stdafx.h"
#include "Game/HeroVitals.h"

CHeroVitals gHeroVitals;

namespace {
// Offsets en CharacterAttribute (actual / máximo): vida, mana, AG.
constexpr int CurrentOffset[MAX_HERO_VITAL] = { 0x1C, 0x1E, 0x24 };
constexpr int MaxOffset[MAX_HERO_VITAL]     = { 0x20, 0x22, 0x26 };

WORD ReadWord(int offset)
{
    const BYTE* ca = (const BYTE*)(uintptr_t)CharacterAttribute;
    return ca ? *(const WORD*)(ca + offset) : 0;
}
}

void CHeroVitals::Reset()
{
    *this = CHeroVitals{};
}

void CHeroVitals::SetCurrent(eHeroVital vital, DWORD value)
{
    m_Current[vital] = value;
    m_HasCurrent[vital] = true;
}

void CHeroVitals::SetMax(eHeroVital vital, DWORD value)
{
    m_Max[vital] = value;
    m_HasMax[vital] = true;
}

DWORD CHeroVitals::Resolve(DWORD value, bool known, int offset) const
{
    // El WORD puede venir recortado por el cliente (ClampToWord, 65535) o por el
    // server (GET_MAX_WORD_VALUE de User.h, 65000): cualquiera de los dos
    // corresponde al mismo valor completo.
    const WORD stored = ReadWord(offset);
    const WORD clientClamp = value > 0xFFFF ? 0xFFFF : (WORD)value;
    const WORD serverClamp = value > 65000 ? 65000 : (WORD)value;
    return known && (stored == clientClamp || stored == serverClamp) ? value : stored;
}

DWORD CHeroVitals::GetCurrent(eHeroVital vital) const
{
    return Resolve(m_Current[vital], m_HasCurrent[vital], CurrentOffset[vital]);
}

DWORD CHeroVitals::GetMax(eHeroVital vital) const
{
    return Resolve(m_Max[vital], m_HasMax[vital], MaxOffset[vital]);
}
