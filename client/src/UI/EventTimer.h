#pragma once
#include "Net/Protocol/GameServerProtocol.h"

// Horarios de eventos recibidos por F3/E6, sin calendario local.
// DESVIACION: el 0.97k no tiene este panel; viene del DLL (EventTimer.cpp) y
// acá es un panel de la columna derecha (x=450) como guild/party/character.
class CEventTimer {
public:
    bool Receive(const BYTE* packet, int size);
    void Clear();
    void Toggle();
    void Close();
    bool IsOpen() const { return m_Open; }
    void UpdateMouse();
    void RenderPanel();
    void ScrollPages(int pages);
    int Count() const { return m_Count; }
    const Proto::PMSG_EVENT_TIME* Get(int index) const;
    void DrawRow(int index, int x, int y, int width) const;

private:
    DWORD RemainingSeconds(const Proto::PMSG_EVENT_TIME& event) const;
    void FormatTime(const Proto::PMSG_EVENT_TIME& event, char* text, size_t capacity) const;
    void* Widget();
    Proto::PMSG_EVENT_TIME m_Events[255] = {};
    int m_Count = 0;
    DWORD m_ReceivedAt = 0;
    void* m_Widget = nullptr;
    bool m_Open = false;
    bool m_Received = false;
};
extern CEventTimer gEventTimer;
