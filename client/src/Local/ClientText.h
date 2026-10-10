#pragma once

// Textos nuevos sin índice equivalente en GlobalText. La selección de idioma
// se integra en su tanda; los callers no contienen las frases ni sus formatos.
enum class ClientTextId {
    WindowTitle, ChaosRatePending, ChaosCostPending,
    EventTitle, EventWaiting, EventEmpty, EventDisabled, EventOpen, EventStarted, EventDays,
    MoveTitle, MoveMap, MoveVip, MoveWaiting, MoveEmpty, PagePrevious, PageNext,
    LanguageDefault, AntilagWeather, AntilagGlow,
    AntilagWorld, AntilagEffects, AntilagInterface,
    ScreenWindowed, ScreenFullscreen, ScreenBorderless,
    FontBold, FontItalic,
    MiniMapGate, MiniMapZoom, MiniMapAlpha,
    ReconnectConnecting, ReconnectLogin, ReconnectCharList, ReconnectCharInfo,
    Count
};
class CClientText {
public:
    const char* Get(ClientTextId id) const;
};
extern CClientText gClientText;

// Ruta de un archivo de Data/Local según el idioma elegido (ver ClientText.cpp).
const char* LocalizedDataPath(const char* base, const char* ext, char* out, size_t capacity);
