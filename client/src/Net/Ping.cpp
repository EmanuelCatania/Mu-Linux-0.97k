#include "stdafx.h"
#include "Net/Ping.h"

CPing gPing;

void CPing::Reset()
{
    for (auto& entry : m_Pending) entry.active = false;
    m_Next = 0;
    m_HasSample = false;
}

void CPing::RecordSend(DWORD tick)
{
    if (!m_Frequency.QuadPart && !QueryPerformanceFrequency(&m_Frequency)) return;
    LARGE_INTEGER now;
    if (!QueryPerformanceCounter(&now)) return;
    m_Pending[m_Next] = { tick, now, true };
    m_Next = (m_Next + 1) % Capacity;
}

void CPing::Receive(DWORD tick)
{
    if (!m_Frequency.QuadPart) return;
    LARGE_INTEGER now;
    if (!QueryPerformanceCounter(&now)) return;
    for (auto& entry : m_Pending) {
        if (!entry.active || entry.tick != tick) continue;
        entry.active = false; // Un eco sólo puede producir una muestra.
        const double seconds = (double)(now.QuadPart - entry.sent.QuadPart) / m_Frequency.QuadPart;
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
    if (!m_HasSample || !m_Frequency.QuadPart) return false;
    LARGE_INTEGER now;
    if (!QueryPerformanceCounter(&now)) return false;
    const double age = (double)(now.QuadPart - m_LastReply.QuadPart) / m_Frequency.QuadPart;
    if (age < 0 || age > TimeoutSeconds) return false;
    value = m_Milliseconds;
    return true;
}
