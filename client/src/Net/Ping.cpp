#include "stdafx.h"
#include "Net/Ping.h"
#include "Platform/ITimer.h"

CPing gPing;

void CPing::Reset()
{
    for (auto& entry : m_Pending) entry.active = false;
    m_Next = 0;
    m_HasSample = false;
}

void CPing::RecordSend(DWORD tick)
{
    if (!m_Frequency) m_Frequency = Platform::GetTimer().CounterFrequency();
    if (!m_Frequency) return;
    int64_t now;
    if (!Platform::GetTimer().ReadCounter(now)) return;
    m_Pending[m_Next] = { tick, now, true };
    m_Next = (m_Next + 1) % Capacity;
}

void CPing::Receive(DWORD tick)
{
    if (!m_Frequency) return;
    int64_t now;
    if (!Platform::GetTimer().ReadCounter(now)) return;
    for (auto& entry : m_Pending) {
        if (!entry.active || entry.tick != tick) continue;
        entry.active = false; // Un eco sólo puede producir una muestra.
        const double seconds = (double)(now - entry.sent) / m_Frequency;
        if (seconds < 0 || seconds > TimeoutSeconds) return;
        // DESVIACION DLL PingSystem: RTT del juego, incluidas sus colas de envío/recepción.
        m_Milliseconds = (DWORD)(seconds * 1000.0 + 0.5);
        m_LastReply = now;
        m_HasSample = true;
        return;
    }
}

bool CPing::GetMilliseconds(DWORD& value) const
{
    if (!m_HasSample || !m_Frequency) return false;
    int64_t now;
    if (!Platform::GetTimer().ReadCounter(now)) return false;
    const double age = (double)(now - m_LastReply) / m_Frequency;
    if (age < 0 || age > TimeoutSeconds) return false;
    value = m_Milliseconds;
    return true;
}
