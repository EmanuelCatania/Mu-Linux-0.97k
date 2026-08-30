#pragma once
// Party.h — barras de HP (0x44), actualización de terreno (0x46) y keepalive (0x71).
void Terrain_TileUpdate(BYTE* pkt);
void Party_Keepalive(void);
// Rebuilds only runtime viewport links (+28/+32); Party membership remains
// authoritative in the server's 0x42/0x43 lifecycle.
void Party_RefreshViewportLinks(void);
// Guild handlers
void Guild_CreateOk(BYTE* pkt);
void Guild_AddMemberResult(BYTE* pkt);
void Guild_MemberList(BYTE* pkt);
void Guild_CharSelectResult(BYTE* pkt);
void Guild_UpdatePos(BYTE* pkt);
void Guild_SetTargetPos(BYTE* pkt);
void Guild_JoinToggle(BYTE* pkt);
