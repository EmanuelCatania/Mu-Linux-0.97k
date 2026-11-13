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
    "Could not initialize OpenGL 3.3 core. Check the graphics driver.",
    "Starts in %s",
    "Render: %s",
    "Applies on restart",
    "Textures: %s",
    "Sharpness: %s",
    "Antialiasing: %s",
    "Advanced",
    "Graphics",
    "High definition: %s",
    "Shadows: %s",
    "Depth fog",
    "Draw distance: %s",
    "Time left: %s", "Cast by: %s", "Damage +%d", "Defense +%d",
    "Damage taken -%d%%", "Max HP +%d%%",
    "%s: %d of %d alive",
    "Movement rejected by the server",
    "Attack too fast",
    "Skills cannot be used right now",
    "Skills cannot be used in a safe zone",
    "You do not have that skill",
    "The skill is not ready yet",
    "Your class cannot use that skill",
    "Skill rejected by the server",
    "Triple Shot needs a bow and arrows",
    "Too far to drop the item there",
    "No NPC is attending you",
    "The NPC is too far away",
    "That NPC does not repair",
    "Trade canceled: too far apart",
    "Open the shop to repair"
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
        { ClientTextId::RenderBackend, "Render: %s", "Render: %s" },
        { ClientTextId::TextureQuality, "Texturas: %s", "Texturas: %s" },
        { ClientTextId::Sharpness, "Nitidez: %s", "Nitidez: %s" },
        { ClientTextId::Antialiasing, "Antialiasing: %s", "Antialiasing: %s" },
        { ClientTextId::OptionsAdvanced, "Avanzado", "Avançado" },
        { ClientTextId::OptionsGraphics, "Gráficos", "Gráficos" },
        { ClientTextId::HDTextures, "Alta definición: %s", "Alta definição: %s" },
        { ClientTextId::Shadows, "Sombras: %s", "Sombras: %s" },
        { ClientTextId::DepthFog, "Niebla de profundidad", "N\xC3\xA9voa de profundidade" },
        { ClientTextId::DrawDistance, "Distancia: %s", "Dist\xC3\xA2ncia: %s" },
        { ClientTextId::BuffTimeLeft, "Tiempo restante: %s", "Tempo restante: %s" },
        { ClientTextId::BuffCaster, "Lanzado por: %s", "Lan\xC3\xA7" "ado por: %s" },
        { ClientTextId::BuffDamage, "Da\xC3\xB1" "o +%d", "Dano +%d" },
        { ClientTextId::BuffDefense, "Defensa +%d", "Defesa +%d" },
        { ClientTextId::BuffDamageReduction, "Da\xC3\xB1" "o recibido -%d%%", "Dano recebido -%d%%" },
        { ClientTextId::BuffMaxLife, "Vida m\xC3\xA1" "xima +%d%%", "Vida m\xC3\xA1" "xima +%d%%" },
        { ClientTextId::InvasionAlive, "%s: %d de %d vivos", "%s: %d de %d vivos" },
        { ClientTextId::RenderBackendRestart, "Se aplica al reiniciar", "Aplica ao reiniciar" },
        { ClientTextId::RenderFps, "FPS: %d", "FPS: %d" },
        { ClientTextId::RenderUnlimited, "FPS: sin l\xC3\xAD" "mite", "FPS: sem limite" },
        { ClientTextId::RejectMove, "El server rechaz\xC3\xB3 el movimiento", "O servidor rejeitou o movimento" },
        { ClientTextId::RejectAttackSpeed, "Ataque demasiado r\xC3\xA1pido", "Ataque r\xC3\xA1pido demais" },
        { ClientTextId::RejectSkillState, "No se pueden usar skills ahora", "N\xC3\xA3o \xC3\xA9 poss\xC3\xADvel usar skills agora" },
        { ClientTextId::RejectSkillSafeZone, "No se pueden usar skills en zona segura", "N\xC3\xA3o \xC3\xA9 poss\xC3\xADvel usar skills na zona segura" },
        { ClientTextId::RejectSkillNotLearned, "No ten\xC3\xA9s ese skill", "Voc\xC3\xAA n\xC3\xA3o tem essa skill" },
        { ClientTextId::RejectSkillDelay, "El skill todav\xC3\xAD" "a no est\xC3\xA1 listo", "A skill ainda n\xC3\xA3o est\xC3\xA1 pronta" },
        { ClientTextId::RejectSkillClass, "Tu clase no puede usar ese skill", "Sua classe n\xC3\xA3o pode usar essa skill" },
        { ClientTextId::RejectSkillSequence, "El server rechaz\xC3\xB3 el skill", "O servidor rejeitou a skill" },
        { ClientTextId::RejectSkillNoArrows, "Triple Shot necesita arco y flechas", "Triple Shot precisa de arco e flechas" },
        { ClientTextId::RejectDropRange, "Demasiado lejos para tirar el item ah\xC3\xAD", "Longe demais para largar o item a\xC3\xAD" },
        { ClientTextId::RejectNpcMissing, "Ning\xC3\xBAn NPC te est\xC3\xA1 atendiendo", "Nenhum NPC est\xC3\xA1 te atendendo" },
        { ClientTextId::RejectNpcRange, "El NPC est\xC3\xA1 demasiado lejos", "O NPC est\xC3\xA1 longe demais" },
        { ClientTextId::RejectNpcNoRepair, "Ese NPC no repara", "Esse NPC n\xC3\xA3o conserta" },
        { ClientTextId::RejectTradeRange, "Trade cancelado: est\xC3\xA1n demasiado lejos", "Troca cancelada: longe demais" },
        { ClientTextId::RejectRepairNoShop, "Abr\xC3\xAD la tienda para reparar", "Abra a loja para consertar" },
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
