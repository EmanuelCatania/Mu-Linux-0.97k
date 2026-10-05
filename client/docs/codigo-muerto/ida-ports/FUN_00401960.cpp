// 0x00401960 FUN_00401960 — nunca activado: IDA_PORT_00401960 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── FUN_00401960 (IDA-only, gated) ──
#if defined(IDA_PORT_00401960)
void __cdecl CSQuest::clearQuest(DWORD This)
{
  int v1; // edi
  signed int v2; // ebx
  int v3; // eax
  int v4; // esi
  char buf[4]; // [esp+14h] [ebp-410h] BYREF
  char v6; // [esp+18h] [ebp-40Ch]
  int v7; // [esp+420h] [ebp-4h]

  *(BYTE *)(This + 116863) = 0;
  CloseInventoryRelatedWindows();
  v1 = 0;
  buf[2] = -63;
  v7 = 0;
  v6 = 49;
  buf[3] = 3;
  v2 = 3;
  if ( s != -1 )
  {
    while ( 1 )
    {
      v3 = send(s, &buf[v1 + 2], 3 - v1, 0);
      v4 = v3;
      if ( v3 == -1 )
      {
        break;
      }
      if ( v3 )
      {
        if ( SocketClientLogPrint )
        {
          nullsub_2(&buf[2], v3);
        }
        v2 -= v4;
        v1 += v4;
        if ( v2 > 0 )
        {
          continue;
        }
      }
      return;
    }
    if ( WSAGetLastError() == 10035 && SocketClientSendBufferLength + 3 <= 0x2000 )
    {
      qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &buf[2], v2);
      SocketClientSendBufferLength += v2;
    }
    else
    {
      CWsctlc::Close((DWORD)&SocketClient);
    }
  }
}
#endif
