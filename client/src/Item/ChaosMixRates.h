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
    bool m_Pending = false;
    bool m_Requested = false;
    bool m_Valid = false;
    int m_Rate = 0;
    int m_Money = 0;
};

extern CChaosMixRates gChaosMixRates;
