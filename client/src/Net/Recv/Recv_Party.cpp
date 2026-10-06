// Recv_Party.cpp — paquetes del server: party.
//
// Ver Net/Recv/NetRecv.h.

#include "stdafx.h"
#include "Net/Recv/NetRecv.h"

// 0x44
void NetRecv_44(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Party HP bars (ProtocolCore @ 004389A0): entries start at
    // pkt[4], high nibble=party slot and low nibble=HP step.
    NetLog("NET:  → 0x44 PartyHPBars size=%d", Size);
    extern void PacketHandler_0x44(BYTE* pkt, int size);
    PacketHandler_0x44((BYTE*)Msg, Size);
}

// 0x40
void NetRecv_40(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ProtocolCore @ 004389A0: party invitation.  The inviter
    // la clave es big-endian y RenderErrorMessage(0x78) resuelve
    // esta clave al nombre del personaje para el prompt.
    if (Size < 5) return;
    DAT_07eaa0e4 = (DWORD)((Msg[3] << 8) | Msg[4]);
    SetErrorMessage(120);
}

// 0x41
void NetRecv_41(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceivePartyResult @ 004345F0: presentational result only.
    // Membership remains authoritative in the following 0x42.
    if (Size < 4) return;
    static const int textIndex[] = { 497, 498, 499, 500, 501, 535 };
    if (Msg[3] < _countof(textIndex))
        UIChatLogWindow_AddText(nullptr, GlobalText[textIndex[Msg[3]]], 2);
    SetErrorMessage(0);
}

// 0x42
void NetRecv_42(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceivePartyList @ 00434660. Esto no es estado opcional de
    // UI: Combat::Attack usa PartyNumber y la tabla Party original
    // de 36 bytes para autorizar Mana Shield y Teleport
    // Ally before emitting their MuEmu packets.
    extern void ReceivePartyList97k(BYTE* pkt, int size);
    ReceivePartyList97k((BYTE*)Msg, Size);
}

// 0x43
void NetRecv_43(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // ReceivePartyDelete @ 00434640: only the server declares the
    // local removal/dissolution.  Do not elect a leader or mutate
    // rows here; remaining users receive a fresh authoritative 0x42.
    PartyNumber = 0;
    UIChatLogWindow_AddText(nullptr, GlobalText[502], 2);
}

// 0x71
void NetRecv_71(BYTE* Msg, int Size, BYTE hdr, BYTE sub, bool bEncrypted)
{
    // Passive legacy Party reply; current MuEmu has no 0x71 route.
    extern void Party_Keepalive(void);
    Party_Keepalive();
}
