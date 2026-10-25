#include "UI/PanelAnchor.h"
#include <cstdlib>

static void Check(bool ok) { if (!ok) std::abort(); }
int main() {
    // Misma conversión de CWindow: alto completo y 4:3 centrado.
    const int sizes[][2] = {{640,480}, {1280,720}, {1920,1080}, {3440,1440}};
    const Ui::Panel singles[] = {Ui::Character, Ui::Party, Ui::Guild,
        Ui::GoldenArcher, Ui::Quest, Ui::EventTimer};
    const Ui::Panel pairs[] = {Ui::Shop, Ui::Trade, Ui::Warehouse, Ui::Chaos, Ui::Event};
    for (const auto& size : sizes) {
        const int uiWidth = size[1] * 640 / 480;
        const int margin = ((size[0] - uiWidth) / 2) * 640 / uiWidth;
        const int offset = (size[0] - uiWidth) / 2;
        const auto world = Ui::MapViewport(0, 640, size[0], uiWidth, offset);
        Check(world.x == 0 && world.width == size[0]);
        // Dos previews de igual tamaño a ambos lados del borde central.
        const auto centerItem = Ui::MapViewport(500, 20, size[0], uiWidth, offset);
        const auto rightItem = Ui::MapViewport(700, 20, size[0], uiWidth, offset);
        Check(rightItem.width == centerItem.width);
        Check(rightItem.x == offset + 700 * uiWidth / 640);
        const auto leftItem = Ui::MapViewport(-80, 20, size[0], uiWidth, offset);
        Check(leftItem.x == offset - 80 * uiWidth / 640);
        const int edge = Ui::WindowRight.Origin(margin);
        for (auto panel : singles) {
            Check(panel.Origin(margin) + panel.width == edge);
            Check(panel.Origin(margin) == Ui::RightPanel.Origin(margin));
        }
        for (auto panel : pairs) {
            Check(panel.Origin(margin) + panel.width == Ui::RightPanel.Origin(margin));
            Check(panel.Origin(margin) == Ui::InventoryPair.Origin(margin));
        }
        Check(Ui::InventoryPair.Origin(margin) + 190 == Ui::Character.Origin(margin));
        Check(Ui::WindowLeft.Origin(margin) == -margin);
        Check(Ui::ClassicChatX(margin) + 10 == Ui::WindowLeft.Origin(margin));
        Check(Ui::Menu.Origin(margin) * 2 + Ui::Menu.width == 640);
        Check((Ui::Panel{Ui::Anchor::Centro, 190}.Origin(margin)) == 225);
    }
    // Paridad exacta del layout original en 4:3.
    Check(Ui::RightPanel.Origin(0) == 450);
    Check(Ui::InventoryPair.Origin(0) == 260);
    Check(Ui::ClassicChatX(0) == -10);
}
