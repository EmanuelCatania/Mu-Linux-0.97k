// 0x004F6A70 Net_Disconnect_Clean — nunca activado: IDA_PORT_004F6A70 nunca definida y src/stubs_IDA_ports.cpp fuera del .vcxproj
// ── Net_Disconnect_Clean (IDA-only, gated) ──
#if defined(IDA_PORT_004F6A70)
char __cdecl Net_Disconnect_Clean(int a1, int a2)
{
  int v3; // edi
  signed int v4; // ebx
  int v5; // eax
  int v6; // esi
  char buf[4]; // [esp+4h] [ebp-410h] BYREF
  char v8; // [esp+8h] [ebp-40Ch]
  int v9; // [esp+410h] [ebp-4h]

  if ( EquipmentItem )
  {
    return 0;
  }
  InventoryOpened = 0;
  CloseInventoryRelatedWindows();
  v3 = 0;
  if ( DAT_07e91388 > 0 )
  {
    FUN_004cd3b0(a1, a2);
  }
  buf[2] = -63;
  v9 = 0;
  v8 = -126;
  buf[3] = 3;
  v4 = 3;
  if ( s != -1 )
  {
    while ( 1 )
    {
      v5 = send(s, &buf[v3 + 2], 3 - v3, 0);
      v6 = v5;
      if ( v5 == -1 )
      {
        break;
      }
      if ( v5 )
      {
        if ( SocketClientLogPrint )
        {
          nullsub_2((int)&buf[2], v5);
        }
        v4 -= v6;
        v3 += v6;
        if ( v4 > 0 )
        {
          continue;
        }
      }
      return 1;
    }
    if ( WSAGetLastError() == 10035 && SocketClientSendBufferLength + 3 <= 0x2000 )
    {
      qmemcpy((char *)&SocketClientSendBuffer + SocketClientSendBufferLength, &buf[2], v4);
      SocketClientSendBufferLength += v4;
    }
    else
    {
      CWsctlc::Close((DWORD)&SocketClient);
    }
  }
  return 1;
}
#endif
