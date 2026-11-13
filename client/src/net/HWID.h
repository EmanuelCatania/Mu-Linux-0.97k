#pragma once
// HWID.h — hardware fingerprint for server anti-cheat / blacklist.
//
// The MuEmu GameServer (see Source/MuServer/GameServer/Blacklist.cpp::CheckHardwareID)
// rejects any F1/01 login whose HardwareID is empty with code 0x05 ("blacklist").
// So the client MUST send a non-empty F1/05 SetHwid packet BEFORE F1/01.
//
// The fingerprint format is 35 chars: "%08X-%08X-%08X-%08X", matching the
// companion-client implementation at Mu-linux-97K/Source/Client/Main/HWID.cpp.
//  id1 = C: volume serial        ^ 0x12B586FE
//  id2 = UUID node bytes[2..5]   ^ 0x5D78A569
//  id3 = UUID seq/clock lo16 | SystemInfo.wProcessorArchitecture << 16  ^ 0xF45BC123
//  id4 = ProcessorLevel | (ProcessorRevision << 16)                     ^ 0xB542D8E1

#include "stdafx.h"

// Compute the hardware ID and cache it. Safe to call once at startup.
// Returns false if the volume serial read fails (in that case a fallback
// deterministic value is used so we still send a non-empty HWID).
bool HWID_Init(void);

// Send the cached HWID to the server as a C1 plaintext F1/05 packet.
// Must be called after JoinServer (F1/00) and before SendLogin (F1/01).
void HWID_Send(void);

// Read-only access to the 35-char (36 with NUL) fingerprint string.
const char* HWID_Get(void);

// Send F1/04 SetLanguage packet.  Companion's Protocol.cpp:771 calls this
// from GCConnectClientRecv right before SendHwid — the MuEmu GameServer
// expects both packets after F1/00 JoinServer and before F1/01 Login,
// otherwise it silently drops the login packet (no LoginResult, no FD_CLOSE).
// Layout matches PMSG_SET_LANG_SEND from companion's Protocol.h:382:
//   PSBMSG_HEAD header (C3, F1, 04, size=8) + int lang (4 bytes) = 8 bytes
void Lang_Send(int langNum);
