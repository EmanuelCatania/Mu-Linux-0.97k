#pragma once
// Trade.h — helpers heredados de shop. Guerra de guild posee los opcodes 0x60–0x63.
int  __cdecl Shop_RegisterItem(DWORD entity_id, DWORD* name, BYTE* item_data);
void __cdecl PacketHandler_0x5a(BYTE* pkt);
void __cdecl Shop_EntitySlots(BYTE* pkt);
void __cdecl PacketHandler_0x5c(BYTE* pkt);
void __cdecl PacketHandler_0x5d(BYTE* pkt);
