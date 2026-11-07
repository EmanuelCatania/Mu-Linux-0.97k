#pragma once
#include "Net/Protocol/GameServerProtocol.h"

// Consulta de porcentaje/costo del DLL, sobre la ventana Chaos existente.
class CChaosMixRates
{
public:
    void Reset();
    void Update(int type, const ITEM* items);
    void Receive(const Proto::PMSG_CHAOS_MIX_RATE_SEND& packet);
    bool Get(int& rate, int& money) const;
    static bool Supports(int type) { return (type >= 1 && type <= 8) || type == 11; }

private:
    struct Material {
        short Type = -1;
        int Level = 0;
        BYTE Durability = 0;
        BYTE Option = 0;
    };
    Material m_Items[32]{};
    int m_Type = 0;
    unsigned int m_Revision = 0;
    unsigned int m_PendingRevision = 0;
    enum class State { Idle, Waiting, Ready };
    State m_State = State::Idle;
    DWORD m_LastSent = 0;
    unsigned int m_Attempts = 0;
    int m_Rate = 0;
    int m_Money = 0;
};

extern CChaosMixRates gChaosMixRates;
