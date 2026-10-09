#include "stdafx.h"
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
    "Default"
};
const char* const Spanish[] = {
    "%.10s || %s: %u || PING: %s || FPS: %.0f",
    "Tasa de \xE9" "xito: pendiente",
    "Costo: pendiente",
    "Eventos",
    "Esperando informaci\xF3" "n de eventos",
    "No hay eventos programados",
    "Deshabilitado",
    "Abierto",
    "En curso",
    "%lu d\xED" "as",
    "Teletransporte",
    "Mapa",
    "VIP",
    "Esperando destinos",
    "Sin destinos",
    "< Anterior",
    "Siguiente >",
    "Predeterminado"
};
const char* const Portuguese[] = {
    "%.10s || %s: %u || PING: %s || FPS: %.0f",
    "Taxa de sucesso: pendente",
    "Custo: pendente",
    "Eventos",
    "Aguardando informa\xE7\xF5" "es de eventos",
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
    "Pr\xF3" "ximo >",
    "Padr\xE3" "o"
};
constexpr size_t Count = (size_t)ClientTextId::Count;
static_assert(sizeof(English) / sizeof(English[0]) == Count, "Faltan textos en ingl\xE9s");
static_assert(sizeof(Spanish) / sizeof(Spanish[0]) == Count, "Faltan textos en espa\xF1ol");
static_assert(sizeof(Portuguese) / sizeof(Portuguese[0]) == Count, "Faltan textos en portugu\xE9s");
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
        sprintf_s(out, capacity, "%s_%s%s", base, suffix, ext);
        if (GetFileAttributesA(out) != INVALID_FILE_ATTRIBUTES) return out;
        char line[160];
        wsprintfA(line, "Idioma: falta %s, se usa %s%s", out, base, ext);
        DbgLogPublic(line);
    }
    sprintf_s(out, capacity, "%s%s", base, ext);
    return out;
}
