#pragma once
// Skills.h — PacketHandler_0x19 (skill dispatch).
void __cdecl PacketHandler_0x19(BYTE* pkt, int len);
// PacketHandler_0x16 removida 2026-09-02: era una rama inventada, no un port
// de 0x0042DB60 (ReceiveDieExp).  El port fiel esta inline en Net_Process.cpp,
// case 0x16.  Ver la nota en Skills.cpp.
