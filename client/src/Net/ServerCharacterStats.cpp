#include "stdafx.h"
#include "Entity/CharacterAttributeView.h"
#include "Net/ServerCharacterStats.h"

CServerCharacterStats gServerCharacterStats;

void CServerCharacterStats::Reset()
{
    m_Attribute = nullptr;
}

void CServerCharacterStats::Set(const Proto::PMSG_NEW_CHARACTER_CALC_SEND& packet)
{
    m_Values = packet;
    m_Attribute = (void*)(uintptr_t)CharacterAttribute;
}

bool CServerCharacterStats::Apply(void* attribute) const
{
    if (!m_Attribute || attribute != m_Attribute ||
        attribute != (void*)(uintptr_t)CharacterAttribute) return false;
    CharacterAttributeView attributes(attribute);
    // DESVIACION MuEmu: F3/E1 prevalece sobre las fórmulas locales del binario.
    // No restaurar HP/MP/AG: otros paquetes modifican los recursos actuales.
    attributes.SetPhysicalSpeed(m_Values.ViewPhysiSpeed);
    attributes.SetMagicSpeed(m_Values.ViewMagicSpeed);
    attributes.SetAttackSuccessRate(m_Values.ViewAttackSuccessRate);
    attributes.SetDefense(m_Values.ViewDefense);
    attributes.SetDefenseSuccessRate(m_Values.ViewDefenseSuccessRate);
    attributes.SetMagicDamageMin(m_Values.ViewMagicDamageMin);
    attributes.SetMagicDamageMax(m_Values.ViewMagicDamageMax);
    return true;
}

bool CServerCharacterStats::GetSpeeds(WORD& physical, WORD& magic) const
{
    if (!m_Attribute || m_Attribute != (void*)(uintptr_t)CharacterAttribute) return false;
    physical = (WORD)(m_Values.ViewPhysiSpeed > 0xFFFFu ? 0xFFFFu : m_Values.ViewPhysiSpeed);
    magic = (WORD)(m_Values.ViewMagicSpeed > 0xFFFFu ? 0xFFFFu : m_Values.ViewMagicSpeed);
    return true;
}
