#include "stdafx.h"
#include "Platform/Text.h"
#include "Platform/IFileSystem.h"
#include "Local/ClientText.h"
#include "Config/UserSettings.h"

extern "C" void DbgLogPublic(const char* msg);

CClientText gClientText;
namespace {
// Las cadenas van en cp1252 (el texto se dibuja con GDI ANSI y el proyecto
// compila en MultiByte): los acentos son escapes y el literal se corta después
// de cada uno para que la letra siguiente no se lea como dígito hexadecimal.
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
const char* const Spanish[] = {
    "%.10s || %s: %u || PING: %s || FPS: %.0f",
    "Tasa de \xC3\xA9" "xito: pendiente",
    "Costo: pendiente",
    "Eventos",
    "Esperando informaci\xC3\xB3" "n de eventos",
    "No hay eventos programados",
    "Deshabilitado",
    "Abierto",
    "En curso",
    "%lu d\xC3\xAD" "as",
    "Teletransporte",
    "Mapa",
    "VIP",
    "Esperando destinos",
    "Sin destinos",
    "< Anterior",
    "Siguiente >",
    "Predeterminado",
    "Eliminar Clima",
    "Eliminar Brillo",
    "Mundo",
    "Efectos",
    "Interfaz",
    "Ventana",
    "Pantalla completa",
    "Sin bordes",
    "Negrita",
    "Cursiva",
    "FPS: %.0f",
    "Desacople", "VSync", "FPS: %d", "FPS: sin l\xC3\xAD" "mite", "VSync no disponible",
    "Puerta a %s",
    "Zoom: %d",
    "Opacidad: %d%%",
    "Reconectando...",
    "Iniciando sesi\xC3\xB3" "n...",
    "Esperando la lista de personajes...",
    "Esperando los datos del personaje...",
    "No se pudo iniciar OpenGL 3.3 compatible. Revisar el controlador de video."
};
const char* const Portuguese[] = {
    "%.10s || %s: %u || PING: %s || FPS: %.0f",
    "Taxa de sucesso: pendente",
    "Custo: pendente",
    "Eventos",
    "Aguardando informa\xC3\xA7\xC3\xB5" "es de eventos",
    "Nenhum evento programado",
    "Desativado",
    "Aberto",
    "Em andamento",
    "%lu dias",
    "Teletransporte",
    "Mapa",
    "VIP",
    "Aguardando destinos",
    "Sem destinos",
    "< Anterior",
    "Pr\xC3\xB3" "ximo >",
    "Padr\xC3\xA3" "o",
    "Remover Clima",
    "Remover Brilho",
    "Mundo",
    "Efeitos",
    "Interface",
    "Janela",
    "Tela cheia",
    "Sem bordas",
    "Negrito",
    "It\xC3\xA1" "lico",
    "FPS: %.0f",
    "Render livre", "VSync", "FPS: %d", "FPS: sem limite", "VSync indispon\xC3\xAD" "vel",
    "Portal para %s",
    "Zoom: %d",
    "Opacidade: %d%%",
    "Reconectando...",
    "Entrando...",
    "Aguardando a lista de personagens...",
    "Aguardando os dados do personagem...",
    "N\xC3\xA3" "o foi poss\xC3\xAD" "vel iniciar OpenGL 3.3 compatibility. Verifique o driver de v\xC3\xAD" "deo."
};
constexpr size_t Count = (size_t)ClientTextId::Count;
static_assert(sizeof(English) / sizeof(English[0]) == Count, "Faltan textos en ingl\xC3\xA9s");
static_assert(sizeof(Spanish) / sizeof(Spanish[0]) == Count, "Faltan textos en espa\xC3\xB1ol");
static_assert(sizeof(Portuguese) / sizeof(Portuguese[0]) == Count, "Faltan textos en portugu\xC3\xA9s");
}

const char* CClientText::Get(ClientTextId id) const
{
    const size_t index = (size_t)id;
    if (index >= Count) return "";
    switch (gUserSettings.GetLanguage()) {
    case USER_LANG_SPANISH:    return Spanish[index];
    case USER_LANG_PORTUGUESE: return Portuguese[index];
    default:              return English[index];
    }
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
