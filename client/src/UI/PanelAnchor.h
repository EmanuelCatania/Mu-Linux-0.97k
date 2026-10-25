#pragma once

// Coordenadas lógicas relativas al 4:3 central; los bordes incluyen los márgenes.
namespace Ui {
enum class Anchor { Izquierda, Centro, Derecha };
struct Panel {
    Anchor anchor;
    int width;
    int column = 0;
    constexpr int Origin(int margin) const {
        return anchor == Anchor::Izquierda ? -margin :
               anchor == Anchor::Centro ? (640 - width) / 2 :
               640 + margin - width * (column + 1);
    }
};
inline constexpr Panel RightPanel { Anchor::Derecha, 190 };
inline constexpr Panel InventoryPair { Anchor::Derecha, 190, 1 };
// Declaraciones de panel: ancla, ancho lógico y columna desde el borde.
inline constexpr Panel Character { Anchor::Derecha, 190 };
inline constexpr Panel Party { Anchor::Derecha, 190 };
inline constexpr Panel Guild { Anchor::Derecha, 190 };
inline constexpr Panel Warehouse { Anchor::Derecha, 190, 1 };
inline constexpr Panel Shop { Anchor::Derecha, 190, 1 };
inline constexpr Panel Trade { Anchor::Derecha, 190, 1 };
inline constexpr Panel Chaos { Anchor::Derecha, 190, 1 };
inline constexpr Panel Event { Anchor::Derecha, 190, 1 };
inline constexpr Panel Move { Anchor::Izquierda, 180 };
struct Rect {
    int x, y, width, height;
    constexpr bool Contains(int mx, int my) const {
        return mx >= x && mx < x + width && my >= y && my < y + height;
    }
};
// El selector de eventos comparte rectángulos entre dibujo y entrada.
constexpr Rect EventButton(bool blood, int row, int margin) {
    return {Event.Origin(margin) + 25, (blood ? 170 : 210) + row * (blood ? 40 : 45),
            140, blood ? 33 : 35};
}
inline constexpr Panel GoldenArcher { Anchor::Derecha, 190 };
inline constexpr Panel Quest { Anchor::Derecha, 190 };
constexpr Rect QuestBounds(int margin) { return {Quest.Origin(margin), 0, Quest.width, 433}; }
constexpr Rect QuestProceed(int margin) { return {Quest.Origin(margin) + 35, 355, 120, 24}; }
constexpr Rect QuestClose(int margin) { return {Quest.Origin(margin) + 25, 395, 24, 24}; }
constexpr int QuestAnswerCenter(int margin) { return Quest.Origin(margin) + 106; }
inline constexpr Panel EventTimer { Anchor::Derecha, 190 };
inline constexpr Panel Menu { Anchor::Centro, 120 };
inline constexpr Panel WindowLeft { Anchor::Izquierda, 0 };
inline constexpr Panel WindowRight { Anchor::Derecha, 0 };
constexpr int ClassicChatX(int margin) { return WindowLeft.Origin(margin) - 10; }

struct HorizontalViewport { int x, width; };
// Sólo un viewport que toca exactamente el borde del 4:3 se extiende.
// Un ítem situado más allá de ese borde conserva su propio ancho.
constexpr HorizontalViewport MapViewport(int x, int width, int realWidth, int uiWidth, int offset) {
    if (realWidth == uiWidth)
        return {int((long long)realWidth * x / 640), int((long long)realWidth * width / 640)};
    const int left = x == 0 ? 0 : offset + int((long long)uiWidth * x / 640);
    const int right = x + width == 640 ? realWidth : offset + int((long long)uiWidth * (x + width) / 640);
    return {left, right - left};
}
}
