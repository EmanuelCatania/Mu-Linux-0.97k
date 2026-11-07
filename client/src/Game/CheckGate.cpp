// CheckGate.cpp
// Entrada a los gates (portales) del mapa.
#include <cstdint>
#include "stdafx.h"
#include "UI/ChatListBox.h"
extern "C" const char* Guild_GetMarkName(int row);
#include "Text/TextCore.h"
#include "Party/Party.h"
#include "Entity/EntityName.h"
#include "Net/NetText.h"
#include "Render/TerrainDraw.h"
void Terrain_BeginBufferedPass();
#include "Render/BufferedDraw.h"
#include "Game/RenderFxSnapshots.h"
#include "Entity/ObjectBounds.h"
#include "Platform/Text.h"
#include "Platform/ISocket.h"
#include "Platform/ITimer.h"
#include "Platform/IInput.h"
#include "Item/ContentCatalog.h"
#include "Item/ItemDefines.h"
#include "Item/RightClickMove.h"
#include "Config/UserSettings.h"
#include "Game/MapManager.h"
#include "structs.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

// El rango de guild pide C1 plano (Encrypt=0).
extern "C" void GuildCreator_CloseFromResult(void);

// Inventory pool bases — defined in src/Render/HUD_Pass3.cpp.
// Lo usan los llamadores de FUN_004d23b0 de abajo para recorrer los arrays de grilla correctos.
extern "C" BYTE OffsetInventoryItems[];
extern "C" BOOL ChaosBoxRequestClose(void);
extern "C" BYTE Inventory[];
extern "C" BYTE ShopItems[];   // pool dedicado de la tienda
extern "C" BYTE OffsetTradeItems[];
extern "C" BYTE OffsetWarehouseItems[];
extern "C" BYTE OffsetMixItems[];

// IDA: CheckGate (0x004AC140)
void __cdecl CheckGate(void)
{
    if (!GateAttribute || !Hero || !CharacterAttribute)
        return;

    const BYTE* const gates = reinterpret_cast<const BYTE*>(GateAttribute);
    BYTE* const hero = (BYTE*)Hero;
    const WORD& level = *(WORD*)((BYTE*)CharacterAttribute + 14);

    for (int gateIndex = 0; gateIndex < 100; ++gateIndex) {
        const BYTE* const gate = gates + gateIndex * 9; // IDA: GateAttribute + 9*i
        if (gate[0] != 1 || gate[1] != (BYTE)gMapManager.GetCurrentMap())
            continue;
        const int heroX = *(const int*)&((CHARACTER*)(uintptr_t)(hero))->PositionX;
        const int heroY = *(const int*)&((CHARACTER*)(uintptr_t)(hero))->PositionY;
        if (heroX < gate[2] || heroY < gate[3] || heroX > gate[4] || heroY > gate[5])
            continue;
        if (DAT_07e11d1c != 0 || hero[EntityOffset(773)] != 0)          // LoadingWorld
            continue;

        unsigned int requiredLevel = gate[8];
        if ((hero[EntityOffset(444)] & 7) == 3) // Magic Gladiator: dos tercios.
            requiredLevel = (requiredLevel * 2) / 3;

        // El gate 28 no entra a la rama comun si no llega al nivel.
        if (gateIndex == 28 && level < requiredLevel)
            continue;

        // Atlans/Tarkan: no con Uniria/Dinorant equipado o agarrado.
        if ((gateIndex >= 45 && gateIndex <= 49) ||
            (gateIndex >= 55 && gateIndex <= 56)) {
            const WORD helper = (WORD)ItemBehaviorType(*(const short*)((const BYTE*)CharacterMachine + 1080));
            const WORD picked = (WORD)ItemBehaviorType(*(const short*)DAT_07e91350);
            if ((helper >= 418 && helper <= 419) ||
                (DAT_07e91388 > 0 && picked >= 418 && picked <= 419)) {
                UIChatLogWindow_AddText("", GlobalText[261], 2);
                continue;
            }
        }

        // Icarus (62..65): alas (384..390) o Dinorant (419); la Uniria no.
        if (gateIndex >= 62 && gateIndex <= 65) {
            const WORD wings = (WORD)ItemBehaviorType(*(const short*)((const BYTE*)CharacterMachine + 1012));
            const WORD helper = (WORD)ItemBehaviorType(*(const short*)((const BYTE*)CharacterMachine + 1080));
            if ((wings < 384 || wings > 390) && helper != 419) {
                UIChatLogWindow_AddText("", GlobalText[263], 2);
                if (level < requiredLevel) {
                    char levelMessage[100];
                    Platform::TextFormatSecure(levelMessage, GlobalText[350], requiredLevel);
                    UIChatLogWindow_AddText("", levelMessage, 2);
                }
                continue;
            }
            if (helper == 418) {
                UIChatLogWindow_AddText("", GlobalText[569], 2);
                continue;
            }
        }

        if (level < requiredLevel) {
            DAT_07e11d1c = 50;                                  // LoadingWorld
            char message[100];
            Platform::TextFormatSecure(message, GlobalText[350], requiredLevel);
            UIChatLogWindow_AddText("", message, 2);
            continue;
        }

        DAT_07e11d1c = 9999999;                                 // LoadingWorld
        if (Teleport || DAT_07e11dc4 || Platform::GetTimer().Ticks() - DAT_07e11dc8 < 3000) {
            DAT_07e11dc4 = 0;
            DAT_07e11d1c = 0;
            continue;
        }

        if (gateIndex == 0)
            Teleport = 1;                                   // Teleport

        const BYTE packet[6] = { 0xC1, 0x06, 0x1C, (BYTE)gateIndex, 0, 0 };
        gNetwork.Send(packet, sizeof(packet));

        SelectedItem = -1;
        SelectedNpc = -1;
        SelectedCharacter = -1;
        SelectedOperate = -1;
        Attacking = -1;                                      // Attacking
        DAT_07e11dc4 = 1;
        DAT_07e11db8 = 0;
    }
}
