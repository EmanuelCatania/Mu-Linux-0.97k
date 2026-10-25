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
    FontBold, FontItalic, FontSmooth, FontShadow, FontOutline, FontColorEmoji,
    RenderFpsOverlay,
    RenderDecoupled, RenderVSync, RenderFps, RenderUnlimited, RenderVSyncUnavailable,
    MiniMapGate, MiniMapZoom, MiniMapAlpha,
    ReconnectConnecting, ReconnectLogin, ReconnectCharList, ReconnectCharInfo,
    RenderGLUnavailable,
    Count
};
// Los textos salen del server (F3/F0, tabla 4, Data/Text/<idioma>/Client.txt).
// Lo que el server no manda sale del inglés que trae el cliente.
class CClientText {
public:
    const char* Get(ClientTextId id) const;
    // false si el texto no sirve (índice inválido o formato distinto del inglés).
    bool SetServerText(size_t id, const char* utf8);
    // Al desconectar: vuelve el inglés incorporado.
    void ClearServerTexts();
};
extern CClientText gClientText;

// Ruta de un archivo de Data/Local según el idioma elegido (ver ClientText.cpp).
const char* LocalizedDataPath(const char* base, const char* ext, char* out, size_t capacity);
