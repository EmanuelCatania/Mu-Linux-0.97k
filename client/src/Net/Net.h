#pragma once
// Net.h - Network layer declarations
//
// Architecture (all functions use a socket context object at 0x055ca160):
//
//   Socket context struct (at 0x055ca160, size ~0x4020+):
//     +0x0008  SOCKET socket             (0xFFFFFFFF = disconnected)
//     +0x000C  byte   send_buf[0x2000]   outgoing data
//     +0x200C  int    send_len           bytes pending in send_buf
//     +0x2010  byte   recv_buf[0x2000]   incoming data
//     +0x4010  int    recv_len           bytes received
//     +0x401C  int    slot_status        (1=free, 0=in-use) used by Net_GetFreeBuffer
//     +0x4024  ...    usable data area   (start of recv payload)
//
//   Connection globals:
//     DAT_055ca168  = SOCKET handle (also at context+8)
//     DAT_05826cf0  = connected flag (1=connected, 0=disconnected)
//     PTR_005615b8  = server IP string
//     DAT_005615bc  = server port (ushort)
//
//   Packet format (Mu "C1" protocol):
//     byte[0] = 0xC1   (header type)
//     byte[1] = length (total packet size)
//     byte[2] = opcode
//     byte[3+] = encrypted payload
//
//   Known opcodes (client→server):
//     0xF1  = login request (sent after connect)
//     0xF3  = character selection (Game_CharSelectTick)
//     0x0E  = keep-alive ping (sent from WM_TIMER every 20s)
//
//   WndProc WM_USER (0x400) sub-dispatch:
//     wParam==1   → Net_Recv     (recv ready via WSAAsyncSelect)
//     wParam==2   → Net_Send     (send ready via WSAAsyncSelect)
//     wParam==0x20→ Net_Connect  then socket ready

#include "stdafx.h"

// Connect to game server.
// param_1: socket context object
// param_1: server IP string (or hostname → resolved via gethostbyname)
// param_2: port
// Returns: 0=ok, 1=no socket, 2=host not found
// @ 0x0043DCD0
int __cdecl Net_Connect(void* ctx, char* ip, unsigned short port, unsigned int flags);

// Receive pending data from server into recv_buf.
// Calls recv(), then processes complete packets.
// Returns: 1=disconnected/error, 3=incomplete header, 0=ok
// @ 0x0043DE70
int Net_Recv(void* ctx);

// Send all pending bytes in send_buf to server.
// Calls send() in a loop until all bytes sent.
// Error 0x2733 (WSAEWOULDBLOCK): returns 1 (retry later).
// Other error: calls Net_Disconnect.
// @ 0x0043DDD0
int Net_Send(int ctx);

// Close socket and mark as disconnected.
// Sets DAT_05826cf0 = 0, closesocket(), socket = 0xFFFFFFFF.
// @ 0x0043DC90
int Net_Disconnect(int ctx);

// Find a free buffer slot in the recv pool.
// Pool: 300 entries at fixed base, stride 0x2008, status at +0x401C.
// Returns ptr+0x4024 (payload area), or 0 if pool full.
// @ 0x0043E010
int Net_GetFreeBuffer(int poolBase);

// Incoming server→client packet dispatcher.
// Called after DispatchMessageA in WinMain.
// Large switch on opcode — 1824 lines, 489 basic blocks.
// @ 0x004389A0
void Net_ProcessPacket(void);

// Send a packet to the server.
// Builds header (0xC1, len, opcode) then queues in send_buf.
// @ address TBD
void Net_SendPacket(const void* data, int size);

// Send raw bytes over the socket (applies MuEmu/HackCheck byte cipher).
// Used for plaintext C1-framed packets like keep-alives and HWID.
// Returns 0 on success, -1 on hard error.
int  Net_SendBuf(const char* buf, int len);

// Send a small packet: plaintext [0xC1][len][opcode][payload].
// Encrypts pkt[1..totalLen-1] via CSimpleModulus and wraps with C3 framing.
void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

// Envía una trama C1 sin cifrar según la política de tramas del protocolo.
void Net_SendC1Packet(const BYTE* pkt, int totalLen);

// Send a large packet: plaintext [0xC1][len][opcode][payload].
// Encrypts pkt[1..totalLen-1] and wraps with [0xC4][hi][lo] framing.
void Net_SendLargePacket(const BYTE* pkt, int totalLen);
