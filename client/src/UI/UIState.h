#pragma once

// Consultas del estado legado; cada sistema conserva su propia política.
namespace UIState {
    bool HasTextInput();
    bool HasModalDialog();
    bool IsItemMovePending();
    bool IsMixBusy();
    bool IsQuestPanelOpen();
    bool HasRightPanel();
    bool CanOpenInformationalPanel();
    bool CanQueryChaosRate();
    void CaptureMouseForUI();
}
