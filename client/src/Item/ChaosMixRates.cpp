#include "stdafx.h"
#include "Item/ChaosMixRates.h"
#include "Net/Network.h"
#include "UI/UIState.h"

CChaosMixRates gChaosMixRates;

void CChaosMixRates::Reset()
{
    *this = CChaosMixRates{};
}

void CChaosMixRates::Update(int type, const ITEM* items)
{
    // Esperar el ACK del movimiento: la vista local puede ser provisional.
    if (!items || !UIState::CanQueryChaosRate()) {
        if (UIState::IsMixBusy() && m_State == State::Ready) {
            m_State = State::Idle;
            m_Attempts = 0;
        }
        return;
    }
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
        if (m_State != State::Waiting) m_State = State::Idle;
        m_Attempts = 0;
    }
    // Mismos tipos que ChaosMixRateSend del DLL. Sin consultas por frame.
    if (!Supports(type) || m_State != State::Idle || m_Attempts >= 3) return;
    const DWORD now = GetTickCount();
    if (m_Attempts && now - m_LastSent < 1000) return;
    Proto::PMSG_CHAOS_MIX_RATE_RECV packet{};
    packet.header = { 0xC1, sizeof(packet), 0x88 };
    packet.type = type;
    m_State = State::Waiting;
    m_LastSent = now;
    ++m_Attempts;
    m_PendingRevision = m_Revision;
    gNetwork.SendC1((const BYTE*)&packet, sizeof(packet));
}

void CChaosMixRates::Receive(const Proto::PMSG_CHAOS_MIX_RATE_SEND& packet)
{
    if (m_State != State::Waiting) return;
    m_State = State::Idle;
    // La respuesta no trae receta: una sola consulta pendiente, por revisiÃ³n.
    if (!UIState::CanQueryChaosRate() || m_PendingRevision != m_Revision) return;
    // 0x88 no tiene ID: drenar la respuesta antes de reintentar, aun si venció.
    if (GetTickCount() - m_LastSent > 5000) return;
    // La UI existente imprime enteros con signo; no aceptar valores imposibles.
    if (packet.rate > 100 || packet.money > 0x7FFFFFFFu) return;
    m_Rate = (int)packet.rate;
    m_Money = (int)packet.money;
    m_State = State::Ready;
}

bool CChaosMixRates::Get(int& rate, int& money) const
{
    if (m_State != State::Ready || !UIState::CanQueryChaosRate() ||
        (int)MixType != m_Type) return false;
    rate = m_Rate;
    money = m_Money;
    return true;
}
