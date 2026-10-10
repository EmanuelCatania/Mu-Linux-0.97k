#include "stdafx.h"
#include <climits>
#include "UI/OptionsMenu.h"
#include "Config/UserSettings.h"
#include "Core/Font.h"
#include "Core/Window.h"
#include "Local/ClientText.h"
#include "Net/HWID.h"
#include "Sound/SoundManager.h"

COptionsMenu gOptionsMenu;

namespace {
// DLL OptionsMenu.cpp: cajas de 120x22 centradas, separadas 7, desde y=60.
struct OptionsLayout {
    static constexpr int X = 320 - 60, Y = 60, Width = 120, Height = 22, Gap = 7;
    static constexpr int Step = Height + Gap;
    static constexpr int BarLabelWidth = Width / 3;           // "Sonido:"
    static constexpr int BarX = X + BarLabelWidth;
    static constexpr int BarWidth = Width - BarLabelWidth - 8;
    static constexpr int BarHeight = 8, BarY = (Height - BarHeight) / 2;
    static constexpr int Levels = 9;                          // CSound: 1..9 (0 = mudo)
};
using Layout = OptionsLayout;

const char* const LanguageNames[MAX_USER_LANGUAGE] = { "English", "Espa\xF1" "ol", "Portugu\xEA" "s" };

int RowY(int row) { return Layout::Y + row * Layout::Step; }

bool Inside(int x, int y, int width, int height)
{
    return MouseX >= x && MouseX < x + width && MouseY >= y && MouseY < y + height;
}

// DLL Util.cpp CenterTextPosY: alto del texto en el espacio de 480.
int CenterTextY(const char* text, int centerY)
{
    SIZE size = {};
    GetTextExtentPointA(gFont.GetTextDC(), text, lstrlenA(text), &size);
    return centerY - ((480 * size.cy / (int)gWindow.GetHeight()) >> 1);
}

void FillTriangle(float x, float y, float width, float height)
{
    const float left = (float)Screen_ToGLX(x), right = (float)Screen_ToGLX(x + width);
    const float top = (float)gWindow.GetHeight() - (float)Screen_ToGLY(y);
    const float bottom = (float)gWindow.GetHeight() - (float)Screen_ToGLY(y + height);
    glBegin(GL_TRIANGLES);
    glVertex2f(left, top);
    glVertex2f(left, bottom);
    glVertex2f(right, (top + bottom) / 2.0f);
    glEnd();
}

void OpenDialogData()
{
    char path[MAX_PATH];
    Dialog_LoadBMD(LocalizedDataPath("Data/Local/Dialog", ".bmd", path, sizeof(path)));
}
}

// IDA: RenderErrorMessage (0x0051AF50) arranca con DisableAlphaBlend; el DLL
// engancha ahí (0x0051AF65) para volver a la lista cuando no hay cartel.
void COptionsMenu::OnErrorMessageFrame()
{
    if (!DAT_083a7c24) m_Page = PAGE_MAIN;   // ErrorMessage
}

bool COptionsMenu::Clicked(int x, int y, int width, int height) const
{
    return Inside(x, y, width, height) && MouseLButton && MouseLButtonPush;
}

void COptionsMenu::ConsumeClick() const
{
    MouseLButtonPush = 0;
    DAT_07e11d28 = 0;   // MouseUpdateTime
    DAT_00559bec = 6;   // MouseUpdateTimeMax
    PlayBuffer(25, 0, 0);
}

// DLL RenderBox: bitmap 240 (Message_box) y, con el mouse encima, otra pasada
// aditiva en (0.8, 0.6, 0.4).  Los títulos usan ese color sin hover.
void COptionsMenu::RenderBox(float x, float y, float width, float height, bool title) const
{
    GL_ResetState();
    if (title) glColor3f(0.8f, 0.6f, 0.4f); else glColor3f(1.0f, 1.0f, 1.0f);
    GL_DrawTexture(240, x, y, width, height, 0.0f, 0.0f, 213.0f / 256.0f, 1.0f, 1, 1);
    if (!title && Inside((int)x, (int)y, (int)width, (int)height)) {
        glColor3f(0.8f, 0.6f, 0.4f);
        GL_SetBlendAdditive();
        GL_DrawTexture(240, x, y, width, height, 0.0f, 0.0f, 213.0f / 256.0f, 1.0f, 1, 1);
        GL_ResetState();
    }
    glColor3f(1.0f, 1.0f, 1.0f);
}

void COptionsMenu::RenderLabel(float x, float y, float width, const char* text) const
{
    EnableAlphaTest(true);
    RenderText((int)x, CenterTextY(text, (int)y + Layout::Height / 2), const_cast<char*>(text),
               (int)(width * gWindow.GetWidth() / 640), 1, nullptr);
}

void COptionsMenu::RenderToggle(float y, const char* label, bool value) const
{
    char text[96];
    sprintf_s(text, "%s: %s", label, value ? "On" : "Off");   // "On"/"Off" como el 0.97k
    RenderButton(y, text);
}

void COptionsMenu::RenderButton(float y, const char* label, bool title) const
{
    RenderBox((float)Layout::X, y, (float)Layout::Width, (float)Layout::Height, title);
    RenderLabel((float)Layout::X, y, (float)Layout::Width, label);
}

// true si el mouse está sobre la fila; `clicked` indica si se pulsó.
bool COptionsMenu::UpdateToggle(int y, bool& clicked) const
{
    clicked = false;
    if (!Inside(Layout::X, y, Layout::Width, Layout::Height)) return false;
    if (Clicked(Layout::X, y, Layout::Width, Layout::Height)) {
        ConsumeClick();
        clicked = true;
    }
    return true;
}

void COptionsMenu::RenderLevelBar(float y, const char* label, bool enabled, int level) const
{
    RenderBox((float)Layout::X, y, (float)Layout::Width, (float)Layout::Height);
    char text[96];
    if (!enabled) {
        sprintf_s(text, "%s: Off", label);
        RenderLabel((float)Layout::X, y, (float)Layout::Width, text);
        return;
    }
    sprintf_s(text, "%s:", label);
    RenderLabel((float)Layout::X, y, (float)Layout::BarLabelWidth, text);
    // DLL RenderSoundVolume: marco gris, fondo negro y un segmento blanco por nivel.
    const float bx = (float)Layout::BarX, by = y + Layout::BarY;
    const float segment = (float)Layout::BarWidth / Layout::Levels;
    EnableAlphaTest(true);
    glColor3f(0.2f, 0.2f, 0.2f);
    GL_DrawRect(bx, by, Layout::BarWidth + 2.0f, (float)Layout::BarHeight);
    glColor3f(0.0f, 0.0f, 0.0f);
    GL_DrawRect(bx + 1.0f, by + 1.0f, (float)Layout::BarWidth, Layout::BarHeight - 2.0f);
    for (int i = 0; i < Layout::Levels; ++i) {
        if (i < level) glColor3f(1.0f, 1.0f, 1.0f); else glColor3f(0.0f, 0.0f, 0.0f);
        GL_DrawRect(bx + 1.0f + i * segment, by + 1.0f, segment, Layout::BarHeight - 2.0f);
    }
    GL_ResetState();
    glColor3f(1.0f, 1.0f, 1.0f);
}

// Segmento de la barra bajo el mouse (nivel 1..9), o -1.  DESVIACION: volver a
// pulsar el primer segmento con nivel 1 deja el sonido en 0 (mudo).
int COptionsMenu::LevelBarHit(int y) const
{
    const int segment = Layout::BarWidth / Layout::Levels;
    for (int i = 0; i < Layout::Levels; ++i)
        if (Clicked(Layout::BarX + 1 + i * segment, y + Layout::BarY, segment, Layout::BarHeight))
            return i + 1;
    return -1;
}

// Caja con texto centrado y flechas a los costados (DLL RenderLanguage:
// bitmaps 0xFE/0xFF/0x100, normal/hover/apretada; la derecha va espejada).
void COptionsMenu::RenderSelector(float y, const char* text, bool canLeft, bool canRight) const
{
    RenderBox((float)Layout::X, y, (float)Layout::Width, (float)Layout::Height);
    RenderLabel((float)Layout::X, y, (float)Layout::Width, text);
    const float size = (float)Layout::Height;
    auto arrow = [&](float x, bool mirrored) {
        const int tex = !Inside((int)x, (int)y, Layout::Height, Layout::Height) ? 0xFE
                      : (MouseLButtonPush ? 0x100 : 0xFF);
        GL_DrawTexture(tex, x, y, size, size, mirrored ? 1.0f : 0.0f, 0.0f,
                       mirrored ? -1.0f : 1.0f, 1.0f, 1, 1);
    };
    if (canLeft) arrow((float)Layout::X, false);
    if (canRight) arrow((float)(Layout::X + Layout::Width - Layout::Height), true);
}

// -1 / +1 si se pulsó una flecha; 0 si el mouse está encima sin pulsar o en la
// caja; INT_MIN si está fuera.
int COptionsMenu::UpdateSelector(int y, bool canLeft, bool canRight) const
{
    const int size = Layout::Height, right = Layout::X + Layout::Width - size;
    if (canLeft && Clicked(Layout::X, y, size, size)) { ConsumeClick(); return -1; }
    if (canRight && Clicked(right, y, size, size)) { ConsumeClick(); return 1; }
    return Inside(Layout::X, y, Layout::Width, Layout::Height) ? 0 : INT_MIN;
}

void COptionsMenu::RenderLanguage(float y) const
{
    const int language = gUserSettings.GetLanguage();
    const char* name = language >= 0 ? LanguageNames[language]
                                     : gClientText.Get(ClientTextId::LanguageDefault);
    RenderSelector(y, name, language > USER_LANG_DEFAULT, language < MAX_USER_LANGUAGE - 1);
}

void COptionsMenu::RenderMusicControls(float y) const
{
    const float width = Layout::Width / 2.0f, x = Layout::X + width / 2.0f;
    RenderBox(x, y, width, (float)Layout::Height);
    const float icon = Layout::Height / 2.0f;
    const float ix = x + (width - icon) / 2.0f, iy = y + (Layout::Height - icon) / 2.0f;
    EnableAlphaTest(true);
    if (gSound.IsMusicStoppedByUser()) {
        glColor3f(0.0f, 1.0f, 0.0f);
        FillTriangle(ix, iy, icon, icon);
    } else {
        glColor3f(1.0f, 0.0f, 0.0f);
        GL_DrawRect(ix, iy, icon, icon);
    }
    GL_ResetState();
    glColor3f(1.0f, 1.0f, 1.0f);
}

// DLL Language::ReloadLanguage: textos, diálogos y aviso al server.
void COptionsMenu::ChangeLanguage(int language)
{
    gUserSettings.SetLanguage(language);
    OpenTextData();
    OpenDialogData();
    if (language != USER_LANG_DEFAULT) Lang_Send(language);
}

void COptionsMenu::Render()
{
    const DWORD color = m_dwTextColor, back = m_dwBackColor;
    m_dwTextColor = 0xFFFFFFFFu;
    m_dwBackColor = 0;
    switch (m_Page) {
    case PAGE_GENERAL: RenderGeneral(); break;
    case PAGE_ANTILAG: RenderAntilag(); break;
    case PAGE_SCREEN: RenderScreen(); break;
    case PAGE_FONT: RenderFont(); break;
    case PAGE_ANTILAG_WORLD:
    case PAGE_ANTILAG_EFFECTS:
    case PAGE_ANTILAG_INTERFACE: RenderAntilagGroup(); break;
    default:           RenderMain(); break;
    }
    m_dwTextColor = color;
    m_dwBackColor = back;
}

bool COptionsMenu::UpdateMouse()
{
    switch (m_Page) {
    case PAGE_GENERAL: return UpdateGeneral();
    case PAGE_ANTILAG: return UpdateAntilag();
    case PAGE_SCREEN: return UpdateScreen();
    case PAGE_FONT: return UpdateFont();
    case PAGE_ANTILAG_WORLD:
    case PAGE_ANTILAG_EFFECTS:
    case PAGE_ANTILAG_INTERFACE: return UpdateAntilagGroup();
    default:           return UpdateMain();
    }
}

// ── Lista principal ─────────────────────────────────────────────────────────
// Título, una caja por página y Cerrar (GlobalText 385, 919/926/920/921, 388).
namespace { const int MainPageText[] = { 919, 926, 920, 921 }; }

void COptionsMenu::RenderMain()
{
    int row = 0;
    RenderBox((float)Layout::X, (float)RowY(row), (float)Layout::Width, (float)Layout::Height, true);
    RenderLabel((float)Layout::X, (float)RowY(row++), (float)Layout::Width, GlobalText[385]);
    for (int text : MainPageText) {
        RenderBox((float)Layout::X, (float)RowY(row), (float)Layout::Width, (float)Layout::Height);
        RenderLabel((float)Layout::X, (float)RowY(row++), (float)Layout::Width, GlobalText[text]);
    }
    RenderBox((float)Layout::X, (float)RowY(row), (float)Layout::Width, (float)Layout::Height);
    RenderLabel((float)Layout::X, (float)RowY(row), (float)Layout::Width, GlobalText[388]);
}

bool COptionsMenu::UpdateMain()
{
    int row = 1;
    for (int page = PAGE_GENERAL; page < PAGE_COUNT; ++page, ++row) {
        if (!Inside(Layout::X, RowY(row), Layout::Width, Layout::Height)) continue;
        if (Clicked(Layout::X, RowY(row), Layout::Width, Layout::Height)) {
            ConsumeClick();
            m_Page = (Page)page;
        }
        return true;
    }
    if (!Inside(Layout::X, RowY(row), Layout::Width, Layout::Height)) return false;
    if (Clicked(Layout::X, RowY(row), Layout::Width, Layout::Height)) {
        // DLL CheckClose: ErrorMessage = NextErrorMessage, como el 0.97k.
        ConsumeClick();
        m_Page = PAGE_MAIN;
        DAT_083a7c24 = DAT_083a7c28;
        DAT_083a7c28 = 0;
    }
    return true;
}

// ── General ─────────────────────────────────────────────────────────────────
// Idioma (sólo en char-select y en el juego, donde hay Text.bmd cargado y
// conexión), ataque automático, sonido de susurros, volúmenes, música y Volver.
namespace {
bool LanguageRowVisible() { return SceneFlag == 4 || SceneFlag == 5; }
}

void COptionsMenu::RenderGeneral()
{
    int row = 0;
    RenderBox((float)Layout::X, (float)RowY(row), (float)Layout::Width, (float)Layout::Height, true);
    RenderLabel((float)Layout::X, (float)RowY(row++), (float)Layout::Width, GlobalText[919]);
    if (LanguageRowVisible()) RenderLanguage((float)RowY(row++));
    RenderToggle((float)RowY(row++), GlobalText[936], gUserSettings.GetPvPWithoutControl());
    RenderToggle((float)RowY(row++), GlobalText[386], m_bAutoAttack != 0);
    RenderToggle((float)RowY(row++), GlobalText[387], m_bWhisperSound != 0);
    RenderLevelBar((float)RowY(row++), GlobalText[922], g_EnableSound != 0, gSound.GetSoundLevel());
    RenderLevelBar((float)RowY(row++), GlobalText[923], gSound.GetMusicEnabled() != 0, gSound.GetMusicLevel());
    if (gSound.GetMusicEnabled()) RenderMusicControls((float)RowY(row++));
    RenderBox((float)Layout::X, (float)RowY(row), (float)Layout::Width, (float)Layout::Height);
    RenderLabel((float)Layout::X, (float)RowY(row), (float)Layout::Width, GlobalText[925]);
}

bool COptionsMenu::UpdateGeneral()
{
    int row = 1;
    const int width = Layout::Width, height = Layout::Height, x = Layout::X;
    if (LanguageRowVisible()) {
        const int language = gUserSettings.GetLanguage();
        const int step = UpdateSelector(RowY(row++), language > USER_LANG_DEFAULT,
                                        language < MAX_USER_LANGUAGE - 1);
        if (step != INT_MIN) {
            if (step) ChangeLanguage(language + step);
            return true;
        }
    }
    bool clicked;
    if (UpdateToggle(RowY(row++), clicked)) {
        if (clicked) gUserSettings.SetPvPWithoutControl(!gUserSettings.GetPvPWithoutControl());
        return true;
    }
    int y = RowY(row++);
    if (Inside(x, y, width, height)) {
        if (Clicked(x, y, width, height)) { ConsumeClick(); m_bAutoAttack ^= 1; }
        return true;
    }
    y = RowY(row++);
    if (Inside(x, y, width, height)) {
        if (Clicked(x, y, width, height)) { ConsumeClick(); m_bWhisperSound ^= 1; }
        return true;
    }
    y = RowY(row++);
    if (g_EnableSound) {
        const int level = LevelBarHit(y);
        if (level > 0) {
            ConsumeClick();
            const int value = (level == 1 && gSound.GetSoundLevel() == 1) ? 0 : level;
            gSound.SetLevels(value, gSound.GetMusicLevel());
            gUserSettings.SetSoundLevel(value);
            return true;
        }
    }
    y = RowY(row++);
    if (gSound.GetMusicEnabled()) {
        const int level = LevelBarHit(y);
        if (level > 0) {
            ConsumeClick();
            const int value = (level == 1 && gSound.GetMusicLevel() == 1) ? 0 : level;
            gSound.SetLevels(gSound.GetSoundLevel(), value);
            gUserSettings.SetMusicLevel(value);
            return true;
        }
        y = RowY(row++);
        const int cx = x + width / 4;
        if (Inside(cx, y, width / 2, height)) {
            if (Clicked(cx, y, width / 2, height)) {
                ConsumeClick();
                gSound.SetMusicStoppedByUser(!gSound.IsMusicStoppedByUser());
            }
            return true;
        }
    }
    y = RowY(row);
    if (Inside(x, y, width, height)) {
        if (Clicked(x, y, width, height)) { ConsumeClick(); m_Page = PAGE_MAIN; }
        return true;
    }
    return false;
}

// ── Antilag ─────────────────────────────────────────────────────────────────
// DESVIACION: el DLL apila una caja por opción.  Con las dos propias (clima y
// brillo) se agrupan en submenús por lo que ocultan.
namespace {
struct AntilagGroup {
    ClientTextId title;
    eAntilag options[5];
    int count;
};
const AntilagGroup AntilagGroups[] = {
    { ClientTextId::AntilagWorld,
      { ANTILAG_SHADOWS, ANTILAG_OBJECTS, ANTILAG_FLOOR, ANTILAG_WEATHER }, 4 },
    { ClientTextId::AntilagEffects,
      { ANTILAG_SKILLS, ANTILAG_STATIC_EFFECTS, ANTILAG_DYNAMIC_EFFECTS, ANTILAG_GLOW,
        ANTILAG_WINGS }, 5 },
    { ClientTextId::AntilagInterface,
      { ANTILAG_INTERFACE, ANTILAG_HEALTH_BAR }, 2 },
};
constexpr int AntilagGroupCount = sizeof(AntilagGroups) / sizeof(AntilagGroups[0]);

// Las del DLL tienen texto en Text.bmd (927-935); clima y brillo son propias.
const char* AntilagLabel(int option)
{
    if (option == ANTILAG_WEATHER) return gClientText.Get(ClientTextId::AntilagWeather);
    if (option == ANTILAG_GLOW) return gClientText.Get(ClientTextId::AntilagGlow);
    return GlobalText[927 + option];
}
}

void COptionsMenu::RenderAntilag()
{
    int row = 0;
    RenderButton((float)RowY(row++), GlobalText[926], true);
    char text[96];
    for (const auto& group : AntilagGroups) {
        sprintf_s(text, "%s >", gClientText.Get(group.title));
        RenderButton((float)RowY(row++), text);
    }
    RenderButton((float)RowY(row), GlobalText[925]);
}

bool COptionsMenu::UpdateAntilag()
{
    int row = 1;
    bool clicked;
    for (int i = 0; i < AntilagGroupCount; ++i) {
        if (!UpdateToggle(RowY(row++), clicked)) continue;
        if (clicked) m_Page = (Page)(PAGE_ANTILAG_WORLD + i);
        return true;
    }
    if (!UpdateToggle(RowY(row), clicked)) return false;
    if (clicked) m_Page = PAGE_MAIN;
    return true;
}

void COptionsMenu::RenderAntilagGroup()
{
    const AntilagGroup& group = AntilagGroups[m_Page - PAGE_ANTILAG_WORLD];
    int row = 0;
    RenderButton((float)RowY(row++), gClientText.Get(group.title), true);
    for (int i = 0; i < group.count; ++i)
        RenderToggle((float)RowY(row++), AntilagLabel(group.options[i]),
                     gUserSettings.GetAntilag(group.options[i]));
    RenderButton((float)RowY(row), GlobalText[925]);
}

bool COptionsMenu::UpdateAntilagGroup()
{
    const AntilagGroup& group = AntilagGroups[m_Page - PAGE_ANTILAG_WORLD];
    int row = 1;
    bool clicked;
    for (int i = 0; i < group.count; ++i) {
        if (!UpdateToggle(RowY(row++), clicked)) continue;
        if (clicked) gUserSettings.SetAntilag(group.options[i], !gUserSettings.GetAntilag(group.options[i]));
        return true;
    }
    if (!UpdateToggle(RowY(row), clicked)) return false;
    if (clicked) m_Page = PAGE_ANTILAG;   // Volver: a la página de Antilag
    return true;
}

// ── Pantalla ────────────────────────────────────────────────────────────────
// Modo (ventana / pantalla completa), sin bordes (sólo en ventana) y la
// resolución.  DESVIACION: el DLL lista una caja por resolución; acá es un
// selector con flechas sobre la misma tabla (CUserSettings).
namespace {
void ApplyScreen(bool windowMode, bool borderless, int resolution)
{
    DWORD width = gWindow.GetWidth(), height = gWindow.GetHeight();
    CUserSettings::GetResolutionSize(resolution, &width, &height);
    gWindow.ChangeMode(windowMode, borderless, width, height);
    gUserSettings.SetWindow(windowMode, borderless, resolution);
}
int CurrentResolution()
{
    return CUserSettings::FindResolution(gWindow.GetWidth(), gWindow.GetHeight());
}
}

void COptionsMenu::RenderScreen()
{
    int row = 0;
    RenderButton((float)RowY(row++), GlobalText[920], true);
    RenderButton((float)RowY(row++), gClientText.Get(gWindow.IsWindowMode()
        ? ClientTextId::ScreenWindowed : ClientTextId::ScreenFullscreen));
    if (gWindow.IsWindowMode())
        RenderToggle((float)RowY(row++), gClientText.Get(ClientTextId::ScreenBorderless),
                     gWindow.IsBorderless());
    char text[32];
    sprintf_s(text, "%lu x %lu", (unsigned long)gWindow.GetWidth(), (unsigned long)gWindow.GetHeight());
    const int resolution = CurrentResolution();
    RenderSelector((float)RowY(row++), text, resolution != 0, resolution < MAX_USER_RESOLUTION - 1);
    RenderButton((float)RowY(row), GlobalText[925]);
}

bool COptionsMenu::UpdateScreen()
{
    int row = 1;
    bool clicked;
    const bool windowMode = gWindow.IsWindowMode(), borderless = gWindow.IsBorderless();
    const int resolution = CurrentResolution();
    if (UpdateToggle(RowY(row++), clicked)) {
        if (clicked) ApplyScreen(!windowMode, borderless, resolution);
        return true;
    }
    if (windowMode) {
        if (UpdateToggle(RowY(row++), clicked)) {
            if (clicked) ApplyScreen(windowMode, !borderless, resolution);
            return true;
        }
    }
    // Fuera de la tabla (resolución del registro): la flecha va a la primera.
    const int step = UpdateSelector(RowY(row++), resolution != 0, resolution < MAX_USER_RESOLUTION - 1);
    if (step != INT_MIN) {
        if (step) ApplyScreen(windowMode, borderless, resolution < 0 ? 0 : resolution + step);
        return true;
    }
    if (!UpdateToggle(RowY(row), clicked)) return false;
    if (clicked) m_Page = PAGE_MAIN;
    return true;
}

// ── Fuente ──────────────────────────────────────────────────────────────────
// Familia (fuentes instaladas), negrita, cursiva, tamaño 1..25 y reiniciar.
// DESVIACION: "Reiniciar fuente" del DLL volvía a Verdana 13; acá borra [Font]
// y vuelve la fuente del 0.97k (Arial con el alto según la resolución).
namespace {
// Configuración vigente: la de [Font] o, sin ella, la que armó CFont.
UserFontSettings CurrentFont()
{
    UserFontSettings font = gUserSettings.GetFont();
    if (!font.present) {
        font = UserFontSettings{};
        strcpy_s(font.faceName, "Arial");
        font.height = gFont.GetHeight();
    }
    return font;
}
void ApplyFont(const UserFontSettings& font)
{
    gUserSettings.SetFont(font);
    gFont.Reload(gWindow.GetWidth());
}
}

void COptionsMenu::RenderFont()
{
    const UserFontSettings font = CurrentFont();
    const int face = gFont.FindFace(font.faceName);
    int row = 0;
    RenderButton((float)RowY(row++), GlobalText[921], true);
    RenderSelector((float)RowY(row++), font.faceName, face != 0, face < gFont.GetFaceCount() - 1);
    RenderToggle((float)RowY(row++), gClientText.Get(ClientTextId::FontBold), font.bold != 0);
    RenderToggle((float)RowY(row++), gClientText.Get(ClientTextId::FontItalic), font.italic != 0);
    char text[16];
    sprintf_s(text, "%d", font.height);
    RenderSelector((float)RowY(row++), text, font.height > 1, font.height < 25);
    RenderButton((float)RowY(row++), GlobalText[924]);
    RenderButton((float)RowY(row), GlobalText[925]);
}

bool COptionsMenu::UpdateFont()
{
    UserFontSettings font = CurrentFont();
    const int face = gFont.FindFace(font.faceName);
    int row = 1;
    bool clicked;
    int step = UpdateSelector(RowY(row++), face != 0, face < gFont.GetFaceCount() - 1);
    if (step != INT_MIN) {
        if (step) {
            // Una fuente fuera de la lista arranca desde la primera.
            strcpy_s(font.faceName, gFont.GetFaceName(face < 0 ? 0 : face + step));
            ApplyFont(font);
        }
        return true;
    }
    if (UpdateToggle(RowY(row++), clicked)) {
        if (clicked) { font.bold = !font.bold; ApplyFont(font); }
        return true;
    }
    if (UpdateToggle(RowY(row++), clicked)) {
        if (clicked) { font.italic = !font.italic; ApplyFont(font); }
        return true;
    }
    step = UpdateSelector(RowY(row++), font.height > 1, font.height < 25);
    if (step != INT_MIN) {
        if (step) { font.height += step; ApplyFont(font); }
        return true;
    }
    if (UpdateToggle(RowY(row++), clicked)) {
        if (clicked) { gUserSettings.ResetFont(); gFont.Reload(gWindow.GetWidth()); }
        return true;
    }
    if (!UpdateToggle(RowY(row), clicked)) return false;
    if (clicked) m_Page = PAGE_MAIN;
    return true;
}
