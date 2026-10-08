#pragma once
#include "Net/Protocol/GameServerProtocol.h"

// Valores calculados de Protocol.h::PMSG_NEW_CHARACTER_CALC_SEND, por personaje.
class CServerCharacterStats
{
public:
    void Reset();
    void Set(const Proto::PMSG_NEW_CHARACTER_CALC_SEND& packet);
    bool Apply(void* attribute) const;
    bool GetSpeeds(WORD& physical, WORD& magic) const;

private:
    Proto::PMSG_NEW_CHARACTER_CALC_SEND m_Values{};
    void* m_Attribute = nullptr;
};

extern CServerCharacterStats gServerCharacterStats;
