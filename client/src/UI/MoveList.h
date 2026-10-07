#pragma once
#include "Net/Protocol/GameServerProtocol.h"

// Menú M del DLL (MoveList.cpp). La lista y los requisitos llegan por F3/E5.
class CMoveList {
public:
    bool Receive(const BYTE* packet, int size);
    void Clear();
    void Toggle();
    void UpdateMouse();
    void Render();
    int Count() const { return m_Count; }
    const Proto::MOVE_LIST_INFO* Get(int index) const;

private:
    bool Blocked() const;
    int VisibleRows() const;
    Proto::MOVE_LIST_INFO m_Maps[255] = {};
    int m_Count = 0;
    int m_Page = 0;
    bool m_Open = false;
    bool m_Received = false;
    BYTE m_PKLimitFree = 0;
};
extern CMoveList gMoveList;
