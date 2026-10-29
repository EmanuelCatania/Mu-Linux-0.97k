#include "stdafx.h"
#include "Platform/Text.h"
#include "Platform/IFileSystem.h"
#include "Local/ClientText.h"
#include "Config/UserSettings.h"
#include "Text/TextCore.h"
#include <string>

extern "C" void DbgLogPublic(const char* msg);

CClientText gClientText;
namespace {
// Cadenas UTF-8. Los literales se cortan después de los escapes hexadecimales
// para que la letra siguiente no se lea como parte del mismo byte.
const char* const English[] = {
    "%.10s || %s: %u || PING: %s || FPS: %.0f",
    "Success rate: pending",
    "Cost: pending",
    "Events",
    "Waiting for event information",
    "No scheduled events",
    "Disabled",
    "Open now",
    "In progress",
    "%lu days",
    "Teleport Window",
    "Map",
    "VIP",
    "Waiting for move information",
    "No destinations",
    "< Previous",
    "Next >",
    "Default",
    "Delete Weather",
    "Delete Glow",
    "World",
    "Effects",
    "Interface",
    "Windowed",
    "Full screen",
    "Borderless",
    "Bold",
    "Italic",
    "Smooth text", "Text shadow", "Text outline", "Color emoji",
    "FPS: %.0f",
    "Free render", "VSync", "FPS: %d", "FPS: unlimited", "VSync unavailable",
    "Gate to %s",
    "Zoom: %d",
    "Alpha: %d%%",
    "Reconnecting...",
    "Logging in...",
    "Waiting character list...",
    "Waiting character info...",
    "Could not initialize OpenGL 3.3 compatibility. Check the graphics driver."
};
constexpr size_t Count = (size_t)ClientTextId::Count;
static_assert(sizeof(English) / sizeof(English[0]) == Count, "Faltan textos en ingl\xC3\xA9s");

// Textos del server, por id.  Vacío = no hay: se usa el inglés.
std::string s_ServerText[Count];

// Antes de conectar todavía no hay tabla del server. Las opciones deben
// respetar el idioma guardado también en esa pantalla.
const char* MenuFallback(ClientTextId id, int language)
{
    struct Entry { ClientTextId id; const char* spanish; const char* portuguese; };
    static const Entry entries[] = {
        { ClientTextId::LanguageDefault, "Predeterminado", "Padr\xC3\xA3" "o" },
        { ClientTextId::ScreenWindowed, "Ventana", "Janela" },
        { ClientTextId::ScreenFullscreen, "Pantalla completa", "Tela cheia" },
        { ClientTextId::ScreenBorderless, "Sin bordes", "Sem bordas" },
        { ClientTextId::RenderDecoupled, "Render libre", "Render livre" },
        { ClientTextId::RenderVSync, "VSync", "VSync" },
        { ClientTextId::RenderFps, "FPS: %d", "FPS: %d" },
        { ClientTextId::RenderUnlimited, "FPS: sin l\xC3\xAD" "mite", "FPS: sem limite" },
        { ClientTextId::RenderVSyncUnavailable, "VSync no disponible", "VSync indispon\xC3\xAD" "vel" },
    };
    for (const Entry& entry : entries) {
        if (entry.id != id) continue;
        if (language == USER_LANG_SPANISH) return entry.spanish;
        if (language == USER_LANG_PORTUGUESE) return entry.portuguese;
        break;
    }
    return nullptr;
}
}

const char* CClientText::Get(ClientTextId id) const
{
    const size_t index = (size_t)id;
    if (index >= Count) return "";
    if (!s_ServerText[index].empty()) return s_ServerText[index].c_str();
    const char* local = MenuFallback(id, gUserSettings.GetLanguage());
    return local ? local : English[index];
}

bool CClientText::SetServerText(size_t id, const char* utf8)
{
    if (id >= Count || !utf8 || !*utf8) return false;
    // Un formato distinto del inglés rompería el printf del que lo usa.
    if (!TextEngine::SameFormatSpecifiers(utf8, English[id])) {
        char line[160];
        Platform::TextFormat(line, "ClientText: el texto %u del server no tiene los mismos especificadores que el ingl\xC3\xA9s, se descarta", (unsigned)id);
        DbgLogPublic(line);
        return false;
    }
    s_ServerText[id] = utf8;
    return true;
}

void CClientText::ClearServerTexts()
{
    for (std::string& text : s_ServerText) text.clear();
}

// DESVIACION (DLL Language.cpp): con [Language] LangSelection el archivo pasa a
// ser `<base>_<Eng|Spn|Por><ext>`.  Sin selección, o si ese archivo no está, se
// usa el nombre del 0.97k (`<base><ext>`).
const char* LocalizedDataPath(const char* base, const char* ext, char* out, size_t capacity)
{
    const char* suffix = CUserSettings::GetLanguageSuffix(gUserSettings.GetLanguage());
    if (suffix) {
        Platform::TextFormatSecure(out, capacity, "%s_%s%s", base, suffix, ext);
        if (Platform::GetFileSystem().Exists(out)) return out;
        char line[160];
        Platform::TextFormat(line, "Idioma: falta %s, se usa %s%s", out, base, ext);
        DbgLogPublic(line);
    }
    Platform::TextFormatSecure(out, capacity, "%s%s", base, ext);
    return out;
}
