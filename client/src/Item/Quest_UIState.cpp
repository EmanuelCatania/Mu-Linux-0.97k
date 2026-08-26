// Quest_UIState.cpp
// Extracted from stubs_misc_helpers.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern void Net_SendC1Packet(const BYTE* pkt, int totalLen);

// CSQuest::clearQuest @ 0x00401960 — cierra la ventana de quest.
// 2026-08-21: acá había un "CharSelect_SendClickPacket" que SÓLO mandaba el
// paquete.  Le faltaban las dos cosas que realmente cierran el panel, así que
// el botón X (y cualquier otro camino de cierre) no hacía nada: el flag
// +0x1C87F seguía en 1, GetScreenWidth seguía devolviendo 450 y el panel
// quedaba dibujado para siempre.  IDA:
//     *(_BYTE *)(This + 116863) = 0;
//     CloseInventoryRelatedWindows();
//     send([C1][03][31]);
// El 0x31 (49) va como C1 plano — HackPacketCheck le da Encrypt = 0.
void __fastcall FUN_00401960(int param_1) {
    if (param_1 == 0) return;
    *(BYTE *)(param_1 + 0x1c87f) = 0;
    CloseInventoryRelatedWindows();
    BYTE pkt[3] = { 0xC1, 0x03, 0x31 };
    Net_SendC1Packet(pkt, 3);
}
