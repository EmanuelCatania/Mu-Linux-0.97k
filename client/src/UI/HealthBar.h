#pragma once
#include "net/Protocol/GameServerProtocol.h"

class CHealthBar
{
public:
    void Clear();
    void Remove(WORD index);
    // F3:E2: filas que cambiaron (se mezclan con las que ya hay).
    bool Receive(const BYTE* packet, int size);
    // % de un monstruo (0x13 al aparecer, 0x15 al recibir daño).
    void Set(WORD index, BYTE type, BYTE rateHP);
    const Proto::PMSG_HEALTH_BAR* Find(WORD index, BYTE type) const;
    void DrawViewport() const;
    bool DrawSelected(const BYTE* entity) const;

private:
    const Proto::PMSG_HEALTH_BAR* FindEntity(const BYTE* entity) const;
    Proto::PMSG_HEALTH_BAR m_Entries[400]{};
    int m_Count = 0;
};

extern CHealthBar gHealthBar;
