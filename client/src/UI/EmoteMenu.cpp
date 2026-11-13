// EmoteMenu.cpp — listas de emotes del chat. Ver EmoteMenu.h. Misma mecanica que el
// selector de emojis (EmojiPicker.cpp): se dibuja en el render del chat y atiende
// sus propios clicks.
#include "stdafx.h"
#include "UI/EmoteMenu.h"
#include "Model/Model_CustomAnim.h"
#include "Render/BufferedDraw.h"
#include "platform/Text.h"
#include "Core/Font.h"
#include "Core/Window.h"
#include <string>
#include <vector>
#include <cstdio>

bool Emote_Play(int n, bool faceFront);                 // UI/Chat_Gestures.cpp
int  ClassicGesture_Count();
const char* ClassicGesture_Name(int index);
bool ClassicGesture_Play(int index, bool faceFront);

namespace EmoteMenu {
namespace {
constexpr int kRows = 12, kCellH = 14;

struct Geometry { int columns, cellW; };
constexpr Geometry kGeometry[2] = { { 4, 112 },    // Fortnite: 4 columnas
                                    { 2, 120 } };  // Clasicos: 2 columnas

struct State {
    bool open = false;
    int page = 0;
    int left = 0, top = 0;      // ultimo rectangulo dibujado, para Contains()
    std::vector<std::string> names;
};
State s_state[2];
int s_pendingWhich = -1;        // lista del emote elegido (-1 = ninguno)
int s_pendingIndex = -1;        // y su indice, a lanzar en el proximo paso de la simulacion
bool s_swallow = false;         // el click ya fue del menu: no pasa al mundo hasta soltar el boton

int PerPage(int which) { return kGeometry[which].columns * kRows; }
int PanelWidth(int which) { return kGeometry[which].columns * kGeometry[which].cellW + 8; }
constexpr int kPanelHeight = kRows * kCellH + 8 + 22;      // grilla + pie con paginas

// "Emote_Hip_Hop" -> "Hip Hop"; "ElectroShuffle" -> "Electro Shuffle"; sin el "Loop" final.
std::string Pretty(const char* raw)
{
    std::string in = raw;
    if (in.rfind("Emote_", 0) == 0) in.erase(0, 6);
    std::string out;
    for (size_t i = 0; i < in.size(); ++i) {
        const char c = in[i];
        if (c == '_') { out += ' '; continue; }
        if (i > 0 && c >= 'A' && c <= 'Z' && in[i - 1] >= 'a' && in[i - 1] <= 'z') out += ' ';
        out += c;
    }
    const std::string loop = " Loop";
    if (out.size() > loop.size() && out.compare(out.size() - loop.size(), loop.size(), loop) == 0)
        out.erase(out.size() - loop.size());
    return out;
}

const std::vector<std::string>& Names(int which)
{
    std::vector<std::string>& names = s_state[which].names;
    const int count = which == Fortnite ? CustomAnim_Count() : ClassicGesture_Count();
    if ((int)names.size() != count) {
        names.clear();
        for (int i = 0; i < count; ++i)
            names.push_back(which == Fortnite ? Pretty(CustomAnim_Name(i)) : ClassicGesture_Name(i));
    }
    return names;
}

bool Inside(int x, int y, int w, int h)
{
    return MouseX >= x && MouseX < x + w && MouseY >= y && MouseY < y + h;
}

void Label(int x, int y, int width, const char* text)
{
    gFont.SelectFont(gFont.GetTextDC(), gFont.GetFont(FONT_NORMAL));
    SIZE size = {};
    gFont.MeasureText(gFont.GetTextDC(), text, Platform::TextLength(text), &size);
    const int cy = y - ((480 * size.cy / (int)gWindow.GetHeight()) >> 1);
    RenderText(x, cy, const_cast<char*>(text), width * (int)gWindow.GetWidth() / 640, 1, nullptr);
}

void Consume()
{
    s_swallow = true;
    MouseLButtonPush = 0;
    DAT_07e11d28 = 0;   // MouseUpdateTime
    DAT_00559bec = 6;   // MouseUpdateTimeMax
    PlayBuffer(25, 0, 0);
}

void RenderOne(int which, int bottomY)
{
    State& st = s_state[which];
    if (!st.open) return;
    const std::vector<std::string>& names = Names(which);
    if (names.empty()) { st.open = false; return; }
    const int perPage = PerPage(which);
    const int pages = ((int)names.size() + perPage - 1) / perPage;
    if (st.page >= pages) st.page = pages - 1;
    const int panelWidth = PanelWidth(which);
    const int cellW = kGeometry[which].cellW;

    st.left = 320 - panelWidth / 2;            // centrada, como la franja de botones
    st.top = bottomY - 6 - kPanelHeight;
    bool click = MouseLButton && MouseLButtonPush;

    EnableAlphaTest(true);
    BufferedDraw::Color4f(0.0f, 0.0f, 0.0f, 0.88f);
    GL_DrawRect((float)st.left, (float)st.top, (float)panelWidth, (float)kPanelHeight);
    BufferedDraw::Color4f(0.8f, 0.6f, 0.4f, 0.9f);
    GL_DrawRect((float)st.left, (float)st.top, (float)panelWidth, 1.0f);
    GL_DrawRect((float)st.left, (float)(st.top + kPanelHeight - 1), (float)panelWidth, 1.0f);
    GL_DrawRect((float)st.left, (float)st.top, 1.0f, (float)kPanelHeight);
    GL_DrawRect((float)(st.left + panelWidth - 1), (float)st.top, 1.0f, (float)kPanelHeight);

    // Lista: se recorre por columnas (de arriba abajo, despues la columna siguiente).
    const int gridX = st.left + 4, gridY = st.top + 4;
    const int first = st.page * perPage;
    int picked = -1;
    for (int k = 0; k < perPage && first + k < (int)names.size(); ++k) {
        const int col = k / kRows, row = k % kRows;
        const int x = gridX + col * cellW, y = gridY + row * kCellH;
        const bool over = Inside(x, y, cellW, kCellH);
        if (over) {
            BufferedDraw::Color4f(1.0f, 1.0f, 1.0f, 0.22f);
            GL_DrawRect((float)x, (float)y, (float)cellW, (float)kCellH);
            picked = first + k;
        }
        BufferedDraw::Color4f(over ? 1.0f : 0.85f, over ? 0.85f : 0.8f, over ? 0.55f : 0.7f, 1.0f);
        Label(x + 3, y + kCellH / 2, cellW - 3, names[first + k].c_str());
    }

    // Pie: pagina anterior, "n / total" y pagina siguiente (solo si hay mas de una).
    int step = 0;
    if (pages > 1) {
        const int footY = gridY + kRows * kCellH + 11;
        const int prevX = st.left + 6, nextX = st.left + panelWidth - 6 - 16;
        char label[16];
        Platform::TextFormatSecure(label, "%d / %d", st.page + 1, pages);
        BufferedDraw::Color4f(1.0f, 1.0f, 1.0f, 1.0f);
        Label(st.left, footY, panelWidth, label);
        const char* const arrows[2] = { "<", ">" };
        const int arrowX[2] = { prevX, nextX };
        const bool enabled[2] = { st.page > 0, st.page < pages - 1 };
        for (int a = 0; a < 2; ++a) {
            if (!enabled[a]) continue;
            const bool over = Inside(arrowX[a], footY - 8, 16, 16);
            BufferedDraw::Color4f(over ? 1.0f : 0.7f, over ? 0.8f : 0.6f, 0.4f, 1.0f);
            Label(arrowX[a], footY, 16, arrows[a]);
            if (over && click) step = a == 0 ? -1 : 1;
        }
    }

    if (click) {
        if (step) { st.page += step; Consume(); }
        else if (picked >= 0) {
            s_pendingWhich = which;
            s_pendingIndex = picked;
            st.open = false;
            Consume();
        } else if (!Inside(st.left, st.top, panelWidth, kPanelHeight)) {
            st.open = false;   // click afuera: se cierra y el click sigue su camino
        }
    }
    BufferedDraw::Color4f(1.0f, 1.0f, 1.0f, 1.0f);
}
}

void Toggle(Which which)
{
    const bool open = !s_state[which].open;
    s_state[Fortnite].open = s_state[Classic].open = false;
    s_state[which].open = open;
    if (open) s_state[which].page = 0;
    s_swallow = true;
}

bool IsOpen(Which which) { return s_state[which].open; }
bool AnyOpen() { return s_state[Fortnite].open || s_state[Classic].open; }

void ApplyPending()
{
    if (s_pendingIndex < 0) return;
    const int which = s_pendingWhich, index = s_pendingIndex;
    s_pendingWhich = s_pendingIndex = -1;
    if (which == Fortnite) Emote_Play(index, true);          // de frente a la camara
    else ClassicGesture_Play(index, true);
}

bool Wheel(int notches)
{
    if (notches == 0) return false;
    for (int w = 0; w < 2; ++w) {
        State& st = s_state[w];
        if (!st.open || !Inside(st.left, st.top, PanelWidth(w), kPanelHeight)) continue;
        const int pages = ((int)Names(w).size() + PerPage(w) - 1) / PerPage(w);
        int page = st.page - (notches > 0 ? 1 : -1);    // rueda hacia arriba = pagina anterior
        if (page < 0) page = 0;
        if (page > pages - 1) page = pages - 1;
        if (page != st.page) { st.page = page; PlayBuffer(25, 0, 0); }
        return true;
    }
    return false;
}

bool Contains(int x, int y)
{
    // Con el boton todavia apretado tras un click del menu (elegir un emote lo cierra),
    // el mundo lo tomaria como "mantener para caminar" y cancelaria el emote al instante.
    if (s_swallow) {
        if (MouseLButton) return true;
        s_swallow = false;
    }
    for (int w = 0; w < 2; ++w) {
        const State& st = s_state[w];
        if (st.open && x >= st.left && x < st.left + PanelWidth(w) && y >= st.top && y < st.top + kPanelHeight + 4)
            return true;
    }
    return false;
}

void Render(int bottomY)
{
    RenderOne(Fortnite, bottomY);
    RenderOne(Classic, bottomY);
}
}
