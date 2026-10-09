#pragma once

// Textos nuevos sin índice equivalente en GlobalText. La selección de idioma
// se integra en su tanda; los callers no contienen las frases ni sus formatos.
enum class ClientTextId {
    WindowTitle, ChaosRatePending, ChaosCostPending,
    EventTitle, EventWaiting, EventEmpty, EventDisabled, EventOpen, EventStarted, EventDays,
    Count
};
class CClientText {
public:
    const char* Get(ClientTextId id) const;
};
extern CClientText gClientText;
