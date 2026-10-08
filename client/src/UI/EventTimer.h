#pragma once
#include "Net/Protocol/GameServerProtocol.h"

// DLL EventTimer.cpp: horarios recibidos por F3/E6, sin calendario local.
class CEventTimer {
public:
    bool Receive(const BYTE* packet, int size);
    void Clear();
    void Toggle();
    void UpdateMouse();
    void Render();
    int Count() const { return m_Count; }
    const Proto::PMSG_EVENT_TIME* Get(int index) const;
    static void FormatTime(const Proto::PMSG_EVENT_TIME& event, char* text, size_t capacity);

private:
    bool Blocked() const;
    int VisibleRows() const;
    Proto::PMSG_EVENT_TIME m_Events[255] = {};
    int m_Count = 0;
    int m_Page = 0;
    bool m_Open = false;
    bool m_Received = false;
};
extern CEventTimer gEventTimer;
