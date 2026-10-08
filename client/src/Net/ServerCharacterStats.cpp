#include "stdafx.h"
#include "Net/Recv/NetRecv.h"
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
    BYTE* CA = (BYTE*)attribute;
    // DESVIACION MuEmu: F3/E1 prevalece sobre las fórmulas locales del binario.
    // No restaurar HP/MP/AG: otros paquetes modifican los recursos actuales.
    *(WORD*)(CA + 0x38) = ClampToWord(m_Values.ViewPhysiSpeed);
    *(WORD*)(CA + 0x44) = ClampToWord(m_Values.ViewMagicSpeed);
    *(WORD*)(CA + 0x3A) = ClampToWord(m_Values.ViewAttackSuccessRate);
    *(WORD*)(CA + 0x4E) = ClampToWord(m_Values.ViewDefense);
    *(WORD*)(CA + 0x4C) = ClampToWord(m_Values.ViewDefenseSuccessRate);
    *(WORD*)(CA + 0x46) = ClampToWord(m_Values.ViewMagicDamageMin);
    *(WORD*)(CA + 0x48) = ClampToWord(m_Values.ViewMagicDamageMax);
    return true;
}

bool CServerCharacterStats::GetSpeeds(WORD& physical, WORD& magic) const
{
    if (!m_Attribute || m_Attribute != (void*)(uintptr_t)CharacterAttribute) return false;
    physical = ClampToWord(m_Values.ViewPhysiSpeed);
    magic = ClampToWord(m_Values.ViewMagicSpeed);
    return true;
}
