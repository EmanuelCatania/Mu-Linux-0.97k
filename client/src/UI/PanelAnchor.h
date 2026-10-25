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
inline constexpr Panel GoldenArcher { Anchor::Derecha, 190 };
inline constexpr Panel Quest { Anchor::Derecha, 190 };
inline constexpr Panel EventTimer { Anchor::Derecha, 190 };
inline constexpr Panel Menu { Anchor::Centro, 120 };
inline constexpr Panel WindowLeft { Anchor::Izquierda, 0 };
inline constexpr Panel WindowRight { Anchor::Derecha, 0 };
constexpr int ClassicChatX(int margin) { return WindowLeft.Origin(margin) - 10; }
}
