// ConnectServerProtocol.h — espejo de los paquetes ConnectServer -> cliente (F4).
//
// Origen: Mu-Linux-0.97k @ 25ceccb,
// Source/MuServer/ConnectServer/ConnectServerProtocol.h. Sin campos View*.
//
// Incluir después de stdafx.h. Todavía no lo usa ningún parseo (Fase 2).
#pragma once

#include "Net/Protocol/ProtocolBase.h"

namespace Proto {

// ── C2:F4:02 ─ lista de servers ── ConnectServerProtocol.h:42 / :48 ───────────
// Net_Process.cpp Recv_ServerList (count en +5, entradas de 4 desde +6).
// El comentario del server dice "C1:F4:02", pero el encabezado es PSWMSG_HEAD (C2).
struct PMSG_SERVER_LIST_SEND
{
    PSWMSG_HEAD header;     // C2:F4:02
    BYTE count;
};
static_assert(sizeof(PMSG_SERVER_LIST_SEND) == 6, "F4:02");
static_assert(offsetof(PMSG_SERVER_LIST_SEND, count) == 5, "F4:02 count");

struct PMSG_SERVER_LIST
{
    WORD ServerCode;
    BYTE UserTotal;
};
static_assert(sizeof(PMSG_SERVER_LIST) == 4, "F4:02 entrada");     // 3 + 1 de padding final
static_assert(offsetof(PMSG_SERVER_LIST, UserTotal) == 2, "F4:02 UserTotal");

// ── C1:F4:03 ─ IP/puerto del GameServer elegido ── ConnectServerProtocol.h:54 ─
// Net_Process.cpp Recv_Redirect (IP en +4, puerto little-endian en +20).
struct PMSG_SERVER_INFO_SEND
{
    PSBMSG_HEAD header;     // C1:F4:03
    char ServerAddress[16];
    WORD ServerPort;
};
static_assert(sizeof(PMSG_SERVER_INFO_SEND) == 22, "F4:03");
static_assert(offsetof(PMSG_SERVER_INFO_SEND, ServerAddress) == 4, "F4:03 ServerAddress");
static_assert(offsetof(PMSG_SERVER_INFO_SEND, ServerPort) == 20, "F4:03 ServerPort");

// ── C2:F4:04 ─ nombres de los grupos de servers ── ConnectServerProtocol.h:30 / :36
// Net_Process.cpp Recv_CustomServerList (count big-endian en +5/+6, entradas de
// 34 desde +7). El comentario del server dice "C1:F4:01", pero lo manda
// CCCustomServerListSend con header.set(0xF4, 0x04) y es PSWMSG_HEAD (C2).
struct PMSG_CUSTOM_SERVER_LIST_SEND
{
    PSWMSG_HEAD header;     // C2:F4:04
    BYTE count[2];          // [0] = HB, [1] = LB
};
static_assert(sizeof(PMSG_CUSTOM_SERVER_LIST_SEND) == 7, "F4:04");
static_assert(offsetof(PMSG_CUSTOM_SERVER_LIST_SEND, count) == 5, "F4:04 count");

struct PMSG_CUSTOM_SERVER_LIST
{
    WORD ServerCode;
    char ServerName[32];
};
static_assert(sizeof(PMSG_CUSTOM_SERVER_LIST) == 34, "F4:04 entrada");
static_assert(offsetof(PMSG_CUSTOM_SERVER_LIST, ServerName) == 2, "F4:04 ServerName");

} // namespace Proto
