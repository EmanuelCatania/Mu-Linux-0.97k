#pragma once
// Party.h — HP bars (0x44), tile update (0x46), keepalive (0x71), char-sync (0x73),
//           guild opcodes 0x90/0x91/0x93/0x94/0x95/0x96/0x99.
void Terrain_TileUpdate(BYTE* pkt);
void Party_Keepalive(void);
void Party_CharSync(BYTE* pkt, int pkt_len);
// Guild handlers
void Guild_CreateOk(BYTE* pkt);
void Guild_AddMemberResult(BYTE* pkt);
void Guild_MemberList(BYTE* pkt);
void Guild_CharSelectResult(BYTE* pkt);
void Guild_UpdatePos(BYTE* pkt);
void Guild_SetTargetPos(BYTE* pkt);
void Guild_JoinToggle(BYTE* pkt);
