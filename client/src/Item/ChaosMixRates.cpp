#include "stdafx.h"
#include "Item/ChaosMixRates.h"
#include "Net/Network.h"

CChaosMixRates gChaosMixRates;

void CChaosMixRates::Reset()
{
    *this = CChaosMixRates{};
}

void CChaosMixRates::Update(int type, const ITEM* items)
{
    // Esperar el ACK del movimiento: la vista local puede ser provisional.
    if (DAT_07eaa165 != 0 || DAT_07eaa140 != 0) return;
    bool changed = (type != m_Type);
    m_Type = type;
    for (int i = 0; i < 32; ++i) {
        Material current;
        current.Type = items[i].Type;
        if (current.Type != -1) {
            current.Level = items[i].Level;
            current.Durability = items[i].Durability;
            current.Option = items[i].Option1;
        }
        const Material& old = m_Items[i];
        changed |= current.Type != old.Type || current.Level != old.Level ||
                   current.Durability != old.Durability || current.Option != old.Option;
        m_Items[i] = current;
    }
    if (changed) {
        ++m_Revision;
        m_Valid = false;
        m_Requested = false;
    }
    // Mismos tipos que ChaosMixRateSend del DLL. Sin consultas por frame.
    if (!((type >= 1 && type <= 8) || type == 11) || m_Pending || m_Requested) return;
    Proto::PMSG_CHAOS_MIX_RATE_RECV packet{};
    packet.header = { 0xC1, sizeof(packet), 0x88 };
    packet.type = type;
    m_Pending = true;
    m_Requested = true;
    m_PendingRevision = m_Revision;
    gNetwork.SendC1((const BYTE*)&packet, sizeof(packet));
}

void CChaosMixRates::Receive(const Proto::PMSG_CHAOS_MIX_RATE_SEND& packet)
{
    if (!m_Pending) return;
    m_Pending = false;
    // La respuesta no trae receta: una sola consulta pendiente, por revisión.
    if (!ChaosMixOpened || m_PendingRevision != m_Revision) return;
    // La UI existente imprime enteros con signo; no aceptar valores imposibles.
    if (packet.rate > 100 || packet.money > 0x7FFFFFFFu) return;
    m_Rate = (int)packet.rate;
    m_Money = (int)packet.money;
    m_Valid = true;
}

bool CChaosMixRates::Get(int& rate, int& money) const
{
    if (!m_Valid || !ChaosMixOpened || (int)MixType != m_Type ||
        DAT_07eaa165 != 0 || DAT_07eaa140 != 0) return false;
    rate = m_Rate;
    money = m_Money;
    return true;
}
