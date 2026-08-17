#pragma once
// Combat.h — PacketHandlers 0x12..0x23: attack, animation, death, ragdoll, ground items.
// All handlers called via Net_ProcessPacket dispatch table.
void __cdecl Combat_PacketDispatch(BYTE opcode, BYTE* pkt, int len);
