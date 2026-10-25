#include "stdafx.h"
#include "UI/EmojiPicker.h"
#include "Render/BufferedDraw.h"
#include "Platform/Text.h"
#include "Text/TextCore.h"
#include "Core/Font.h"
#include "Core/Window.h"
#include <string>
#include <vector>

extern "C" bool Input_AppendText(DWORD slot, const char* utf8);

namespace EmojiPicker {
namespace {
// Dentro del campo de texto del chat, pegado a su borde derecho y antes del
// campo del susurro (que arranca en x=376).
constexpr int kButtonX = 358, kButtonY = 420, kButtonSize = 16;
constexpr int kCell = 20, kColumns = 10, kRows = 8;
constexpr int kPanelWidth = kColumns * kCell + 8;
constexpr int kPanelHeight = kRows * kCell + 8 + 20;        // grilla + pie con páginas
constexpr int kPanelX = kButtonX + kButtonSize - kPanelWidth, kPanelY = kButtonY - 4 - kPanelHeight;

bool s_Open = false;
int s_Page = 0;

// Página 0: el bloque Emoticons entero (caras y gestos). Página 1: lo que más
// se usa en el juego, a mano.
std::vector<std::string> BuildPage(int page)
{
    std::vector<std::string> list;
    if (page == 0) {
        for (char32_t cp = 0x1F600; cp <= 0x1F64F; ++cp) {
            std::string s;
            TextEngine::Append(s, cp);
            list.push_back(s);
        }
        return list;
    }
    static const char32_t extra[] = {
        0x1F44D, 0x1F44E, 0x1F44F, 0x1F64C, 0x1F64F, 0x1F4AA, 0x1F44B, 0x1F44C, 0x270C, 0x1F91D,
        0x1F44A, 0x270A, 0x270B, 0x1F448, 0x1F449, 0x1F446, 0x1F447, 0x1F918, 0x1F440, 0x1F9E0,
        0x2764, 0x1F9E1, 0x1F49B, 0x1F49A, 0x1F499, 0x1F49C, 0x1F5A4, 0x1F494, 0x1F495, 0x1F496,
        0x1F525, 0x2728, 0x2B50, 0x1F31F, 0x1F4A5, 0x1F4AF, 0x1F389, 0x1F381, 0x1F3C6, 0x1F3AE,
        0x2694, 0x1F6E1, 0x1F3F9, 0x1F48E, 0x1F4B0, 0x1F451, 0x1F480, 0x1F47B, 0x1F47D, 0x1F916,
        0x1F409, 0x1F43A, 0x1F984, 0x1F431, 0x1F436, 0x1F37A, 0x1F37B, 0x2615, 0x1F355, 0x1F354,
        0x1F382, 0x26A1, 0x2744, 0x1F30A, 0x2600, 0x1F319, 0x1F340, 0x1F339, 0x1F4A9, 0x1F4A4,
    };
    for (char32_t cp : extra) {
        std::string s;
        TextEngine::Append(s, cp);
        // Selector de variación: sin él algunos símbolos salen en blanco y negro.
        if (cp < 0x1F000) TextEngine::Append(s, 0xFE0F);
        list.push_back(s);
    }
    return list;
}
constexpr int kPages = 2;

bool Inside(int x, int y, int w, int h)
{
    return MouseX >= x && MouseX < x + w && MouseY >= y && MouseY < y + h;
}

// El texto del selector va con el estilo del chat: sin tintes de otro panel.
void Label(int x, int y, int width, const char* text)
{
    SIZE size = {};
    gFont.MeasureText(gFont.GetTextDC(), text, Platform::TextLength(text), &size);
    const int cy = y - ((480 * size.cy / (int)gWindow.GetHeight()) >> 1);
    RenderText(x, cy, const_cast<char*>(text), width * (int)gWindow.GetWidth() / 640, 1, nullptr);
}

void Consume()
{
    MouseLButtonPush = 0;
    DAT_07e11d28 = 0;   // MouseUpdateTime
    DAT_00559bec = 6;   // MouseUpdateTimeMax
    PlayBuffer(25, 0, 0);
}
}

void OnChatClosed() { s_Open = false; }

bool Contains(int x, int y)
{
    if (x >= kButtonX && x < kButtonX + kButtonSize && y >= kButtonY && y < kButtonY + kButtonSize) return true;
    return s_Open && x >= kPanelX && x < kPanelX + kPanelWidth && y >= kPanelY && y < kPanelY + kPanelHeight + 4;
}

void Render()
{
    bool click = MouseLButton && MouseLButtonPush;
    // ── Botón ──
    EnableAlphaTest(true);
    BufferedDraw::Color4f(0.0f, 0.0f, 0.0f, 0.75f);
    GL_DrawRect((float)kButtonX, (float)kButtonY, (float)kButtonSize, (float)kButtonSize);
    const bool overButton = Inside(kButtonX, kButtonY, kButtonSize, kButtonSize);
    BufferedDraw::Color4f(overButton || s_Open ? 0.8f : 0.45f, overButton || s_Open ? 0.6f : 0.35f, 0.4f, 0.9f);
    GL_DrawRect((float)kButtonX, (float)kButtonY, (float)kButtonSize, 1.0f);
    GL_DrawRect((float)kButtonX, (float)(kButtonY + kButtonSize - 1), (float)kButtonSize, 1.0f);
    GL_DrawRect((float)kButtonX, (float)kButtonY, 1.0f, (float)kButtonSize);
    GL_DrawRect((float)(kButtonX + kButtonSize - 1), (float)kButtonY, 1.0f, (float)kButtonSize);
    BufferedDraw::Color4f(1.0f, 1.0f, 1.0f, 1.0f);
    Label(kButtonX, kButtonY + kButtonSize / 2, kButtonSize, u8"\U0001F600");
    if (click && overButton) { s_Open = !s_Open; Consume(); click = false; }
    if (!s_Open) return;

    // ── Panel ──
    EnableAlphaTest(true);
    BufferedDraw::Color4f(0.0f, 0.0f, 0.0f, 0.88f);
    GL_DrawRect((float)kPanelX, (float)kPanelY, (float)kPanelWidth, (float)kPanelHeight);
    BufferedDraw::Color4f(0.8f, 0.6f, 0.4f, 0.9f);
    GL_DrawRect((float)kPanelX, (float)kPanelY, (float)kPanelWidth, 1.0f);
    GL_DrawRect((float)kPanelX, (float)(kPanelY + kPanelHeight - 1), (float)kPanelWidth, 1.0f);
    GL_DrawRect((float)kPanelX, (float)kPanelY, 1.0f, (float)kPanelHeight);
    GL_DrawRect((float)(kPanelX + kPanelWidth - 1), (float)kPanelY, 1.0f, (float)kPanelHeight);

    const std::vector<std::string> list = BuildPage(s_Page);
    const int gridX = kPanelX + 4, gridY = kPanelY + 4;
    int picked = -1;
    for (int i = 0; i < (int)list.size() && i < kColumns * kRows; ++i) {
        const int x = gridX + (i % kColumns) * kCell, y = gridY + (i / kColumns) * kCell;
        if (Inside(x, y, kCell, kCell)) {
            BufferedDraw::Color4f(1.0f, 1.0f, 1.0f, 0.22f);
            GL_DrawRect((float)x, (float)y, (float)kCell, (float)kCell);
            picked = i;
        }
        BufferedDraw::Color4f(1.0f, 1.0f, 1.0f, 1.0f);
        Label(x, y + kCell / 2, kCell, list[i].c_str());
    }

    // Pie: página anterior, "n / total" y página siguiente.
    const int footY = gridY + kRows * kCell + 10;
    const bool canPrev = s_Page > 0, canNext = s_Page < kPages - 1;
    const int prevX = kPanelX + 6, nextX = kPanelX + kPanelWidth - 6 - 16;
    char label[16];
    Platform::TextFormatSecure(label, "%d / %d", s_Page + 1, kPages);
    BufferedDraw::Color4f(1.0f, 1.0f, 1.0f, 1.0f);
    Label(kPanelX, footY, kPanelWidth, label);
    const char* const arrows[2] = { "<", ">" };
    const int arrowX[2] = { prevX, nextX };
    const bool enabled[2] = { canPrev, canNext };
    int step = 0;
    for (int a = 0; a < 2; ++a) {
        if (!enabled[a]) continue;
        const bool over = Inside(arrowX[a], footY - 8, 16, 16);
        BufferedDraw::Color4f(over ? 1.0f : 0.7f, over ? 0.8f : 0.6f, 0.4f, 1.0f);
        Label(arrowX[a], footY, 16, arrows[a]);
        if (over && click) step = a == 0 ? -1 : 1;
    }

    if (click) {
        if (step) { s_Page += step; Consume(); }
        else if (picked >= 0) {
            // Sin lugar (límite de bytes o de caracteres) no se agrega nada.
            Input_AppendText(0, list[picked].c_str());
            Consume();
        } else if (!Inside(kPanelX, kPanelY, kPanelWidth, kPanelHeight)) {
            s_Open = false;   // click afuera: se cierra y el click sigue su camino
        }
    }
    BufferedDraw::Color4f(1.0f, 1.0f, 1.0f, 1.0f);
}
}
