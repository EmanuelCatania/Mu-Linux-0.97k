#include "stdafx.h"
#include "UI/UIState.h"

namespace UIState {

bool HasTextInput()
{
    // DLL Defines.h: CheckInputInterfaces; DAT_07e11d71 es TabInputEnable.
    return InputEnable || GuildInputEnable || GoldInputEnable || DAT_07e11d71;
}

bool HasModalDialog() { return DAT_083a7c24 != 0; } // ErrorMessage
bool IsItemMovePending() { return DAT_07eaa165 != 0; } // EquipmentItem
bool IsMixBusy() { return MixState != 0; }

// IDA: GetScreenWidth (0x004CB520), flag de apertura de quest.
bool IsQuestPanelOpen()
{
    constexpr size_t OpenFlag = 0x1C87F;
    return g_csQuest && *((const BYTE*)(uintptr_t)g_csQuest + OpenFlag) != 0;
}

bool HasRightPanel()
{
    return InventoryOpened || CharacterOpened || GuildOpened || PartyOpened ||
        ShopOpened || WarehouseOpened || ChaosMixOpened || TradeOpened ||
        EventWindowOpened || GuildCreatorOpened || _g_bEventChipDialogEnable ||
        ServerDivisionOpened || IsQuestPanelOpen();
}

bool CanOpenInformationalPanel()
{
    // Conserva los bloqueos de M/H; la alternancia lateral se integra con su UI.
    return SceneFlag == 5 && !HasTextInput() && !HasModalDialog() &&
        !IsItemMovePending() && !HasRightPanel();
}

bool CanQueryChaosRate()
{
    // Chaos debe estar abierto; no comparte el bloqueo de paneles informativos.
    return ChaosMixOpened && !IsItemMovePending() && !IsMixBusy();
}

void CaptureMouseForUI() { DAT_07d78094 = 1; } // MouseOnWindow

} // namespace UIState
