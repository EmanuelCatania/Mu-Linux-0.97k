#pragma once
// Trade.h — Shop item table, opcodes 0x5A-0x5D/0x60-0x63: NPC shop, trade open/result.
int  __cdecl Shop_RegisterItem(DWORD entity_id, DWORD* name, BYTE* item_data);
void __cdecl PacketHandler_0x5a(BYTE* pkt);
void __cdecl Shop_EntitySlots(BYTE* pkt);
void __cdecl PacketHandler_0x5c(BYTE* pkt);
void __cdecl PacketHandler_0x5d(BYTE* pkt);
