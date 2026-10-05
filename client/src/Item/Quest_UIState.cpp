// Quest_UIState.cpp

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);

// CSQuest::clearQuest @ 0x00401960 — cierra la ventana de quest.
// IDA:
//     *(_BYTE *)(This + 116863) = 0;
//     CloseInventoryRelatedWindows();
//     send([C1][03][31]);
// El 0x31 (49) va como C1 plano — HackPacketCheck le da Encrypt = 0.
// IDA: CSQuest::clearQuest (0x00401960)
void __fastcall CSQuest_clearQuest(int param_1) {
    if (param_1 == 0) return;
    *(BYTE *)(param_1 + 0x1c87f) = 0;
    CloseInventoryRelatedWindows();
    BYTE pkt[3] = { 0xC1, 0x03, 0x31 };
    Net_SendC1Packet(pkt, 3);
}
