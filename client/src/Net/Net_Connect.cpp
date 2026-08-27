// Net_Connect.cpp
// Network connection setup and packet hash-table bootstrap.
//
// FUN_00423920 @ 0x00423920 — Net_Connect_Server
//
// Called from login flow to establish TCP connection to game/login server.
// Initialises the packet hash table for session key tracking.

#include "stdafx.h"
#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);

// FUN_00423920 — Net_Connect_Server
// Connects to server at param_1 (IP string) on port param_2.
// On failure: logs error, calls FUN_004055a0(1) to abort, plays music 0x71.
// On success: bootstraps the send/recv hash table entries for the session key
// bytes at DAT_05826ceb/cec (encrypt + decrypt key registration via
// FUN_00404330/FUN_00423710).
// Returns unaff_EBP (register spill — result from lower-level connect call).
void __cdecl Net_ConnectServer(const char *param_1,unsigned int param_2)
{
  byte bVar1;
  int iVar2;
  uint uVar3;
  void *pvVar4;
  byte *pbVar5;
  uint uVar6;
  int iVar7;
  char **ppcVar8;
  byte *pbVar9;
  byte *pbVar10;
  bool bVar11;
  bool bVar12;
  byte local_4 [4];

  local_4[0] = 1;
  local_4[1] = 0;
  local_4[2] = 0;
  local_4[3] = 0;
  {
    char dbg[160];
    wsprintfA(dbg, "NET: Net_ConnectServer connect_req ip=%.64s port=%u",
              param_1 ? param_1 : "(null)", (unsigned)(param_2 & 0xFFFF));
    DbgLogPublic(dbg);
  }
  if (DAT_055ca15c == '\0') {
    DAT_055ca15c = '\x01';
    FUN_0043db30(((int)(uintptr_t)DAT_055ca160));
  }
  FUN_0043dbf0(&DAT_055ca160,(int)(uintptr_t)DAT_055c9ffc);
  iVar2 = Net_Connect(&DAT_055ca160,(char*)param_1,(u_short)param_2,0x400);
  {
    char dbg[64];
    wsprintfA(dbg, "NET: Net_ConnectServer Net_Connect returned %d", iVar2);
    DbgLogPublic(dbg);
  }
  if (iVar2 == 0) {
    DbgLogPublic("NET: Net_ConnectServer CONNECT FAILED → abort/SetErrorMessage(0x71)");
    FUN_00405540(&DAT_055c9bf0,s_Failed_to_connect__00559688);
    FUN_004055a0(1);
    FUN_005142d0(0x71);
    // Original-client behaviour: the server can't be reached (no listener,
    // wrong IP/port, firewall) → pop a modal dialog and terminate.  Previous
    // build silently carried on and the player saw nothing.  Same rationale
    // as the FD_CLOSE handler in WinMain.cpp:765 — MessageBoxA is modal, then
    // PostQuitMessage exits the game-loop cleanly.  Guard against re-entry
    // in case the login retry logic fires Net_Connect a second time.
    static bool s_ConnFailHandled = false;
    if (!s_ConnFailHandled) {
        s_ConnFailHandled = true;
        MessageBoxA((HWND)(uintptr_t)DAT_055c9ffc,
                    "No se puede conectar con el servidor.",
                    "Mu Online",
                    MB_OK | MB_ICONERROR);
        PostQuitMessage(0);
    }
    // Skip the hash-table bootstrap below — it operates on a live session key
    // and there is no session if connect failed.  Returning here also avoids
    // touching DAT_05826ceb/cec on a half-initialised ctx.
    return;
  }
  // Lo que sigue es ofuscación HashTable inlined sobre DAT_05826ceb/cec
  // (ref-count de session-key bytes). Neutralizado vía fake vtable en
  // DAT_055c9bc8 + capacity (DAT_055c9bd4) = 0 → loops se saltean.
  param_1 = &DAT_05826ceb;
  uVar3 = (**(code **)(DAT_055c9bc8 + 0xc))(&DAT_05826ceb);
  uVar6 = 0;
  param_1 = (char *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar11 = false;
      iVar7 = 0;
      bVar12 = true;
      pbVar5 = (byte *)(DAT_055c9bd0 + uVar3 * 4);
      iVar2 = 4;
      ppcVar8 = (char**)&param_1;
      pbVar9 = pbVar5;
      do {
        if (iVar2 == 0) break;
        iVar2 = iVar2 + -1;
        bVar11 = *(byte *)ppcVar8 < *pbVar9;
        bVar12 = *(byte *)ppcVar8 == *pbVar9;
        ppcVar8 = (char **)((int)ppcVar8 + 1);
        pbVar9 = pbVar9 + 1;
      } while (bVar12);
      if (!bVar12) {
        iVar7 = (1 - (uint)bVar11) - (uint)(bVar11 != 0);
      }
      if (iVar7 == 0) goto LAB_00423a34;
      iVar2 = 4;
      bVar11 = false;
      iVar7 = 0;
      bVar12 = true;
      pbVar9 = (byte *)local_4;
      do {
        if (iVar2 == 0) break;
        iVar2 = iVar2 + -1;
        bVar11 = *pbVar9 < *pbVar5;
        bVar12 = *pbVar9 == *pbVar5;
        pbVar9 = pbVar9 + 1;
        pbVar5 = pbVar5 + 1;
      } while (bVar12);
      if (!bVar12) {
        iVar7 = (1 - (uint)bVar11) - (uint)(bVar11 != 0);
      }
      if (iVar7 == 0) {
        if (uVar3 == 0xffffffff) goto LAB_00423a34;
        pbVar5 = (byte *)FUN_00404280(&DAT_055c9bc8,&DAT_05826ceb);
        bVar1 = pbVar5[1];
        pbVar5[1] = bVar1 + 1;
        if ((byte)(bVar1 + 1) < 2) {
          FUN_00404330(&DAT_05826ceb,pbVar5);
        }
        goto LAB_00423a52;
      }
      uVar3 = (uVar3 + 1) % DAT_055c9bd4;
      uVar6 = uVar6 + 1;
    } while (uVar6 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00423a34:
  pvVar4 = operator_new(2);
  *(undefined1 *)((int)pvVar4 + 1) = 1;
  FUN_00403f80(&DAT_055c9bc8,pvVar4,&DAT_05826ceb);
LAB_00423a52:
  DAT_05826ceb = 0;
  uVar3 = (**(code **)(DAT_055c9bc8 + 0xc))(&DAT_05826ceb);
  uVar6 = 0;
  if (DAT_055c9bd4 != 0) {
    do {
      iVar2 = 4;
      bVar11 = false;
      iVar7 = 0;
      bVar12 = true;
      pbVar5 = (byte *)(DAT_055c9bd0 + uVar3 * 4);
      pbVar9 = (byte *)local_4;
      pbVar10 = pbVar5;
      do {
        if (iVar2 == 0) break;
        iVar2 = iVar2 + -1;
        bVar11 = *pbVar9 < *pbVar10;
        bVar12 = *pbVar9 == *pbVar10;
        pbVar9 = pbVar9 + 1;
        pbVar10 = pbVar10 + 1;
      } while (bVar12);
      if (!bVar12) {
        iVar7 = (1 - (uint)bVar11) - (uint)(bVar11 != 0);
      }
      if (iVar7 == 0) goto LAB_00423b59;
      iVar2 = 4;
      bVar11 = false;
      iVar7 = 0;
      bVar12 = true;
      pbVar9 = local_4;
      do {
        if (iVar2 == 0) break;
        iVar2 = iVar2 + -1;
        bVar11 = *pbVar9 < *pbVar5;
        bVar12 = *pbVar9 == *pbVar5;
        pbVar9 = pbVar9 + 1;
        pbVar5 = pbVar5 + 1;
      } while (bVar12);
      if (!bVar12) {
        iVar7 = (1 - (uint)bVar11) - (uint)(bVar11 != 0);
      }
      if (iVar7 == 0) {
        if (uVar3 != 0xffffffff) {
          pbVar5 = (byte *)FUN_00404280(&DAT_055c9bc8,&DAT_05826ceb);
          bVar1 = pbVar5[1];
          pbVar5[1] = bVar1 - 1;
          if ((byte)(bVar1 - 1) == 0) {
            FUN_00423710(pbVar5,&DAT_05826ceb);
          }
        }
        goto LAB_00423b59;
      }
      uVar3 = (uVar3 + 1) % DAT_055c9bd4;
      uVar6 = uVar6 + 1;
    } while (uVar6 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_00423b59:
  uVar3 = HashTable_GetIndex(&DAT_055c9bc8,&DAT_05826cec);
  if (uVar3 == 0xffffffff) {
    pvVar4 = operator_new(2);
    *(undefined1 *)((int)pvVar4 + 1) = 1;
    FUN_00403f80(&DAT_055c9bc8,pvVar4,&DAT_05826cec);
  }
  else {
    uVar3 = HashTable_GetIndex(&DAT_055c9bc8,&DAT_05826cec);
    if (uVar3 == 0xffffffff) {
      pbVar5 = (byte *)0x0;
    }
    else {
      pbVar5 = *(byte **)(DAT_055c9bcc + uVar3 * 4);
    }
    bVar1 = pbVar5[1];
    pbVar5[1] = bVar1 + 1;
    if ((byte)(bVar1 + 1) < 2) {
      FUN_00404330(&DAT_05826cec,pbVar5);
    }
  }
  DAT_05826cec = 0;
  uVar3 = HashTable_GetIndex(&DAT_055c9bc8,&DAT_05826cec);
  if (uVar3 != 0xffffffff) {
    uVar3 = HashTable_GetIndex(&DAT_055c9bc8,&DAT_05826cec);
    if (uVar3 == 0xffffffff) {
      pbVar5 = (byte *)0x0;
    }
    else {
      pbVar5 = *(byte **)(DAT_055c9bcc + uVar3 * 4);
    }
    bVar1 = pbVar5[1];
    pbVar5[1] = bVar1 - 1;
    if ((byte)(bVar1 - 1) == 0) {
      FUN_00423710(pbVar5,&DAT_05826cec);
    }
  }
  return;
}


// =============================================================================
// FUN_0043dcd0 @ 0x0043DCD0 — CWsctlc::Connect(this, ip_addr, port, WinMsgNum)
// (251 bytes per IDA decompile raw/0043DCD0_CWsctlc_Connect.c).
//
// Lower-level TCP connect helper used by CWsctlc socket sessions. The `This`
// pointer is a CWsctlc instance with layout:
//   This[+0]  HWND  hWnd  (the window that will receive WSAAsyncSelect events)
//   This[+8]  SOCKET socket descriptor (created earlier by Net_Connect_Server)
//
// Behavior:
//   1. Validate hWnd != 0; otherwise show error MessageBox + return 0.
//   2. Resolve ip_addr: try inet_addr first, fall back to gethostbyname.
//   3. Call connect(); accept either success or WSAEWOULDBLOCK (10035).
//   4. Register WSAAsyncSelect for FD_READ|FD_WRITE|FD_CONNECT|FD_CLOSE
//      (= 0x23 = bit pattern from IDA constant).
//
// Returns:
//   1 = connect ok (or pending non-blocking)
//   2 = host name resolution failure
//   0 = hWnd null OR connect failed -> closesocket
//
// 2026-05-08: ported as part of the companion-DLL Offsets.h cross-reference.
// Our existing Net_Connect_Server (FUN_00423920) handles the higher-level
// connect flow including hash-table bootstrap; CWsctlc::Connect is the inner
// connect+select helper.  Currently no caller in our build references it
// directly (we connect via different code paths) but the symbol is exported
// for completeness.
// =============================================================================
extern "C" int __cdecl CWsctlc_Connect(DWORD This, const char* ip_addr,
                                       unsigned short port, DWORD WinMsgNum)
{
    if (!This || !*(DWORD*)This) {
        MessageBoxA(0, "Window handle missing", "CWsctlc::Connect error", 0);
        return 0;
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip_addr);

    if (addr.sin_addr.s_addr == INADDR_NONE) {
        struct hostent* host = gethostbyname(ip_addr);
        if (!host || !host->h_addr_list || !host->h_addr_list[0]) {
            return 2;  // host resolution failure
        }
        memcpy(&addr.sin_addr, host->h_addr_list[0], host->h_length);
    }

    SOCKET sock = (SOCKET)*(DWORD*)(This + 8);
    int conn = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    if (conn != -1 || WSAGetLastError() == WSAEWOULDBLOCK) {
        // Async select: FD_READ|FD_WRITE|FD_CONNECT|FD_CLOSE = 0x23
        // (per IDA constant — matches FD_READ(1) | FD_CONNECT(0x10) |
        //  FD_CLOSE(0x20) — the binary uses 0x35 in some calls, 0x23 here.)
        if (WSAAsyncSelect(sock, *(HWND*)This, WinMsgNum, 0x23) != -1) {
            return 1;
        }
    }
    closesocket(sock);
    return 0;
}

// IDA-style FUN_ alias.
int __cdecl FUN_0043dcd0(DWORD This, const char* ip_addr,
                         unsigned short port, DWORD WinMsgNum)
{
    return CWsctlc_Connect(This, ip_addr, port, WinMsgNum);
}
